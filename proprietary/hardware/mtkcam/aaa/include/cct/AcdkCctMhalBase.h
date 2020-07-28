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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkMhalBase.h


#ifndef _ACDKCCTMHALBASE_H_
#define _ACDKCCTMHALBASE_H_

#include <mtkcam/def/common.h>
#include <mtkcam/drv/IHalSensor.h>
#include <mtkcam/aaa/cct/common.h>
#include "AcdkCallback.h"

#include <imem_3a.h>

using NSCam::IHalSensor;

namespace NSAcdkCctMhal
{
    /**
         *@enum acdkCctMhalState_enum
         *@brief State enum of AcdkMhal
       */
    typedef enum acdkCctMhalState_enum
    {
        ACDK_CCT_MHAL_NONE           = 0x0000,
        ACDK_CCT_MHAL_INIT           = 0x0001,
        ACDK_CCT_MHAL_IDLE           = 0x0002,
        //
        ACDK_CCT_MHAL_PREVIEW_MASK   = 0x0100,
        ACDK_CCT_MHAL_PREVIEW        = 0x0100,
        //
        ACDK_CCT_MHAL_CAPTURE_MASK   = 0x0200,
        ACDK_CCT_MHAL_PRE_CAPTURE    = 0x0200,
        ACDK_CCT_MHAL_CAPTURE        = 0x0201,
        //
        ACDK_CCT_MHAL_PREVIEW_STOP   = 0x0400,
        //
        ACDK_CCT_MHAL_UNINIT         = 0x0800,
        //
        ACDK_CCT_MHAL_ERROR          = 0x8000
    } acdkCctMhalState_e;

    /**
         *@struct acdkCctMhalCamFrame_s
         *@brief  Struct for paramter of prview frame
       */
    typedef struct acdkCctMhalCamFrame_s
    {
        MUINT32 w;
        MUINT32 h;
        MUINT32 orientation;
        MBOOL flip;
        MUINT32 stride[3];
        MUINT32 frmFormat;
    } acdkCctMhalCamFrame_t;

    /**
         *@struct acdkCctMhalPrvParam_s
         *@brief  Struct for preview HW scenario parameter
       */
    typedef struct acdkCctMhalPrvParam_s
    {
        MUINT32 scenarioHW;//NSImageio::NSIspio::EScenarioID scenarioHW;
        MUINT32 sensorID;//halSensorDev_e sensorID;
        MUINT32 sensorIndex;//halSensorDev_e sensorID;
        IHalSensor::ConfigParam sensorConfigParam[1];
        MUINT32 sensorWidth;
        MUINT32 sensorHeight;
        MUINT32 sensorFullWidth;
        MUINT32 sensorFullHeight;
        MUINT32 rrzoWidth;
        MUINT32 sensorStride[3];
        MUINT32 rrzoStride[3];
        MUINT32 sensorType;
        MUINT32 sensorFormat;
        MUINT32 sensorColorOrder;
        MUINT32 mu4SensorDelay;
        IMEM_BUF_INFO *imgImemBuf;
        IMEM_BUF_INFO *rrzoImemBuf;
        IMEM_BUF_INFO *dispImemBuf;
        acdkCctMhalCamFrame_t frmParam;
        acdkObserver acdkMainObserver;
        MBOOL IsFactoryMode;
        MUINT32 mPxlMode;//
        MUINT16 mHDR_EN;
        MUINT32 PreviewSensorScenario;
    }acdkCctMhalPrvParam_t;
    /**
         *@struct acdkCctMhalCapParam_s
         *@brief  Struct for Capture HW scenario parameter
       */
    typedef struct acdkCctMhalCapParam_s
    {
        MUINT32 sensorID;//halSensorDev_e sensorID;
        MUINT32 sensorIndex;//halSensorDev_e sensorID;
        MUINT32 CaptureMode; //scenario ID
        MUINT32 mCapType;
        MUINT32 mCapWidth;
        MUINT32 mCapHeight;
        MUINT32 sensorFullWidth;
        MUINT32 sensorFullHeight;
        MUINT32 mMFLL_EN;
        MUINT16 mHDR_EN;
        MUINT32 mEMultiNR;
        acdkObserver acdkMainObserver;
    }acdkCctMhalCapParam_t;
    /**
         *@class AcdkCctMhalBase
         *@brief This class handle opeation with ISP
       */
    class AcdkCctMhalBase
    {
    public:
            /**
                       *@brief AcdkCctMhalBase constructor
                     */
            AcdkCctMhalBase() {};

