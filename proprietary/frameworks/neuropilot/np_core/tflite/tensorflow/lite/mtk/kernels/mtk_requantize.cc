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
#include "tensorflow/lite/mtk/kernels/mtk_ops.h"
#include "tensorflow/lite/mtk/mtk_utils.h"
#include "tensorflow/lite/mtk/mtk_helper.h"
#include "tensorflow/lite/kernels/internal/compatibility.h"
#include "tensorflow/lite/kernels/internal/quantization_util.h"
#include "tensorflow/lite/kernels/internal/tensor.h"
#include "tensorflow/lite/kernels/kernel_util.h"
#include "tensorflow/lite/kernels/op_macros.h"
#include "tensorflow/lite/kernels/padding.h"
#include "tensorflow/lite/nnapi/NeuralNetworksShim.h"

namespace tflite {
namespace ops {
namespace mtk {
namespace requantize {

enum KernelType {
  kReference,
};

constexpr int kInputTensor = 0;
constexpr int kOutputTensor = 0;


void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  OpData* data = new OpData;
  return data;
}

void Free(TfLiteContext* context, void* buffer) {
  delete reinterpret_cast<OpData*>(buffer);
}

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  TF_LITE_ENSURE_EQ(context, NumInputs(node), 1);
  TF_LITE_ENSURE_EQ(context, NumOutputs(node), 1);

  OpData* data = reinterpret_cast<OpData*>(node->user_data);
  const TfLiteTensor* input = GetInput(context, node, kInputTensor);
  TfLiteTensor* output = GetOutput(context, node, kOutputTensor);

  TF_LITE_ENSURE(context, input->type == kTfLiteUInt8 || input->type == kTfLiteInt16);
  TF_LITE_ENSURE(context, output->type == kTfLiteUInt8 || output->type == kTfLiteInt16);

  data->input_offset = -input->params.zero_point;
  data->output_offset = output->params.zero_point;

  if (input->type == kTfLiteUInt8) {
    data->left_shift = 20;
  }
  else if (input->type == kTfLiteInt16) {
    data->left_shift = 12;
  }

  const double input_scale = input->params.scale;
  const double output_scale = output->params.scale;
  const double real_output_multiplier = input_scale / ((1 << data->left_shift) * output_scale);
  QuantizeMultiplierSmallerThanOneExp(
      real_output_multiplier, &data->output_multiplier, &data->output_shift);

  // FIXME: output size does not have batch
  return context->ResizeTensor(context, output,
                               TfLiteIntArrayCopy(input->dims));
}

template <KernelType kernel_type>
TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {

  OpData* data = reinterpret_cast<OpData*>(node->user_data);
  TfLiteTensor* input = &context->tensors[node->inputs->data[0]];
  TfLiteTensor* output = &context->tensors[node->outputs->data[0]];

  tflite::MtkRequantizeParams op_params;
  op_params.left_shift = data->left_shift;
  op_params.input_offset = data->input_offset;
  op_params.output_offset = data->output_offset;
  op_params.output_multiplier = data->output_multiplier;
  op_params.output_shift = data->output_shift;

#define TFLITE_REQUANTIZE(input_type, output_type)                  \
  do {                                                              \
    reference_ops_mtk_nbits::Requantize<input_type, output_type>(   \
        op_params,                                                  \
        GetTensorShape(input), GetTensorData<uint8_t>(input),       \
        GetTensorShape(output), GetTensorData<uint8_t>(output));    \
  } while (0)

  const auto itype = input->type;
  const auto otype = output->type;

  if (itype == kTfLiteUInt8 && otype == kTfLiteUInt8)
    TFLITE_REQUANTIZE(uint8_t, uint8_t);
  else if (itype == kTfLiteUInt8 && otype == kTfLiteInt16)
    TFLITE_REQUANTIZE(uint8_t, int16_t);
  else if (itype == kTfLiteInt16 && otype == kTfLiteUInt8)
    TFLITE_REQUANTIZE(int16_t, uint8_t);
  else if (itype == kTfLiteInt16 && otype == kTfLiteInt16)
    TFLITE_REQUANTIZE(int16_t, int16_t);

  return kTfLiteOk;
}

int32_t add_ann_params(ANeuralNetworksModel* nn_model,
                       std::vector<uint32_t>& augmented_inputs,
                       uint32_t& next_id,
                       void* data) {
  return ::tflite::mtk::Hash("requantizemtk");
}

}  // namespace requantize


TfLiteRegistration* Register_MTK_REQUANTIZE_REF() {
  static TfLiteRegistration r = {requantize::Init,
                                 requantize::Free,
                                 requantize::Prepare,
                                 requantize::Eval<requantize::kReference>};
  ::tflite::mtk::CustomOpHelper::GetInstance().SetMtkExtOpParameterFunc(
      "MTK_REQUANTIZE", requantize::add_ann_params);
  return &r;
}

TfLiteRegistration* Register_MTK_REQUANTIZE() {
    return Register_MTK_REQUANTIZE_REF();
}

}  // namespace mtk
}  // namespace ops
}  // namespace tflite
