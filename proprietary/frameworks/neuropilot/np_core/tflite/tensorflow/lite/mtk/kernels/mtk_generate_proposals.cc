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
#include "tensorflow/lite/mtk/kernels/internal/mtk_types.h"
#include "flatbuffers/flexbuffers.h"

namespace tflite {
namespace ops {
namespace mtk {
namespace generate_proposals {

constexpr int kScoreTensor = 0;
constexpr int kDeltaTensor = 1;
constexpr int kAnchorTensor = 2;
constexpr int kImageSizeTensor = 3;
constexpr int kImageScaleTensor = 4;
constexpr int kDeltaWeightTensor = 5;
constexpr int kOutputBoxTensor = 0;
constexpr int kOutputBoxIndexTensor = 1;
constexpr int kOutputScoreTensor = 2;

enum KernelType {
  kReference,
  kGenericOptimized,  // Neon-free
  kNeonOptimized,
};

TfLiteStatus SetTensorSizes(TfLiteContext* context, TfLiteTensor* tensor,
                            std::initializer_list<int> values) {
  TfLiteIntArray* size = TfLiteIntArrayCreate(values.size());
  int index = 0;
  for (int v : values) {
    size->data[index] = v;
    ++index;
  }
  return context->ResizeTensor(context, tensor, size);
}

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  OpData* data = new OpData;

  const uint8_t* buffer_t = reinterpret_cast<const uint8_t*>(buffer);
  const flexbuffers::Map& m = flexbuffers::GetRoot(buffer_t, length).AsMap();
  data->spatial_scale = m["spatial_scale"].AsDouble();
  data->min_size = m["min_size"].AsInt64();
  data->nms_threshold = m["nms_threshold"].AsDouble();
  data->pre_nms_top_n = m["pre_nms_top_n"].AsInt64();
  data->post_nms_top_n = m["post_nms_top_n"].AsInt64();

  context->AddTensors(context, 1, &data->transformed_box_index);

  return data;
}

void Free(TfLiteContext* context, void* buffer) {
  delete reinterpret_cast<OpData*>(buffer);
}

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  OpData* data = reinterpret_cast<OpData*>(node->user_data);

  // check node input/output
  TF_LITE_ENSURE_EQ(context, NumInputs(node), 6);
  TF_LITE_ENSURE_EQ(context, NumOutputs(node), 3);
  const TfLiteTensor* score = GetInput(context, node, kScoreTensor);
  const TfLiteTensor* delta = GetInput(context, node, kDeltaTensor);
  const TfLiteTensor* anchor = GetInput(context, node, kAnchorTensor);
  const TfLiteTensor* image_size = GetInput(context, node, kImageSizeTensor);
  const TfLiteTensor* image_scale = GetInput(context, node, kImageScaleTensor);
  const TfLiteTensor* delta_weight = GetInput(context, node, kDeltaWeightTensor);
  TfLiteTensor* out_box = GetOutput(context, node, kOutputBoxTensor);
  TfLiteTensor* out_box_index = GetOutput(context, node, kOutputBoxIndexTensor);
  TfLiteTensor* out_score = GetOutput(context, node, kOutputScoreTensor);

