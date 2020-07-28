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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <utils/Log.h>
#include <errno.h>
#include <unistd.h>

#ifdef MTK_GM_30
#define SUPPORT_RW_NVRAM
#endif

/* read NVRAM */
#ifdef SUPPORT_RW_NVRAM
#include "libnvram.h"
#include "CFG_FG_File.h"
#include "CFG_FG_Default.h"
#include "CFG_file_lid.h"
#include "Custom_NvRam_LID.h"
#endif

#include "at_tok.h"
#include "atci_service.h"
#include "atci_util.h"
#include "atci_gm_cmd.h"

// Add for AT CMD IOCTRL-----------------------------------------
#define Get_META_BAT_CAR_TUNE_VALUE _IOW('k', 12, int)
#define Set_META_BAT_CAR_TUNE_VALUE _IOW('k', 13, int)
#define Set_BAT_DISABLE_NAFG _IOW('k', 14, int)

#ifdef SUPPORT_RW_NVRAM
static ap_nvram_fg_config_struct stFGReadback, stFGWriteback;
#endif
int meta_adc_fd = 0;

typedef enum {
    OP_GET_CAR_TUNE_VALUE = 0,
    OP_SET_CAR_TUNE_VALUE,
    OP_DISABLE_NAFG,
    AT_OP_END
} AT_BAT_OP;

#ifdef SUPPORT_RW_NVRAM
static int read_NVRAM() {
    F_ID fg_nvram_fd;
    int rec_size;
    int rec_num;
    bool IsRead = true;

    memset(&stFGReadback, 0, sizeof(stFGReadback));

    fg_nvram_fd = NVM_GetFileDesc(AP_CFG_CUSTOM_FILE_FUEL_GAUGE_LID, &rec_size, &rec_num, IsRead);
    ALOGD("read NVRAM, rec_size %d, rec_num %d\n", rec_size, rec_num);
    if (fg_nvram_fd.iFileDesc > 0) {   /* >0 means ok */
        if (read(fg_nvram_fd.iFileDesc, &stFGReadback, rec_size * rec_num) < 0) {
            ALOGE("read NVRAM error, %s\n", strerror(errno));
        }
        NVM_CloseFileDesc(fg_nvram_fd);

        if (strlen(stFGReadback.dsp_dev) != 0) {
            ALOGD("[NVRAM Area] FG NVRam size:%d, number:%d, [Read]\n", rec_size, rec_num);
            ALOGD("[NVRAM Item] dsp_dev : %s\n", stFGReadback.dsp_dev);
            ALOGD("[NVRAM Item] nvram_car_tune_value : %d\n", stFGReadback.nvram_car_tune_value);
        } else {
            ALOGE("FG NVRam mnl_config.dev_dsp == NULL \n");
            return -1;
        }
    } else {
        ALOGE("FG read NVRam fg_nvram_fd.iFileDesc == %d \n", fg_nvram_fd.iFileDesc);
        return -1;
    }
    return 0;
}

static int write_NVRAM() {
    F_ID fg_nvram_fd;
    int rec_size;
    int rec_num;
    bool IsRead = false;

    fg_nvram_fd = NVM_GetFileDesc(AP_CFG_CUSTOM_FILE_FUEL_GAUGE_LID, &rec_size, &rec_num, IsRead);
    ALOGD("write NVRAM, rec_size %d, rec_num %d\n", rec_size, rec_num);
    if (fg_nvram_fd.iFileDesc > 0) {   /* >0 means ok */
        if (write(fg_nvram_fd.iFileDesc, &stFGWriteback, rec_size * rec_num) < 0) {
            ALOGE("write NVRAM error, %s\n", strerror(errno));
        }
        NVM_CloseFileDesc(fg_nvram_fd);

        if (strlen(stFGWriteback.dsp_dev) != 0) {
            ALOGD("[NVRAM Area] FG NVRam size:%d, number:%d, [Write]\n", rec_size, rec_num);
            ALOGD("[NVRAM Item] dsp_dev : %s\n", stFGWriteback.dsp_dev);
            ALOGD("[NVRAM Item] nvram_car_tune_value : %d\n", stFGWriteback.nvram_car_tune_value);
        } else {
            ALOGE("FG NVRam mnl_config.dev_dsp == NULL \n");
            return -1;
        }
    } else {
        ALOGE("FG write NVRam fg_nvram_fd.iFileDesc == %d \n", fg_nvram_fd.iFileDesc);
        return -1;
    }
    return 0;
}
#endif

