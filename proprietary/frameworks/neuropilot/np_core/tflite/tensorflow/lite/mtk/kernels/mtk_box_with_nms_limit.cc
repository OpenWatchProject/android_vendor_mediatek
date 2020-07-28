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
#include "tensorflow/lite/string_util.h"
#include "tensorflow/lite/mtk/kernels/mtk_ops.h"
#include "tensorflow/lite/nnapi/NeuralNetworksShim.h"
#include "tensorflow/lite/mtk/mtk_utils.h"
#include "tensorflow/lite/mtk/mtk_helper.h"
#include "flatbuffers/flexbuffers.h"

namespace tflite {
namespace ops {
namespace mtk {
namespace box_with_nms_limit {

constexpr int kScoreTensor = 0;
constexpr int kBoxTensor = 1;
constexpr int kBatchSplitTensor = 2;
constexpr int kOutputScoreTensor = 0;
constexpr int kOutputBoxTensor = 1;
constexpr int kOutputBoxIndexTensor = 2;
constexpr int kOutputClassTensor = 3;
constexpr int kOutputBatchSplitTensor = 4;

enum KernelType {
  kReference,
};

TfLiteStatus checkLimitation(TfLiteContext* context, OpData* data) {
  TF_LITE_ENSURE(context, data->detection_per_image > 0);
  // MTK limitation
  TF_LITE_ENSURE_EQ(context, data->soft_nms_method, 0); // 0 imply hard nms
  return kTfLiteOk;
}

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  OpData* data = new OpData;

  const uint8_t* buffer_t = reinterpret_cast<const uint8_t*>(buffer);
  const flexbuffers::Map& m = flexbuffers::GetRoot(buffer_t, length).AsMap();
  data->score_threshold = m["score_threshold"].AsDouble();
  data->nms_threshold = m["nms_threshold"].AsDouble();
  data->detection_per_image = m["detection_per_image"].AsInt64();
  data->soft_nms_method = m["soft_nms_method"].AsInt64();
  data->sigma = m["sigma"].AsDouble();

  checkLimitation(context, data);

  return data;
}

void Free(TfLiteContext* context, void* buffer) {
  delete reinterpret_cast<OpData*>(buffer);
}

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  OpData* data = reinterpret_cast<OpData*>(node->user_data);

  // check node input/output
  TF_LITE_ENSURE_EQ(context, NumInputs(node), 3);
  TF_LITE_ENSURE_EQ(context, NumOutputs(node), 5);
  const TfLiteTensor* score = GetInput(context, node, kScoreTensor);
  const TfLiteTensor* box = GetInput(context, node, kBoxTensor);
  const TfLiteTensor* batch_split = GetInput(context, node, kBatchSplitTensor);
  TfLiteTensor* out_score = GetOutput(context, node, kOutputScoreTensor);
  TfLiteTensor* out_box = GetOutput(context, node, kOutputBoxTensor);
  TfLiteTensor* out_box_index = GetOutput(context, node, kOutputBoxIndexTensor);
  TfLiteTensor* out_class = GetOutput(context, node, kOutputClassTensor);
  TfLiteTensor* out_batch_split = GetOutput(context, node, kOutputBatchSplitTensor);

  // check input dimension
  TF_LITE_ENSURE_EQ(context, NumDimensions(score), 2);
  TF_LITE_ENSURE_EQ(context, NumDimensions(box), 2);
  TF_LITE_ENSURE_EQ(context, SizeOfDimension(box, 0), SizeOfDimension(score, 0));
  TF_LITE_ENSURE_EQ(context, SizeOfDimension(box, 1), SizeOfDimension(score, 1)*4);
  TF_LITE_ENSURE_EQ(context, NumDimensions(batch_split), 1);
  auto max_detection = SizeOfDimension(batch_split, 0) * data->detection_per_image;
  if (NumDimensions(out_score) != 0) {
    TF_LITE_ENSURE_EQ(context, NumDimensions(out_score), 1);
    TF_LITE_ENSURE_EQ(context, SizeOfDimension(out_score, 0), max_detection);
  }
  if (NumDimensions(out_box) != 0) {
    TF_LITE_ENSURE_EQ(context, NumDimensions(out_box), 2);
    TF_LITE_ENSURE_EQ(context, SizeOfDimension(out_box, 0), max_detection);
    TF_LITE_ENSURE_EQ(context, SizeOfDimension(out_box, 1), 4);
  }
  if (NumDimensions(out_box_index) != 0) {
    TF_LITE_ENSURE_EQ(context, NumDimensions(out_box_index), 1);
    TF_LITE_ENSURE_EQ(context, SizeOfDimension(out_box_index, 0), max_detection);
  }
  if (NumDimensions(out_class) != 0) {
    TF_LITE_ENSURE_EQ(context, NumDimensions(out_class), 1);
    TF_LITE_ENSURE_EQ(context, SizeOfDimension(out_class, 0), max_detection);
  }
  if (NumDimensions(out_batch_split) != 0) {
    TF_LITE_ENSURE_EQ(context, NumDimensions(out_batch_split), 1);
    TF_LITE_ENSURE_EQ(context, SizeOfDimension(out_batch_split, 0), SizeOfDimension(batch_split, 0));
  }

