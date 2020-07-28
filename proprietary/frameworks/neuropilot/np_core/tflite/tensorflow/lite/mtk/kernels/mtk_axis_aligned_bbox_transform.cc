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
namespace axis_aligned_bbox_transform {

constexpr int kBoxTensor = 0;
constexpr int kBoxIndexTensor = 1;
constexpr int kDeltaTensor = 2;
constexpr int kImageSizeTensor = 3;
constexpr int kImageScaleTensor = 4;
constexpr int kDeltaWeightTensor = 5;
constexpr int kOutputBoxTensor = 0;
constexpr int kOutputBatchSplitTensor = 1;

enum KernelType {
  kReference,
  kGenericOptimized,  // Neon-free
  kNeonOptimized,
};

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  OpData* data = new OpData;

  const uint8_t* buffer_t = reinterpret_cast<const uint8_t*>(buffer);
  const flexbuffers::Map& m = flexbuffers::GetRoot(buffer_t, length).AsMap();
  data->apply_scale = m["apply_scale"].AsBool();

  return data;
}

void Free(TfLiteContext* context, void* buffer) {
  delete reinterpret_cast<OpData*>(buffer);
}

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  // check node input/output
  TF_LITE_ENSURE_EQ(context, NumInputs(node), 6);
  TF_LITE_ENSURE_EQ(context, NumOutputs(node), 2);
  const TfLiteTensor* box = GetInput(context, node, kBoxTensor);
  const TfLiteTensor* box_index = GetInput(context, node, kBoxIndexTensor);
  const TfLiteTensor* delta = GetInput(context, node, kDeltaTensor);
  const TfLiteTensor* image_size = GetInput(context, node, kImageSizeTensor);
  const TfLiteTensor* image_scale = GetInput(context, node, kImageScaleTensor);
  const TfLiteTensor* delta_weight = GetInput(context, node, kDeltaWeightTensor);
  TfLiteTensor* out_box = GetOutput(context, node, kOutputBoxTensor);
  TfLiteTensor* out_batch_split = GetOutput(context, node, kOutputBatchSplitTensor);

  // check input dimension
  TF_LITE_ENSURE_EQ(context, NumDimensions(box), 2);
  TF_LITE_ENSURE_EQ(context, SizeOfDimension(box, 1), 4);
  TF_LITE_ENSURE_EQ(context, NumDimensions(box_index), 1);
  TF_LITE_ENSURE_EQ(context, SizeOfDimension(box_index, 0), SizeOfDimension(box, 0));
  TF_LITE_ENSURE_EQ(context, NumDimensions(delta), 2);
  TF_LITE_ENSURE_EQ(context, SizeOfDimension(delta, 0), SizeOfDimension(box, 0));
  TF_LITE_ENSURE_EQ(context, SizeOfDimension(delta, 1)%4, 0);
  TF_LITE_ENSURE_EQ(context, NumDimensions(image_size), 2);
  TF_LITE_ENSURE_EQ(context, SizeOfDimension(image_size, 1), 2);
  TF_LITE_ENSURE_EQ(context, NumDimensions(image_scale), 1);
  TF_LITE_ENSURE_EQ(context, SizeOfDimension(image_scale, 0), SizeOfDimension(image_size, 0));
  TF_LITE_ENSURE_EQ(context, NumDimensions(delta_weight), 1);
  TF_LITE_ENSURE_EQ(context, SizeOfDimension(delta_weight, 0), 4);
  if (NumDimensions(out_box) != 0) { // if shape already given
    TF_LITE_ENSURE_EQ(context, NumDimensions(out_box), 2);
    TF_LITE_ENSURE_EQ(context, SizeOfDimension(out_box, 0), SizeOfDimension(delta, 0));
    TF_LITE_ENSURE_EQ(context, SizeOfDimension(out_box, 1), SizeOfDimension(delta, 1));
  }
  if (NumDimensions(out_batch_split) != 0) { // if shape already given
    TF_LITE_ENSURE_EQ(context, NumDimensions(out_batch_split), 1);
    TF_LITE_ENSURE_EQ(context, SizeOfDimension(out_batch_split, 0), SizeOfDimension(image_size, 0));
  }

  // check data type
  const TfLiteType data_type = box->type;
  TF_LITE_ENSURE(context, data_type == kTfLiteFloat32);
  TF_LITE_ENSURE_EQ(context, box_index->type, kTfLiteInt32);
  TF_LITE_ENSURE_EQ(context, delta->type, data_type);
  TF_LITE_ENSURE_EQ(context, image_size->type, data_type);
  TF_LITE_ENSURE_EQ(context, image_scale->type, data_type);
  TF_LITE_ENSURE_EQ(context, delta_weight->type, data_type);
  TF_LITE_ENSURE_EQ(context, out_box->type, data_type);
  TF_LITE_ENSURE_EQ(context, out_batch_split->type, kTfLiteInt32);

  // update output tensor size
  TfLiteIntArray* out_box_size = TfLiteIntArrayCreate(2);
  out_box_size->data[0] = SizeOfDimension(delta, 0);
  out_box_size->data[1] = SizeOfDimension(delta, 1);
  TfLiteIntArray* out_batch_split_size = TfLiteIntArrayCreate(1);
  out_batch_split_size->data[0] = SizeOfDimension(image_size, 0);

  TfLiteStatus ret = kTfLiteOk;
  ret = context->ResizeTensor(context, out_box, out_box_size);
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
               const TfLiteTensor* box, const TfLiteTensor* box_index,
               const TfLiteTensor* delta, const TfLiteTensor* image_size,
               const TfLiteTensor* image_scale, const TfLiteTensor* delta_weight,
               bool apply_scale,
               TfLiteTensor* out_box, TfLiteTensor* out_batch_split) {
#define MTK_AXIS_ALIGNED_BBOX_TRANSFORM()                                           \
  reference_ops_mtk::AxisAlignedBBoxTransform                                       \
          (GetTensorData<float>(box), GetTensorShape(box),                          \
           GetTensorData<int32_t>(box_index),                                       \
           GetTensorData<float>(delta), GetTensorShape(delta),                      \
           GetTensorData<float>(image_size), GetTensorShape(image_size),            \
           GetTensorData<float>(image_scale), GetTensorShape(image_scale),          \
           GetTensorData<float>(delta_weight), GetTensorShape(delta_weight),        \
           apply_scale,                                                             \
           GetTensorData<float>(out_box), GetTensorShape(out_box),                  \
           GetTensorData<int32_t>(out_batch_split), GetTensorShape(out_batch_split))
  if (kernel_type == kReference) {
    MTK_AXIS_ALIGNED_BBOX_TRANSFORM();
  } else {
    // TBD
  }
#undef MTK_AXIS_ALIGNED_BBOX_TRANSFORM
}

