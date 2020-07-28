/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly
 * prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
 * ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
 * THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK
 * SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO
 * RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN
 * FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER
 * WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT
 * ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER
 * TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include "tensorflow/lite/mtk/examples/tflite_runner/tflite_runner.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/mtk/examples/tflite_runner/cnpy.h"
#include "tensorflow/lite/mtk/experimental/addon/easyloggingpp/easylogging++.h"
#include "tensorflow/lite/mtk/kernels/mtk_register.h"
#include "tensorflow/lite/optional_debug_tools.h"
#include "tensorflow/lite/mtk/mtk_interpreter.h"

#include <cutils/properties.h>
#include <math.h>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

typedef enum {
  kNone = -1,
  kNpy = 0,
  kNpz,
  kBin,
} DataOption;

DataOption getDataOptionByFilePath(const char* path) {
  DataOption ret = kNone;
  std::string file_path(path);

  std::string extension_name = file_path.substr(file_path.find_last_of("."));

  if (extension_name == ".npy") {
    ret = kNpy;
  } else if (extension_name == ".npz") {
    ret = kNpz;
  } else if (extension_name == ".bin" || extension_name == ".input" ||
             extension_name == ".output") {
    ret = kBin;
  }

  return ret;
}

size_t getSizeOfTFLiteType(TfLiteType type) {
  switch (type) {
    case kTfLiteFloat32:
      return sizeof(float);
    case kTfLiteInt32:
      return sizeof(int32_t);
    case kTfLiteUInt8:
      return sizeof(uint8_t);
    case kTfLiteInt8:
      return sizeof(int8_t);
    case kTfLiteInt64:
      return sizeof(int64_t);
    case kTfLiteBool:
      return sizeof(bool);
    case kTfLiteInt16:
      return sizeof(int16_t);
    case kTfLiteFloat16:
      return sizeof(TfLiteFloat16);
    case kTfLiteComplex64:
    case kTfLiteNoType:
    case kTfLiteString:
    default:
      return -1;
  }
}

class TFLiteRunner {
 public:
  explicit TFLiteRunner(std::unique_ptr<tflite::FlatBufferModel>& model,
                        std::unique_ptr<tflite::Interpreter>& interpreter)
      : model_(std::move(model)), interpreter_(std::move(interpreter)) {}

  TfLiteStatus Run(const std::vector<std::string>& xs_list,
                   const std::vector<std::string>& ys_list);

  void SetLoopCount(int32_t count) { loop_count_ = count; }

  void SetUseNnApi(bool enable) { use_nnapi_ = enable; }

  void SetShowOutput(bool enable) { show_output_ = enable; }

  void SetSaveOutput(bool enable) { save_output_ = enable; }

  void SetAllowThreshold(bool allow) { allow_threshold_ = allow; }

  void SetContinuousInput(bool enable) { continuous_input_ = enable; }

  void SetBreakOnFailure(bool enable) { break_on_failure_ = enable; }

  tflite::Interpreter* GetInterpreter() { return interpreter_.get(); }

 private:
  TfLiteStatus ReshapeInputs(const char* batch_xs);

  TfLiteStatus ReshapeInput(int tensor_id, cnpy::NpyArray array);

  TfLiteStatus ClearOutputs(void);

  TfLiteStatus PrepareInputs(const char* batch_xs);

  TfLiteStatus PrepareInput(int tensor_id, cnpy::NpyArray array);

  TfLiteStatus PrepareInput(int tensor_id, const char* batch_xs);

  TfLiteStatus PrepareInput(int tensor_id, void* input_data, size_t size);

  TfLiteStatus SaveOutputs(const char* batch_ys);

  TfLiteStatus CompareOutputs(const char* batch_ys);

  TfLiteStatus CompareOutput(int tensor_id, cnpy::NpyArray array);

  TfLiteStatus CompareOutput(int tensor_id, const char* batch_ys);

  template <typename T>
  TfLiteStatus CompareOutput(int tensor_id, T* golden_data);

  TfLiteStatus RunWithRandomInput(void);

  template <typename T>
  TfLiteStatus SaveNpz(const char* batch_ys, const char* tensor_name,
                       TfLiteTensor* tensor, bool append);

  template <typename T>
  TfLiteStatus SaveNpy(const char* batch_ys, TfLiteTensor* tensor);

  std::unique_ptr<tflite::FlatBufferModel> model_;

  std::unique_ptr<tflite::Interpreter> interpreter_;

  bool use_nnapi_ = true;

  bool show_output_ = false;

  bool save_output_ = false;

  bool break_on_failure_ = false;

