/*
 * Copyright (C) 2008 Nokia Corporation
 * Copyright (c) International Business Machines Corp., 2006
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See
 * the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * Generate UBI images.
 *
 * Authors: Artem Bityutskiy
 *          Oliver Lohmann
 */

#define PROGRAM_NAME    "ubinize"

#include <sys/stat.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <fcntl.h>
#include <libgen.h>

#include <mtd/ubi-media.h>
#include <libubigen.h>
#include <libiniparser.h>
#include <libubi.h>
#include "common.h"
#include "ubiutils-common.h"
#include "sparse_format.h"

#define SPARSE_HEADER_MAJOR_VER 1
#define SPARSE_HEADER_MINOR_VER 0
#define SPARSE_HEADER_LEN       (sizeof(sparse_header_t))
#define CHUNK_HEADER_LEN (sizeof(chunk_header_t))
#define MAX_FILE_NAME		256
#define SLC_MODE_IMAGE		"tmp-slc.image"
#define TLC_MODE_IMAGE		"tmp-tlc.image"
static const char doc[] = PROGRAM_NAME " version " VERSION
" - a tool to generate UBI images. An UBI image may contain one or more UBI "
"volumes which have to be defined in the input configuration ini-file. The "
"ini file defines all the UBI volumes - their characteristics and the and the "
"contents, but it does not define the characteristics of the flash the UBI "
"image is generated for. Instead, the flash characteristics are defined via "
"the command-line options. Note, if not sure about some of the command-line "
"parameters, do not specify them and let the utility to use default values.";

static const char optionsstr[] =
"-o, --output=<file name>     output file name\n"
"-p, --peb-size=<bytes>       size of the physical eraseblock of the flash\n"
"                             this UBI image is created for in bytes,\n"
"                             kilobytes (KiB), or megabytes (MiB)\n"
"                             (mandatory parameter)\n"
"-m, --min-io-size=<bytes>    minimum input/output unit size of the flash\n"
"                             in bytes\n"
"-s, --sub-page-size=<bytes>  minimum input/output unit used for UBI\n"
"                             headers, e.g. sub-page size in case of NAND\n"
"                             flash (equivalent to the minimum input/output\n"
"                             unit size by default)\n"
"-O, --vid-hdr-offset=<num>   offset if the VID header from start of the\n"
"                             physical eraseblock (default is the next\n"
"                             minimum I/O unit or sub-page after the EC\n"
"                             header)\n"
"-e, --erase-counter=<num>    the erase counter value to put to EC headers\n"
"                             (default is 0)\n"
"-x, --ubi-ver=<num>          UBI version number to put to EC headers\n"
"                             (default is 1)\n"
"-Q, --image-seq=<num>        32-bit UBI image sequence number to use\n"
"                             (by default a random number is picked)\n"
"-v, --verbose                be verbose\n"
"-h, --help                   print help message\n"
"-V, --version                print program version";

static const char usage[] =
"Usage: " PROGRAM_NAME " [-o filename] [-p <bytes>] [-m <bytes>] [-s <bytes>] [-O <num>] [-e <num>]\n"
"\t\t[-x <num>] [-Q <num>] [-v] [-h] [-V] [--output=<filename>] [--peb-size=<bytes>]\n"
"\t\t[--min-io-size=<bytes>] [--sub-page-size=<bytes>] [--vid-hdr-offset=<num>]\n"
"\t\t[--erase-counter=<num>] [--ubi-ver=<num>] [--image-seq=<num>] [--verbose] [--help]\n"
"\t\t[--version] ini-file\n"
"Example: " PROGRAM_NAME " -o ubi.img -p 16KiB -m 512 -s 256 cfg.ini - create UBI image\n"
"         'ubi.img' as described by configuration file 'cfg.ini'";

