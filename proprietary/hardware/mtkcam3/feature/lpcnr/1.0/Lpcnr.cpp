/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
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
#define LOG_TAG "LPCNR"

#include "Lpcnr.h"

#include <mtkcam/utils/std/Log.h> // CAM_LOGD
// Allocate working buffer. Be aware of that we use AOSP library
#include <mtkcam3/feature/utils/ImageBufferUtils.h>
// Isp profile mapper
#include <mtkcam3/feature/utils/ISPProfileMapper.h>


// platform dependent headers
#include <dip_reg.h> // dip_x_reg_t
// ISP profile
//#include <tuning_mapping/cam_idx_struct_ext.h>
//tuning utils
#include <mtkcam/utils/TuningUtils/FileReadRule.h>
#include <cutils/properties.h>
#include <mtkcam/aaa/IHalISP.h>
#include <mtkcam/utils/exif/DebugExifUtils.h> // for debug exif
#include <mtkcam/custom/ExifFactory.h>
// Custom debug exif
#include <debug_exif/dbg_id_param.h>
#include <debug_exif/cam/dbg_cam_param.h>

// Std
#include <unordered_map> // std::unordered_map
#include <deque> // std::deque


using namespace NSCam::NSIoPipe;
using namespace lpcnr;
using namespace NSCam::TuningUtils;

// ----------------------------------------------------------------------------
// MY_LOG
// ----------------------------------------------------------------------------
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

#define __DEBUG // enable debug


#ifdef __DEBUG
#include <mtkcam/utils/std/Trace.h>
#define FUNCTION_TRACE()                            CAM_TRACE_CALL()
#define FUNCTION_TRACE_NAME(name)                   CAM_TRACE_NAME(name)
#define FUNCTION_TRACE_BEGIN(name)                  CAM_TRACE_BEGIN(name)
#define FUNCTION_TRACE_END()                        CAM_TRACE_END()
#define FUNCTION_TRACE_ASYNC_BEGIN(name, cookie)    CAM_TRACE_ASYNC_BEGIN(name, cookie)
#define FUNCTION_TRACE_ASYNC_END(name, cookie)      CAM_TRACE_ASYNC_END(name, cookie)
#else
#define FUNCTION_TRACE()
#define FUNCTION_TRACE_NAME(name)
#define FUNCTION_TRACE_BEGIN(name)
#define FUNCTION_TRACE_END()
#define FUNCTION_TRACE_ASYNC_BEGIN(name, cookie)
#define FUNCTION_TRACE_ASYNC_END(name, cookie)
#endif

//-----------------------------------------------------------------------------
// LPCNR util
//-----------------------------------------------------------------------------

static auto getDebugExif()
{
    static auto const inst = MAKE_DebugExif();
    return inst;
}

//-----------------------------------------------------------------------------
// ILpcnrCore methods
//-----------------------------------------------------------------------------
std::shared_ptr<ILpcnr> ILpcnr::createInstance()
{
    std::shared_ptr<ILpcnr> pLpcnrCore = std::make_shared<LpcnrCore>();
    return pLpcnrCore;
}
bool ILpcnr::supportLpcnr()
{
    return true;
}
//-----------------------------------------------------------------------------
// LpcnrCore implementation
//-----------------------------------------------------------------------------
LpcnrCore::LpcnrCore()
    : m_workingBuffer(nullptr)
    , m_regTableSize(0)
{
}
//-----------------------------------------------------------------------------
LpcnrCore::~LpcnrCore()
{
    // wait alloc working future done (timeout 1 second)
    if (CC_LIKELY( m_WorkAllocThread.valid() )) {
        auto _status = m_WorkAllocThread.wait_for(std::chrono::seconds(1));
        if (CC_UNLIKELY( _status != std::future_status::ready )) {
            MY_LOGE("wait future status is not ready");
        }
    }
    ImageBufferUtils::getInstance().deallocBuffer(m_workingBuffer.get());
}
//-----------------------------------------------------------------------------
enum LpcnrErr LpcnrCore::init(ILpcnr::ConfigParams const& params)
{
    enum LpcnrErr err = LpcnrErr_Ok;

