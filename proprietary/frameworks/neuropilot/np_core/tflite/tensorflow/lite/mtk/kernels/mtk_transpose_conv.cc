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
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <limits>

#include "tensorflow/lite/c/builtin_op_data.h"
#include "tensorflow/lite/c/c_api_internal.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/mtk/kernels/internal/reference/mtk_reference_ops.h"
#include "tensorflow/lite/kernels/internal/quantization_util.h"
#include "tensorflow/lite/kernels/internal/tensor.h"
#include "tensorflow/lite/kernels/kernel_util.h"
#include "tensorflow/lite/kernels/op_macros.h"
#include "tensorflow/lite/kernels/padding.h"

#include "tensorflow/lite/mtk/kernels/mtk_ops.h"

#include "flatbuffers/flexbuffers.h"

#include "tensorflow/lite/mtk/mtk_utils.h"
#include "tensorflow/lite/mtk/mtk_helper.h"

namespace tflite {
namespace ops {
namespace mtk {
namespace transpose_conv {

constexpr int kOutputShapeTensor = 0;
constexpr int kWeightsTensor = 1;
constexpr int kDataInputTensor = 2;
constexpr int kBiasTensor = 3;
constexpr int kOutputTensor = 0;

enum KernelType {
  kReference,
};


TfLiteStatus ResizeOutputShape(TfLiteContext* context,
                               const TfLiteTensor* output_shape,
                               TfLiteTensor* output) {
  // Currently only support int32 for output shape.
  if (output_shape->type != kTfLiteInt32) {
    context->ReportError(context, "Output shape is %d, not int32.",
                         output_shape->type);
    return kTfLiteError;
  }
  const int output_dimensions = NumElements(output_shape);
  TfLiteIntArray* output_shape_array = TfLiteIntArrayCreate(output_dimensions);
  for (int i = 0; i < output_dimensions; ++i) {
    output_shape_array->data[i] = GetTensorData<int32_t>(output_shape)[i];
  }

  return context->ResizeTensor(context, output, output_shape_array);
}

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  //convert paddding and activation enum, copy from lite/model.cc
  auto parse_padding = [](Padding padding) {
    switch (padding) {
      case Padding_SAME:
        return kTfLitePaddingSame;
      case Padding_VALID:
        return kTfLitePaddingValid;
    }
    return kTfLitePaddingUnknown;
  };
  auto parse_activation = [](ActivationFunctionType activation) {
    switch (activation) {
      case ActivationFunctionType_NONE:
        return kTfLiteActNone;
      case ActivationFunctionType_RELU:
        return kTfLiteActRelu;
      case ActivationFunctionType_RELU_N1_TO_1:
        return kTfLiteActRelu1;
      case ActivationFunctionType_RELU6:
        return kTfLiteActRelu6;
      case ActivationFunctionType_TANH:
        return kTfLiteActTanh;
      case ActivationFunctionType_SIGN_BIT:
        return kTfLiteActSignBit;
    }
    return kTfLiteActNone;
  };

  OpData* data = new OpData;

  const uint8_t* buffer_t = reinterpret_cast<const uint8_t*>(buffer);
  const flexbuffers::Map& m = flexbuffers::GetRoot(buffer_t, length).AsMap();
  data->padding = parse_padding(static_cast<Padding>(m["PaddingType"].AsInt64()));
  data->stride_width = m["stride_width"].AsInt64();
  data->stride_height = m["stride_height"].AsInt64();
  data->activation = parse_activation(static_cast<ActivationFunctionType>(m["activation"].AsInt64()));
  data->depth_multiplier =  m["depth_multiplier"].AsInt64();
  data->dilation_width_factor = m["dilation_width_factor"].AsInt64();
  data->dilation_height_factor = m["dilation_height_factor"].AsInt64();

  return data;
}

void Free(TfLiteContext* context, void* buffer) {
  delete reinterpret_cast<OpData*>(buffer);
}

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  OpData* data = reinterpret_cast<OpData*>(node->user_data);

  // Currently, transpose convolution don't support depthwise computation
  TF_LITE_ENSURE_EQ(context, data->depth_multiplier, 1);

  bool hasBias = NumInputs(node) == 4;
  TF_LITE_ENSURE(context, hasBias || NumInputs(node) == 3);
  TF_LITE_ENSURE_EQ(context, NumOutputs(node), 1);

  const TfLiteTensor* output_shape =
      GetInput(context, node, kOutputShapeTensor);
  const TfLiteTensor* filter = GetInput(context, node, kWeightsTensor);
  const TfLiteTensor* input = GetInput(context, node, kDataInputTensor);
  TfLiteTensor* output = GetOutput(context, node, kOutputTensor);