  // check data type
  const TfLiteType data_type = score->type;
  TF_LITE_ENSURE(context, data_type == kTfLiteFloat32);
  TF_LITE_ENSURE_EQ(context, box->type, data_type);
  TF_LITE_ENSURE_EQ(context, batch_split->type, kTfLiteInt32);
  TF_LITE_ENSURE_EQ(context, out_score->type, data_type);
  TF_LITE_ENSURE_EQ(context, out_box->type, data_type);
  TF_LITE_ENSURE_EQ(context, out_box_index->type, kTfLiteInt32);
  TF_LITE_ENSURE_EQ(context, out_class->type, kTfLiteInt32);
  TF_LITE_ENSURE_EQ(context, out_batch_split->type, kTfLiteInt32);

  // update output tensor size
  TfLiteIntArray* out_score_size = TfLiteIntArrayCreate(1);
  out_score_size->data[0] = max_detection;
  TfLiteIntArray* out_box_size = TfLiteIntArrayCreate(2);
  out_box_size->data[0] = max_detection;
  out_box_size->data[1] = 4;
  TfLiteIntArray* out_box_index_size = TfLiteIntArrayCreate(1);
  out_box_index_size->data[0] = max_detection;
  TfLiteIntArray* out_class_size = TfLiteIntArrayCreate(1);
  out_class_size->data[0] = max_detection;
  TfLiteIntArray* out_batch_split_size = TfLiteIntArrayCreate(1);
  out_batch_split_size->data[0] = SizeOfDimension(batch_split, 0);

