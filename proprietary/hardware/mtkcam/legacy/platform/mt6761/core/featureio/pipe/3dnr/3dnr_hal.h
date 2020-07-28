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

#ifndef _3DNR_HAL_H_
#define _3DNR_HAL_H_

#include <featureio/3dnr_hal_base.h>
//
#include <queue>

#include <mtkcam/common.h>
#include <utils/RefBase.h>
#include <mtkcam/iopipe/PostProc/IFeatureStream.h>
#include <mtkcam/featureio/eis_hal.h>    // For EisHal in FeaturePipeNode.h.
#include <mtkcam/featureio/eis_type.h>  // For EIS_P1_RESULT struct.


using namespace NSCam;
using namespace android;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSPostProc;
/******************************************************************************
 *
 ******************************************************************************/


class Hal3dnr
    : public hal3dnrBase, public VirtualLightRefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    //
    static hal3dnrBase*                 getInstance();
    virtual void                        destroyInstance();

    //
    /////////////////////////////////////////////////////////////////////////
    //
    // halFDBase () -
    //! \brief 3dnr Hal constructor
    //
    /////////////////////////////////////////////////////////////////////////
    Hal3dnr();

    /////////////////////////////////////////////////////////////////////////
    //
    // ~mhalCamBase () -
    //! \brief mhal cam base descontrustor
    //
    /////////////////////////////////////////////////////////////////////////
    virtual                             ~Hal3dnr();

    /**
     * @brief init the 3ndr hal
     *
     * @details
     * - Prepare all 3dnr variable and set to init status.
     * - Create NR3D struct object.
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    virtual MBOOL                       init();

    /**
     * @brief uninit the 3ndr hal
     *
     * @details
     * - Set all variable value to init status.
     * - Delete NR3D object.
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    virtual MBOOL                       uninit();

    /**
     * @brief prepare the 3ndr hal
     *
     * @details
     * - Do 3DNR State Machine operation.
     * - Reset m3dnrErrorStatus.
     * - 3DNR HW module on/off according ISO value.
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    virtual MBOOL                       prepare(MUINT32 frameNo, MINT32 iso);

    /**
     * @brief setGMV to 3dnr hal
     *
     * @details
     * - 3DNR GMV Calculation according to input parameters that may from pipeline metadata.
     * - Check GMV value is valid to do 3dnr to not.
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    virtual MBOOL                       setGMV(MUINT32 frameNo, MINT32 gmvX, MINT32 gmvY, MINT32 cmvX_Int, MINT32 cmvY_Int);

    /**
     * @brief compare IMG3O size with previous frame
     *
     * @details
     * - Check IMG3O buffer size with previous frame buffer.
     * - Update State Machine when size is different.
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    virtual MBOOL                       checkIMG3OSize(MUINT32 frameNo, MUINT32 imgiW, MUINT32 imgiH);

    /**
     * @brief Check can config vipi or not and set all related parameters to Nr3dParam
     *
     * @details
     * - Check VIPI buffer is exist according to input parameter isVIPIIn.
     * - Calculate VIPI start addr offset.
     * - Calculate VIPI valid region w/h.
     * - Check State Machine and set the right Nr3dParam.
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    virtual MBOOL                       setVipiParams(MBOOL isVIPIIn, MUINT32 vipiW, MUINT32 vipiH, MINT imgFormat, size_t stride);

    /**
     * @brief Get 3dnr parametes
     *
     * @details
     * - Get Nr3dParam from 3dnr hal.
     * - Save current image size for next frame compared.
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    virtual MBOOL                       get3dnrParams(MUINT32 frameNo, MUINT32 imgiW, MUINT32 imgiH, NR3D* &pNr3dParam);

    /**
     * @brief Check 3dnr hal State Machine is equal to input status parameter or not
     *
     * @details
     * - As function name, check 3dnr hal State Machine is equal to input status parameter or not.
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    virtual MBOOL                       checkStateMachine(NR3D_STATE_ENUM status);



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Variables.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    IFeatureStream*                     mpIFeatureStream;
    MSize                               mMaxInputImageSize;
    // queue<QParams>                      mqJob;
    // queue<FeaturePipeNodeImgReqJob_s>   m3dnrPrvFrmQueue;

    MUINT32                             mPrevFrameWidth;       // Keep track of previous frame width.
    MUINT32                             mPrevFrameHeight;      // Keep track of previous frame height.
    MINT32                              mNmvX;
    MINT32                              mNmvY;
    MUINT32                             mCmvX;                 // Current frame CMV X.
    MUINT32                             mCmvY;                 // Current frame CMV Y.
    MUINT32                             mPrevCmvX;             // Keep track of previous CMV X.
    MUINT32                             mPrevCmvY;             // Keep track of previous CMV Y.
    MINT32                              m3dnrGainZeroCount;
    MUINT32                             m3dnrErrorStatus;
    NR3D_STATE_ENUM                     m3dnrStateMachine;

    NSCam::NSIoPipe::NSPostProc::NR3D    *mpNr3dParam;  // For NR3D struct in PostProc.
};


#endif  //_3DNR_HAL_H_