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
#ifndef __MFLLNVRAM_H__
#define __MFLLNVRAM_H__

// MFNR Core Lib
#include <mtkcam3/feature/mfnr/IMfllNvram.h>

// MTKCAM
/*To-Do: remove when NVRAM 2.0 is all done*/
#define ENABLE_NEW_NVRAM_FEATURE 1
/*To-Do: remove when cam_idx_mgr.h reorder camear_custom_nvram.h order before isp_tuning_cam_info.h */
#include <camera_custom_nvram.h>
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>

// AOSP
#include <utils/Mutex.h>

// STL
#include <memory>

using android::sp;
using android::Mutex;

namespace mfll {

class MfllNvram : public IMfllNvram {
public:
    MfllNvram();

/* implementation from IMfllNvram */
public:
    virtual enum MfllErr init(int sensorId);
    virtual enum MfllErr init(IMfllNvram::ConfigParams& params);
    virtual std::shared_ptr<char> chunk(size_t *bufferSize = nullptr);
    virtual std::shared_ptr<char> chunkMfnr(size_t *bufferSize = nullptr);
    virtual std::shared_ptr<char> chunkMfnrTh(size_t *bufferSize = nullptr);
    virtual const char* getChunk(size_t *bufferSize = nullptr);
    virtual const char* getChunkMfnr(size_t *bufferSize = nullptr);
    virtual const char* getChunkMfnrTh(size_t *bufferSize = nullptr);

/* Attributes */
private:
    std::shared_ptr<char> m_nvramChunkMfnr;
    std::shared_ptr<char> m_nvramChunkMfnrTh;
    Mutex m_mutex;

protected:
    virtual ~MfllNvram(void);
};
}; /* namespace mfll */
#endif//__MFLLNVRAM_H__
