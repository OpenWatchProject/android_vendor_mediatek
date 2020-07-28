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
#define LOG_TAG "MtkBuiltinOpResolver"
#include "tensorflow/lite/mtk/kernels/mtk_register.h"
#include "tensorflow/lite/mtk/mtk_minimal_logging.h"
#include "tensorflow/lite/util.h"

namespace tflite {
namespace ops {

namespace mtk {
TfLiteRegistration* Register_MTK_ABS();
TfLiteRegistration* Register_MTK_AXIS_ALIGNED_BBOX_TRANSFORM();
TfLiteRegistration* Register_MTK_BOX_WITH_NMS_LIMIT();
TfLiteRegistration* Register_MTK_CHANNEL_SHUFFLE();
TfLiteRegistration* Register_MTK_CROP_AND_RESIZE();
TfLiteRegistration* Register_MTK_DEPTH_TO_SPACE();
TfLiteRegistration* Register_MTK_ELU();
TfLiteRegistration* Register_MTK_GENERATE_PROPOSALS();
TfLiteRegistration* Register_MTK_LAYER_NORMALIZATION();
TfLiteRegistration* Register_MTK_OPT();
TfLiteRegistration* Register_MTK_QUANTIZE_REF();
TfLiteRegistration* Register_MTK_REQUANTIZE();
TfLiteRegistration* Register_MTK_REVERSE();
TfLiteRegistration* Register_MTK_ROI_ALIGN();
TfLiteRegistration* Register_MTK_TRANSPOSE_CONV_REF();
}  // namespace mtk

namespace builtin {
MtkBuiltinOpResolver::MtkBuiltinOpResolver() {
  BuiltinOpResolver::AddCustom("MTK_ABS",
                              mtk::Register_MTK_ABS());
  BuiltinOpResolver::AddCustom("MTK_AXIS_ALIGNED_BBOX_TRANSFORM",
                              mtk::Register_MTK_AXIS_ALIGNED_BBOX_TRANSFORM());
  BuiltinOpResolver::AddCustom("MTK_BOX_WITH_NMS_LIMIT",
                              mtk::Register_MTK_BOX_WITH_NMS_LIMIT());
  BuiltinOpResolver::AddCustom("MTK_CHANNEL_SHUFFLE",
                              mtk::Register_MTK_CHANNEL_SHUFFLE());
  BuiltinOpResolver::AddCustom("MTK_CROP_AND_RESIZE",
                              mtk::Register_MTK_CROP_AND_RESIZE());
  BuiltinOpResolver::AddCustom("MTK_DEPTH_TO_SPACE",
                              mtk::Register_MTK_DEPTH_TO_SPACE());
  BuiltinOpResolver::AddCustom("MTK_ELU",
                              mtk::Register_MTK_ELU());
  BuiltinOpResolver::AddCustom("MTK_GENERATE_PROPOSALS",
                              mtk::Register_MTK_GENERATE_PROPOSALS());
  BuiltinOpResolver::AddCustom("MTK_LAYER_NORMALIZATION",
                              mtk::Register_MTK_LAYER_NORMALIZATION());
  BuiltinOpResolver::AddCustom("MTK_OPT",
                              mtk::Register_MTK_OPT());
  BuiltinOpResolver::AddCustom("MTK_QUANTIZE",
                              mtk::Register_MTK_QUANTIZE_REF());
  BuiltinOpResolver::AddCustom("MTK_REQUANTIZE",
                              mtk::Register_MTK_REQUANTIZE());
  BuiltinOpResolver::AddCustom("MTK_REVERSE",
                              mtk::Register_MTK_REVERSE());
  BuiltinOpResolver::AddCustom("MTK_ROI_ALIGN",
                              mtk::Register_MTK_ROI_ALIGN());
  BuiltinOpResolver::AddCustom("MTK_TRANSPOSE_CONV",
                              mtk::Register_MTK_TRANSPOSE_CONV_REF());
}

}  // namespace builtin
}  // namespace ops
}  // namespace tflite