  bool allow_threshold_ = true;

  bool continuous_input_ = false;

  int32_t loop_count_ = 1;

  std::vector<double> inference_time_list_;
};

TfLiteStatus TFLiteRunner::Run(const std::vector<std::string>& xs_list,
                               const std::vector<std::string>& ys_list) {
  if (xs_list.empty() && ys_list.empty()) {
    return RunWithRandomInput();
  }

  std::chrono::high_resolution_clock::time_point start_time_point;
  std::chrono::duration<double> elapsed;

  // Reshape input tensor
  CHECK(kTfLiteOk == ReshapeInputs(xs_list[0].c_str()))
      << "Fail to reshape input tensor";
  // Allocate tensors
  CHECK(kTfLiteOk == interpreter_->AllocateTensors())
      << "Fail to Allocate tensors";
  // Do pre-invike
  CHECK(kTfLiteOk == interpreter_->Invoke()) << "Fail to invoke";

  TfLiteStatus ret = kTfLiteOk;
  for (int32_t i = 0; i < loop_count_; i++) {
    for (size_t j = 0; j < xs_list.size(); j++) {
      LOG(INFO) << "Inference with " << xs_list[j];
      // Clear outputs[0]
      CHECK(kTfLiteOk == ClearOutputs()) << "Fail to clear output tensor";
      // Prepare inputs[0]
      CHECK(kTfLiteOk == PrepareInputs(xs_list[j].c_str()))
          << "Fail to prepare input data";

      start_time_point = std::chrono::high_resolution_clock::now();
      CHECK(kTfLiteOk == interpreter_->Invoke()) << "Fail to invoke";
      elapsed = std::chrono::high_resolution_clock::now() - start_time_point;
      inference_time_list_.push_back(elapsed.count() * 1000);

      // Compare result with golden answer
      if (!save_output_) {
        LOG(INFO) << "Compare the output with " << ys_list[j];
        if (kTfLiteOk != CompareOutputs(ys_list[j].c_str())) {
          // Compare entire list.
          // Raise the error flag once there is an error.
          ret = kTfLiteError;
        }
      }

      if (i == 0 && j == 0) {
        LOG(INFO) << "Model preparation + OP support query + Inference : "
                  << elapsed.count() * 1000 << " ms";
      } else {
        LOG(INFO) << "Inference time : " << elapsed.count() * 1000 << " ms";
      }

      if (save_output_) {
        // Save outputs
        CHECK(kTfLiteOk == SaveOutputs(ys_list[j].c_str()))
            << "Fail to save output";
      }
    }
  }

  return ret;
}

TfLiteStatus TFLiteRunner::RunWithRandomInput(void) {
  std::chrono::high_resolution_clock::time_point start_time_point;
  std::chrono::duration<double> elapsed;

  // Allocate tensors
  CHECK(kTfLiteOk == interpreter_->AllocateTensors())
      << "Fail to Allocate tensors";
  // Clear outputs[0]
  CHECK(kTfLiteOk == ClearOutputs()) << "Fail to clear output tensor";

  for (int32_t i = 0; i < loop_count_; i++) {
    start_time_point = std::chrono::high_resolution_clock::now();
    TF_LITE_ENSURE_STATUS(interpreter_->Invoke());
    elapsed = std::chrono::high_resolution_clock::now() - start_time_point;
    inference_time_list_.push_back(elapsed.count() * 1000);
    if (i == 0) {
      LOG(INFO) << "Model preparation + OP support query + Inference : "
                << elapsed.count() * 1000 << " ms";
    } else {
      LOG(INFO) << "Inference time: " << elapsed.count() * 1000 << " ms";
    }
  }

  if (loop_count_ > 1) {
    double total_inference_time = 0;
    double avg_inference_time = 0;

    // Calculate the average inference time except the first run.
    for (size_t i = 1; i < inference_time_list_.size(); i++) {
      total_inference_time += inference_time_list_.at(i);
    }

    int32_t num = inference_time_list_.size() - 1;
    avg_inference_time = total_inference_time / static_cast<float>(num);
    LOG(INFO) << "Avg Inference time: " << avg_inference_time << " ms";
  } else {
    LOG(INFO) << "Use -c loop_count (e.g. -c 11) to get the "
              << "average inference time";
  }

  return kTfLiteOk;
}

TfLiteStatus TFLiteRunner::ClearOutputs(void) {
  const std::vector<int> outputs = interpreter_->outputs();

  for (size_t i = 0; i < outputs.size(); i++) {
    TfLiteTensor* tensor = interpreter_->tensor(interpreter_->outputs()[i]);
    memset(tensor->data.raw, 0, tensor->bytes);
  }

  return kTfLiteOk;
}

