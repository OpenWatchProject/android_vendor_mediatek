/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

#ifndef MEDIATEK_ML_NN_RUNTIME_SPORTS_MODE_H
#define MEDIATEK_ML_NN_RUNTIME_SPORTS_MODE_H

#include <vector>
#include <binder/IServiceManager.h>

namespace android {
namespace nn {

class MtkModelBuilder;
class Memory;

#define BENCHMARK_MODEL_COUNT 2
#define BENCHMARK_SCAN_COMPLETE 3

#define STAGE_FOUND_PATTERN 127
#define STAGE_FOUND_NOT_PATTERN -1
#define STAGE_INITIAL_STAGE 0

#define SPORTS_MODE_NOT_SURE  0
#define SPORTS_MODE_TURN_OFF  1
#define SPORTS_MODE_TURN_ON   2

#define BUFFER_SOURCE_POINTER   0
#define BUFFER_SOURCE_MEMORYT   1

struct SportModeBufferInfo {
    int source;
    void* buffer;
    size_t offset;
    size_t length;
};

class MtkSportsMode {
public:
    MtkSportsMode();
    void compareModels(MtkModelBuilder* model, int modelIdx);
    void setInput(uint32_t inputIndex, const void* buffer, size_t length);
    void setInputFromMemory(
            uint32_t inputIndex, const Memory* memory, size_t offset, size_t length);

    void comparePatterns(uint32_t inputIndex, SportModeBufferInfo info);

    int getSportsMode();
    void notifySportsMode();
    std::vector<int8_t> getStages() { return mStages; }

private:
    android::sp<android::IBinder> mPowerHalMgrBinder = nullptr;
    std::vector<int8_t> mStages;
    bool mNotify = false;
    int mSportMode = SPORTS_MODE_NOT_SURE;
};

}  // namespace nn
}  // namespace android

#endif  // MEDIATEK_ML_NN_RUNTIME_SPORTS_MODE_H