            /**
                       *@brief AcdkCctMhalBase destructor
                     */
            virtual ~AcdkCctMhalBase() {};

            /**
                       *@brief Create AcdkCctMhalBase Object
                       *@note Actually, it will create AcdkMhalPure or AcdkMhalEng object
                     */
            static AcdkCctMhalBase *createInstance(MUINT32 OpenMode=0);

            /**
                       *@brief Destory AcdkCctMhalBase Object
                     */
            virtual void destroyInstance() = 0;

            /**
                       *@brief Set current sate of AcdkCctMhalBase to newState
                       *
                       *@param[in] newState : new state
                     */
            virtual MVOID acdkCctMhalSetState(acdkCctMhalState_e newState) = 0;

            /**
                       *@brief Get current sate of AcdkCctMhalBase
                     */
            virtual acdkCctMhalState_e acdkCctMhalGetState() = 0;

            /**
                       *@brief Indicates whether is ready for capture or not
                       *@return
                       *-MFALSE indicates not ready, MTRUE indicates ready
                     */
            virtual MBOOL acdkCctMhalReadyForCap() = 0;

            /**
                       *@brief Initialize function
                       *@note Must call this function right after createInstance and before other functions
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkCctMhalInit() = 0;

            /**
                       *@brief Uninitialize function
                       *@note Must call this function before destroyInstance
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkCctMhalUninit() = 0;

            /**
                       *@brief Handle callback
                       *
                       *@param[in] a_type : callback type
                       *@param[in] a_addr1 : return data address
                       *@param[in] a_addr2 : return data address
                       *@param[in] a_dataSize : return data size
                     */
            virtual MVOID  acdkCctMhalCBHandle(MUINT32 a_type, MUINT8* a_addr1, MUINT8* a_addr2 = 0, MUINT32 const a_dataSize = 0) = 0;

            /**
                       *@brief Start preview
                       *@note Config and control ISP to start preview
                       *
                       *@param[in] a_pBuffIn : pointer to acdkCctMhalPrvParam_t data
                       *
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkCctMhalPreviewStart(MVOID *a_pBuffIn) = 0;

            /**
                       *@brief Stop preview
                       *@note Config and control ISP to stop preview
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkCctMhalPreviewStop() = 0;

            /**
                       *@brief Start capture
                       *
                       *@param[in] a_pBuffIn
                       *
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkCctMhalCaptureStart(MVOID *a_pBuffIn) = 0;

            /**
                       *@brief Stop capture
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkCctMhalCaptureStop() = 0;

            /**
                       *@brief Execute preview process
                       *@note Here is a preview loop
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkCctMhalPreviewProc() = 0;

            /**
                       *@brief Change state to preCapture state and do related opertion
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkCctMhalPreCapture() = 0;

            /**
                       *@brief  Execute capture process
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MINT32 acdkCctMhalCaptureProc() = 0;

            /**
                       *@brief  Get shutter time in us
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MUINT32 acdkCctMhalGetShutTime() = 0;

            /**
                       *@brief  Set shutter time
                       *
                       *@param[in] a_time : specific shutter time in us
                       *
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MVOID acdkCctMhalSetShutTime(MUINT32 a_time) = 0;
            /**
                       *@brief  Get AF information
                       *@return
                       *-0 indicates success, otherwise indicates fail
                     */
            virtual MUINT32 acdkCctMhalGetAFInfo() = 0;
            /**
             *@brief  Set AF mode
             *@return
              *-0 indicates success, otherwise indicates fail
               */
            virtual MUINT32 acdkCctMhalSetAFMode(MBOOL isAutoMode) = 0;

    };
};
#endif //end AcdkCctMhalBase.h