TfLiteStatus TFLiteRunner::ReshapeInput(int tensor_id, cnpy::NpyArray array) {
  const TfLiteTensor* tensor = interpreter_->tensor(tensor_id);
  size_t data_type_size = getSizeOfTFLiteType(tensor->type);
  if (array.word_size != data_type_size) {
    LOG(ERROR) << "Input array word_size " << array.word_size
               << "!= " << data_type_size;
    return kTfLiteError;
  }

  uint8_t* src_data = array.data<uint8_t>();
  if (!src_data) {
    return kTfLiteError;
  }

  std::vector<int> shape;

  for (size_t s : array.shape) {
    shape.push_back(static_cast<int>(s));
  }

  interpreter_->ResizeInputTensor(tensor_id, shape);
  return kTfLiteOk;
}

TfLiteStatus TFLiteRunner::ReshapeInputs(const char* batch_xs) {
  std::ifstream fp(batch_xs, std::ifstream::in | std::ifstream::binary);

  if (!fp.is_open() || !fp.good()) {
    LOG(ERROR) << "Could not read " << batch_xs;
    return kTfLiteError;
  }

  TfLiteStatus result = kTfLiteOk;
  DataOption data_option = getDataOptionByFilePath(batch_xs);

  if (data_option == kNpz) {
    // check inputs size
    const std::vector<int> inputs = interpreter_->inputs();
    cnpy::npz_t arrs = cnpy::npz_load(batch_xs);

    if (arrs.size() != inputs.size()) {
      LOG(INFO) << "Input npz arrays size " << arrs.size()
                << "!= network inputs size" << inputs.size();
      return kTfLiteError;
    }

    for (size_t i = 0; i < inputs.size(); i++) {
      int tensor_id = inputs[i];
      const char* tensor_name = interpreter_->GetInputName(i);

      if (arrs.find(tensor_name) == arrs.end()) {
        LOG(ERROR) << "Could not find input array name " << tensor_name
                   << "in npz arrays";
        return kTfLiteError;
      }

      result = ReshapeInput(tensor_id, arrs[tensor_name]);

      if (result != kTfLiteOk) {
        return result;
      }
    }

    return result;
  } else if (data_option == kNpy) {  // use npy, so default inputs.size() == 1
    cnpy::NpyArray arr = cnpy::npy_load(batch_xs);
    int tensor_id = interpreter_->inputs()[0];
    return ReshapeInput(tensor_id, arr);
  }

  return result;
}

TfLiteStatus TFLiteRunner::PrepareInput(int tensor_id, cnpy::NpyArray array) {
  return PrepareInput(tensor_id, reinterpret_cast<void*>(array.data<uint8_t>()),
                      array.num_bytes());
}

TfLiteStatus TFLiteRunner::PrepareInput(int tensor_id, const char* batch_xs) {
  std::ifstream fp(batch_xs, std::ifstream::in | std::ifstream::binary);

  if (!fp.is_open() || !fp.good()) {
    LOG(ERROR) << "Could not read " << batch_xs;
    return kTfLiteError;
  }

  TfLiteTensor* tensor = interpreter_->tensor(tensor_id);

  fp.seekg(0, fp.end);
  size_t batch_xs_len = fp.tellg();

  if (batch_xs_len != tensor->bytes) {
    LOG(ERROR) << "The size in bytes of " << batch_xs
               << "is not matched to input tensor";
    return kTfLiteError;
  }

  fp.seekg(0, fp.beg);
  TfLiteStatus ret = kTfLiteError;
  char* loaded_data = new char[batch_xs_len];

  fp.read(loaded_data, batch_xs_len);
  ret = PrepareInput(tensor_id, reinterpret_cast<void*>(loaded_data),
                     batch_xs_len);

  delete[] loaded_data;
  return ret;
}

TfLiteStatus TFLiteRunner::PrepareInput(int tensor_id, void* input_data,
                                        size_t size) {
  TfLiteTensor* tensor = interpreter_->tensor(tensor_id);

  if (!tensor->data.raw || !input_data) {
    LOG(ERROR) << "Fail to prepare input";
    return kTfLiteError;
  }

  if (size != tensor->bytes) {
    LOG(ERROR) << "The size in bytes of input data"
               << "is not matched to input tensor";
    return kTfLiteError;
  }

  int num = 1;
  LOG(INFO) << "Input tensor";
  std::string dimension("    Dimension[ ");

  for (int i = 0; i < tensor->dims->size; i++) {
    dimension.append(std::to_string(tensor->dims->data[i]));
    dimension.append(std::string(" "));
    num *= tensor->dims->data[i];
  }

  dimension.append("]");
  LOG(INFO) << dimension;
  LOG(INFO) << "    Size in bytes: " << tensor->bytes;
  LOG(INFO) << "    Num of elemnts: " << num;

  memcpy(tensor->data.raw, reinterpret_cast<const void*>(input_data), size);
  memcpy(tensor->data.raw, reinterpret_cast<const void*>(input_data), size);
  return kTfLiteOk;
}

