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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#ifndef _MTK_HARDWARE_MTKCAM_UTILS_HW_PLUGPROCESSING4CELL_H_
#define _MTK_HARDWARE_MTKCAM_UTILS_HW_PLUGPROCESSING4CELL_H_
//
#include "PlugProcessingBase.h"
#include "PlugProcessingUtils.h"
#include <utils/KeyedVector.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {

#ifdef PLUGPROCESSING_CLASSNAME
#undef PLUGPROCESSING_CLASSNAME
#endif
#define PLUGPROCESSING_CLASSNAME   PlugProcessingFCell

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Plug Processing.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class PLUGPROCESSING_CLASSNAME
    : public PlugProcessingBase
{
public:
                            PLUGPROCESSING_CLASSNAME(MUINT32 id);
    virtual                 ~PLUGPROCESSING_CLASSNAME();


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  function member. (need to implement)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    MERROR                  initImp(void);

    MERROR                  uninitImp(void);

    MERROR                  sendCommandImp(
                                __attribute__((unused)) MINT32 const cmd,
                                __attribute__((unused)) MINTPTR arg1 = 0,
                                __attribute__((unused)) MINTPTR arg2 = 0,
                                __attribute__((unused)) MINTPTR arg3 = 0,
                                __attribute__((unused)) MINTPTR arg4 = 0);
private:

    //void                    dump();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  data member. (if need)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    // funtion point
    typedef bool (*pfnInit_T)
            (MINT32 id, NSCam::PlugInitParam initParam);

    typedef bool (*pfnSendCommand_T)
            (MINT32 id, MINT32 const cmd,
             MINTPTR arg1, MINTPTR arg2,
             MINTPTR arg3, MINTPTR arg4);

    typedef void (*pfnDeinit_T)(MINT32 id);

    // add data member here
    pfnInit_T           mpInitFun;
    pfnSendCommand_T    mpSendCommandFuc;
    pfnDeinit_T         mpDeinitFun;
    //
struct bufferInfo{
    size_t bufSize;
    char* bufAddr;
};

    MINT32              mDeviceId;
    MUINT32             muEEPROMSize;
    android::KeyedVector< MINT32, bufferInfo > mvWorkingBuffer;
    PlugInitParam       mInitParam;
    PlugProcessingParam mProcessingParam;
    // Debug: Use OR operation to combine the data imported from sdcard: sdcard/..
    // [1]: tuning tbl, ex. s5k3p8sp, ov20880, imx376...
    // [2]: dpc data, ex. ov20880, imx376...
    // [4]: fd dpc data, ex. imx376... 
    MINT32              mDebugQuerySensorData;

};

};  //namespace NSCam

#endif//_MTK_HARDWARE_MTKCAM_UTILS_HW_PLUGPROCESSING4CELL_H_