    int sensorId = params.openId;

    m_config = params;

    //m_config.buffSize.w = (m_config.buffSize.w * 12)/8;

    // Start to check params content
    if(CC_UNLIKELY(checkParams(m_config) != LpcnrErr_Ok)) {
        MY_LOGE("check param fail");
        return LpcnrErr_BadArgument;
    }

     /* RAII for INormalStream */
    m_pNormalStream = decltype(m_pNormalStream)(
        INormalStream::createInstance(sensorId),
        [](INormalStream* p) {
            if (!p) return;
            p->uninit(LOG_TAG);
            p->destroyInstance();
        }
    );

    if (CC_UNLIKELY( m_pNormalStream.get() == nullptr )) {
        MY_LOGE("create INormalStream fail");
        return LpcnrErr_BadArgument;
    }
    else {
        auto bResult = m_pNormalStream->init(LOG_TAG);
        if (CC_UNLIKELY(bResult == MFALSE)) {
            MY_LOGE("init INormalStream returns MFALSE");
            return LpcnrErr_BadArgument;
        }
    }

    // get tuning size
    m_regTableSize = m_pNormalStream->getRegTableSize();

    // Allocate working buffer
    m_WorkAllocThread = std::async(std::launch::async,
            [ this ](MSize buffSize) ->int {
            auto ret = ImageBufferUtils::getInstance().allocBuffer(
                m_workingBuffer, buffSize.w, buffSize.h/2, eImgFmt_STA_12BIT);
            if(CC_UNLIKELY(ret == MFALSE)) {
                MY_LOGE("allocate FeoBase buffer error!!!");
                return LpcnrErr_BadArgument;
            }
            return LpcnrErr_Ok;
        }, m_config.buffSize);

lbExit:
    return err;
}

enum LpcnrErr LpcnrCore::doLpcnr()
{
    enum LpcnrErr err = LpcnrErr_Ok;
    FUNCTION_TRACE();

    // Working buffer get
    if (CC_LIKELY( m_WorkAllocThread.valid() )) {
        if (CC_UNLIKELY( m_WorkAllocThread.get() != 0 )) {
            MY_LOGE("allocate working buffer failed");
            return LpcnrErr_BadArgument;
        }

        if (CC_UNLIKELY( m_workingBuffer.get() == nullptr )) {
            MY_LOGE("lpcnr working buffers are nullptr");
            return LpcnrErr_BadArgument;
        }
    }

    // Start to check params content
    if(CC_UNLIKELY(checkParams(m_config) != LpcnrErr_Ok)) {
        MY_LOGE("check param fail");
        return LpcnrErr_BadArgument;
    }

    // Input/Output buffer
    IImageBuffer* inputBuffer   = m_config.inputBuff;
    IImageBuffer* outputBuffer  = m_config.outputBuff;
    MSize buffSize = inputBuffer->getImgSize();
    MINT fmt = inputBuffer->getImgFormat();

    int dump = ::property_get_int32("vendor.debug.camera.dump.nr", 0);
    if(dump) {
        IMetadata* halMeta = m_config.halMeta;
        if(halMeta) {
            bufferDump(halMeta, inputBuffer, YUV_PORT_UNDEFINED, "lpcnr-input");
        }
    }

    // Lpcnr need two run for ISP
    QParams enqueParams;
    FrameParams frameParams, frameParams2;

    // frame tag
    frameParams.mStreamTag  = NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_LPCNR_Pass1;
    frameParams2.mStreamTag = NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_LPCNR_Pass2;

    // Run1/Run2 input
    // Origin UV input
    Input oriUVIn;
    oriUVIn.mPortID          = PORT_LPCNR_YUVI;
    oriUVIn.mBuffer          = inputBuffer;
    oriUVIn.mPortID.group    = 0;
    frameParams.mvIn.push_back(oriUVIn);
    frameParams2.mvIn.push_back(oriUVIn);