TfLiteStatus TFLiteRunner::PrepareInputs(const char* batch_xs) {
  std::ifstream fp(batch_xs, std::ifstream::in | std::ifstream::binary);

  if (!fp.is_open() || !fp.good()) {
    LOG(ERROR) << "Could not read " << batch_xs;
    return kTfLiteError;
  }

  TfLiteStatus result = kTfLiteError;
  DataOption data_option = getDataOptionByFilePath(batch_xs);

  if (data_option == kNpz) {
    // check inputs size
    const std::vector<int> inputs = interpreter_->inputs();
    cnpy::npz_t arrs = cnpy::npz_load(batch_xs);

    if (arrs.size() != inputs.size()) {
      LOG(ERROR) << "Input npz arrays size " << arrs.size() << "!= "
                 << " network inputs size" << inputs.size();
      return kTfLiteError;
    }

    for (size_t i = 0; i < inputs.size(); i++) {
      int tensor_id = inputs[i];
      const char* tensor_name = interpreter_->GetInputName(i);

      if (arrs.find(tensor_name) == arrs.end()) {
        LOG(ERROR) << "Could not find input array name " << tensor_name
                   << " in npz arrays";
        return kTfLiteError;
      }

      result = PrepareInput(tensor_id, arrs[tensor_name]);

      if (result != kTfLiteOk) {
        return result;
      }
    }

    return result;
  } else if (data_option == kNpy) {
    cnpy::NpyArray arr = cnpy::npy_load(batch_xs);
    int tensor_id = interpreter_->inputs()[0];
    return PrepareInput(tensor_id, arr);
  } else if (data_option == kBin) {
    int tensor_id = interpreter_->inputs()[0];
    return PrepareInput(tensor_id, batch_xs);
  }

  return result;
}

TfLiteStatus TFLiteRunner::SaveOutputs(const char* batch_ys) {
  TfLiteStatus result = kTfLiteError;
  DataOption data_option = getDataOptionByFilePath(batch_ys);

  if (data_option == kNpz) {
    const std::vector<int> outputs = interpreter_->outputs();
    bool append = false;

    for (size_t o = 0; o < outputs.size(); o++) {
      int tensor_id = outputs[o];
      const char* tensor_name = interpreter_->GetOutputName(o);
      LOG(INFO) << "Saving output tensor [" << tensor_id
                << "]: " << tensor_name;

      TfLiteTensor* tensor = interpreter_->tensor(tensor_id);
      switch (tensor->type) {
        case kTfLiteFloat32:
          result = SaveNpz<float>(batch_ys, tensor_name, tensor, append);
          break;
        case kTfLiteInt32:
          result = SaveNpz<int32_t>(batch_ys, tensor_name, tensor, append);
          break;
        case kTfLiteUInt8:
          result = SaveNpz<uint8_t>(batch_ys, tensor_name, tensor, append);
          break;
        case kTfLiteInt8:
          result = SaveNpz<int8_t>(batch_ys, tensor_name, tensor, append);
          break;
        case kTfLiteInt64:
          result = SaveNpz<int64_t>(batch_ys, tensor_name, tensor, append);
          break;
        case kTfLiteInt16:
          result = SaveNpz<int16_t>(batch_ys, tensor_name, tensor, append);
          break;
        default:
          break;
      }
      append = true;
      result = kTfLiteOk;
    }
  } else if (data_option == kNpy) {
    TfLiteTensor* tensor = interpreter_->tensor(interpreter_->outputs()[0]);
    switch (tensor->type) {
      case kTfLiteFloat32:
        result = SaveNpy<float>(batch_ys, tensor);
        break;
      case kTfLiteInt32:
        result = SaveNpy<int32_t>(batch_ys, tensor);
        break;
      case kTfLiteUInt8:
        result = SaveNpy<uint8_t>(batch_ys, tensor);
        break;
      case kTfLiteInt8:
        result = SaveNpy<int8_t>(batch_ys, tensor);
        break;
      case kTfLiteInt64:
        result = SaveNpy<int64_t>(batch_ys, tensor);
        break;
      case kTfLiteInt16:
        result = SaveNpy<int16_t>(batch_ys, tensor);
        break;
      default:
        break;
    }
  } else if (data_option == kBin) {
    TfLiteTensor* tensor = interpreter_->tensor(interpreter_->outputs()[0]);
    FILE* fp = fopen(batch_ys, "wb");

    if (fp != nullptr) {
      fwrite(tensor->data.raw, sizeof(char), tensor->bytes, fp);
      fclose(fp);
      result = kTfLiteOk;
    }
  }

  if (result == kTfLiteOk) {
    LOG(INFO) << "Saved Output to: " << batch_ys;
  } else {
    LOG(ERROR) << "Fail to save Output to: " << batch_ys;
  }
  return result;
}

