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

#include <common/3dnr/3dnr_hal_base.h>
//
#include <queue>

#include <mtkcam/def/common.h>
#include <utils/RefBase.h>
#include <utils/Mutex.h>
#include <utils/Condition.h>

#include <mtkcam/drv/iopipe/CamIO/IHalCamIO.h>
#include <mtkcam/utils/sys/SensorProvider.h>

//#include <eis_hal.h>    // For EisHal in FeaturePipeNode.h.
//#include <eis_type.h>  // For EIS_P1_RESULT struct.


using namespace NSCam;
using namespace android;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSPostProc;
/******************************************************************************
 *
 ******************************************************************************/
struct hal3dnrSavedFrameInfo
{
    MUINT32             CmvX;             // Keep track of CMV X.
    MUINT32             CmvY;             // Keep track of CMV Y.

    MBOOL               isCRZUsed;
    MBOOL               isIMGO;
    MBOOL               isBinning;

    hal3dnrSavedFrameInfo()
        : CmvX(0)
        , CmvY(0)
        , isCRZUsed(MFALSE)
        , isIMGO(MFALSE)
        , isBinning(MFALSE)
        {};
};

class hal3dnrBase_v2
{
public:
    virtual ~hal3dnrBase_v2(){};

    virtual MBOOL SavedFrameInfo(const NR3DHALParam &nr3dHalParam) = 0;
    virtual MBOOL handle3DNROnOffPolicy(const NR3DHALParam &nr3dHalParam) = 0;
    virtual MBOOL handleAlignVipiIMGI(const NR3DHALParam &nr3dHalParam, NR3DParam &outNr3dParam) = 0;
    virtual MBOOL configNR3D(const NR3DHALParam& nr3dHalParam, const NR3DParam &Nr3dParam) = 0;

protected:
    hal3dnrSavedFrameInfo mCurSavedFrameInfo;
    hal3dnrSavedFrameInfo mPreSavedFrameInfo;
};

class Hal3dnr
    : public hal3dnrBase, public hal3dnrBase_v2, public VirtualLightRefBase
{
    template<const unsigned int> friend class Hal3dnrObj;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    //
    static hal3dnrBase*                 getInstance();
    virtual void                        destroyInstance();
    static hal3dnrBase*                 getInstance(char const *userName, const MUINT32 sensorIdx);
    virtual void                        destroyInstance(char const *userName, const MUINT32 sensorIdx);

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
    // halFDBase () -
    //! \brief 3dnr Hal constructor
    //
    /////////////////////////////////////////////////////////////////////////
    Hal3dnr(const MUINT32 sensorIdx);

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
    virtual MBOOL                       init(MINT32 force3DNR = 0);

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
     * @brief setCMVMode
     *
     * @details
     * - set 3DNR GMV/CMV mode
     * @note
     *
     * @return
     * - NONE
     */
    virtual MVOID                       setCMVMode(MBOOL useCMV);

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
    virtual MBOOL                       get3dnrParams(MUINT32 frameNo, MUINT32 imgiW, MUINT32 imgiH, NR3DParam* &pNr3dParam);//deprecated
    virtual MBOOL                       get3dnrParams(MUINT32 frameNo, MUINT32 imgiW, MUINT32 imgiH, NR3DParam &nr3dParam);

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

      /**
     * @brief Wrap several APIs into one compact API
     *
     * @details
     * - prepare all necessary parameters and set NR3D register into NR3D HW by ISP
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure.
     */
     virtual MBOOL                      do3dnrFlow(
                                            void *pTuningData,
                                            MBOOL useCMV,
                                            MRect const &dst_resizer_rect,
                                            NR3DGMVInfo const &GMVInfo,
                                            NSCam::IImageBuffer *pIMGBufferVIPI,
                                            MINT32 iso,
                                            MUINT32 requestNo,
                                            NS3Av3::IHal3A* p3A);

     /**
    * @brief Wrap several APIs into one compact API V2
    *
    * @details
    * - prepare all necessary parameters and set NR3D register into NR3D HW by ISP
    * @note
    *
    * @return
    * - MTRUE indicates success;
    * - MFALSE indicates failure.
    */
    virtual MBOOL                       do3dnrFlow_v2(const NR3DHALParam &nr3dHalParam);
    virtual MBOOL                       SavedFrameInfo(const NR3DHALParam &nr3dHalParam);
    virtual MBOOL                       handle3DNROnOffPolicy(const NR3DHALParam &nr3dHalParam);
    virtual MBOOL                       handleAlignVipiIMGI(const NR3DHALParam &nr3dHalParam, NR3DParam &outNr3dParam);
    virtual MBOOL                       configNR3D(const NR3DHALParam& nr3dHalParam, const NR3DParam &Nr3dParam);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Variables.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MINT32                              mLogLevel;
    MINT32                              mForce3DNR;// hal force support 3DNR
    MBOOL                               mSupportZoom3DNR;
    MSize                               mMaxInputImageSize;// TODO: deprecated
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

    NR3DParam                           *mpNr3dParam;  // For NR3D struct in PostProc.
    const MUINT32                       mSensorIdx;
    volatile MINT32                     mUsers;
    mutable Mutex                       mLock;

    MBOOL                               mIsCMVMode;
    android::sp<NSCam::Utils::SensorProvider> mpSensorProvider;

    // adjInput: NR3DCustom::AdjustmentInput*.
    // We don't include the custom header here
    MBOOL fillGyroForAdjustment(void *adjInput);
};

template<const unsigned int sensorIdx>
class Hal3dnrObj : public Hal3dnr
{
private:
    static Hal3dnrObj<sensorIdx>* spInstance;
    static Mutex    s_instMutex;

    Hal3dnrObj() : Hal3dnr(sensorIdx) {}
    ~Hal3dnrObj() {}
public:
    static Hal3dnr *GetInstance(char const *userName)
    {
        (void)userName; // Unused

        AutoMutex lock(s_instMutex);
        if (NULL == spInstance)
        {
            spInstance = new Hal3dnrObj();
        }

        return spInstance;
    }

    static void destroyInstance(char const *userName)
    {
        (void)userName; // Unused

        AutoMutex lock(s_instMutex);
        if (NULL != spInstance)
        {
            delete spInstance;
            spInstance = NULL;
        }
    }

};

#endif  //_3DNR_HAL_H_