  TF_LITE_ENSURE_EQ(context, NumDimensions(output_shape), 1);
  TF_LITE_ENSURE_EQ(context, NumDimensions(input), 4);
  TF_LITE_ENSURE_EQ(context, NumDimensions(filter), 4);

  // // Check types. (We assume that UINT8 refers to quantized tensors)
  // const TfLiteType data_type = input->type;
  // TF_LITE_ENSURE(context,
  //                data_type == kTfLiteFloat32 || data_type == kTfLiteUInt8);
  // TF_LITE_ENSURE_EQ(context, output->type, data_type);
  // TF_LITE_ENSURE_EQ(context, filter->type, data_type);

  // Chia-Lin Yu @ Mediatek
  // Check if it uses nbits implementation
  const TfLiteType data_type = input->type;
  const TfLiteType filter_type = filter->type;
  const TfLiteType output_type = output->type;

  if (data_type == kTfLiteFloat32) {
    TF_LITE_ENSURE(context, filter_type == kTfLiteFloat32);
    TF_LITE_ENSURE(context, output_type == kTfLiteFloat32);
  }
  else {
    TF_LITE_ENSURE(context, data_type == kTfLiteUInt8 ||
                            data_type == kTfLiteInt8 ||
                            data_type == kTfLiteInt16);
    TF_LITE_ENSURE(context, filter_type == kTfLiteUInt8 ||
                            filter_type == kTfLiteInt8 ||
                            filter_type == kTfLiteInt16);
    TF_LITE_ENSURE(context, output_type == kTfLiteUInt8 ||
                            output_type == kTfLiteInt8 ||
                            output_type == kTfLiteInt16);
  }

  // Ensure that filter and inputs have the same channel dimension.
  // Note: TOCO will reorder filter in the following format: OHWI.
  TF_LITE_ENSURE_EQ(context, SizeOfDimension(input, 3),
                    SizeOfDimension(filter, 3));

  const TfLiteTensor* bias = (hasBias) ? GetInput(context, node, kBiasTensor) : nullptr;
  if (hasBias) {
    if (data_type == kTfLiteUInt8 || data_type == kTfLiteInt16 ||
        data_type == kTfLiteInt8) {
      TF_LITE_ENSURE_EQ(context, bias->type, kTfLiteInt32);
      TF_LITE_ENSURE_EQ(context, bias->params.zero_point, 0);
    } else {
      TF_LITE_ENSURE_EQ(context, bias->type, data_type);
    }
    TF_LITE_ENSURE_EQ(context, NumDimensions(bias), 1);
    TF_LITE_ENSURE_EQ(context, SizeOfDimension(bias, 0), SizeOfDimension(filter, 0));
  }

  // Note that quantized inference requires that all tensors have their
  // parameters set. This is usually done during quantized training.
  if (data_type != kTfLiteFloat32) {
    TF_LITE_ENSURE_EQ(context, filter->quantization.type,
                      kTfLiteAffineQuantization);
    const auto* affine_quantization =
        reinterpret_cast<TfLiteAffineQuantization*>(
            filter->quantization.params);
    TF_LITE_ENSURE(context, affine_quantization);
    TF_LITE_ENSURE(context, affine_quantization->scale);
    const int number_channel = affine_quantization->scale->size;
    data->per_channel_output_multiplier.resize(number_channel);
    data->per_channel_output_shift.resize(number_channel);
    data->per_channel_filter_offset.resize(number_channel);
    TF_LITE_ENSURE_STATUS(tflite::PopulateConvolutionQuantizationParams(
        context, input, filter, bias, output, data->activation,
        &data->output_multiplier, &data->output_shift,
        &data->output_activation_min, &data->output_activation_max,
        data->per_channel_output_multiplier.data(),
        data->per_channel_output_shift.data()));
  }

  if (!IsConstantTensor(output_shape)) {
    SetTensorToDynamic(output);
    return kTfLiteOk;
  }
  auto ret = ResizeOutputShape(context, output_shape, output);

  // Check the expected input shape from the output shape
  // with the VALID padding condition.
  auto output_width = SizeOfDimension(output, 2);
  auto output_height = SizeOfDimension(output, 1);
  auto filter_width = SizeOfDimension(filter, 2);
  auto filter_height = SizeOfDimension(filter, 1);

  // Matching GetWindowedOutputSize in TensorFlow.
  auto padding = data->padding;
  auto computeOutSize = [padding](int imageSize, int filterSize,
                                  int stride, int dilationRate) -> int {
    int effectiveFilterSize = (filterSize - 1) * dilationRate + 1;
    return padding == kTfLitePaddingSame
               ? (imageSize + stride - 1) / stride
               : padding == kTfLitePaddingValid
                     ? (imageSize - effectiveFilterSize + stride) / stride
                     : 0;
  };
  int expected_width = computeOutSize(output_width, filter_width, data->stride_width, data->dilation_width_factor);
  int expected_height = computeOutSize(output_height, filter_height, data->stride_height, data->dilation_height_factor);