static const char ini_doc[] = "INI-file format.\n"
"The input configuration ini-file describes all the volumes which have to\n"
"be included to the output UBI image. Each volume is described in its own\n"
"section which may be named arbitrarily. The section consists on\n"
"\"key=value\" pairs, for example:\n\n"
"[jffs2-volume]\n"
"mode=ubi\n"
"image=../jffs2.img\n"
"vol_id=1\n"
"vol_size=30MiB\n"
"vol_type=dynamic\n"
"vol_name=jffs2_volume\n"
"vol_flags=autoresize\n"
"vol_alignment=1\n\n"
"This example configuration file tells the utility to create an UBI image\n"
"with one volume with ID 1, volume size 30MiB, the volume is dynamic, has\n"
"name \"jffs2_volume\", \"autoresize\" volume flag, and alignment 1. The\n"
"\"image=../jffs2.img\" line tells the utility to take the contents of the\n"
"volume from the \"../jffs2.img\" file. The size of the image file has to be\n"
"less or equivalent to the volume size (30MiB). The \"mode=ubi\" line is\n"
"mandatory and just tells that the section describes an UBI volume - other\n"
"section modes may be added in the future.\n"
"Notes:\n"
"  * size in vol_size might be specified kilobytes (KiB), megabytes (MiB),\n"
"    gigabytes (GiB) or bytes (no modifier);\n"
"  * if \"vol_size\" key is absent, the volume size is assumed to be\n"
"    equivalent to the size of the image file (defined by \"image\" key);\n"
"  * if the \"image\" is absent, the volume is assumed to be empty;\n"
"  * volume alignment must not be greater than the logical eraseblock size;\n"
"  * one ini file may contain arbitrary number of sections, the utility will\n"
"    put all the volumes which are described by these section to the output\n"
"    UBI image file.";

static const struct option long_options[] = {
	{ .name = "output",         .has_arg = 1, .flag = NULL, .val = 'o' },
	{ .name = "peb-size",       .has_arg = 1, .flag = NULL, .val = 'p' },
	{ .name = "min-io-size",    .has_arg = 1, .flag = NULL, .val = 'm' },
	{ .name = "sub-page-size",  .has_arg = 1, .flag = NULL, .val = 's' },
	{ .name = "vid-hdr-offset", .has_arg = 1, .flag = NULL, .val = 'O' },
	{ .name = "erase-counter",  .has_arg = 1, .flag = NULL, .val = 'e' },
	{ .name = "ubi-ver",        .has_arg = 1, .flag = NULL, .val = 'x' },
	{ .name = "image-seq",      .has_arg = 1, .flag = NULL, .val = 'Q' },
	{ .name = "verbose",        .has_arg = 0, .flag = NULL, .val = 'v' },
	{ .name = "help",           .has_arg = 0, .flag = NULL, .val = 'h' },
	{ .name = "version",        .has_arg = 0, .flag = NULL, .val = 'V' },
	{ NULL, 0, NULL, 0}
};

struct args {
	const char *f_in;
	const char *f_out;
	int out_fd;
	int in_fd;
	int peb_size;
	int min_io_size;
	int subpage_size;
	int vid_hdr_offs;
	int ec;
	int ubi_ver;
	uint32_t image_seq;
	int verbose;
	dictionary *dict;
	//for tlc mode
	uint32_t tlc_mode;
	uint32_t slc_block;
	uint32_t tlc_block;
	uint32_t used_slc_blk;
	int fd_slc;
	int fd_tlc;
};

static struct args args = {
	.peb_size     = -1,
	.min_io_size  = -1,
	.subpage_size = -1,
	.ubi_ver      = 1,
	.slc_block	  = 0,
	.tlc_block	  = 0,
	.tlc_mode	  = 0,
};

