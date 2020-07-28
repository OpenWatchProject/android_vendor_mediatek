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

#define LOG_TAG "NeuroPilotTFLite"

#include "tensorflow/lite/mtk/experimental/neuropilot_c_api/neuropilot_c_api.h"
#include "flatbuffers/flexbuffers.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/mtk/experimental/addon/easyloggingpp/easylogging++.h"
#include "tensorflow/lite/mtk/experimental/neuropilot_c_api/neuropilot_trace.h"
#include "tensorflow/lite/mtk/kernels/mtk_register.h"
#include "tensorflow/lite/mtk/mtk_minimal_logging.h"
#include "tensorflow/lite/mtk/mtk_utils.h"
#include "tensorflow/lite/mtk/mtk_interpreter.h"

#include <algorithm>
#include <mutex>
#include <utility>
#include <vector>

#ifndef UNUSED
#define UNUSED(expr) \
  do {               \
    (void)(expr);    \
  } while (0)
#endif

#define CHECK_UNEXPECTED_NULL(expr)                                \
  do {                                                             \
    if (expr == nullptr) {                                         \
      LOG(ERROR) << "Check failed: [" << #expr << " != nullptr] "; \
      return ANEURALNETWORKS_UNEXPECTED_NULL;                      \
    }                                                              \
  } while (0)

#define ELPP_THREAD_SAFE
INITIALIZE_EASYLOGGINGPP

static std::mutex tfliteBufferMutex;

typedef struct {
  bool allowFp16PrecisionForFp32 = true;
  bool execParallel = true;
  bool useNNAPI = true;
  std::unordered_map<int32_t, std::vector<int>> inputDimensions;
  const char* cache_dir = nullptr;
  int encryption_level = 0;
} NpTFLiteOptions;

typedef struct {
  void* tflite;           // The TFLite instance which holds the tensor
  TFLiteBufferType type;  // Input or output tensor
  uint32_t index;         // Zero-based index
} NpTFLiteTensor;

class LogHandler : public el::LogDispatchCallback {
 public:
  void handle(const el::LogDispatchData* data) {
#ifdef __ANDROID__
    switch (data->logMessage()->level()) {
      case el::Level::Error:
        TFLITE_MTK_LOG_ERROR("%s", data->logMessage()->message().c_str());
        break;
      case el::Level::Warning:
        TFLITE_MTK_LOG_WARN("%s", data->logMessage()->message().c_str());
        break;
      default:
        TFLITE_MTK_LOG_INFO("%s", data->logMessage()->message().c_str());
        break;
    }
#else
    ELPP_COUT << LOG_TAG << ": " << data << std::endl;
#endif
  }
};

class InitLog {
    public:
        InitLog(){
            el::Loggers::removeFlag(el::LoggingFlag::ColoredTerminalOutput);
            el::Configurations defaultConf;
            defaultConf.setGlobally(el::ConfigurationType::Format, "%msg");
            el::Loggers::reconfigureLogger("default", defaultConf);

            LogHandler* logHandler =
                el::Helpers::logDispatchCallback<LogHandler>("NpCApiLogHandler");
            if (!logHandler) {
              el::Helpers::installLogDispatchCallback<LogHandler>("NpCApiLogHandler");
              el::Loggers::addFlag(el::LoggingFlag::DisableApplicationAbortOnFatalLog);
            }
        }
};

static InitLog doInitLog;

class NpTFLite {
 public:
  explicit NpTFLite(const char* modelPath, NpTFLiteOptions* options) {
    InitOptions(options);
    model_ = tflite::FlatBufferModel::BuildFromFile(modelPath);
    tflite::InterpreterBuilder(*model_, resolver_)(&interpreter_);
  }

  NpTFLite(const char* buffer, size_t bufferSize, NpTFLiteOptions* options) {
    InitOptions(options);
    const char* ptr =
        CloneModelBuffer(buffer, bufferSize) ? modelBuffer_ : buffer;
    model_ = tflite::FlatBufferModel::BuildFromBuffer(ptr, bufferSize);
    tflite::InterpreterBuilder(*model_, resolver_)(&interpreter_);
  }

