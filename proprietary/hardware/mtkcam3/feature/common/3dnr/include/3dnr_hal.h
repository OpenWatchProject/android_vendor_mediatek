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

#define HAL_3DNR_USER_NAME_LEN 128


#include <MTKTnr.h>

using namespace NSCam;
using namespace android;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSPostProc;
using namespace NSCam::NR3D;


class MTKTnr;
/******************************************************************************
 *
 ******************************************************************************/


// === Lib3dnrInfo
#define LIB_3DNR_USER_NAME_LEN 20
typedef struct Lib3dnrInfo
{
    MTKTnr                             *pTnrLib = NULL;
    MINT32                              libMode = -1;// hal force support 3DNR
    android::sp<IImageBuffer>           tnrWorkBuf = NULL;
    MINT64                              tnrLastestTS = 0;

    char                                userName[LIB_3DNR_USER_NAME_LEN] = {0};

    Lib3dnrInfo(const char *pUserName);
    virtual ~Lib3dnrInfo();

    MTKTnr* createTnrLib();
    void    cleanup();
} Lib3dnrInfo;

// === Hal3dnr ===
class Hal3dnr
    : public Hal3dnrBase, public VirtualLightRefBase
{
    template<const unsigned int> friend class Hal3dnrObj;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    //
    static Hal3dnrBase*                 getInstance(char const *userName, const MUINT32 sensorIdx);
    virtual void                        destroyInstance(char const *userName, const MUINT32 sensorIdx);

    /////////////////////////////////////////////////////////////////////////
    //
    // Hal3dnr () -
    //! \brief 3dnr Hal constructor
    //
    /////////////////////////////////////////////////////////////////////////
    Hal3dnr(const char *userName, const MUINT32 sensorIdx);

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
    virtual MBOOL                       init(const char *userName);

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
    virtual MBOOL                       uninit(const char *userName);

    virtual MBOOL                       update3DNRMvInfo(
                                            const NR3DHALParam& nr3dHalParam,
                                            // input
                                            void *pTnrLib, const MUINT32 featMask,
                                            const MUINT32 nLayers,
                                            const MSize srcCropSizes[],
                                            const MSize dstSizes[],
                                            const MINT32 uniqueKey,
                                            const MINT32 requestNo, const MINT32 frameNo,
                                            const NR3DMVInfo& lmvInfo,
                                            const NR3DRSCInfo& rscInfo,
                                            // output
                                            NR3D::NR3DMVInfo outGmvInfoResults[],
                                            MBOOL &outIsGMVInfoUpdated);

    virtual MBOOL                       update3DNRMvInfo(const NR3DHALParam &nr3dHalParam,
                                            NR3D::NR3DMVInfo &GMVInfoResult,
                                            MBOOL &outIsGMVInfoUpdated);
    virtual MBOOL                       do3dnrFlow(const NR3DHALParam &nr3dHalParam, NR3DHALResult &nr3dHalResult);
    // !!NOTES: legacy = isp50 or older
    void                                configNR3D_legacy(void *pTuning, void *p3A,
                                            const MRect& dstRect,
                                            const NR3DHwParam &nr3dHwParam);

    virtual MBOOL                       updateISPMetadata(
                                            NSCam::IMetadata *pMeta_InHal,
                                            const NSCam::NR3D::NR3DTuningInfo &tuningInfo);

protected:

    MTKTnr*                             createTnrLib(MUINT32 featMask);
    MTKTnr*                             getTnrLibPtr(const NR3DHALParam &nr3dHalParam);
    MBOOL                               savedFrameInfo(const NR3DHALParam& nr3dHalParam,
                                            NR3DHALResult &nr3dHalResult);
    MUINT32                             handleState(MUINT32 errStatus, NR3D_STATE_ENUM &stateMachine);
    // mv related
    void                                forceGMVByProp(
                                            const NR3DHALParam& nr3dHalParam,
                                            const MUINT32 layer,
                                            MINT32 &outGmvX, MINT32 &outGmvY,
                                            MFLOAT &outGmvX_f, MFLOAT &outGmvY_f);

    void                                calCMV(const Hal3dnrSavedFrameInfo &prevSavedFrameInfo,
                                            NR3DMVInfo &GMVInfo);
    void                                calGMV(const NR3DHALParam &nr3dHalParam,
                                            NR3DMVInfo &gmvInfo,
                                            NR3DHALResult &nr3dHalResult);

    // vipi/imgi align related
    MBOOL                               handleFrameAlign(const NR3DHALParam &nr3dHalParam,
                                           const NR3DMVInfo &gmvInfo, NR3DHwParam &outNr3dHwParam);
    MBOOL                               handleFrameZoomAlign(const NR3DHALParam &nr3dHalParam,
                                           const NR3DMVInfo &gmvInfo,
                                           NR3DHwParam &outNr3dHwParam,
                                           NR3DHALResult &nr3dHalResult);
    // 3dnr lib related
    MUINT32                             check3DNRPolicy(const NR3DHALParam &nr3dHalParam,
                                            const NR3DHALResult &nr3dHalResult);
    MUINT32                             checkIso(const NR3DHALParam &nr3dHalParam);
    MUINT32                             checkVipiImgiFrameSize(const NR3DHALParam &nr3dHalParam,
                                            const Hal3dnrSavedFrameInfo &preSavedFrameInfo);

    MBOOL                               handle3DNROnOffPolicy(const NR3DHALParam &nr3dHalParam,
                                            NR3DHALResult &nr3dHalResult);
    MBOOL                               handleAlignVipiIMGI(const NR3DHALParam &nr3dHalParam,
                                            NR3DHwParam &outNr3dParam,
                                            NR3DHALResult &nr3dHalResult);

    // === debug APIs ===
    void                                print_NR3DHALParam(const NR3DHALParam& nr3dHalParam);
    void                                print_NR3DHALResult(const NR3DHALParam& nr3dHalParam,
                                            const NR3DHALResult &result);

    void                                print_TNR_SET_PROC_INFO(
                                            const NR3DHALParam& nr3dHalParam,
                                            const TNR_SET_PROC_INFO_STRUCT& info);
    void                                print_TNR_RESULT_INFO(
                                            const NR3DHALParam& nr3dHalParam,
                                            const TNR_RESULT_INFO_STRUCT& info);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Variables.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MINT32                              mLogLevel = 0;
    MINT32                              mDebugEnable = 0;
    MBOOL                               mSupportZoom3DNR = MTRUE;

    const MUINT32                       mSensorIdx = 0;
    volatile MINT32                     mUsers = 0;
    mutable Mutex                       mLock;

    android::sp<NSCam::Utils::SensorProvider> mpSensorProvider;

    Lib3dnrInfo                         mTnrLibInfo_basic = Lib3dnrInfo("basic");
    Lib3dnrInfo                         mTnrLibInfo_basic_dsdn20 = Lib3dnrInfo("basic_dsdn20");
    Lib3dnrInfo                         mTnrLibInfo_basic_dsdn25 = Lib3dnrInfo("basic_dsdn25");
    Lib3dnrInfo                         mTnrLibInfo_smvrB  = Lib3dnrInfo("smvrB");
    Lib3dnrInfo                         mTnrLibInfo_smvrB_dsdn20  = Lib3dnrInfo("smvrB_dsdn20");
    Lib3dnrInfo                         mTnrLibInfo_smvrB_dsdn25  = Lib3dnrInfo("smvrB_dsdn25");
    Lib3dnrInfo                         mTnrLibInfo_multicam  = Lib3dnrInfo("multicam");
    Lib3dnrInfo                         mTnrLibInfo_unknown = Lib3dnrInfo("unknown");

    std::string                         mUserName;
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

    Hal3dnrObj(const char *userName) : Hal3dnr(userName, sensorIdx) {}
    ~Hal3dnrObj() {}
public:
    static Hal3dnr *getInstance(char const *userName);
    static void destroyInstance(char const *userName);
};

#endif  //_3DNR_HAL_H_
