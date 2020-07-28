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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "isp_tuning_buf"

#include <mtkcam/utils/std/Trace.h>
#include <sys/stat.h>
#include "property_utils.h"
#include <string.h>

#include <isp_tuning_buf.h>
#include <isp_tuning.h>
#include <mtkcam/utils/std/Log.h>
#include <aaa_types.h>

#include <mtkcam/utils/sys/IFileCache.h>
//tuning utils
#include <camera_custom_nvram.h>
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>
#include <imageio/Cam_Notify_datatype.h>

using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSCam::TuningUtils;

IspTuningBufCtrl*
IspTuningBufCtrl::
getInstance(MINT32 i4SensorDevId)
{
    switch (i4SensorDevId)
    {
        case ESensorDev_Main:
            return IspTuningBufCtrlDev<ESensorDev_Main>::getInstance();
        case ESensorDev_Sub:
            return IspTuningBufCtrlDev<ESensorDev_Sub>::getInstance();
        case ESensorDev_MainSecond:
            return IspTuningBufCtrlDev<ESensorDev_MainSecond>::getInstance();
        case ESensorDev_SubSecond:
            return IspTuningBufCtrlDev<ESensorDev_SubSecond>::getInstance();
        case ESensorDev_MainThird:
            return IspTuningBufCtrlDev<ESensorDev_MainThird>::getInstance();
        default:
            ALOGE("Unsupport sensor device ID: %d\n", i4SensorDevId);
            return MNULL;
    }
}

MVOID*
IspTuningBufCtrl::
getP1Buffer()
{
    return &m_P1TuningBuffer;
}

MUINT32
IspTuningBufCtrl::
getP1BufferSize()
{
    return sizeof(m_P1TuningBuffer);
}

MVOID
IspTuningBufCtrl::
clearP1Buffer()
{
    memset(&m_P1TuningBuffer, 0, P1_BUFFER_SIZE*4);
}

MVOID
IspTuningBufCtrl::
dumpP1CbBuffer(MVOID *pInput, MVOID *pOutput, MINT32 i4SensorDev){
    std::lock_guard<std::mutex> lock(m_Lock);
    CQ_DUMP_INFO *pReg_in = (CQ_DUMP_INFO*)pInput;

    ALOGD("[dumpP1CbBuffer] receive call back (%d)", pReg_in->magic);
    BufferInfo_T rBufferinfo;
    memset(&rBufferinfo, 0, sizeof(BufferInfo_T));
    rBufferinfo.i4MagicNum = pReg_in->magic;
    rBufferinfo.i4Size= sizeof(rBufferinfo.p1Buf);
    memcpy(&(rBufferinfo.p1Buf), pReg_in->pReg, sizeof(rBufferinfo.p1Buf));
    mBufferInfoQ.push_back(rBufferinfo);

    //if Queue size is full, erase begin(oldest)
    if (mBufferInfoQ.size() > m_u4Capacity){
        std::list<BufferInfo_T>::iterator it = mBufferInfoQ.begin();
        mBufferInfoQ.erase(it);
    }
}

MVOID
IspTuningBufCtrl::
updateHint(MVOID *hint, MINT32 i4MagicNum){
    std::lock_guard<std::mutex> lock(m_Lock);

    MINT32 i4Size = mBufferInfoQ.size();
    if (i4Size == 0)
        return;
    MINT32 i4Ret = -1;
    MINT32 i4Pos = 0;

    std::list<BufferInfo_T>::iterator it = mBufferInfoQ.begin();
    for (; it != mBufferInfoQ.end(); it++, i4Pos++)
    {
        if (it->i4MagicNum < i4MagicNum){
            mBufferInfoQ.erase(it);
        }
        else if (it->i4MagicNum == i4MagicNum)
        {
            NSCam::TuningUtils::FILE_DUMP_NAMING_HINT   rHint;
            memcpy(&rHint, hint, sizeof(FILE_DUMP_NAMING_HINT));
            char strlscdump[512] = {'\0'};
            genFileName_Reg(strlscdump, sizeof(strlscdump), &rHint, "P1CB");
            //call dump util
            NSCam::TuningUtils::FileReadRule tuningfilep1;
            tuningfilep1.DumpP1ForDP(strlscdump, (const char*)&(it->p1Buf), it->i4Size);
            mBufferInfoQ.erase(it);
            break;
        }
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IspTuningBufCtrl::IspTuningBufCtrl()
    : m_u4Capacity(10)
    , m_Lock()
{
    memset(&m_P1TuningBuffer, 0, P1_BUFFER_SIZE*4);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IspTuningBufCtrl::~IspTuningBufCtrl()
{}
