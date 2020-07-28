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

#include <time.h>
#include <gtest/gtest.h>
#include <IImageBuffer.h>
#include <utils/Vector.h>
#include <utils/String8.h>
#include <utils/include/ImageBufferHeap.h>
#include <mtkcam/feature/effectHalBase/EffectRequest.h>

#include <mtkcam/feature/stereo/pipe/IBMDeNoisePipe.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "BMDeNoisePipe_UT"

#define MY_LOGD(fmt, arg...)    printf("[D][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGI(fmt, arg...)    printf("[I][%s]" fmt"\n", __func__, ##arg)
#define MY_LOGW(fmt, arg...)    printf("[W][%s] WRN(%5d):" fmt"\n", __func__, __LINE__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("[E][%s] %s ERROR(%5d):" fmt"\n", __func__,__FILE__, __LINE__, ##arg)

namespace NSCam{
namespace NSCamFeature{
namespace NSFeaturePipe{

void onEffectRequestFinished(MVOID* tag, String8 status, sp<EffectRequest>& request)
{
	MY_LOGD("onEffectRequestFinished + ");
	MY_LOGD("onEffectRequestFinished - ");
}

TEST(BMDeNoisePipeTest, BASIC_TEST)
{
    MY_LOGD("BASIC_TEST + ");

    MINT32 openSensorIndex = 0;
    MINT32 runPath = 0;
    sp<IBMDeNoisePipe> myBMDeNoisePipe = IBMDeNoisePipe::createInstance(openSensorIndex, runPath);

    myBMDeNoisePipe->setFlushOnStop(MTRUE);

    EXPECT_EQ(true, myBMDeNoisePipe->init()) << "myBMDeNoisePipe->init";

    sp<EffectRequest> enque_req = new EffectRequest(0, onEffectRequestFinished, (void*)this);

    // crashed because:
    // 01-01 00:15:38.852824  2972  2972 E MtkCam/FeaturePipe [CamGraph:: init]mRootNode not set (init){#169:./vendor/mediatek/proprietary/hardware/mtkcam/feature/core/featurePipe/core/include/CamGraph.h}
    EXPECT_EQ(true, myBMDeNoisePipe->enque(enque_req)) << "myBMDeNoisePipe->enque(enque_req)";

    MY_LOGD("BASIC_TEST - ");
}

}; // namespace NSFeaturePipe
}; // namespace NSCamFeature
}; // namespace NSCam