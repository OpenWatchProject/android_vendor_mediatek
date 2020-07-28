/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*
* MediaTek Inc. (C) 2017. All rights reserved.
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

#include <condition_variable>
#include <cutils/log.h>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "omadm_service_api.h"
#include "pal.h"
#include "pal_internal.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "PAL"

using namespace android;
using namespace omadm_service_api;

static std::mutex m_mutex;
static std::condition_variable cond;
static int adm_status = ADMINNW_UNKNOWN;

bool is_adminNW_available() {
    return (ADMINNW_UNKNOWN == adm_status) ? false : true;
}

void on_admNW_changed(void* status)
{
    std::unique_lock<std::mutex> lck(m_mutex);
    adm_status = (long)status;
    cond.notify_one();
    ALOGD("on_admNW_changed(): status = %d\n", adm_status);
}

static omadmCallback network_status_cb = on_admNW_changed;

int request_admin_network(bool enable, omadmCallback admin_cb)
{
    int status = NO_ERR;
    OMADM_SRV_READY();

    if(enable) {
        status = pal_register_omadm_callback(OMADM_ADMIN_NETWORK_STATUS, admin_cb);
        if (status != NO_ERR) {
            ALOGE("pal_register_omadm_callback(OMADM_ADMIN_NETWORK_STATUS): error %d", status);
            return status;
        }
    } else {
        status = pal_unregister_omadm_callback(OMADM_ADMIN_NETWORK_STATUS);
        if (status != NO_ERR) {
            ALOGE("pal_unregister_omadm_callback(OMADM_ADMIN_NETWORK_STATUS): error %d", status);
        }
    }
    
    bool ret = OmadmServiceAPI::requestAdminNetwork(enable);
    ALOGD("request_admin_network(%d) ret = %d", enable, ret);
    
    return ret;
}

int pal_request_admin_network(bool enable)
{
    int ret = -1;
    ALOGD("pal_request_admin_network() enter");
    if (enable) {
        ret = request_admin_network(enable, network_status_cb);
        if( NO_ERR == ret ) {
            ALOGD("pal_request_admin_network(true): return = %d\n", ret);
        } else {
            ALOGE("Exiting with error. pal_request_admin_network(false): return = %d\n", ret);
            return adm_status;
        }
        // wait for network
        while(1) {
            std::unique_lock<std::mutex> lck(m_mutex);
            cond.wait(lck, is_adminNW_available);
            if(is_adminNW_available())
                break;
        }
    } else {
        ret = request_admin_network(enable, NULL);
        if( NO_ERR == ret ) {
            ALOGD("pal_request_admin_network(false): return = %d\n", ret);
        } else {
            ALOGE("Exiting with error. pal_request_admin_network(false): return = %d\n", ret);
        }
        adm_status = ADMINNW_UNKNOWN;
    }
    if (ADMINNW_UNKNOWN != adm_status) {
        sleep(ADMINNW_TMOUT);
    }
    return adm_status;
}
