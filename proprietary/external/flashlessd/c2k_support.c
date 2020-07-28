/*
 *    Filename: oem_support.c
 *
 * Description: Flashless oem functions for CBP FSM file 
 *
 *      Author: qli@via-telecom.com
 *     Created: 07/20/2012 
 *     Version: 1.0
 *    Revision: 
 *              1. 07/19/2013 License statements added by xpwan@via-telecom.com 
 *
 * This Software is the property of VIA Telecom, Inc. and may only be used pursuant to a license from VIA Telecom, Inc.   
 * Any unauthorized use inconsistent with the terms of such license is strictly prohibited. 
 *
 * Copyright (c) 2012-2013 VIA Telecom, Inc. All rights reserved.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/wait.h>
#include <utils/Log.h> 

#include <c2kutils.h>
#include "c2k_support.h"

#define LOGD(fmt, args...) LOG_PRI(ANDROID_LOG_DEBUG, "Flashless", fmt, ##args)

int fsm_calibration_backup(__attribute__((unused))const char *path)
{
	int ret = -1;

	ret = FileOp_BackupToBinRegion_All();
	if(ret){
		ret = 0;
	}else{
		LOGD("FileOp_BackupToBinRegion_All failed %d\n", ret);
		ret = -1;
	}
	
	return ret;
}

int fsm_calibration_restore(const char *path)
{
	int ret = -1;

	ret = rfs_access_ok();
	
	if(ret < 0){
		LOGD("rfs_access_ok failed %d\n", ret);
		ret = -1;
		return ret;
	}
	
	ret = NVM_RestoreFromBinRegion_OneFile(-1, path);

	if(ret){
		ret = 0;
	}else{
		LOGD("NVM_RestoreFromBinRegion_OneFile failed %d\n", ret);
		ret = -1;
	}

	return ret;
}

/*Reset the host control when get modem error*/
int reset_modem_host(void)
{
	return system("echo 1 > /proc/EVDO_HOST_RESET");
}


char *get_customized_external_sdcard_path(void)
{
    return "/storage/sdcard1"; //for MTK-ZTE MT6589 + CBP8.2
}

char *get_customized_internal_sdcard_path(void)
{
    return "/storage/sdcard0"; //for MTK-ZTE MT6589 + CBP8.2
}

/*Send broadcast message to Taglog after ramdump image uploaded successfully*/
void notify_taglog_ramdump_event(char *ramdump_img_path)
{
    char *buf = NULL;
    int  len = 0;
    len = strlen("am broadcast -a com.mediatek.log2server.EXCEPTION_HAPPEND "
                 "--es path FromVIASend --es db_filename FromVIASend "
                 "--es via_modem_path ") + strlen(ramdump_img_path) +
                 strlen(" --es zz_filename ZZ_INTERNAL");
    buf = (char *)malloc(len + 1);
    if (!buf) {
        return;
    }
    memset(buf, 0, len+1); 
    sprintf(buf,  "am broadcast -a com.mediatek.log2server.EXCEPTION_HAPPEND "
            "--es path FromVIASend --es db_filename FromVIASend "
            "--es via_modem_path %s --es zz_filename ZZ_INTERNAL", ramdump_img_path);

    system(buf);

    free(buf);
}