  TfLiteStatus BuildGraph() {
    LOG(DEBUG) << "BuildGraph, allowFp16PrecisionForFp32: "
               << options_.allowFp16PrecisionForFp32
               << " execParallel: " << options_.execParallel
               << " useNNAPI: " << options_.useNNAPI;

    // Resize input tensor. Must be called before UseNNAPI()
    for (auto& x : options_.inputDimensions) {
      LOG(DEBUG) << "Resize input " << x.first;
      for (auto y = x.second.begin(); y != x.second.end(); ++y) {
        LOG(DEBUG) << *y << " ";
      }

      interpreter_->ResizeInputTensor(interpreter_->inputs()[x.first],
                                      x.second);
    }

    // SetAllowFp16PrecisionForFp32() must be called before UseNNAPI()
    interpreter_->SetAllowFp16PrecisionForFp32(
        options_.allowFp16PrecisionForFp32);
    interpreter_->UseNNAPI(options_.useNNAPI);
    reinterpret_cast<tflite::MtkInterpreter*>(interpreter_.get())->SetCacheDir(
        options_.cache_dir);
    reinterpret_cast<tflite::MtkInterpreter*>(interpreter_.get())->SetEncryptionLevel(
        options_.encryption_level);
    TF_LITE_ENSURE_STATUS(interpreter_->AllocateTensors());
    TF_LITE_ENSURE_STATUS(interpreter_->Invoke());
    return kTfLiteOk;
  }

  TfLiteStatus RebuildInterpreter() {
    interpreter_.reset();
    tflite::InterpreterBuilder(*model_, resolver_)(&interpreter_);
    return BuildGraph();
  }

  std::vector<float> GetDequantizedOutput(int outputTensorIndex) {
    // Transfer the output tensor index to actual tensor index
    int index = interpreter_->outputs()[outputTensorIndex];
    return Dequantize<uint8_t>(ExtractVector<uint8_t>(index),
                               GetTensorScale(index),
                               GetTensorZeroPoint(index));
  }

  ~NpTFLite() {
    if (modelBuffer_ != nullptr) {
      free(modelBuffer_);
      modelBuffer_ = nullptr;
    }
  }

  std::unique_ptr<tflite::Interpreter> interpreter_;
  std::unique_ptr<tflite::FlatBufferModel> model_;
  tflite::ops::builtin::MtkBuiltinOpResolver resolver_;
  NpTFLiteOptions options_;

 private:
  bool CloneModelBuffer(const char* buffer, size_t bufferSize) {
    modelBuffer_ = reinterpret_cast<char*>(malloc(bufferSize));
    if (modelBuffer_ == nullptr) {
      return false;
    }
    memcpy(modelBuffer_, buffer, bufferSize);
    return true;
  }

  void InitOptions(NpTFLiteOptions* options) {
    if (options != nullptr) {
      options_.allowFp16PrecisionForFp32 = options->allowFp16PrecisionForFp32;
      options_.execParallel = options->execParallel;
      options_.useNNAPI = options->useNNAPI;
      options_.inputDimensions = options->inputDimensions;
      options_.cache_dir = options->cache_dir;
      options_.encryption_level = options->encryption_level;
    }
  }

  size_t GetTensorSize(int index) const {
    TfLiteTensor* t = interpreter_->tensor(index);

    if (t == nullptr) {
      return -1;
    }

    size_t total_size = 1;

    for (auto i = 0; i < t->dims->size; ++i) {
      total_size *= t->dims->data[i];
    }

    return total_size;
  }

  float GetTensorScale(int index) {
    TfLiteTensor* t = interpreter_->tensor(index);

    if (t != nullptr) {
      return t->params.scale;
    }

    LOG(ERROR) << "Fail to get tensor: %d" << index;
    return -1;
  }

  int32_t GetTensorZeroPoint(int index) {
    TfLiteTensor* t = interpreter_->tensor(index);

    if (t != nullptr) {
      return t->params.zero_point;
    }

    LOG(ERROR) << "Fail to get tensor: " << index;
    return -1;
  }

  // Return a vector with the flattened contents of a tensor.
  template <typename T>
  std::vector<T> ExtractVector(int index) {
    T* v = interpreter_->typed_tensor<T>(index);

    if (v == nullptr) {
      LOG(ERROR) << "Fail to extract vector from tensor: " << index;
      return std::vector<T>();
    }

    return std::vector<T>(v, v + GetTensorSize(index));
  }

  template <typename T>
  std::vector<float> Dequantize(const std::vector<T>& data, float scale,
                                int32_t zero_point) {
    std::vector<float> f;

    for (T q : data) {
      f.push_back(scale * (q - zero_point));
    }

    return f;
  }

  char* modelBuffer_ = nullptr;
};

