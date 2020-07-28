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


#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <linux/input.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <cutils/properties.h>

#include "common.h"
#include "ftm.h"
#include "utils.h"

struct key keys_keymap[10];
int keys_num = 0;
ctext_t keys_ctext[10];
unsigned char mask[KEY_CNT/8 + 1];
#define test_bit(bit) ((mask[(bit)/8])&(1<<((bit)%8)))

#define TAG         "[FTM UTILS] "
#define BOOTMODE_PATH "/sys/class/BOOT/BOOT/boot/boot_mode"

int readSys_int(char const * path)
{
    int fd;
    
    if (path == NULL)
        return -1;

    fd = open(path, O_RDONLY);
    if (fd >= 0) {
        char buffer[20];
        int amt = read(fd, buffer, sizeof(int));          
        close(fd);
        return amt == -1 ? -errno : atoi(buffer);
    }
    LOGE(TAG "write_int failed to open %s\n", path);
    return -errno;    
}

int writeSys_int(char const * path, int value)
{
    int fd;

    if (path == NULL)
        return -1;

    fd = open(path, O_RDWR);
    if (fd >= 0) {
        char buffer[20];
        int bytes = sprintf(buffer, "%d\n", value);
        int amt = write(fd, buffer, bytes);          
        close(fd);
        return amt == -1 ? -errno : 0;
    }
    LOGE(TAG "write_int failed to open %s\n", path);
    return -errno;    
}

int getBootMode()
{
    int bootMode;

    bootMode = readSys_int(BOOTMODE_PATH);

    if(FACTORY_BOOT== bootMode)
        LOGD(TAG "Factory mode boot!\n");
    else if(ATE_FACTORY_BOOT== bootMode)
        LOGD(TAG "ATE Factory mode boot!\n");
    else {
        bootMode = -1;
        LOGD(TAG "Unsupported factory mode!\n");
    }
     
    return bootMode;   
}

FILE * open_file(char* filename, int mode)
{
	FILE *fp = NULL;
    if(filename == NULL)
    {
        return NULL;
    }
	char file_md[4] = "r";
	switch (mode)
	{
    case 0:
		strncpy(file_md, "r", 2);
        break;
	case 1:
		strncpy(file_md, "w", 2);
		break;
	default:
		break;
	}
	if (NULL == (fp = fopen(filename, file_md)))
	{
		LOGD(TAG "Open file fail\n");
	}
	return fp;
}

void ftm_save_key_map(int key_code)
{
	switch(key_code){
		case KEY_VOLUMEDOWN:
			keys_keymap[keys_num].code = key_code;
			memcpy(keys_keymap[keys_num].name, "VoDown", 6);
			keys_num++;
			break;
		case KEY_VOLUMEUP:
			keys_keymap[keys_num].code = key_code;
			memcpy(keys_keymap[keys_num].name, "VoUp", 5);
			keys_num++;
			break;
		case KEY_POWER:
			keys_keymap[keys_num].code = key_code;
			memcpy(keys_keymap[keys_num].name, "Power", 5);
			keys_num++;
			break;
		case KEY_HOME:
			keys_keymap[keys_num].code = key_code;
			memcpy(keys_keymap[keys_num].name, "Home", 4);
			keys_num++;
			break;
		case KEY_END:
			keys_keymap[keys_num].code = key_code;
			memcpy(keys_keymap[keys_num].name, "EndCal", 6);
			keys_num++;
			break;
		case KEY_SEND:
			keys_keymap[keys_num].code = key_code;
			memcpy(keys_keymap[keys_num].name, "Call", 4);
			keys_num++;
			break;
		case KEY_BACK:
			keys_keymap[keys_num].code = key_code;
			memcpy(keys_keymap[keys_num].name, "Back", 4);
			keys_num++;
			break;
		case KEY_RESTART:
			keys_keymap[keys_num].code = key_code;
			memcpy(keys_keymap[keys_num].name, "Mrdump", 6);
			keys_num++;
			break;
		default:
			LOGD(TAG " kpd = %d", key_code);
			break;
	}
}
void ftm_get_key_map(void)
{
	char name[64];
	int fd, key_code = 0, m;
	LOGD(TAG " start get key map device");
	char buf[256] = {0,};
	for(m = 0; m <32; m++){
		sprintf(name, "/dev/input/event%d", m);
		if((fd = open(name, O_RDONLY, 0))>=0){
			//LOGD(TAG " kpd kpd_fd = %d", fd);
			ioctl(fd,EVIOCGNAME(sizeof(buf)), buf);
			if(memcmp(buf, "mtk-kpd", 7)==0 || 
				memcmp(buf, "mtk-pmic-keys", 13) == 0){/*add for pmic keys upstream driver*/
				//LOGD(TAG " kpd name = %s", buf);
				ioctl(fd,EVIOCGBIT(EV_KEY,sizeof(mask)), mask);
				for(key_code=0; key_code<KEY_CNT; key_code++){
					if(test_bit(key_code)){
						ftm_save_key_map(key_code);
					}
				}
				close(fd);
				/*break;*//*disable for pmic keys upstream driver*/
			}else{
				close(fd);
			}
		}else{
			LOGD(TAG "open device failed");
		}
	}
}

int ftm_get_sim_num(void) {
    int num = 0;
    char sim_num[128] = {0};
    property_get("persist.vendor.radio.msimmode", sim_num, "ss");
    LOGD(TAG "After __FUNCTION__ : %s\n", sim_num);
    if (strlen(sim_num) > 0) {
        if (strcmp("ss", sim_num) == 0) {
            num = 1;
        } else if (strcmp("dsds", sim_num) == 0) {
            num = 2;
        } else if (strcmp("dsda", sim_num) == 0) {
            num = 2;
        }else if (strcmp("tsts", sim_num) == 0) {
            num = 3;
        } else if (strcmp("qsqs", sim_num) == 0) {
            num = 4;
        }
    }
    LOGD(TAG "__FUNCTION__ : %d\n", num);
    return num;
}

