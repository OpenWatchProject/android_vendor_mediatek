/* Copyright 2017 The TensorFlow Authors. All Rights Reserved.

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
#ifndef TENSORFLOW_LITE_KERNELS_INTERNAL_REFERENCE_MTK_REFERENCE_OPS_H_
#define TENSORFLOW_LITE_KERNELS_INTERNAL_REFERENCE_MTK_REFERENCE_OPS_H_

#include "tensorflow/lite/kernels/internal/reference/reference_ops.h"
#include "tensorflow/lite/mtk/kernels/internal/mtk_types.h"
#include "tensorflow/lite/kernels/internal/types.h"

#include <map>

namespace tflite {
namespace reference_ops_mtk {
inline void TransposeConv(
    const ConvParams& params, const RuntimeShape& input_shape,
    const float* input_data, const RuntimeShape& filter_shape,
    const float* filter_data, const RuntimeShape& bias_shape,
    const float* bias_data, const RuntimeShape& output_shape,
    float* output_data, const RuntimeShape& im2col_shape, float* im2col_data) {

  const int stride_width = params.stride_width;
  const int stride_height = params.stride_height;
  const int dilation_width_factor = params.dilation_width_factor;
  const int dilation_height_factor = params.dilation_height_factor;
  const int pad_width = params.padding_values.width;
  const int pad_height = params.padding_values.height;
  TFLITE_DCHECK_EQ(input_shape.DimensionsCount(), 4);
  TFLITE_DCHECK_EQ(filter_shape.DimensionsCount(), 4);
  TFLITE_DCHECK_EQ(output_shape.DimensionsCount(), 4);
  (void)im2col_data;   // only used in optimized code.
  (void)im2col_shape;  // only used in optimized code.

  const int batches = MatchingDim(input_shape, 0, output_shape, 0);
  const int input_depth = MatchingDim(input_shape, 3, filter_shape, 3);
  const int output_depth = MatchingDim(filter_shape, 0, output_shape, 3);
  const int input_height = input_shape.Dims(1);
  const int input_width = input_shape.Dims(2);
  const int filter_height = filter_shape.Dims(1);
  const int filter_width = filter_shape.Dims(2);
  const int output_height = output_shape.Dims(1);
  const int output_width = output_shape.Dims(2);
  const float output_activation_min = params.float_activation_min;
  const float output_activation_max = params.float_activation_max;
  if (bias_data) {
    TFLITE_DCHECK_EQ(bias_shape.FlatSize(), output_depth);
  }

  // Although transpose convolution simplifies to convolution with transposed
  // weights for strides of 1, non-unitary striding complicates matters. To
  // keep this reference implementation as clear as possible, we use a
  // "scatter" access pattern, where we loop through all the input elements,
  // computing their influence on the output, rather than looping through the
  // output elements in the typical "gather" access pattern of a conv. We
  // therefore must initialize the output array to zero.
  for (int batch = 0; batch < batches; batch++) {
    for (int y = 0; y < output_height; y++) {
      for (int x = 0; x < output_width; x++) {
        for (int c = 0; c < output_depth; c++) {
          const int output_index = Offset(output_shape, batch, y, x, c);
          if (bias_data) {
            output_data[output_index] = bias_data[c];
          } else {
            output_data[output_index] = 0.0f;
          }
        }
      }
    }
  }

  // Loop through input elements one at a time.
  for (int batch = 0; batch < batches; ++batch) {
    for (int in_y = 0; in_y < input_height; ++in_y) {
      for (int in_x = 0; in_x < input_width; ++in_x) {
        for (int in_channel = 0; in_channel < input_depth; ++in_channel) {
          // Loop through the output elements it will influence
          const int out_x_origin = (in_x * stride_width) - pad_width;
          const int out_y_origin = (in_y * stride_height) - pad_height;
          for (int filter_y = 0; filter_y < filter_height; ++filter_y) {
            for (int filter_x = 0; filter_x < filter_width; ++filter_x) {
              for (int out_channel = 0; out_channel < output_depth;
                   ++out_channel) {
                // Compute output element location
                const int out_x =
                    out_x_origin + dilation_width_factor * filter_x;
                const int out_y =
                    out_y_origin + dilation_height_factor * filter_y;
                // We cannot accumulate out of bounds
                if ((out_x >= 0) && (out_x < output_width) &&
                    (out_y >= 0) && (out_y < output_height)) {
                  float input_value = input_data[Offset(
                      input_shape, batch, in_y, in_x, in_channel)];
                  float filter_value =
                      filter_data[Offset(filter_shape, out_channel, filter_y,
                                         filter_x, in_channel)];
                  output_data[Offset(
                      output_shape, batch, out_y, out_x, out_channel)] +=
                      input_value * filter_value;
                }
              }
            }
          }
        }
      }
    }
  }
  // Apply activation.
  const int num_elements = output_shape.FlatSize();
  for (int o_i = 0; o_i < num_elements; o_i++) {
    auto value = output_data[o_i];
    value = std::max(value, output_activation_min);
    value = std::min(value, output_activation_max);
    output_data[o_i] = value;
  }
}

inline void TransposeConv(
    const ConvParams& params, const RuntimeShape& input_shape,
    const uint8* input_data, const RuntimeShape& filter_shape,
    const uint8* filter_data, const RuntimeShape& bias_shape,
    const int32* bias_data, const RuntimeShape& output_shape,
    uint8* output_data, const RuntimeShape& im2col_shape, uint8* im2col_data) {

  (void)im2col_data;   // only used in optimized code.
  (void)im2col_shape;  // only used in optimized code.
  const int stride_width = params.stride_width;
  const int stride_height = params.stride_height;
  const int dilation_width_factor = params.dilation_width_factor;
  const int dilation_height_factor = params.dilation_height_factor;
  const int pad_width = params.padding_values.width;
  const int pad_height = params.padding_values.height;
  TFLITE_DCHECK_EQ(input_shape.DimensionsCount(), 4);
  TFLITE_DCHECK_EQ(filter_shape.DimensionsCount(), 4);
  TFLITE_DCHECK_EQ(output_shape.DimensionsCount(), 4);
  const int32 input_offset = params.input_offset;
  const int32 filter_offset = params.weights_offset;
  const int32 output_offset = params.output_offset;
  const int32 output_multiplier = params.output_multiplier;
  const int output_shift = params.output_shift;
  const int32 output_activation_min = params.quantized_activation_min;
  const int32 output_activation_max = params.quantized_activation_max;

  TFLITE_DCHECK_LE(output_activation_min, output_activation_max);
  const int batches = MatchingDim(input_shape, 0, output_shape, 0);
  const int input_depth = MatchingDim(input_shape, 3, filter_shape, 3);
  const int output_depth = MatchingDim(filter_shape, 0, output_shape, 3);
  const int input_height = input_shape.Dims(1);
  const int input_width = input_shape.Dims(2);
  const int filter_height = filter_shape.Dims(1);
  const int filter_width = filter_shape.Dims(2);
  const int output_height = output_shape.Dims(1);
  const int output_width = output_shape.Dims(2);
  if (bias_data) {
    TFLITE_DCHECK_EQ(bias_shape.FlatSize(), output_depth);
  }

  int32* output_data_meta =
      new int32[batches*output_height*output_width*output_depth];
  // Although transpose convolution simplifies to convolution with transposed
  // weights for strides of 1, non-unitary striding complicates matters. To
  // keep this reference implementation as clear as possible, we use a
  // "scatter" access pattern, where we loop through all the input elements,
  // computing their influence on the output, rather than looping through the
  // output elements in the typical "gather" access pattern of a conv. We
  // therefore must initialize the output array to zero.
  for (int batch = 0; batch < batches; batch++) {
    for (int y = 0; y < output_height; y++) {
      for (int x = 0; x < output_width; x++) {
        for (int c = 0; c < output_depth; c++) {
          const int output_index = Offset(output_shape, batch, y, x, c);
          if (bias_data) {
            output_data_meta[output_index] = bias_data[c];
          } else {
            output_data_meta[output_index] = 0;
          }
        }
      }
    }
  }

  // Loop through input elements one at a time.
  for (int batch = 0; batch < batches; ++batch) {
    for (int in_y = 0; in_y < input_height; ++in_y) {
      for (int in_x = 0; in_x < input_width; ++in_x) {
        for (int in_channel = 0; in_channel < input_depth; ++in_channel) {
          // Loop through the output elements it will influence
          const int out_x_origin = (in_x * stride_width) - pad_width;
          const int out_y_origin = (in_y * stride_height) - pad_height;
          for (int filter_y = 0; filter_y < filter_height; ++filter_y) {
            for (int filter_x = 0; filter_x < filter_width; ++filter_x) {
              for (int out_channel = 0; out_channel < output_depth;
                  ++out_channel) {
                // Compute output element location
                const int out_x =
                    out_x_origin + dilation_width_factor * filter_x;
                const int out_y =
                    out_y_origin + dilation_height_factor * filter_y;
                // We cannot accumulate out of bounds
                if ((out_x >= 0) && (out_x < output_width) &&
                    (out_y >= 0) && (out_y < output_height)) {
                  int32 input_value = input_data[Offset(
                      input_shape, batch, in_y, in_x, in_channel)];
                  int32 filter_value =
                      filter_data[Offset(filter_shape, out_channel, filter_y,
                                         filter_x, in_channel)];
                  output_data_meta[Offset(
                      output_shape, batch, out_y, out_x, out_channel)] +=
                      (input_value + input_offset) *
                      (filter_value + filter_offset);
                }
              }
            }
          }
        }
      }
    }
  }
  // Apply activation and requant.
  const int num_elements = output_shape.FlatSize();
  for (int o_i = 0; o_i < num_elements; o_i++) {
    auto acc = output_data_meta[o_i];
    acc = MultiplyByQuantizedMultiplier(acc, output_multiplier, output_shift);
    acc += output_offset;
    acc = std::max(acc, output_activation_min);
    acc = std::min(acc, output_activation_max);
    output_data[o_i] = static_cast<uint8>(acc);
  }
  delete [] output_data_meta;
}

inline void CropAndResize(const MtkCropAndResizeParams& params,
                          const RuntimeShape& input_shape, const float* input_data,
                          const RuntimeShape& box_shape, const float* box_data,
                          const RuntimeShape& box_index_shape,
                          const int32* box_index_data,
                          const RuntimeShape& output_size_shape,
                          const int32* output_size_data,
                          const RuntimeShape& output_shape, float* output_data) {
  int32 input_height = input_shape.Dims(1);
  int32 input_width = input_shape.Dims(2);
  int32 depth = MatchingDim(input_shape, 3, output_shape, 3);

  TFLITE_DCHECK_EQ(output_size_shape.Dims(0), 2);

  int32 batches = MatchingDim(box_shape, 0, output_shape, 0);
  int32 output_height = output_shape.Dims(1);
  int32 output_width = output_shape.Dims(2);
  float ori_height_scale =
      static_cast<float>(input_height - 1) / (output_height - 1);
  float ori_width_scale =
      static_cast<float>(input_width - 1) / (output_width - 1);

  for (int b = 0; b < batches; b++) {
    float box_y1 = box_data[b * box_shape.Dims(1) + 0];
    float box_x1 = box_data[b * box_shape.Dims(1) + 1];
    float box_y2 = box_data[b * box_shape.Dims(1) + 2];
    float box_x2 = box_data[b * box_shape.Dims(1) + 3];
    float height_scale = static_cast<float>(box_y2 - box_y1) *
        ori_height_scale;
    float width_scale = static_cast<float>(box_x2 - box_x1) *
        ori_width_scale;
    float offset_y = box_y1 * (input_height - 1);
    float offset_x = box_x1 * (input_width - 1);
    int32 batch_index = box_index_data[b];
    for (int y = 0; y < output_height; y++) {
      float input_y = y * height_scale + offset_y;
      int y0 = std::floor(input_y);
      int y1 = std::min(input_height - 1, y0 + 1);
      for (int x = 0; x < output_width; x++) {
        float input_x = x * width_scale + offset_x;
        int x0 = std::floor(input_x);
        int x1 = std::min(input_width - 1, x0 + 1);
        for (int c = 0; c < depth; c++) {
          output_data[Offset(output_shape, b, y, x, c)] =
              static_cast<float>(
                  input_data[Offset(input_shape, batch_index, y0, x0, c)] *
                  (1 - (input_y - y0)) * (1 - (input_x - x0)) +
                  input_data[Offset(input_shape, batch_index, y1, x0, c)] *
                  (input_y - y0) * (1 - (input_x - x0)) +
                  input_data[Offset(input_shape, batch_index, y0, x1, c)] *
                  (1 - (input_y - y0)) * (input_x - x0) +
                  input_data[Offset(input_shape, batch_index, y1, x1, c)] *
                  (input_y - y0) * (input_x - x0));
        }
      }
    }
  }
}

inline void CropAndResize(const MtkCropAndResizeParams& params,
                          const RuntimeShape& input_shape, const uint8* input_data,
                          const RuntimeShape& box_shape, const uint8* box_data,
                          const RuntimeShape& box_index_shape,
                          const int32* box_index_data,
                          const RuntimeShape& output_size_shape,
                          const int32* output_size_data,
                          const RuntimeShape& output_shape, uint8* output_data) {
  int32 input_height = input_shape.Dims(1);
  int32 input_width = input_shape.Dims(2);
  int32 depth = MatchingDim(input_shape, 3, output_shape, 3);

  TFLITE_DCHECK_EQ(output_size_shape.Dims(0), 2);

  int32 batches = MatchingDim(box_shape, 0, output_shape, 0);
  int32 output_height = output_shape.Dims(1);
  int32 output_width = output_shape.Dims(2);
  float ori_height_scale =
      static_cast<float>(input_height - 1) / (output_height - 1);
  float ori_width_scale =
      static_cast<float>(input_width - 1) / (output_width - 1);

  float box_scale = params.box_scale;
  int32 box_offset = params.box_offset;
  for (int b = 0; b < batches; b++) {
    float box_y1 = static_cast<float>(
        box_data[b * box_shape.Dims(1) + 0] + box_offset) * box_scale;
    float box_x1 = static_cast<float>(
        box_data[b * box_shape.Dims(1) + 1] + box_offset) * box_scale;
    float box_y2 = static_cast<float>(
        box_data[b * box_shape.Dims(1) + 2] + box_offset) * box_scale;
    float box_x2 = static_cast<float>(
        box_data[b * box_shape.Dims(1) + 3] + box_offset) * box_scale;
    float height_scale = static_cast<float>(box_y2 - box_y1) *
        ori_height_scale;
    float width_scale = static_cast<float>(box_x2 - box_x1) *
        ori_width_scale;
    float offset_y = box_y1 * (input_height - 1);
    float offset_x = box_x1 * (input_width - 1);
    int32 batch_index = box_index_data[b];
    for (int y = 0; y < output_height; y++) {
      float input_y = y * height_scale + offset_y;
      int y0 = std::floor(input_y);
      int y1 = std::min(input_height - 1, y0 + 1);
      for (int x = 0; x < output_width; x++) {
        float input_x = x * width_scale + offset_x;
        int x0 = std::floor(input_x);
        int x1 = std::min(input_width - 1, x0 + 1);
        for (int c = 0; c < depth; c++) {
          output_data[Offset(output_shape, b, y, x, c)] =
              static_cast<uint8>(
                  input_data[Offset(input_shape, batch_index, y0, x0, c)] *
                  (1 - (input_y - y0)) * (1 - (input_x - x0)) +
                  input_data[Offset(input_shape, batch_index, y1, x0, c)] *
                  (input_y - y0) * (1 - (input_x - x0)) +
                  input_data[Offset(input_shape, batch_index, y0, x1, c)] *
                  (1 - (input_y - y0)) * (input_x - x0) +
                  input_data[Offset(input_shape, batch_index, y1, x1, c)] *
                  (input_y - y0) * (input_x - x0));
        }
      }
    }
  }
}

// TODO:
// box_shape is 2D
// box_index_shape is 1D
// output_size_shape is 1D
inline void RoiAlign(const MtkRoiAlignParams& params,
                     const RuntimeShape& input_shape, const float* input_data,
                     const RuntimeShape& box_shape, const float* box_data,
                     const RuntimeShape& box_index_shape,
                     const int32* box_index_data,
                     const RuntimeShape& output_size_shape,
                     const int32* output_size_data,
                     const RuntimeShape& resized_image_shape,
                     float* resized_image_data,
                     const RuntimeShape& output_shape, float* output_data) {
  int32 input_height = input_shape.Dims(1);
  int32 input_width = input_shape.Dims(2);
  int32 depth = MatchingDim(input_shape, 3, output_shape, 3);

  TFLITE_DCHECK_EQ(output_size_shape.Dims(0), 2);

  int32 batches = MatchingDim(box_shape, 0, output_shape, 0);
  int32 output_height = resized_image_shape.Dims(1);
  int32 output_width = resized_image_shape.Dims(2);
  float ori_height_scale =
      static_cast<float>(input_height - 1) / (output_height - 1);
  float ori_width_scale =
      static_cast<float>(input_width - 1) / (output_width - 1);

  int32 kernel_height = params.kernel_height;
  int32 kernel_width = params.kernel_width;
  for (int b = 0; b < batches; b++) {
    float box_y1 = box_data[b * box_shape.Dims(1) + 0];
    float box_x1 = box_data[b * box_shape.Dims(1) + 1];
    float box_y2 = box_data[b * box_shape.Dims(1) + 2];
    float box_x2 = box_data[b * box_shape.Dims(1) + 3];
    float height_scale = static_cast<float>(box_y2 - box_y1) *
        ori_height_scale;
    float width_scale = static_cast<float>(box_x2 - box_x1) *
        ori_width_scale;
    float offset_y = box_y1 * (input_height - 1);
    float offset_x = box_x1 * (input_width - 1);
    int32 batch_index = box_index_data[b];
    for (int y = 0; y < output_height; y++) {
      float input_y = y * height_scale + offset_y;
      int y0 = std::floor(input_y);
      int y1 = std::min(input_height - 1, y0 + 1);
      for (int x = 0; x < output_width; x++) {
        float input_x = x * width_scale + offset_x;
        int x0 = std::floor(input_x);
        int x1 = std::min(input_width - 1, x0 + 1);
        for (int c = 0; c < depth; c++) {
          resized_image_data[Offset(resized_image_shape, b, y, x, c)] =
              static_cast<float>(
                  input_data[Offset(input_shape, batch_index, y0, x0, c)] *
                  (1 - (input_y - y0)) * (1 - (input_x - x0)) +
                  input_data[Offset(input_shape, batch_index, y1, x0, c)] *
                  (input_y - y0) * (1 - (input_x - x0)) +
                  input_data[Offset(input_shape, batch_index, y0, x1, c)] *
                  (1 - (input_y - y0)) * (input_x - x0) +
                  input_data[Offset(input_shape, batch_index, y1, x1, c)] *
                  (input_y - y0) * (input_x - x0));
        }
      }
    }
  }
  // this is the same reference code as tflite version
  int stride_width = kernel_width;
  int stride_height = kernel_height;
  int pad_width = 0;
  int pad_height = 0;
  int filter_width = kernel_width;
  int filter_height = kernel_height;
  output_height = output_size_data[0];
  output_width = output_size_data[1];
  for (int batch = 0; batch < batches; ++batch) {
    for (int out_y = 0; out_y < output_height; ++out_y) {
      for (int out_x = 0; out_x < output_width; ++out_x) {
        for (int channel = 0; channel < depth; ++channel) {
          const int in_x_origin = (out_x * stride_width) - pad_width;
          const int in_y_origin = (out_y * stride_height) - pad_height;
          // Compute the boundaries of the filter region clamped so as to
          // ensure that the filter window fits in the input array.
          const int filter_x_start = std::max(0, -in_x_origin);
          const int filter_x_end =
              std::min(filter_width, input_width - in_x_origin);
          const int filter_y_start = std::max(0, -in_y_origin);
          const int filter_y_end =
              std::min(filter_height, input_height - in_y_origin);
          float max = std::numeric_limits<float>::lowest();
          for (int filter_y = filter_y_start; filter_y < filter_y_end;
               ++filter_y) {
            for (int filter_x = filter_x_start; filter_x < filter_x_end;
                 ++filter_x) {
              const int in_x = in_x_origin + filter_x;
              const int in_y = in_y_origin + filter_y;
              max = std::max(max, resized_image_data[Offset(
                    resized_image_shape, batch, in_y, in_x, channel)]);
            }
          }
          output_data[Offset(output_shape, batch, out_y, out_x, channel)] = max;
        }
      }
    }
  }
}

inline void RoiAlign(const MtkRoiAlignParams& params,
                     const RuntimeShape& input_shape, const uint8* input_data,
                     const RuntimeShape& box_shape, const uint8* box_data,
                     const RuntimeShape& box_index_shape,
                     const int32* box_index_data,
                     const RuntimeShape& output_size_shape,
                     const int32* output_size_data,
                     const RuntimeShape& resized_image_shape,
                     uint8* resized_image_data,
                     const RuntimeShape& output_shape, uint8* output_data) {
  int32 input_height = input_shape.Dims(1);
  int32 input_width = input_shape.Dims(2);
  int32 depth = MatchingDim(input_shape, 3, output_shape, 3);

  TFLITE_DCHECK_EQ(output_size_shape.Dims(0), 2);

  int32 batches = MatchingDim(box_shape, 0, output_shape, 0);
  int32 output_height = resized_image_shape.Dims(1);
  int32 output_width = resized_image_shape.Dims(2);
  float ori_height_scale =
      static_cast<float>(input_height - 1) / (output_height - 1);
  float ori_width_scale =
      static_cast<float>(input_width - 1) / (output_width - 1);

  int32 kernel_height = params.kernel_height;
  int32 kernel_width = params.kernel_width;
  float box_scale = params.box_scale;
  int32 box_offset = params.box_offset;
  for (int b = 0; b < batches; b++) {
    float box_y1 = static_cast<float>(
        box_data[b * box_shape.Dims(1) + 0] + box_offset) * box_scale;
    float box_x1 = static_cast<float>(
        box_data[b * box_shape.Dims(1) + 1] + box_offset) * box_scale;
    float box_y2 = static_cast<float>(
        box_data[b * box_shape.Dims(1) + 2] + box_offset) * box_scale;
    float box_x2 = static_cast<float>(
        box_data[b * box_shape.Dims(1) + 3] + box_offset) * box_scale;
    float height_scale = static_cast<float>(box_y2 - box_y1) *
        ori_height_scale;
    float width_scale = static_cast<float>(box_x2 - box_x1) *
        ori_width_scale;
    float offset_y = box_y1 * (input_height - 1);
    float offset_x = box_x1 * (input_width - 1);
    int32 batch_index = box_index_data[b];
    for (int y = 0; y < output_height; y++) {
      float input_y = y * height_scale + offset_y;
      int y0 = std::floor(input_y);
      int y1 = std::min(input_height - 1, y0 + 1);
      for (int x = 0; x < output_width; x++) {
        float input_x = x * width_scale + offset_x;
        int x0 = std::floor(input_x);
        int x1 = std::min(input_width - 1, x0 + 1);
        for (int c = 0; c < depth; c++) {
          resized_image_data[Offset(resized_image_shape, b, y, x, c)] =
              static_cast<uint8>(
                  input_data[Offset(input_shape, batch_index, y0, x0, c)] *
                  (1 - (input_y - y0)) * (1 - (input_x - x0)) +
                  input_data[Offset(input_shape, batch_index, y1, x0, c)] *
                  (input_y - y0) * (1 - (input_x - x0)) +
                  input_data[Offset(input_shape, batch_index, y0, x1, c)] *
                  (1 - (input_y - y0)) * (input_x - x0) +
                  input_data[Offset(input_shape, batch_index, y1, x1, c)] *
                  (input_y - y0) * (input_x - x0));
        }
      }
    }
  }
  // this is the same reference code as tflite version
  int stride_width = kernel_width;
  int stride_height = kernel_height;
  int pad_width = 0;
  int pad_height = 0;
  int filter_width = kernel_width;
  int filter_height = kernel_height;
  output_height = output_size_data[0];
  output_width = output_size_data[1];
  for (int batch = 0; batch < batches; ++batch) {
    for (int out_y = 0; out_y < output_height; ++out_y) {
      for (int out_x = 0; out_x < output_width; ++out_x) {
        for (int channel = 0; channel < depth; ++channel) {
          const int in_x_origin = (out_x * stride_width) - pad_width;
          const int in_y_origin = (out_y * stride_height) - pad_height;
          // Compute the boundaries of the filter region clamped so as to
          // ensure that the filter window fits in the input array.
          const int filter_x_start = std::max(0, -in_x_origin);
          const int filter_x_end =
              std::min(filter_width, input_width - in_x_origin);
          const int filter_y_start = std::max(0, -in_y_origin);
          const int filter_y_end =
              std::min(filter_height, input_height - in_y_origin);
          uint8 max = 0;
          for (int filter_y = filter_y_start; filter_y < filter_y_end;
               ++filter_y) {
            for (int filter_x = filter_x_start; filter_x < filter_x_end;
                 ++filter_x) {
              const int in_x = in_x_origin + filter_x;
              const int in_y = in_y_origin + filter_y;
              max = std::max(max, resized_image_data[Offset(
                    resized_image_shape, batch, in_y, in_x, channel)]);
            }
          }
          output_data[Offset(output_shape, batch, out_y, out_x, channel)] = max;
        }
      }
    }
  }
}

inline void SingleClassNms(const float* scores, const float* boxes, const float nmsThreshold,
                           const int detectionPerImage, int class_index, int class_num,
                           std::vector<int>& box_index_by_class,
                           std::vector<std::pair<int, int>>& selected) {
  auto cmp = [&scores, &class_num, &class_index](int idx1, int idx2) {
    float score1 = scores[idx1 * class_num + class_index];
    float score2 = scores[idx2 * class_num + class_index];
    return score1 > score2;
  };
  std::sort(box_index_by_class.begin(), box_index_by_class.end(), cmp);
  std::vector<bool> eliminated(box_index_by_class.size(), false);
  int per_class_count(0);
  for (int i = 0; i < box_index_by_class.size(); i++) {
    if (per_class_count == detectionPerImage)
      break;
    if (eliminated[i])
      continue;
    int index_i = box_index_by_class[i];
    int offset_i = index_i * class_num * 4 + class_index * 4;
    selected.push_back({index_i, class_index});
    per_class_count++;
    float y1_i = boxes[offset_i];
    float x1_i = boxes[offset_i + 1];
    float y2_i = boxes[offset_i + 2];
    float x2_i = boxes[offset_i + 3];
    float area_i = (y2_i - y1_i) * (x2_i - x1_i);
    for (int j = i + 1; j < box_index_by_class.size(); j++) {
      int index_j = box_index_by_class[j];
      int offset_j = index_j * class_num * 4 + class_index * 4;
      float y1_j = boxes[offset_j];
      float x1_j = boxes[offset_j + 1];
      float y2_j = boxes[offset_j + 2];
      float x2_j = boxes[offset_j + 3];
      float y1_intersec = std::max(y1_i, y1_j);
      float x1_intersec = std::max(x1_i, x1_j);
      float y2_intersec = std::min(y2_i, y2_j);
      float x2_intersec = std::min(x2_i, x2_j);
      if (y1_intersec > y2_intersec || x1_intersec > x2_intersec)
        continue;
      float area_j = (y2_j - y1_j) * (x2_j - x1_j);
      float area_intersec = (y2_intersec - y1_intersec) * (x2_intersec - x1_intersec);
      if (area_intersec / (area_i + area_j - area_intersec) > nmsThreshold)
        eliminated[j] = true;
    }
  }
}

inline void GetBoxAboveThreshold(const float* scores, std::vector<int>& box_index_by_class,
                                 float threshold, int class_index, int box_num, int class_num) {
  for (int i = 0; i < box_num; i++) {
    float cur_score = scores[i * class_num + class_index];
    if (cur_score > threshold)
      box_index_by_class.push_back(i);
  }
}

inline bool BoxWithNmsLimit(const float* scores, const RuntimeShape& scoresShape,
                            const float* boxes, const RuntimeShape& boxesShape,
                            const int* batchSplit, const RuntimeShape& batchSplitShape,
                            const float scoreThreshold, const float nmsThreshold,
                            const int detectionPerImage, const int nmsMethod, const float sigma,
                            float* outScores, const RuntimeShape& outScoresShape,
                            float* outBoxes, const RuntimeShape& outBoxesShape,
                            int* outClasses, const RuntimeShape& outClassesShape,
                            int* outBatchSplit, const RuntimeShape& outBatchSplitShape,
                            int* outBoxIndex, const RuntimeShape& outBoxIndexShape) {
  int box_num = scoresShape.Dims(0);
  int class_num = scoresShape.Dims(1);
  int max_detection = outScoresShape.Dims(0);
  int batch_num = batchSplitShape.Dims(0);
  memset(outScores, 0, outScoresShape.FlatSize() * sizeof(float));
  memset(outBoxes, 0, outBoxesShape.FlatSize() * sizeof(float));
  memset(outClasses, 0, outClassesShape.FlatSize() * sizeof(int));
  memset(outBatchSplit, 0, outBatchSplitShape.FlatSize() * sizeof(int));
  memset(outBoxIndex, 0, outBoxIndexShape.FlatSize() * sizeof(int));
  std::vector<std::pair<int, int>> selected;
  std::map<int, int> batch_map;
  for (int i = 0, total_count = 0; i < batch_num; i++) {
    int cur_num = batchSplit[i];
    for (int j = total_count; j < total_count + cur_num; j++) {
      batch_map[j] = i;
    }
    total_count += cur_num;
  }
  for (int i = 0; i < class_num; i++) {
    std::vector<int> box_index_by_class;
    GetBoxAboveThreshold(scores, box_index_by_class, scoreThreshold, i, box_num, class_num);
    SingleClassNms(scores, boxes, nmsThreshold, detectionPerImage, i, class_num,
                   box_index_by_class, selected);
  }
  auto cmp = [&scores, &class_num](std::pair<int, int> p1, std::pair<int, int> p2) {
    float score1 = scores[p1.first * class_num + p1.second];
    float score2 = scores[p2.first * class_num + p2.second];
    return score1 > score2;
  };
  std::sort(selected.begin(), selected.end(), cmp);
  int output_num = std::min(max_detection, int(selected.size()));
  for (int i = 0; i < output_num; i++) {
    int box_index = selected[i].first;
    int class_index = selected[i].second;
    outScores[i] = scores[box_index * class_num + class_index];
    int offset = box_index * class_num * 4 + class_index * 4;
    for (int j = 0; j < 4; j++)
      outBoxes[i * 4 + j] = boxes[offset + j];
    outClasses[i] = class_index;
    outBatchSplit[batch_map[box_index]]++;
    outBoxIndex[i] = batch_map[box_index];
  }
  return true;
}

inline void ClipBoxesAxisAligned(float xRoiCenter, float yRoiCenter,
                                 float roiWidth, float roiHeight,
                                 float imageWidth, float imageHeight, float* outputBase) {
  float x1 = xRoiCenter - roiWidth / 2;
  float y1 = yRoiCenter - roiHeight / 2;
  // float x2 = xRoiCenter + roiWidth / 2 - 1.0f;
  // float y2 = yRoiCenter + roiHeight / 2 - 1.0f;
  float x2 = xRoiCenter + roiWidth / 2;
  float y2 = yRoiCenter + roiHeight / 2;

  // x1 = std::min(std::max(x1, 0.0f), imageWidth - 1.0f);
  // y1 = std::min(std::max(y1, 0.0f), imageHeight - 1.0f);
  // x2 = std::min(std::max(x2, 0.0f), imageWidth - 1.0f);
  // y2 = std::min(std::max(y2, 0.0f), imageHeight - 1.0f);
  x1 = std::min(std::max(x1, 0.0f), imageWidth);
  y1 = std::min(std::max(y1, 0.0f), imageHeight);
  x2 = std::min(std::max(x2, 0.0f), imageWidth);
  y2 = std::min(std::max(y2, 0.0f), imageHeight);

  outputBase[1] = x1;
  outputBase[0] = y1;
  outputBase[3] = x2;
  outputBase[2] = y2;
}

inline void ClipBoxesRotated(float xRoiCenter, float yRoiCenter, float roiWidth, float roiHeight,
                             float roiAngle, bool angleBoundOn, int32_t angleBoundHigh,
                             int32_t angleBoundLow, float clipAngleThreshold, float imageWidth,
                             float imageHeight, float* outputBase) {
  if (angleBoundOn) {
    const int32_t angleBoundRange = angleBoundHigh - angleBoundLow;
    while (roiAngle < angleBoundLow) {
      roiAngle += angleBoundRange;
    }
    while (roiAngle > angleBoundHigh) {
      roiAngle -= angleBoundRange;
    }
  }

  if (std::abs(roiAngle) < clipAngleThreshold) {
    float x1 = xRoiCenter - (roiWidth - 1.0f) / 2;
    float x2 = xRoiCenter + (roiWidth - 1.0f) / 2;
    float y1 = yRoiCenter - (roiHeight - 1.0f) / 2;
    float y2 = yRoiCenter + (roiHeight - 1.0f) / 2;
    x1 = std::min(std::max(x1, 0.0f), imageWidth - 1.0f);
    x2 = std::min(std::max(x2, 0.0f), imageWidth - 1.0f);
    y1 = std::min(std::max(y1, 0.0f), imageHeight - 1.0f);
    y2 = std::min(std::max(y2, 0.0f), imageHeight - 1.0f);
    xRoiCenter = (x1 + x2) / 2;
    yRoiCenter = (y1 + y2) / 2;
    roiWidth = x2 - x1 + 1.0f;
    roiHeight = y2 - y1 + 1.0f;
  }

  outputBase[1] = xRoiCenter;
  outputBase[0] = yRoiCenter;
  outputBase[3] = roiWidth;
  outputBase[2] = roiHeight;
  outputBase[4] = roiAngle;
}

inline void BBoxTransform(const float* roiData, const RuntimeShape& roiShape,
                          const int32_t* batchIndexData, const float* bboxDeltasData,
                          const RuntimeShape& bboxDeltasShape, const float* imageSizeData,
                          const RuntimeShape& imageSizeDataShape, const float* imageScaleData,
                          const RuntimeShape& imageScaleDataShape, const float* weightsData,
                          const RuntimeShape& weightsDataShape, int32_t applyScale, bool rotated,
                          bool angleBoundOn, int32_t angleBoundLow, int32_t angleBoundHigh,
                          float clipAngleThreshold, float* outputData, const RuntimeShape& outputShape,
                          int32_t* batchSplitData, const RuntimeShape& batchSplitShape) {
  const uint32_t kRoiDim = rotated ? 5 : 4;
  const float PI = 3.14159265358979323846;
  const float kMaxTransform = std::log(1000.0 / 16.0);
  uint32_t numRois = roiShape.Dims(0);
  uint32_t roiInfoLength = roiShape.Dims(1);
  uint32_t numClasses = bboxDeltasShape.Dims(1) / kRoiDim;
  uint32_t imageSizeLength = imageSizeDataShape.Dims(1);
  uint32_t imageScaleLength = imageScaleDataShape.Dims(1);

  memset(batchSplitData, 0, batchSplitShape.FlatSize() * sizeof(float));

  const float* roiDataEnd =
          roiData + static_cast<uint64_t>(numRois) * static_cast<uint64_t>(roiInfoLength);
  const float* bboxDeltasBase = bboxDeltasData;
  float* outPtr = outputData;
  for (const float* roiBase = roiData; roiBase < roiDataEnd; roiBase += kRoiDim) {
    uint32_t batchIndex = batchIndexData[0];
    batchIndexData++;
    batchSplitData[batchIndex]++;

    const float* imageSizeBase = imageSizeData + batchIndex * imageSizeLength;
    const float* imageScaleBase = imageScaleData + batchIndex * imageScaleLength;
    float imageScale = imageScaleBase[0];
    float imageHeight = std::round(imageSizeBase[0] / imageScale);
    float imageWidth = std::round(imageSizeBase[1] / imageScale);

    float xRoiCenterBefore, yRoiCenterBefore;
    float roiWidthBefore, roiHeightBefore, roiAngleBefore;
    if (rotated) {
      xRoiCenterBefore = roiBase[1] / imageScale;
      yRoiCenterBefore = roiBase[0] / imageScale;
      roiWidthBefore = roiBase[3] / imageScale;
      roiHeightBefore = roiBase[2] / imageScale;
      roiAngleBefore = roiBase[4];
    } else {
      // roiWidthBefore = (roiBase[3] - roiBase[1]) / imageScale + 1.0f;
      // roiHeightBefore = (roiBase[2] - roiBase[0]) / imageScale + 1.0f;
      roiWidthBefore = (roiBase[3] - roiBase[1]) / imageScale;
      roiHeightBefore = (roiBase[2] - roiBase[0]) / imageScale;
      xRoiCenterBefore = roiBase[1] / imageScale + roiWidthBefore / 2;
      yRoiCenterBefore = roiBase[0] / imageScale + roiHeightBefore / 2;
      roiAngleBefore = 0;
    }

    for (uint32_t i = 0; i < numClasses; i++) {
      float roiWidthLogScale = std::min(bboxDeltasBase[3] / weightsData[3], kMaxTransform);
      float roiHeightLogScale = std::min(bboxDeltasBase[2] / weightsData[2], kMaxTransform);
      float xRoiCenterDelta = bboxDeltasBase[1] / weightsData[1] * roiWidthBefore;
      float yRoiCenterDelta = bboxDeltasBase[0] / weightsData[0] * roiHeightBefore;
      float angleDelta = rotated ? bboxDeltasBase[4] * 180.0f / PI : 0;

      float roiWidthAfter = std::exp(roiWidthLogScale) * roiWidthBefore;
      float roiHeightAfter = std::exp(roiHeightLogScale) * roiHeightBefore;
      float xRoiCenterAfter = xRoiCenterBefore + xRoiCenterDelta;
      float yRoiCenterAfter = yRoiCenterBefore + yRoiCenterDelta;
      float roiAngleAfter = roiAngleBefore + angleDelta;

      if (rotated) {
        ClipBoxesRotated(xRoiCenterAfter, yRoiCenterAfter, roiWidthAfter, roiHeightAfter,
                         roiAngleAfter, angleBoundOn, angleBoundHigh, angleBoundLow,
                         clipAngleThreshold, imageWidth, imageHeight, outPtr);
      } else {
        ClipBoxesAxisAligned(xRoiCenterAfter, yRoiCenterAfter, roiWidthAfter,
                             roiHeightAfter, imageWidth, imageHeight, outPtr);
      }

      if (applyScale) {
        outPtr[0] *= imageScale;
        outPtr[1] *= imageScale;
        outPtr[2] *= imageScale;
        outPtr[3] *= imageScale;
      }

      bboxDeltasBase += kRoiDim;
      outPtr += kRoiDim;
    }
  }
}

inline void AxisAlignedBBoxTransform(const float* roiData, const RuntimeShape& roiShape,
                                     const int32_t* batchIndexData,
                                     const float* bboxDeltasData, const RuntimeShape& bboxDeltasShape,
                                     const float* imageSizeData, const RuntimeShape& imageSizeDataShape,
                                     const float* imageScaleData, const RuntimeShape& imageScaleDataShape,
                                     const float* weightsData, const RuntimeShape& weightsDataShape,
                                     bool applyScale, float* outputData, const RuntimeShape& outputShape,
                                     int32_t* batchSplitData, const RuntimeShape& batchSplitShape) {
  BBoxTransform(roiData, roiShape, batchIndexData, bboxDeltasData, bboxDeltasShape,
                imageSizeData, imageSizeDataShape, imageScaleData, imageScaleDataShape,
                weightsData, weightsDataShape, applyScale, 0, 0, 0, 0,
                false,  // rotated = false
                outputData, outputShape, batchSplitData, batchSplitShape);
}

inline void BBoxTransform(const float* deltas, const RuntimeShape& deltasShape,
                          const float* anchors, const RuntimeShape& anchorsShape,
                          float* boxes, const RuntimeShape& boxesShape, const float* imageSize,
                          const float imageScale, const float spatialScale, const float* weights) {
  const RuntimeShape extented_anchors_shape =
    RuntimeShape::ExtendedShape(4, anchorsShape);
  for (int b = 0; b < deltasShape.Dims(0); b++) {
    for (int h = 0; h < deltasShape.Dims(1); h++) {
      for (int w = 0; w < deltasShape.Dims(2); w++) {
        for (int n = 0; n < anchorsShape.Dims(0); n++) {
          float y = h * (1 / spatialScale);
          float x = w * (1 / spatialScale);
          float in_y1 = y + anchors[Offset(extented_anchors_shape, 0, 0, n, 0)];
          float in_x1 = x + anchors[Offset(extented_anchors_shape, 0, 0, n, 1)];
          float in_y2 = y + anchors[Offset(extented_anchors_shape, 0, 0, n, 2)];
          float in_x2 = x + anchors[Offset(extented_anchors_shape, 0, 0, n, 3)];
          // clip boxes
          in_y1 = std::min(std::max(in_y1, 0.0f), imageSize[0]);
          in_x1 = std::min(std::max(in_x1, 0.0f), imageSize[1]);
          in_y2 = std::min(std::max(in_y2, 0.0f), imageSize[0]);
          in_x2 = std::min(std::max(in_x2, 0.0f), imageSize[1]);
          float in_y_center = (in_y1 + in_y2) / 2;
          float in_x_center = (in_x1 + in_x2) / 2;
          float in_height = in_y2 - in_y1;
          float in_width = in_x2 - in_x1;
          float out_y_center =
              in_y_center + in_height * deltas[Offset(deltasShape, b, h, w, n * 4 + 0)] / weights[0];
          float out_x_center =
              in_x_center + in_width * deltas[Offset(deltasShape, b, h, w, n * 4 + 1)] / weights[1];
          float out_height =
              in_height * std::exp(deltas[Offset(deltasShape, b, h, w, n * 4 + 2)] / weights[2]);
          float out_width =
              in_width * std::exp(deltas[Offset(deltasShape, b, h, w, n * 4 + 3)] / weights[3]);
          // out y1
          boxes[Offset(boxesShape, b, h, w, n * 4 + 0)] =
              std::min(std::max(out_y_center - out_height / 2, 0.0f), imageSize[0]);
          // out x1
          boxes[Offset(boxesShape, b, h, w, n * 4 + 1)] =
              std::min(std::max(out_x_center - out_width / 2, 0.0f), imageSize[1]);
          // out y2
          boxes[Offset(boxesShape, b, h, w, n * 4 + 2)] =
              std::min(std::max(out_y_center + out_height / 2, 0.0f), imageSize[0]);
          // out x2
          boxes[Offset(boxesShape, b, h, w, n * 4 + 3)] =
              std::min(std::max(out_x_center + out_width / 2, 0.0f), imageSize[1]);
        }
      }
    }
  }
}

inline bool GenerateProposals(const float* scores, const RuntimeShape& scoresShape,
                              const float* deltas, const RuntimeShape& deltasShape,
                              const float* anchors, const RuntimeShape& anchorsShape,
                              const float* imageSize, const RuntimeShape& imageSizeShape,
                              const float* imageScale, const RuntimeShape& imageScaleShape,
                              const float* weight, const RuntimeShape& weightShape,
                              float* transformedBox, const RuntimeShape& transformedBoxShape,
                              const int minSize, const int preNmsTopN, const int postNmsTopN,
                              const float spatialScale, const float nmsThreshold,
                              float* outBoxes, const RuntimeShape& outBoxesShape,
                              float* outBoxIndex, const RuntimeShape& outBoxIndexShape,
                              float* outScore, const RuntimeShape& outScoreShape) {
  BBoxTransform(deltas, deltasShape, anchors, anchorsShape, transformedBox,
                transformedBoxShape, imageSize, imageScale[0], spatialScale, weight);
  int box_num = scoresShape.FlatSize();
  std::vector<int> box_index_by_class;
  std::vector<std::pair<int, int32>> selected;
  // take all boxes without any constraint
  GetBoxAboveThreshold(scores, box_index_by_class, 0, 0, box_num, 1);
  // preNmsTopN == 0 means there is no limitation
  if (preNmsTopN > 0 && box_index_by_class.size() > preNmsTopN)
    box_index_by_class.resize(preNmsTopN);
  // set class number = 1, class index = 0 since there is only one class, background or foreground
  SingleClassNms(scores, transformedBox, nmsThreshold, postNmsTopN, 0, 1,
                 box_index_by_class, selected);
  int one_batch_size = scoresShape.Dims(1) * scoresShape.Dims(2) * scoresShape.Dims(3);
  int size = std::min(postNmsTopN, int(selected.size()));
  for (int i = 0; i < size; i++) {
    int box_index = selected[i].first;
    outBoxes[i * 4 + 0] = transformedBox[box_index * 4 + 0];
    outBoxes[i * 4 + 1] = transformedBox[box_index * 4 + 1];
    outBoxes[i * 4 + 2] = transformedBox[box_index * 4 + 2];
    outBoxes[i * 4 + 3] = transformedBox[box_index * 4 + 3];
    outScore[i] = scores[box_index];
    outBoxIndex[i] = box_index / one_batch_size;
  }
  return true;
}

template <typename In, typename Out>
inline bool ReduceWithMean(const In* input_data, const int* input_dims,
                           const int* output_dims, const int input_num_dims,
                           const int output_num_dims, const int* axis,
                           const int num_axis, int* input_iter,
                           Out reducer(Out current, Out mean, const In in, bool* overflow),
                           Out* output_data, Out* mean_data) {
  // Reset input iterator.
  TFLITE_DCHECK(input_num_dims > 0);
  for (int idx = 0; idx < input_num_dims; ++idx) {
    input_iter[idx] = 0;
  }
  // Iterate through input_data.
  do {
    size_t input_offset =
        ReducedOutputOffset(input_num_dims, input_dims, input_iter, 0, nullptr);
    size_t output_offset = ReducedOutputOffset(input_num_dims, input_dims,
                                               input_iter, num_axis, axis);
    bool overflow = false;
    output_data[output_offset] = reducer(output_data[output_offset], mean_data[output_offset],
                                         input_data[input_offset], &overflow);
    if (overflow) return false;
  } while (NextIndex(input_num_dims, input_dims, input_iter));
  return true;
}

template <typename In, typename Out>
inline bool ReduceVarianceImpl(const In* input_data, const int* input_dims,
                               const int* output_dims, const int input_num_dims,
                               const int output_num_dims, const int* axis,
                               const int num_axis, int* input_iter,
                               Out* output_data, Out* mean_data) {
  auto reducer = [](Out current, Out mean, const In in, bool* overflow) -> Out {
    const Out actual_in = static_cast<Out>(in);
    return current + (actual_in - mean) * (actual_in - mean);
  };
  return ReduceWithMean<In, Out>(input_data, input_dims, output_dims, input_num_dims,
                                 output_num_dims, axis, num_axis, input_iter, reducer,
                                 output_data, mean_data);
}

template <typename T, typename U>
inline bool Variance(const T* input_data, const int* input_dims,
                     const int input_num_dims, T* output_data,
                     const int* output_dims, const int output_num_dims,
                     const int* axis, const int num_axis_dimensions, bool keep_dims,
                     int* temp_index, int* resolved_axis, U* temp_var, U* mean_data) {
  // Reset output data.
  size_t num_outputs = 1;
  for (int idx = 0; idx < output_num_dims; ++idx) {
    size_t current = static_cast<size_t>(output_dims[idx]);
    // Overflow prevention.
    if (num_outputs > std::numeric_limits<size_t>::max() / current) {
      return false;
    }
    num_outputs *= current;
  }
  for (size_t idx = 0; idx < num_outputs; ++idx) {
    output_data[idx] = T();
    temp_var[idx] = U();
  }

  // Resolve axis.
  int num_resolved_axis = 0;
  if (!reference_ops::ResolveAxis(input_num_dims, axis, num_axis_dimensions,
                                  resolved_axis, &num_resolved_axis)) {
    return false;
  }

  if (!ReduceVarianceImpl<T, U>(input_data, input_dims, output_dims, input_num_dims,
                                output_num_dims, resolved_axis, num_resolved_axis,
                                temp_index, temp_var, mean_data)) {
    return false;
  }

  // Calculate mean by dividing output_data by num of aggregated element.
  U num_elements_in_axis = 1;
  for (int idx = 0; idx < num_resolved_axis; ++idx) {
    size_t current = static_cast<size_t>(input_dims[resolved_axis[idx]]);
    // Overflow prevention.
    if (current > (std::numeric_limits<U>::max() / num_elements_in_axis)) {
      return false;
    }
    num_elements_in_axis *= current;
  }

  if (num_elements_in_axis > 0) {
    for (size_t idx = 0; idx < num_outputs; ++idx) {
      output_data[idx] =
          static_cast<T>(temp_var[idx] / static_cast<U>(num_elements_in_axis));
    }
  }
  return true;
}

inline void LayerNormalization(const float* input_data, const int* input_dims,
                               const int input_num_dims, float* output_data,
                               const int* temp_dims, const int temp_num_dims,
                               const int* axis, const int num_axis_dimensions,
                               const float* gamma, const float* beta, int* temp_index,
                               int* resolved_axis, float* temp_mean, float* temp_var,
                               const RuntimeShape& tf_input_dims,
                               const RuntimeShape& tf_temp_dims,
                               const RuntimeShape& tf_params_dims,
                               const RuntimeShape& tf_output_dims) {
  bool keep_dims = true;
  reference_ops::Mean(input_data, input_dims, input_num_dims, temp_mean,
                      temp_dims, temp_num_dims, axis, num_axis_dimensions,
                      keep_dims, temp_index, resolved_axis, temp_mean);
  Variance(input_data, input_dims, input_num_dims, temp_var,
           temp_dims, temp_num_dims, axis, num_axis_dimensions, keep_dims,
           temp_index, resolved_axis, temp_var, temp_mean);
  int total_size(1);
  for (int i = 0; i < temp_num_dims; i++)
    total_size *= temp_dims[i];
  float epsilon = 1e-6;
  for (int i = 0; i < total_size; i++)
    temp_var[i] = std::sqrt(temp_var[i] + epsilon);
  ArithmeticParams params;
  params.float_activation_min = std::numeric_limits<float>::lowest();
  params.float_activation_max = std::numeric_limits<float>::max();
  reference_ops::BroadcastSub4DSlow(params, tf_input_dims, input_data,
                                    tf_temp_dims, temp_mean,
                                    tf_output_dims, output_data);
  reference_ops::BroadcastDiv4DSlow(params, tf_output_dims, output_data,
                                    tf_temp_dims, temp_var,
                                    tf_output_dims, output_data);
  reference_ops::BroadcastMul4DSlow(params, tf_output_dims, output_data,
                                    tf_params_dims, gamma,
                                    tf_output_dims, output_data);
  reference_ops::BroadcastAdd4DSlow(params, tf_output_dims, output_data,
                                    tf_params_dims, beta,
                                    tf_output_dims, output_data);
}

} // end namespace reference_ops_mtk

namespace reference_ops_mtk_nbits {

template <typename input_type, typename filter_type, typename output_type>
inline void TransposeConv(
    const PerChannelConvParams& params, const RuntimeShape& input_shape,
    const void* input_data, const RuntimeShape& filter_shape,
    const void* filter_data, const RuntimeShape& bias_shape,
    const int32* bias_data, const RuntimeShape& output_shape,
    void* output_data, const RuntimeShape& im2col_shape, uint8* im2col_data) {

  const auto* typed_input_data =
      reinterpret_cast<const input_type*>(input_data);
  const auto* typed_filter_data =
      reinterpret_cast<const filter_type*>(filter_data);
  auto* typed_output_data = reinterpret_cast<output_type*>(output_data);

  (void)im2col_data;   // only used in optimized code.
  (void)im2col_shape;  // only used in optimized code.
  const int stride_width = params.stride_width;
  const int stride_height = params.stride_height;
  const int dilation_width_factor = params.dilation_width_factor;
  const int dilation_height_factor = params.dilation_height_factor;
  const int pad_width = params.padding_values.width;
  const int pad_height = params.padding_values.height;
  TFLITE_DCHECK_EQ(input_shape.DimensionsCount(), 4);
  TFLITE_DCHECK_EQ(filter_shape.DimensionsCount(), 4);
  TFLITE_DCHECK_EQ(output_shape.DimensionsCount(), 4);
  const int32 input_offset = params.input_offset;
  const int32 output_offset = params.output_offset;
  const int32 output_activation_min = params.quantized_activation_min;
  const int32 output_activation_max = params.quantized_activation_max;

  // Check per_channel quantization param size
  const int per_channel_quantization_size = params.weights_offsets.size();
  TFLITE_DCHECK_EQ(
      per_channel_quantization_size, params.output_multipliers.size());
  TFLITE_DCHECK_EQ(per_channel_quantization_size, params.output_shifts.size());

  TFLITE_DCHECK_LE(output_activation_min, output_activation_max);
  const int batches = MatchingDim(input_shape, 0, output_shape, 0);
  const int input_depth = MatchingDim(input_shape, 3, filter_shape, 3);
  const int output_depth = MatchingDim(filter_shape, 0, output_shape, 3);
  const int input_height = input_shape.Dims(1);
  const int input_width = input_shape.Dims(2);
  const int filter_height = filter_shape.Dims(1);
  const int filter_width = filter_shape.Dims(2);
  const int output_height = output_shape.Dims(1);
  const int output_width = output_shape.Dims(2);
  if (bias_data) {
    TFLITE_DCHECK_EQ(bias_shape.FlatSize(), output_depth);
  }

  int32* output_data_meta =
      new int32[batches*output_height*output_width*output_depth];
  // Although transpose convolution simplifies to convolution with transposed
  // weights for strides of 1, non-unitary striding complicates matters. To
  // keep this reference implementation as clear as possible, we use a
  // "scatter" access pattern, where we loop through all the input elements,
  // computing their influence on the output, rather than looping through the
  // output elements in the typical "gather" access pattern of a conv. We
  // therefore must initialize the output array to zero.
  for (int batch = 0; batch < batches; batch++) {
    for (int y = 0; y < output_height; y++) {
      for (int x = 0; x < output_width; x++) {
        for (int c = 0; c < output_depth; c++) {
          const int output_index = Offset(output_shape, batch, y, x, c);
          if (bias_data) {
            output_data_meta[output_index] = bias_data[c];
          } else {
            output_data_meta[output_index] = 0;
          }
        }
      }
    }
  }

  // Loop through input elements one at a time.
  for (int batch = 0; batch < batches; ++batch) {
    for (int in_y = 0; in_y < input_height; ++in_y) {
      for (int in_x = 0; in_x < input_width; ++in_x) {
        for (int in_channel = 0; in_channel < input_depth; ++in_channel) {
          // Loop through the output elements it will influence
          const int out_x_origin = (in_x * stride_width) - pad_width;
          const int out_y_origin = (in_y * stride_height) - pad_height;
          for (int filter_y = 0; filter_y < filter_height; ++filter_y) {
            for (int filter_x = 0; filter_x < filter_width; ++filter_x) {
              for (int out_channel = 0; out_channel < output_depth;
                  ++out_channel) {

                const int per_channel_quantization_idx =
                  out_channel % per_channel_quantization_size;
                const int32 filter_offset =
                  params.weights_offsets[per_channel_quantization_idx];
                const int32 output_multiplier =
                  params.output_multipliers[per_channel_quantization_idx];
                const int output_shift =
                  params.output_shifts[per_channel_quantization_idx];

                // Compute output element location
                const int out_x =
                    out_x_origin + dilation_width_factor * filter_x;
                const int out_y =
                    out_y_origin + dilation_height_factor * filter_y;
                // We cannot accumulate out of bounds
                if ((out_x >= 0) && (out_x < output_width)
                    && (out_y >= 0) && (out_y < output_height)) {
                  int32 input_value = typed_input_data[Offset(
                      input_shape, batch, in_y, in_x, in_channel)];
                  int32 filter_value =
                      typed_filter_data[Offset(filter_shape, out_channel,
                                               filter_y, filter_x, in_channel)];
                  output_data_meta[Offset(
                      output_shape, batch, out_y, out_x, out_channel)] +=
                      (input_value + input_offset) *
                      (filter_value + filter_offset);
                }
              }
            }
          }
        }
      }
    }
  }
  // Apply activation and requant.
  for (int batch = 0; batch < batches; batch++) {
    for (int y = 0; y < output_height; y++) {
      for (int x = 0; x < output_width; x++) {
        for (int c = 0; c < output_depth; c++) {
          const int per_channel_quantization_idx =
              c % per_channel_quantization_size;
          const int32 output_multiplier =
            params.output_multipliers[per_channel_quantization_idx];
          const int output_shift =
            params.output_shifts[per_channel_quantization_idx];

          const int output_index = Offset(output_shape, batch, y, x, c);
          auto acc = output_data_meta[output_index];
          acc = MultiplyByQuantizedMultiplier(acc, output_multiplier,
                                              output_shift);
          acc += output_offset;
          acc = std::max(acc, output_activation_min);
          acc = std::min(acc, output_activation_max);
          typed_output_data[output_index] = static_cast<output_type>(acc);
        }
      }
    }
  }
  delete [] output_data_meta;
}

template <typename input_type, typename output_type>
inline void Requantize(const MtkRequantizeParams& params,
                       const RuntimeShape& input_shape, const void* input_data,
                       const RuntimeShape& output_shape, void* output_data) {

  const auto* typed_input_data =
      reinterpret_cast<const input_type*>(input_data);
  auto* typed_output_data =
      reinterpret_cast<output_type*>(output_data);

  const int flat_size = MatchingFlatSize(output_shape, input_shape);
  for (int i = 0; i < flat_size; i++) {
    const int32 val = typed_input_data[i] + params.input_offset;
    const int32 shifted_val = val * (1 << params.left_shift);
    int32 output_val =
        MultiplyByQuantizedMultiplierSmallerThanOneExp(
            shifted_val, params.output_multiplier, params.output_shift) +
            params.output_offset;

    output_val = std::max(output_val,
                          int32(std::numeric_limits<output_type>::min()));
    output_val = std::min(output_val,
                          int32(std::numeric_limits<output_type>::max()));

    typed_output_data[i] = static_cast<output_type>(output_val);
  }
}

template <typename input_type, typename output_type>
inline void Abs(const MtkQuantizedAbsParams& params,
                const RuntimeShape& input_shape, const void* input_data,
                const RuntimeShape& output_shape, void* output_data) {

  const auto* typed_input_data =
      reinterpret_cast<const input_type*>(input_data);
  auto* typed_output_data =
      reinterpret_cast<output_type*>(output_data);

  const int flat_size = MatchingFlatSize(output_shape, input_shape);
  for (int i = 0; i < flat_size; i++) {
    const int32 val = std::abs(typed_input_data[i] + params.input_offset);
    int32 output_val =
        MultiplyByQuantizedMultiplier(val, params.output_multiplier,
                                      params.output_shift);

    output_val += params.output_offset;

    output_val = std::max(output_val, params.quantized_activation_min);
    output_val = std::min(output_val, params.quantized_activation_max);
    typed_output_data[i] = static_cast<output_type>(output_val);
  }
}

template <typename output_type>
inline void Quantize(const MtkQuantizeParams& params,
                     const RuntimeShape& input_shape, const float* input_data,
                     const RuntimeShape& output_shape, void* output_data) {

  auto* typed_output_data =
      reinterpret_cast<output_type*>(output_data);

  const output_type qmin = std::numeric_limits<output_type>::min();
  const output_type qmax = std::numeric_limits<output_type>::max();
  const int flat_size = MatchingFlatSize(output_shape, input_shape);
  const int32 zero_point = params.zero_point;
  const float scale = params.scale;

  for (int i = 0; i < flat_size; i++) {
    float val = input_data[i];
    val = TfLiteRound(val/scale) + zero_point;
    val = std::max(std::min(val, float(qmax)), float(qmin));
    typed_output_data[i] = static_cast<output_type>(val);
  }
}

template <typename input_type, typename output_type>
inline void Elu(const MtkQuantizedEluParams& params,
                const RuntimeShape& input_shape, const void* input_data,
                const RuntimeShape& output_shape, void* output_data) {

  const auto* typed_input_data =
      reinterpret_cast<const input_type*>(input_data);
  auto* typed_output_data =
      reinterpret_cast<output_type*>(output_data);

  const int batches = MatchingDim(input_shape, 0, output_shape, 0);
  const int height = MatchingDim(input_shape, 1, output_shape, 1);
  const int width = MatchingDim(input_shape, 2, output_shape, 2);
  const int depth = MatchingDim(input_shape, 3, output_shape, 3);
  for (int b = 0; b < batches; ++b) {
    for (int y = 0; y < height; ++y) {
      for (int x = 0; x < width; ++x) {
        for (int c = 0; c < depth; ++c) {
          const int32 input_val =
              typed_input_data[Offset(input_shape, b, y, x, c)] +
              params.input_offset;
          int32 unclamped_output;
          if (input_val <= -params.input_range_radius) {
            unclamped_output = -params.quantized_one; // quantized negative 1
          } else if (input_val >= 0) {
            const int32 input_val_rescaled =
                MultiplyByQuantizedMultiplierGreaterThanOne(
                    input_val, params.input_multiplier, params.input_shift);
            unclamped_output =
                MultiplyByQuantizedMultiplierSmallerThanOneExp(
                    input_val_rescaled, params.output_multiplier,
                    params.output_shift);
          } else {
            const int32 input_val_rescaled =
                MultiplyByQuantizedMultiplierGreaterThanOne(
                    input_val, params.input_multiplier, params.input_shift);
            using FixedPoint4 = gemmlowp::FixedPoint<int32, 4>;
            using FixedPoint0 = gemmlowp::FixedPoint<int32, 0>;
            const FixedPoint4 input_val_f4 =
                FixedPoint4::FromRaw(input_val_rescaled);
            const FixedPoint0 output_val_f0 =
                gemmlowp::exp_on_negative_values(input_val_f4);
            int32 output_val_rescaled =
                MultiplyByQuantizedMultiplierSmallerThanOneExp(
                    output_val_f0.raw(), params.output_multiplier,
                    params.output_shift);
            using gemmlowp::RoundingDivideByPOT;
            int32 output_val_s32 = RoundingDivideByPOT(output_val_rescaled, 4);
            unclamped_output = output_val_s32 - params.quantized_one;
          }
          unclamped_output += params.output_offset;
          const int32 clamped_output =
              std::min(params.quantized_activation_max, std::max(
                    params.quantized_activation_min, unclamped_output));
          typed_output_data[Offset(output_shape, b, y, x, c)] =
              static_cast<output_type>(clamped_output);
        }
      }
    }
  }
}

template <typename T>
inline void Reverse(const RuntimeShape& input_shape, const T* input_data,
                    const RuntimeShape& axis_shape, const int* axis_data,
                    const RuntimeShape& output_shape, T* output_data) {

  const int batches = MatchingDim(input_shape, 0, output_shape, 0);
  const int height = MatchingDim(input_shape, 1, output_shape, 1);
  const int width = MatchingDim(input_shape, 2, output_shape, 2);
  const int depth = MatchingDim(input_shape, 3, output_shape, 3);

  auto input_dim = input_shape.DimensionsCount();
  auto axis_count = axis_shape.Dims(0);

  // Check if the axis is valid
  for (auto i = 0 ; i < axis_count ; i ++) {
    int current = axis_data[i] < 0 ? (axis_data[i] + input_dim) : axis_data[i];
    TFLITE_DCHECK(current >= 0 && current < input_dim);
  }

  // Resolve the axis to be reversed
  auto* reversed_axis = new bool[input_dim];
  for (auto i = 0 ; i < input_dim ; i ++) {
    reversed_axis[i] = false;
  }
  for (auto i = 0 ; i < axis_count ; i ++) {
    int current = axis_data[i] < 0 ? (axis_data[i] + input_dim) : axis_data[i];
    reversed_axis[current] = true;
  }

  for (int in_b = 0; in_b < batches; ++in_b) {
    const int out_b = (reversed_axis[0]) ? batches - 1 - in_b : in_b;
    for (int in_h = 0; in_h < height; ++in_h) {
      const int out_h = (reversed_axis[1]) ? height - 1 - in_h : in_h;
      for (int in_w = 0; in_w < width; ++in_w) {
        const int out_w = (reversed_axis[2]) ? width - 1 - in_w : in_w;
        for (int in_d = 0; in_d < depth; ++in_d) {
          const int out_d = (reversed_axis[3]) ? depth - 1 - in_d : in_d;

          const int input_index = Offset(input_shape, in_b, in_h, in_w, in_d);
          const int output_index =
              Offset(output_shape, out_b, out_h, out_w, out_d);
          output_data[output_index] = input_data[input_index];
        }
      }
    }
  }

  delete [] reversed_axis;
}

template <typename T>
inline void ChannelShuffle(const MtkChannelShuffleParams op_params,
                           const RuntimeShape& input_shape, const T* input_data,
                           const RuntimeShape& output_shape, T* output_data) {

  const int num_groups = op_params.num_groups;
  const int batches = MatchingDim(input_shape, 0, output_shape, 0);
  const int height = MatchingDim(input_shape, 1, output_shape, 1);
  const int width = MatchingDim(input_shape, 2, output_shape, 2);
  const int depth = MatchingDim(input_shape, 3, output_shape, 3);
  const int group_size = depth / num_groups;

  for (int in_b = 0; in_b < batches; ++in_b) {
    for (int in_h = 0; in_h < height; ++in_h) {
      for (int in_w = 0; in_w < width; ++in_w) {
        for (int in_d = 0; in_d < depth; ++in_d) {

          const int group_idx = int32(floor(in_d / group_size));
          const int offset = in_d % group_size;
          const int out_d = offset * num_groups + group_idx;

          const int in_index = Offset(input_shape, in_b, in_h, in_w, in_d);
          const int out_index = Offset(output_shape, in_b, in_h, in_w, out_d);
          output_data[out_index] = input_data[in_index];
        }
      }
    }
  }
}

template <typename elem_type>
inline void CropAndResize(const MtkCropAndResizeParams& params,
                          const RuntimeShape& input_shape, const void* input_data,
                          const RuntimeShape& box_shape, const void* box_data,
                          const RuntimeShape& box_index_shape,
                          const int32* box_index_data,
                          const RuntimeShape& output_size_shape,
                          const int32* output_size_data,
                          const RuntimeShape& output_shape, void* output_data) {

  const auto* typed_input_data =
      reinterpret_cast<const elem_type*>(input_data);
  const auto* typed_box_data =
      reinterpret_cast<const elem_type*>(box_data);
  auto* typed_output_data =
      reinterpret_cast<elem_type*>(output_data);

  int32 input_height = input_shape.Dims(1);
  int32 input_width = input_shape.Dims(2);
  int32 depth = MatchingDim(input_shape, 3, output_shape, 3);

  TFLITE_DCHECK_EQ(output_size_shape.Dims(0), 2);

  int32 batches = MatchingDim(box_shape, 0, output_shape, 0);
  int32 output_height = output_shape.Dims(1);
  int32 output_width = output_shape.Dims(2);
  float ori_height_scale =
      static_cast<float>(input_height - 1) / (output_height - 1);
  float ori_width_scale =
      static_cast<float>(input_width - 1) / (output_width - 1);

  float box_scale = params.box_scale;
  int32 box_offset = params.box_offset;
  for (int b = 0; b < batches; b++) {
    float box_y1 = static_cast<float>(
        typed_box_data[b * box_shape.Dims(1) + 0] + box_offset) * box_scale;
    float box_x1 = static_cast<float>(
        typed_box_data[b * box_shape.Dims(1) + 1] + box_offset) * box_scale;
    float box_y2 = static_cast<float>(
        typed_box_data[b * box_shape.Dims(1) + 2] + box_offset) * box_scale;
    float box_x2 = static_cast<float>(
        typed_box_data[b * box_shape.Dims(1) + 3] + box_offset) * box_scale;
    float height_scale = static_cast<float>(box_y2 - box_y1) *
        ori_height_scale;
    float width_scale = static_cast<float>(box_x2 - box_x1) *
        ori_width_scale;
    float offset_y = box_y1 * (input_height - 1);
    float offset_x = box_x1 * (input_width - 1);
    int32 batch_index = box_index_data[b];
    for (int y = 0; y < output_height; y++) {
      float input_y = y * height_scale + offset_y;
      int y0 = std::floor(input_y);
      int y1 = std::min(input_height - 1, y0 + 1);
      for (int x = 0; x < output_width; x++) {
        float input_x = x * width_scale + offset_x;
        int x0 = std::floor(input_x);
        int x1 = std::min(input_width - 1, x0 + 1);
        for (int c = 0; c < depth; c++) {
          typed_output_data[Offset(output_shape, b, y, x, c)] =
              static_cast<elem_type>(
                  typed_input_data[Offset(input_shape, batch_index, y0, x0, c)] *
                  (1 - (input_y - y0)) * (1 - (input_x - x0)) +
                  typed_input_data[Offset(input_shape, batch_index, y1, x0, c)] *
                  (input_y - y0) * (1 - (input_x - x0)) +
                  typed_input_data[Offset(input_shape, batch_index, y0, x1, c)] *
                  (1 - (input_y - y0)) * (input_x - x0) +
                  typed_input_data[Offset(input_shape, batch_index, y1, x1, c)] *
                  (input_y - y0) * (input_x - x0));
        }
      }
    }
  }
}

template <typename elem_type>
inline void RoiAlign(const MtkRoiAlignParams& params,
                     const RuntimeShape& input_shape, const void* input_data,
                     const RuntimeShape& box_shape, const void* box_data,
                     const RuntimeShape& box_index_shape,
                     const int32* box_index_data,
                     const RuntimeShape& output_size_shape,
                     const int32* output_size_data,
                     const RuntimeShape& resized_image_shape,
                     void* resized_image_data,
                     const RuntimeShape& output_shape, void* output_data) {

  const auto* typed_input_data =
      reinterpret_cast<const elem_type*>(input_data);
  const auto* typed_box_data =
      reinterpret_cast<const elem_type*>(box_data);
  auto* typed_resized_image_data =
      reinterpret_cast<elem_type*>(resized_image_data);
  auto* typed_output_data =
      reinterpret_cast<elem_type*>(output_data);

  int32 input_height = input_shape.Dims(1);
  int32 input_width = input_shape.Dims(2);
  int32 depth = MatchingDim(input_shape, 3, output_shape, 3);

  TFLITE_DCHECK_EQ(output_size_shape.Dims(0), 2);

  int32 batches = MatchingDim(box_shape, 0, output_shape, 0);
  int32 output_height = resized_image_shape.Dims(1);
  int32 output_width = resized_image_shape.Dims(2);
  float ori_height_scale =
      static_cast<float>(input_height - 1) / (output_height - 1);
  float ori_width_scale =
      static_cast<float>(input_width - 1) / (output_width - 1);

  int32 kernel_height = params.kernel_height;
  int32 kernel_width = params.kernel_width;
  float box_scale = params.box_scale;
  int32 box_offset = params.box_offset;
  for (int b = 0; b < batches; b++) {
    float box_y1 = static_cast<float>(
        typed_box_data[b * box_shape.Dims(1) + 0] + box_offset) * box_scale;
    float box_x1 = static_cast<float>(
        typed_box_data[b * box_shape.Dims(1) + 1] + box_offset) * box_scale;
    float box_y2 = static_cast<float>(
        typed_box_data[b * box_shape.Dims(1) + 2] + box_offset) * box_scale;
    float box_x2 = static_cast<float>(
        typed_box_data[b * box_shape.Dims(1) + 3] + box_offset) * box_scale;
    float height_scale = static_cast<float>(box_y2 - box_y1) *
        ori_height_scale;
    float width_scale = static_cast<float>(box_x2 - box_x1) *
        ori_width_scale;
    float offset_y = box_y1 * (input_height - 1);
    float offset_x = box_x1 * (input_width - 1);
    int32 batch_index = box_index_data[b];
    for (int y = 0; y < output_height; y++) {
      float input_y = y * height_scale + offset_y;
      int y0 = std::floor(input_y);
      int y1 = std::min(input_height - 1, y0 + 1);
      for (int x = 0; x < output_width; x++) {
        float input_x = x * width_scale + offset_x;
        int x0 = std::floor(input_x);
        int x1 = std::min(input_width - 1, x0 + 1);
        for (int c = 0; c < depth; c++) {
          typed_resized_image_data[Offset(resized_image_shape, b, y, x, c)] =
              static_cast<elem_type>(
                  typed_input_data[Offset(input_shape, batch_index, y0, x0, c)] *
                  (1 - (input_y - y0)) * (1 - (input_x - x0)) +
                  typed_input_data[Offset(input_shape, batch_index, y1, x0, c)] *
                  (input_y - y0) * (1 - (input_x - x0)) +
                  typed_input_data[Offset(input_shape, batch_index, y0, x1, c)] *
                  (1 - (input_y - y0)) * (input_x - x0) +
                  typed_input_data[Offset(input_shape, batch_index, y1, x1, c)] *
                  (input_y - y0) * (input_x - x0));
        }
      }
    }
  }
  // this is the same reference code as tflite version
  int stride_width = kernel_width;
  int stride_height = kernel_height;
  int pad_width = 0;
  int pad_height = 0;
  int filter_width = kernel_width;
  int filter_height = kernel_height;
  output_height = output_size_data[0];
  output_width = output_size_data[1];
  for (int batch = 0; batch < batches; ++batch) {
    for (int out_y = 0; out_y < output_height; ++out_y) {
      for (int out_x = 0; out_x < output_width; ++out_x) {
        for (int channel = 0; channel < depth; ++channel) {
          const int in_x_origin = (out_x * stride_width) - pad_width;
          const int in_y_origin = (out_y * stride_height) - pad_height;
          // Compute the boundaries of the filter region clamped so as to
          // ensure that the filter window fits in the input array.
          const int filter_x_start = std::max(0, -in_x_origin);
          const int filter_x_end =
              std::min(filter_width, input_width - in_x_origin);
          const int filter_y_start = std::max(0, -in_y_origin);
          const int filter_y_end =
              std::min(filter_height, input_height - in_y_origin);
          elem_type max = elem_type(std::numeric_limits<elem_type>::min());
          for (int filter_y = filter_y_start; filter_y < filter_y_end;
               ++filter_y) {
            for (int filter_x = filter_x_start; filter_x < filter_x_end;
                 ++filter_x) {
              const int in_x = in_x_origin + filter_x;
              const int in_y = in_y_origin + filter_y;
              max = std::max(max, typed_resized_image_data[Offset(
                    resized_image_shape, batch, in_y, in_x, channel)]);
            }
          }
          typed_output_data[Offset(output_shape, batch, out_y, out_x, channel)] = max;
        }
      }
    }
  }
}

} // end namespace reference_ops_mtk_nbits
} // end namespace tflite

#endif  // TENSORFLOW_LITE_KERNELS_INTERNAL_REFERENCE_MTK_REFERENCE_OPS_H_
