/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein is
* confidential and proprietary to MediaTek Inc. and/or its licensors. Without
* the prior written permission of MediaTek inc. and/or its licensors, any
* reproduction, modification, use or disclosure of MediaTek Software, and
* information contained herein, in whole or in part, shall be strictly
* prohibited.
*
* MediaTek Inc. (C) 2010. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
* ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
* WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
* NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
* RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
* INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
* TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
* RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
* OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
* SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
* RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
* ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
* RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
* MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
* CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* The following software/firmware and/or related documentation ("MediaTek
* Software") have been modified by MediaTek Inc. All revisions are subject to
* any receiver's applicable license agreements with MediaTek Inc.
*/
#define LOG_TAG "DebugUtil"
#include <debug/DebugUtil.h>
#include <mtkcam/utils/std/Log.h>
#include <isp_tuning.h>
#include <cutils/properties.h>

#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_3A_HAL(String) \
          do { \
              aee_system_exception( \
                  "DebugUtil", \
                  NULL, \
                  DB_OPT_DEFAULT, \
                  String); \
          } while(0)
#else
#define AEE_ASSERT_3A_HAL(String)
#endif

#define INIT_DROPPED_FRAME 5

DebugUtil::
DebugUtil(int i4SensorDevId)
    : m_i4SensorDevId(i4SensorDevId)
    , m_i4detectAECnt(0)
    , m_i4detectAFCnt(0)
    , m_bEnableDetect(1)
    , m_i4ResumeDetectCnt(0)
{
}

DebugUtil*
DebugUtil::
getInstance(int i4SensorDevId)
{
    switch (i4SensorDevId)
    {
        case NSIspTuning::ESensorDev_Main:
            return DebugUtilDev<NSIspTuning::ESensorDev_Main>::getInstance();
        case NSIspTuning::ESensorDev_Sub:
            return DebugUtilDev<NSIspTuning::ESensorDev_Sub>::getInstance();
        case NSIspTuning::ESensorDev_MainSecond:
            return DebugUtilDev<NSIspTuning::ESensorDev_MainSecond>::getInstance();
        case NSIspTuning::ESensorDev_SubSecond:
            return DebugUtilDev<NSIspTuning::ESensorDev_SubSecond>::getInstance();
        case NSIspTuning::ESensorDev_MainThird:
            return DebugUtilDev<NSIspTuning::ESensorDev_MainThird>::getInstance();
        default:
            CAM_LOGW("Unsupport sensor device ID: %d\n", i4SensorDevId);
            return NULL;
    }
}

void
DebugUtil::
update(std::string moduleName, std::string tag, int value)
{
    int enable = ::property_get_int32("vendor.debug.3a.update", 0);
    if(!enable)
        return;

    std::lock_guard<std::mutex> lock(m_Lock);
    std::map<std::string,IDebugEntry>::iterator it;

    it = mMap.find(moduleName);
    if(it == mMap.end()){
        IDebugEntry pEntry;
        pEntry.update(tag,value);
        mMap[moduleName] = pEntry;
    } else
        it->second.update(tag,value);
}

void
DebugUtil::
dump()
{
    std::lock_guard<std::mutex> lock(m_Lock);
    std::map<std::string,IDebugEntry>::iterator it;
    for(it = mMap.begin(); it != mMap.end(); ++it){
        CAM_LOGW(" (%d) module : %s +", m_i4SensorDevId, it->first.c_str());
        it->second.dump();
        CAM_LOGW(" (%d) module : %s -", m_i4SensorDevId, it->first.c_str());
    }
}

void
DebugUtil::
clear()
{
    std::lock_guard<std::mutex> lock(m_Lock);
    if(!mMap.empty())
        mMap.clear();
}

void
DebugUtil::
detectPreframe(DEBUG_MODULE module, int i4CurFrame)
{
    int enable = ::property_get_int32("vendor.debug.perframe3a.detect", 0);
    if(!enable)
        return;

    std::lock_guard<std::mutex> lock(m_Lock);

    if(!m_bEnableDetect)
    {
        CAM_LOGW("[%s] Pause Detect", __FUNCTION__);
        return;
    } else if(m_i4ResumeDetectCnt > 0)
    {
        CAM_LOGW("[%s] Pause Detect(%d)", __FUNCTION__, m_i4ResumeDetectCnt);
        m_i4ResumeDetectCnt--;
        return;
    }
    switch(module)
    {
        case DBG_AE:
            CAM_LOGW("[%s] AE i4CurFrame(%d), m_i4detectAECnt(%d)", __FUNCTION__, i4CurFrame,m_i4detectAECnt);
            if(m_i4detectAECnt > INIT_DROPPED_FRAME && (i4CurFrame - m_i4detectAECnt) != 1)
                AEE_ASSERT_3A_HAL("pre-frame AE fail!");
            m_i4detectAECnt = i4CurFrame;
            break;
        case DBG_AF:
            CAM_LOGW("[%s] AF i4CurFrame(%d), m_i4detectAFCnt(%d)", __FUNCTION__, i4CurFrame,m_i4detectAFCnt);
            if(m_i4detectAFCnt > INIT_DROPPED_FRAME  && (i4CurFrame - m_i4detectAFCnt) != 1)
                AEE_ASSERT_3A_HAL("pre-frame AF fail!");
            m_i4detectAFCnt = i4CurFrame;
            break;
        default:
            break;
    }
}

void
DebugUtil::
pauseDetect()
{
    m_bEnableDetect = 0;
    m_i4ResumeDetectCnt = INIT_DROPPED_FRAME;
    resetDetect(DBG_AE);
    resetDetect(DBG_AF);
}

void
DebugUtil::
resumeDetect()
{
    m_bEnableDetect = 1;
}
void
DebugUtil::
resetDetect(DEBUG_MODULE module)
{
    switch(module)
    {
        case DBG_AE:
            m_i4detectAECnt = 0;
            break;
        case DBG_AF:
            m_i4detectAFCnt = 0;
            break;
        default:
            break;
    }

}

int
DebugUtil::
getDebugLevel(DEBUG_MODULE module)
{
    switch(module)
    {
        case DBG_3A:
            return ::property_get_int32("vendor.debug.3A_level.enable", 0);
        case DBG_FLASH:
            return ::property_get_int32("vendor.debug.flash_dump.enable", 0);
        default:
            return 0;
    }
}