int ANeuroPilotTFLiteOptions_create(ANeuralNetworksTFLiteOptions** options) {
  CHECK_UNEXPECTED_NULL(options);

  NpTFLiteOptions* s = new NpTFLiteOptions();
  s->allowFp16PrecisionForFp32 = true;
  s->execParallel = true;
  s->useNNAPI = true;

  *options = reinterpret_cast<ANeuralNetworksTFLiteOptions*>(s);
  return ANEURALNETWORKS_NO_ERROR;
}

int ANeuroPilotTFLiteOptions_setExecParallel(
    ANeuralNetworksTFLiteOptions* options, bool enableParallel) {
  CHECK_UNEXPECTED_NULL(options);

  NpTFLiteOptions* s = reinterpret_cast<NpTFLiteOptions*>(options);
  s->execParallel = enableParallel;
  return ANEURALNETWORKS_NO_ERROR;
}

int ANeuroPilotTFLiteOptions_setAllowFp16PrecisionForFp32(
    ANeuralNetworksTFLiteOptions* options, bool allow) {
  CHECK_UNEXPECTED_NULL(options);

  NpTFLiteOptions* s = reinterpret_cast<NpTFLiteOptions*>(options);
  s->allowFp16PrecisionForFp32 = allow;
  return ANEURALNETWORKS_NO_ERROR;
}

int ANeuroPilotTFLiteOptions_setAccelerationMode(
    ANeuralNetworksTFLiteOptions* options, AccelerationMode mode) {
  CHECK_UNEXPECTED_NULL(options);

  NpTFLiteOptions* s = reinterpret_cast<NpTFLiteOptions*>(options);
  s->useNNAPI = (mode == NP_ACCELERATION_NNAPI);
  return ANEURALNETWORKS_NO_ERROR;
}

int ANeuroPilotTFLiteOptions_setCacheDir(
    ANeuralNetworksTFLiteOptions* options,
    const char* cache_dir) {
  CHECK_UNEXPECTED_NULL(options);

  NpTFLiteOptions* s = reinterpret_cast<NpTFLiteOptions*>(options);
  s->cache_dir = cache_dir;
  return ANEURALNETWORKS_NO_ERROR;
}

int ANeuroPilotTFLiteOptions_setEncryptionLevel(
    ANeuralNetworksTFLiteOptions* options,
    int encryption_level) {
  CHECK_UNEXPECTED_NULL(options);

  NpTFLiteOptions* s = reinterpret_cast<NpTFLiteOptions*>(options);
  s->encryption_level = encryption_level;
  return ANEURALNETWORKS_NO_ERROR;
}

int ANeuroPilotTFLiteOptions_resizeInputTensor(
    ANeuralNetworksTFLiteOptions* options, int32_t index, const int* dims,
    int32_t dimsSize) {
  CHECK(options != nullptr);

  NpTFLiteOptions* s = reinterpret_cast<NpTFLiteOptions*>(options);
  std::vector<int> d{dims, dims + dimsSize};
  LOG(DEBUG) << "TFLiteOptions resize input " << index;
  for (auto x = d.begin(); x != d.end(); ++x) {
    LOG(DEBUG) << *x << " ";
  }
  std::pair<int32_t, std::vector<int>> p = std::make_pair(index, d);
  s->inputDimensions.insert(p);
  return ANEURALNETWORKS_NO_ERROR;
}

void ANeuroPilotTFLiteOptions_free(ANeuralNetworksTFLiteOptions* options) {
  CHECK(options != nullptr);

  NpTFLiteOptions* s = reinterpret_cast<NpTFLiteOptions*>(options);
  delete s;
  s = nullptr;
}

int ANeuroPilotTFLite_create(ANeuralNetworksTFLite** tflite,
                             const char* modelPath) {
  return ANeuroPilotTFLite_createAdv(tflite, modelPath, nullptr);
}

int ANeuroPilotTFLite_createAdv(ANeuralNetworksTFLite** tflite,
                                const char* modelPath,
                                ANeuralNetworksTFLiteOptions* options) {
  MTK_ATRACE_CALL();

  CHECK_UNEXPECTED_NULL(tflite);
  CHECK_UNEXPECTED_NULL(modelPath);

  std::ifstream m(modelPath);

  if (!m.good()) {
    LOG(ERROR) << "Fail to read model file: " << modelPath;
    return ANEURALNETWORKS_BAD_DATA;
  }

  NpTFLiteOptions* s = reinterpret_cast<NpTFLiteOptions*>(options);
  NpTFLite* tf = new NpTFLite(modelPath, s);
  CHECK_UNEXPECTED_NULL(tf);

  if (tf->BuildGraph() != kTfLiteOk) {
    LOG(ERROR) << "Fail to build graph";
    *tflite = nullptr;
    delete tf;
    tf = nullptr;
    return ANEURALNETWORKS_INCOMPLETE;
  }

  *tflite = reinterpret_cast<ANeuralNetworksTFLite*>(tf);
  return ANEURALNETWORKS_NO_ERROR;
}

