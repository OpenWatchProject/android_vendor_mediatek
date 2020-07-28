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

#include "omadm_service_api.h"
#include "pal.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "PAL"

#include "./pal_internal.h"
#include <fstream>

using namespace android;
using namespace omadm_service_api;

__BEGIN_DECLS

#define IMEI_PREFIX "IMEI:"

//device unique ID IMEI
int pal_system_dev_id_get(data_buffer_t *buff) {

    if ((NULL == buff) || (NULL == buff->data) || (0 == buff->size))
        return RESULT_BUFFER_NOT_DEFINED;

    AutoMutex _l(OmadmServiceAPI::mPalLock);

    if (!OmadmServiceAPI::isPalInitialized())
        return RESULT_ERROR_PAL_NO_INIT;

    String16* reply = NULL;
    reply = OmadmServiceAPI::getDeviceId();

    // Can return NULL on failure
    if (NULL == reply)
        return RESULT_ERROR;

    int prefix_len = strlen(IMEI_PREFIX);
    if ((reply->size() + prefix_len + 1) > buff->size) {
        delete reply;
        return RESULT_BUFFER_OVERFLOW;
    }
    // According to LTE_OTADM_Requirements:
    // 4.1.3.8 DevInfo Subtree - DevId format shall be "IMEI:x",
    // with x being the value of the IMEI.
    memset(buff->data, 0, buff->size);
    strncpy(buff->data, IMEI_PREFIX, prefix_len);
    strncat(buff->data, String8(*reply).string(), ( buff->size - prefix_len) );
    delete reply;
#ifdef DEBUG
    ALOGD("Device IMEI: %s", Buffer->data);
#endif
    return RESULT_SUCCESS;
}


// Device Manufacturer
int pal_system_man_get(data_buffer_t *buff){
    AutoMutex _l(OmadmServiceAPI::mPalLock);

    if(!OmadmServiceAPI::isPalInitialized())
        return RESULT_ERROR_PAL_NO_INIT;

    if((NULL == buff)||(NULL == buff->data)||
            (0 == buff->size))
        return RESULT_BUFFER_NOT_DEFINED;

    String16* reply = NULL;
    reply = OmadmServiceAPI::getManufacturer();

    // Can return NULL on failure
    if(NULL == reply)
        return RESULT_ERROR_UNDEFINED;

    if((reply->size() + 1) > buff->size) {
        delete reply;
        return RESULT_BUFFER_OVERFLOW;
    }
    memset(buff->data, 0, buff->size);
    strncpy(buff->data, String8(*reply).string(), buff->size);
    
    delete reply;
#ifdef DEBUG
    ALOGD("Manufactor ID %s", buff->data);
#endif
    return RESULT_SUCCESS;
}

// model name
int pal_system_mod_get(data_buffer_t *buff){
    AutoMutex _l(OmadmServiceAPI::mPalLock);

    if(!OmadmServiceAPI::isPalInitialized())
        return RESULT_ERROR_PAL_NO_INIT;

    if((NULL == buff)||(NULL == buff->data)||
            (0 == buff->size))
        return RESULT_BUFFER_NOT_DEFINED;

    String16* reply = NULL;
    reply = OmadmServiceAPI::getModel();

    // Can return NULL on failure
    if(NULL == reply)
        return RESULT_ERROR_UNDEFINED;

    if((reply->size() + 1) > buff->size) {
        delete reply;
        return RESULT_BUFFER_OVERFLOW;
    }
    memset(buff->data, 0, buff->size);
    strncpy(buff->data, String8(*reply).string(), buff->size);
    delete reply;
#ifdef DEBUG
    ALOGD("Model ID %s", buff->data);
#endif
    return RESULT_SUCCESS;
}

