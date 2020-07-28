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

#include "tensorflow/lite/context.h"
#include "tensorflow/lite/kernels/internal/optimized/optimized_ops.h"
#include "tensorflow/lite/kernels/internal/quantization_util.h"
#include "tensorflow/lite/mtk/kernels/internal/reference/mtk_reference_ops.h"
#include "tensorflow/lite/kernels/internal/tensor.h"
#include "tensorflow/lite/kernels/kernel_util.h"
#include "tensorflow/lite/kernels/op_macros.h"
#include "flatbuffers/flexbuffers.h"
#include "tensorflow/lite/mtk/kernels/mtk_ops.h"
#include "tensorflow/lite/mtk/mtk_helper.h"
#include "tensorflow/lite/mtk/mtk_utils.h"

namespace tflite {
namespace ops {
namespace mtk {
namespace layer_norm {

enum KernelType {
  kReference,
  kGenericOptimized,  // Neon-free
  kNeonOptimized,
};

constexpr int kInputTensor1 = 0;
constexpr int kInputTensor2 = 1;
constexpr int kInputTensor3 = 2;
constexpr int kInputTensor4 = 3;
constexpr int kInputTensor5 = 4;
constexpr int kOutputTensor = 0;

// two additional tensors to store temporary result, which are mean and var
// two additional tensors to store axis information
void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  auto* data = new MtkLayerNormParams;
  context->AddTensors(context, 1, &data->scratch_index);
  context->AddTensors(context, 1, &data->real_axis_index);
  context->AddTensors(context, 1, &data->axis_index);
  context->AddTensors(context, 1, &data->mean_index);
  context->AddTensors(context, 1, &data->variance_index);
  return data;
}

void Free(TfLiteContext* context, void* buffer) {
  delete reinterpret_cast<MtkLayerNormParams*>(buffer);
}

// Resizes temp tensor that stores resolved axis.
TfLiteStatus ResizeTempAxis(TfLiteContext* context, TfLiteTensor* axis,
                            TfLiteTensor* resolved_axis) {
  TfLiteIntArray* axis_size = TfLiteIntArrayCreate(1);
  axis_size->data[0] = static_cast<int>(NumElements(axis));
  return context->ResizeTensor(context, resolved_axis, axis_size);
}

// Resizes tensors stored variance and mean based on the input size and resolved axis.
TfLiteStatus ResizeTempTensor(TfLiteContext* context, TfLiteNode* node,
                              TfLiteTensor* temp_tensor, int begin_norm) {
  const TfLiteTensor* input = GetInput(context, node, kInputTensor1);
  const TfLiteIntArray* input_dims = input->dims;
  int input_num_dims = NumDimensions(input);
  // keep_dim is always true
  TfLiteIntArray* temp_dims = TfLiteIntArrayCreate(input_num_dims);
  for (int idx = 0; idx < input_num_dims; ++idx) {
    if (idx < begin_norm) {
      temp_dims->data[idx] = input_dims->data[idx];
    } else {
      temp_dims->data[idx] = 1;
    }
  }
  return context->ResizeTensor(context, temp_tensor, temp_dims);
}

// Initializes temp tensors to store index and resolved axis.
TfLiteStatus InitializeTemporaries(TfLiteContext* context, TfLiteNode* node,
                                   int begin_norm) {
  auto* params = reinterpret_cast<MtkLayerNormParams*>(node->user_data);
  const TfLiteTensor* input1 = GetInput(context, node, kInputTensor1);
  TfLiteIntArrayFree(node->temporaries);
  node->temporaries = TfLiteIntArrayCreate(5);
  // Creates a temp index to iterate through input data.
  node->temporaries->data[0] = params->scratch_index;
  TfLiteTensor* scratch_tensor = GetTemporary(context, node, 0);
  scratch_tensor->type = kTfLiteInt32;
  scratch_tensor->allocation_type = kTfLiteArenaRw;
  TfLiteIntArray* index_size = TfLiteIntArrayCreate(1);
  index_size->data[0] = NumDimensions(input1);
  TF_LITE_ENSURE_OK(context, context->ResizeTensor(context, scratch_tensor, index_size));
  // Creates a temp tensor to store real axis data.
  node->temporaries->data[1] = params->real_axis_index;
  TfLiteTensor* real_axis = GetTemporary(context, node, 1);
  real_axis->type = kTfLiteInt32;
  real_axis->allocation_type = kTfLiteArenaRw;
  TfLiteIntArray* real_axis_size = TfLiteIntArrayCreate(1);
  real_axis_size->data[0] = NumDimensions(input1) - begin_norm;
  TF_LITE_ENSURE_OK(context, context->ResizeTensor(context, real_axis, real_axis_size));
  // Creates a temp tensor to store resolved axis given input data.
  node->temporaries->data[2] = params->axis_index;
  TfLiteTensor* resolved_axis = GetTemporary(context, node, 2);
  resolved_axis->type = kTfLiteInt32;
  // Creates temp tensors to store temp mean and variance.
  node->temporaries->data[3] = params->mean_index;
  TfLiteTensor* temp_mean = GetTemporary(context, node, 3);
  node->temporaries->data[4] = params->variance_index;
  TfLiteTensor* temp_var = GetTemporary(context, node, 4);

  switch (input1->type) {
    case kTfLiteFloat32:
      temp_mean->type = kTfLiteFloat32;
      temp_var->type = kTfLiteFloat32;
      break;
    case kTfLiteInt32:
      temp_mean->type = kTfLiteInt64;
      temp_var->type = kTfLiteInt64;
      break;
    case kTfLiteInt64:
      temp_mean->type = kTfLiteInt64;
      temp_var->type = kTfLiteInt64;
      break;
    case kTfLiteUInt8:
      temp_mean->type = kTfLiteInt32;
      temp_var->type = kTfLiteInt32;
      break;
    default:
      return kTfLiteError;
  }
  return kTfLiteOk;
}

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  TF_LITE_ENSURE_EQ(context, NumInputs(node), 5);
  TF_LITE_ENSURE_EQ(context, NumOutputs(node), 1);
  // input1: input tensor
  // input2: gamma
  // input3: beta
  // input4: begin_norm_axis
  // input5: begin_params_axis
  const TfLiteTensor* input1 = GetInput(context, node, kInputTensor1);
  const TfLiteTensor* input2 = GetInput(context, node, kInputTensor2);
  const TfLiteTensor* input3 = GetInput(context, node, kInputTensor3);
  const TfLiteTensor* input4 = GetInput(context, node, kInputTensor4);
  const TfLiteTensor* input5 = GetInput(context, node, kInputTensor5);
  TfLiteTensor* output = GetOutput(context, node, kOutputTensor);
  int begin_norm = input4->data.i32[0];
  if (begin_norm < 0)
    begin_norm = std::max(0, begin_norm + NumDimensions(input1));
  int begin_params = input5->data.i32[0];
  if (begin_params < 0)
    begin_params = std::max(0, begin_params + NumDimensions(input1));
  // Check begin norm axis and begin_params axis are less than input dimension
  TF_LITE_ENSURE(context, begin_norm < NumDimensions(input1));
  TF_LITE_ENSURE(context, begin_params < NumDimensions(input1));
  // the shape of gamma and beta must be the same,
  // and equals to input_shape[begin_params_axis:]
  TF_LITE_ENSURE_EQ(context, NumDimensions(input2), NumDimensions(input3));
  TF_LITE_ENSURE_EQ(context, NumDimensions(input2), NumDimensions(input1) - begin_params);
  for (int i = begin_params; i < NumDimensions(input1); i++)
    TF_LITE_ENSURE_EQ(context, input2->dims->data[i - begin_params], input1->dims->data[i]);
  for (int i = 0; i < NumDimensions(input2); i++)
    TF_LITE_ENSURE_EQ(context, input2->dims->data[i], input3->dims->data[i]);

