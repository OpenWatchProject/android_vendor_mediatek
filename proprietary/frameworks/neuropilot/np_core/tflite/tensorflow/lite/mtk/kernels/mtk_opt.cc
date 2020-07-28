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

#include "tensorflow/lite/context.h"
#include "tensorflow/lite/kernels/internal/quantization_util.h"
#include "tensorflow/lite/kernels/internal/tensor.h"
#include "tensorflow/lite/kernels/kernel_util.h"

namespace tflite {
namespace ops {
namespace mtk {
namespace opt {

TfLiteStatus OptPrepare(TfLiteContext* context, TfLiteNode* node) {
  TF_LITE_ENSURE_EQ(context, NumInputs(node), 1);
  TF_LITE_ENSURE_EQ(context, NumOutputs(node), 1);
  const TfLiteTensor* input = GetInput(context, node, 0);
  TfLiteTensor* output = GetOutput(context, node, 0);
  TF_LITE_ENSURE_EQ(context, input->type, output->type);

  return context->ResizeTensor(context, output,
                               TfLiteIntArrayCopy(input->dims));
}


TfLiteStatus OptEval(TfLiteContext* context, TfLiteNode* node) {
  const TfLiteTensor* input = GetInput(context, node, 0);
  TfLiteTensor* output = GetOutput(context, node, 0);

  switch (input->type) {
    case kTfLiteFloat32: {
      size_t elements = input->bytes / sizeof(float);
      float* in = input->data.f;
      float* in_end = in + elements;
      float* out = output->data.f;
      for (; in < in_end; in++, out++) *out = *in;
      return kTfLiteOk;
    } break;
    case kTfLiteUInt8: {
      TF_LITE_ENSURE_EQ(context, input->params.scale, output->params.scale);
//      TF_LITE_ENSURE_EQ(context, 0, output->params.zero_point);
      size_t elements = input->bytes / sizeof(uint8_t);
      uint8_t* in = input->data.uint8;
      uint8_t* in_end = in + elements;
      uint8_t* out = output->data.uint8;
      for (; in < in_end; in++, out++){
        *out = (uint8_t)(*in);
      }
      return kTfLiteOk;
    }
    break;
    case kTfLiteInt32: {
      TF_LITE_ENSURE_EQ(context, input->params.scale, output->params.scale);
      size_t elements = input->bytes / sizeof(int32_t);
      int32_t* in = input->data.i32;
      int32_t* in_end = in + elements;
      int32_t* out = output->data.i32;
      for (; in < in_end; in++, out++){
        *out = (int32_t)(*in);
      }
      return kTfLiteOk;
    }
    break;
    default:
      context->ReportError(context, "Only float32, uint8 & int32 supported currently.");
      return kTfLiteError;
  }

  return kTfLiteOk;
}

}  // namespace opt

TfLiteRegistration* Register_MTK_OPT_REF() {
  static TfLiteRegistration r = {/*init=*/nullptr, /*free=*/nullptr,
                                 opt::OptPrepare,
                                 opt::OptEval};
  r.custom_name = strdup("MTK_OPT");
  return &r;
}

TfLiteRegistration* Register_MTK_OPT() {
    return Register_MTK_OPT_REF();
}

}  // namespace mtk
}  // namespace ops
}  // namespace tflite