static int parse_opt(int argc, char * const argv[])
{
	ubiutils_srand();
	args.image_seq = rand();

	while (1) {
		int key, error = 0;
		unsigned long int image_seq;

		key = getopt_long(argc, argv, "o:p:m:s:O:e:x:Q:vhV", long_options, NULL);
		if (key == -1)
			break;

		switch (key) {
		case 'o':
			args.out_fd = open(optarg, O_CREAT | O_TRUNC | O_WRONLY,
					   S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH);
			if (args.out_fd == -1)
				return sys_errmsg("cannot open file \"%s\"", optarg);
			args.f_out = optarg;
			break;

		case 'p':
			args.peb_size = ubiutils_get_bytes(optarg);
			if (args.peb_size <= 0)
				return errmsg("bad physical eraseblock size: \"%s\"", optarg);
			break;

		case 'm':
			args.min_io_size = ubiutils_get_bytes(optarg);
			if (args.min_io_size <= 0)
				return errmsg("bad min. I/O unit size: \"%s\"", optarg);
			if (!is_power_of_2(args.min_io_size))
				return errmsg("min. I/O unit size should be power of 2");
			break;

		case 's':
			args.subpage_size = ubiutils_get_bytes(optarg);
			if (args.subpage_size <= 0)
				return errmsg("bad sub-page size: \"%s\"", optarg);
			if (!is_power_of_2(args.subpage_size))
				return errmsg("sub-page size should be power of 2");
			break;

		case 'O':
			args.vid_hdr_offs = simple_strtoul(optarg, &error);
			if (error || args.vid_hdr_offs < 0)
				return errmsg("bad VID header offset: \"%s\"", optarg);
			break;

		case 'e':
			args.ec = simple_strtoul(optarg, &error);
			if (error || args.ec < 0)
				return errmsg("bad erase counter value: \"%s\"", optarg);
			break;

		case 'x':
			args.ubi_ver = simple_strtoul(optarg, &error);
			if (error || args.ubi_ver < 0)
				return errmsg("bad UBI version: \"%s\"", optarg);
			break;

		case 'Q':
			image_seq = simple_strtoul(optarg, &error);
			if (error || image_seq > 0xFFFFFFFF)
				return errmsg("bad UBI image sequence number: \"%s\"", optarg);
			args.image_seq = image_seq;
			break;

		case 'v':
			args.verbose = 1;
			break;

		case 'h':
			ubiutils_print_text(stdout, doc, 80);
			printf("\n%s\n\n", ini_doc);
			printf("%s\n\n", usage);
			printf("%s\n", optionsstr);
			exit(EXIT_SUCCESS);

		case 'V':
			common_print_version();
			exit(EXIT_SUCCESS);

		default:
			fprintf(stderr, "Use -h for help\n");
			return -1;
		}
	}

	if (optind == argc)
		return errmsg("input configuration file was not specified (use -h for help)");

	if (optind != argc - 1)
		return errmsg("more then one configuration file was specified (use -h for help)");

	args.f_in = argv[optind];

	if (args.peb_size < 0)
		return errmsg("physical eraseblock size was not specified (use -h for help)");

#ifdef	__MTK_SLC_BUFFER_SUPPORT
	verbose(args.verbose, "physical eraseblock size %d", args.peb_size);
#else
	if (args.peb_size > UBI_MAX_PEB_SZ)
		return errmsg("too high physical eraseblock size %d", args.peb_size);
#endif
	if (args.min_io_size < 0)
		return errmsg("min. I/O unit size was not specified (use -h for help)");

	if (args.subpage_size < 0)
		args.subpage_size = args.min_io_size;

	if (args.subpage_size > args.min_io_size)
		return errmsg("sub-page cannot be larger then min. I/O unit");

	if (args.peb_size % args.min_io_size)
		return errmsg("physical eraseblock should be multiple of min. I/O units");

	if (args.min_io_size % args.subpage_size)
		return errmsg("min. I/O unit size should be multiple of sub-page size");

	if (!args.f_out)
		return errmsg("output file was not specified (use -h for help)");

	if (args.vid_hdr_offs) {
		if (args.vid_hdr_offs + (int)UBI_VID_HDR_SIZE >= args.peb_size)
			return errmsg("bad VID header position");
		if (args.vid_hdr_offs % 8)
			return errmsg("VID header offset has to be multiple of min. I/O unit size");
	}

	return 0;
}

static int read_section(const struct ubigen_info *ui, const char *sname,
			struct ubigen_vol_info *vi, const char **img,
			struct stat *st)
{
	char buf[256];
	const char *p;

	*img = NULL;

	if (strlen(sname) > 128)
		return errmsg("too long section name \"%s\"", sname);

	/* Make sure mode is UBI, otherwise ignore this section */
	sprintf(buf, "%s:mode", sname);
	p = iniparser_getstring(args.dict, buf, NULL);
	if (!p) {
		errmsg("\"mode\" key not found in section \"%s\"", sname);
		errmsg("the \"mode\" key is mandatory and has to be "
		       "\"mode=ubi\" if the section describes an UBI volume");
		return -1;
	}

	/* If mode is not UBI, skip this section */
	if (strcmp(p, "ubi")) {
		verbose(args.verbose, "skip non-ubi section \"%s\"", sname);
		return 1;
	}

	verbose(args.verbose, "mode=ubi, keep parsing");

	/* Fetch volume type */
	sprintf(buf, "%s:vol_type", sname);
	p = iniparser_getstring(args.dict, buf, NULL);
	if (!p) {
		normsg("volume type was not specified in "
		       "section \"%s\", assume \"dynamic\"\n", sname);
		vi->type = UBI_VID_DYNAMIC;
	} else {
		if (!strcmp(p, "static"))
			vi->type = UBI_VID_STATIC;
		else if (!strcmp(p, "dynamic"))
			vi->type = UBI_VID_DYNAMIC;
		else
			return errmsg("invalid volume type \"%s\" in section  \"%s\"",
				      p, sname);
	}

	verbose(args.verbose, "volume type: %s",
		vi->type == UBI_VID_DYNAMIC ? "dynamic" : "static");

