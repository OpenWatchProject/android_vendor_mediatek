/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   ccci_lib.c
 *
 * Project:
 * --------
 *
 *
 * Description:
 * ------------
 *
 *
 * Author:
 * -------
 *
 *
 ****************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
//#include <cutils/properties.h>
//#include <android/log.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdint.h>
#include <pthread.h>
#include "hardware/ccci_intf.h"
#include "ccci_lib_platform.h"
#include "ccci_lib.h"

/*=================  debug option setting ============================*/
/* #define CCB_POLL_LOG_EN */

//----------------debug log define-----------------//
#define LOGV(...) do{ __android_log_print(ANDROID_LOG_VERBOSE, "ccci_lib", __VA_ARGS__); }while(0)

#define LOGD(...) do{ __android_log_print(ANDROID_LOG_DEBUG, "ccci_lib", __VA_ARGS__); }while(0)

#define LOGI(...) do{ __android_log_print(ANDROID_LOG_INFO, "ccci_lib", __VA_ARGS__); }while(0)

#define LOGW(...) do{ __android_log_print(ANDROID_LOG_WARN, "ccci_lib", __VA_ARGS__); }while(0)

#define LOGE(...) do{ __android_log_print(ANDROID_LOG_ERROR, "ccci_lib", __VA_ARGS__); }while(0)
//#include <fs_mgr.h>
#define IMG_MAGIC		0x58881688
#define EXT_MAGIC		0x58891689

#define IMG_NAME_SIZE		32
#define IMG_HDR_SIZE		512
#define SEC_IMG_AUTH_SIZE	0x100
#define SEC_IMG_AUTH_ALIGN	0x1000

#define W_BUF_SIZE		(1024*4)

typedef union {
	struct {
		unsigned int magic;	/* always IMG_MAGIC */
		unsigned int dsize;	/* image size, image header and padding are not included */
		char name[IMG_NAME_SIZE];
		unsigned int maddr;	/* image load address in RAM */
		unsigned int mode;	/* maddr is counted from the beginning or end of RAM */
		/* extension */
		unsigned int ext_magic;	/* always EXT_MAGIC */
		unsigned int hdr_size;	/* header size is 512 bytes currently, but may extend in the future */
		unsigned int hdr_version; /* see HDR_VERSION */
		unsigned int img_type;	/* please refer to #define beginning with SEC_IMG_TYPE_ */
		unsigned int img_list_end; /* end of image list? 0: this image is followed by another image 1: end */
		unsigned int align_size; /* image size alignment setting in bytes, 16 by default for AES encryption */
		unsigned int dsize_extend; /* high word of image size for 64 bit address support */
		unsigned int maddr_extend; /* high word of image load address in RAM for 64 bit address support */
	} info;
	unsigned char data[IMG_HDR_SIZE];
} prt_img_hdr_t;


#define PARTITION_PATH_LEN	128

static const char *mdimg_node[2] = {
	"/dev/block/platform/bootdevice/by-name/md1img",
	"/dev/block/platform/bootdevice/by-name/md3img"
};

/*return 0 if image is found, and return negative value if not*/
int find_image_from_pt_internal(int which_md_fd, char *img_name, int *offset, int *img_size)
{
	prt_img_hdr_t *p_hdr = NULL;
	int ret, curr_img_size, load_size, sec_padding_size = 0;
	int fd = which_md_fd;

	p_hdr = (prt_img_hdr_t *)malloc(sizeof(prt_img_hdr_t));
	if (p_hdr == NULL) {
		LOGE("alloc memory for hdr fail\n");
		ret = -4;
		goto find_exit;
	}
	memset(p_hdr, 0, sizeof(prt_img_hdr_t));
	*offset = 0;

	do {
		load_size = read(fd, (char *)p_hdr, sizeof(prt_img_hdr_t));
		if (load_size != sizeof(prt_img_hdr_t)) {
			LOGE("load hdr fail(%d)\n", load_size);
			ret = -5;
			goto find_exit;
		}
		if (p_hdr->info.magic!=IMG_MAGIC) {
			*offset += sec_padding_size;
			lseek(fd, (off_t)(*offset), SEEK_SET);
			load_size = read(fd, (char *)p_hdr, sizeof(prt_img_hdr_t));
			if (load_size != sizeof(prt_img_hdr_t)) {
				LOGE("load hdr fail again(%d)\n", load_size);
				ret = -5;
				goto find_exit;
			}
			if (p_hdr->info.magic!=IMG_MAGIC) {
				LOGE("invalid magic(%x) at 0x%x, ref(%x)\n", p_hdr->info.magic, *offset, IMG_MAGIC);
				ret = -6;
				goto find_exit;
			}
		}

		if (strcmp(img_name, p_hdr->info.name) == 0) {
			*offset += sizeof(prt_img_hdr_t);
			*img_size = p_hdr->info.dsize;
			LOGI("find image %s, offset 0x%x, size 0x%x\n", img_name, *offset, *img_size);
			ret = 0;
			break;
		}
		if (p_hdr->info.img_list_end == 0) {
			if (p_hdr->info.align_size != 0)
				curr_img_size = (p_hdr->info.dsize + p_hdr->info.align_size - 1) &
					(~(p_hdr->info.align_size -1));
			else {
				curr_img_size = p_hdr->info.dsize;
				LOGI("image %s align size is 0!\n", img_name);
			}
			if (p_hdr->info.hdr_size != 0)
				curr_img_size = p_hdr->info.hdr_size + curr_img_size;
			else
				curr_img_size = IMG_HDR_SIZE + curr_img_size;
			*offset += curr_img_size;
			sec_padding_size = ((curr_img_size + SEC_IMG_AUTH_ALIGN - 1) & (~(SEC_IMG_AUTH_ALIGN -1)))
				- curr_img_size + SEC_IMG_AUTH_SIZE;
			LOGI("next image offset is 0x%x, sec_padding 0x%x\n", *offset, sec_padding_size);

			ret = lseek(fd, (off_t)(*offset), SEEK_SET);
			if (ret < 0) {
				LOGE("lseek err! offset 0x%x, errno %d", *offset, errno);
				break;
			}
		} else {
			ret = -7;
			break;
		}
	} while(1);

find_exit:
	if (p_hdr) {
		free(p_hdr);
		p_hdr = NULL;
	}

	return ret;
}

