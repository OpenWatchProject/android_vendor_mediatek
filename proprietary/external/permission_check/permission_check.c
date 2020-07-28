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
 *   
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
#include <cutils/properties.h>
#include <android/log.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <stdlib.h>

#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "permission_check",__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "permission_check",__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , "permission_check",__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , "permission_check",__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "permission_check",__VA_ARGS__)

/* 
 * Notice
 * For origin design, ccci_fsd may create files and folders whose attribute is "0000, root.root".
 * In order to make cell phone more safe, ccci_fsd change its user from root to ccci.
 * For, MOTA update, data patition will not be ereased; then, ccci_fsd loss the capability to read/write md nvram.
 * EE will occur. So, we modify md nvram files's attribute fisrt here
 */
void change_md_nvram_attr(void)
{
    struct stat statbuf;
    LOGD("change_md_nvram_attr++");
    if(stat("/data/ccci_cfg/md_new_ver.1", &statbuf) == 0){
        LOGD("md_new_ver.1 file exist!!!");
        LOGD("change_md_nvram_attr--0");
        return;
    }

    LOGD("new_ver file NOT exist, change attr");
    umask(0007);
    // Begin to change file mode and group
    //system("chmod 0770 /data/nvram/md");
    system("chmod 0770 /data/nvram/md/NVRAM");
    system("chmod 0770 /data/nvram/md/NVRAM/NVD_IMEI");
    system("chmod 0770 /data/nvram/md/NVRAM/IMPORTNT");
    system("chmod 0770 /data/nvram/md/NVRAM/CALIBRAT");
    system("chmod 0770 /data/nvram/md/NVRAM/NVD_CORE");
    system("chmod 0770 /data/nvram/md/NVRAM/NVD_DATA");
    system("chmod 0660 /data/nvram/md/NVRAM/NVD_IMEI/*");
    system("chmod 0660 /data/nvram/md/NVRAM/IMPORTNT/*");
    system("chmod 0660 /data/nvram/md/NVRAM/CALIBRAT/*");
    system("chmod 0660 /data/nvram/md/NVRAM/NVD_CORE/*");
    system("chmod 0660 /data/nvram/md/NVRAM/NVD_DATA/*");
    
    system("chmod 0777 /data/mdl");
    system("chmod 0777 /data/mdl/*");
    
    system("chmod 0775 /data/nvram/dm");
    system("chmod 0775 /data/nvram/dm/*");
    
    // Make sure files has correct owner and group
    system("chown radio.system /data/nvram/md");
    system("chown -R radio.system /data/nvram/md/*");
    system("chown radio.system /data/nvram/md2");
    system("chown -R radio.system /data/nvram/md2/*");
    system("chown radio.system /data/nvram/md5");
    system("chown -R radio.system /data/nvram/md5/*");

    system("chown system.radio /data/ccci_cfg");
    system("chown -R system.radio /data/ccci_cfg/*");

    system("chown radio.system /protect_f/md");
    system("chown -R radio.system /protect_f/md/*");
    system("chown radio.system /protect_f/md2");
    system("chown -R radio.system /protect_f/md2/*");
    system("chown radio.system /protect_f/md5");
    system("chown -R radio.system /protect_f/md5/*");

    system("chown radio.system /protect_s/md");
    system("chown -R radio.system /protect_s/md/*");
    system("chown radio.system /protect_s/md2");
    system("chown -R radio.system /protect_s/md2/*");
    system("chown radio.system /protect_s/md5");
    system("chown -R radio.system /protect_s/md5/*");

#if 0
    system("chown root.nvram /data/nvram/md");
    system("chown root.nvram /data/nvram/md/NVRAM");
    system("chown root.nvram /data/nvram/md/NVRAM/NVD_IMEI");
    system("chown root.nvram /data/nvram/md/NVRAM/IMPORTNT");
    system("chown root.nvram /data/nvram/md/NVRAM/CALIBRAT");
    system("chown root.nvram /data/nvram/md/NVRAM/NVD_CORE");
    system("chown root.nvram /data/nvram/md/NVRAM/NVD_DATA");
    system("chown root.nvram /data/nvram/md/NVRAM/NVD_IMEI/*");
    system("chown root.nvram /data/nvram/md/NVRAM/IMPORTNT/*");
    system("chown root.nvram /data/nvram/md/NVRAM/CALIBRAT/*");
    system("chown root.nvram /data/nvram/md/NVRAM/NVD_CORE/*");
    system("chown root.nvram /data/nvram/md/NVRAM/NVD_DATA/*");
    system("chown shell.shell /data/mdl");
    system("chown shell.shell /data/mdl/*");
    
    system("chown system.system /data/nvram/dm");
    system("chown system.system /data/nvram/dm/*");
#endif

    system("echo flag > /data/ccci_cfg/md_new_ver.1");
    system("chmod 0660 /data/ccci_cfg/md_new_ver.1");
    LOGD("change_md_nvram_attr--1");
}