TfLiteStatus TFLiteRunner::CompareOutput(int tensor_id, cnpy::NpyArray array) {
  TfLiteStatus result = kTfLiteError;
  TfLiteTensor* tensor = interpreter_->tensor(tensor_id);
  switch (tensor->type) {
    case kTfLiteFloat32:
      result = CompareOutput<float>(
          tensor_id, reinterpret_cast<float*>(array.data<float>()));
      break;
    case kTfLiteInt32:
      result = CompareOutput<int32_t>(
          tensor_id, reinterpret_cast<int32_t*>(array.data<int32_t>()));
      break;
    case kTfLiteUInt8:
      result = CompareOutput<uint8_t>(
          tensor_id, reinterpret_cast<uint8_t*>(array.data<uint8_t>()));
      break;
    case kTfLiteInt8:
      result = CompareOutput<int8_t>(
          tensor_id, reinterpret_cast<int8_t*>(array.data<int8_t>()));
      break;
    case kTfLiteInt64:
      result = CompareOutput<int64_t>(
          tensor_id, reinterpret_cast<int64_t*>(array.data<int64_t>()));
      break;
    case kTfLiteInt16:
      result = CompareOutput<int16_t>(
          tensor_id, reinterpret_cast<int16_t*>(array.data<int16_t>()));
      break;
    default:
      break;
  }
  return result;
}

TfLiteStatus TFLiteRunner::CompareOutput(int tensor_id, const char* batch_ys) {
  std::ifstream fp(batch_ys, std::ifstream::in | std::ifstream::binary);

  if (!fp.is_open() || !fp.good()) {
    LOG(ERROR) << "Could not read " << batch_ys;
    return kTfLiteError;
  }

  TfLiteTensor* tensor = interpreter_->tensor(tensor_id);

  fp.seekg(0, fp.end);
  size_t ys_len = fp.tellg();

  if (ys_len != tensor->bytes) {
    LOG(ERROR) << "The size in bytes of " << batch_ys
               << " is not matched to output tensor";
    return kTfLiteError;
  }

  fp.seekg(0, fp.beg);
  TfLiteStatus result = kTfLiteError;
  char* loaded_data = new char[ys_len];

  fp.read(loaded_data, ys_len);
  switch (tensor->type) {
    case kTfLiteFloat32:
      result = CompareOutput<float>(tensor_id,
                                    reinterpret_cast<float*>(loaded_data));
      break;
    case kTfLiteInt32:
      result = CompareOutput<int32_t>(tensor_id,
                                      reinterpret_cast<int32_t*>(loaded_data));
      break;
    case kTfLiteUInt8:
      result = CompareOutput<uint8_t>(tensor_id,
                                      reinterpret_cast<uint8_t*>(loaded_data));
      break;
    case kTfLiteInt8:
      result = CompareOutput<int8_t>(tensor_id,
                                     reinterpret_cast<int8_t*>(loaded_data));
      break;
    case kTfLiteInt64:
      result = CompareOutput<int64_t>(tensor_id,
                                     reinterpret_cast<int64_t*>(loaded_data));
      break;
    case kTfLiteInt16:
      result = CompareOutput<int16_t>(tensor_id,
                                      reinterpret_cast<int16_t*>(loaded_data));
      break;
    default:
      break;
  }

  delete[] loaded_data;
  return result;
}

