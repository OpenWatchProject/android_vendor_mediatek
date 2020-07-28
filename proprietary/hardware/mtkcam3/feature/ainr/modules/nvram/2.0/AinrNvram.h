/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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
#ifndef __AINRNVRAM_H__
#define __AINRNVRAM_H__

// AINR Core Lib
#include <mtkcam3/feature/ainr/IAinrNvram.h>

// MTKCAM
#include <camera_custom_nvram.h>
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>

// ISP profile/EModule_T
#include <tuning_mapping/cam_idx_struct_ext.h>

// STL
#include <memory>

using android::sp;

namespace ainr {

class AinrNvram : public IAinrNvram {
public:
    AinrNvram();

/* implementation from IAinrNvram */
public:
    virtual enum AinrErr init(int sensorId);
    virtual enum AinrErr init(IAinrNvram::ConfigParams& params);
    virtual const std::shared_ptr<char>
            chunk(size_t *bufferSize = nullptr) const;
    virtual const std::shared_ptr<char>
            chunkAinrTh(size_t *bufferSize = nullptr) const;
    virtual const char* getChunk(size_t *bufferSize = nullptr) const;
    virtual const char* getChunkAinrTh(size_t *bufferSize = nullptr) const;
    virtual const char* getSpecificChunk(const nvram_hint hint,
            size_t* bufferSize = nullptr) const;
    virtual ~AinrNvram(void);

private:
    int32_t doQueryIndex(uint32_t sensorDev, EModule_T moduleId);
    enum AinrErr loadNvramChunk(IAinrNvram::ConfigParams& params);
    std::shared_ptr<char> loadInterpolationChunk(int32_t openId,
            NSIspTuning::EModule_T moduleId, size_t size, int32_t iso);

/* Class members */
private:
    // Kery (EModule_T), Value (std::shared_ptr<char>)
    std::unordered_map<int32_t, std::shared_ptr<char>> m_tuning_map;

    // ISP profile
    NSIspTuning::EIspProfile_T m_ispProfile;
};
};  // namespace ainr
#endif  //__AINRNVRAM_H__
