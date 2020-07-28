/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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
#ifndef __AINRCORE_H__
#define __AINRCORE_H__

#include <mtkcam3/feature/ainr/AinrDefs.h>
#include <mtkcam3/feature/ainr/AinrTypes.h>

/* interfaces */
#include <mtkcam3/feature/ainr/IAinrCore.h>
#include <mtkcam3/feature/ainr/IAinrNvram.h>
#include <mtkcam3/feature/ainr/IAinrFeFm.h>
#include <mtkcam3/feature/ainr/IAinrWarping.h>

/* standard libs */
#include <cstdint>
#include <semaphore.h>
#include <vector>
#include <string>
#include <future>
#include <atomic>
#include <memory> //std::shared_ptr
#include <condition_variable>
#include <thread>
#include <future>
#include <unordered_map>

#include <utils/RefBase.h>

/* MTKCAM */
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h>
#include <mtkcam/utils/std/JobQueue.h>

/* NVRAM */
/*To-Do: remove when cam_idx_mgr.h reorder camear_custom_nvram.h order before isp_tuning_cam_info.h */
#include <camera_custom_nvram.h>
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>

/* AINR Algo */
#include <MTKNRUtils.h>
#include <MTKNRCore.h>
#include <MTKHG.h>

/* Algo DNG */
#include <MTKDngOp.h>

using std::vector; // use std::vector as vector
using namespace android;
using namespace NSCam::TuningUtils;

namespace ainr {

class AinrCore : public IAinrCore {
public:
    AinrCore(void);
    virtual ~AinrCore(void);

    struct coreTuningData {
        unsigned int blend_R;
        unsigned int blend_Gr;
        unsigned int blend_Gb;
        unsigned int blend_B;
        //
        coreTuningData()
            : blend_R(0)
            , blend_Gr(0)
            , blend_Gb(0)
            , blend_B(0)
            {}
    };

    struct blendingTuningData {
        unsigned int blend_Y;
        unsigned int blend_UV;
        blendingTuningData()
            : blend_Y(0)
            , blend_UV(0)
            {}
    };

/* implementations */
public:
    /*
     * We initialize NRAlgo and P2 related modules (FEFM, Warping)
     * We ask jobQueue to do memory allocation.
    */
    virtual enum AinrErr init(const AinrConfig_t &cfg);
    virtual enum AinrErr doAinr();
    virtual enum AinrErr doCancel();
    virtual enum AinrErr setNvramProvider(const std::shared_ptr<IAinrNvram> &nvramProvider);
    virtual enum AinrErr addInput(const std::vector<AinrPipelinePack>& inputPack);
    virtual enum AinrErr addOutput(IImageBuffer *outBuffer);
    virtual void registerCallback(std::function<void(int32_t, int32_t)> cb);
    virtual bool queryAlgoSupport(NSCam::MSize size);
    virtual AINRCORE_VERSION queryVersion() const;
    virtual int32_t queryMainFrameIndex() const;

private:
    enum AinrErr doMatching();
    enum AinrErr doNrCore();
    enum AinrErr releaseFeFmBuffer();
    enum AinrErr releaseWarpingBuffer();
    enum AinrErr releaseGridBuffer();
    enum AinrErr releaseUnpackBuffer();
    enum AinrErr setUpTuning(NRCORE_PROC1_IN_STRUCT *unPackBayerCfg);
    enum AinrErr setUpTileModeTuning(NRCORE_PROC2_IN_STRUCT *unPackBayerCfg);
    enum AinrErr setUpBayerOrder(IImageBuffer *rawBuffer);
    enum AinrErr formatConverter(IImageBuffer *pSrcImg, IImageBuffer *pDstImg);
    enum AinrErr unpackRawBuffer(IImageBuffer* pInPackBuf, IImageBuffer* pOutUnpackBuf);
    void setDebugExif(IMetadata* metadata, int algoType);
    void nextCapture();
    void parsingMeta();
    void acquireJobQueue();
    void bufferReload();
    void prepareJobs();
    void dumpAinrTuning(FEATURE_NVRAM_AINR_T *data);
    void bufferDumpBlob(IMetadata *halMeta, IImageBuffer* buff, const char *pUserString, MUINT8 ispProfile);
    void bufferDumpRaw(IMetadata *halMeta, IImageBuffer* buff, RAW_PORT type, const char *pUserString, MUINT8 ispProfile);
    void bufferDumpYuv(IMetadata *halMeta, IImageBuffer* buff, YUV_PORT type, const char *pUserString, MUINT8 ispProfile);
/* threads, we use std::async to begin threads */
public:
    struct FuturePack
    {
        std::future<void>   fu;
        std::mutex          mx;
        inline void wait() { if (fu.valid()) fu.wait(); }
    };
private:
    //
    int                             m_DumpWorking;
    // Bittrue
    int                             m_bittrueEn;
    //
    int                             m_onlineTuning;
    //
    int                             m_autoTest;
    // Metadata and input buffer container
    mutable std::mutex m_DataLock;
    std::vector<AinrPipelinePack> m_vDataPackage;
    // Config information
    // Mutex lock to protect
    mutable std::mutex           m_cfgMx;
    int             m_sensorId;
    unsigned int    m_imgoWidth; // for full raw size and full size
    unsigned int    m_imgoHeight; // for full raw size and full size
    size_t          m_imgoStride;
    unsigned int    m_rrzoWidth; // for rrzo size
    unsigned int    m_rrzoHeight; // for rrzo size
    uint32_t        m_uniqueKey; //MTK_PLUGIN_UNIQUEKEY of frame 0
    MUINT8          m_fefmIspProfile;
    MUINT8          m_mainPathProfile;
    int             m_captureNum;
    int             m_requestNum;
    int             m_frameNum;
    // DGN gain
    unsigned int                     m_dgnGain;
    int                              m_obFst;

