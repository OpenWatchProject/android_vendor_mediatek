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
#include "tensorflow/lite/kernels/internal/quantization_util.h"
#include "tensorflow/lite/kernels/internal/tensor.h"
#include "tensorflow/lite/kernels/kernel_util.h"
#include "tensorflow/lite/nnapi/NeuralNetworksShim.h"
#include "tensorflow/lite/mtk/mtk_utils.h"
#include "tensorflow/lite/mtk/mtk_helper.h"
#include "tensorflow/lite/mtk/kernels/internal/mtk_types.h"

namespace tflite {
namespace ops {
namespace mtk {
namespace absolute {

TfLiteStatus AbsPrepare(TfLiteContext* context, TfLiteNode* node) {
  TF_LITE_ENSURE_EQ(context, NumInputs(node), 1);
  TF_LITE_ENSURE_EQ(context, NumOutputs(node), 1);
  const TfLiteTensor* input = GetInput(context, node, 0);
  TfLiteTensor* output = GetOutput(context, node, 0);

  if (input->type == kTfLiteFloat32) {
    TF_LITE_ENSURE_EQ(context, input->type, output->type);
  } else {
    TF_LITE_ENSURE(context, input->type == kTfLiteInt16 ||
                            input->type == kTfLiteUInt8);
    TF_LITE_ENSURE(context, output->type == kTfLiteInt16 ||
                            output->type == kTfLiteUInt8);
  }

  return context->ResizeTensor(context, output,
                               TfLiteIntArrayCopy(input->dims));
}


TfLiteStatus AbsEval(TfLiteContext* context, TfLiteNode* node) {
  const TfLiteTensor* input = GetInput(context, node, 0);
  TfLiteTensor* output = GetOutput(context, node, 0);

  if (input->type == kTfLiteFloat32) {
    size_t elements = input->bytes / sizeof(float);
    float* in = input->data.f;
    float* in_end = in + elements;
    float* out = output->data.f;
    for (; in < in_end; in++, out++) *out = std::fabs(*in);
    return kTfLiteOk;
  } else if (input->type == kTfLiteInt16 || input->type == kTfLiteUInt8) {

    const double input_scale = input->params.scale;
    const double output_scale = output->params.scale;
    const double real_output_multiplier = input_scale / output_scale;

    tflite::MtkQuantizedAbsParams op_params;
    op_params.input_offset = -input->params.zero_point;
    op_params.output_offset = output->params.zero_point;
    QuantizeMultiplier(real_output_multiplier,
                       &op_params.output_multiplier, &op_params.output_shift);
    CalculateActivationRangeQuantized(context, kTfLiteActNone, output,
                                      &op_params.quantized_activation_min,
                                      &op_params.quantized_activation_max);

#define TF_LITE_QUANTIZED_ABS(input_type, output_type)                      \
    do {                                                                    \
      reference_ops_mtk_nbits::Abs<input_type, output_type>(op_params,      \
                  GetTensorShape(input), GetTensorData<uint8_t>(input),     \
                  GetTensorShape(output), GetTensorData<uint8_t>(output));  \
    } while (0)

    const auto itype = input->type;
    const auto otype = output->type;

    if (itype == kTfLiteUInt8 && otype == kTfLiteUInt8)
      TF_LITE_QUANTIZED_ABS(uint8_t, uint8_t);
    else if (itype == kTfLiteUInt8 && otype == kTfLiteInt16)
      TF_LITE_QUANTIZED_ABS(uint8_t, int16_t);
    else if (itype == kTfLiteInt16 && otype == kTfLiteUInt8)
      TF_LITE_QUANTIZED_ABS(int16_t, uint8_t);
    else if (itype == kTfLiteInt16 && otype == kTfLiteInt16)
      TF_LITE_QUANTIZED_ABS(int16_t, int16_t);

#undef TF_LITE_QUANTIZED_ABS

    return kTfLiteOk;
  } else {
    context->ReportError(context,
        "Only float32, int16, and uint8 input types are supported currently.");
    return kTfLiteError;
  }
}

int32_t add_ann_params(ANeuralNetworksModel* nn_model,
                       std::vector<uint32_t>& augmented_inputs,
                       uint32_t& next_id,
                       void* data) {
  return ::tflite::mtk::Hash("absmtk");
}

}  // namespace absolute

TfLiteRegistration* Register_MTK_ABS_REF() {
  static TfLiteRegistration r = {/*init=*/nullptr, /*free=*/nullptr,
                                 absolute::AbsPrepare,
                                 absolute::AbsEval};
  ::tflite::mtk::CustomOpHelper::GetInstance().SetMtkExtOpParameterFunc(
      "MTK_ABS", absolute::add_ann_params);
  return &r;
}

TfLiteRegistration* Register_MTK_ABS() {
    return Register_MTK_ABS_REF();
}

}  // namespace mtk
}  // namespace ops
}  // namespace tflite