	/* Fetch the name of the volume image file */
	sprintf(buf, "%s:image", sname);
	p = iniparser_getstring(args.dict, buf, NULL);
	if (p) {
		*img = p;
		if(*img[0] != '/') {
			int path_len = strlen(args.f_in) + strlen(*img);
			char *tmp_path, *new_path = malloc(path_len);
			tmp_path = strdup(args.f_in);
			memset(new_path, 0, path_len);
			sprintf(new_path, "%s/%s", dirname(tmp_path), *img);
			*img = new_path;
		}

		if (stat(*img, st))
			return sys_errmsg("cannot stat \"%s\" referred from section \"%s\"",
					  *img, sname);
		if (st->st_size == 0)
			return errmsg("empty file \"%s\" referred from section \"%s\"",
				       p, sname);
	} else if (vi->type == UBI_VID_STATIC)
		return errmsg("image is not specified for static volume in section \"%s\"",
			      sname);

	/* Fetch volume id */
	sprintf(buf, "%s:vol_id", sname);
	vi->id = iniparser_getint(args.dict, buf, -1);
	if (vi->id == -1)
		return errmsg("\"vol_id\" key not found in section  \"%s\"", sname);
	if (vi->id < 0)
		return errmsg("negative volume ID %d in section \"%s\"",
			      vi->id, sname);
	if (vi->id >= ui->max_volumes)
		return errmsg("too high volume ID %d in section \"%s\", max. is %d",
			      vi->id, sname, ui->max_volumes);

	verbose(args.verbose, "volume ID: %d", vi->id);

	/* Fetch volume size */
	sprintf(buf, "%s:vol_size", sname);
	p = iniparser_getstring(args.dict, buf, NULL);
	if (p) {
		vi->bytes = ubiutils_get_bytes(p);
		if (vi->bytes <= 0)
			return errmsg("bad \"vol_size\" key value \"%s\" (section \"%s\")",
				      p, sname);

		/* Make sure the image size is not larger than volume size */
		if (*img && st->st_size > vi->bytes)
			return errmsg("error in section \"%s\": size of the image file "
				      "\"%s\" is %lld, which is larger than volume size %lld",
				      sname, *img, (long long)st->st_size, vi->bytes);
		verbose(args.verbose, "volume size: %lld bytes", vi->bytes);
	} else {
		struct stat st;

		if (!*img)
			return errmsg("neither image file (\"image=\") nor volume size "
				      "(\"vol_size=\") specified in section \"%s\"", sname);

		if (stat(*img, &st))
			return sys_errmsg("cannot stat \"%s\"", *img);

		vi->bytes = st.st_size;

		if (vi->bytes == 0)
			return errmsg("file \"%s\" referred from section \"%s\" is empty",
				      *img, sname);

		normsg_cont("volume size was not specified in section \"%s\", assume"
			    " minimum to fit image \"%s\"", sname, *img);
		ubiutils_print_bytes(vi->bytes, 1);
		printf("\n");
	}

	/* Fetch volume name */
	sprintf(buf, "%s:vol_name", sname);
	p = iniparser_getstring(args.dict, buf, NULL);
	if (!p)
		return errmsg("\"vol_name\" key not found in section \"%s\"", sname);

	vi->name = p;
	vi->name_len = strlen(p);
	if (vi->name_len > UBI_VOL_NAME_MAX)
		return errmsg("too long volume name in section \"%s\", max. is %d characters",
			      vi->name, UBI_VOL_NAME_MAX);

	verbose(args.verbose, "volume name: %s", p);

	/* Fetch volume alignment */
	sprintf(buf, "%s:vol_alignment", sname);
	vi->alignment = iniparser_getint(args.dict, buf, -1);
	if (vi->alignment == -1)
		vi->alignment = 1;
	else if (vi->id < 0)
		return errmsg("negative volume alignement %d in section \"%s\"",
			      vi->alignment, sname);

	verbose(args.verbose, "volume alignment: %d", vi->alignment);

	/* Fetch volume flags */
	sprintf(buf, "%s:vol_flags", sname);
	p = iniparser_getstring(args.dict, buf, NULL);
	if (p) {
		if (!strcmp(p, "autoresize")) {
			verbose(args.verbose, "autoresize flags found");
			vi->flags |= UBI_VTBL_AUTORESIZE_FLG;
		} else {
			return errmsg("unknown flags \"%s\" in section \"%s\"",
				      p, sname);
		}
	}

	/* Initialize the rest of the volume information */
	vi->data_pad = ui->leb_size % vi->alignment;
	vi->usable_leb_size = ui->leb_size - vi->data_pad;
	if (vi->type == UBI_VID_DYNAMIC)
		vi->used_ebs = (vi->bytes + vi->usable_leb_size - 1) / vi->usable_leb_size;
	else
		vi->used_ebs = (st->st_size + vi->usable_leb_size - 1) / vi->usable_leb_size;
	vi->compat = 0;
	return 0;
}