template <typename T>
TfLiteStatus TFLiteRunner::CompareOutput(int tensor_id, T* golden_data) {
  const double kRelativeThreshold = 1e-2f;
  const double kAbsoluteThreshold = 1e-4f;
  constexpr int kUint8AbsoluteThreshold = 2;
  TfLiteTensor* tensor = interpreter_->tensor(tensor_id);
  T* src_data = interpreter_->typed_tensor<T>(tensor_id);
  T* dst_data = golden_data;

  if (!dst_data || !src_data) {
    return kTfLiteError;
  }

  size_t num = tensor->bytes / sizeof(T);
  LOG(INFO) << "Output tensor";
  std::string dimension("    Dimension[ ");

  for (int i = 0; i < tensor->dims->size; i++) {
    dimension.append(std::to_string(tensor->dims->data[i]));
    dimension.append(std::string(" "));
  }

  dimension.append("]");
  LOG(INFO) << dimension;
  LOG(INFO) << "    Size in bytes: " << tensor->bytes;
  LOG(INFO) << "    Num of elemnts: " << num;

  if (show_output_) {
    LOG(INFO) << "Result output data";

    if (std::is_same<T, float>::value) {
      for (int h = 0; h < 10; h++) {
        LOG(INFO) << "    " << src_data[h * 10 + 0] << " "
                  << src_data[h * 10 + 1] << " " << src_data[h * 10 + 2] << " "
                  << src_data[h * 10 + 3] << " " << src_data[h * 10 + 4] << " "
                  << src_data[h * 10 + 5] << " " << src_data[h * 10 + 6] << " "
                  << src_data[h * 10 + 7] << " " << src_data[h * 10 + 8] << " "
                  << src_data[h * 10 + 9];
      }

      LOG(INFO) << "Golden output data";

      for (int h = 0; h < 10; h++) {
        LOG(INFO) << "    " << dst_data[h * 10 + 0] << " "
                  << dst_data[h * 10 + 1] << " " << dst_data[h * 10 + 2] << " "
                  << dst_data[h * 10 + 3] << " " << dst_data[h * 10 + 4] << " "
                  << dst_data[h * 10 + 5] << " " << dst_data[h * 10 + 6] << " "
                  << dst_data[h * 10 + 7] << " " << dst_data[h * 10 + 8] << " "
                  << dst_data[h * 10 + 9];
      }
    } else if (std::is_same<T, uint8_t>::value) {
      for (int h = 0; h < 10; h++) {
        LOG(INFO) << "    " << static_cast<int>(src_data[h * 10 + 0]) << " "
                  << static_cast<int>(src_data[h * 10 + 1]) << " "
                  << static_cast<int>(src_data[h * 10 + 2]) << " "
                  << static_cast<int>(src_data[h * 10 + 3]) << " "
                  << static_cast<int>(src_data[h * 10 + 4]) << " "
                  << static_cast<int>(src_data[h * 10 + 5]) << " "
                  << static_cast<int>(src_data[h * 10 + 6]) << " "
                  << static_cast<int>(src_data[h * 10 + 7]) << " "
                  << static_cast<int>(src_data[h * 10 + 8]) << " "
                  << static_cast<int>(src_data[h * 10 + 9]);
      }

      LOG(INFO) << "Golden output data";

      for (int h = 0; h < 10; h++) {
        LOG(INFO) << "    " << static_cast<int>(dst_data[h * 10 + 0]) << " "
                  << static_cast<int>(dst_data[h * 10 + 1]) << " "
                  << static_cast<int>(dst_data[h * 10 + 2]) << " "
                  << static_cast<int>(dst_data[h * 10 + 3]) << " "
                  << static_cast<int>(dst_data[h * 10 + 4]) << " "
                  << static_cast<int>(dst_data[h * 10 + 5]) << " "
                  << static_cast<int>(dst_data[h * 10 + 6]) << " "
                  << static_cast<int>(dst_data[h * 10 + 7]) << " "
                  << static_cast<int>(dst_data[h * 10 + 8]) << " "
                  << static_cast<int>(dst_data[h * 10 + 9]);
      }
    }
  }

  if (std::is_same<T, float>::value) {
    for (size_t i = 0; i < num; i++) {
      float diff = std::abs(src_data[i] - dst_data[i]);
      bool error_is_large = false;

      // For very small numbers, try absolute error, otherwise go with
      // relative.
      float abs_value = std::abs(static_cast<float>(dst_data[i]));
      if (abs_value < kRelativeThreshold) {
        error_is_large = (diff > kAbsoluteThreshold);
      } else {
        error_is_large = (diff > kRelativeThreshold * abs_value);
      }

      if ((!allow_threshold_ && diff != 0) || error_is_large) {
        LOG(ERROR) << "Result " << i << "= " << src_data[i];
        LOG(ERROR) << "Golden " << i << "= " << dst_data[i];
        LOG(ERROR) << "Result is not matched to golden value";
        return kTfLiteError;
      }
    }
  } else if (std::is_same<T, uint8_t>::value) {
    for (size_t i = 0; i < num; i++) {
      int diff = std::abs(static_cast<int>(src_data[i]) -
                          static_cast<int>(dst_data[i]));

      if ((!allow_threshold_ && diff != 0) || diff >= kUint8AbsoluteThreshold) {
        LOG(ERROR) << "Result " << i << "= " << static_cast<int>(src_data[i]);
        LOG(ERROR) << "Golden " << i << "= " << static_cast<int>(dst_data[i]);
        LOG(ERROR) << "Result is not matched to golden value";
        return kTfLiteError;
      }
    }
  }

  LOG(INFO) << "Result is matched!!";
  return kTfLiteOk;
}

