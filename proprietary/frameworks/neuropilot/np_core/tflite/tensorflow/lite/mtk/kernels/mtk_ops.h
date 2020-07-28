/* Copyright 2018 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/
#ifndef TENSORFLOW_CONTRIB_LITE_KERNELS_MTK_MTK_OPS_H_
#define TENSORFLOW_CONTRIB_LITE_KERNELS_MTK_MTK_OPS_H_

namespace tflite {
namespace ops {
namespace mtk {
namespace depth_to_space {

struct OpData {
  int64_t block_size;
};

}  // namespace depth_to_space

namespace elu {

struct OpData {
  int32_t input_multiplier = 0;
  int input_left_shift = 0;
  int32_t output_multiplier = 0;
  int output_left_shift = 0;
  int32_t input_range_radius = 0;
  int diff_min = 0;
  //Added by MDLA calculating scale with shifting bits
  int new_output_shift;
};

}  // namespace elu

namespace pooling {

struct OpData {
  TfLitePadding padding = kTfLitePaddingUnknown;
  TfLitePaddingValues paddingValues;
  int32_t stride_width = 0;
  int32_t stride_height = 0;
  int32_t kwidth = 0;
  int32_t kheight = 0;
  TfLiteFusedActivation activation = kTfLiteActNone;
};

}  // namespace pooling

namespace requantize {

struct OpData {
  int32_t input_offset;
  int32_t output_offset;
  int32_t left_shift;
  // The scaling factor from input to output (aka the 'real multiplier') can
  // be represented as a fixed point multipler plus a left shift.
  int32_t output_multiplier;
  int output_shift;
};

}  // namespace requantize

namespace crop_and_resize {

struct OpData {
  float extrapolation_value = 0;
};

}  // namespace crop_and_resize

namespace roi_align {

struct OpData {
  float extrapolation_value = 0;
  int kheight = 0;
  int kwidth = 0;
};

}  // namespace roi_align

namespace transpose_conv {

struct OpData {
  TfLitePadding padding = kTfLitePaddingUnknown;
  TfLitePaddingValues paddingValues;
  int32_t stride_width = 0;
  int32_t stride_height = 0;
  TfLiteFusedActivation activation = kTfLiteActNone;
  int32_t depth_multiplier = 0;
  int32_t dilation_width_factor = 0;
  int32_t dilation_height_factor = 0;
  // The scaling factor from input to output (aka the 'real multiplier') can
  // be represented as a fixed point multipler plus a left shift.
  int32_t output_multiplier;
  int output_shift;
  // The range of the fused activation layer. For example for kNone and
  // uint8_t these would be 0 and 255.
  int32_t output_activation_min;
  int32_t output_activation_max;

  // For per-channel implementation
  std::vector<int32_t> per_channel_output_multiplier;
  std::vector<int> per_channel_output_shift;
  std::vector<int32_t> per_channel_filter_offset;
};

}  // namespace transpose_conv

namespace axis_aligned_bbox_transform {

struct OpData {
  bool apply_scale;
};

}  // namespace axis_aligned_bbox_transform

namespace box_with_nms_limit {

struct OpData {
  float score_threshold = 0.0f;
  float nms_threshold = 1.0f;
  int32_t detection_per_image = 1;
  int32_t soft_nms_method = 0; // 0 imply hard nms
  float sigma = 1.0f;
};

}  // namespace box_with_nms_limit

namespace generate_proposals {

struct OpData {
  float spatial_scale = 1.0f;
  int32_t min_size = 0;
  float nms_threshold = 1.0f;
  int32_t pre_nms_top_n = 1;
  int32_t post_nms_top_n = 1;
  int transformed_box_index;
};

}  // namespace generate_proposals

namespace channel_shuffle {

struct OpData {
  int32_t num_groups = 0;
};

}  // namespace channel_shuffle

namespace layer_norm {

typedef struct {
  int scratch_index;
  int real_axis_index;
  int axis_index;
  int mean_index;
  int variance_index;
}  MtkLayerNormParams;

}  // namespace layer_norm

}  // namespace mtk
}  // namespace ops
}  // namespace tflite

#endif  // TENSORFLOW_CONTRIB_LITE_KERNELS_MTK_MTK_OPS_H_