int ubinize_fill_empty_block(const struct ubigen_info *ui, int out) 
{
	char *outbuf;
	outbuf = malloc(ui->peb_size);
	if (!outbuf) {
		sys_errmsg("cannot allocate %d bytes of memory", ui->peb_size);
		goto out_free;
	}
	memset(outbuf, 0xFF, ui->peb_size);
	if (write(out, outbuf, ui->peb_size) != ui->peb_size) {
		sys_errmsg("cannot write %d bytes to the output file", ui->peb_size);
		goto out_free;
	}
	free(outbuf);
	return 0;

out_free:
	free(outbuf);
	return -1;
}

int add_sparse_image(int fd_src, int chunk_mode, int nand_mode, uint64_t hole_len)
{
	int err;
	size_t len, total = 0;
	int rnd_up_len, left;
	chunk_header_t chunk_header;
	unsigned char *buf = NULL;

	if(CHUNK_TYPE_DONT_CARE == chunk_mode){
		/* We are skipping data, so emit a don't care chunk. */
		chunk_header.chunk_type = CHUNK_TYPE_DONT_CARE;
		chunk_header.reserved1 = nand_mode;
		chunk_header.chunk_sz = hole_len / args.peb_size;
		chunk_header.total_sz = CHUNK_HEADER_LEN;
		err = write(args.out_fd, &chunk_header, CHUNK_HEADER_LEN);
		if (err < 0)
			return err;
		return 0;
	}
	lseek(fd_src, 0, SEEK_SET);
	len = lseek(fd_src, 0, SEEK_END);

	/* Round up the data length to a multiple of the block size */
	rnd_up_len = ALIGN(len, args.peb_size);
	left = len % args.peb_size;
	/* Finally we can safely emit a chunk of data */
	chunk_header.chunk_type = chunk_mode;
	chunk_header.reserved1 = nand_mode;
	chunk_header.chunk_sz = rnd_up_len / args.peb_size;
	chunk_header.total_sz = CHUNK_HEADER_LEN + rnd_up_len;

	err = write(args.out_fd, &chunk_header, CHUNK_HEADER_LEN);
	if (err < 0){
		return -errno;
	}

	buf = malloc(args.peb_size);
	if(!buf){
		return sys_errmsg("failed to malloc tmp buf\n");
	}

	lseek(fd_src, 0, SEEK_SET);
	while (total < (len - left)) {
		err = read(fd_src, buf, args.peb_size);
		if (err < 0){
			free(buf);
			return -errno;
		}

		if (err == 0)
			break;

		total += err;
		err = write(args.out_fd, buf, err);
		if (err < 0){
			free(buf);
			return -errno;
		}
	}

	return 0;
}

int make_sparse_image(int *tlc_used_peb)
{
	int err;
	size_t slc_len, tlc_len,  part_size;
	uint64_t hole_len;
	sparse_header_t sparse_header;

	lseek(args.fd_slc, 0, SEEK_SET);
	slc_len = lseek(args.fd_slc, 0, SEEK_END);
	part_size = args.slc_block * args.peb_size;
	if(slc_len > part_size){
		return sys_errmsg("slc part size is not enough to storge slc image\n");
	}

	lseek(args.fd_tlc, 0, SEEK_SET);
	tlc_len = lseek(args.fd_tlc, 0, SEEK_END);
	part_size = args.tlc_block * args.peb_size;
	if(tlc_len > part_size){
		return sys_errmsg("tlc part size is not enough to storge tlc image\n");
	}
	*tlc_used_peb = tlc_len / args.peb_size;
	printf("tlc_len %d *tlc_used_peb %d\n", tlc_len, *tlc_used_peb);
	sparse_header.magic = SPARSE_HEADER_MAGIC;
	sparse_header.major_version = SPARSE_HEADER_MAJOR_VER;
	sparse_header.minor_version = SPARSE_HEADER_MINOR_VER;
	sparse_header.file_hdr_sz = SPARSE_HEADER_LEN;
	sparse_header.chunk_hdr_sz = CHUNK_HEADER_LEN;
	sparse_header.blk_sz = args.peb_size;
	sparse_header.total_blks = slc_len / args.peb_size + args.tlc_block;
	sparse_header.total_chunks = 3;//tlc chunk,hole chunk and slc chunk data
	sparse_header.image_checksum = 0;

	lseek(args.out_fd, 0, SEEK_SET);
	err = write(args.out_fd, &sparse_header, SPARSE_HEADER_LEN);
	if(err < 0){
		return sys_errmsg("failed to add sparse file header\n");
	}

	err = add_sparse_image(args.fd_tlc, CHUNK_TYPE_RAW, NAND_TLC_PART, 0);
	if(err < 0){
		return sys_errmsg("failed to add tlc part to sparse image\n");
	}
	hole_len = (uint64_t)args.tlc_block * args.peb_size - tlc_len;
	err = add_sparse_image(0, CHUNK_TYPE_DONT_CARE, NAND_TLC_PART, hole_len);
	if(err < 0){
		return sys_errmsg("failed to add tlc hole to sparse image\n");
	}
	err = add_sparse_image(args.fd_slc, CHUNK_TYPE_RAW, NAND_SLC_PART, slc_len);
	if(err < 0){
		return sys_errmsg("failed to add slc part to sparse image\n");
	}
	return 0;
}