int car_tune_cmd_handler(char* cmdline, ATOP_t at_op, char* response) {
    int adc_out_data[2] = {1, 1};
    int ret = -1;
#ifdef SUPPORT_RW_NVRAM
    int ret2 = -1;
#endif

    meta_adc_fd = open("/dev/MT_pmic_adc_cali", O_RDWR);
    if (meta_adc_fd == -1) {
        ALOGE("ERROR!!!!!!Open /dev/MT_pmic_adc_cali : ERROR \n");
        ALOGE("Open /dev/MT_pmic_adc_cali : ERROR \n");
        sprintf(response, "%s", "ERROR");
        return -1;
    }

    if (at_op == AT_READ_OP) {
        /* to test read data from NVRAM or get car_tune data from Kernel. */
        /* If support nvram , means test read car_tune data from NVRAM */
        /* If Not support nvram, means get car_tune from mtk_battery.c */
#ifdef SUPPORT_RW_NVRAM
        memset(&stFGReadback, 0, sizeof(stFGReadback));
        ret = read_NVRAM();
        adc_out_data[0] = stFGReadback.nvram_car_tune_value;
        ALOGD("Read car_tune[%d] from NVRAM nvram_car_tune_value\n", (int)adc_out_data[0]);
#else
        ret = ioctl(meta_adc_fd, Get_META_BAT_CAR_TUNE_VALUE, adc_out_data);
        ALOGD("Read car_tune[%d] from mtk_battery.c\n", (int)adc_out_data[0]);
#endif

        if (ret == -1) {
            ALOGE("ADC_OP_GET_CAR_TUNE_VALUE Get_META_BAT_CAR_TUNE_VALUE,ERROR!!!!!!! \n");
        } else {
            sprintf(response, "%d", (int)adc_out_data[0]);
            close(meta_adc_fd);
            return 0;
        }
    } else if (at_op == AT_SET_OP) {
        int invalue = atoi(cmdline);
        adc_out_data[0] = 0;
        adc_out_data[1] = invalue;
        ret = ioctl(meta_adc_fd, Set_META_BAT_CAR_TUNE_VALUE, adc_out_data);
        if (ret == -1) {
            ALOGE("OP_SET_CAR_TUNE_VALUE, Set_META_BAT_CAR_TUNE_VALUE  ERROR!!!invalue=%d\n", invalue);
            ALOGE("OP_SET_CAR_TUNE_VALUE ERROR, %s\n", strerror(errno));
        } else {
            ALOGD("OP_SET_CAR_TUNE_VALUE,car tune from mtk_battery is:%d,invalue=%d\n",
                    (int)adc_out_data[0], invalue);
        }

#ifdef SUPPORT_RW_NVRAM
        ALOGD("Write car_tune[%d] to NVRAM nvram_car_tune_value\n", (int)adc_out_data[0]);
        memset(&stFGWriteback, 0, sizeof(stFGWriteback));
        strncpy(stFGWriteback.dsp_dev, "/dev/stpfgd", 12);
        stFGWriteback.nvram_car_tune_value = (int)adc_out_data[0];
        ret2 = write_NVRAM();

        if (ret2 != 0) {
            ALOGE("nvram ioctl write NVRAM check FAIL \n");
            ret = -1;
        } else {
            ALOGD("nvram ioctl write NVRAM check SUCCESS \n");
        }
#endif
    }

    close(meta_adc_fd);
    ALOGD("car_tune_cmd_handler Finish !\n");
    if (ret == 0) {
        sprintf(response, "%s", "OK");
    } else {
        sprintf(response, "%s", "ERROR");
    }
    return ret;
}

int nafg_cmd_handler(char* cmdline, ATOP_t at_op, char* response) {
    int adc_out_data[2] = {1, 1};
    UNUSED(cmdline);
    int ret = -1;

    meta_adc_fd = open("/dev/MT_pmic_adc_cali", O_RDWR, 0);
    if (meta_adc_fd == -1) {
        ALOGE("ERROR!!!!!!Open /dev/MT_pmic_adc_cali : ERROR \n");
        ALOGE("Open /dev/MT_pmic_adc_cali : ERROR \n");
        sprintf(response, "%s", "ERROR");
        return -1;
    }

    if (at_op == AT_SET_OP) {
        int invalue = atoi(cmdline);
        /* invalue = 1 means disable NAFG */
        adc_out_data[0] = 0;
        adc_out_data[1] = invalue;
        ret = ioctl(meta_adc_fd, Set_BAT_DISABLE_NAFG, adc_out_data);
        if (ret == -1) {
            ALOGE("OP_DISABLE_NAFG, Set_BAT_DISABLE_NAFG  ERROR!!!%d\n", invalue);
            ALOGE("OP_DISABLE_NAFG, %s\n", strerror(errno));
        } else {
            ALOGD("OP_DISABLE_NAFG, Set_BAT_DISABLE_NAFG success,invalue=%d\n", invalue);
        }
    }

    close(meta_adc_fd);
    ALOGD("nafg_cmd_handler Finish !\n");
    if (ret == 0) {
        sprintf(response, "%s", "OK");
    } else {
        sprintf(response, "%s", "ERROR");
    }
    return ret;
}