int ANeuroPilotTFLite_createWithBuffer(ANeuralNetworksTFLite** tflite,
                                       const char* buffer, size_t bufferSize) {
  return ANeuroPilotTFLite_createAdvWithBuffer(tflite, buffer, bufferSize,
                                               nullptr);
}

int ANeuroPilotTFLite_createAdvWithBuffer(
    ANeuralNetworksTFLite** tflite, const char* buffer, size_t bufferSize,
    ANeuralNetworksTFLiteOptions* options) {
  MTK_ATRACE_CALL();

  CHECK_UNEXPECTED_NULL(tflite);
  CHECK_UNEXPECTED_NULL(buffer);

  NpTFLiteOptions* s = reinterpret_cast<NpTFLiteOptions*>(options);
  NpTFLite* tf = new NpTFLite(buffer, bufferSize, s);
  CHECK_UNEXPECTED_NULL(tf);

  if (tf->BuildGraph() != kTfLiteOk) {
    LOG(ERROR) << "Fail to build graph";
    *tflite = nullptr;
    delete tf;
    tf = nullptr;
    return ANEURALNETWORKS_INCOMPLETE;
  }

  *tflite = reinterpret_cast<ANeuralNetworksTFLite*>(tf);
  return ANEURALNETWORKS_NO_ERROR;
}

int ANeuroPilotTFLite_getTensorCount(ANeuralNetworksTFLite* tflite,
                                     TFLiteBufferType btype, int32_t* count) {
  CHECK_UNEXPECTED_NULL(tflite);
  CHECK_UNEXPECTED_NULL(count);
  int ret = ANEURALNETWORKS_NO_ERROR;
  NpTFLite* tf = reinterpret_cast<NpTFLite*>(tflite);

  if (btype == TFLITE_BUFFER_TYPE_INPUT) {
    *count = static_cast<int32_t>(tf->interpreter_->inputs().size());
  } else if (btype == TFLITE_BUFFER_TYPE_OUTPUT) {
    *count = static_cast<int32_t>(tf->interpreter_->outputs().size());
  } else {
    *count = 0;
    ret = ANEURALNETWORKS_BAD_DATA;
  }

  return ret;
}

int ANeuroPilotTFLite_getTensorRank(ANeuralNetworksTFLite* tflite,
                                    TFLiteBufferType btype, int index,
                                    int* rank) {
  CHECK_UNEXPECTED_NULL(tflite);
  CHECK_UNEXPECTED_NULL(rank);
  NpTFLite* tf = reinterpret_cast<NpTFLite*>(tflite);
  int tensor_index = 0;

  if (btype == TFLITE_BUFFER_TYPE_INPUT) {
    tensor_index = tf->interpreter_->inputs()[index];
  } else {
    tensor_index = tf->interpreter_->outputs()[index];
  }
  TfLiteTensor* tensor = tf->interpreter_->tensor(tensor_index);
  *rank = tensor->dims->size;
  return ANEURALNETWORKS_NO_ERROR;
}

int ANeuroPilotTFLite_getTensorDimensions(ANeuralNetworksTFLite* tflite,
                                          TFLiteBufferType btype, int index,
                                          int* dimensions) {
  CHECK_UNEXPECTED_NULL(tflite);
  CHECK_UNEXPECTED_NULL(dimensions);
  NpTFLite* tf = reinterpret_cast<NpTFLite*>(tflite);
  int tensor_index = 0;

  if (btype == TFLITE_BUFFER_TYPE_INPUT) {
    tensor_index = tf->interpreter_->inputs()[index];
  } else {
    tensor_index = tf->interpreter_->outputs()[index];
  }
  TfLiteTensor* tensor = tf->interpreter_->tensor(tensor_index);
  for (auto i = 0; i < tensor->dims->size; i++) {
    *dimensions = tensor->dims->data[i];
    dimensions++;
  }
  return ANEURALNETWORKS_NO_ERROR;
}