  TfLiteStatus ret = kTfLiteOk;
  ret = context->ResizeTensor(context, out_score, out_score_size);
  if (ret != kTfLiteOk) {
    TfLiteIntArrayFree(out_box_size);
    TfLiteIntArrayFree(out_box_index_size);
    TfLiteIntArrayFree(out_class_size);
    TfLiteIntArrayFree(out_batch_split_size);
    return ret;
  }
  ret = context->ResizeTensor(context, out_box, out_box_size);
  if (ret != kTfLiteOk) {
    TfLiteIntArrayFree(out_box_index_size);
    TfLiteIntArrayFree(out_class_size);
    TfLiteIntArrayFree(out_batch_split_size);
    return ret;
  }
  ret = context->ResizeTensor(context, out_box_index, out_box_index_size);
  if (ret != kTfLiteOk) {
    TfLiteIntArrayFree(out_class_size);
    TfLiteIntArrayFree(out_batch_split_size);
    return ret;
  }
  ret = context->ResizeTensor(context, out_class, out_class_size);
  if (ret != kTfLiteOk) {
    TfLiteIntArrayFree(out_batch_split_size);
    return ret;
  }
  ret = context->ResizeTensor(context, out_batch_split, out_batch_split_size);
  if (ret != kTfLiteOk) {
    return ret;
  }
  return kTfLiteOk;
}

template <KernelType kernel_type>
void EvalFloat(TfLiteContext* context, TfLiteNode* node,
               const TfLiteTensor* score, const TfLiteTensor* box,
               const TfLiteTensor* batch_split, const OpData* params,
               TfLiteTensor* out_score, TfLiteTensor* out_box,
               TfLiteTensor* out_box_index, TfLiteTensor* out_class,
               TfLiteTensor* out_batch_split) {
#define MTK_BOX_WITH_NMS_LIMIT()                                                             \
  reference_ops_mtk::BoxWithNmsLimit                                                         \
              (GetTensorData<float>(score), GetTensorShape(score),                           \
               GetTensorData<float>(box), GetTensorShape(box),                               \
               GetTensorData<int32_t>(batch_split), GetTensorShape(batch_split),             \
               params->score_threshold, params->nms_threshold, params->detection_per_image,  \
               params->soft_nms_method, params->sigma,                                       \
               GetTensorData<float>(out_score), GetTensorShape(out_score),                   \
               GetTensorData<float>(out_box), GetTensorShape(out_box),                       \
               GetTensorData<int32_t>(out_class), GetTensorShape(out_class),                 \
               GetTensorData<int32_t>(out_batch_split), GetTensorShape(out_batch_split),     \
               GetTensorData<int32_t>(out_box_index), GetTensorShape(out_box_index))
  if (kernel_type == kReference) {
    MTK_BOX_WITH_NMS_LIMIT();
  } else {
    // TBD
  }
#undef MTK_BOX_WITH_NMS_LIMIT
}

template <KernelType kernel_type>
TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {
  OpData* data = reinterpret_cast<OpData*>(node->user_data);

  const TfLiteTensor* score = GetInput(context, node, kScoreTensor);
  const TfLiteTensor* box = GetInput(context, node, kBoxTensor);
  const TfLiteTensor* batch_split = GetInput(context, node, kBatchSplitTensor);
  TfLiteTensor* out_score = GetOutput(context, node, kOutputScoreTensor);
  TfLiteTensor* out_box = GetOutput(context, node, kOutputBoxTensor);
  TfLiteTensor* out_box_index = GetOutput(context, node, kOutputBoxIndexTensor);
  TfLiteTensor* out_class = GetOutput(context, node, kOutputClassTensor);
  TfLiteTensor* out_batch_split = GetOutput(context, node, kOutputBatchSplitTensor);

  // check batch split
  const int32* batch_split_data = GetTensorData<int32>(batch_split);
  int32 all_box = 0;
  for (auto i = 0 ; i < SizeOfDimension(batch_split, 0) ; i++) {
    TF_LITE_ENSURE(context, batch_split_data[i] >= 0);
    all_box += batch_split_data[i];
  }
  TF_LITE_ENSURE(context, all_box <= SizeOfDimension(score, 0));

  switch (score->type) {
    case kTfLiteFloat32: {
      EvalFloat<kernel_type>(context, node, score, box, batch_split, data,
                             out_score, out_box, out_box_index, out_class,
                             out_batch_split);
      return kTfLiteOk;
    }
    break;
    default:
      context->ReportError(context, "Only float32 is supported currently.");
      return kTfLiteError;
  }
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

  auto add_scalar_float32 = [&nn_model, &augmented_inputs,
                             &next_id](float value) {
    ANeuralNetworksOperandType operand_type{
        .type = ANEURALNETWORKS_FLOAT32};
    CHECK_NN(ANeuralNetworksModel_addOperand(nn_model, &operand_type))
    CHECK_NN(ANeuralNetworksModel_setOperandValue(nn_model, next_id, &value,
                                                  sizeof(float)))
    augmented_inputs.push_back(next_id++);
  };

  auto builtin = reinterpret_cast<OpData*>(data);

  add_scalar_float32(builtin->score_threshold);
  add_scalar_float32(builtin->nms_threshold);
  add_scalar_int32(builtin->detection_per_image);
  add_scalar_int32(builtin->soft_nms_method);
  add_scalar_float32(builtin->sigma);
  return ::tflite::mtk::Hash("boxwithnmslimitmtk");
}

}  // namespace box_with_nms_limit

TfLiteRegistration* Register_MTK_BOX_WITH_NMS_LIMIT_REF() {
  static TfLiteRegistration r = {box_with_nms_limit::Init,
                                 box_with_nms_limit::Free,
                                 box_with_nms_limit::Prepare,
                                 box_with_nms_limit::Eval<box_with_nms_limit::kReference>};
  ::tflite::mtk::CustomOpHelper::GetInstance().SetMtkExtOpParameterFunc(
      "MTK_BOX_WITH_NMS_LIMIT", box_with_nms_limit::add_ann_params);
  return &r;
}

TfLiteRegistration* Register_MTK_BOX_WITH_NMS_LIMIT() {
    return Register_MTK_BOX_WITH_NMS_LIMIT_REF();
}

}  // namespace mtk
}  // namespace ops
}  // namespace tflite
