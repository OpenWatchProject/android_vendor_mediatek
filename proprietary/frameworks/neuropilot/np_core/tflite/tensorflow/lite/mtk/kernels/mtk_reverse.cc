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
#include <unistd.h>
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
#include "tensorflow/lite/nnapi/NeuralNetworksShim.h"
#include "tensorflow/lite/mtk/mtk_utils.h"
#include "tensorflow/lite/mtk/mtk_helper.h"
#include "flatbuffers/flexbuffers.h"

namespace tflite {
namespace ops {
namespace mtk {
namespace reverse {

enum KernelType {
  kReference,
};

constexpr int kInputTensor = 0;
constexpr int kAxisTensor = 1;
constexpr int kOutputTensor = 0;


TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  TF_LITE_ENSURE_EQ(context, NumInputs(node), 2);
  TF_LITE_ENSURE_EQ(context, NumOutputs(node), 1);

  const TfLiteTensor* input = GetInput(context, node, kInputTensor);
  const TfLiteTensor* axis = GetInput(context, node, kAxisTensor);
  TfLiteTensor* output = GetOutput(context, node, kOutputTensor);

  // Currently only support 4D input
  TF_LITE_ENSURE_EQ(context, NumDimensions(input), 4);
  TF_LITE_ENSURE_EQ(context, NumDimensions(axis), 1);
  TF_LITE_ENSURE_EQ(context, axis->type, kTfLiteInt32);

  TF_LITE_ENSURE_EQ(context, input->type, output->type);
  if (input->type == kTfLiteUInt8 || input->type == kTfLiteInt16) {
    TF_LITE_ENSURE_EQ(context, input->params.scale, output->params.scale);
    TF_LITE_ENSURE_EQ(context, input->params.zero_point, output->params.zero_point);
  }

  // FIXME: output size does not have batch
  return context->ResizeTensor(context, output,
                               TfLiteIntArrayCopy(input->dims));
}

template <KernelType kernel_type>
TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {

  const TfLiteTensor* input = GetInput(context, node, kInputTensor);
  const TfLiteTensor* axis = GetInput(context, node, kAxisTensor);
  TfLiteTensor* output = GetOutput(context, node, kOutputTensor);

#define TFLITE_REVERSE(elem_type)                                   \
  do {                                                              \
    reference_ops_mtk_nbits::Reverse(                               \
        GetTensorShape(input), GetTensorData<elem_type>(input),     \
        GetTensorShape(axis), GetTensorData<int32_t>(axis),         \
        GetTensorShape(output), GetTensorData<elem_type>(output));  \
  } while (0)

  switch (input->type) {
    case kTfLiteFloat32:
      TFLITE_REVERSE(float);
      break;
    case kTfLiteUInt8:
      TFLITE_REVERSE(uint8_t);
      break;
    case kTfLiteInt16:
      TFLITE_REVERSE(int16_t);
      break;
    default:
      context->ReportError(
          context, "Type %d is currently not supported by MtkReverse.", input->type);
      return kTfLiteError;
  }

  return kTfLiteOk;
}

int32_t add_ann_params(ANeuralNetworksModel* nn_model,
                       std::vector<uint32_t>& augmented_inputs,
                       uint32_t& next_id,
                       void* data) {
  return ::tflite::mtk::Hash("reversemtk");
}

}  // namespace reverse


TfLiteRegistration* Register_MTK_REVERSE_REF() {
  static TfLiteRegistration r = {nullptr, nullptr,
                                 reverse::Prepare,
                                 reverse::Eval<reverse::kReference>};
  ::tflite::mtk::CustomOpHelper::GetInstance().SetMtkExtOpParameterFunc(
      "MTK_REVERSE", reverse::add_ann_params);
  return &r;
}

TfLiteRegistration* Register_MTK_REVERSE() {
    return Register_MTK_REVERSE_REF();
}

}  // namespace mtk
}  // namespace ops
}  // namespace tflite
