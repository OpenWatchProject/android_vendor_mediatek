/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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

#include <stdlib.h>
#include <string.h>
#include <cutils/log.h>
#include <cutils/properties.h>

#include "sysenv_utils.h"
#include "MetaPub.h"
#include "meta_msim_para.h"

#undef LOG_TAG
#define LOG_TAG "META_MSIM"

#define DBG(f, ...)   ALOGD("%s: " f, __func__, ##__VA_ARGS__)

#define TRUE 1
#define FALSE 0

#define FAIL 0
#define SUCCESS_REBOOT 1
#define SUCCESS_NO_REBOOT 2

#define PROPERTY_BUILD_MSIM_CONFIG           "persist.vendor.radio.multisimslot"

#define SS "ss"
#define DSDS "dsds"

// for sysenv
#define MSIM_CONFIG "msim_config"
/*************************/

#define MAX_MSIM_STRING_LEN 16
static MSIM_CNF msim_cnf;

bool META_MSIM_init() {
    DBG("start");

    return true;
}

void META_MSIM_deinit() {

}

void META_MSIM_OP(MSIM_REQ *req, char *peer_buff __unused, unsigned short peer_len __unused) {
    char temp[PROPERTY_VALUE_MAX]={ 0 };

    memset(&msim_cnf, 0, sizeof(MSIM_CNF));
    msim_cnf.header.id = FT_MSIM_CNF_ID;
    msim_cnf.header.token = req->header.token;
    msim_cnf.op = req->op;
    property_get(PROPERTY_BUILD_MSIM_CONFIG, temp, "dsds");
    DBG("req->op:%d, cur msim:%s, input:%s\n", req->op, temp, req->cmd.m_msim.m_MsimValue);
    switch(req->op) {
        case MSIM_OP_GET_MSIM:
            memcpy(msim_cnf.MSIM_result.m_getMsim.m_MsimValue, &temp,
                    sizeof(msim_cnf.MSIM_result.m_getMsim.m_MsimValue));
            msim_cnf.drv_status = TRUE;
            msim_cnf.status = META_SUCCESS;
            DBG("current msim:%s - %s\n", temp, msim_cnf.MSIM_result.m_getMsim.m_MsimValue);
            WriteDataToPC(&msim_cnf, sizeof(MSIM_CNF), NULL, 0);
            break;

        case MSIM_OP_SET_MSIM:
            if (strcmp(req->cmd.m_msim.m_MsimValue, temp) == 0) {
                DBG("META_MSIM_OP SUCCESS_NO_REBOOT");
                msim_cnf.MSIM_result.m_setMsim.m_Ret = SUCCESS_NO_REBOOT;
                msim_cnf.drv_status = TRUE;
                msim_cnf.status = META_SUCCESS;
            } else {
                DBG("write %s = %s\n", MSIM_CONFIG, req->cmd.m_msim.m_MsimValue);
                sysenv_set_static(MSIM_CONFIG, req->cmd.m_msim.m_MsimValue);
                DBG("read  %s = %s\n", MSIM_CONFIG, sysenv_get_static(MSIM_CONFIG));
                msim_cnf.MSIM_result.m_setMsim.m_Ret = SUCCESS_REBOOT;
                msim_cnf.drv_status = TRUE;
                msim_cnf.status = META_SUCCESS;
            }
            WriteDataToPC(&msim_cnf, sizeof(MSIM_CNF), NULL, 0);
            break;

        default:
            DBG("Not support OPCODE:req->op [%d]\n", req->op);
            msim_cnf.status = META_FAILED;
            WriteDataToPC(&msim_cnf, sizeof(MSIM_CNF), NULL, 0);
            break;
    }
}