template <KernelType kernel_type>
TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {
  OpData* data = reinterpret_cast<OpData*>(node->user_data);

  const TfLiteTensor* box = GetInput(context, node, kBoxTensor);
  const TfLiteTensor* box_index = GetInput(context, node, kBoxIndexTensor);
  const TfLiteTensor* delta = GetInput(context, node, kDeltaTensor);
  const TfLiteTensor* image_size = GetInput(context, node, kImageSizeTensor);
  const TfLiteTensor* image_scale = GetInput(context, node, kImageScaleTensor);
  const TfLiteTensor* delta_weight = GetInput(context, node, kDeltaWeightTensor);
  TfLiteTensor* out_box = GetOutput(context, node, kOutputBoxTensor);
  TfLiteTensor* out_batch_split = GetOutput(context, node, kOutputBatchSplitTensor);

  // check index range
  const int32* box_index_data = GetTensorData<int32>(box_index);
  const int32 input_batch = SizeOfDimension(image_size, 0);
  for (auto i = 0 ; i < SizeOfDimension(box_index, 0) ; i++) {
    TF_LITE_ENSURE(context, (box_index_data[i] >= 0) && (box_index_data[i] < input_batch));
  }

  if (out_box->type == kTfLiteFloat32) {
    EvalFloat<kernel_type>(context, node,
                           box, box_index, delta, image_size, image_scale, delta_weight,
                           data->apply_scale,
                           out_box, out_batch_split);
  } else if (out_box->type == kTfLiteUInt8) {
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
  auto add_scalar_bool = [&nn_model, &augmented_inputs,
                          &next_id](bool value) {
    ANeuralNetworksOperandType operand_type{
        .type = ANEURALNETWORKS_BOOL};
    CHECK_NN(ANeuralNetworksModel_addOperand(nn_model, &operand_type))
    CHECK_NN(ANeuralNetworksModel_setOperandValue(nn_model, next_id, &value,
                                                  sizeof(bool)))
    augmented_inputs.push_back(next_id++);
  };
  auto builtin = reinterpret_cast<OpData*>(data);
  add_scalar_bool(builtin->apply_scale);
  return ::tflite::mtk::Hash("axisalignedbboxtransformmtk");
}

}  // namespace axis_aligned_bbox_transform