    // need Tile mode
    bool                             mbNeedTileMode;
    // Low memory
    bool                             m_bIsLowMem;

    // algo type
    int                              m_algoType;

    // Module instance
    mutable std::mutex               m_nvMx;
    std::shared_ptr<IAinrNvram>      m_NvramPtr;

    // FeFm working buffer
    mutable std::mutex              m_fefmMx;
    std::condition_variable         m_fefmCond;
    volatile bool                   mb_AllocfefmDone;
    std::vector<sp<IImageBuffer>>   m_vFeo;
    std::vector<sp<IImageBuffer>>   m_vRrzoYuv; // for bittrue
    std::vector<sp<IImageBuffer>>   m_vFmoBase; //m_vFmo[0] is dummy buffer. no use
    std::vector<sp<IImageBuffer>>   m_vFmoRef; //m_vFmo[0] is dummy buffer. no use
    std::shared_ptr<IAinrFeFm>      m_fefm;

    // RRZO crop for homography
    MRect                           m_rrzCrop;

    // BSS
    std::vector<int>                mv_bssOrder;

    // GenGrid
    std::vector<sp<IImageBuffer>>   m_vGridX;
    std::vector<sp<IImageBuffer>>   m_vGridY;

    // Warping reference buffer RG/GB
    std::vector<sp<IImageBuffer>>   m_vRg;
    std::vector<sp<IImageBuffer>>   m_vGb;
    std::shared_ptr<IAinrWarping>   m_warping;

    // Unpack raws for AINR MDLA
    // 0: base frame 1~N: Reference frame
    mutable std::mutex              m_AllocUpMx;
    std::condition_variable         m_AllocUpCond;
    volatile bool                   mb_AllocUpDone;
    std::vector<sp<IImageBuffer>>   mv_unpackRaws;

    // Final output from MDLA
    IImageBuffer*                   m_outBuffer;

    // JobQueue
    std::shared_ptr<
        NSCam::JobQueue<void()>
    >                               mMinorJobQueue;

    // Unpack async control
    // mutable std::mutex              m_unpackMx;
    // std::condition_variable         m_unpackCond;
    // volatile bool                   mb_unpackDone;

    mutable std::mutex              m_upBaseMx;
    std::condition_variable         m_upBaseCond;
    bool                            mb_upBaseDone;

    // AINR tuning data
    coreTuningData                  mCoreTuning;

    // AINR callback
    mutable std::mutex              m_cbMx;
    std::function<void(int32_t, int32_t)>
                                    m_cb;

    // AINR bittrue
    std::vector<AinrMetaPack>       mvMetaPacks;

    // Pipeline stage
    // Warping function
    std::function<enum AinrErr(int)>           m_warpingMethod;
    std::function<enum AinrErr(int)>           m_unpackMethod;
    std::function<enum AinrErr(int)>           m_homoMethod;
    std::function<void(int)>           m_matchingMethod;
    std::function<void(int)>           m_AsyncLauncher;
    //
    using matchingFutureType = std::future<void>;
    mutable std::mutex                 m_jobsMx;
    std::vector<matchingFutureType>    m_jobs;
}; /* class AinrCore */
}; /* namespace ainr */
#endif /* __AINRCORE_H__ */