  // check input dimension
  TF_LITE_ENSURE_EQ(context, NumDimensions(score), 4);
  TF_LITE_ENSURE_EQ(context, NumDimensions(delta), 4);
  TF_LITE_ENSURE_EQ(context, SizeOfDimension(delta, 0), SizeOfDimension(score, 0));
  TF_LITE_ENSURE_EQ(context, SizeOfDimension(delta, 1), SizeOfDimension(score, 1));
  TF_LITE_ENSURE_EQ(context, SizeOfDimension(delta, 2), SizeOfDimension(score, 2));
  TF_LITE_ENSURE_EQ(context, SizeOfDimension(delta, 3), SizeOfDimension(score, 3)*4);
  TF_LITE_ENSURE_EQ(context, NumDimensions(anchor), 2);
  TF_LITE_ENSURE_EQ(context, SizeOfDimension(anchor, 0), SizeOfDimension(score, 3));
  TF_LITE_ENSURE_EQ(context, SizeOfDimension(anchor, 1), 4);
  TF_LITE_ENSURE_EQ(context, NumDimensions(image_size), 2);
  TF_LITE_ENSURE_EQ(context, SizeOfDimension(image_size, 1), 2);
  TF_LITE_ENSURE_EQ(context, NumDimensions(image_scale), 1);
  TF_LITE_ENSURE_EQ(context, SizeOfDimension(image_scale, 0), SizeOfDimension(image_size, 0));
  TF_LITE_ENSURE_EQ(context, NumDimensions(delta_weight), 1);
  TF_LITE_ENSURE_EQ(context, SizeOfDimension(delta_weight, 0), 4);
  if (NumDimensions(out_box) != 0) {
    TF_LITE_ENSURE_EQ(context, NumDimensions(out_box), 2);
    TF_LITE_ENSURE_EQ(context, SizeOfDimension(out_box, 0), data->post_nms_top_n);
    TF_LITE_ENSURE_EQ(context, SizeOfDimension(out_box, 1), 4);
  }
  if (NumDimensions(out_box_index) != 0) {
    TF_LITE_ENSURE_EQ(context, NumDimensions(out_box_index), 1);
    TF_LITE_ENSURE_EQ(context, SizeOfDimension(out_box_index, 0), data->post_nms_top_n);
  }
  if (NumDimensions(out_score) != 0) {
    TF_LITE_ENSURE_EQ(context, NumDimensions(out_score), 1);
    TF_LITE_ENSURE_EQ(context, SizeOfDimension(out_score, 0), data->post_nms_top_n);
  }

  // check data type
  const TfLiteType data_type = score->type;
  TF_LITE_ENSURE(context, data_type == kTfLiteFloat32);
  TF_LITE_ENSURE_EQ(context, delta->type, data_type);
  TF_LITE_ENSURE_EQ(context, anchor->type, data_type);
  TF_LITE_ENSURE_EQ(context, image_size->type, data_type);
  TF_LITE_ENSURE_EQ(context, image_scale->type, data_type);
  TF_LITE_ENSURE_EQ(context, delta_weight->type, data_type);
  TF_LITE_ENSURE_EQ(context, out_box->type, data_type);
  TF_LITE_ENSURE_EQ(context, out_box_index->type, kTfLiteInt32);
  TF_LITE_ENSURE_EQ(context, out_score->type, data_type);

  // set temp memory for transformed_box
  TfLiteIntArrayFree(node->temporaries);
  node->temporaries = TfLiteIntArrayCreate(1);
  node->temporaries->data[0] = data->transformed_box_index;
  TfLiteTensor* transformed_box = &context->tensors[data->transformed_box_index];
  transformed_box->type = data_type;
  transformed_box->allocation_type = kTfLiteArenaRw;
  SetTensorSizes(context, transformed_box,
                 {delta->dims->data[0],
                  delta->dims->data[1],
                  delta->dims->data[2],
                  delta->dims->data[3]});

  // update output tensor size
  TfLiteIntArray* out_box_size = TfLiteIntArrayCreate(2);
  out_box_size->data[0] = data->post_nms_top_n;
  out_box_size->data[1] = 4;
  TfLiteIntArray* out_box_index_size = TfLiteIntArrayCreate(1);
  out_box_index_size->data[0] = data->post_nms_top_n;
  TfLiteIntArray* out_score_size = TfLiteIntArrayCreate(1);
  out_score_size->data[0] = data->post_nms_top_n;