//OMADM DMClient version
int pal_system_dmv_get(data_buffer_t *buff){
    AutoMutex _l(OmadmServiceAPI::mPalLock);

    if(!OmadmServiceAPI::isPalInitialized())
        return RESULT_ERROR_PAL_NO_INIT;

    if((NULL == buff)||(NULL == buff->data)||
            (0 == buff->size))
        return RESULT_BUFFER_OVERFLOW;

    String16* reply = NULL;
    reply = OmadmServiceAPI::getDmv();

    // Can return NULL on failure
    if(NULL == reply)
        return RESULT_ERROR_UNDEFINED;

    if((reply->size() + 1) > buff->size) {
        delete reply;
        return RESULT_BUFFER_OVERFLOW;
    }
    memset(buff->data, 0, buff->size);
    strncpy(buff->data, String8(*reply).string(), buff->size);
    delete reply;
#ifdef DEBUG
    ALOGD("Dmv ID %s", buff->data);
#endif
    return RESULT_SUCCESS;
}

//get language
int pal_system_lang_get(data_buffer_t *buff){
    AutoMutex _l(OmadmServiceAPI::mPalLock);

    if(!OmadmServiceAPI::isPalInitialized())
        return RESULT_ERROR_PAL_NO_INIT;

    if((NULL == buff)||(NULL == buff->data)||
            (0 == buff->size))
        return RESULT_BUFFER_NOT_DEFINED;

    String16* reply = NULL;
    reply = OmadmServiceAPI::getLanguage();

    // Can return NULL on failure
    if(NULL == reply)
        return RESULT_ERROR_UNDEFINED;

    if((reply->size() + 1) > buff->size) {
        delete reply;
        return RESULT_BUFFER_OVERFLOW;
    }
    memset(buff->data, 0, buff->size);
    strncpy(buff->data, String8(*reply).string(), buff->size);
    delete reply;
#ifdef DEBUG
    ALOGD("Language ID %s", buff->data);
#endif
    return RESULT_SUCCESS;
}

// Devinfo extension part
int pal_system_ext_get(data_buffer_t *buff){
    AutoMutex _l(OmadmServiceAPI::mPalLock);

    if(!OmadmServiceAPI::isPalInitialized())
        return RESULT_ERROR_PAL_NO_INIT;

    if((NULL == buff)||(NULL == buff->data)||
            (0 == buff->size))
        return RESULT_BUFFER_NOT_DEFINED;

    String16* reply = NULL;
    reply = OmadmServiceAPI::getExt();

    // Can return NULL on failure
    if(NULL == reply)
        return RESULT_ERROR_UNDEFINED;

    if((reply->size() + 1) > buff->size) {
        delete reply;
        return RESULT_BUFFER_OVERFLOW;
    }
    memset(buff->data, 0, buff->size);
    strncpy(buff->data, String8(*reply).string(), buff->size);
    delete reply;
#ifdef DEBUG
    ALOGD("Ext %s", buff->data);
#endif
    return RESULT_SUCCESS;
}

int pal_system_iccid_get(data_buffer_t *buff) {
    AutoMutex _l(OmadmServiceAPI::mPalLock);

    if(!OmadmServiceAPI::isPalInitialized())
        return RESULT_ERROR_PAL_NO_INIT;

    if((NULL == buff)||(NULL == buff->data)||
            (0 == buff->size))
        return RESULT_BUFFER_NOT_DEFINED;

    String16* reply = NULL;
    reply = OmadmServiceAPI::getIccid();

    // Can return NULL on failure
    if(NULL == reply)
        return RESULT_ERROR_UNDEFINED;

    if((reply->size() + 1) > buff->size) {
        delete reply;
        return RESULT_BUFFER_OVERFLOW;
    }

    memset(buff->data, 0, buff->size);
    strncpy(buff->data, String8(*reply).string(), buff->size);
    delete reply;
#ifdef DEBUG
    ALOGD("Sim serial number is %s", buff->data);
#endif
    return RESULT_SUCCESS;
}

__END_DECLS