TfLiteRegistration* Register_MTK_AXIS_ALIGNED_BBOX_TRANSFORM_REF() {
  static TfLiteRegistration r = {axis_aligned_bbox_transform::Init,
                                 axis_aligned_bbox_transform::Free,
                                 axis_aligned_bbox_transform::Prepare,
                                 axis_aligned_bbox_transform::Eval<axis_aligned_bbox_transform::kReference>};
  ::tflite::mtk::CustomOpHelper::GetInstance().SetMtkExtOpParameterFunc(
      "MTK_AXIS_ALIGNED_BBOX_TRANSFORM", axis_aligned_bbox_transform::add_ann_params);
  return &r;
}

TfLiteRegistration* Register_MTK_AXIS_ALIGNED_BBOX_TRANSFORM_GENERIC_OPT() {
  static TfLiteRegistration r = {axis_aligned_bbox_transform::Init,
                                 axis_aligned_bbox_transform::Free,
                                 axis_aligned_bbox_transform::Prepare,
                                 axis_aligned_bbox_transform::Eval<axis_aligned_bbox_transform::kReference>};
  ::tflite::mtk::CustomOpHelper::GetInstance().SetMtkExtOpParameterFunc(
      "MTK_AXIS_ALIGNED_BBOX_TRANSFORM", axis_aligned_bbox_transform::add_ann_params);
  return &r;
}

TfLiteRegistration* Register_MTK_AXIS_ALIGNED_BBOX_TRANSFORM_NEON_OPT() {
  static TfLiteRegistration r = {axis_aligned_bbox_transform::Init,
                                 axis_aligned_bbox_transform::Free,
                                 axis_aligned_bbox_transform::Prepare,
                                 axis_aligned_bbox_transform::Eval<axis_aligned_bbox_transform::kNeonOptimized>};
  ::tflite::mtk::CustomOpHelper::GetInstance().SetMtkExtOpParameterFunc(
      "MTK_AXIS_ALIGNED_BBOX_TRANSFORM", axis_aligned_bbox_transform::add_ann_params);
  return &r;
}

TfLiteRegistration* Register_MTK_AXIS_ALIGNED_BBOX_TRANSFORM() {
#ifdef USE_NEON
  return Register_MTK_AXIS_ALIGNED_BBOX_TRANSFORM_GENERIC_OPT();
#else
  return Register_MTK_AXIS_ALIGNED_BBOX_TRANSFORM_GENERIC_OPT();
#endif
}

}  // namespace mtk
}  // namespace ops
}  // namespace tflite
