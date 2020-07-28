/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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
#ifndef __MFLLEXIFINFO_PLATFORM_H__
#define __MFLLEXIFINFO_PLATFORM_H__

#include <mtkcam3/feature/mfnr/IMfllGyro.h>

#include <cstdint>

#include <custom/debug_exif/dbg_exif_param.h>

#if ((MFLL_MF_TAG_VERSION > 0) && (MFLL_MF_TAG_VERSION != 13))
#   error "MT6785 only supports MF_TAG_VERSION 13"
#endif


enum MfllErr MfllExifInfo::updateInfo(IMfllNvram *pNvram)
{
    size_t chunkSize;
    const char *pChunk = pNvram->getChunk(&chunkSize);
    if (pChunk == NULL) {
        mfllLogE("get NVRAM chunk failed");
        return MfllErr_UnexpectedError;
    }
    char *pMutableChunk = const_cast<char*>(pChunk);

    /* reading NVRAM */
    NVRAM_CAMERA_FEATURE_MFLL_STRUCT* n = reinterpret_cast<NVRAM_CAMERA_FEATURE_MFLL_STRUCT*>(pMutableChunk);
#if 0 //(MFLL_MF_TAG_VERSION > 0)
    using namespace __namespace_mf(MFLL_MF_TAG_VERSION);

    m_dataMap[MF_TAG_MFNR_ISO_TH]                   = n->mfll_iso_th;
    m_dataMap[MF_TAG_AIS_ISO_LEVEL1]                = n->iso_level1;
    m_dataMap[MF_TAG_AIS_ISO_LEVEL2]                = n->iso_level2;
    m_dataMap[MF_TAG_AIS_ISO_LEVEL3]                = n->iso_level3;
    m_dataMap[MF_TAG_AIS_ISO_LEVEL4]                = n->iso_level4;
    m_dataMap[MF_TAG_AIS_ISO_LEVEL5]                = n->iso_level5;
    m_dataMap[MF_TAG_AIS_ISO_LEVEL6]                = n->iso_level6;
    m_dataMap[MF_TAG_AIS_ISO_LEVEL7]                = n->iso_level7;
    m_dataMap[MF_TAG_AIS_FRAME_NUM1]                = n->frame_num1;
    m_dataMap[MF_TAG_AIS_FRAME_NUM2]                = n->frame_num2;
    m_dataMap[MF_TAG_AIS_FRAME_NUM3]                = n->frame_num3;
    m_dataMap[MF_TAG_AIS_FRAME_NUM4]                = n->frame_num4;
    m_dataMap[MF_TAG_AIS_FRAME_NUM5]                = n->frame_num5;
    m_dataMap[MF_TAG_AIS_FRAME_NUM6]                = n->frame_num6;
    m_dataMap[MF_TAG_AIS_FRAME_NUM7]                = n->frame_num7;
    m_dataMap[MF_TAG_AIS_FRAME_NUM8]                = n->frame_num8;
#endif
    return MfllErr_Ok;
}

enum MfllErr MfllExifInfo::updateInfo(const MfllCoreDbgInfo_t &dbgInfo __attribute__((unused)))
{
#if (MFLL_MF_TAG_VERSION > 0)
    using namespace __namespace_mf(MFLL_MF_TAG_VERSION);

    std::unique_lock<std::mutex> _l(m_mutex);
    m_dataMap[MF_TAG_CAPTURE_M] = dbgInfo.frameMaxCapture;
    m_dataMap[MF_TAG_BLENDED_N] = dbgInfo.frameMaxBlend;
    m_dataMap[MF_TAG_MAX_FRAME_NUMBER] = dbgInfo.frameCapture;
    m_dataMap[MF_TAG_PROCESSING_NUMBER] = dbgInfo.frameBlend;

    /* iso & exposure */
    m_dataMap[MF_TAG_EXPOSURE] = dbgInfo.exp;
    m_dataMap[MF_TAG_ISO] = dbgInfo.iso;

