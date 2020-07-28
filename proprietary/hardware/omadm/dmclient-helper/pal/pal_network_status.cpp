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

#include <cutils/log.h>
#include <stdlib.h>
#include <string.h>
#include "omadm_service_api.h"
#include "pal.h"


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "PAL"

#define CLBK_LEN 3
static network_conditions_changed_t nwchang_listener[CLBK_LEN];

static void nw_cond_changed(void *);
static int get_empty_clbk_id(network_conditions_changed_t
        event);
/**
 * Set callback function for process cellular network events
 *
 * @param[in] netm_handler_process_event Callback function
 * @return non-negative id of registered callback if success, otherwise error
 * code.
 */
int pal_notify_on_network_conditions_changed(network_conditions_changed_t net_event)
{
    ALOGD("pal_notify_on_network_conditions_changed()");
    int status = 0;
    if (net_event == NULL) {
        ALOGE("pal_notify_on_network_conditions_changed() error callback is NULL!!!");
    }
    int id = get_empty_clbk_id(net_event);
    if (id < 0) {
        return id;
    }
    
    nwchang_listener[id] = net_event;
    status = pal_register_omadm_callback(OMADM_NET_MANAGER_REPLY, nw_cond_changed);
    
    if (status == NO_ERR) {
        return id;
    } else {
        nwchang_listener[id] = NULL;
    }
    
    return -ERR_ADD_CLBK;
}

int pal_notify_stop_on_network_conditions_changed(int clbkid)
{
    ALOGD("pal_notify_stop_on_network_conditions_changed()");
    if (clbkid < 0 || clbkid > CLBK_LEN - 1) {
        return -ERR_DEL_CLBK;
    }
    nwchang_listener[clbkid] = NULL;
    int status = pal_unregister_omadm_callback(OMADM_NET_MANAGER_REPLY);
    
    return -status;
}

void nw_cond_changed(void *cond)
{
    network_conditions_t *nc = (network_conditions_t *)cond;
    if (nc == NULL) {
        ALOGE("nw_cond_changed() condition is NULL!!!");
        return;
    }
    ALOGD("nw_cond_changed() type [%d], enabled [%d]", (int)nc->net_feature, (int)nc->enabled );
    for (int i = 0; i < CLBK_LEN; ++i) {
        if (nwchang_listener[i] == NULL) {
            ALOGD("nw_cond_changed() nwchang_listener[%d] wasn't properly set and missed", i);
        } else {
            network_conditions_t local;
            memcpy(&local, nc, sizeof(network_conditions_t));
            ALOGE("notify listener [%d]", i);
            nwchang_listener[i](local);
        }
    }
    free(nc);
    
    return;
}

/**
 * @return -1 if specified callback was already registered.
 * -2 if there is no free slot for a new callback was found.
 */
int get_empty_clbk_id(network_conditions_changed_t event) {
    int id;
    int i;
    for (i = 0; i < CLBK_LEN; ++i) {
        if (nwchang_listener[i] == NULL) {
            id = i;
            break;
        } else if (nwchang_listener[i] == event) {
            id = -1;
            break;
        }
    }
    if (i == CLBK_LEN) {
        id = -2;
        ALOGE("get_empty_clbk_id() id slot is FULL for callback!!");
    }
    return id;
}