    // Output
    Output dst;
    dst.mPortID                = PORT_LPCNR_UVO;
    dst.mBuffer                = m_workingBuffer.get();
    dst.mPortID.group          = 0;
    frameParams.mvOut.push_back(dst);

    // Use first run output(UV) as second run input
    Input input2UV;
    input2UV.mPortID         = PORT_LPCNR_UVI;
    input2UV.mBuffer         = m_workingBuffer.get();
    input2UV.mPortID.group   = 0;
    frameParams2.mvIn.push_back(input2UV);

    // Second run UV output
    Output dst2;
    dst2.mPortID             = PORT_LPCNR_YUVO;
    dst2.mBuffer             = outputBuffer;
    dst2.mPortID.group       = 0;
    frameParams2.mvOut.push_back(dst2);

    // prepare tuning buffer
    // copy input metadata to meta set
    NS3Av3::MetaSet_T inMetaSet(*(m_config.appMeta), *(m_config.halMeta));
    NS3Av3::MetaSet_T outMetaSet;

    // update isp profile if necessary
    // Query ispProfile
    ISPProfileMapper* ispProfileMapper = ISPProfileMapper::getInstance();
    if (CC_UNLIKELY(ispProfileMapper == nullptr)) {
        MY_LOGF("ispProfileMapper is nullptr");
        return LpcnrErr_BadArgument;
    }

    auto mappingKey
            = ispProfileMapper->queryMappingKey(&inMetaSet.appMeta,
                &inMetaSet.halMeta,
                false,  // is physical or not
                eMappingScenario_Capture,
                eSensorComb_Single);

    NSIspTuning::EIspProfile_T ispProfile = {};
    auto profileStage = eStage_chromaNR;
    if(!ispProfileMapper->mappingProfile(mappingKey, profileStage,
            ispProfile)) {
        MY_LOGW("Failed to get ISP Profile, stage(%d) use default", profileStage);
    } else {
        MY_LOGD("lpcnr ispProfile(%d)", ispProfile);
        IMetadata::setEntry<MUINT8>(&inMetaSet.halMeta, MTK_3A_ISP_PROFILE, ispProfile);
    }

    NS3Av3::TuningParam rTuningParam1, rTuningParam2;

    std::unique_ptr< NS3Av3::IHalISP, std::function<void(NS3Av3::IHalISP*)> >
                                    pHalIsp =
                                    std::unique_ptr< NS3Av3::IHalISP, std::function<void(NS3Av3::IHalISP*)> >
                                    (
                                        MAKE_HalISP(m_config.openId, "AinrLpcnr"),
                                        [](NS3Av3::IHalISP* p) { if(CC_LIKELY(p)) p->destroyInstance("AinrLpcnr"); }
                                    );

    if (CC_UNLIKELY(pHalIsp.get() == nullptr)) {
        MY_LOGF("create IHalIsp failed");
        return LpcnrErr_BadArgument;
    }

    auto firstTuningBuf = std::unique_ptr<char[]>(new char[m_regTableSize]{0});
    if(CC_UNLIKELY(firstTuningBuf.get() == nullptr)) {
        MY_LOGF("allocate first run tunning buffer error");
        return LpcnrErr_BadArgument;
    }

    auto secondTuningBuf = std::unique_ptr<char[]>(new char[m_regTableSize]{0});
    if(CC_UNLIKELY(secondTuningBuf.get() == nullptr)) {
        MY_LOGF("allocate first run tunning buffer error");
        return LpcnrErr_BadArgument;
    }

    // Indicate ISP it is LPCNR
    MUINT8 tuningMode1 = 3;
    MUINT8 tuningMode2 = 4;

    if(fmt == eImgFmt_MTK_YUV_P010) {
        tuningMode1 = 6;
        tuningMode2 = 7;
    }

    MY_LOGD("LPCNR tuning mode(%d, %d)", tuningMode1, tuningMode2);

    IMetadata::setEntry<MUINT8>(&inMetaSet.halMeta, MTK_ISP_P2_TUNING_UPDATE_MODE, tuningMode1);

    rTuningParam1.pRegBuf = firstTuningBuf.get();
    rTuningParam1.pLcsBuf = nullptr;

