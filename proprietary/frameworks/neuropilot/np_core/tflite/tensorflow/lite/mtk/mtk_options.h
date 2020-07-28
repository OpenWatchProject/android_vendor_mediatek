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

#ifndef MTK_TENSORFLOW_LITE_OPTIONS_H_
#define MTK_TENSORFLOW_LITE_OPTIONS_H_

#include "tensorflow/lite/mtk/mtk_utils.h"

namespace tflite {
namespace mtk {

inline bool IsDumpOpSupported() {
  static bool enabled =
    tflite::mtk::PropertyGetBool("debug.mtk_tflite.DumpOp",
                                false);
  return enabled;
}

inline bool IsPrintLogSupported() {
  static bool enabled = tflite::mtk::PropertyGetBool("debug.mtk_tflite.vlog",
                                                    false);
  return enabled;
}

inline int GetOpDumpFolder(char** folder_path) {
  return PropertyGet("debug.mtk_tflite.dump_folder", folder_path);
}

inline int GetOpDumpPath(char** path) {
  return PropertyGet("debug.mtk_tflite.dump_path", path);
}

inline bool IsDynamicTensorSupported() {
  static bool enabled =
    tflite::mtk::PropertyGetBool("debug.mtk_tflite.allow_dynamic_tensor",
                                false);
  return enabled;
}

inline bool IsUnknownDimensionTensorSupported() {
  static bool enabled =
    tflite::mtk::PropertyGetBool(
        "debug.mtk_tflite.allow_unknown_dimension_tensor", true);
  return enabled;
}

inline bool IsForceUseNnApi() {
  static bool enabled =
    tflite::mtk::PropertyGetBool(
        "debug.mtk_tflite.force_use_nnapi", false);
  return enabled;
}

inline int GetTargetNnApiLevel() {
  return tflite::mtk::PropertyGetInt("debug.mtk_tflite.target_nnapi", 0);
}

} // namespace mtk
} // namespace tflite

#endif  // MTK_TENSORFLOW_LITE_OPTIONS_H_
