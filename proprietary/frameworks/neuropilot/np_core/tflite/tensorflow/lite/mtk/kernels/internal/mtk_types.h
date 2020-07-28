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

#ifndef MTK_TENSORFLOW_CONTRIB_LITE_KERNELS_INTERNAL_TYPES_H_
#define MTK_TENSORFLOW_CONTRIB_LITE_KERNELS_INTERNAL_TYPES_H_

#include "tensorflow/lite/kernels/internal/common.h"

#include <cstring>
#include <iterator>

namespace tflite {

struct MtkCropAndResizeParams {
  float box_scale;
  int32 box_offset;
};

struct MtkRoiAlignParams {
  int kernel_height;
  int kernel_width;
  float box_scale;
  int32 box_offset;
};

struct MtkQuantizedEluParams {
  int input_range_radius;
  int32 input_offset;
  int32 output_offset;
  int32 input_multiplier;
  int input_shift;
  int32 output_multiplier;
  int output_shift;
  int32 quantized_one;
  int32 quantized_activation_min;
  int32 quantized_activation_max;
};

struct MtkRequantizeParams {
  int left_shift;
  int32 input_offset;
  int32 output_offset;
  int32 output_multiplier;
  int output_shift;
};

struct MtkQuantizeParams {
  int32 zero_point;
  float scale;
};

struct MtkQuantizedAbsParams {
  int32 input_offset;
  int32 output_offset;
  int32 output_multiplier;
  int output_shift;
  int32 quantized_activation_min;
  int32 quantized_activation_max;
};

struct MtkQuantizedPreluParams {
  int32 input_offset;
  int32 alpha_offset;
  int32 output_offset;
  int32 pos_output_multiplier;
  int pos_output_shift;
  int32 neg_output_multiplier;
  int neg_output_shift;
  int32 quantized_activation_min;
  int32 quantized_activation_max;
};

struct MtkChannelShuffleParams {
  int num_groups;
};

struct PerChannelConvParams {
  PaddingType padding_type;
  PaddingValues padding_values;
  int16 stride_width;
  int16 stride_height;
  int16 dilation_width_factor;
  int16 dilation_height_factor;
  // uint8 inference params.
  int32 input_offset;
  std::vector<int32> weights_offsets;
  int32 output_offset;
  std::vector<int32> output_multipliers;
  std::vector<int> output_shifts;
  // uint8, etc, activation params.
  int32 quantized_activation_min;
  int32 quantized_activation_max;
};

}  // namespace tflite

#endif  // MTK_TENSORFLOW_CONTRIB_LITE_KERNELS_INTERNAL_TYPES_H_
