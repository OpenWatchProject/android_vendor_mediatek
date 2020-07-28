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
namespace crop_and_resize {

constexpr int kDataInputTensor = 0;
constexpr int kBoxTensor = 1;
constexpr int kBoxIndexTensor = 2;
constexpr int kOutputSizeTensor = 3;
constexpr int kOutputTensor = 0;

enum KernelType {
  kReference,
};

//for temp tensor, copy from tensorflow/lite/kernels/detection_postprocess.cc
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

TfLiteStatus checkMethod(TfLiteContext* context, string method) {
  TF_LITE_ENSURE(context, strcmp(method.c_str(), "bilinear") == 0);
  return kTfLiteOk;
}

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  OpData* data = new OpData;

  const uint8_t* buffer_t = reinterpret_cast<const uint8_t*>(buffer);
  const flexbuffers::Map& m = flexbuffers::GetRoot(buffer_t, length).AsMap();
  data->extrapolation_value = m["extrapolation_value"].AsDouble();

  // MTK limitation
  checkMethod(context, string(m["method"].AsString().c_str()));

  return data;
}

void Free(TfLiteContext* context, void* buffer) {
  delete reinterpret_cast<OpData*>(buffer);
}

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  OpData* data = reinterpret_cast<OpData*>(node->user_data);

  // check node input/output
  TF_LITE_ENSURE_EQ(context, NumInputs(node), 4);
  TF_LITE_ENSURE_EQ(context, NumOutputs(node), 1);
  const TfLiteTensor* input = GetInput(context, node, kDataInputTensor);
  const TfLiteTensor* box = GetInput(context, node, kBoxTensor);
  const TfLiteTensor* box_index = GetInput(context, node, kBoxIndexTensor);
  const TfLiteTensor* output_size = GetInput(context, node, kOutputSizeTensor);
  TfLiteTensor* output = GetOutput(context, node, kOutputTensor);

  // check input dimension
  TF_LITE_ENSURE_EQ(context, NumDimensions(input), 4);
  TF_LITE_ENSURE_EQ(context, NumDimensions(box), 2);
  TF_LITE_ENSURE_EQ(context, SizeOfDimension(box, 1), 4);
  TF_LITE_ENSURE_EQ(context, NumDimensions(box_index), 1);
  TF_LITE_ENSURE_EQ(context, SizeOfDimension(box_index, 0), SizeOfDimension(box, 0));
  TF_LITE_ENSURE_EQ(context, NumDimensions(output_size), 1);
  TF_LITE_ENSURE_EQ(context, SizeOfDimension(output_size, 0), 2);
  if (NumDimensions(output) != 0) { // if shape already given
    TF_LITE_ENSURE_EQ(context, NumDimensions(output), 4);
    TF_LITE_ENSURE_EQ(context, SizeOfDimension(output, 0), SizeOfDimension(box, 0));
    TF_LITE_ENSURE_EQ(context, SizeOfDimension(output, 3), SizeOfDimension(input, 3));
  }

  // check data type
  const TfLiteType data_type = input->type;
  TF_LITE_ENSURE(context, data_type == kTfLiteFloat32 ||
                          data_type == kTfLiteInt16 ||
                          data_type == kTfLiteUInt8);
  TF_LITE_ENSURE_EQ(context, box->type, data_type);
  TF_LITE_ENSURE_EQ(context, box_index->type, kTfLiteInt32);
  TF_LITE_ENSURE_EQ(context, output_size->type, kTfLiteInt32);
  TF_LITE_ENSURE_EQ(context, output->type, data_type);
  if (data_type == kTfLiteInt16 || data_type == kTfLiteUInt8) {
    TF_LITE_ENSURE_EQ(context, input->params.zero_point, output->params.zero_point);
    TF_LITE_ENSURE_EQ(context, input->params.scale, output->params.scale);
  }

  // for update output tensor size
  TF_LITE_ENSURE(context, IsConstantTensor(output_size));
  const int32* output_size_data = GetTensorData<int32>(output_size);
  TfLiteIntArray* output_size_array = TfLiteIntArrayCreate(4);
  output_size_array->data[0] = SizeOfDimension(box, 0);
  output_size_array->data[1] = output_size_data[0];
  output_size_array->data[2] = output_size_data[1];
  output_size_array->data[3] = SizeOfDimension(input, 3);

  return context->ResizeTensor(context, output, output_size_array);
}