  TF_LITE_ENSURE_EQ(context, SizeOfDimension(input, 2), expected_width);
  TF_LITE_ENSURE_EQ(context, SizeOfDimension(input, 1), expected_height);

  return ret;
}

void EvalQuantizedPerChannelNbits(TfLiteContext* context, TfLiteNode* node,
                 OpData* data, const TfLiteTensor* input, const TfLiteTensor* filter,
                 const TfLiteTensor* bias, TfLiteTensor* output) {
#if 0 // TODO: Fix the implementation later
  tflite::PerChannelConvParams op_params;
  op_params.padding_type = PaddingType::kSame;
  op_params.padding_values.width = data->paddingValues.width;
  op_params.padding_values.height = data->paddingValues.height;
  op_params.stride_width = data->stride_width;
  op_params.stride_height = data->stride_height;
  op_params.dilation_width_factor = data->dilation_width_factor;
  op_params.dilation_height_factor = data->dilation_height_factor;
  op_params.input_offset = -input->params.zero_point;
  op_params.weights_offsets = data->per_channel_filter_offset;
  op_params.output_offset = output->params.zero_point;
  op_params.output_multipliers = data->per_channel_output_multiplier;
  op_params.output_shifts = data->per_channel_output_shift;
  op_params.quantized_activation_min = data->output_activation_min;
  op_params.quantized_activation_max = data->output_activation_max;

  reference_ops_mtk::TransposeConv(
      op_params,
      GetTensorShape(input), GetTensorData<uint8_t>(input), input->type,
      GetTensorShape(filter), GetTensorData<uint8_t>(filter), filter->type,
      GetTensorShape(bias), GetTensorData<int32_t>(bias),
      GetTensorShape(output), GetTensorData<uint8_t>(output), output->type,
      GetTensorShape(output), GetTensorData<uint8_t>(output));
#endif
}

template <KernelType kernel_type>
void EvalQuantized(TfLiteContext* context, TfLiteNode* node,
                 OpData* data, const TfLiteTensor* input, const TfLiteTensor* filter,
                 const TfLiteTensor* bias, TfLiteTensor* output) {
  auto input_offset = -input->params.zero_point;
  auto filter_offset = -filter->params.zero_point;
  auto output_offset = output->params.zero_point;

  tflite::ConvParams op_params;
  op_params.padding_type = PaddingType::kSame;
  op_params.padding_values.width = data->paddingValues.width;
  op_params.padding_values.height = data->paddingValues.height;
  op_params.stride_width = data->stride_width;
  op_params.stride_height = data->stride_height;
  op_params.dilation_width_factor = data->dilation_width_factor;
  op_params.dilation_height_factor = data->dilation_height_factor;
  op_params.input_offset = input_offset;
  op_params.weights_offset = filter_offset;
  op_params.output_offset = output_offset;
  op_params.output_multiplier = data->output_multiplier;
  op_params.output_shift = data->output_shift;
  op_params.quantized_activation_min = data->output_activation_min;
  op_params.quantized_activation_max = data->output_activation_max;

  if (kernel_type == kReference) {
    reference_ops_mtk::TransposeConv(
      op_params, GetTensorShape(input), GetTensorData<uint8_t>(input),
      GetTensorShape(filter), GetTensorData<uint8_t>(filter),
      GetTensorShape(bias), GetTensorData<int32_t>(bias),
      GetTensorShape(output), GetTensorData<uint8_t>(output),
      // Last two args specify im2col which reference_ops ignores.
      GetTensorShape(output), GetTensorData<uint8_t>(output));
  } else {
    // TDDO: Optimized version.
    context->ReportError(context, "Optimized quantized implementation is not currently supported.");
  }
}

template <KernelType kernel_type>
void EvalFloat(TfLiteContext* context, TfLiteNode* node,
               OpData* data, const TfLiteTensor* input, const TfLiteTensor* filter,
               const TfLiteTensor* bias, TfLiteTensor* output) {
  float output_activation_min, output_activation_max;
  CalculateActivationRange(data->activation, &output_activation_min,
                           &output_activation_max);

  tflite::ConvParams op_params;
  op_params.padding_type = PaddingType::kSame;
  op_params.padding_values.width = data->paddingValues.width;
  op_params.padding_values.height = data->paddingValues.height;
  op_params.stride_width = data->stride_width;
  op_params.stride_height = data->stride_height;
  op_params.dilation_width_factor = data->dilation_width_factor;
  op_params.dilation_height_factor = data->dilation_height_factor;
  op_params.float_activation_min = output_activation_min;
  op_params.float_activation_max = output_activation_max;

  if (kernel_type == kReference) {
    reference_ops_mtk::TransposeConv(
      op_params, GetTensorShape(input), GetTensorData<float>(input),
      GetTensorShape(filter), GetTensorData<float>(filter),
      GetTensorShape(bias), GetTensorData<float>(bias),
      GetTensorShape(output), GetTensorData<float>(output),
      // Last two args specify im2col which reference_ops ignores.
      GetTensorShape(output), GetTensorData<float>(output));
  } else {
    // TDDO: Optimized version.
    context->ReportError(context, "Optimized quantized implementation is not currently supported.");
  }
}

template <KernelType kernel_type>
TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {
  const TfLiteTensor* output_shape =
      GetInput(context, node, kOutputShapeTensor);
  const TfLiteTensor* weights = GetInput(context, node, kWeightsTensor);
  const TfLiteTensor* input = GetInput(context, node, kDataInputTensor);
  TfLiteTensor* output = GetOutput(context, node, kOutputTensor);
  bool hasBias = NumInputs(node) == 4;
  const TfLiteTensor* bias =
    hasBias ? GetInput(context, node, kBiasTensor) : nullptr;

  OpData* data = reinterpret_cast<OpData*>(node->user_data);

  if (IsDynamicTensor(output)) {
    TF_LITE_ENSURE_OK(context,
                      ResizeOutputShape(context, output_shape, output));
  }

  // Get height and width of the output image.
  const int width = SizeOfDimension(output, 2);
  const int height = SizeOfDimension(output, 1);
  const int filter_width = SizeOfDimension(weights, 2);
  const int filter_height = SizeOfDimension(weights, 1);

  const int stride_width = data->stride_width;
  const int stride_height = data->stride_height;
  const int dilation_height_factor = data->dilation_height_factor;
  const int dilation_width_factor = data->dilation_width_factor;
  int unused_output_height, unused_output_width;
  data->paddingValues =
      ComputePaddingHeightWidth(stride_height, stride_width, dilation_height_factor,
                                dilation_width_factor, height, width,
                                filter_height, filter_width, data->padding,
                                &unused_output_height, &unused_output_width);

  switch (input->type) {  // Already know in/out types are same.
    case kTfLiteFloat32:
      EvalFloat<kernel_type>(context, node, data, input, weights, bias, output);
      break;
    case kTfLiteInt16:
    case kTfLiteInt8:
    case kTfLiteUInt8:
      EvalQuantizedPerChannelNbits(
          context, node, data, input, weights, bias, output);
      // EvalQuantized<kernel_type>(context, node, data, input, weights, bias, output);
      break;
    default:
      context->ReportError(context, "Type %d, not currently supported.",
                           input->type);
      return kTfLiteError;
  }
  return kTfLiteOk;
}

#define CHECK_NN(x)                                         \
  if (x != ANEURALNETWORKS_NO_ERROR) {                      \
    exit(1);                                                \
  }

int32_t add_ann_params(ANeuralNetworksModel* nn_model,
                       std::vector<uint32_t>& augmented_inputs,
                       uint32_t& next_id,
                       void* data) {
  auto add_scalar_int32 = [&nn_model, &augmented_inputs,
                           &next_id](int value) {
    ANeuralNetworksOperandType operand_type{.type = ANEURALNETWORKS_INT32};
    CHECK_NN(ANeuralNetworksModel_addOperand(nn_model, &operand_type))
    CHECK_NN(ANeuralNetworksModel_setOperandValue(nn_model, next_id, &value,
                                                  sizeof(int32_t)))
    augmented_inputs.push_back(next_id++);
  };

  auto builtin = reinterpret_cast<ops::mtk::transpose_conv::OpData*>(data);
  add_scalar_int32(builtin->padding);
  add_scalar_int32(builtin->stride_width);
  add_scalar_int32(builtin->stride_height);
  add_scalar_int32(builtin->activation);
  add_scalar_int32(builtin->depth_multiplier);
  add_scalar_int32(builtin->dilation_width_factor);
  add_scalar_int32(builtin->dilation_height_factor);
  return tflite::mtk::Hash("transposeconvmtk");
}

}  // namespace transpose_conv

TfLiteRegistration* Register_MTK_TRANSPOSE_CONV_REF() {
  static TfLiteRegistration r = {transpose_conv::Init,
                                 transpose_conv::Free,
                                 transpose_conv::Prepare,
                                 transpose_conv::Eval<transpose_conv::kReference>};
  ::tflite::mtk::CustomOpHelper::GetInstance().SetMtkExtOpParameterFunc(
      "MTK_TRANSPOSE_CONV", transpose_conv::add_ann_params);
  return &r;
}

TfLiteRegistration* Register_MTK_TRANSPOSE_CONV() {
  return Register_MTK_TRANSPOSE_CONV_REF();
}

}  // namespace mtk
}  // namespace ops
}  // namespace tflite
