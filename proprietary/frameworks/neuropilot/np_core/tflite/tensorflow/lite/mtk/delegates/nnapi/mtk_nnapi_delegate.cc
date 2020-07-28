/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
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

#define LOG_TAG "MtkNnApiDelegateBuilder"
#include "tensorflow/lite/nnapi/nnapi_implementation.h"
#include "tensorflow/lite/mtk/mtk_minimal_logging.h"
#include "tensorflow/lite/mtk/delegates/nnapi/mtk_nnapi_delegate.h"
#include "tensorflow/lite/mtk/delegates/nnapi/mtk_nnapi_delegate_p.h"
#include "tensorflow/lite/mtk/delegates/nnapi/mtk_nnapi_delegate_q.h"

namespace tflite {

constexpr int32_t kMinSdkVersionForNNAPI = 27;
constexpr int32_t kMinSdkVersionForNNAPI11 = 28;
constexpr int32_t kMinSdkVersionForNNAPI12 = 29;

TfLiteStatus MtkNnApiDelegateBuilder::operator()(
    std::unique_ptr<MtkNnApiDelegate>* delegate) {
  const NnApi* nnapi = NnApiImplementation();

  // M: ALPS04679427: Distinguish Delegate by Hal Version @{
  uint32_t numDevices = 0;
  int64_t featureLevel = kMinSdkVersionForNNAPI11;
  int64_t tempFeatureLevel = 0;
  int32_t androidSdkVersion = mtk::GetAndroidSdkVersionCached();
  if (androidSdkVersion >= kMinSdkVersionForNNAPI12) {
    // Check device count and feature level if current Android version
    // is higher than API level 29
    if (nnapi->ANeuralNetworks_getDeviceCount(&numDevices)
          == ANEURALNETWORKS_NO_ERROR) {
        for (uint32_t i=0; i < numDevices; i++) {
          ANeuralNetworksDevice *device = nullptr;
          if (nnapi->ANeuralNetworks_getDevice(i, &device)
                == ANEURALNETWORKS_NO_ERROR) {
              if (nnapi->ANeuralNetworksDevice_getFeatureLevel(device,
                  &tempFeatureLevel) == ANEURALNETWORKS_NO_ERROR) {
                  if (tempFeatureLevel > featureLevel &&
                     tempFeatureLevel < __ANDROID_API_FUTURE__) {
                      featureLevel = tempFeatureLevel;
                  }
              }
          }
        }
    }
  }

  // Overwrite featureLevel if necessary.(for debug or test purpose)
  tempFeatureLevel = mtk::GetTargetNnApiLevel();
  if (tempFeatureLevel != 0) {
    featureLevel = tempFeatureLevel;
  }

  // M: @}
  if (featureLevel < kMinSdkVersionForNNAPI ||
      !nnapi->nnapi_exists) {
      TFLITE_MTK_LOG_INFO("Create NNAPI dummy delegate");
      delegate->reset(new MtkNnApiDelegate());
  } else {
    switch (featureLevel) {
      case kMinSdkVersionForNNAPI11:
        TFLITE_MTK_LOG_INFO("Create NNAPI delegate P");
        delegate->reset(new MtkNnApiDelegateP());
        break;
      case kMinSdkVersionForNNAPI12:
        TFLITE_MTK_LOG_INFO("Create NNAPI delegate Q");
        delegate->reset(new MtkNnApiDelegateQ());
        break;
      default:
        break;
    }
  }
  return kTfLiteOk;
}

}  // namespace tflite
