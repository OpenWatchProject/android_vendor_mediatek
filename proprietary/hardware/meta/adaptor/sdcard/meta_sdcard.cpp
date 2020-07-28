#include "meta_sdcard.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <dirent.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <log/log.h>


#define FREEIF(p)	do { if(p) free(p); (p) = NULL; } while(0)
#define CHKERR(x)	do { if((x) < 0) goto error; } while(0)

#define UNUSED(x) (void)(x)

static char *path_data = NULL;
static SDCARD_CNF_CB cnf_cb = NULL;

static int meta_sdcard_read_info(const char *filename, void *buf, ssize_t bufsz)
{
	int fd, rsize;
	if ((fd = open(filename, O_RDONLY)) < 0) {
		META_SDCARD_LOG("Open %s failed errno(%s)",filename,(char*)strerror(errno));
		return -1;
	}

	rsize = read(fd, buf, bufsz);
	if(rsize < 0)
		META_SDCARD_LOG("read %s failed errno(%s)",filename,(char*)strerror(errno));
	close(fd);

	return rsize;
}

static void meta_sdcard_send_resp(SDCARD_CNF *cnf)
{
	if (cnf_cb)
		cnf_cb(cnf);
	else
		WriteDataToPC(cnf, sizeof(SDCARD_CNF), NULL, 0);
}

void Meta_SDcard_Register(SDCARD_CNF_CB callback)
{
	cnf_cb = callback;
}

/* for boot type usage */
#define BOOTDEV_NAND            (0)
#define BOOTDEV_SDMMC           (1)
#define BOOTDEV_UFS             (2)
static int get_boot_type(void) {
	int fd;
	size_t s;
	char boot_type[4] = {'0'};

	fd = open("/sys/class/BOOT/BOOT/boot/boot_type", O_RDONLY);
	if (fd < 0) {
		META_SDCARD_LOG("fail to open: %s\n", "/sys/class/BOOT/BOOT/boot/boot_type");
		return -1;
	}

	s = read(fd, (void *)&boot_type, sizeof(boot_type) - 1);
	close(fd);

	if (s <= 0) {
		META_SDCARD_LOG("could not read boot type sys file\n");
		return -1;
	}

	boot_type[s] = '\0';

	return atoi((char *)&boot_type);
}

bool Meta_SDcard_Init(SDCARD_REQ *req)
{
	int id = (int)req->dwSDHCIndex;
	char name[20];
	char *ptr;
	int sd_id, mt_boot_type;

	META_SDCARD_LOG("meta_sdcard_init, ID(%d)\n", id);

	mt_boot_type = get_boot_type();
	if (mt_boot_type == BOOTDEV_SDMMC)
		sd_id = id;     /* eMMC booting, sd host is mmc1 */
	else
		sd_id = id - 1; /* No eMMC, sd host is mmc0 */

	if ((sd_id < 0) || (id > (MAX_NUM_SDCARDS - 1))){
		META_SDCARD_LOG("ID error(%d)\n", id);
		return false;
	}

	/* init path_data of /dev/block/mmcblkx */
	if (!path_data && NULL == (path_data = (char *) malloc(512))) {
		META_SDCARD_LOG("No memory\n");
		return false;
	}

	sprintf(name, "mmcblk%d", sd_id);

	ptr = path_data;
	ptr += sprintf(ptr, "/dev/block/%s", name);

	META_SDCARD_LOG("[META_SD] path_data: %s/\n", path_data);

	sprintf(name, "mmc%d", sd_id);

	return true;
}

bool Meta_SDcard_Deinit(void)
{
	FREEIF(path_data);
	return true;
}


void Meta_SDcard_OP(SDCARD_REQ *req, char *peer_buf, unsigned short peer_len)
{
	SDCARD_CNF cnf;
	int bufsz = 512;
	char fname[512];
	char buf[512];
	unsigned char csd[16];

	UNUSED(peer_buf);
	UNUSED(peer_len);

	memset(&cnf, 0, sizeof(SDCARD_CNF));

	cnf.header.id = FT_SDCARD_CNF_ID;
	cnf.header.token = req->header.token;
	cnf.status = META_SUCCESS;

	META_SDCARD_LOG("[META_SD] read %s \n", path_data);
	sprintf(fname, "%s", path_data);
	CHKERR(meta_sdcard_read_info(fname, buf, bufsz));
	memcpy(csd,buf,16*sizeof(unsigned char));

	META_SDCARD_LOG("[META_SD] send resp, pass\n");
	meta_sdcard_send_resp(&cnf);
	return;

error:
	META_SDCARD_LOG("[META_SD] send resp, failed\n");
	cnf.status = META_FAILED;
	meta_sdcard_send_resp(&cnf);
	return;
}

