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
#define LOG_TAG "AinrCore/Warping"

#include "AinrWarping.h"
#include <mtkcam3/feature/ainr/AinrUlog.h>
// Pass2 header
#include <mtkcam/drv/def/ispio_port_index.h> // port id

#include <unordered_map> // std::unordered_map

// platform dependent headers
// If you want to debug with dip_reg.h remember to add
// "in mk (AINR_INCLUDE_PATH += $(MTK_PATH_SOURCE)/hardware/mtkcam/drv/include/dip/isp_50/)"
//#include <dip_reg.h> // dip_x_reg_t
// ISP profile
#include <tuning_mapping/cam_idx_struct_ext.h>
// Allocate working buffer. Be aware of that we use AOSP library
#include <mtkcam3/feature/utils/ImageBufferUtils.h>
#include <mtkcam/utils/TuningUtils/FileDumpNamingRule.h> // tuning file naming

//AOSP
#include <utils/Vector.h>



using namespace ainr;
using namespace NSImageio::NSIspio;

//-----------------------------------------------------------------------------
// IAinrWarping methods
//-----------------------------------------------------------------------------
std::shared_ptr<IAinrWarping> IAinrWarping::createInstance()
{
    std::shared_ptr<IAinrWarping> pAinrWarping = std::make_shared<AinrWarping>();
    return pAinrWarping;
}

AinrWarping::AinrWarping()
    : m_sensorId(0)
    , m_regTableSize(0)
{
}

AinrWarping::~AinrWarping()
{
    m_pNormalStream = nullptr;
}

enum AinrErr AinrWarping::init(IAinrWarping::ConfigParams const& params)
{
    enum AinrErr err = AinrErr_Ok;
    auto sensorId = params.openId;

    m_sensorId = params.openId;
    /* RAII for INormalStream */
    m_pNormalStream = decltype(m_pNormalStream)(
        INormalStream::createInstance(sensorId),
        [](INormalStream* p) {
            if (!p) return;
            p->uninit(LOG_TAG, NSCam::NSIoPipe::EStreamPipeID_WarpEG);
            p->destroyInstance();
        }
    );

    if (CC_UNLIKELY( m_pNormalStream.get() == nullptr )) {
        ainrLogE("create INormalStream fail");
        return AinrErr_UnexpectedError;
    }
    else {
        auto bResult = m_pNormalStream->init(LOG_TAG, NSCam::NSIoPipe::EStreamPipeID_WarpEG);
        if (CC_UNLIKELY(bResult == MFALSE)) {
            ainrLogE("init INormalStream returns MFALSE");
            return AinrErr_UnexpectedError;
        }
    }

    // Tuning size
    m_regTableSize = m_pNormalStream->getRegTableSize();

lbExit:
    return err;

}

enum AinrErr AinrWarping::doWarping(IAinrWarping::WarpingPackage &infoPack)
{
    CAM_ULOGM_FUNCLIFE();
    enum AinrErr err = AinrErr_Ok;
    if(CC_UNLIKELY(checkPackage(infoPack))) {
        return AinrErr_NullPointer;
    }

    QParams qParams;
    WPEAINRParams wpe_AINRparamsOdd, wpe_AINRparamsEven;
    WPEQParams    enqueWpeParamsOdd, enqueWpeParamsEven;

    err = prepareWarping(qParams, infoPack, WARPING_ODD, wpe_AINRparamsOdd, enqueWpeParamsOdd);
    err = prepareWarping(qParams, infoPack, WARPING_EVEN, wpe_AINRparamsEven, enqueWpeParamsEven);

    Pass2Async p2Async;
    qParams.mpCookie = static_cast<void*>(&p2Async);
    qParams.mpfnCallback = [](QParams& rParams)->MVOID
    {
        if (CC_UNLIKELY(rParams.mpCookie == nullptr))
            return;

        Pass2Async* pAsync = static_cast<Pass2Async*>(rParams.mpCookie);
        std::lock_guard<std::mutex> __l(pAsync->getLocker());
        pAsync->notifyOne();
        ainrLogD("Warping pass2 done");
    };