int ANeuroPilotTFLite_getTensorByteSize(ANeuralNetworksTFLite* tflite,
                                        TFLiteBufferType btype, int index,
                                        size_t* size) {
  CHECK_UNEXPECTED_NULL(tflite);
  CHECK_UNEXPECTED_NULL(size);
  NpTFLite* tf = reinterpret_cast<NpTFLite*>(tflite);
  int tensor_index = 0;

  if (btype == TFLITE_BUFFER_TYPE_INPUT) {
    tensor_index = tf->interpreter_->inputs()[index];
  } else {
    tensor_index = tf->interpreter_->outputs()[index];
  }
  TfLiteTensor* tensor = tf->interpreter_->tensor(tensor_index);
  *size = tensor->bytes;

  return ANEURALNETWORKS_NO_ERROR;
}

int ANeuroPilotTFLite_getTensorType(ANeuralNetworksTFLite* tflite,
                                    TFLiteBufferType btype, int index,
                                    TFLiteTensorType* ttype) {
  CHECK_UNEXPECTED_NULL(tflite);
  CHECK_UNEXPECTED_NULL(ttype);
  NpTFLite* tf = reinterpret_cast<NpTFLite*>(tflite);
  int tensor_index = 0;

  if (btype == TFLITE_BUFFER_TYPE_INPUT) {
    tensor_index = tf->interpreter_->inputs()[index];
  } else {
    tensor_index = tf->interpreter_->outputs()[index];
  }
  TfLiteTensor* tensor = tf->interpreter_->tensor(tensor_index);

  switch (tensor->type) {
    case kTfLiteFloat32:
      *ttype = TFLITE_TENSOR_TYPE_FLOAT;
      break;
    case kTfLiteUInt8:
      *ttype = TFLITE_TENSOR_TYPE_UINT8;
      break;
    default:
      *ttype = TFLITE_TENSOR_TYPE_NONE;
      break;
  }
  return ANEURALNETWORKS_NO_ERROR;
}

int ANeuroPilotTFLite_setInputTensorData(ANeuralNetworksTFLite* tflite,
                                         int index, const void* data,
                                         size_t size) {
  CHECK_UNEXPECTED_NULL(tflite);
  CHECK_UNEXPECTED_NULL(data);
  NpTFLite* tf = reinterpret_cast<NpTFLite*>(tflite);
  int tensor_index = 0;
  tensor_index = tf->interpreter_->inputs()[index];
  TfLiteTensor* tensor = tf->interpreter_->tensor(tensor_index);
  CHECK(size == tensor->bytes);
  memcpy(tensor->data.raw, data, size);

  return ANEURALNETWORKS_NO_ERROR;
}

int ANeuroPilotTFLite_getOutputTensorData(ANeuralNetworksTFLite* tflite,
                                          int index, void* data, size_t size) {
  CHECK_UNEXPECTED_NULL(tflite);
  CHECK_UNEXPECTED_NULL(data);
  NpTFLite* tf = reinterpret_cast<NpTFLite*>(tflite);
  int tensor_index = 0;
  tensor_index = tf->interpreter_->outputs()[index];
  TfLiteTensor* tensor = tf->interpreter_->tensor(tensor_index);
  CHECK(size == tensor->bytes);
  memcpy(data, tensor->data.raw, size);

  return ANEURALNETWORKS_NO_ERROR;
}

int ANeuroPilotTFLite_getDequantizedOutputByIndex(ANeuralNetworksTFLite* tflite,
                                                  void* buffer,
                                                  size_t bufferByteSize,
                                                  int tensorIndex) {
  MTK_ATRACE_CALL();
  CHECK_UNEXPECTED_NULL(tflite);
  CHECK_UNEXPECTED_NULL(buffer);

  NpTFLite* tf = reinterpret_cast<NpTFLite*>(tflite);

  if (float* data = tf->interpreter_->typed_tensor<float>(0)) {
    LOG(ERROR) << "Can't get dequantized output with float model";
    return ANEURALNETWORKS_BAD_DATA;
  }

  std::vector<float> v = tf->GetDequantizedOutput(tensorIndex);

  if (v.empty()) {
    LOG(ERROR) << "Empty dequantized data";
    return ANEURALNETWORKS_BAD_DATA;
  }

  if (bufferByteSize != v.size() * sizeof(float)) {
    LOG(ERROR) << "Invalid buffer size: " << bufferByteSize
               << " != " << (size_t)(v.size() * sizeof(float));
    return ANEURALNETWORKS_BAD_DATA;
  }

  std::copy(v.begin(), v.end(), reinterpret_cast<float*>(buffer));
  return ANEURALNETWORKS_NO_ERROR;
}

