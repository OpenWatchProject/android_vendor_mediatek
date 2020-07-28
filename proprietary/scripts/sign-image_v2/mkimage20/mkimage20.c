/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */


#include <stdint.h>
#include <unistd.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define IMG_MAGIC         0x58881688
#define EXT_MAGIC         0x58891689

#define IMG_NAME_SIZE       32
#define IMG_HDR_SIZE        512

/* image types */
#define IMG_TYPE_ID_OFFSET           (0)
#define IMG_TYPE_RESERVED0_OFFSET    (8)
#define IMG_TYPE_RESERVED1_OFFSET    (16)
#define IMG_TYPE_GROUP_OFFSET        (24)

#define IMG_TYPE_ID_MASK             (0xffU << IMG_TYPE_ID_OFFSET)
#define IMG_TYPE_RESERVED0_MASK      (0xffU << IMG_TYPE_RESERVED0_OFFSET)
#define IMG_TYPE_RESERVED1_MASK      (0xffU << IMG_TYPE_RESERVED1_OFFSET)
#define IMG_TYPE_GROUP_MASK          (0xffU << IMG_TYPE_GROUP_OFFSET)

#define IMG_TYPE_GROUP_AP            (0x00U << IMG_TYPE_GROUP_OFFSET)
#define IMG_TYPE_GROUP_MD            (0x01U << IMG_TYPE_GROUP_OFFSET)
#define IMG_TYPE_GROUP_CERT          (0x02U << IMG_TYPE_GROUP_OFFSET)

/* AP group */
#define IMG_TYPE_IMG_AP_BIN (0x00 | IMG_TYPE_GROUP_AP)

/* MD group */
#define IMG_TYPE_IMG_MD_LTE (0x00 | IMG_TYPE_GROUP_MD)
#define IMG_TYPE_IMG_MD_C2K (0x01 | IMG_TYPE_GROUP_MD)

/* CERT group */
#define IMG_TYPE_CERT1      (0x00 | IMG_TYPE_GROUP_CERT)
#define IMG_TYPE_CERT1_MD   (0x01 | IMG_TYPE_GROUP_CERT)
#define IMG_TYPE_CERT2      (0x02 | IMG_TYPE_GROUP_CERT)

#define HDR_VERSION         1

#define IMG_PATH_IDX        1
#define IMG_CFG_IDX         2

//#define DEBUG_MODE

typedef union {
	struct {
		unsigned int magic;     /* always IMG_MAGIC */
		unsigned int
		dsize;     /* image size, image header and padding are not included */
		char name[IMG_NAME_SIZE];
		unsigned int maddr;     /* image load address in RAM */
		unsigned int mode;      /* maddr is counted from the beginning or end of RAM */
		/* extension */
		unsigned int ext_magic;    /* always EXT_MAGIC */
		unsigned int
		hdr_size;     /* header size is 512 bytes currently, but may extend in the future */
		unsigned int hdr_version;  /* see HDR_VERSION */
		unsigned int
		img_type;     /* please refer to #define beginning with IMG_TYPE_ */
		unsigned int
		img_list_end; /* end of image list? 0: this image is followed by another image 1: end */
		unsigned int
		align_size;   /* image size alignment setting in bytes, 16 by default for AES encryption */
		unsigned int
		dsize_extend; /* high word of image size for 64 bit address support */
		unsigned int
		maddr_extend; /* high word of image load address in RAM for 64 bit address support */
	} info;
	unsigned char data[IMG_HDR_SIZE];
} IMG_HDR_T;

unsigned int filesize(char *name)
{
	struct stat statbuf;

	if (stat(name, &statbuf) != 0) {
		fprintf(stderr, "Cannot open file %s\n", name);
		exit(0);
	}
	return statbuf.st_size;
}

char *readfile(char *name, unsigned int size)
{
	FILE *f;
	char *buf = NULL;

	f = fopen(name, "rb");
	if (f == NULL) {
		fprintf(stderr, "Cannot open file %s\n", name);
		goto _end;
	}

	buf = (char *)malloc(size);
	if (!buf) {
		fprintf(stderr, "error while malloc(%d)\n", size);
		goto _error;
	}

	if (fread(buf, 1, size, f) != size) {
		fprintf(stderr, "Error while reading file %s\n", name);
		free(buf);
		buf = NULL;
		goto _error;
	}

_error:
	fclose(f);
_end:
	return buf;
}

char xtod(char c)
{
	if (c >= '0' && c <= '9') return c - '0';
	if (c >= 'A' && c <= 'F') return c - 'A' + 10;
	if (c >= 'a' && c <= 'f') return c - 'a' + 10;
	return 0;
}

unsigned long long hex2dec(char *hex, int l)
{
	if (*hex == 0)
		return l;
	return hex2dec(hex + 1, l * 16 + xtod(*hex));
}

unsigned long long xstr2int(char *hex)
{
	return hex2dec(hex, 0);
}

int remove_chr_from_string(char *string, char c)
{
	int final_str_len = 0;
	final_str_len = strlen(string);
	int i = 0;

	while (i < final_str_len) {
		if (string[i] == c) {
			memmove(&string[i], &string[i + 1], final_str_len - i - 1);
			final_str_len--;
			string[final_str_len] = 0;
		}
		i++;
	}

	return 0;
}