int write_to_slc_image(struct ubigen_info *ui,
			const struct ubigen_vol_info *vi, const char *img)
{
	int lnum=0;
	int fd;
	int err;
	off_t len;
	struct stat st;

	fd = open(img, O_RDONLY);
	if (fd == -1) {
		return sys_errmsg("slc:cannot open \"%s\"", img);
	}

	verbose(args.verbose, "slc:writing volume %d", vi->id);
	verbose(args.verbose, "slc:image file: %s", img);

	if (stat(img, &st))
		return sys_errmsg("slc:cannot stat \"%s", img);

	len = ui->leb_size * args.used_slc_blk;
	if (st.st_size < len ){
		len = st.st_size;
	}
	err = ubigen_write_volume(ui, vi, args.ec, len, fd, 0, args.fd_slc, &lnum);
	close(fd);
	if (err) {
		return errmsg("slc:cannot write volume to slc section ");
	}
	verbose(args.verbose, "slc:writing volume size %d", lnum);

	return 0;
}


int write_to_tlc_image(struct ubigen_info *ui,
		struct ubigen_vol_info *vi, const char *img)
{
	int lnum=0;
	int fd;
	int err;
	off_t seek;
	off_t len;
	struct stat st;

	fd = open(img, O_RDONLY);
	if (fd == -1) {
		return sys_errmsg("tlc:cannot open \"%s\"", img);
	}

	verbose(args.verbose, "tlc:writing volume %d", vi->id);
	verbose(args.verbose, "tlc:image file: %s", img);

	if (stat(img, &st))
		return sys_errmsg("tlc:cannot stat \"%s", img);

	len = ui->leb_size * args.used_slc_blk;
	if ( st.st_size >= len ){
		len = st.st_size - len;
		seek = ui->leb_size * args.used_slc_blk;
		if (lseek(fd, seek, SEEK_SET) != seek) {
			return sys_errmsg("tlc:cannot seek file \"%s\"", TLC_MODE_IMAGE);
		}
	} else {
		return 0;
	}
	seek = lseek(args.fd_tlc, 0, SEEK_END);
	vi->start = seek / args.peb_size;
	err = ubigen_write_volume(ui, vi, args.ec, len, fd, args.used_slc_blk, args.fd_tlc, &lnum);
	close(fd);
	if (err) {
		return errmsg("tlc:cannot write volume to slc section ");
	}
	seek = lseek(args.fd_tlc, 0, SEEK_END);
	vi->stop = seek / args.peb_size;
	verbose(args.verbose, "tlc:writing volume size %d, from %d to %d", lnum, vi->start, vi->stop);

	return 0;
}