/*return which md if image is found,0:md1,1:md3, and return negative value if not*/
int find_image_from_pt(char *img_name, int *offset, int *img_size)
{
	int fd = -1, ret = -1, i = 0;
	char partition_path[PARTITION_PATH_LEN];
	char buf[PROPERTY_VALUE_MAX] ={ 0 };

        AB_image_get(buf);

	for(i = 0; i < 2; i++) {
		snprintf(partition_path, PARTITION_PATH_LEN, "%s%s", mdimg_node[i], buf);
		fd = open(partition_path, O_RDONLY);
		if (fd < 0) {
			LOGE("open md1img node %s fail! errno = %d\n", partition_path, errno);
			continue;
		}
		ret = find_image_from_pt_internal(fd, img_name, offset, img_size);
		close(fd);
		if(ret != 0) {
			LOGE("not found img:%s in the %s image\n", img_name, partition_path);
		}else {
			ret = i;
			break;
		}
	}
	return ret;
}

static int read_image_from_pt_internal(int which_md_fd, char *img_name, char *buf, int offset, int read_len)
{
	int fd = which_md_fd;
	int ret = -1;

	ret = lseek(fd, offset, SEEK_SET);
	if (ret < 0) {
		LOGE("lseek mdimg fail! offset = %d, errno = %d\n", offset, errno);
		return ret;
	}
	ret = read(fd, buf, read_len);
	if (ret < 0)
		LOGE("read img %s return %d, errno = %d\n", img_name, ret, errno);

	return ret;
}

/*return restored image size on success, and return negative value if not*/
int restore_image_from_pt(char *img_name, char *restore_path)
{
	int ret = 0, fd, r_len, rr_len, offset, img_size = 0;
	int w_count, w_len = 0, which_md = -1, which_md_fd = -1;
	char buff[W_BUF_SIZE];
	char partition_path[PARTITION_PATH_LEN];
	char buf[PROPERTY_VALUE_MAX] ={ 0 };

	if (img_name == NULL || restore_path == NULL) {
		LOGE("invalid arg for restore_image_from_pt\n");
		return -1;
	}

	which_md = find_image_from_pt(img_name, &offset, &img_size);
	if (which_md < 0) {
		LOGE("not find %s in partition\n", img_name);
		return -1;
	}
	if ((img_size > 512*1024*1024) || (img_size < 0)) {
		LOGE("MD image size abnormal %d(>512MB or <0MB)\n", img_size);
		return -1;
	}
	LOGI("img %s (size 0x%x) is at 0x%x in partition\n", img_name, img_size, offset);

	fd = creat(restore_path, 0644);
	if (fd < 0) {
		LOGE("create %s fail, errno %d\n", restore_path, errno);
		return -2;
	}

        AB_image_get(buf);

	snprintf(partition_path, PARTITION_PATH_LEN, "%s%s", mdimg_node[which_md], buf);
	which_md_fd = open(partition_path, O_RDONLY);
	if (which_md_fd < 0) {
		LOGE("open md1img node %s fail! errno = %d\n", partition_path, errno);
		return -2;
	}
	LOGI("md %s (md id:%d)\n", partition_path, which_md);

	for (w_count = 0; w_count < img_size;) {
		if (img_size - w_count > W_BUF_SIZE)
			r_len = W_BUF_SIZE;
		else
			r_len = img_size - w_count;
		rr_len = read_image_from_pt_internal(which_md_fd, img_name, buff, offset + w_count, r_len);
		if (rr_len > 0) {
			w_len = write(fd, buff, rr_len);
			if (w_len > 0) {
				w_count += w_len;
				ret = w_count;
			} else {
				ret = w_len;
				LOGE("write image %s to %s ret %d, errno %d\n", img_name, restore_path, ret, errno);
				break;
			}
		} else if (rr_len == 0) {
			LOGW("read image %s done, w_count = %d\n", img_name, w_count);
			break;
		} else {
			ret = rr_len;
			LOGE("read image %s ret %d, errno %d\n", img_name, ret, errno);
			break;
		}
	}
	LOGI("restore %s to %s done, return %d\n", img_name, restore_path, ret);

	close(fd);
	close(which_md_fd);
	return ret;
}

