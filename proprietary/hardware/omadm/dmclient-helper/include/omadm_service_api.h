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

#ifndef __OMADM_SERVICE_API__
#define __OMADM_SERVICE_API__ 1

#include <utils/Mutex.h>
#include <utils/KeyedVector.h>
#include <utils/Log.h>
#include <utils/String16.h>

#include "pal.h"
#include "pal_event_handler.h"
#include "service_listener.h"

#define NO_ERR       0
#define ERR_ARGS     1
#define ERR_SYSCALL  2
#define ERR_RETURN   3
#define ERR_INIT     4
#define ERR_NOMEM    5
#define ERR_ADD_CLBK 6
#define ERR_DEL_CLBK 7
#define ERR_GET_CLBK 8

using namespace android;

#define SERVICE_NAME "omadm_hidl_service"

enum OMADMHIDL_STRINGINFO_API_ENUM{
    HIDL_GET_DEVICE_ID,
    // Interface for pal_network_apn_class* functions
    HIDL_NW_APN_GET_ID,
    HIDL_NW_APN_GET_NAME,
    HIDL_NW_APN_GET_IP,
    
    // Interface for pal_*_ims_* functions
    HIDL_NETWORK_IMS_DOMAIN,
    HIDL_NETWORK_IMS_SMS_FORMAT_GET,

    //Interface for pal_devDetail functions
    HIDL_SYSTEM_OEM_GET,
    HIDL_SYSTEM_FWV_GET,
    HIDL_SYSTEM_SWV_GET,
    HIDL_SYSTEM_HWV_GET,
    HIDL_SYSTEM_SUPPORT_LRGOBJ_GET,
    HIDL_SYSTEM_DATE_GET,
    HIDL_SYSTEM_TIMEUTC_GET,
    HIDL_SYSTEM_HOSTDEVICE_MANU_GET,
    HIDL_SYSTEM_HOSTDEVICE_MODEL_GET,
    HIDL_SYSTEM_HOSTDEVICE_SWV_GET,
    HIDL_SYSTEM_HOSTDEVICE_FWV_GET,
    HIDL_SYSTEM_HOSTDEVICE_HWV_GET,
    HIDL_SYSTEM_HOSTDEVICE_DATESTAMP_GET,
    HIDL_SYSTEM_HOSTDEVICE_DEVICEID_GET,
    HIDL_SYSTEM_DEVICETYPE_GET,

    // Interface for pal_devInfo functions
    HIDL_MANUFACTURER,
    HIDL_MODEL,
    HIDL_DMV,
    HIDL_GET_LANGUAGE,
    HIDL_ICCID,
    HIDL_GET_EXT,
};

/*********************COMMON GET INT TYPE API*************************/
enum OMADMHIDL_GET_INTINFO_API_ENUM{
    HIDL_IMS_SMS_OVER_IP_IS_ENABLED,
    HIDL_IMS_SMS_FORMAT_GET,
    HIDL_DCMO_VLT_GET,
    HIDL_DCMO_LVC_GET,
    HIDL_DCMO_VWF_GET,
};

/*********************COMMON SET INT TYPE API*************************/
enum OMADMHIDL_SET_INTINFO_API_ENUM{
    HIDL_IMS_SMS_OVER_IP_ENABLE,
    HIDL_IMS_SMS_FORMAT_SET,
    HIDL_DCMO_VLT_SET,
    HIDL_DCMO_LVC_SET,
    HIDL_DCMO_VWF_SET,
    HIDL_OMADM_ISFACTORY_SET,
};

enum OMADMHIDL_APN_GET_STRINFO_API_ENUM{
    HIDL_APN_GET_NAME,
};

enum OMADMHIDL_APN_SET_STRINFO_API_ENUM{
    HIDL_APN_SET_NAME,
    HIDL_APN_RESTORE_INFO,
};

enum OMADMHIDL_APN_GET_INTINFO_API_ENUM{
    HIDL_APN_GET_IPVX,
    HIDL_APN_GET_ID,
    HIDL_APN_IS_ENABLE,
};

enum OMADMHIDL_APN_SET_INTINFO_API_ENUM{
    HIDL_APN_SET_IPVX,
    HIDL_APN_SET_ENABLE,
};

namespace omadm_service_api {
    class OmadmServiceAPI
    {
        public:
            static bool mInitialized;
            static KeyedVector<omadmCallback_id, omadmCallback> *mOmadmCallbackData;

            // Listners
            static int registerOmadmCallback(
                    omadmCallback_id omadmCallbackId, omadmCallback omadmcallback);
            static int unregisterOmadmCallback(omadmCallback_id omadmCallbackId);

            static int initBinder(bool wait_forever);
            static int isPalInitialized();
            static Mutex mPalLock;
            static Mutex mPalOmadmControllerLock;
            static bool gInitialized;

            //DevInfo
            static String16* getDeviceId();
            static String16* getManufacturer();
            static String16* getModel();
            static String16* getDmv();
            static String16* getLanguage();
            static String16* getIccid();
            static String16* getExt();

            //DevDetail
            static String16* getDeviceOEM();
            static String16* getFWV();
            static String16* getSWV();
            static String16* getHWV();
            static String16* support_lrgobj_get();
            static String16* getDate();
            static String16* getTime();
            static String16* getHostDeviceManu();
            static String16* getHostDeviceModel();
            static String16* getHostHWV();
            static String16* getHostFWV();
            static String16* getHostSWV();
            static String16* getHostDateStamp();
            static String16* getHostID();
            static String16* getTyp();
            static int getApnChrBuff(int32_t iidex, int32_t apncls, data_buffer_t *obuf);
            static int setApnChrBuff(int32_t iidex, int32_t apncls, const char*ibuf);
            static int getApnIntTypeInfo(int32_t iidex, int32_t apncls, int32_t* getInfo);
            static int setApnIntTypeInfo(int32_t iidex, int32_t apncls, int32_t setInfo);
            static int getImsDomain(data_buffer_t *obuf);
            static int getSmsImsFormat(int32_t* fmt);
            static int setSmsImsFormat(int32_t fmt);
            
            static int requestAdminNetwork(bool enable);
            static int getDcmoIntTypeInfo(int32_t iidex, int32_t* getInfo);
            static int setDcmoIntTypeInfo(int32_t iidex, int32_t setInfo);
    };

} //end of namespace omadm_service_api
#endif //end of ifndef __OMADM_SERVICE_API__
