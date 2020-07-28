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
#include "tensorflow/lite/mtk/kernels/mtk_ops.h"

namespace tflite {
namespace ops {
namespace mtk {
namespace elu {

constexpr int kDataInputTensor = 0;
constexpr int kOutputTensor = 0;

enum KernelType {
  kReference,
};


void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  return new OpData;
}

void Free(TfLiteContext* context, void* buffer) {
  delete reinterpret_cast<OpData*>(buffer);
}


TfLiteStatus EluPrepare(TfLiteContext* context, TfLiteNode* node) {
  OpData* data = reinterpret_cast<OpData*>(node->user_data);

  TF_LITE_ENSURE_EQ(context, NumInputs(node), 1);
  TF_LITE_ENSURE_EQ(context, NumOutputs(node), 1);
  const TfLiteTensor* input = GetInput(context, node, kDataInputTensor);
  TfLiteTensor* output = GetOutput(context, node, kOutputTensor);

  const TfLiteType input_type = input->type;
  const TfLiteType output_type = output->type;

  if (input_type == kTfLiteFloat32) {
    TF_LITE_ENSURE_EQ(context, input->type, output->type);
  }
  else {
    TF_LITE_ENSURE(context, input_type == kTfLiteUInt8 || input_type == kTfLiteInt16);
    TF_LITE_ENSURE(context, output_type == kTfLiteUInt8 || output_type == kTfLiteInt16);
  }

  if (input->type == kTfLiteUInt8 || input->type == kTfLiteInt16) {
    static constexpr int kInputIntegerBits = 4;

    const double input_real_multiplier =
        input->params.scale *
        static_cast<double>(1 << (31 - kInputIntegerBits));

    const double output_real_multiplier =
        1.0 / (static_cast<double>(1 << (31 - kInputIntegerBits)) * output->params.scale);

    QuantizeMultiplierGreaterThanOne(input_real_multiplier,
                                     &data->input_multiplier,
                                     &data->input_left_shift);

    QuantizeMultiplierSmallerThanOneExp(output_real_multiplier,
                                        &data->output_multiplier,
                                        &data->output_left_shift);
    data->input_range_radius =
        CalculateInputRadius(kInputIntegerBits, data->input_left_shift);
  }

  return context->ResizeTensor(context, output,
      TfLiteIntArrayCopy(input->dims));
}


TfLiteStatus EluEval(TfLiteContext* context, TfLiteNode* node) {
  const TfLiteTensor* input = GetInput(context, node, 0);
  TfLiteTensor* output = GetOutput(context, node, 0);
  OpData* data = reinterpret_cast<OpData*>(node->user_data);
  switch (input->type) {
    case kTfLiteFloat32: {
      size_t elements = input->bytes / sizeof(float);
      float* in = input->data.f;
      float* in_end = in + elements;
      float* out = output->data.f;
      for (; in < in_end; in++, out++){
          float pos = std::max(*in, 0.f);
          float min = std::min(*in, 0.f);
          float expo = (std::exp(min))-1;
          *out = pos + expo;
      }
      return kTfLiteOk;
    }
    break;
    case kTfLiteInt16:
    case kTfLiteUInt8: {
      tflite::MtkQuantizedEluParams op_params;
      op_params.input_range_radius = data->input_range_radius;
      op_params.input_offset = -input->params.zero_point;
      op_params.output_offset = output->params.zero_point;
      op_params.input_multiplier = data->input_multiplier;
      op_params.input_shift = data->input_left_shift;
      op_params.output_multiplier = data->output_multiplier;
      op_params.output_shift = data->output_left_shift;
      op_params.quantized_one = std::round(1.0 / output->params.scale);
      CalculateActivationRangeQuantized(context, kTfLiteActNone, output,
                                        &op_params.quantized_activation_min,
                                        &op_params.quantized_activation_max);

#define TF_LITE_QUANTIZED_ELU(input_type, output_type)                \
      do {                                                            \
        reference_ops_mtk_nbits::Elu<input_type, output_type>(        \
            op_params,                                                \
            GetTensorShape(input), GetTensorData<uint8_t>(input),     \
            GetTensorShape(output), GetTensorData<uint8_t>(output));  \
      } while (0)

      const auto itype = input->type;
      const auto otype = output->type;

      if (itype == kTfLiteUInt8 && otype == kTfLiteUInt8)
        TF_LITE_QUANTIZED_ELU(uint8_t, uint8_t);
      else if (itype == kTfLiteUInt8 && otype == kTfLiteInt16)
        TF_LITE_QUANTIZED_ELU(uint8_t, int16_t);
      else if (itype == kTfLiteInt16 && otype == kTfLiteUInt8)
        TF_LITE_QUANTIZED_ELU(int16_t, uint8_t);
      else if (itype == kTfLiteInt16 && otype == kTfLiteInt16)
      TF_LITE_QUANTIZED_ELU(int16_t, int16_t);

#undef TF_LITE_QUANTIZED_ELU

      return kTfLiteOk;
    }
    break;
    default:
      context->ReportError(context, "Only float32, uint8 and int32 supported currently.");
      return kTfLiteError;
  }
}

#define CHECK_NN(x)                                         \
  if (x != ANEURALNETWORKS_NO_ERROR) {                      \
    exit(1);                                                \
  }

int32_t add_ann_params(ANeuralNetworksModel* nn_model,
        std::vector<uint32_t>& augmented_inputs, uint32_t& next_id, void* data) {
  auto add_scalar_int32 = [&nn_model, &augmented_inputs,
                           &next_id](int value) {
    ANeuralNetworksOperandType operand_type{.type = ANEURALNETWORKS_INT32};
    CHECK_NN(ANeuralNetworksModel_addOperand(nn_model, &operand_type))
    CHECK_NN(ANeuralNetworksModel_setOperandValue(nn_model, next_id, &value,
                                                  sizeof(int32_t)))
    augmented_inputs.push_back(next_id++);
  };

  auto builtin = reinterpret_cast<ops::mtk::elu::OpData*>(data);
  add_scalar_int32(builtin->input_multiplier);
  add_scalar_int32(builtin->input_left_shift);
  add_scalar_int32(builtin->output_multiplier);
  add_scalar_int32(builtin->output_left_shift);
  add_scalar_int32(builtin->input_range_radius);
  add_scalar_int32(builtin->diff_min);
  add_scalar_int32(builtin->new_output_shift);
  return ::tflite::mtk::Hash("elumtk");
}

}  // namespace elu

TfLiteRegistration* Register_MTK_ELU_REF() {
  static TfLiteRegistration r = {elu::Init,
                                 elu::Free,
                                 elu::EluPrepare,
                                 elu::EluEval};
  ::tflite::mtk::CustomOpHelper::GetInstance().SetMtkExtOpParameterFunc(
        "MTK_ELU", elu::add_ann_params);
  return &r;
}

TfLiteRegistration* Register_MTK_ELU() {
    return Register_MTK_ELU_REF();
}

}  // namespace mtk
}  // namespace ops
}  // namespace tflite
