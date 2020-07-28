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

#ifndef PAL_COMMON_H
#define PAL_COMMON_H

#include <stdint.h>
#include <sys/cdefs.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Structure which is used to exchange data with PAL library
 */
typedef struct {
    unsigned int size; //size of the following data
    char *data;
} data_buffer_t;

/// OMADM request types
typedef enum {
    PACKAGE0,         ///< wap push or UI type
    GENERIC_ALERT,    ///< management generic object alert type
    SESSION_ALERT,    ///< management object alert type
    SU_CANCEL,        ///< SU cancel sms type
    USER_SYS_UPDATE,  ///< start system update by user request
    START_AUTOSESSION,///< start system update by autosession request
    UDM_REQUEST,      ///< start system update by udm request
} omadm_request_type_t;

/**
 * Omadm Callback functions identification
 */
typedef enum {
    OMADM_ADMIN_NETWORK_STATUS = 101,
    OMADM_NET_MANAGER_REPLY = 102,
    OMADM_UNUSED = 200
} OmadmCallbackId;

// Declare callback
typedef int omadmCallback_id;
typedef void (*omadmCallback)(void*);

#define ADMINNW_UNKNOWN     0
#define ADMINNW_AVAILABLE   1
#define ADMINNW_UNAVAILABLE 2
#define ADMINNW_LOST        3
#define ADMINNW_TMOUT 5

typedef enum
{
    NETWORK_WIFI_CONNECTED = 0,
    NETWORK_CELLULAR_CONNECTED, // is actual for any cellular data connection
                                // such as LTE or 3G if possible
    NETWORK_DATA_ROAMING,
    NETWORK_ACTIVE_VOICE_CALL  // is actual for active voice call
} network_conditions_type_t;

/**
 *  @brief network event description
 */
typedef struct
{
    network_conditions_type_t net_feature;
    bool enabled;
} network_conditions_t;

#ifdef __cplusplus
}
#endif

#endif // PAL_COMMON_H