    m_dataMap[MF_TAG_RAW_WIDTH] = dbgInfo.width;
    m_dataMap[MF_TAG_RAW_HEIGHT] = dbgInfo.height;
    m_dataMap[MF_TAG_BLD_YUV_WIDTH] = dbgInfo.blend_yuv_width;
    m_dataMap[MF_TAG_BLD_YUV_HEIGHT] = dbgInfo.blend_yuv_height;
    m_dataMap[MF_TAG_BSS_ENABLE] = dbgInfo.bss_enable;
    m_dataMap[MF_TAG_MEMC_SKIP] = dbgInfo.memc_skip;
    m_dataMap[MF_TAG_MFB_MODE] = dbgInfo.shot_mode;
#endif
    return MfllErr_Ok;
}


// ----------------------------------------------------------------------------
// extensions, for sendCommand(const std::string&, const std::deque<void*>&)
//
const std::unordered_map<std::string, std::function<enum MfllErr(IMfllExifInfo*, const std::deque<void*>&)>>*
getExtFunctionMap()
{
    static std::unordered_map<std::string, std::function<enum MfllErr(IMfllExifInfo*, const std::deque<void*>&)>>
        sExtFunctions = {
            {
                "ADD_GYRO_INFO",
                []
                (
                    IMfllExifInfo* pExifInfo            __attribute__((unused)),
                    const std::deque<void*> dataset     __attribute__((unused))
                )
                -> enum MfllErr
                {

#if 0 //(MFLL_MF_TAG_VERSION > 0)
                    using namespace __namespace_mf(MFLL_MF_TAG_VERSION);

                    // datatype of container
                    typedef std::deque<std::deque<IMfllGyro::MfllGyroInfo>> T; // data type

                    if (dataset.size() <= 0)
                        return MfllErr_BadArgument;

                    const T* pGyroInfoStack = reinterpret_cast<T*>(dataset[0]);

                    // we has 6 frames' gyro info
                    for (size_t i = 0; i < 6; i++) {
                        bool bValid = false;

                        // move keys
                        unsigned int step = 3 * MFLL_GYRO_NUMBER_PER_FRAME * i; // moves to the next x, each frame has MFLL_GYRO_NUMBER_PER_FRAME gyro info
                        unsigned int key_x = (MF_TAG_GYRO_X0_00 + step);
                        unsigned int key_y = (MF_TAG_GYRO_Y0_00 + step);
                        unsigned int key_z = (MF_TAG_GYRO_Z0_00 + step);

                        if (i < pGyroInfoStack->size()) {
                            const std::deque<IMfllGyro::MfllGyroInfo>& qGyroInfo = pGyroInfoStack->at(i);

                            // if there's no any gyro info of the frame, add invalid key
                            if (qGyroInfo.size() < MFLL_GYRO_NUMBER_PER_FRAME) {
                                bValid = false;
                            }
                            else {
                                // convert const float& to uint32_t as HEX
                                auto CONST_FLOAT_TO_HEX_UINT = [](const float& f) -> uint32_t {
                                    uint32_t rVal32 = *reinterpret_cast<uint32_t*>(
                                            &const_cast<float&>(f) // cast non-const
                                        );
                                    return rVal32;
                                };

                                for (size_t j = 0; j < MFLL_GYRO_NUMBER_PER_FRAME; j++) {
                                    pExifInfo->updateInfo(key_x + (j * 3), CONST_FLOAT_TO_HEX_UINT(qGyroInfo[j].vector.x()));
                                    pExifInfo->updateInfo(key_y + (j * 3), CONST_FLOAT_TO_HEX_UINT(qGyroInfo[j].vector.y()));
                                    pExifInfo->updateInfo(key_z + (j * 3), CONST_FLOAT_TO_HEX_UINT(qGyroInfo[j].vector.z()));
                                }
                                bValid = true;
                            }
                        }

                        // fill up invalid value if necessary
                        if (bValid == false) {
                            for (size_t j = 0; j < 3; j++) {
                                pExifInfo->updateInfo(key_x + (j * 3), 0xFFFFFFFF);
                                pExifInfo->updateInfo(key_y + (j * 3), 0xFFFFFFFF);
                                pExifInfo->updateInfo(key_z + (j * 3), 0xFFFFFFFF);
                            }
                        }
                    }
#endif
                    return MfllErr_Ok;
                }
            }
        };

    return &sExtFunctions;
}

#endif//__MFLLEXIFINFO_PLATFORM_H__
