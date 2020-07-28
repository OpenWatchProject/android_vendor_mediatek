/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
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
#ifndef __AINRFEFM_H__
#define __AINRFEFM_H__

// AINR Core Lib
#include <mtkcam3/feature/ainr/IAinrFeFm.h>
// Pass2 header
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <mtkcam/drv/def/IPostProcFeFm.h>
#include <mtkcam/drv/def/Dip_Notify_datatype.h> //_SRZ_SIZE_INFO_, EDipModule_SRZ1

// 3A
#include <mtkcam/aaa/IHal3A.h> // setIsp ...
#include <mtkcam/aaa/IHalISP.h>
// STL
#include <memory> //std::shared_ptr
#include <condition_variable>
#include <thread>
#include <future>

// AOSP
#include <utils/RefBase.h>

using android::sp;


// namespace for pass2
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSPostProc;
using namespace NS3Av3;

namespace ainr {

class AinrFeFm : public IAinrFeFm {

public:
    AinrFeFm();

/* implementation from IAinrNvram */
public:
    virtual enum AinrErr init(IAinrFeFm::ConfigParams const& params, IAinrFeFm::OutInfo *outParam = nullptr);
    virtual enum AinrErr getFeFmSize(MSize &feoSize, MSize &fmoSize);
    virtual enum AinrErr buildFe(IAinrFeFm::DataPackage *package);
    virtual enum AinrErr buildFm(IImageBuffer *baseBuf, IImageBuffer *refBuf, IImageBuffer *fmo, int index, bool needCb);
    virtual enum AinrErr doFe(const IAinrFeFm::DataPackage *package);
    virtual enum AinrErr doFeFm();
    virtual ~AinrFeFm(void);
private:
    // Check package content is correct or not
    enum AinrErr checkPackage(IAinrFeFm::BufferPackage &bufferSet, IAinrFeFm::MetaDataPackage &metaSet);
    enum AinrErr setupFeFmTuning();
    // Return FEFM block size
    int getFeFmBlockSize(const int FE_MODE);
    enum AinrErr prepareFE(QParams &qParams, IImageBuffer *inputBuff,
            IImageBuffer *outputFeo, IImageBuffer *outputYuv,
            std::vector<int> *matrix, MetaSet_T &metaset);
    enum AinrErr prepareFM(QParams &qParams, IImageBuffer *feoBase, IImageBuffer *feoRef, IImageBuffer *fmo, int index, bool needCb);

    void bufferDump(IImageBuffer* buff, IMetadata* halMeta
            , const char *pUserString, int port, formatType type);
private:
    // Sensor id
    int m_sensorId;
    // Normal stream
    std::unique_ptr< INormalStream, std::function<void(INormalStream*)> >
                                    m_pNormalStream;
    // Hal3a
    std::unique_ptr< NS3Av3::IHalISP, std::function<void(NS3Av3::IHalISP*)> >
                                    m_pHalIsp;
    // used for pass2 async
    mutable std::mutex m_mutex;
    std::condition_variable m_cv;
    // FEO size
    // AinrSizeInfo m_feoSize;
    MSize m_feoSize;
    // FMO size
    MSize m_fmoSize;
    // FEFM block size
    int m_blockSize;
    // FEFM resolution. Because of the fact that FEFM resolution should be
    // block size alignment. We need to record it and feed it into Pass2 module.
    MSize m_CropSize;
    // Buffer size
    MSize m_bufferSize;
    // FEFM. SRZ info
    _SRZ_SIZE_INFO_ m_srzSizeInfo;
    // Pass2 tuning
    std::vector<std::shared_ptr<char>> m_vTuningData;
    // Feo info
    FEInfo m_feInfo;
    // Fmo info
    FMInfo m_fmInfo;
    // Tuning size
    size_t m_regTableSize;

    // Tuning buffer container
    std::vector<std::shared_ptr<char>>
            m_tuningBuffers;

    // Callback methods
    mutable std::mutex m_cbMx;
    using cbMethodType = std::function<void(int)>;
    cbMethodType m_cbMethod;
    // For P2 enqueue
    QParams m_qParams;
};
}; /* namespace ainr */
#endif//__MFLLNVRAM_H__
