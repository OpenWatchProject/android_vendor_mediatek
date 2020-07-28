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
#include <stdint.h>
#include <sys/types.h>
#include <utils/Log.h>
#include <cutils/properties.h>
#include <binder/BinderService.h>
#include <binder/IServiceManager.h>
#include "TerService.h"
#include "libterservice.h"

// especially for c functions
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>

namespace android
{

#define PROPERTY_TER_ENABLE "vendor.ter.service.enable"
#define PROPERTY_TER_ENABLE_CURRENT "persist.vendor.ter.service.current"

//static pthread_key_t sigbuskey;

//int TerService::instantiate()
//{
//    RLOGI("[TerService] TerService instantiate");
//
//    int r = defaultServiceManager()->addService(
//                String16("terservice"), new TerService());
//    RLOGI("[TerService] TerService r = %d/n", r);
//    return r;
//}

TerService::TerService()
{
    int enable = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };

    RLOGI("[TerService] TerService created");
//    pthread_key_create(&sigbuskey, NULL);

    memset(property_value, 0, sizeof(property_value));
    property_get(PROPERTY_TER_ENABLE, property_value, "0");
    enable = atoi(property_value);
    bServiceEnableDefault = (enable == 1 ? true : false);
    RLOGD("[TerService] init bServiceEnableDefault (enable=%d) from property %s=%s", enable, PROPERTY_TER_ENABLE, property_value);
}
TerService::~TerService()
{
//    pthread_key_delete(sigbuskey);
    RLOGI("[TerService] TerService destroyed");
}

bool TerService::isEarlyReadServiceEnabled()
{
    int current = 0;
    char property_value[PROPERTY_VALUE_MAX] = { 0 };

    RLOGI("[TerService] isEarlyReadServiceEnabled");

    memset(property_value, 0, sizeof(property_value));
    property_get(PROPERTY_TER_ENABLE_CURRENT, property_value, (bServiceEnableDefault ? "1" : "0"));
    current = atoi(property_value);
    RLOGI("[TerService] get %s=%s", PROPERTY_TER_ENABLE_CURRENT, property_value);

    return ( (current == 1) ? true : false );

    /*
     * MTK_TER_SUPPORT=yes
     * ter.service.enable=0/1
     *
     *
     * property_get(PROPERTY_TER_ENABLE, "1");
     *
    errcode = property_set(PROPERTY_TER_ENABLE, "1");
    memset(property_value, 0, sizeof(property_value));
    property_get(PROPERTY_TER_ENABLE, property_value, "0");
    LOGD("ril.fd.mode=%s, errcode=%d", property_value, errcode);
    */

//    return false;
    return true;
}

void TerService::setEarlyReadServiceEnable(bool onoff)
{
    RLOGI("[TerService] setEarlyReadServiceEnable onoff=%s", (onoff ? "true" : "false" ));

    int errcode = -1;
    errcode = property_set(PROPERTY_TER_ENABLE_CURRENT, (onoff ? "1" : "0" ));
    RLOGI("[TerService] set %s (%s), errcode=%d", PROPERTY_TER_ENABLE_CURRENT, (onoff ? "1" : "0" ), errcode);

    int retVal = -1;
    if (errcode != -1) {
        retVal = enableCcciSimType(0, onoff);
    }
    RLOGI("[TerService] set enableCcciSimType (errcode=%d, retVal=%d)", errcode, retVal);

    return;
}

bool TerService::isEarlyDataReady()
{
    RLOGI("[TerService] isEarlyDataReady");

    if (isEarlyReadServiceEnabled() == false)
        return false;

    unsigned int simType = 0xEEEEEEEE;
    int retVal = getCcciSimType(0, &simType);

    if ((retVal<0) || (simType==0xEEEEEEEE))
        return false;
    else
        return true;
}

status_t TerService::getSimMccMnc(String8* outStr)
{
    uint32_t simId = 0;

    RLOGI("[TerService] getSimMccMnc");

    return getSimMccMncGemini(simId, outStr);
}