  TfLiteStatus ret = kTfLiteOk;
  ret = context->ResizeTensor(context, out_box, out_box_size);
  if (ret != kTfLiteOk) {
    TfLiteIntArrayFree(out_box_index_size);
    TfLiteIntArrayFree(out_score_size);
    return ret;
  }
  ret = context->ResizeTensor(context, out_box_index, out_box_index_size);
  if (ret != kTfLiteOk) {
    TfLiteIntArrayFree(out_score_size);
    return ret;
  }
  ret = context->ResizeTensor(context, out_score, out_score_size);
  if (ret != kTfLiteOk) {
    return ret;
  }
  return kTfLiteOk;
}

template <KernelType kernel_type>
void EvalFloat(TfLiteContext* context, TfLiteNode* node,
               const TfLiteTensor* score, const TfLiteTensor* delta,
               const TfLiteTensor* anchor, const TfLiteTensor* image_size,
               const TfLiteTensor* image_scale, const TfLiteTensor* delta_weight,
               TfLiteTensor* transformed_box, TfLiteTensor* out_box,
               TfLiteTensor* out_box_index, TfLiteTensor* out_score,
               OpData* data) {
#define MTK_GENERATE_PROPOSALS()                                                             \
  reference_ops_mtk::GenerateProposals                                                       \
              (GetTensorData<float>(score), GetTensorShape(score),                           \
               GetTensorData<float>(delta), GetTensorShape(delta),                           \
               GetTensorData<float>(anchor), GetTensorShape(anchor),                         \
               GetTensorData<float>(image_size), GetTensorShape(image_size),                 \
               GetTensorData<float>(image_scale), GetTensorShape(image_scale),               \
               GetTensorData<float>(delta_weight), GetTensorShape(delta_weight),             \
               GetTensorData<float>(transformed_box), GetTensorShape(transformed_box),       \
               data->min_size, data->pre_nms_top_n, data->post_nms_top_n,                    \
               data->spatial_scale, data->nms_threshold,                                     \
               GetTensorData<float>(out_box), GetTensorShape(out_box),                       \
               GetTensorData<float>(out_box_index), GetTensorShape(out_box_index),           \
               GetTensorData<float>(out_score), GetTensorShape(out_score))
  if (kernel_type == kReference) {
    MTK_GENERATE_PROPOSALS();
  } else {
    // TBD
  }
#undef MTK_GENERATE_PROPOSALS
}

template <KernelType kernel_type>
TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {
  OpData* data = reinterpret_cast<OpData*>(node->user_data);

  const TfLiteTensor* score = GetInput(context, node, kScoreTensor);
  const TfLiteTensor* delta = GetInput(context, node, kDeltaTensor);
  const TfLiteTensor* anchor = GetInput(context, node, kAnchorTensor);
  const TfLiteTensor* image_size = GetInput(context, node, kImageSizeTensor);
  const TfLiteTensor* image_scale = GetInput(context, node, kImageScaleTensor);
  const TfLiteTensor* delta_weight = GetInput(context, node, kDeltaWeightTensor);
  TfLiteTensor* transformed_box = GetTemporary(context, node, 0);
  TfLiteTensor* out_box = GetOutput(context, node, kOutputBoxTensor);
  TfLiteTensor* out_box_index = GetOutput(context, node, kOutputBoxIndexTensor);
  TfLiteTensor* out_score = GetOutput(context, node, kOutputScoreTensor);

  switch (score->type) {
    case kTfLiteFloat32: {
      EvalFloat<kernel_type>(context, node, score, delta, anchor, image_size,
                             image_scale, delta_weight, transformed_box, out_box,
                             out_box_index, out_score, data);
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
  add_scalar_float32(builtin->spatial_scale);
  add_scalar_int32(builtin->min_size);
  add_scalar_float32(builtin->nms_threshold);
  add_scalar_int32(builtin->pre_nms_top_n);
  add_scalar_int32(builtin->post_nms_top_n);
  add_scalar_int32(builtin->transformed_box_index);

  return ::tflite::mtk::Hash("generateproposalsmtk");
}

}  // namespace generate_proposals

TfLiteRegistration* Register_MTK_GENERATE_PROPOSALS_REF() {
  static TfLiteRegistration r = {generate_proposals::Init,
                                 generate_proposals::Free,
                                 generate_proposals::Prepare,
                                 generate_proposals::Eval<generate_proposals::kReference>};
  ::tflite::mtk::CustomOpHelper::GetInstance().SetMtkExtOpParameterFunc(
      "MTK_GENERATE_PROPOSALS", generate_proposals::add_ann_params);
  return &r;
}

TfLiteRegistration* Register_MTK_GENERATE_PROPOSALS() {
    return Register_MTK_GENERATE_PROPOSALS_REF();
}

}  // namespace mtk
}  // namespace ops
}  // namespace tflite
