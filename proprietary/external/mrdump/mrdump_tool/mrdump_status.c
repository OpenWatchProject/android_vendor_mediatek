/* open */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* read */
#include <unistd.h>

/* free */
#include <stdlib.h>

/* ioctl */
#include <sys/ioctl.h>

/* lseek64 */
#define _LARGEFILE64_SOURCE
#include <sys/types.h>
#include <unistd.h>

/* bzero */
#include <strings.h>

/* strlen, memset, strncpy, strtok... */
#include <string.h>

/* errno */
#include <errno.h>

/* BLKGETSIZE64 */
#include <linux/fs.h>

/* mrdump related */
#include "mrdump_log.h"
#include "mrdump_common.h"
#include "mrdump_status.h"
#include "mrdump_status_private.h"

static int file_read_string(const char* path, char *content, int len)
{
    if (len <= 0) {
        return -2;
    }

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        return -1;
    }

    /* Preserved NULL byte */
    len--;
    int size = 0;
    do {
        int ret = read(fd, content + size, len - size);
        if (ret <= 0)
            break;
        size = size + ret;
    } while (size < len);
    content[size] = 0;

    close(fd);
    return size;
}

static int expdb_open(struct partinfo *partinfo)
{
    uint64_t part_size;
    uint32_t part_blksize;

    memset(partinfo, 0, sizeof(struct partinfo));

    char *pp = mrdump_get_device_node(MRDUMP_EXPDB_NAME);
    if (pp == NULL) {
        MD_LOGE("%s: No expdb partition found", __func__);
        return -1;
    }

    int fd = open(pp, O_RDWR);
    if (fd < 0) {
        MD_LOGE("%s: open expdb failed(%d)", __func__, errno);
        free(pp);
        return -1;
    }
    free(pp);

    if (ioctl(fd, BLKGETSIZE64, &part_size) < 0) {
        MD_LOGE("%s, get expdb partition size fail(%d)", __func__, errno);
        close(fd);
        return -1;
    }

    if (ioctl(fd, BLKSSZGET, &part_blksize) < 0) {
        MD_LOGE("%s, get sector size fail(%d)", __func__, errno);
        close(fd);
        return -1;
    }

    partinfo->fd = fd;
    partinfo->size = part_size;
    partinfo->blksize = part_blksize;

    return 0;
}

int mrdump_is_supported(void)
{
    char kversion[16], lversion[16];
    int klen, llen;

    bzero(kversion, sizeof(kversion));
    bzero(lversion, sizeof(lversion));

    if (file_read_string(MRDUMP_KVER, kversion, sizeof(kversion)) < 0) {
        MD_LOGE("%s: cannot get kernel version\n", __func__);
        return 0;
    }

    klen = strlen(kversion);
    if (klen == 0) {
        MD_LOGE("%s: null kernel version\n", __func__);
        return 0;
    }

    if (file_read_string(MRDUMP_LVER, lversion, sizeof(lversion)) < 0) {
        MD_LOGE("%s: cannot get lk version\n", __func__);
        return 0;
    }

    llen = strlen(lversion);
    if (llen == 0) {
        MD_LOGE("%s: null lk version\n", __func__);
        return 0;
    }

    if ((klen != llen) || (strncmp(kversion, lversion, klen) != 0)) {
        MD_LOGE("%s: kernel and lk version mismatched.\n", __func__);
        return 0;
    }

    MD_LOGI("%s: true\n", __func__);
    return 1;
}

