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
#ifndef __DEBUG_UTIL_H__
#define __DEBUG_UTIL_H__

#include <debug/IDebugEntry.h>
#include <map>
#include <string>
#include <utils/Mutex.h>

typedef enum
{
    DBG_3A = 0,
    DBG_AE,
    DBG_AF,
    DBG_AWB,
    DBG_FLASH,
    DBG_FLICKER,
    DBG_SHADING,
    DBG_ISP,
    DBG_NVRAM,
    DBG_NUM
} DEBUG_MODULE;

class DebugUtil
{
public:
                                DebugUtil(int i4SensorDevId);
    virtual                     ~DebugUtil(){}

private: // disable copy constructor and copy assignment operator
    DebugUtil(const DebugUtil&);
    DebugUtil& operator=(const DebugUtil&);

public:
    static DebugUtil*            getInstance(int i4SensorDevId);
    virtual void                 update(std::string moduleName, std::string tag, int value);
    virtual void                 dump();
    virtual void                 clear();
    virtual void                 detectPreframe(DEBUG_MODULE module, int i4CurFrame);
    virtual void                 pauseDetect();
    virtual void                 resumeDetect();
    virtual void                 resetDetect(DEBUG_MODULE module);
    static int                   getDebugLevel(DEBUG_MODULE module);
    uint64_t                    TS_AFDone;

private:
    std::map<std::string, IDebugEntry> mMap;
    mutable android::Mutex             m_Lock;
    int                                m_i4SensorDevId;
    int                                m_i4detectAECnt;
    int                                m_i4detectAFCnt;
    int                                m_bEnableDetect;
    int                                m_i4ResumeDetectCnt;
};

#endif //__DEBUG_UTIL_H__
