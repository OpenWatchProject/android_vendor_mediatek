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
#include "tensorflow/lite/mtk/kernels/mtk_ops.h"
#include "tensorflow/lite/nnapi/NeuralNetworksShim.h"
#include "tensorflow/lite/mtk/mtk_utils.h"
#include "tensorflow/lite/mtk/mtk_helper.h"
#include "flatbuffers/flexbuffers.h"

namespace tflite {
namespace ops {
namespace mtk {
namespace channel_shuffle {

constexpr int kInputTensor = 0;
constexpr int kOutputTensor = 0;

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  OpData* data = new OpData;

  const uint8_t* buffer_t = reinterpret_cast<const uint8_t*>(buffer);
  const flexbuffers::Map& m = flexbuffers::GetRoot(buffer_t, length).AsMap();
  data->num_groups = m["num_groups"].AsInt32();

  return data;
}

void Free(TfLiteContext* context, void* buffer) {
  delete reinterpret_cast<OpData*>(buffer);
}

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  OpData* data = reinterpret_cast<OpData*>(node->user_data);

  TF_LITE_ENSURE_EQ(context, NumInputs(node), 1);
  TF_LITE_ENSURE_EQ(context, NumOutputs(node), 1);

  const TfLiteTensor* input = GetInput(context, node, kInputTensor);
  TfLiteTensor* output = GetOutput(context, node, kOutputTensor);

  // Currently only support 4D input
  TF_LITE_ENSURE_EQ(context, NumDimensions(input), 4);
  TF_LITE_ENSURE(context, data->num_groups >= 1);
  TF_LITE_ENSURE_EQ(context, SizeOfDimension(input, 3) % data->num_groups, 0);

  TF_LITE_ENSURE_EQ(context, input->type, output->type);
  if (input->type == kTfLiteUInt8 || input->type == kTfLiteInt16) {
    TF_LITE_ENSURE_EQ(context, input->params.scale, output->params.scale);
    TF_LITE_ENSURE_EQ(context,
        input->params.zero_point, output->params.zero_point);
  }

  // FIXME: output size does not have batch
  return context->ResizeTensor(context, output,
                               TfLiteIntArrayCopy(input->dims));
}

TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {
  OpData* data = reinterpret_cast<OpData*>(node->user_data);

  const TfLiteTensor* input = GetInput(context, node, kInputTensor);
  TfLiteTensor* output = GetOutput(context, node, kOutputTensor);

  MtkChannelShuffleParams op_params;
  op_params.num_groups = data->num_groups;

#define TFLITE_CHANNEL_SHUFFLE(elem_type)                           \
  do {                                                              \
    reference_ops_mtk_nbits::ChannelShuffle(op_params,              \
        GetTensorShape(input), GetTensorData<elem_type>(input),     \
        GetTensorShape(output), GetTensorData<elem_type>(output));  \
  } while(0)

  switch (input->type) {
    case kTfLiteFloat32:
      TFLITE_CHANNEL_SHUFFLE(float);
      break;
    case kTfLiteUInt8:
      TFLITE_CHANNEL_SHUFFLE(uint8_t);
      break;
    case kTfLiteInt16:
      TFLITE_CHANNEL_SHUFFLE(int16_t);
      break;
    default:
      context->ReportError(context,
          "Type %d is currently not supported by "
          "MtkChannelShuffle.", input->type);
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
    ANeuralNetworksOperandType operand_type{
        .type = ANEURALNETWORKS_INT32};
    CHECK_NN(ANeuralNetworksModel_addOperand(nn_model, &operand_type))
    CHECK_NN(ANeuralNetworksModel_setOperandValue(nn_model, next_id, &value,
                                                  sizeof(int32_t)))
    augmented_inputs.push_back(next_id++);
  };

  auto builtin = reinterpret_cast<OpData*>(data);
  add_scalar_int32(builtin->num_groups);
  return ::tflite::mtk::Hash("channelshuffletmtk");
}

}  // namespace channel_shuffle

TfLiteRegistration* Register_MTK_CHANNEL_SHUFFLE() {
  static TfLiteRegistration r = {channel_shuffle::Init,
                                 channel_shuffle::Free,
                                 channel_shuffle::Prepare,
                                 channel_shuffle::Eval};
  ::tflite::mtk::CustomOpHelper::GetInstance().SetMtkExtOpParameterFunc(
      "MTK_CHANNEL_SHUFFLE", channel_shuffle::add_ann_params);
  return &r;
}

}  // namespace mtk
}  // namespace ops
}  // namespace tflite