int get_img_hdr_setting_from_cfg(const char *cfg_path, IMG_HDR_T *img_hdr)
{
#define MAX_LINE_LENGTH  (80)
	int ret = 0;
	FILE *fp = NULL;
	char line[MAX_LINE_LENGTH] = {0};

	fp = fopen(cfg_path, "r");
	if (NULL == fp) {
		fprintf(stderr, "Cannot open file %s\n", cfg_path);
		exit(0);
	}

	while (fgets(line, MAX_LINE_LENGTH, fp) != NULL) {
		int i = 0;
		char *obj_name = NULL;
		char *obj_value_str = NULL;
		unsigned int obj_value = 0;
		ret = remove_chr_from_string(line, ' ');
		ret = remove_chr_from_string(line, '\n');
		obj_name  = strtok(line, "=");
		if (NULL == obj_name)
			continue;
		obj_value_str = strtok(NULL, "=");
		if (NULL == obj_value_str || !strcmp(obj_name, "NAME"))
			obj_value = 0;
		else if (obj_value_str[0] == '0' && obj_value_str[1] == 'x')
			obj_value = xstr2int(obj_value_str);
		else
			obj_value = atoi(obj_value_str);

#ifdef DEBUG_MODE
		fprintf(stderr, "name = %s, value_str = %s, value = %d\n", obj_name,
			obj_value_str, obj_value);
#endif

		if (!strcmp(obj_name, "LOAD_ADDR"))
			img_hdr->info.maddr = obj_value;
		else if (!strcmp(obj_name, "LOAD_ADDR_H"))
			img_hdr->info.maddr_extend = obj_value;
		else if (!strcmp(obj_name, "LOAD_MODE"))
			img_hdr->info.mode = obj_value;
		else if (!strcmp(obj_name, "NAME"))
			strncpy(img_hdr->info.name, obj_value_str, IMG_NAME_SIZE);
		else if (!strcmp(obj_name, "IMG_TYPE"))
			img_hdr->info.img_type = obj_value;
		else if (!strcmp(obj_name, "IMG_LIST_END"))
			img_hdr->info.img_list_end = obj_value;
		else if (!strcmp(obj_name, "ALIGN_SIZE"))
			img_hdr->info.align_size = obj_value;
		else {
#ifdef DEBUG_MODE
			fprintf(stderr, "==> unknown object\n");
#endif
		}
	}

	fclose(fp);
_end:
	return ret;
}

int main(int argc, char *argv[])
{
	IMG_HDR_T img_hdr;
	char *img = NULL;
	char *img_padding = NULL;
	uint32_t img_padding_size = 0;
	int ret = 0;

	if (argc != 3) {
		fprintf(stderr, "Usage: ./mkimage <img_path> <cfg_path> > out_image\n");
		return 0;
	}

	memset(&img_hdr, 0xff, sizeof(IMG_HDR_T));

	/* legacy fields */
	img_hdr.info.magic = IMG_MAGIC;
	img_hdr.info.dsize = (unsigned int)(filesize(argv[IMG_PATH_IDX]) & 0xffffffff);
	memset(img_hdr.info.name, 0x0, sizeof(img_hdr.info.name));
	img_hdr.info.maddr = 0xffffffff;
	img_hdr.info.mode = 0xffffffff;

	/* extension fields */
	img_hdr.info.ext_magic = EXT_MAGIC;
	img_hdr.info.hdr_size = IMG_HDR_SIZE;
	img_hdr.info.hdr_version = HDR_VERSION;
	img_hdr.info.img_type = IMG_TYPE_IMG_AP_BIN;
	img_hdr.info.img_list_end = 0;
	img_hdr.info.align_size = 16;
	img_hdr.info.dsize_extend = 0;
	img_hdr.info.maddr_extend = 0;

	/* if external config exists, use it to override */
	/* add code here */
	if (argc > IMG_CFG_IDX)
		ret = get_img_hdr_setting_from_cfg(argv[IMG_CFG_IDX], &img_hdr);
	if (ret)
		goto _error;

#ifdef DEBUG_MODE
	{
		int i = 0;
		for (i = 0; i < 512; i++) {
			fprintf(stderr, "%02x ", img_hdr.data[i]);
			if ((i + 1) % 16 == 0)
				fprintf(stderr, "\n");
		}
	}
#endif

	/* current implementation will encounter malloc fail issue if image size is extremely large */
	img = readfile(argv[1], img_hdr.info.dsize);
	img_padding_size = ((img_hdr.info.dsize + (img_hdr.info.align_size - 1)) /
			    img_hdr.info.align_size) * img_hdr.info.align_size - img_hdr.info.dsize;
#ifdef DEBUG_MODE
	fprintf(stderr, "img_padding_size = 0x%x\n", img_padding_size);
#endif
	img_padding = malloc(img_padding_size);
	if (img_padding)
		memset(img_padding, 0x0, img_padding_size);

	/* for linux version mkimage, we only support this method */
	write(STDOUT_FILENO, &img_hdr, sizeof(IMG_HDR_T));
	write(STDOUT_FILENO, img, img_hdr.info.dsize);
	write(STDOUT_FILENO, img_padding, img_padding_size);

	return 0;
_error:
	free(img);
	free(img_padding);
	exit(1);
}
