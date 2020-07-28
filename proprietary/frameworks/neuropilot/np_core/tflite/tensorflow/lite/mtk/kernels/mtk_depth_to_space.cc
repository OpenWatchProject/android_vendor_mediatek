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

#include "tensorflow/lite/c/builtin_op_data.h"
#include "tensorflow/lite/c/c_api_internal.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/kernels/internal/tensor.h"
#include "tensorflow/lite/kernels/kernel_util.h"
#include "tensorflow/lite/kernels/op_macros.h"
#include "flatbuffers/flexbuffers.h"
#include "tensorflow/lite/nnapi/NeuralNetworksShim.h"

#define LOG_TAG "MtkDepthToSpace"
#include "tensorflow/lite/mtk/mtk_minimal_logging.h"
#include "tensorflow/lite/mtk/kernels/mtk_ops.h"
#include "tensorflow/lite/mtk/kernels/internal/reference/mtk_reference_ops.h"
#include "tensorflow/lite/mtk/mtk_utils.h"
#include "tensorflow/lite/mtk/mtk_helper.h"

namespace tflite {
namespace ops {
namespace mtk {
namespace depth_to_space {

// This file has two implementation of DepthToSpace. Note that DepthToSpace
// only works on 4D tensors.
enum KernelType {
 kReference,
};


void* Init(TfLiteContext* context, const char* buffer, size_t length) {

  OpData* data = new OpData;

  const uint8_t* buffer_t = reinterpret_cast<const uint8_t*>(buffer);
  const flexbuffers::Map& m = flexbuffers::GetRoot(buffer_t, length).AsMap();

  data->block_size = m["block_size"].AsInt64();

  return data;
}

void Free(TfLiteContext* context, void* buffer) {
  delete reinterpret_cast<OpData*>(buffer);
}


constexpr int kInputTensor = 0;
constexpr int kOutputTensor = 0;

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {

  OpData* data = reinterpret_cast<OpData*>(node->user_data);

  TF_LITE_ENSURE_EQ(context, NumInputs(node), 1);
  TF_LITE_ENSURE_EQ(context, NumOutputs(node), 1);

  const TfLiteTensor* input = GetInput(context, node, kInputTensor);
  TfLiteTensor* output = GetOutput(context, node, kOutputTensor);

  TF_LITE_ENSURE_EQ(context, NumDimensions(input), 4);

  auto data_type = output->type;
  TF_LITE_ENSURE(context,
                 data_type == kTfLiteFloat32 ||
                 data_type == kTfLiteUInt8 ||
                 data_type == kTfLiteInt16);
  TF_LITE_ENSURE_EQ(context, input->type, output->type);

  if (data_type == kTfLiteUInt8 || data_type == kTfLiteInt16) {
    TF_LITE_ENSURE_EQ(context, input->params.scale, output->params.scale);
    TF_LITE_ENSURE_EQ(context, input->params.zero_point, output->params.zero_point);
  }

  const int block_size = data->block_size;
  const int input_height = input->dims->data[1];
  const int input_width = input->dims->data[2];
  int output_height = input_height * block_size;
  int output_width = input_width * block_size;

  TF_LITE_ENSURE_EQ(context, input_height * block_size, output_height);
  TF_LITE_ENSURE_EQ(context, input_width * block_size, output_width);

  TfLiteIntArray* output_size = TfLiteIntArrayCreate(4);
  output_size->data[0] = input->dims->data[0];
  output_size->data[1] = output_height;
  output_size->data[2] = output_width;
  output_size->data[3] = input->dims->data[3] / block_size / block_size;

  return context->ResizeTensor(context, output, output_size);
}

template <KernelType kernel_type>
TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {

  OpData* data = reinterpret_cast<OpData*>(node->user_data);

  const TfLiteTensor* input = GetInput(context, node, kInputTensor);
  TfLiteTensor* output = GetOutput(context, node, kOutputTensor);

  tflite::DepthToSpaceParams op_params;
  op_params.block_size = data->block_size;

#define TF_LITE_DEPTH_TO_SPACE(scalar)                                          \
  reference_ops::DepthToSpace<scalar>(                                          \
      op_params,                                                                \
      GetTensorShape(input), GetTensorData<scalar>(input),                      \
      GetTensorShape(output), GetTensorData<scalar>(output))                    \

  switch (input->type) {  // Already know in/out types are same.
    case kTfLiteFloat32:
      TF_LITE_DEPTH_TO_SPACE(float);
      break;
    case kTfLiteInt16:
      TF_LITE_DEPTH_TO_SPACE(int16_t);
      break;
    case kTfLiteUInt8:
      TF_LITE_DEPTH_TO_SPACE(uint8_t);
      break;
    default:
      context->ReportError(context, "Type %d not currently supported.", input->type);
      return kTfLiteError;
  }
#undef TF_LITE_DEPTH_TO_SPACE

  return kTfLiteOk;
}

#define CHECK_NN(x)                                               \
  if (x != ANEURALNETWORKS_NO_ERROR) {                            \
    TFLITE_MTK_LOG_ERROR("Aborting since NN returned failure.");  \
    exit(1);                                                      \
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

  auto builtin = reinterpret_cast<ops::mtk::depth_to_space::OpData*>(data);
  add_scalar_int32(builtin->block_size);
  return -1;
}

}  // namespace depth_to_space

TfLiteRegistration* Register_MTK_DEPTH_SPACE_TO_REF() {
  static TfLiteRegistration r = {
      depth_to_space::Init, depth_to_space::Free, depth_to_space::Prepare,
      depth_to_space::Eval<depth_to_space::kReference>};
  ::tflite::mtk::CustomOpHelper::GetInstance().SetMtkExtOpParameterFunc(
      "MTK_DEPTH_TO_SPACE", depth_to_space::add_ann_params);
  return &r;
}

TfLiteRegistration* Register_MTK_DEPTH_TO_SPACE() {
  return Register_MTK_DEPTH_SPACE_TO_REF();
}

}  // namespace mtk
}  // namespace ops



}  // namespace tflite