    {
        FUNCTION_TRACE_NAME("First run setIsp");
        if (0 > pHalIsp->setP2Isp(0, inMetaSet, &rTuningParam1, nullptr))
        {
            MY_LOGW("pass2 setIsp - skip tuning pushing");
            return LpcnrErr_BadArgument;
        }
    }

    // TODO: We need to remove this kind of memory copy
    memcpy(secondTuningBuf.get(), firstTuningBuf.get(), m_regTableSize);
    // Second run setIsp
    // Indicate ISP it is LPCNR
    IMetadata::setEntry<MUINT8>(&inMetaSet.halMeta, MTK_ISP_P2_TUNING_UPDATE_MODE, tuningMode2);
    rTuningParam2.pRegBuf = secondTuningBuf.get();
    rTuningParam2.pLcsBuf = nullptr;

    {
        FUNCTION_TRACE_NAME("Second run setIsp");
        if (0 > pHalIsp->setP2Isp(0, inMetaSet, &rTuningParam2, nullptr))
        {
            MY_LOGW("pass2 setIsp - skip tuning pushing");
            return LpcnrErr_BadArgument;
        }
    }

    // setup first run tuning buffer in P2 parameter
    frameParams.mTuningData  = firstTuningBuf.get();
    // setup second run tuning buffer in P2 parameter
    frameParams2.mTuningData = secondTuningBuf.get();

    // First run
    enqueParams.mvFrameParams.push_back(frameParams);
    // Second run
    enqueParams.mvFrameParams.push_back(frameParams2);
    // Callback
    Pass2Async p2Async;
    enqueParams.mpCookie = static_cast<void*>(&p2Async);
    enqueParams.mpfnCallback = [](QParams& rParams)->MVOID
    {
        if (CC_UNLIKELY(rParams.mpCookie == nullptr))
            return;

        Pass2Async* pAsync = static_cast<Pass2Async*>(rParams.mpCookie);
        std::lock_guard<std::mutex> __l(pAsync->getLocker());
        pAsync->notifyOne();
        MY_LOGD("LPCNR pass2 done");
    };


    // Start to enque request to pass2
    {
        FUNCTION_TRACE_NAME("LPCNR processing");
        auto __l = p2Async.uniqueLock();
        MBOOL bEnqueResult = MTRUE;
        {
            if (CC_UNLIKELY(m_pNormalStream.get() == nullptr)) {
                MY_LOGE("INormalStream instance is NULL");
                return LpcnrErr_BadArgument;
            }
            MY_LOGD("Lpcnr pass2 enque");
            bEnqueResult = m_pNormalStream->enque(enqueParams);
        }

        if (CC_UNLIKELY(!bEnqueResult)) {
            MY_LOGE("Lpcnr enque fail");
            return LpcnrErr_BadArgument;
        }
        else {
            p2Async.wait(std::move(__l));
        }
    }

    if(dump) {
        if(m_config.halMeta) {
            bufferDump(m_config.halMeta, m_workingBuffer.get(), YUV_PORT_TIMGO, "lpcnrout1");
            bufferDump(m_config.halMeta, m_config.outputBuff, YUV_PORT_TIMGO, "lpcnrout2");
        }
    }

lbExit:
    return err;
}

enum LpcnrErr LpcnrCore::checkParams(ILpcnr::ConfigParams const& params)
{
    enum LpcnrErr err = LpcnrErr_Ok;

    if(CC_UNLIKELY(params.appMeta == nullptr)) {
        MY_LOGE("appMeta is null");
        err = LpcnrErr_BadArgument;
        goto lbExit;
    }

    if(CC_UNLIKELY(params.halMeta == nullptr)) {
        MY_LOGE("halMeta is null");
        err = LpcnrErr_BadArgument;
        goto lbExit;
    }

    if(CC_UNLIKELY(params.appDynamic == nullptr)) {
        MY_LOGE("appDynamic is null");
        err = LpcnrErr_BadArgument;
        goto lbExit;
    }