    // Start to enque request to pass2
    {
        auto __l = p2Async.uniqueLock();
        MBOOL bEnqueResult = MTRUE;
        {
            if (CC_UNLIKELY(m_pNormalStream.get() == nullptr)) {
                ainrLogE("INormalStream instance is NULL");
                return AinrErr_BadArgument;
            }
            ainrLogD("Warping pass2 enque");
            bEnqueResult = m_pNormalStream->enque(qParams);
        }

        if (CC_UNLIKELY(!bEnqueResult)) {
            ainrLogE("Warping enque fail");
            return AinrErr_BadArgument;
        }
        else {
            p2Async.wait(std::move(__l));
        }
    }
    return AinrErr_Ok;
}

enum AinrErr AinrWarping::checkPackage(const IAinrWarping::WarpingPackage &infoPack)
{
    if(CC_UNLIKELY(!infoPack.buffer
        && !infoPack.gridX
        && !infoPack.gridY
        && !infoPack.outRg
        && !infoPack.outGb)) {
        ainrLogE("Check pacakge fail because of null pointer");
        return AinrErr_NullPointer;
    }
    return AinrErr_Ok;
}

enum AinrErr AinrWarping::prepareWarping(QParams &qParams, IAinrWarping::WarpingPackage &infoPack, int mode
                                                , WPEAINRParams &wpe_AINRparams, WPEQParams &enqueWpeParams)
{
    IImageBuffer* inputBuffer = infoPack.buffer;
    IImageBuffer* gridXBuffer = infoPack.gridX;
    IImageBuffer* gridYBuffer = infoPack.gridY;
    IImageBuffer* out         = nullptr;

    if(mode == WARPING_ODD) {
        out = infoPack.outRg;
    } else {
        out = infoPack.outGb;
    }

    FrameParams frameParams;

    // Set up performance decision
    struct timeval current = {};
    ::gettimeofday(&current, nullptr);
    frameParams.ExpectedEndTime = current;

    // Input buffer
    NSCam::NSIoPipe::Input src;
    src.mPortID       = PORT_WPEI;
    src.mBuffer       = inputBuffer;
    src.mPortID.group = 0;
    frameParams.mvIn.push_back(src);

    //crop information
    MSize inputSize = inputBuffer->getImgSize();
    MCrpRsInfo crop;
    crop.mFrameGroup                = 0;
    crop.mGroupID                   = 1;
    MCrpRsInfo crop2;
    crop2.mFrameGroup               = 0;
    crop2.mGroupID                  = 2;
    MCrpRsInfo crop3;
    crop3.mFrameGroup               = 0;
    crop3.mGroupID                  = 3;
    crop.mCropRect.p_fractional.x   = 0;
    crop.mCropRect.p_fractional.y   = 0;
    crop.mCropRect.p_integral.x     = 0;
    crop.mCropRect.p_integral.y     = 0;
    crop.mCropRect.s.w              = inputSize.w;
    crop.mCropRect.s.h              = inputSize.h;
    crop.mResizeDst.w               = inputSize.w;
    crop.mResizeDst.h               = inputSize.h;
    crop2.mCropRect.p_fractional.x  = 0;
    crop2.mCropRect.p_fractional.y  = 0;
    crop2.mCropRect.p_integral.x    = 0;
    crop2.mCropRect.p_integral.y    = 0;
    crop2.mCropRect.s.w             = inputSize.w;
    crop2.mCropRect.s.h             = inputSize.h;
    crop2.mResizeDst.w              = inputSize.w;
    crop2.mResizeDst.h              = inputSize.h;
    crop3.mCropRect.p_fractional.x  = 0;
    crop3.mCropRect.p_fractional.y  = 0;
    crop3.mCropRect.p_integral.x    = 0;
    crop3.mCropRect.p_integral.y    = 0;
    crop3.mCropRect.s.w             = inputSize.w;
    crop3.mCropRect.s.h             = inputSize.h;
    crop3.mResizeDst.w              = inputSize.w;
    crop3.mResizeDst.h              = inputSize.h;

    frameParams.mvCropRsInfo.push_back(crop);
    frameParams.mvCropRsInfo.push_back(crop2);
    frameParams.mvCropRsInfo.push_back(crop3);

    // Output buffer
    NSCam::NSIoPipe::Output dst;
    // Stand alone
    dst.mPortID         = PORT_WPEO;
    dst.mBuffer         = out;
    dst.mPortID.group   = 0;
    frameParams.mvOut.push_back(dst);

    // Warping parameter
    //NSCam::NSIoPipe::NSWpe::WPEQParams enqueWpeParams;
    // TODO: We should remove hard code here
    enqueWpeParams.warp_veci_info.virtAddr          = gridXBuffer->getBufVA(0);
    enqueWpeParams.warp_veci_info.phyAddr           = gridXBuffer->getBufPA(0);
    enqueWpeParams.warp_veci_info.bus_size          = NSCam::NSIoPipe::NSWpe::WPE_BUS_SIZE_32_BITS;
    enqueWpeParams.warp_veci_info.stride            = 0x00000918; // because it is four byte
    enqueWpeParams.warp_veci_info.width             = 0x246; //582
    enqueWpeParams.warp_veci_info.height            = 0x1B5; //437
    enqueWpeParams.warp_veci_info.veci_v_flip_en    = 0;
    enqueWpeParams.warp_veci_info.addr_offset       = 0;

    enqueWpeParams.warp_vec2i_info.virtAddr         = gridYBuffer->getBufVA(0);
    enqueWpeParams.warp_vec2i_info.phyAddr          = gridYBuffer->getBufPA(0);
    enqueWpeParams.warp_vec2i_info.bus_size         = NSCam::NSIoPipe::NSWpe::WPE_BUS_SIZE_32_BITS;
    enqueWpeParams.warp_vec2i_info.stride           = 0x00000918;
    enqueWpeParams.warp_vec2i_info.width            = 0x246;
    enqueWpeParams.warp_vec2i_info.height           = 0x1B5;
    enqueWpeParams.warp_vec2i_info.veci_v_flip_en   = 0;
    enqueWpeParams.warp_vec2i_info.addr_offset      = 0;

    // Indicate stand alone
    enqueWpeParams.wpe_mode = 0;

    enqueWpeParams.vgen_hmg_mode = 0;

    MCrpRsInfo vgenCropInfo;
    vgenCropInfo.mCropRect.p_integral.x = 0;
    vgenCropInfo.mCropRect.p_integral.y = 0;
    vgenCropInfo.mCropRect.p_fractional.x = 0;
    vgenCropInfo.mCropRect.p_fractional.y = 0;//0x00C16FFE;

    enqueWpeParams.mwVgenCropInfo.push_back(vgenCropInfo);

    enqueWpeParams.wpecropinfo.x_start_point = 0;
    enqueWpeParams.wpecropinfo.y_start_point = 0;
    enqueWpeParams.wpecropinfo.x_end_point = inputSize.w - 1;
    enqueWpeParams.wpecropinfo.y_end_point = inputSize.h - 1;

    enqueWpeParams.tbl_sel_v = 24;
    enqueWpeParams.tbl_sel_h = 24;

    enqueWpeParams.psp_border_color = 0x01000000;//bit24 en, 0~23 border

    enqueWpeParams.extra_feature_index = NSCam::NSIoPipe::NSWpe::EWPE_AINR;

    //NSCam::NSIoPipe::NSWpe::WPEAINRParams wpe_AINRparams;
    wpe_AINRparams.AINR_line_mode = mode; //0: odd line  for  RG, 1: even line for GB
    enqueWpeParams.extra_feature_params =(void *) &(wpe_AINRparams);

    NSCam::NSIoPipe::ExtraParam eParams;

    eParams.CmdIdx = NSCam::NSIoPipe::EPIPE_WPE_INFO_CMD;
    eParams.moduleStruct = (void *) &(enqueWpeParams);

    frameParams.mvExtraParam.push_back(eParams);

    qParams.mvFrameParams.push_back(frameParams);
    return AinrErr_Ok;
}

void AinrWarping::bufferDump(IMetadata *halMeta, IImageBuffer* buff, int index, int ispProfle, const char* fileType) {
    // dump input buffer
    String8                 fileResultName;
    char                    pre_filename[512];
    // file dump hint
    NSCam::TuningUtils::FILE_DUMP_NAMING_HINT   dumpNamingHint;

    if(!halMeta) {
        ainrLogE("HalMeta is nullptr, dump fail");
        return;
    }

    extract(&dumpNamingHint, halMeta);
    dumpNamingHint.SensorDev = m_sensorId;
    dumpNamingHint.IspProfile = ispProfle; //EIspProfile_Capture;
    genFileName_TUNING(pre_filename, sizeof(pre_filename), &dumpNamingHint);

    fileResultName = String8::format("%s_%d_%d_%dx%d.%s"
        , pre_filename
        , index
        , 0
        , buff->getImgSize().w
        , buff->getImgSize().h
        , fileType);
    buff->saveToFile(fileResultName);
}