TfLiteStatus TFLiteRunner::CompareOutputs(const char* batch_ys) {
  TfLiteStatus result = kTfLiteError;
  DataOption data_option = getDataOptionByFilePath(batch_ys);

  if (data_option == kNpz) {
    // check output size
    const std::vector<int> outputs = interpreter_->outputs();
    cnpy::npz_t arrs = cnpy::npz_load(batch_ys);

    if (arrs.size() != outputs.size()) {
      LOG(ERROR) << "Output npz arrays size " << arrs.size()
                 << "!= network outputs size" << outputs.size();
      return kTfLiteError;
    }

    for (size_t i = 0; i < outputs.size(); i++) {
      int tensor_id = outputs[i];
      const char* tensor_name = interpreter_->GetOutputName(i);

      if (arrs.find(tensor_name) == arrs.end()) {
        LOG(ERROR) << "Could not find output array name " << tensor_name
                   << "in npz arrays";
        return kTfLiteError;
      }

      LOG(INFO) << "Compare tensor: " << tensor_name;
      result = CompareOutput(tensor_id, arrs[tensor_name]);

      if (result != kTfLiteOk) {
        return result;
      }
    }

    return result;
  } else if (data_option == kNpy) {
    cnpy::NpyArray arr = cnpy::npy_load(batch_ys);
    int tensor_id = interpreter_->outputs()[0];
    return CompareOutput(tensor_id, arr);
  } else if (data_option == kBin) {
    int tensor_id = interpreter_->outputs()[0];
    return CompareOutput(tensor_id, batch_ys);
  }

  return result;
}

template <typename T>
TfLiteStatus TFLiteRunner::SaveNpy(const char* batch_ys, TfLiteTensor* tensor) {
  if (!tensor->data.raw) {
    return kTfLiteError;
  }

  T* out_data = reinterpret_cast<T*>(tensor->data.raw);

  // get output shape
  std::vector<size_t> npyshape;

  for (int i = 0; i < tensor->dims->size; i++) {
    npyshape.push_back(tensor->dims->data[i]);
  }

  std::vector<T> npydata;
  size_t num = tensor->bytes / sizeof(T);

  for (size_t idx = 0; idx < num; idx++) {
    npydata.push_back(out_data[idx]);
  }

  cnpy::npy_save(batch_ys, &npydata[0], npyshape, "w");

  return kTfLiteOk;
}

template <typename T>
TfLiteStatus TFLiteRunner::SaveNpz(const char* batch_ys,
                                   const char* tensor_name,
                                   TfLiteTensor* tensor, bool append) {
  if (!tensor->data.raw) {
    return kTfLiteError;
  }

  T* out_data = reinterpret_cast<T*>(tensor->data.raw);

  // get shape
  std::vector<size_t> npyshape;

  for (int i = 0; i < tensor->dims->size; i++) {
    npyshape.push_back(tensor->dims->data[i]);
  }

  // get data
  std::vector<T> npydata;
  size_t num = tensor->bytes / sizeof(T);

  for (size_t idx = 0; idx < num; idx++) {
    npydata.push_back(out_data[idx]);
  }

  cnpy::npz_save(batch_ys, tensor_name, &npydata[0], npyshape,
                 (append) ? "a" : "w");
  return kTfLiteOk;
}

int TFLiteRunner_Create(TFLiteRunnerHandle** handle,
                        const std::string& model_path) {
  std::unique_ptr<tflite::FlatBufferModel> model;
  std::unique_ptr<tflite::Interpreter> interpreter;
  tflite::ops::builtin::MtkBuiltinOpResolver resolver;

  model = tflite::FlatBufferModel::BuildFromFile(model_path.c_str());
  tflite::InterpreterBuilder(*model, resolver)(&(interpreter));

  for (size_t i = 0; i < interpreter->inputs().size(); i++) {
    TfLiteType tensor_type =
        interpreter->tensor(interpreter->inputs()[i])->type;
    LOG(INFO) << "Input tensor[" << i
              << "] data type: " << tflite::TensorTypeName(tensor_type);
  }

  for (size_t i = 0; i < interpreter->outputs().size(); i++) {
    TfLiteType tensor_type =
        interpreter->tensor(interpreter->outputs()[i])->type;
    LOG(INFO) << "Output tensor[" << i
              << "] data type: " << tflite::TensorTypeName(tensor_type);
  }

  TFLiteRunner* r = new TFLiteRunner(model, interpreter);
  *handle = reinterpret_cast<TFLiteRunnerHandle*>(r);

  return 0;
}