int init_tlc_info(void)
{
	char buf[256];
	const char *sname = "tlc_info";
	off_t seek;

	sprintf(buf, "%s:tlc_blk", sname);
	args.tlc_block = iniparser_getint(args.dict, buf, -1);
	if (args.tlc_block <= 0)
		return errmsg("bad tlc block number");
	verbose(args.verbose, "tlc block number:                  %d", args.tlc_block);

	sprintf(buf, "%s:slc_blk", sname);
	args.slc_block = iniparser_getint(args.dict, buf, -1);
	if (args.slc_block <= 0)
		return errmsg("bad slc block number");
	verbose(args.verbose, "slc block number:                  %d", args.slc_block);

	sprintf(buf, "%s:ocp_slc", sname);
	args.used_slc_blk = iniparser_getint(args.dict, buf, -1);
	if (args.used_slc_blk <= 0)
		return errmsg("bad used slc block number");
	verbose(args.verbose, "used block number:                 %d", args.used_slc_blk);
	args.tlc_mode = 1;

	seek = args.peb_size * 3;
	if (lseek(args.fd_slc, seek, SEEK_SET) != seek) {
		return sys_errmsg("cannot seek slc file \"%s\"", SLC_MODE_IMAGE);
	}
	return 0;
}
int main(int argc, char * const argv[])
{
	int err = -1, sects, i, autoresize_was_already = 0;
	struct ubigen_info ui;
	struct ubi_vtbl_record *vtbl;
	struct ubigen_vol_info *vi;
	off_t seek;
	int fm_disabled = 0;
	char *tmp = NULL;
	char *filename = NULL;

	err = parse_opt(argc, argv);
	if (err)
		return -1;

	verbose(args.verbose, "loaded the ini-file \"%s\"", args.f_in);	
	args.dict = iniparser_load(args.f_in);
	if (!args.dict) {
		errmsg("cannot load the input ini file \"%s\"", args.f_in);
		goto out;
	}

	err = iniparser_find_entry(args.dict, "tlc_info");
	if (err) {
		verbose(args.verbose, "find tlc info in the ini-file");

		tmp = malloc(MAX_FILE_NAME);
		filename = malloc(MAX_FILE_NAME);
		if(!tmp || !filename){
			printf("Failed to alloc memory!\n");
			return -1;
		}

		memset(tmp, 0, MAX_FILE_NAME);
		memset(filename, 0, MAX_FILE_NAME);
		strcpy(tmp, args.f_out);
		sprintf(filename, "%s/%s", dirname(tmp), SLC_MODE_IMAGE);
		args.fd_slc= open(filename, O_CREAT | O_TRUNC | O_RDWR, S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH);

		memset(tmp, 0, MAX_FILE_NAME);
		memset(filename, 0, MAX_FILE_NAME);
		strcpy(tmp, args.f_out);
		sprintf(filename, "%s/%s", dirname(tmp), TLC_MODE_IMAGE);
		args.fd_tlc = open(filename, O_CREAT | O_TRUNC | O_RDWR, S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH);
		if ((args.fd_slc == -1) || (args.fd_tlc== -1))
			return sys_errmsg("cannot create slc or tlc image ");

		init_tlc_info();
		free(tmp);
		free(filename);
	} else {
		verbose(args.verbose, "can not find tlc info in the ini-file");
		args.tlc_mode = 0;
	}

#ifdef __MTK_SLC_BUFFER_SUPPORT
	    ubigen_info_init(&ui, args.peb_size, args.min_io_size,
				 args.subpage_size, args.tlc_block, args.vid_hdr_offs,
				 args.ubi_ver, args.image_seq);
#else
		ubigen_info_init(&ui, args.peb_size, args.min_io_size,
				 args.subpage_size, args.vid_hdr_offs,
				 args.ubi_ver, args.image_seq);
#endif

	verbose(args.verbose, "LEB size:                  %d", ui.leb_size);
	verbose(args.verbose, "PEB size:                  %d", ui.peb_size);
	verbose(args.verbose, "min. I/O size:             %d", ui.min_io_size);
	verbose(args.verbose, "sub-page size:             %d", args.subpage_size);
	verbose(args.verbose, "VID offset:                %d", ui.vid_hdr_offs);
	verbose(args.verbose, "data offset:               %d", ui.data_offs);
#ifdef __MTK_SLC_BUFFER_SUPPORT
	verbose(args.verbose, "maintain table size:       %d", ui.mtbl_size);
#endif
	verbose(args.verbose, "UBI image sequence number: %u", ui.image_seq);

	vtbl = ubigen_create_empty_vtbl(&ui);
	if (!vtbl)
		goto out;

	/* Each section describes one volume */
	sects = iniparser_getnsec(args.dict);
	if (sects == -1) {
		errmsg("ini-file parsing error (iniparser_getnsec)");
		goto out_dict;
	}

	verbose(args.verbose, "count of sections: %d", sects);
	if (sects == 0) {
		errmsg("no sections found the ini-file \"%s\"", args.f_in);
		goto out_dict;
	}

	if (sects > ui.max_volumes) {
		errmsg("too many sections (%d) in the ini-file \"%s\"",
		       sects, args.f_in);
		normsg("each section corresponds to an UBI volume, maximum "
		       "count of volumes is %d", ui.max_volumes);
		goto out_dict;
	}

	vi = calloc(sizeof(struct ubigen_vol_info), sects);
	if (!vi) {
		errmsg("cannot allocate memory");
		goto out_dict;
	}

	/*
	 * Skip 2 PEBs at the beginning of the file for the volume table which
	 * will be written later.
	 */
	seek = ui.peb_size * 2;
	if (lseek(args.out_fd, seek, SEEK_SET) != seek) {
		sys_errmsg("cannot seek file \"%s\"", args.f_out);
		goto out_free;
	}
#ifdef MTK_NAND_UBIFS_FASTMAP_SUPPORT
	ubinize_fill_empty_block(&ui, args.out_fd);
#endif

	for (i = 0; i < sects; i++) {
		const char *sname = iniparser_getsecname(args.dict, i);
		const char *img = NULL;
		struct stat st;
		int fd, j;

		if(!strcmp(sname, "tlc_info")){
			verbose(args.verbose, "skip parsing section \"%s\"", sname);
			continue;
		}

		if (!sname) {
			errmsg("ini-file parsing error (iniparser_getsecname)");
			goto out_free;
		}

		if (args.verbose)
			printf("\n");
		verbose(args.verbose, "parsing section \"%s\"", sname);

		err = read_section(&ui, sname, &vi[i], &img, &st);
		if (err == -1)
			goto out_free;

		verbose(args.verbose, "adding volume %d", vi[i].id);

		/*
		 * Make sure that volume ID and name is unique and that only
		 * one volume has auto-resize flag
		 */
		for (j = 0; j < i; j++) {
			if (vi[j].name && strcmp(vi[j].name, "tlc_info")){
				;
			} else {
				continue;
			}

			if (vi[i].id == vi[j].id) {
				errmsg("volume IDs must be unique, but ID %d "
				       "in section \"%s\" is not",
				       vi[i].id, sname);
				goto out_free;
			}

			if (!strcmp(vi[i].name, vi[j].name)) {
				errmsg("volume name must be unique, but name "
				       "\"%s\" in section \"%s\" is not",
				       vi[i].name, sname);
				goto out_free;
			}
		}

		if (vi[i].flags & UBI_VTBL_AUTORESIZE_FLG) {
			if (autoresize_was_already)
				return errmsg("only one volume is allowed "
					      "to have auto-resize flag");
			autoresize_was_already = 1;
		}

		err = ubigen_add_volume(&ui, &vi[i], vtbl);
		if (err) {
			errmsg("cannot add volume for section \"%s\"", sname);
			goto out_free;
		}

		if (args.tlc_mode && img){
			err = write_to_slc_image(&ui, &vi[i], img);
			if (err) {
				return errmsg("failed to write volume :\"%s\" to slc image", vi[i].name);
			}
			err = write_to_tlc_image(&ui, &vi[i], img);
			if (err) {
				return errmsg("failed to write volume :\"%s\" to tlc image", vi[i].name);
			}
		} else if (img) {
			int lnum=0;
			fd = open(img, O_RDONLY);
			if (fd == -1) {
				sys_errmsg("cannot open \"%s\"", img);
				goto out_free;
			}

			verbose(args.verbose, "writing volume %d", vi[i].id);
			verbose(args.verbose, "image file: %s", img);
				
			err = ubigen_write_volume(&ui, &vi[i], args.ec, st.st_size, fd, 0, args.out_fd, &lnum);
			close(fd);
			if (err) {
				errmsg("cannot write volume for section \"%s\"", sname);
				goto out_free;
			}
			verbose(args.verbose, "writing volume size %d", lnum);
		} else {
			vi[i].start = -1;
			vi[i].stop = -1;
		}

		if (args.verbose)
			printf("\n");
	}

	verbose(args.verbose, "writing layout volume");
	if (args.tlc_mode){
		int tlc_peb=0;
		err = ubigen_write_layout_vol(&ui, 0, 1, args.ec, args.ec, vtbl, args.fd_slc);
		if (err) {
			errmsg("cannot write layout volume");
			goto out_free;
		}
		verbose(args.verbose, "writing maintain volume");
		err = ubigen_write_maintain_vol(&ui, 2, args.tlc_block, args.ec, args.fd_slc, vi, sects);
		if (err) {
			errmsg("cannot write maintain volume");
			goto out_free;
		}

		if (lseek(args.out_fd, 0, SEEK_SET) != 0) {
			return sys_errmsg("cannot seek file \"%s\"", args.f_out);
		}
		err = make_sparse_image(&tlc_peb);
		if (err){
			errmsg("failed to make sparse image");
			goto out_free;
		}

		close(args.fd_slc);
		close(args.fd_tlc);
	}else {
		err = ubigen_write_layout_vol(&ui, 0, 1, args.ec, args.ec, vtbl, args.out_fd);
		if (err) {
			errmsg("cannot write layout volume");
			goto out_free;
		}
	}
	verbose(args.verbose, "done");

	free(vi);
	iniparser_freedict(args.dict);
	free(vtbl);
	close(args.out_fd);
	return 0;

out_free:
	free(vi);
out_dict:
	iniparser_freedict(args.dict);
out:
	close(args.out_fd);
	remove(args.f_out);
	return err;
}