    if(CC_UNLIKELY(params.inputBuff == nullptr)) {
        MY_LOGE("inputBuff is null");
        err = LpcnrErr_BadArgument;
        goto lbExit;
    }

    if(CC_UNLIKELY(params.outputBuff == nullptr)) {
        MY_LOGE("outputBuff is null");
        err = LpcnrErr_BadArgument;
        goto lbExit;
    }

lbExit:
    return err;
}

enum LpcnrErr LpcnrCore::makeDebugInfo(IMetadata* metadata)
{
    bool haveExif = false;
    {
        IMetadata::IEntry entry = metadata->entryFor(MTK_HAL_REQUEST_REQUIRE_EXIF);
        if (! entry.isEmpty()  && entry.itemAt(0, Type2Type<MUINT8>()) )
                haveExif = true;
    }
    //
    if (haveExif)
    {
        IMetadata::Memory memory_dbgInfo;
        memory_dbgInfo.resize(sizeof(DEBUG_RESERVEA_INFO_T));
        DEBUG_RESERVEA_INFO_T& dbgInfo =
            *reinterpret_cast<DEBUG_RESERVEA_INFO_T*>(memory_dbgInfo.editArray());
        ssize_t idx = 0;
#define addPair(debug_info, index, id, value)           \
        do{                                             \
            debug_info.Tag[index].u4FieldID = (0x01000000 | id); \
            debug_info.Tag[index].u4FieldValue = value; \
            index++;                                    \
        } while(0)
        //
        addPair(dbgInfo , idx , LPCNR_ENABLE, 1);
        //
#undef addPair
        //
        IMetadata exifMeta;
        // query from hal metadata first
        {
            IMetadata::IEntry entry = metadata->entryFor(MTK_3A_EXIF_METADATA);
            if (! entry.isEmpty() )
                exifMeta = entry.itemAt(0, Type2Type<IMetadata>());
        }
        // update
        IMetadata::IEntry entry_key(MTK_POSTNR_EXIF_DBGINFO_NR_KEY);
        entry_key.push_back(DEBUG_EXIF_MID_CAM_RESERVE1, Type2Type<MINT32>());
        exifMeta.update(entry_key.tag(), entry_key);
        //
        IMetadata::IEntry entry_data(MTK_POSTNR_EXIF_DBGINFO_NR_DATA);
        entry_data.push_back(memory_dbgInfo, Type2Type<IMetadata::Memory>());
        exifMeta.update(entry_data.tag(), entry_data);
        //
        IMetadata::IEntry entry_exif(MTK_3A_EXIF_METADATA);
        entry_exif.push_back(exifMeta, Type2Type<IMetadata>());
        metadata->update(entry_exif.tag(), entry_exif);
    }
    else
    {
        MY_LOGD("no need to dump exif");
    }
    //
    return LpcnrErr_Ok;
}

void LpcnrCore::bufferDump(IMetadata *halMeta, IImageBuffer* buff, YUV_PORT type, const char *pUserString) {
    // dump input buffer
    char                      fileName[512];
    FILE_DUMP_NAMING_HINT     dumpNamingHint;
    //
    MUINT8 ispProfile = NSIspTuning::EIspProfile_Capture;

    if(!halMeta || !buff) {
        MY_LOGE("HalMeta or buff is nullptr, dump fail");
        return;
    }

    if (!IMetadata::getEntry<MUINT8>(halMeta, MTK_3A_ISP_PROFILE, ispProfile)) {
        MY_LOGW("cannot get ispProfile at ainr capture");
    }

    // Extract hal metadata and fill up file name;
    extract(&dumpNamingHint, halMeta);
    // Extract buffer information and fill up file name;
    extract(&dumpNamingHint, buff);
    // Extract by sensor id
    extract_by_SensorOpenId(&dumpNamingHint, m_config.openId);
    // IspProfile
    dumpNamingHint.IspProfile = ispProfile; //EIspProfile_Capture;

    genFileName_YUV(fileName, sizeof(fileName), &dumpNamingHint, type, pUserString);
    buff->saveToFile(fileName);
}