int TFLiteRunner_Invoke(TFLiteRunnerHandle* handle,
                        const std::vector<std::string>& xs_list,
                        const std::vector<std::string>& ys_list) {
  TFLiteRunner* r = reinterpret_cast<TFLiteRunner*>(handle);
  CHECK_NOTNULL(r);
  return (r->Run(xs_list, ys_list) == kTfLiteOk ? 0 : 1);
}

void TFLiteRunner_Free(TFLiteRunnerHandle* handle) {
  CHECK_NOTNULL(handle);

  TFLiteRunner* r = reinterpret_cast<TFLiteRunner*>(handle);
  CHECK_NOTNULL(r);
  delete r;
}

void TFLiteRunner_SetUseNnApi(TFLiteRunnerHandle* handle, bool enable) {
  TFLiteRunner* r = reinterpret_cast<TFLiteRunner*>(handle);
  CHECK_NOTNULL(r);
  CHECK_NOTNULL(r->GetInterpreter());
  r->GetInterpreter()->UseNNAPI(enable);
}

void TFLiteRunner_SetCacheDir(TFLiteRunnerHandle* handle, const char* cache_dir) {
  TFLiteRunner* r = reinterpret_cast<TFLiteRunner*>(handle);
  CHECK_NOTNULL(r);
  CHECK_NOTNULL(r->GetInterpreter());
  reinterpret_cast<tflite::MtkInterpreter*>(r->GetInterpreter())->SetCacheDir(cache_dir);
}

void TFLiteRunner_SetEncryptionLevel(TFLiteRunnerHandle* handle, int encryption_level) {
  TFLiteRunner* r = reinterpret_cast<TFLiteRunner*>(handle);
  CHECK_NOTNULL(r);
  CHECK_NOTNULL(r->GetInterpreter());
  reinterpret_cast<tflite::MtkInterpreter*>(r->GetInterpreter())->
    SetEncryptionLevel(encryption_level);
}

void TFLiteRunner_SetAllowThreshold(TFLiteRunnerHandle* handle, bool allow) {
  TFLiteRunner* r = reinterpret_cast<TFLiteRunner*>(handle);
  CHECK_NOTNULL(r);
  r->SetAllowThreshold(allow);
}

void TFLiteRunner_SetAllowFp16Precision(TFLiteRunnerHandle* handle,
                                        bool allow) {
  TFLiteRunner* r = reinterpret_cast<TFLiteRunner*>(handle);
  CHECK_NOTNULL(r);
  r->GetInterpreter()->SetAllowFp16PrecisionForFp32(allow);
}

void TFLiteRunner_SetLoopCount(TFLiteRunnerHandle* handle, int32_t count) {
  TFLiteRunner* r = reinterpret_cast<TFLiteRunner*>(handle);
  CHECK_NOTNULL(r);
  r->SetLoopCount(count);
}

void TFLiteRunner_PrintState(TFLiteRunnerHandle* handle) {
  TFLiteRunner* r = reinterpret_cast<TFLiteRunner*>(handle);
  CHECK_NOTNULL(r);
  PrintInterpreterState(r->GetInterpreter());
}

void TFLiteRunner_SetInterpreterNumThreads(TFLiteRunnerHandle* handle,
                                           int32_t num) {
  TFLiteRunner* r = reinterpret_cast<TFLiteRunner*>(handle);
  CHECK_NOTNULL(r);
  r->GetInterpreter()->SetNumThreads(num);
}

void TFLiteRunner_SetShowOutput(TFLiteRunnerHandle* handle, bool allow) {
  TFLiteRunner* r = reinterpret_cast<TFLiteRunner*>(handle);
  CHECK_NOTNULL(r);
  r->SetShowOutput(allow);
}

void TFLiteRunner_SetSaveOutput(TFLiteRunnerHandle* handle, bool allow) {
  TFLiteRunner* r = reinterpret_cast<TFLiteRunner*>(handle);
  CHECK_NOTNULL(r);
  r->SetSaveOutput(allow);
}