const char *prop_md_perm = "persist.md.perm.checked";

static int wait_decrypt_done(void)
{
#define MAX_RETRY_TIMES 120
	  int retry=0;
    char property_val[PROPERTY_VALUE_MAX] = {0};
    LOGD("waiting vold.decrypt=trigger_restart_framework\n");	
    property_get("vold.decrypt", property_val, NULL);
    while(strcmp(property_val, "trigger_restart_framework") != 0){
    	retry++;
    	if((retry%MAX_RETRY_TIMES)==0)
    	{
    			LOGD("wait vold.decrypt...,%s\n",property_val);
    	}
    	usleep(500*1000);
    	property_get("vold.decrypt", property_val, NULL);
    }
    LOGD("wait vold.decrypt=%s done success!\n",property_val);
    return 0;
}

static int wait_nvram_ready(void)
{
#define MAX_RETRY_TIMES 120
	int retry=0;
	int ret=-1;
	char property_val[PROPERTY_VALUE_MAX] = {0};
	LOGD("waiting nvram ready! %d\n", retry);
	while(retry < MAX_RETRY_TIMES){
		retry++;
		property_get("vendor.service.nvram_init", property_val, NULL);
		if(strcmp(property_val, "Ready") == 0){
			LOGD("nvram ready! %d\n", retry);
			return 0;
		}else{
			usleep(500*1000);
		}
	}
	LOGD("nvram not ready! %d\n", retry);			
	return -1;
}
static int check_decrypt_ready(void)
{
		int ret; //0 success, 1: skip, 2:error
    char property_val[PROPERTY_VALUE_MAX] = {0};
    // Check whether is at decrypt state
    property_get("ro.crypto.state", property_val, NULL);
    LOGD("ro.crypto.state=%s\n",property_val);	
    if(strcmp(property_val, "") == 0){
   		LOGD("auto encrypt & decrypt\n");
   		wait_decrypt_done();
   		wait_nvram_ready();
   		return 0;
    }else if(strcmp(property_val, "unencrypted") == 0){
			LOGD("unencrypted!!\n");
			return 0;
		}else if(strcmp(property_val, "encrypted") == 0){
			property_get("vold.decrypt", property_val, NULL);
			while(strcmp(property_val, "") == 0){
				property_get("vold.decrypt", property_val, NULL);
			}
			LOGD("vold.decrypt=%s\n",property_val);
			if(strcmp(property_val, "trigger_restart_framework") == 0){
				LOGD("trigger_restart_framework\n");
				//wait_decrypt_done();
				wait_nvram_ready();
   			return 0;		
			}else if(strcmp(property_val, "trigger_restart_min_framework") == 0){
				LOGD("trigger_restart_min_framework!!\n");
				return 1;
			}
  	}
  	LOGD("error encrypted!!\n");
  	return 0;
}

#define MAX_NVRAM_RESTRORE_READY_RETRY_NUM 22

int main(int argc, char **argv)
{
    int  retry = 0;
    int  ready = 0;
    int  need_check = 0;
    char property_val[PROPERTY_VALUE_MAX] = {0};
    struct stat statbuf;

    LOGD("permission check ver:0.02\n");
    umask(0007);
    if (stat("/data/ccci_cfg",&statbuf)<0)
    {
        LOGE("No /data/ccci_cfg dir.\n");
        if (mkdir("/data/ccci_cfg",0770)<0)
            LOGE("mkdir for ccci_cfg failed.\n");
    }

    ready = check_decrypt_ready();
    LOGD(" check_decrypt_ready:%d\n", ready);
    if (ready==1) {
    	  system("echo flag > /data/ccci_cfg/md_new_ver.1");
    		system("chmod 0660 /data/ccci_cfg/md_new_ver.1");
        LOGE("no decrypt,create md_new_ver flag on tmpfs, ret directly!\n");
        return 1;
    }else if(ready==2){
        LOGE("Wait decrypt timeout, ret directly!\n");
        return 1;
    }else{
        LOGE("do permission check!\n");
    }
    // Check whether is at decrypt state
    property_get(prop_md_perm, property_val, NULL);

    LOGD("[%s]:[%s]\n", prop_md_perm, property_val);

    change_md_nvram_attr();
    property_set(prop_md_perm, "1");

    return 0;
}