static int parse_info(char raw_data[], int raw_size, char name[], char val[], int size)
{
    int i, j=0;
    char tmp_name[32];
    char tmp_val[32];
    int state = 0;

    LOGD("parse_info name:%s", name);

    for(i=0; i<raw_size; i++) {
        switch(state) {
        case 0: // Init
            if(raw_data[i] == '[') {
                j=0;
                state = 1;
            }
            break;

        case 1: // Read name
            if(raw_data[i] == ']') {
                state =2; // name parse done
                tmp_name[j] = '\0';
                j = 0;
            } else if((raw_data[i] == '\r')||(raw_data[i] == '\n')) {
                j = 0;
                state = 0;
            } else {
                tmp_name[j] = raw_data[i];
                j++;
            }
            break;

        case 2: // Get ':'
            if(raw_data[i] == ':') {
                state = 3; //divider found
                tmp_val[0] = 0;
            } else
                state = 0; // Invalid format
            break;

        case 3: // Read value
            if((raw_data[i] == '\r')||(raw_data[i] == '\n')) {
                state =4; // value parse done
                tmp_val[j]='\0';
                j = 0;
            } else {
                tmp_val[j] = raw_data[i];
                j++;
                break;
            }

        case 4: // Check if name is match
            if(strcmp(tmp_name, name)==0) {
                // Copy value
                snprintf(val, size, "%s", tmp_val);
                return 0;
            }
            state = 0;
            break;
        default:
            break;
        }
    }
    return -1;
}

int query_kcfg_setting(char name[], char val[], int size)
{
    char *raw_buf;
    int fd;
    int ret;

    fd = open("/sys/kernel/ccci/kcfg_setting", O_RDONLY);
    if(fd < 0) {
        LOGE("open sys file fail(%d)", errno);
        return -1;
    }
    raw_buf = (char*)malloc(4096);
    if(NULL == raw_buf) {
        LOGE("allock memory fail");
        close(fd);
        return -2;
    }
    ret = read(fd, raw_buf, 4096 - 1);
    if(ret <= 0) {
        LOGE("read info fail ret%d(%d)", ret, errno);
        ret = -3;
        goto _Exit;
    }
    raw_buf[ret] = '\0';
    LOGD("read info:%s", raw_buf);

    //------------------------------------------------
    ret = parse_info(raw_buf, ret, name, val, size);
    //------------------------------------------------

_Exit:
    free(raw_buf);
    close(fd);

    return ret;
}

static struct ccci_ccb_config * ccci_ccb_ports;
unsigned int ccb_info_len;