int ANeuroPilotTFLite_invoke(ANeuralNetworksTFLite* tflite) {
  MTK_ATRACE_CALL();
  CHECK_UNEXPECTED_NULL(tflite);

  NpTFLite* tf = reinterpret_cast<NpTFLite*>(tflite);
  return tf->interpreter_->Invoke();
}

void ANeuroPilotTFLite_free(ANeuralNetworksTFLite* tflite) {
  MTK_ATRACE_CALL();
  CHECK(tflite != nullptr);

  NpTFLite* tf = reinterpret_cast<NpTFLite*>(tflite);
  delete tf;
  tf = nullptr;
}

/**
 * Deprecated
 */
int ANeuroPilotTFLite_bindToDeivce(ANeuralNetworksTFLite* tflite,
                                   uint32_t device) {
  UNUSED(tflite);
  UNUSED(device);
  return ANEURALNETWORKS_NO_ERROR;
}

/**
 * Deprecated
 */
int ANeuroPilotTFLite_setExecParallel(ANeuralNetworksTFLite* tflite,
                                      bool enableParallel) {
  UNUSED(tflite);
  UNUSED(enableParallel);
  return ANEURALNETWORKS_NO_ERROR;
}

/**
 * Deprecated
 */
int ANeuroPilotTFLite_setAllowFp16PrecisionForFp32(
    ANeuralNetworksTFLite* tflite, bool allow) {
  MTK_ATRACE_CALL();
  CHECK_UNEXPECTED_NULL(tflite);

  TfLiteStatus status = kTfLiteOk;
  NpTFLite* tf = reinterpret_cast<NpTFLite*>(tflite);

  int input = tf->interpreter_->inputs()[0];
  TfLiteType input_data_type = tf->interpreter_->tensor(input)->type;

  if (input_data_type != kTfLiteFloat32) {
    LOG(ERROR) << "Can't set allow FP16 precision with a non-float model";
    return ANEURALNETWORKS_BAD_STATE;
  }

  if (tf->options_.allowFp16PrecisionForFp32 == allow) {
    return ANEURALNETWORKS_NO_ERROR;
  }

  LOG(DEBUG) << "Set allow FP16 precision for FP32: " << allow;
  tf->options_.allowFp16PrecisionForFp32 = allow;
  // Set false to reset NNAPIDelegate
  tf->interpreter_->UseNNAPI(false);
  // Must be called before UseNNAPI()
  tf->interpreter_->SetAllowFp16PrecisionForFp32(
      tf->options_.allowFp16PrecisionForFp32);
  // Set true to recreate NNAPIDelegate
  tf->interpreter_->UseNNAPI(tf->options_.useNNAPI);
  status = tf->interpreter_->Invoke();
  return (status == kTfLiteOk ? ANEURALNETWORKS_NO_ERROR
                              : ANEURALNETWORKS_BAD_STATE);
}

/**
 * Deprecated
 */
int ANeuroPilotTFLiteCustomOp_getInput(TfLiteContext* context, TfLiteNode* node,
                                       int index,
                                       TFLiteTensorExt* tfliteTensor) {
  MTK_ATRACE_CALL();
  TfLiteTensor* tensor = &context->tensors[node->inputs->data[index]];
  CHECK_UNEXPECTED_NULL(tensor);

  for (int i = 0; i < tensor->dims->size; i++) {
    if (i >= TFLITE_TENSOR_MAX_DIMENSTIONS) break;

    tfliteTensor->dims[i] = tensor->dims->data[i];
  }

  tfliteTensor->dimsSize = tensor->dims->size;
  tfliteTensor->dimsSize =
      (tfliteTensor->dimsSize >= TFLITE_TENSOR_MAX_DIMENSTIONS)
          ? TFLITE_TENSOR_MAX_DIMENSTIONS
          : tfliteTensor->dimsSize;

  if (tensor->type == kTfLiteFloat32) {
    tfliteTensor->buffer = reinterpret_cast<void*>(tensor->data.raw);
    tfliteTensor->type = TFLITE_TENSOR_TYPE_FLOAT;
    tfliteTensor->bufferSize = tensor->bytes;
  } else if (tensor->type == kTfLiteUInt8) {
    tfliteTensor->buffer = reinterpret_cast<void*>(tensor->data.raw);
    tfliteTensor->type = TFLITE_TENSOR_TYPE_UINT8;
    tfliteTensor->bufferSize = tensor->bytes;
  } else {
    LOG(ERROR) << "Input or Output is not float nor uint8 data";
    return ANEURALNETWORKS_UNEXPECTED_NULL;
  }

  return ANEURALNETWORKS_NO_ERROR;
}