status_t TerService::getSimMccMncGemini(uint32_t simId, String8* outStr)
{
    unsigned int simType = 0xEEEEEEEE;

    RLOGI("[TerService] getSimMccMncGemini simId=%d", simId);

    if (isEarlyReadServiceEnabled() == false)
        return INVALID_OPERATION;

//    RLOGD("##### getSimMccMncGemini sizeof(simType)=%d, (%x)", sizeof(simType), simType);
    int retVal = getCcciSimType(0, &simType);
    if (retVal<0)
    {
        RLOGI("[TerService] query mccmnc error. (retVal=%d, simType=%x)", retVal, simType);
        return UNKNOWN_ERROR;
    }

    if (simType == 0xFFFFFFFF || simType == 0xEEEEEEEE)
    {
        RLOGI("[TerService] query mccmnc no valid mccmnc yet. (retVal=%d, simType=%x)", retVal, simType);
        outStr->setTo("");
    }
    else
    {
        int nSimTypeVal[7] = {0, 0, 0, 0, 0, 0, 0};

        if (supportIccId9th()) {
            // ICCID 9th digit
            nSimTypeVal[6] = (simType & (0xF<<28)) >> 24;
            RLOGI("[TerService] ICCID 9th=%d", nSimTypeVal[6]);
        }
        // 12345f: ff 45 3f 12
        nSimTypeVal[5] = (simType & (0xF<<28)) >> 28;
        nSimTypeVal[3] = (simType & (0xF<<20)) >> 20;
        nSimTypeVal[4] = (simType & (0xF<<16)) >> 16;
        nSimTypeVal[2] = (simType & (0xF<<12)) >> 12;
        nSimTypeVal[0] = (simType & (0xF<<4)) >> 4;
        nSimTypeVal[1] = (simType & (0xF));


        RLOGI("[TerService] query mccmnc retVal=%d, simType=%x, nSimTypeVal=(%d,%d,%d,%d,%d,%d)",
                retVal, simType, nSimTypeVal[0], nSimTypeVal[1], nSimTypeVal[2], nSimTypeVal[3],
                nSimTypeVal[4], nSimTypeVal[5]);

        char* tmpStr;
        if (nSimTypeVal[5] == 0xF) {
            asprintf(&tmpStr, "%d%d%d%d%d", nSimTypeVal[0], nSimTypeVal[1], nSimTypeVal[2], nSimTypeVal[3], nSimTypeVal[4]);
        } else {
            asprintf(&tmpStr, "%d%d%d%d%d%d", nSimTypeVal[0], nSimTypeVal[1], nSimTypeVal[2], nSimTypeVal[3], nSimTypeVal[4], nSimTypeVal[5]);
        }
        char *finalStr = NULL;
        bool releaseFinal = true;
        if (supportIccId9th() && nSimTypeVal[6] != 0xF) {
            asprintf(&finalStr, "%s%d", tmpStr, nSimTypeVal[6]);
        } else {
            finalStr = tmpStr;
            releaseFinal = false;
        }
        outStr->setTo(finalStr);
        free(tmpStr);
        if (releaseFinal) {
            free(finalStr);
        }
    }

/*
    if (simId == 0)
    {
//        property_get("vendor.ter.mccmnc", value, "ab1");
        outStr->setTo("ab1-2");
    }
    else if (simId == 1)
    {
//        property_get("vendor.ter.mccmnc2", value, "ab2");
        outStr->setTo("ab2-2");
    }
    else
    {
        outStr->setTo("ab0-2");
    }
*/

    RLOGI("[TerService] getSimMccMncGemini (%s)(%zd) (simType=%d)", outStr->string(),
            outStr->bytes(), simType);

    return NO_ERROR;
}


//extern "C" {

// for c libs

/*#define CCCI_IOCTL2_PORT                "/dev/ccci_ioctl2"
#define CCCI_IOC_MAGIC                  'C'
#define CCCI_IOC_GET_SIM_TYPE           _IOR(CCCI_IOC_MAGIC, 26, unsigned int)
#define CCCI_IOC_ENABLE_GET_SIM_TYPE    _IOW(CCCI_IOC_MAGIC, 27, unsigned int)

#define GEMINI_SIM_1 (0)
#define GEMINI_SIM_2 (1)

int getIoctlSimType(int ccci_sys_fd) {
    int val_to_ret = 0;

    int open_fd_locally = 0;
    if (ccci_sys_fd == 0) {
        open_fd_locally = 1;
        ccci_sys_fd = open(CCCI_IOCTL2_PORT, O_RDWR | O_NONBLOCK);
    }

    if (ccci_sys_fd < 0) {
        RLOGD("open CCCI ioctl2 port failed [%d, %d]", ccci_sys_fd, errno);
        return -1;
    }
    unsigned int simMode = 0; // set to 0 (a invalid number). real mode range: 1~4.
    int ret_ioctl_val = ioctl(ccci_sys_fd, CCCI_IOC_GET_SIM_TYPE, &simMode);
    if (ret_ioctl_val<0)
    {
        RLOGD("CCCI ioctl result: ret_val=%d, request=%d, [err: %s]", ret_ioctl_val, CCCI_IOC_GET_SIM_TYPE, strerror(errno));
    }
    else
    {
        RLOGD("CCCI ioctl result: ret_val=%d, request=%d, simMode=%d", ret_ioctl_val, CCCI_IOC_GET_SIM_TYPE, simMode);
        val_to_ret = simMode;
    }

//    char* mode;
//    asprintf(&mode, "%d", val_to_ret);
//    property_set(PROPERTY_SIM_SWITCH_MODE, mode);
//    free(mode);

    if (open_fd_locally==1)
        close(ccci_sys_fd); // remember to close it if it is opened locally.
    return val_to_ret;
}

int getIoctlEnableSimType(int ccci_sys_fd) {
    int val_to_ret = 0;

    int open_fd_locally = 0;
    if (ccci_sys_fd == 0) {
        open_fd_locally = 1;
        ccci_sys_fd = open(CCCI_IOCTL2_PORT, O_RDWR | O_NONBLOCK);
    }

    if (ccci_sys_fd < 0) {
        RLOGD("open CCCI ioctl2 port failed [%d, %d]", ccci_sys_fd, errno);
        return -1;
    }
    unsigned int simMode = 0; // set to 0 (a invalid number). real mode range: 1~4.
    int ret_ioctl_val = ioctl(ccci_sys_fd, CCCI_IOC_ENABLE_GET_SIM_TYPE, &simMode);
    if (ret_ioctl_val<0)
    {
        RLOGD("CCCI ioctl result: ret_val=%d, request=%d, [err: %s]", ret_ioctl_val, CCCI_IOC_ENABLE_GET_SIM_TYPE, strerror(errno));
    }
    else
    {
        RLOGD("CCCI ioctl result: ret_val=%d, request=%d, simMode=%d", ret_ioctl_val, CCCI_IOC_ENABLE_GET_SIM_TYPE, simMode);
        val_to_ret = simMode;
    }

//    char* mode;
//    asprintf(&mode, "%d", val_to_ret);
//    property_set(PROPERTY_SIM_SWITCH_MODE, mode);
//    free(mode);

    if (open_fd_locally==1)
        close(ccci_sys_fd); // remember to close it if it is opened locally.
    return val_to_ret;
}*/

//} // extern "C"


}; // namespace android