static int ccci_ccb_ctrl_get(struct ccb_ctrl_info *ctrl_info, unsigned char **address)
{
    char dev_port[32];
    unsigned int i;
    int fd, ret;

    snprintf(dev_port, 32, "%s", "/dev/ccci_ccb_ctrl");
    fd = open(dev_port, O_RDWR);
    if (fd < 0) {
        LOGE("open %s failed, errno=%d, user%d", dev_port, errno, ctrl_info->user_id);
        return -ENODEV;
    }

    ret = ioctl(fd, CCCI_IOC_GET_CCB_CONFIG_LENGTH, &ccb_info_len);
    if (ret || ccb_info_len <= 0) {
        LOGE("CCCI_IOC_GET_CCB_CONFIG_LENGTH fail on %s, err=%d, len = %d\n", dev_port, errno, ccb_info_len);
        close(fd);
        fd = -1;
        return ret;
    } else {
        LOGD("ccb_info_len=%d\n", ccb_info_len);
    }
    ccci_ccb_ports = (struct ccci_ccb_config*) malloc(sizeof(struct ccci_ccb_config) * ccb_info_len);

    for (i = 0; i < ccb_info_len; i++) {
        /* use user_id as input param, it will be overwrite by return value. */
        ccci_ccb_ports[i].user_id = i;
        ioctl(fd, CCCI_IOC_GET_CCB_CONFIG, &ccci_ccb_ports[i]);
    }

    /* dump memory layout got from kernel */
#if DEBUG_MSG_ON
    LOGD("dump memory layout got from kernel:\n");
    for (i=0; i<ccb_info_len; i++)
        LOGD("%d, %d, %d, %d, %d, %d\n", ccci_ccb_ports[i].user_id,
                ccci_ccb_ports[i].core_id,
                ccci_ccb_ports[i].dl_page_size,
                ccci_ccb_ports[i].ul_page_size,
                ccci_ccb_ports[i].dl_buff_size,
                ccci_ccb_ports[i].ul_buff_size);
#endif

    ret = ioctl(fd, CCCI_IOC_CCB_CTRL_INFO, ctrl_info);
    if (ret) {
        LOGE("CCCI_IOC_CCB_CTRL_INFO fail on %s, err=%d\n", dev_port, errno);
        close(fd);
        fd = -1;
        return ret;
    }

    LOGD("new ccb_ctrl mmap on %s(%d) for addr=0x%x, len=%d\n", dev_port, fd, ctrl_info->ctrl_addr, ctrl_info->ctrl_length);
    *address = (void *)mmap(NULL, ctrl_info->ctrl_length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (*address == MAP_FAILED) {
        if (ctrl_info->ctrl_length > 0)
            LOGE("mmap on %s failed, %d\n", dev_port, errno);
        close(fd);
        fd = -1;
        return -EFAULT;
    }

    return fd;
}

/************ for users who control share memory by themslves **********/
int ccci_smem_get(CCCI_MD md_id, CCCI_USER user_id, unsigned char **address, unsigned int *length)
{
    char dev_port[32];
    int fd, ret;
    unsigned int addr = 0, len = 0;

    snprintf(dev_port, 32, "%s", ccci_get_node_name(user_id, md_id));
    fd = open(dev_port, O_RDWR);
    if (fd < 0) {
        LOGE("open %s failed, errno=%d, user%d, md%d", dev_port, errno, user_id, md_id);
        return -ENODEV;
    }

    ret = ioctl(fd, CCCI_IOC_SMEM_BASE, &addr);
    if (ret) {
        LOGE("CCCI_IOC_SMEM_BASE fail on %s, err=%d\n", dev_port, errno);
        close(fd);
        fd = -1;
        return ret;
    }

    ret = ioctl(fd, CCCI_IOC_SMEM_LEN, &len);
    if (ret) {
        LOGE("CCCI_IOC_SMEM_LEN fail on %s, err=%d\n", dev_port, errno);
        close(fd);
        fd = -1;
        return ret;
    }
    LOGD("mmap on %s(%d) for addr=0x%x, len=%d\n", dev_port, fd, addr, len);
    *address = (void *)mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    *length = len;
    if (*address == MAP_FAILED) {
        LOGE("mmap on %s failed, %d\n", dev_port, errno);
        close(fd);
        fd = -1;
        return -EFAULT;
    }
    return fd;
}

int ccci_ccb_ctrl_put(int fd, unsigned char *address, unsigned int length)
{
    if (fd < 0)
        return -EINVAL;
    close(fd);
    LOGD("munmap on (%d) for addr=%p, len=%d\n", fd, address, length);
    return munmap(address, length);
}

int ccci_smem_put(int fd, unsigned char *address, unsigned int length)
{
    if (fd < 0)
        return -EINVAL;
    close(fd);
    LOGD("munmap on (%d) for addr=%p, len=%d\n", fd, address, length);
    return munmap(address, length);
}

/************ for md_init to check share memory **********/


int ccci_ccb_get_config(CCCI_MD md_id, CCCI_USER user_id, unsigned int buffer_id, struct ccci_ccb_size *config)
{
    unsigned int i, count;

    if (md_id != MD_SYS1 || !config)
        return -EINVAL;
    count = 0;
    for (i = 0; i < ccb_info_len; i++) {
        if (ccci_ccb_ports[i].user_id != user_id - CCB_START_INDEX)
            continue;
        if (count++ == buffer_id) {
            config->ccb_page_header_size = 8; // FIXME, hardcode
            config->dl_page_size = ccci_ccb_ports[i].dl_page_size;
            config->ul_page_size = ccci_ccb_ports[i].ul_page_size;
            config->dl_buff_size = ccci_ccb_ports[i].dl_buff_size;
            config->ul_buff_size = ccci_ccb_ports[i].ul_buff_size;
            return 0;
        }
    }
    LOGE("get_config failed, md_id=%d, user_id=%d, buffer_id=%d, ccb_info_len=%d, count=%d\n", md_id, user_id, buffer_id,
            ccb_info_len, count);
    return -EFAULT;
}

static int ccci_ccb_init_user(CCCI_MD md_id, CCCI_USER user_id)
{
    int ctrl_fd = -1;
    unsigned char *ctrl_base_address = NULL, *ptr;
    unsigned int i, data;
    unsigned int ret = 0;
    struct buffer_header *header_ptr;
    struct ccb_ctrl_info ctrl_info;

    if (md_id != MD_SYS1)
        return -EINVAL;

    ctrl_info.user_id = user_id - CCB_START_INDEX;

    /* we must get both ctrl_base_address, and user's ctrl_offset, because all users share the same ctrl buffer.*/
    ctrl_fd = ccci_ccb_ctrl_get(&ctrl_info, &ctrl_base_address);
    if (ctrl_fd < 0) {
        LOGE("ctrl_fd error. fd=%d\n", ctrl_fd);
        ret = -ENODEV;
        goto exit;
    }

    ptr = ctrl_base_address + ctrl_info.ctrl_offset * sizeof(struct buffer_header) * 2;
    for (i = 0; i < ccb_info_len; i++) {
        if (ccci_ccb_ports[i].user_id != user_id - CCB_START_INDEX)
            continue;
        LOGD("user %d DL, slot %d, address=%p (%d, %d)\n", user_id, i, ptr,
                ccci_ccb_ports[i].dl_page_size,
                ccci_ccb_ports[i].dl_buff_size);
        // downlink
        header_ptr = (struct buffer_header *)ptr;
        memset(header_ptr, 0, sizeof(struct buffer_header));
        header_ptr->page_size = ccci_ccb_ports[i].dl_page_size;
        header_ptr->data_buffer_size = ccci_ccb_ports[i].dl_buff_size;
        header_ptr->guard_band_e = TAIL_MAGIC_BEFORE;

        ptr += sizeof(struct buffer_header);

        // uplink
        LOGD("user %d UL, slot %d, address=%p (%d, %d)\n", user_id, i, ptr,
                ccci_ccb_ports[i].ul_page_size,
                ccci_ccb_ports[i].ul_buff_size);
        header_ptr = (struct buffer_header *)ptr;
        memset(header_ptr, 0, sizeof(struct buffer_header));
        header_ptr->page_size = ccci_ccb_ports[i].ul_page_size;
        header_ptr->data_buffer_size = ccci_ccb_ports[i].ul_buff_size;
        header_ptr->guard_band_e = TAIL_MAGIC_BEFORE;

        ptr += sizeof(struct buffer_header);
    }

exit:
    data = ret ? CCB_USER_ERR : CCB_USER_OK;
    /*smem state has been set in ccci_ccb_ctrl_get()*/
    /*ioctl(fd, CCCI_IOC_SMEM_SET_STATE, &data);*/

    ccci_ccb_ctrl_put(ctrl_fd, ctrl_base_address, ctrl_info.ctrl_length);
    LOGD("init user%d md%d ret=%d", user_id, md_id, ret);
    return ret;
}

int ccci_ccb_init_users(int md_id)
{
    ccci_ccb_init_user(md_id, USR_SMEM_CCB_DHL);
    ccci_ccb_init_user(md_id, USR_SMEM_CCB_MD_MONITOR);
    ccci_ccb_init_user(md_id, USR_SMEM_CCB_META);
    return 0;
}

int ccci_ccb_check_users(__attribute__((unused))int md_id)
{
    // no need now, as AP configured share memory layout
    return 0;
}

static struct ccci_ccb_config *ccci_ccb_query_config(unsigned int user_id, unsigned int buffer_id)
{
    unsigned int i;
    unsigned int buffer_count = 0;

    for (i = 0; i < ccb_info_len; i++) {
        if (ccci_ccb_ports[i].user_id == user_id - CCB_START_INDEX) {
            if (buffer_count == buffer_id)
                return &ccci_ccb_ports[i];
            buffer_count++;
        }
    }
    return NULL;
}

/************ for users who use share memory through CCB API **********/
static struct ccci_ccb_control_user *user;

int ccci_ccb_get_fd()
{
    if (user != NULL)
        return user->fd;
    return -1;
}

/*
 * return value: <0: error; >0: buffer slot number; =0: not possible
 * register/unregiser must be paired
 */
int ccci_ccb_register(CCCI_MD md_id, CCCI_USER user_id)
{
    unsigned int i;
    unsigned char *ptr, *data_buf_ptr;
    unsigned int data = 0xFF;
    struct ccci_ccb_control_buff *buff;

    if (md_id != MD_SYS1)
        return -EINVAL;

    ccci_ccb_unregister();
    if (user) {
        LOGE("user%d md%d already registered\n", user_id, md_id);
        return -EFAULT;
    }
    user = malloc(sizeof(struct ccci_ccb_control_user));
    if (!user) {
        LOGE("alloc user%d md%d struct fail\n", user_id, md_id);
        return -EFAULT;
    }
    memset(user, 0, sizeof(struct ccci_ccb_control_user));
    user->fd = user->ctrl_fd = -1;
    user->fd = ccci_smem_get(md_id, user_id, &user->base_address, &user->total_length);
    if (user->fd < 0) {
        free(user);
        user = NULL;
        return -ENODEV;
    }

    ioctl(user->fd, CCCI_IOC_SMEM_GET_STATE, &data);
    if (data != CCB_USER_OK) {
        ccci_ccb_unregister();
        LOGE("register user %d state wrong %d\n", user_id, data);
        return -EINVAL;
    }

    user->ctrl_info.user_id = user_id - CCB_START_INDEX;
    user->ctrl_fd = ccci_ccb_ctrl_get(&user->ctrl_info, &user->ctrl_base_address);
    if (user->ctrl_fd < 0) {
        ccci_ccb_unregister();
        return -ENODEV;
    }

    LOGD("register user%d md%d: base=%p, len=%d, ctrl_base=%p, ctrl_offset=%d\n", user_id, md_id,
        user->base_address, user->total_length, user->ctrl_base_address, user->ctrl_info.ctrl_offset);

    for (i = 0; i < ccb_info_len; i++) {
        if (ccci_ccb_ports[i].user_id != user_id - CCB_START_INDEX)
            continue;
        user->buffer_num++;
    }
    if (!user->buffer_num) {
        ccci_ccb_unregister();
        LOGE("user %d of MD %d has no smem info\n", user_id, md_id);
        return -EINVAL;
    }
    user->buffers = malloc(sizeof(struct ccci_ccb_control_buff) * user->buffer_num);
    if (!user->buffers) {
        ccci_ccb_unregister();
        LOGE("alloc user%d md%d buffer struct fail\n", user_id, md_id);
        return -EFAULT;
    }
    memset(user->buffers, 0, sizeof(struct ccci_ccb_control_buff) * user->buffer_num);

    user->user_id = user_id;
    ptr = user->ctrl_base_address + user->ctrl_info.ctrl_offset * sizeof(struct buffer_header) * 2;
    data_buf_ptr = user->base_address;

    for (i = 0; i < user->buffer_num; i++) {
        buff = user->buffers + i;
        /*init ul_header at first, it will be checked on unregister*/
        buff->ul_header = NULL;
        buff->dl_header = (struct buffer_header *)ptr;
        ccci_ccb_read_flush(i);
        if (buff->dl_header->read_index != 0)
            LOGD("dl index: read = %d, free = %d, write = %d, alloc = %d\n", buff->dl_header->read_index,
               buff->dl_header->free_index, buff->dl_header->write_index, buff->dl_header->allocate_index);
        buff->dl_header->free_index = buff->dl_header->read_index;
        if (buff->dl_header->guard_band == 0) {
            LOGE("register: DL buffer %d pattern wrong\n", i);
            ccci_ccb_unregister();
            return -EFAULT;
        }
        buff->dl_page_num = buff->dl_header->data_buffer_size / buff->dl_header->page_size;
        buff->dl_header->guard_band_e = TAIL_MAGIC_AFTER;

        ptr += sizeof(struct buffer_header);
        buff->dl_pages =  (struct page_header *)data_buf_ptr;

        LOGD("register user%d md%d DL%d, pattern=%x, dl_page=%p\n", user_id, md_id, i, buff->dl_header->guard_band, buff->dl_pages);

        data_buf_ptr += buff->dl_header->data_buffer_size;

        /* here begins upload buffer */
        buff->ul_header = (struct buffer_header *)ptr;
        buff->ul_header->allocate_index = buff->ul_header->write_index;
        if (buff->ul_header->write_index != 0)
            LOGD("ul index: read = %d, free = %d, write = %d, alloc = %d\n", buff->ul_header->read_index,
                buff->ul_header->free_index, buff->ul_header->write_index, buff->ul_header->allocate_index);
        if (buff->ul_header->guard_band == 0) {
            LOGE("register: UL buffer %d pattern wrong\n", i);
            ccci_ccb_unregister();
            return -EFAULT;
        }
        buff->ul_page_num = buff->ul_header->data_buffer_size / buff->ul_header->page_size;
        buff->ul_header->guard_band_e = TAIL_MAGIC_AFTER;

        ptr += sizeof(struct buffer_header);
        buff->ul_pages =  (struct page_header *)data_buf_ptr;

        LOGD("register user%d md%d UL%d, pattern=%x, ul_page=%p\n", user_id, md_id, i, buff->ul_header->guard_band, buff->ul_pages);

        data_buf_ptr += buff->ul_header->data_buffer_size;
    }

    return user->buffer_num;
}

int ccci_ccb_unregister()
{
    unsigned int i;
    int ret = 0;
    struct ccci_ccb_control_buff *buff;

    if (!user)
        return -EFAULT;
    if (user->buffers) {
        for (i = 0; i < user->buffer_num; i++) {
            buff = user->buffers + i;
            if (buff->dl_header && buff->ul_header) {
                buff->dl_header->guard_band_e = TAIL_MAGIC_BEFORE;
                buff->ul_header->guard_band_e = TAIL_MAGIC_BEFORE;
            } else {
                LOGE("unregister: invalid header DL %p, UL %p\n", buff->dl_header, buff->ul_header);
                break;
            }
        }
    }
    if (user->fd >= 0)
        ret = ccci_smem_put(user->fd, user->base_address, user->total_length);
    if (user->ctrl_fd >= 0)
        ccci_ccb_ctrl_put(user->ctrl_fd, user->ctrl_base_address, user->ctrl_info.ctrl_length);
    if (user->buffers) {
        free(user->buffers);
        user->buffers = NULL;
    }
    free(user);
    user = NULL;
    return ret;
}

/*
 * should be called after registered successfully
 */
int ccci_ccb_query_status()
{
    unsigned int i;
    int ret = CCB_USER_OK;
    struct ccci_ccb_control_buff *buff;

    if (!user)
        return -EFAULT;
    for (i = 0; i < user->buffer_num; i++) {
        buff = user->buffers + i;

        if (buff->dl_header->guard_band != HEADER_MAGIC_AFTER) {
            LOGD("user%d DL %d not ready, %x\n", user->user_id, i, buff->dl_header->guard_band);
            ret = CCB_USER_ERR;
            break;
        }

        if (buff->ul_header->guard_band != HEADER_MAGIC_AFTER) {
            LOGD("user%d UL %d not ready, %x\n", user->user_id, i, buff->ul_header->guard_band);
            ret = CCB_USER_ERR;
            break;
        }
    }

    return ret == CCB_USER_OK ? 0 : -EAGAIN;
}

static inline struct page_header *ccci_ccb_get_page(struct buffer_header *buff_h, struct page_header *pages, unsigned int index)
{
    unsigned char *ptr = (unsigned char *)pages;
    ptr += buff_h->page_size * index;
    return  (struct page_header *)ptr;
}

/*
 * buffer ID is private to each user, and numbered from 0
 * return value: =NULL: error; !=NULL: valid address to write data
 */
unsigned char *ccci_ccb_write_alloc(unsigned int buffer_id)
{
    struct ccci_ccb_control_buff *buff;
    struct page_header *page = NULL;
    unsigned int available, total;

    if (!user)
        return NULL;
    if (buffer_id > user->buffer_num)
        return NULL;
    buff = user->buffers + buffer_id;

    if (buff->ul_header->guard_band != HEADER_MAGIC_AFTER || buff->ul_header->guard_band_e != TAIL_MAGIC_AFTER) {
        LOGE("write_alloc of user%d buffer%d: MD not ready on UL, pattern=0x%0x, pattern_e=0x%x\n",
            user->user_id, buffer_id, buff->ul_header->guard_band, buff->ul_header->guard_band_e);
        return NULL;
    }

    total = buff->ul_page_num;
#if DEBUG_MSG_ON
    LOGD("write_alloc of user%d, total_page_num:%d\n", user->user_id, total);
#endif
    if (buff->ul_header->allocate_index == buff->ul_header->free_index) {
        available = total - 1;
    } else if (buff->ul_header->allocate_index > buff->ul_header->free_index) {
        available = total - buff->ul_header->allocate_index + buff->ul_header->free_index - 1;
    } else {
        available = buff->ul_header->free_index - buff->ul_header->allocate_index - 1;
    }
    if (available) {
        page = ccci_ccb_get_page(buff->ul_header, buff->ul_pages, buff->ul_header->allocate_index);
        page->page_status = PAGE_STATUS_ALLOC;
        if(buff->ul_header->allocate_index + 1 >= total)
            buff->ul_header->allocate_index = 0;
        else
            buff->ul_header->allocate_index++;
    }
#if DEBUG_MSG_ON
    LOGD("write alloc of user%d buffer%d alloc=%d free=%d, available=%u", user->user_id, buffer_id,
        buff->ul_header->allocate_index, buff->ul_header->free_index, available);
#endif
    if (page)
        return page->buffer;
    else
        return NULL;
}

int ccci_ccb_write_done(unsigned int buffer_id, unsigned char *address, unsigned int length)
{
    struct ccci_ccb_control_buff *buff;
    struct page_header *page = NULL;
    struct ccci_ccb_config *config;
    int ret;
    unsigned int data, i;

#if DEBUG_MSG_ON
    unsigned int  retval;
    struct ccci_ccb_debug debug_it;
#endif

    if (!user)
        return -EFAULT;
    if (buffer_id > user->buffer_num)
        return -EINVAL;
    buff = user->buffers + buffer_id;

    if (buff->ul_header->guard_band != HEADER_MAGIC_AFTER || buff->ul_header->guard_band_e != TAIL_MAGIC_AFTER) {
        LOGE("write_done of user%d buffer%d: MD not ready on UL, pattern=0x%0x, pattern=0x%x\n",
            user->user_id, buffer_id, buff->ul_header->guard_band, buff->ul_header->guard_band_e);
        return -EINVAL;
    }
    if (length > buff->ul_header->page_size) {
        LOGE("write_done of user%d buffer%d: invalid length=%d\n", user->user_id, buffer_id, length);
        return -EINVAL;
    }

    // search for the page
    for (i = 0; i < buff->ul_page_num; i++) {
        page = ccci_ccb_get_page(buff->ul_header, buff->ul_pages, i);
        if (page->buffer == address)
            break;
    }
    if (i == buff->ul_page_num) {
        LOGE("write done of user%d buffer%d: invalid address=%p\n", user->user_id, buffer_id, address);
        return -EINVAL;
    }
    if (page->page_status != PAGE_STATUS_ALLOC) {
        LOGE("write done of user%d buffer%d: invalid page%d, address=%p, status=%d\n",
            user->user_id, buffer_id, i, address, page->page_status);
        return -EINVAL;
    }
    page->page_status = PAGE_STATUS_WRITE_DONE;
    page->valid_length = length;

    // flush data before updating write pointer
    asm volatile("dmb ish":::"memory");
#if DEBUG_MSG_ON
    //add in debug start
    debug_it.buffer_id = buffer_id;
    debug_it.page_id = buff->ul_header->write_index;
    retval=ioctl(user->fd, CCCI_IOC_GET_CCB_DEBUG_VAL, &debug_it);
    LOGD("buf%d readback of write val=%d\n", buffer_id, debug_it.value);
    //add in debug end
#endif
    // update write index
    i = buff->ul_header->write_index;
    while (i != buff->ul_header->allocate_index) {
        page = ccci_ccb_get_page(buff->ul_header, buff->ul_pages, i);
        if (page->page_status == PAGE_STATUS_WRITE_DONE) {
            if(buff->ul_header->write_index + 1 >= buff->ul_page_num)
                buff->ul_header->write_index = 0;
            else
                buff->ul_header->write_index++;
        } else {
            break;
        }
        i++;
        if (i >= buff->ul_page_num)
            i = 0;
    }

    LOGD("write done of user%d buffer%d: OK, i=%d write=%d, alloc=%d, free=%d, len=%d\n", user->user_id, buffer_id, i,
        buff->ul_header->write_index, buff->ul_header->allocate_index, buff->ul_header->free_index, length);

    // send tx notify
    config = ccci_ccb_query_config(user->user_id, buffer_id);
    data = config->core_id;
    ret = ioctl(user->fd, CCCI_IOC_SMEM_TX_NOTIFY, &data);
    return ret;
}

static inline unsigned int ccci_ccb_read_available(struct ccci_ccb_control_buff *buff)
{
    int available = buff->dl_header->write_index -  buff->dl_header->read_index;
    if (available < 0)
        available += buff->dl_page_num;
    return available;
}

/*
 * input is bitmask of all buffer slots which this user has
 * return value: <0: error; >0: bitmask of availbe buffer slots; =0: nothing new
 */
int ccci_ccb_poll(unsigned int bitmask) {
    int ret = 0, available = 0;
    unsigned int i;
    struct ccci_ccb_control_buff *buff;
retry:
    if (!user)
        return -EFAULT;

    for (i = 0; i < user->buffer_num; i++) {
        if (bitmask & (1 << i)) {
            buff = user->buffers + i;
            if (ccci_ccb_read_available(buff)) {
                available |= (1 << i);
#if DEBUG_MSG_ON
                LOGD("IF, i=%d, GS=%X, a=%d, f=%d, r=%d, w=%d, pagesize=%X, bufsize=%X, GB=%X\n", i,
                        buff->dl_header->guard_band,
                        buff->dl_header->allocate_index,
                        buff->dl_header->free_index,
                        buff->dl_header->read_index,
                        buff->dl_header->write_index,
                        buff->dl_header->page_size,
                        buff->dl_header->data_buffer_size,
                        buff->dl_header->guard_band_e);
#endif
            } else {
#if DEBUG_MSG_ON
                LOGD("ELSE, i=%d, GS=%X, a=%d, f=%d, r=%d, w=%d, pagesize=%X, bufsize=%X, GB=%X\n", i,
                        buff->dl_header->guard_band,
                        buff->dl_header->allocate_index,
                        buff->dl_header->free_index,
                        buff->dl_header->read_index,
                        buff->dl_header->write_index,
                        buff->dl_header->page_size,
                        buff->dl_header->data_buffer_size,
                        buff->dl_header->guard_band_e);
#endif
            }
        }
    }

    #ifdef CCB_POLL_LOG_EN
    LOGD("poll on user%d available=%d, bitmask=%x\n", user->user_id, available, bitmask);
    #endif

    if (!available) {
        ret = ioctl(user->fd, CCCI_IOC_SMEM_RX_POLL, &bitmask);
        if (ret < 0) {
            if (errno == ENODEV) {
                LOGE("ccb poll fail, ret=%d, md is not ready\n", ret);
                return -2;
            } else {
                #ifdef CCB_POLL_LOG_EN
                LOGD("ccb poll fail, ret=%d, errno=%d\n", ret, errno);
                #endif
                return ret;
            }
        }
        goto retry;
    }

    return available;
}

int ccci_ccb_read_get(unsigned int buffer_id, unsigned char **address, unsigned int *length)
{
    struct ccci_ccb_control_buff *buff;
    struct page_header *page;
    unsigned int available;

    if (!user)
        return -EFAULT;
    if (buffer_id > user->buffer_num)
        return -EINVAL;
    buff = user->buffers + buffer_id;

    if (buff->dl_header->guard_band_e != TAIL_MAGIC_AFTER) {
        LOGE("read get of user%d buffer%d: MD not ready on DL, pattern=0x%0x, pattern_e=0x%x\n",
            user->user_id, buffer_id, buff->dl_header->guard_band, buff->dl_header->guard_band_e);
        return -EINVAL;
    }

    available = ccci_ccb_read_available(buff);
    if (available) {
        page = ccci_ccb_get_page(buff->dl_header, buff->dl_pages, buff->dl_header->read_index);
        if (page->page_status != PAGE_STATUS_WRITE_DONE) {
            LOGE("read get of user%d buffer%d: invalid status=%d, read=%d\n", user->user_id, buffer_id,
                page->page_status, buff->dl_header->read_index);
            return -EFAULT;
        }
        *address = page->buffer;
        *length = page->valid_length;

        #ifdef CCB_POLL_LOG_EN
        LOGD("read get of user%d buffer%d: read=%d\n", user->user_id, buffer_id, buff->dl_header->read_index);
        #endif
        if(buff->dl_header->read_index + 1 >= buff->dl_page_num)
            buff->dl_header->read_index = 0;
        else
            buff->dl_header->read_index++;
    } else {
        *address = NULL;
        *length = 0;
        return -EAGAIN;
    }
    return 0;
}

int ccci_ccb_read_done(unsigned int buffer_id)
{
    struct ccci_ccb_control_buff *buff;
    unsigned int free_index_tobe;

    if (!user)
        return -EFAULT;
    if (buffer_id > user->buffer_num)
        return -EINVAL;
    buff = user->buffers + buffer_id;

    if (buff->dl_header->guard_band_e != TAIL_MAGIC_AFTER) {
        LOGE("read done of user%d buffer%d: MD not ready on DL, pattern=0x%0x, pattern_e=0x%x\n",
            user->user_id, buffer_id, buff->dl_header->guard_band, buff->dl_header->guard_band_e);
        return -EINVAL;
    }

    free_index_tobe = buff->dl_header->free_index + 1;
    if (free_index_tobe >= buff->dl_page_num)
        free_index_tobe = 0;
    if (buff->dl_header->read_index != free_index_tobe) {
        LOGE("read done of user%d buffer%d: invalid index, read=%d, free=%d\n", user->user_id, buffer_id,
                buff->dl_header->read_index, buff->dl_header->free_index);
        return -EFAULT;
    }

    #ifdef CCB_POLL_LOG_EN
    LOGD("read done of user%d buffer%d: OK free=%d\n", user->user_id, buffer_id, buff->dl_header->free_index);
    #endif

    buff->dl_header->free_index = free_index_tobe;
    return 0;
}

/* handle dirty data for uplayer */
int ccci_ccb_read_flush(unsigned int buffer_id)
{
    struct ccci_ccb_control_buff *buff;

    buff = user->buffers + buffer_id;

    buff->dl_header->read_index = buff->dl_header->write_index;
    buff->dl_header->free_index = buff->dl_header->read_index;


    return 0;
}

int ccci_get_ccb_support( CCCI_USER usr_id, CCCI_MD md_id)
{
	struct stat buf;
	char dev_port[32];
	int ret;

	if (md_id != MD_SYS1)
		return 0;

	snprintf(dev_port, 32, "%s", ccci_get_node_name(usr_id, md_id));

	switch(usr_id) {
		case USR_SMEM_CCB_DHL:
			ret = stat(dev_port, &buf);
			if (ret < 0) {
				LOGE("not support ccb emdlogger\n");
				break;
			}
			return (USR_SMEM_CCB_DHL - CCB_START_INDEX + 1);
		case USR_SMEM_CCB_MD_MONITOR:
			ret = stat(dev_port, &buf);
			if (ret < 0) {
				LOGE("not support ccb md monitor\n");
				break;
			}
			return (USR_SMEM_CCB_MD_MONITOR - CCB_START_INDEX + 1);
		case USR_SMEM_CCB_META:
			ret = stat(dev_port, &buf);
			if (ret < 0) {
				LOGE("not support ccb meta\n");
				break;
			}
			return (USR_SMEM_CCB_META - CCB_START_INDEX + 1);
		default:
			break;
	}
	return 0;
}
