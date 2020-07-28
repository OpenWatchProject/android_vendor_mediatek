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

#include "tensorflow/lite/context.h"
#include "tensorflow/lite/mtk/kernels/internal/reference/mtk_reference_ops.h"
#include "tensorflow/lite/kernels/internal/tensor.h"
#include "tensorflow/lite/kernels/kernel_util.h"
#include "tensorflow/lite/kernels/op_macros.h"
#include "tensorflow/lite/nnapi/NeuralNetworksShim.h"
#include "tensorflow/lite/mtk/mtk_utils.h"
#include "tensorflow/lite/mtk/mtk_helper.h"

namespace tflite {
namespace ops {
namespace mtk {
namespace quantize {

TfLiteStatus QuantizePrepare(TfLiteContext* context, TfLiteNode* node) {
  TF_LITE_ENSURE_EQ(context, NumInputs(node), 1);
  TF_LITE_ENSURE_EQ(context, NumOutputs(node), 1);
  const TfLiteTensor* input = GetInput(context, node, 0);
  TfLiteTensor* output = GetOutput(context, node, 0);
  TF_LITE_ENSURE(context, input->type == kTfLiteFloat32);
  TF_LITE_ENSURE(context, output->type == kTfLiteUInt8 || output->type == kTfLiteInt16);

  return context->ResizeTensor(context, output,
                               TfLiteIntArrayCopy(input->dims));
}

TfLiteStatus QuantizeEval(TfLiteContext* context, TfLiteNode* node) {

  TfLiteTensor* output = &context->tensors[node->outputs->data[0]];
  TfLiteTensor* input = &context->tensors[node->inputs->data[0]];

  tflite::MtkQuantizeParams op_params;
  op_params.zero_point = output->params.zero_point;
  op_params.scale = output->params.scale;

#define TFLITE_QUANTIZE(output_type)                                \
  do {                                                              \
    reference_ops_mtk_nbits::Quantize<output_type>(                 \
        op_params,                                                  \
        GetTensorShape(input), GetTensorData<float>(input),         \
        GetTensorShape(output), GetTensorData<uint8_t>(output));    \
  } while(0)

  const auto otype = output->type;

  if (otype == kTfLiteUInt8)
    TFLITE_QUANTIZE(uint8_t);
  else if (otype == kTfLiteInt16)
    TFLITE_QUANTIZE(int16_t);

  return kTfLiteOk;
}

int32_t add_ann_params(ANeuralNetworksModel* nn_model,
                       std::vector<uint32_t>& augmented_inputs,
                       uint32_t& next_id,
                       void* data) {
  return ::tflite::mtk::Hash("quantizemtk");
}

}  // namespace quantize

TfLiteRegistration* Register_MTK_QUANTIZE_REF() {
  static TfLiteRegistration r = {nullptr, nullptr,
                                 quantize::QuantizePrepare,
                                 quantize::QuantizeEval};
  ::tflite::mtk::CustomOpHelper::GetInstance().SetMtkExtOpParameterFunc(
      "MTK_QUANTIZE", quantize::add_ann_params);
  return &r;
}

TfLiteRegistration* Register_MTK_QUANTIZE() {
    return Register_MTK_QUANTIZE_REF();
}

}  // namespace mtk
}  // namespace ops
}  // namespace tflite