TfLiteStatus Eval(TfLiteContext* context, TfLiteNode* node) {
  OpData* data = reinterpret_cast<OpData*>(node->user_data);

  const TfLiteTensor* input = GetInput(context, node, kDataInputTensor);
  const TfLiteTensor* box = GetInput(context, node, kBoxTensor);
  const TfLiteTensor* box_index = GetInput(context, node, kBoxIndexTensor);
  const TfLiteTensor* output_size = GetInput(context, node, kOutputSizeTensor);
  TfLiteTensor* output = GetOutput(context, node, kOutputTensor);

  // check index range
  const int32* box_index_data = GetTensorData<int32>(box_index);
  const int32 input_batch = SizeOfDimension(input, 0);
  for (auto i = 0 ; i < SizeOfDimension(box_index, 0) ; i++) {
    TF_LITE_ENSURE(context, (box_index_data[i] >= 0) && (box_index_data[i] < input_batch));
  }

  // MTK limitation
  // check box range
  const TfLiteType data_type = input->type;
  const float* box_data = GetTensorData<float>(box);
  for (auto i = 0 ; i < NumElements(box) ; i++) {
    if (data_type == kTfLiteFloat32) {
      const float* box_data = GetTensorData<float>(box);
      TF_LITE_ENSURE(context, (box_data[i] >= 0) && (box_data[i] <= 1));
    } else if (data_type == kTfLiteUInt8) {
      const uint8_t* box_data = GetTensorData<uint8_t>(box);
      float box_data_float = (box_data[i] - box->params.zero_point) * box->params.scale;
      TF_LITE_ENSURE(context, (box_data_float >= 0) && (box_data_float <= 1));
    } else if (data_type == kTfLiteInt16) {
      const int16_t* box_data = GetTensorData<int16_t>(box);
      float box_data_float = (box_data[i] - box->params.zero_point) * box->params.scale;
      TF_LITE_ENSURE(context, (box_data_float >= 0) && (box_data_float <= 1));
    }
  }
  // check box x,y order
  for (auto i = 0 ; i < SizeOfDimension(box, 0) ; i++) {
    if (data_type == kTfLiteFloat32) {
      const float* box_data = GetTensorData<float>(box);
      TF_LITE_ENSURE(context, (box_data[i*4+2] >= box_data[i*4]) && (box_data[i*4+3] >= box_data[i*4+1]));
    } else if (data_type == kTfLiteUInt8) {
      const uint8_t* box_data = GetTensorData<uint8_t>(box);
      float box_data_float_y1 = (float(box_data[i*4]) - box->params.zero_point) * box->params.scale;
      float box_data_float_x1 = (float(box_data[i*4+1]) - box->params.zero_point) * box->params.scale;
      float box_data_float_y2 = (float(box_data[i*4+2]) - box->params.zero_point) * box->params.scale;
      float box_data_float_x2 = (float(box_data[i*4+3]) - box->params.zero_point) * box->params.scale;
      TF_LITE_ENSURE(context, (box_data_float_y2 >= box_data_float_y1) &&
                              (box_data_float_x2 >= box_data_float_x1));
    } else if (data_type == kTfLiteInt16) {
      const int16_t* box_data = GetTensorData<int16_t>(box);
      float box_data_float_y1 = (float(box_data[i*4]) - box->params.zero_point) * box->params.scale;
      float box_data_float_x1 = (float(box_data[i*4+1]) - box->params.zero_point) * box->params.scale;
      float box_data_float_y2 = (float(box_data[i*4+2]) - box->params.zero_point) * box->params.scale;
      float box_data_float_x2 = (float(box_data[i*4+3]) - box->params.zero_point) * box->params.scale;
      TF_LITE_ENSURE(context, (box_data_float_y2 >= box_data_float_y1) &&
                              (box_data_float_x2 >= box_data_float_x1));
    }
  }

  switch (input->type) {
    case kTfLiteFloat32: {
      MtkCropAndResizeParams op_params;
      reference_ops_mtk::CropAndResize(op_params,
          GetTensorShape(input), GetTensorData<float>(input),
          GetTensorShape(box), GetTensorData<float>(box),
          GetTensorShape(box_index), GetTensorData<int32_t>(box_index),
          GetTensorShape(output_size), GetTensorData<int32_t>(output_size),
          GetTensorShape(output), GetTensorData<float>(output));
      return kTfLiteOk;
    }
    break;
    case kTfLiteUInt8: {
      MtkCropAndResizeParams op_params;
      op_params.box_scale = box->params.scale;
      op_params.box_offset = -box->params.zero_point;
      reference_ops_mtk_nbits::CropAndResize<uint8_t>(op_params,
          GetTensorShape(input), GetTensorData<uint8_t>(input),
          GetTensorShape(box), GetTensorData<uint8_t>(box),
          GetTensorShape(box_index), GetTensorData<int32_t>(box_index),
          GetTensorShape(output_size), GetTensorData<int32_t>(output_size),
          GetTensorShape(output), GetTensorData<uint8_t>(output));
      return kTfLiteOk;
    }
    break;
    case kTfLiteInt16: {
      MtkCropAndResizeParams op_params;
      op_params.box_scale = box->params.scale;
      op_params.box_offset = -box->params.zero_point;
      reference_ops_mtk_nbits::CropAndResize<int16_t>(op_params,
          GetTensorShape(input), GetTensorData<uint8_t>(input),
          GetTensorShape(box), GetTensorData<uint8_t>(box),
          GetTensorShape(box_index), GetTensorData<int32_t>(box_index),
          GetTensorShape(output_size), GetTensorData<int32_t>(output_size),
          GetTensorShape(output), GetTensorData<uint8_t>(output));
      return kTfLiteOk;
    }
    break;
    default:
      context->ReportError(context, "Only float32, uint8 and int16 supported currently.");
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
  add_scalar_float32(builtin->extrapolation_value);
  return ::tflite::mtk::Hash("cropandresizemtk");
}

}  // namespace crop_and_resize

TfLiteRegistration* Register_MTK_CROP_AND_RESIZE_REF() {
  static TfLiteRegistration r = {crop_and_resize::Init,
                                 crop_and_resize::Free,
                                 crop_and_resize::Prepare,
                                 crop_and_resize::Eval};
  ::tflite::mtk::CustomOpHelper::GetInstance().SetMtkExtOpParameterFunc(
      "MTK_CROP_AND_RESIZE", crop_and_resize::add_ann_params);
  return &r;
}

TfLiteRegistration* Register_MTK_CROP_AND_RESIZE() {
    return Register_MTK_CROP_AND_RESIZE_REF();
}

}  // namespace mtk
}  // namespace ops
}  // namespace tflite