/**
 * Deprecated
 */
int ANeuroPilotTFLiteCustomOp_getOutput(TfLiteContext* context,
                                        TfLiteNode* node, int index,
                                        TFLiteTensorExt* tfliteTensor) {
  MTK_ATRACE_CALL();
  TfLiteTensor* tensor = &context->tensors[node->outputs->data[index]];
  CHECK_UNEXPECTED_NULL(tensor);

  for (int i = 0; i < tensor->dims->size; i++) {
    if (i >= TFLITE_TENSOR_MAX_DIMENSTIONS) break;

    tfliteTensor->dims[i] = tensor->dims->data[i];
  }

  tfliteTensor->dimsSize = tensor->dims->size;
  tfliteTensor->dimsSize =
      (tfliteTensor->dimsSize >= TFLITE_TENSOR_MAX_DIMENSTIONS)
          ? TFLITE_TENSOR_MAX_DIMENSTIONS
          : tfliteTensor->dimsSize;

  if (tensor->type == kTfLiteFloat32) {
    tfliteTensor->buffer = reinterpret_cast<void*>(tensor->data.raw);
    tfliteTensor->type = TFLITE_TENSOR_TYPE_FLOAT;
    tfliteTensor->bufferSize = tensor->bytes;
  } else if (tensor->type == kTfLiteUInt8) {
    tfliteTensor->buffer = reinterpret_cast<void*>(tensor->data.raw);
    tfliteTensor->type = TFLITE_TENSOR_TYPE_UINT8;
    tfliteTensor->bufferSize = tensor->bytes;
  } else {
    LOG(ERROR) << "Input or Output is not float nor uint8 data";
    return ANEURALNETWORKS_UNEXPECTED_NULL;
  }

  return ANEURALNETWORKS_NO_ERROR;
}

/**
 * Deprecated
 */
int ANeuroPilotTFLiteCustomOp_resizeOutput(TfLiteContext* context,
                                           TfLiteNode* node, int index,
                                           TfLiteIntArray* new_size) {
  MTK_ATRACE_CALL();
  TfLiteTensor* tensor = &context->tensors[node->outputs->data[index]];
  return (kTfLiteOk == context->ResizeTensor(
                           context, tensor,
                           reinterpret_cast<TfLiteIntArray*>(new_size))
              ? ANEURALNETWORKS_NO_ERROR
              : ANEURALNETWORKS_BAD_DATA);
}

/**
 * Deprecated
 */
void* ANeuroPilotTFLiteCustomOp_getUserData(TfLiteNode* node) {
  MTK_ATRACE_CALL();
  return node->user_data;
}

/**
 * Deprecated
 */
int ANeuroPilotTFLiteCustomOp_getFloatAttribute(const char* buffer,
                                                size_t length, const char* attr,
                                                float* outValue) {
  MTK_ATRACE_CALL();

  if (attr == nullptr || outValue == nullptr) {
    return ANEURALNETWORKS_BAD_DATA;
  }

  flexbuffers::Map m =
      flexbuffers::GetRoot((unsigned char*)buffer, length).AsMap();
  const auto& keys = m.Keys();

  for (size_t i = 0; i < keys.size(); ++i) {
    const auto key = keys[i].AsKey();

    if (std::strcmp(key, attr) == 0) {
      const auto& value = m[key];

      if (value.GetType() == flexbuffers::FBT_FLOAT) {
        *outValue = value.AsFloat();
        return ANEURALNETWORKS_NO_ERROR;
      }
    }
  }

  return ANEURALNETWORKS_BAD_DATA;
}

/**
 * Deprecated
 */
int ANeuroPilotTFLiteCustomOp_getIntAttribute(const char* buffer, size_t length,
                                              const char* attr,
                                              int32_t* outValue) {
  MTK_ATRACE_CALL();

  if (attr == nullptr || outValue == nullptr) {
    return ANEURALNETWORKS_BAD_DATA;
  }

  flexbuffers::Map m =
      flexbuffers::GetRoot((unsigned char*)buffer, length).AsMap();
  const auto& keys = m.Keys();

  for (size_t i = 0; i < keys.size(); ++i) {
    const auto key = keys[i].AsKey();

    if (std::strcmp(key, attr) == 0) {
      const auto& value = m[key];

      if (value.GetType() == flexbuffers::FBT_INT) {
        *outValue = value.AsInt32();
        return ANEURALNETWORKS_NO_ERROR;
      }
    }
  }

  return ANEURALNETWORKS_BAD_DATA;
}

