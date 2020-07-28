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

#ifndef _FSC_HAL_IMP_H_
#define _FSC_HAL_IMP_H_

#include <utils/Mutex.h>
#include <utils/Condition.h>
#include <mtkcam/feature/fsc/fsc_hal.h>

using namespace NS3Av3;
class MTKFSC;
/*******************************************************************************
*
********************************************************************************/



/*******************************************************************************
*
********************************************************************************/

class FSCHalImp : public FSCHal, public IHal3ACb
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    FSCHalImp(MUINT32 sensorIdx);

    virtual void        doNotifyCb (
                            MINT32 _msgType,
                            MINTPTR _ext1,
                            MINTPTR _ext2,
                            MINTPTR _ext3
                        );

private:

    virtual ~FSCHalImp();

    virtual MINT32      Init(IHal3A* p3A, MSize sensorSize);
    virtual MINT32      Uninit(IHal3A* p3A);
    virtual MINT32      getAFInitParams(IHal3A* p3A);
    virtual MINT32      processResult(
                            MBOOL isBinEn,
                            IMetadata* pInAPP,//inAPP
                            IMetadata* pInHAL,//inHAL
                            MetaSet_T & result3A,
                            IHal3A* p3A,
                            MINT32 const currMagicNum,
                            QBufInfo const &deqBuf,
                            MUINT32 const bufIdxRss,
                            MUINT32 const bufIdxRrz,
                            MUINT32 const index,
                            IMetadata &rOutputFSC
                        );


    // common
    const MUINT32       mSensorIdx;
    MINT32              mDebugLevel;
    // AF
    MBOOL               mFirstFrame;
    FSC_DAC_INIT_INFO_T mDacInit;
    FSC_FRM_INFO_T      mLastFSCFrameInfo;
    MINT32              mlastTargetDAC;
    // FSC provider
    MTKFSC              *mpFSCProvider;
    MSize               mSensorSize;
    mutable Mutex       m_lock;
    Condition           m_condition;

    // TODO add fsc provider
};
#endif
