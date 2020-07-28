/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef _MTK_HARDWARE_PIPELINE_EXTENSION_UTIL_RAW2YUVENCODER_H_
#define _MTK_HARDWARE_PIPELINE_EXTENSION_UTIL_RAW2YUVENCODER_H_

// MTKCAM
#include <mtkcam/def/BuiltinTypes.h> // MINT32, MUINT32 ...
#include <mtkcam/def/Errors.h> // MERROR
#include <mtkcam/def/UITypes.h> // MRect, MSize, MPoint
#include <mtkcam/utils/imgbuf/IImageBuffer.h> // IImageBuffer
#include <mtkcam/utils/metadata/IMetadata.h> // IMetadata
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h> // INormalStream, ENormalStreamTag_Normal...
#include <mtkcam/aaa/IHal3A.h> // setIsp ...

// STL
#include <memory>
#include <functional>
#include <mutex>

// [Compile Pass2Encoder with vtable or not]
//
//  W/o vtable that may reduce time for searching virtual table, may have a litte
//  performance speed up
#define _ENABLE_PASS2ENCODER_VTABLE_  0

#if defined(_P2ENC_VIRTUAL)
#   error "Symbols has beed duplicated defined."
#endif

#if _ENABLE_PASS2ENCODER_VTABLE_
#   define  _P2ENC_VIRTUAL  virtual
#else
#   define  _P2ENC_VIRTUAL
#endif

namespace NSCam {
namespace plugin {

// Raw2Yuv Encoder is an utility to encode a pure RAW image to YUV, or some other
// specific operation using P2 driver.
//
// For common usage: encode a pure RAW to YUV,
// Caller need to prepare:
//  1. RAW
//  2. 3A info (says Hal metadata, IMetadata)
//  3. Output buffer
//  4. (optional) cropWindow of source image
//  5. (optional) rotation of output buffer
//  4. (optional) IspProfile
//
// Note: All methods in this class are thread-safe.
class Pass2Encoder {
public:
    struct ConfigRaw2Yuv
    {
        // input
        IImageBuffer*   pSrcRaw; // [MUST]
        IImageBuffer*   pLcsoRaw; // [optional, can be NULL]
        IMetadata*      pAppMeta; // [MUST]
        IMetadata*      pHalMeta; // [MUST]

        // output
        IImageBuffer*   pOutput; // [MUST]
        IImageBuffer*   pOutput2;    // [optional]
        IMetadata*      pOutAppMeta; // [optional]
        IMetadata*      pOutHalMeta; // [optional]

        // configs
        MBOOL           isZsd; // [optional]
        MINT32          ispProfile; // [optional, default is EIspProfile_Capture]
        MINT32          rotation1; // [optional, rotation cannot be assign the the same time]
        MINT32          rotation2; // [optional]
        MRect           cropWindow1; // [optional]
        MRect           cropWindow2; // [optional]

        ConfigRaw2Yuv() noexcept;
        bool isValid() const noexcept;
    };


public:
    // Encode pure RAW to YUV.
    //  @param cfg              ConfigRaw2Yuv, at least, these arguments should
    //                          be given:
    //                            - ConfigRaw2Yuv::pSrcRaw
    //                            - ConfigRaw2Yuv::pAppMeta
    //                            - ConfigRaw2Yuv::pHalMeta
    //                            - ConfigRaw2Yuv::pOutput
    //  @return                 If encode successfully, returns OK.
    //  @note                   This method is synchronous call.
    _P2ENC_VIRTUAL MERROR       encodeRaw2Yuv(const ConfigRaw2Yuv* cfg) const;


//
// Attributes
//
protected:
    MINT32  m_sensorId;
    size_t  m_regTableSize;

    typedef NSCam::NSIoPipe::NSPostProc::INormalStream INormalStream_T;

    std::unique_ptr< INormalStream_T, std::function<void(INormalStream_T*)> >
        m_pNormalStream;

    mutable std::mutex m_pNormalStreamLock;

    std::unique_ptr< NS3Av3::IHal3A, std::function<void(NS3Av3::IHal3A*)> >
        m_pHal3A;

    mutable std::mutex m_pHal3ALock;


public:
    Pass2Encoder(MINT32 sensorId);
    _P2ENC_VIRTUAL ~Pass2Encoder();


}; // Raw2YuvEncoder
}; // namespace plugin
}; // namespace NSCam
#endif // _MTK_HARDWARE_PIPELINE_EXTENSION_UTIL_RAW2YUVENCODER_H_