  TF_LITE_ENSURE_OK(context, InitializeTemporaries(context, node, begin_norm));

  TfLiteTensor* real_axis = GetTemporary(context, node, 1);
  TfLiteTensor* resolved_axis = GetTemporary(context, node, 2);
  TfLiteTensor* temp_mean = GetTemporary(context, node, 3);
  TfLiteTensor* temp_var = GetTemporary(context, node, 4);

  resolved_axis->allocation_type = kTfLiteArenaRw;
  TF_LITE_ENSURE_OK(context, ResizeTempAxis(context, real_axis, resolved_axis));
  temp_mean->allocation_type = kTfLiteArenaRw;
  TF_LITE_ENSURE_OK(context, ResizeTempTensor(context, node, temp_mean, begin_norm));
  temp_var->allocation_type = kTfLiteArenaRw;
  TF_LITE_ENSURE_OK(context, ResizeTempTensor(context, node, temp_var, begin_norm));
  return context->ResizeTensor(context, output, TfLiteIntArrayCopy(input1->dims));
}

template <KernelType kernel_type>
void EvalLayerNormalizationFloat(TfLiteContext* context, TfLiteNode* node,
                                 const TfLiteTensor* input1, const TfLiteTensor* input2,
                                 const TfLiteTensor* input3, const TfLiteTensor* input4,
                                 const TfLiteTensor* input5, TfLiteTensor* temp_index,
                                 TfLiteTensor* real_axis, TfLiteTensor* resolved_axis,
                                 TfLiteTensor* temp_mean, TfLiteTensor* temp_var,
                                 TfLiteTensor* output) {
  // transform begin_norm_aixs into real axis data
  // ex: input shape is [b, h, w, c], begin_norm axis is 2
  // real axis is [2, 3]
  int begin_norm = NumDimensions(input1) - NumElements(real_axis);
  for (int i = 0; i < NumElements(real_axis); i++)
    real_axis->data.i32[i] = i + begin_norm;
  // define broadcast dimension of beta and gamma
  int begin_params = input5->data.i32[0];
  if (begin_params < 0)
    begin_params = std::max(0, begin_params + NumDimensions(input1));
  std::vector<int32_t> params_dim(NumDimensions(input1));
  for (int i = 0; i < NumDimensions(input1); i++) {
    if (i < begin_params)
      params_dim[i] = 1;
    else
      params_dim[i] = input1->dims->data[i];
  }
#define MTK_LAYER_NORMALIZATION(type, opname)                                         \
  type::opname(GetTensorData<float>(input1),                                          \
               input1->dims->data, NumDimensions(input1),                             \
               GetTensorData<float>(output),                                          \
               temp_mean->dims->data, NumDimensions(temp_mean),                       \
               GetTensorData<int>(real_axis), NumElements(real_axis),                 \
               GetTensorData<float>(input2), GetTensorData<float>(input3),            \
               GetTensorData<int>(temp_index), GetTensorData<int>(resolved_axis),     \
               GetTensorData<float>(temp_mean), GetTensorData<float>(temp_var),       \
               GetTensorShape(input1), GetTensorShape(temp_mean),                     \
               GetTensorShape(params_dim), GetTensorShape(output))
  if (kernel_type == kReference) {
    MTK_LAYER_NORMALIZATION(reference_ops_mtk, LayerNormalization);
  } else {
    // TBD
  }
#undef MTK_LAYER_NORMALIZATION
}

template <KernelType kernel_type>
TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {
  // input
  const TfLiteTensor* input1 = GetInput(context, node, kInputTensor1);
  // gamma
  const TfLiteTensor* input2 = GetInput(context, node, kInputTensor2);
  // beta
  const TfLiteTensor* input3 = GetInput(context, node, kInputTensor3);
  // begin_norm axis
  const TfLiteTensor* input4 = GetInput(context, node, kInputTensor4);
  // begin params axis
  const TfLiteTensor* input5 = GetInput(context, node, kInputTensor5);
  // output
  TfLiteTensor* output = GetOutput(context, node, kOutputTensor);

  TfLiteTensor* temp_index = GetTemporary(context, node, 0);
  TfLiteTensor* real_axis = GetTemporary(context, node, 1);
  TfLiteTensor* resolved_axis = GetTemporary(context, node, 2);
  TfLiteTensor* temp_mean = GetTemporary(context, node, 3);
  TfLiteTensor* temp_var = GetTemporary(context, node, 4);

  if (output->type == kTfLiteFloat32) {
    EvalLayerNormalizationFloat<kernel_type>(context, node, input1, input2,
                                             input3, input4, input5, temp_index,
                                             real_axis, resolved_axis, temp_mean,
                                             temp_var, output);
  } else if (output->type == kTfLiteUInt8) {
    context->ReportError(context,
                         "Uint8 type is currently not supported");
  } else {
    context->ReportError(context,
                         "Inputs and outputs not all float|uint8 types.");
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

  auto builtin =
      reinterpret_cast<ops::mtk::layer_norm::MtkLayerNormParams*>(data);
  add_scalar_int32(builtin->scratch_index);
  add_scalar_int32(builtin->real_axis_index);
  add_scalar_int32(builtin->axis_index);
  add_scalar_int32(builtin->mean_index);
  add_scalar_int32(builtin->variance_index);
  return ::tflite::mtk::Hash("layernormalizationmtk");
}

}  // namespace layer_norm

TfLiteRegistration* Register_MTK_LAYER_NORMALIZATION_REF() {
  static TfLiteRegistration r = {layer_norm::Init, layer_norm::Free, layer_norm::Prepare,
                                 layer_norm::Eval<layer_norm::kReference>};
  ::tflite::mtk::CustomOpHelper::GetInstance().SetMtkExtOpParameterFunc(
      "MTK_LAYER_NORMALIZATION", layer_norm::add_ann_params);
  return &r;
}

TfLiteRegistration* Register_MTK_LAYER_NORMALIZATION_GENERIC_OPT() {
  static TfLiteRegistration r = {layer_norm::Init, layer_norm::Free, layer_norm::Prepare,
                                 layer_norm::Eval<layer_norm::kReference>};
  ::tflite::mtk::CustomOpHelper::GetInstance().SetMtkExtOpParameterFunc(
      "MTK_LAYER_NORMALIZATION", layer_norm::add_ann_params);
  return &r;
}

TfLiteRegistration* Register_MTK_LAYER_NORMALIZATION_NEON_OPT() {
  static TfLiteRegistration r = {layer_norm::Init, layer_norm::Free, layer_norm::Prepare,
                                 layer_norm::Eval<layer_norm::kNeonOptimized>};
  ::tflite::mtk::CustomOpHelper::GetInstance().SetMtkExtOpParameterFunc(
      "MTK_LAYER_NORMALIZATION", layer_norm::add_ann_params);
  return &r;
}

TfLiteRegistration* Register_MTK_LAYER_NORMALIZATION() {
#ifdef USE_NEON
  return Register_MTK_LAYER_NORMALIZATION_GENERIC_OPT();
#else
  return Register_MTK_LAYER_NORMALIZATION_GENERIC_OPT();
#endif
}

}  // namespace mtk
}  // namespace ops
}  // namespace tflite