/**
 * Deprecated
 */
TfLiteIntArray* ANeuroPilotTFLite_createIntArray(int size) {
  return TfLiteIntArrayCreate(size);
}

/**
 * Deprecated
 */
int ANeuroPilotTFLite_freeIntArray(TfLiteIntArray* v) {
  free(v);
  return ANEURALNETWORKS_NO_ERROR;
}

int ANeuroPilot_getInferencePreference(void) {
  MTK_ATRACE_CALL();
  int ret = NP_INFERENCE_TYPE_NONE;
  static int32_t version = tflite::mtk::GetAndroidSdkVersionCached();
  static bool mtk_nn_quant_preferred =
      tflite::mtk::PropertyGetBool("ro.vendor.mtk_nn_quant_preferred", false);
  if (version <= 27) {
    ret = NP_INFERENCE_TYPE_QNAUT;
  } else {
    ret = (mtk_nn_quant_preferred ? NP_INFERENCE_TYPE_QNAUT
                                  : NP_INFERENCE_TYPE_FLOAT);
  }
  return ret;
}

/**
 * Deprecated
 */
int ANeuroPilotTFLite_getTensor(ANeuralNetworksTFLite* tflite,
                                TFLiteBufferType btype,
                                TFLiteTensorExt* tfliteTensor) {
  return ANeuroPilotTFLite_getTensorByIndex(tflite, btype, tfliteTensor, 0);
}

/**
 * Deprecated
 */
int ANeuroPilotTFLite_getTensorByIndex(ANeuralNetworksTFLite* tflite,
                                       TFLiteBufferType btype,
                                       TFLiteTensorExt* tfliteTensor,
                                       int tensorIndex) {
  MTK_ATRACE_CALL();
  CHECK_UNEXPECTED_NULL(tflite);
  CHECK_UNEXPECTED_NULL(tfliteTensor);

  NpTFLite* tf = reinterpret_cast<NpTFLite*>(tflite);
  int index = 0;

  tfliteTensor->type = TFLITE_TENSOR_TYPE_NONE;
  tfliteTensor->dimsSize = 0;
  memset(&tfliteTensor->dims[0], 0,
         sizeof(int) * TFLITE_TENSOR_MAX_DIMENSTIONS);
  tfliteTensor->buffer = nullptr;
  tfliteTensor->bufferSize = 0;

  if (btype == TFLITE_BUFFER_TYPE_INPUT) {
    index = tf->interpreter_->inputs()[tensorIndex];
  } else if (btype == TFLITE_BUFFER_TYPE_OUTPUT) {
    index = tf->interpreter_->outputs()[tensorIndex];
  }

  TfLiteTensor* tensor = tf->interpreter_->tensor(index);

  for (int i = 0; i < tensor->dims->size; i++) {
    if (i >= TFLITE_TENSOR_MAX_DIMENSTIONS) break;

    tfliteTensor->dims[i] = tensor->dims->data[i];
  }

  tfliteTensor->dimsSize = tensor->dims->size;
  tfliteTensor->dimsSize =
      (tfliteTensor->dimsSize >= TFLITE_TENSOR_MAX_DIMENSTIONS)
          ? TFLITE_TENSOR_MAX_DIMENSTIONS
          : tfliteTensor->dimsSize;

  if (float* data = tf->interpreter_->typed_tensor<float>(index)) {
    tfliteTensor->buffer = reinterpret_cast<void*>(data);
    tfliteTensor->type = TFLITE_TENSOR_TYPE_FLOAT;
    tfliteTensor->bufferSize = tensor->bytes;
  } else if (unsigned char* data =
                 tf->interpreter_->typed_tensor<unsigned char>(index)) {
    tfliteTensor->buffer = reinterpret_cast<void*>(data);
    tfliteTensor->type = TFLITE_TENSOR_TYPE_UINT8;
    tfliteTensor->bufferSize = tensor->bytes;
  } else {
    LOG(ERROR) << "Input or Output is not float nor uint8 data";
    return ANEURALNETWORKS_UNEXPECTED_NULL;
  }

  return ANEURALNETWORKS_NO_ERROR;
}