bool mrdump_status_clear(void)
{
    struct partinfo partinfo;

    if (expdb_open(&partinfo) >= 0) {
	if (lseek64(partinfo.fd, partinfo.size - MRDUMP_OFFSET, SEEK_SET) < 0) {
	    MD_LOGE("%s: Can't seek part fd %d\n", __func__, partinfo.fd);
	    close(partinfo.fd);
	    return false;
	}

	struct mrdump_cblock_result cblock_result;
	if (read(partinfo.fd, &cblock_result, sizeof(struct mrdump_cblock_result)) != sizeof(struct mrdump_cblock_result)) {
	    MD_LOGE("%s: Can't read part fd %d\n", __func__, partinfo.fd);
	    close(partinfo.fd);
	    return false;
	}
	memset(cblock_result.status, 0, sizeof(cblock_result.status));
	strncpy(cblock_result.status, "CLEAR", 5);

	if (lseek64(partinfo.fd, partinfo.size - MRDUMP_OFFSET, SEEK_SET) < 0) {
	    MD_LOGE("%s: Can't seek part fd %d\n", __func__, partinfo.fd);
	    close(partinfo.fd);
	    return false;
	}
	if (write(partinfo.fd, &cblock_result, sizeof(struct mrdump_cblock_result)) != sizeof(struct mrdump_cblock_result)) {
	    MD_LOGE("%s: Can't write part fd %d\n", __func__, partinfo.fd);
	    close(partinfo.fd);
	    return false;
	}
	close(partinfo.fd);
	return true;
    }
    return false;
}

bool mrdump_status_get(struct mrdump_status_result *result)
{
    memset(result, 0, sizeof(struct mrdump_status_result));
    result->struct_size = sizeof(struct mrdump_status_result);

    struct partinfo partinfo;
    if (expdb_open(&partinfo) >= 0) {
	if (lseek64(partinfo.fd, partinfo.size - MRDUMP_OFFSET, SEEK_SET) < 0) {
	    MD_LOGE("%s: Can't seek part fd %d\n", __func__, partinfo.fd);
	    close(partinfo.fd);
	    return false;
	}

	struct mrdump_cblock_result cblock_result;
	if (read(partinfo.fd, &cblock_result, sizeof(struct mrdump_cblock_result)) != sizeof(struct mrdump_cblock_result)) {
	    MD_LOGE("%s: Can't read part fd %d\n", __func__, partinfo.fd);
	    close(partinfo.fd);
	    return false;
	}
	close(partinfo.fd);

	if (strcmp(cblock_result.sig, MRDUMP_SIG) != 0) {
	    MD_LOGE("%s: Signature mismatched\n", __func__);
	    return false;
	}
	/* Copy/parsing status line */
	strncpy(result->status_line, cblock_result.status, sizeof(cblock_result.status));
	result->status_line[sizeof(result->status_line) - 1] = 0;

	char *saveptr;
	cblock_result.status[sizeof(cblock_result.status) - 1] = 0;
	char *strval = strtok_r(cblock_result.status, "\n", &saveptr);
	if (strval != NULL) {
	    if (strcmp(strval, "OK") == 0) {
		result->status = MRDUMP_STATUS_OK;
		result->output = MRDUMP_OUTPUT_NULL;

		do {
		    strval = strtok_r(NULL, "\n", &saveptr);
		    if (strval != NULL) {
                        if (strncmp(strval, "OUTPUT:", 7) == 0) {
			    if (strcmp(strval + 7, "DATA_FS") == 0) {
				result->output = MRDUMP_OUTPUT_DATA_FS;
			    }
			    else if (strcmp(strval + 7, "PARTITION_DATA") == 0) {
				result->output = MRDUMP_OUTPUT_PARTITION;
			    }
			    else {
				return false;
			    }
			}
			else if (strncmp(strval, "MODE:", 5) == 0) {
			    strlcpy(result->mode, strval + 5, sizeof(result->mode));
			}
		    }
		} while (strval != NULL);
	    }
	    else if (strcmp(strval, "NONE") == 0) {
		result->status = MRDUMP_STATUS_NONE;
	    }
	    else if (strcmp(strval, "CLEAR") == 0) {
		result->status = MRDUMP_STATUS_NONE;
	    }
	    else {
		result->status = MRDUMP_STATUS_FAILED;
	    }
	}
	else {
	    MD_LOGE("%s: status parsing error \"%s\"\n", __func__, cblock_result.status);
	    return false;
	}

	strncpy(result->log_buf, cblock_result.log_buf, sizeof(cblock_result.log_buf));
	result->log_buf[sizeof(result->log_buf) - 1] = 0;
	return true;
    }
    return false;
}
