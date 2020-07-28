/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
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
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "MtkNnApiDelegateP"
#include "tensorflow/lite/mtk/mtk_minimal_logging.h"
#include "tensorflow/lite/mtk/mtk_helper.h"
#include "tensorflow/lite/mtk/delegates/nnapi/mtk_nnapi_delegate_p.h"
#include "tensorflow/lite/builtin_ops.h"
#include "tensorflow/lite/context_util.h"
#include "tensorflow/lite/c/builtin_op_data.h"
#include "tensorflow/lite/nnapi/NeuralNetworksShim.h"
#include "tensorflow/lite/nnapi/NeuralNetworksOEM.h"
#include "tensorflow/lite/schema/schema_generated.h"

#include <map>
#include <vector>

#include <sys/mman.h>
#include <sys/system_properties.h>
#include <unistd.h>

namespace tflite {
namespace {

#define CHECK_NN(context, code)                                           \
  if (code != ANEURALNETWORKS_NO_ERROR) {                                 \
    context->ReportError(context, "NN API returned error (%d).\n", code); \
    return kTfLiteError;                                                  \
  }

constexpr int32_t kMinSdkVersionForNNAPI = 27;
constexpr int32_t kMinSdkVersionForNNAPI11 = 28;
static const int32_t kAndroidSdkVersion = mtk::GetAndroidSdkVersionCached();
static const bool kUseOemScaler = (kAndroidSdkVersion>=29) ? true :
    mtk::IsFeatureSupported(mtk::ANEUROPILOT_FEATURE_OEM_STRING_SCALAR);

// RAII NN API Model Destructor for use with std::unique_ptr
struct NNFreeModel {
  void operator()(ANeuralNetworksModel* model) {
    ANeuralNetworksModel_free(model);
  }
};

// RAII NN API Compilation Destructor for use with std::unique_ptr
struct NNFreeCompilation {
  void operator()(ANeuralNetworksCompilation* model) {
    ANeuralNetworksCompilation_free(model);
  }
};

// Manage NNAPI shared memory handle
class NNMemory {
 public:
  NNMemory(const char* name, size_t size) {
#ifdef __ANDROID__
    byte_size_ = size;
    fd_ = ASharedMemory_create(name, size);
    data_ptr_ = reinterpret_cast<uint8_t*>(
        mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0));
    ANeuralNetworksMemory_createFromFd(size, PROT_READ | PROT_WRITE, fd_, 0,
                                       &nn_memory_handle_);
#endif
  }

  ~NNMemory() {
#ifdef __ANDROID__
    if (data_ptr_) {
      munmap(data_ptr_, byte_size_);
    }
    if (nn_memory_handle_) {
      ANeuralNetworksMemory_free(nn_memory_handle_);
    }
    if (fd_ > 0) close(fd_);
#endif
  }

  ANeuralNetworksMemory* get_handle() { return nn_memory_handle_; }
  uint8_t* get_data_ptr() { return data_ptr_; }

 private:
#ifdef __ANDROID__
  int fd_ = 0;
  size_t byte_size_ = 0;
#endif
  uint8_t* data_ptr_ = nullptr;
  ANeuralNetworksMemory* nn_memory_handle_ = nullptr;
};  // namespace

// Track tensor indices to NN API tensor indices mapping.
class OperandMapping {
 public:
  // Given a TFLite index return the ANN index. If it doesn't exist
  // return -1.
  int lite_index_to_ann(int index) const {
    if ((size_t)index < lite_tensor_to_ann_tensor_.size())
      return lite_tensor_to_ann_tensor_[index];
    else
      return -1;
  }

  // NN API uses non tensor operands instead of structs. This creates one
  // and returns the index. It uses a std::vector and resizes it as needed
  // keeping -1 to unmapped values. Intermediate tensors likely will not
  // be mapped.
  int add_new_non_tensor_operand() { return next_ann_tensor_index_++; }
  /// M: NeuroPilot {@
  void set_new_non_tensor_operand(int new_index) {
    next_ann_tensor_index_ = new_index;
  }
  /// M: NeuroPilot @}
  // Add a new mapping from `tflite_index` and return the NN API tensor index.
  int add_new_ann_tensor_index(int tflite_index) {
    if ((size_t)tflite_index >= lite_tensor_to_ann_tensor_.size()) {
      lite_tensor_to_ann_tensor_.resize(tflite_index + 1, -1);
    }
    int new_tensor_index = next_ann_tensor_index_++;
    lite_tensor_to_ann_tensor_[tflite_index] = new_tensor_index;
    return new_tensor_index;
  }

 private:
  // Next index of ann tensor
  int next_ann_tensor_index_ = 0;

  // Mapping from lite index. Use a std::vector for speed and code size
  // rather than a map.
  std::vector<int> lite_tensor_to_ann_tensor_;
};

// Abstract builder for building an op in the NN API graph. This handles
// the disparity between TFLite and NN API operand types. NN API has singular
// operands for both tensors and parameters, and TFLite separates the two.
class NNAPIOpBuilder {
 public:
  NNAPIOpBuilder(TfLiteContext* context, OperandMapping* tensor_mapping,
                 ANeuralNetworksModel* nn_model)
      : context_(context),
        operand_mapping_(tensor_mapping),
        nn_model_(nn_model) {}

  TfLiteStatus AddScalarInt32Operand(int32_t value) {
    return AddScalarOperand<int32_t>(value, ANEURALNETWORKS_INT32);
  }

  TfLiteStatus AddScalarFloat32Operand(float value) {
    return AddScalarOperand<float>(value, ANEURALNETWORKS_FLOAT32);
  }

  TfLiteStatus AddVectorInt32Operand(const int32_t* values,
                                     uint32_t num_values) {
    return AddVectorOperand<int32_t>(values, num_values,
                                     ANEURALNETWORKS_TENSOR_INT32);
  }

  TfLiteStatus AddVectorFloat32Operand(const float* values,
                                       uint32_t num_values) {
    return AddVectorOperand<float>(values, num_values,
                                   ANEURALNETWORKS_TENSOR_FLOAT32);
  }

  /// M: NeuroPilot @{
  TfLiteStatus AddOemScalarStringOperand(int32_t op_hash,
                                         const char* op_string) {
    auto scalar_it = oem_scalar_map_.find(op_hash);
    auto size_it = oem_scalar_size_map_.find(op_hash);
    if (scalar_it != oem_scalar_map_.end()) {
      return AddOemScalarOperand(scalar_it->second, size_it->second);
    }
    size_t oem_scalar_size = 0;
    uint8_t* oem_scalar = nullptr;
    oem_scalar_size = mtk::PackOemScalarString(op_string, &oem_scalar);
    oem_scalar_map_.insert(std::pair<int32_t, uint8_t*>(op_hash,
                                                        oem_scalar));
    oem_scalar_size_map_.insert(std::pair<int32_t, uint32_t>(op_hash,
                                                             oem_scalar_size));
    return AddOemScalarOperand(oem_scalar, oem_scalar_size);
  }
  /// M: NeuroPilot @}

  TfLiteStatus AddPoolingParams(void* data) {
    auto builtin = reinterpret_cast<TfLitePoolParams*>(data);
    AddScalarInt32Operand(builtin->padding);
    AddScalarInt32Operand(builtin->stride_width);
    AddScalarInt32Operand(builtin->stride_height);
    AddScalarInt32Operand(builtin->filter_width);
    AddScalarInt32Operand(builtin->filter_height);
    AddScalarInt32Operand(builtin->activation);
    return kTfLiteOk;
  }

  TfLiteStatus AddTensorInput(int tensor_index) {
    int ann_index;
    TF_LITE_ENSURE_STATUS(AddTensor(tensor_index, &ann_index));
    augmented_inputs_.push_back(ann_index);
    return kTfLiteOk;
  }

  TfLiteStatus AddTensorOutput(int tensor_index) {
    int ann_index;
    TF_LITE_ENSURE_STATUS(AddTensor(tensor_index, &ann_index));
    augmented_outputs_.push_back(ann_index);
    return kTfLiteOk;
  }

  TfLiteStatus AddAdditionalFloat32OutputTensor(uint32_t dimension_count) {
    std::vector<uint32_t> dims(dimension_count, 0);
    ANeuralNetworksOperandType operand_type{
        .type = ANEURALNETWORKS_TENSOR_FLOAT32,
        .dimensionCount = dimension_count,
        .dimensions = dims.data()};
    CHECK_NN(context_,
             ANeuralNetworksModel_addOperand(nn_model_, &operand_type));
    int ann_operand = operand_mapping_->add_new_non_tensor_operand();
    augmented_outputs_.push_back(ann_operand);
    return kTfLiteOk;
  }

  TfLiteStatus AddStateFloat32Tensor(int tensor_index,
                                     int* ann_tensor_index_out) {
    TfLiteTensor* tensor = &context_->tensors[tensor_index];
    int ann_index = operand_mapping_->add_new_non_tensor_operand();

    ANeuralNetworksOperandType operand_type{
        ANEURALNETWORKS_TENSOR_FLOAT32,
        static_cast<uint32_t>(tensor->dims->size),
        reinterpret_cast<uint32_t*>(tensor->dims->data), tensor->params.scale,
        tensor->params.zero_point};
    CHECK_NN(context_,
             ANeuralNetworksModel_addOperand(nn_model_, &operand_type));
    augmented_outputs_.push_back(ann_index);

    *ann_tensor_index_out = ann_index;
    return kTfLiteOk;
  }

  // Adds a new NN API tensor that shadows the TF Lite tensor `tensor_index`.
  // This returns the NN API tensor index corresponding to the created tensor.
  // If another caller previously created a NN API tensor for `tensor_index`
  // then the existing one is returned.
  TfLiteStatus AddTensor(int tensor_index, int* ann_tensor_index_out) {
    int ann_tensor_index = operand_mapping_->lite_index_to_ann(tensor_index);
    if (ann_tensor_index != -1) {
      *ann_tensor_index_out = ann_tensor_index;
      return kTfLiteOk;
    }
    // Allocate a new tensor index
    ann_tensor_index = operand_mapping_->add_new_ann_tensor_index(tensor_index);
    TFLITE_MTK_LOG_INFO("AddTensor tensor_index: %d, ann_tensor_index: %d",
      tensor_index, ann_tensor_index);
    // Parameters needed for new type.
    int32_t nn_type = 0;
    float scale = 0.0f;
    int32_t zeroPoint = 0;
    TfLiteTensor* tensor = &context_->tensors[tensor_index];
    switch (tensor->type) {
      case kTfLiteNoType:
        // Tensors added during initialization of Ops don't have a type yet and
        // should not be registered with the NNAPI.
        *ann_tensor_index_out = -1;
        return kTfLiteOk;
      case kTfLiteFloat32:
        nn_type = ANEURALNETWORKS_TENSOR_FLOAT32;
        break;
      case kTfLiteUInt8:
        nn_type = ANEURALNETWORKS_TENSOR_QUANT8_ASYMM;
        scale = tensor->params.scale;
        zeroPoint = tensor->params.zero_point;
        if (scale == 0) {
          // TENSOR_QUANT8_ASYMM with zero scale is not valid in NNAPI.
          scale = 1;
        }
        break;
      case kTfLiteInt32:
        nn_type = ANEURALNETWORKS_TENSOR_INT32;
        scale = tensor->params.scale;
        zeroPoint = tensor->params.zero_point;
        break;
      default:
        context_->ReportError(context_, "Logic error in NN API Delegate.\n");
        return kTfLiteError;
    }
    /// M: NeuroPilot {@
    TfLiteIntArray* tmpDims = nullptr;
    if (tensor->dims->size == 0) {
      tmpDims = TfLiteIntArrayCreate(1);
      tmpDims->data[0] = 1;
      tensor->dims->size = 1;
      tensor->dims = tmpDims;
      context_->ReportError(context_,
            "Redirect empty shape tensor to NN API (tensor %d name %s)",
            tensor_index, tensor->name);
    }
    if (tensor->dims->size > 4) {
      context_->ReportError(context_,
            "NNAPI doesn't support tensors with rank > 4 (tensor %d name %s)",
            tensor_index, tensor->name);
      return kTfLiteError;
    }
    /// M: NeuroPilot @}
    ANeuralNetworksOperandType operand_type{
        nn_type, static_cast<uint32_t>(tensor->dims->size),
        reinterpret_cast<uint32_t*>(tensor->dims->data), scale, zeroPoint};
    CHECK_NN(context_,
             ANeuralNetworksModel_addOperand(nn_model_, &operand_type));

    if (tensor->allocation_type == kTfLiteMmapRo) {
      // TODO(b/80630405): Use NNAPIAllocation.
      CHECK_NN(context_, ANeuralNetworksModel_setOperandValue(
                             nn_model_, ann_tensor_index, tensor->data.raw,
                             tensor->bytes));
    }
    /// M: NeuroPilot {@
    if (tmpDims != nullptr) {
      TfLiteIntArrayFree(tmpDims);
    }
    /// @}
    *ann_tensor_index_out = ann_tensor_index;
    return kTfLiteOk;
  }

  // Finish emitting the op (of type `type`) into the NN API.
  TfLiteStatus FinalizeAddOperation(ANeuralNetworksOperationType type) {
    // Actually add a NN API operation
    CHECK_NN(context_, ANeuralNetworksModel_addOperation(
                           nn_model_, type,
                           static_cast<uint32_t>(augmented_inputs_.size()),
                           augmented_inputs_.data(),
                           static_cast<uint32_t>(augmented_outputs_.size()),
                           augmented_outputs_.data()));
    augmented_inputs_.clear();
    augmented_outputs_.clear();
    return kTfLiteOk;
  }

  /// M: NeuroPilot {@
  uint32_t GetNextAnnOperandIdx(void) {
    return (uint32_t)operand_mapping_->add_new_non_tensor_operand();
  }

  void UpdateAnnOperandIdx(int new_idx) {
    operand_mapping_->set_new_non_tensor_operand(new_idx);
  }

  std::vector<uint32_t>& GetAugmentedInputs(void) {
    return augmented_inputs_;
  }

  std::vector<uint32_t>& GetAugmentedOutputs(void) {
    return augmented_outputs_;
  }

  ANeuralNetworksModel* GetNnModel(void) {
    return nn_model_;
  }

  void PopbackInput(void) {
    augmented_inputs_.pop_back();
  }

  void PopbackOutput(void) {
    augmented_outputs_.pop_back();
  }
  /// M: NeuroPilot @}

 private:
  template <typename T>
  TfLiteStatus AddScalarOperand(T value, int32_t nn_type) {
    ANeuralNetworksOperandType operand_type{.type = nn_type};
    CHECK_NN(context_,
             ANeuralNetworksModel_addOperand(nn_model_, &operand_type));
    int ann_operand = operand_mapping_->add_new_non_tensor_operand();
    CHECK_NN(context_, ANeuralNetworksModel_setOperandValue(
                           nn_model_, ann_operand, &value, sizeof(T)));
    augmented_inputs_.push_back(ann_operand);
    return kTfLiteOk;
  }

  template <typename T>
  TfLiteStatus AddVectorOperand(const T* values, uint32_t num_values,
                                int32_t nn_type) {
    ANeuralNetworksOperandType operand_type{
        .type = nn_type, .dimensionCount = 1, .dimensions = &num_values};
    CHECK_NN(context_,
             ANeuralNetworksModel_addOperand(nn_model_, &operand_type));
    int ann_operand = operand_mapping_->add_new_non_tensor_operand();
    CHECK_NN(context_,
             ANeuralNetworksModel_setOperandValue(
                 nn_model_, ann_operand, values, sizeof(T) * num_values));
    augmented_inputs_.push_back(ann_operand);
    return kTfLiteOk;
  }

  /// M: NeuroPilot {@
  TfLiteStatus AddOemScalarOperand(uint8_t* value, size_t value_size) {
    ANeuralNetworksOperandType operand_type{.type = ANEURALNETWORKS_OEM_SCALAR};
    CHECK_NN(context_,
             ANeuralNetworksModel_addOperand(nn_model_, &operand_type));
    int ann_operand = operand_mapping_->add_new_non_tensor_operand();
    CHECK_NN(context_, ANeuralNetworksModel_setOperandValue(
                           nn_model_, ann_operand, value, value_size));
    augmented_inputs_.push_back(ann_operand);
    return kTfLiteOk;
  }

  std::map<int32_t, uint8_t*> oem_scalar_map_;
  std::map<int32_t, uint32_t> oem_scalar_size_map_;
  /// M: NeuroPilot @}

  // TfLiteContext for error handling. Must be named context for macros to
  // work.
  TfLiteContext* context_;

  // Tracks relationship between indices
  OperandMapping* operand_mapping_;

  // The model
  ANeuralNetworksModel* nn_model_;

  // Inputs and outputs for the current op. These are augmented in the sense
  // that NN API uses operands for all arguments, not just tensors, unlike
  // TensorFlow lite.
  std::vector<uint32_t> augmented_inputs_;
  std::vector<uint32_t> augmented_outputs_;
};

struct NNAPIOpMappingArgs {
  TfLiteContext* context;
  NNAPIOpBuilder* builder;
  TfLiteNode* node;
  std::vector<int>* model_state_outputs;
  std::vector<int>* model_state_tfl_inputs;
  /// M: NeuroPilot {@
  TfLiteRegistration* registration;
  mtk::CustomOpHelper& custom_op_helper;
  mtk::CustomOpHelper::ParameterFunc (*GetParamFunc)(std::string op_name);
  mtk::CustomOpHelper::MtkExtOpParameterFunc (*GetMtkExtOpParamFunc)(
                                                std::string op_name);
  /// M: NeuroPilot @}
};

// The kernel that represents the subgraph of TF Lite being run on NN API.
class NNAPIDelegateKernel {
 public:
  NNAPIDelegateKernel() = default;

  typedef ANeuralNetworksOperationType (*MappingFn)(
      const NNAPIOpMappingArgs& mapping_args);

  // Return a function that knows how to translate a node into its operands
  // when called. You can use this function to see if a node is supported
  // (i.e. that MappingFn is not nullptr).
  MappingFn Map(TfLiteContext* context, int builtin_code, int version,
                TfLiteNode* node, TfLiteRegistration* registration) {
    switch (builtin_code) {
      case kTfLiteBuiltinAdd:
        if (version == 1) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            auto builtin = reinterpret_cast<TfLiteAddParams*>(
                mapping_args.node->builtin_data);
            mapping_args.builder->AddScalarInt32Operand(builtin->activation);
            return ANEURALNETWORKS_ADD;
          };
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinMul:
        if (version == 1) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            auto builtin = reinterpret_cast<TfLiteMulParams*>(
                mapping_args.node->builtin_data);
            mapping_args.builder->AddScalarInt32Operand(builtin->activation);
            return ANEURALNETWORKS_MUL;
          };
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinAveragePool2d:
        if (version == 1) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            mapping_args.builder->AddPoolingParams(
                mapping_args.node->builtin_data);
            return ANEURALNETWORKS_AVERAGE_POOL_2D;
          };
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinMaxPool2d:
        if (version == 1) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            mapping_args.builder->AddPoolingParams(
                mapping_args.node->builtin_data);
            return ANEURALNETWORKS_MAX_POOL_2D;
          };
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinL2Pool2d:
        if (version == 1) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            mapping_args.builder->AddPoolingParams(
                mapping_args.node->builtin_data);
            return ANEURALNETWORKS_L2_POOL_2D;
          };
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinConv2d:
        if (version == 1) {
          auto builtin =
              reinterpret_cast<TfLiteConvParams*>(node->builtin_data);
          if (builtin->dilation_width_factor != 1 ||
              builtin->dilation_height_factor != 1 || node->inputs->size != 3) {
            // NNAPI does not support dilated Conv2D.
            // return nullptr;
            /// M: NeuroPilot {@
            return [](const NNAPIOpMappingArgs& mapping_args)
                      -> ANeuralNetworksOperationType {
            auto builtin = reinterpret_cast<TfLiteConvParams*>(
                mapping_args.node->builtin_data);
              mapping_args.builder->AddScalarInt32Operand(builtin->padding);
              mapping_args.builder->AddScalarInt32Operand(
                  builtin->stride_width);
              mapping_args.builder->AddScalarInt32Operand(
                  builtin->stride_height);
              mapping_args.builder->AddScalarInt32Operand(
                  builtin->dilation_width_factor);
              mapping_args.builder->AddScalarInt32Operand(
                  builtin->dilation_height_factor);
              mapping_args.builder->AddScalarInt32Operand(builtin->activation);
              int32_t op_hash = mtk::Hash("dilatedconv2dmtk");
              if (kUseOemScaler) {
                mapping_args.builder->AddOemScalarStringOperand(op_hash,
                                  "dilatedconv2dmtk");
              } else {
                mapping_args.builder->AddScalarInt32Operand(op_hash);
              }
              return ANEURALNETWORKS_OEM_OPERATION;
            };
            /// M: NeuroPilot @}
          }
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            auto builtin = reinterpret_cast<TfLiteConvParams*>(
                mapping_args.node->builtin_data);
            mapping_args.builder->AddScalarInt32Operand(builtin->padding);
            mapping_args.builder->AddScalarInt32Operand(builtin->stride_width);
            mapping_args.builder->AddScalarInt32Operand(builtin->stride_height);
            mapping_args.builder->AddScalarInt32Operand(builtin->activation);
            return ANEURALNETWORKS_CONV_2D;
          };
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinDepthwiseConv2d:
        if (version == 1 || version == 2) {
          /// M: NeuroPilot {@
          auto builtin =
              reinterpret_cast<TfLiteDepthwiseConvParams*>(node->builtin_data);
          if (builtin->dilation_width_factor > 1 ||
              builtin->dilation_height_factor > 1) {
            return [](const NNAPIOpMappingArgs& mapping_args)
                      -> ANeuralNetworksOperationType {
              auto builtin = reinterpret_cast<TfLiteDepthwiseConvParams*>(
                  mapping_args.node->builtin_data);
              mapping_args.builder->AddScalarInt32Operand(builtin->padding);
              mapping_args.builder->AddScalarInt32Operand(
                  builtin->stride_width);
              mapping_args.builder->AddScalarInt32Operand(
                  builtin->stride_height);
              mapping_args.builder->AddScalarInt32Operand(
                  builtin->depth_multiplier);
              mapping_args.builder->AddScalarInt32Operand(builtin->activation);
              mapping_args.builder->AddScalarInt32Operand(
                  builtin->dilation_width_factor);
              mapping_args.builder->AddScalarInt32Operand(
                  builtin->dilation_height_factor);
              int32_t op_hash = mtk::Hash("dilateddepthwiseconv2dmtk");
              if (kUseOemScaler) {
                mapping_args.builder->AddOemScalarStringOperand(op_hash,
                                  "dilateddepthwiseconv2dmtk");
              } else {
                mapping_args.builder->AddScalarInt32Operand(op_hash);
              }
              return ANEURALNETWORKS_OEM_OPERATION;
            };
          } else {
            return [](const NNAPIOpMappingArgs& mapping_args)
                      -> ANeuralNetworksOperationType {
              auto builtin = reinterpret_cast<TfLiteDepthwiseConvParams*>(
                  mapping_args.node->builtin_data);
              mapping_args.builder->AddScalarInt32Operand(builtin->padding);
              mapping_args.builder->AddScalarInt32Operand(
                  builtin->stride_width);
              mapping_args.builder->AddScalarInt32Operand(
                  builtin->stride_height);
              mapping_args.builder->AddScalarInt32Operand(
                  builtin->depth_multiplier);
              mapping_args.builder->AddScalarInt32Operand(builtin->activation);
              return ANEURALNETWORKS_DEPTHWISE_CONV_2D;
            };
          }
          /// M: NeuroPilot @}
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinFullyConnected:
        if (version == 1) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            auto builtin = reinterpret_cast<TfLiteFullyConnectedParams*>(
                mapping_args.node->builtin_data);
            mapping_args.builder->AddScalarInt32Operand(builtin->activation);
            return ANEURALNETWORKS_FULLY_CONNECTED;
          };
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinSoftmax:
        if (version == 1) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            auto builtin = reinterpret_cast<TfLiteSoftmaxParams*>(
                mapping_args.node->builtin_data);
            mapping_args.builder->AddScalarFloat32Operand(builtin->beta);
            return ANEURALNETWORKS_SOFTMAX;
          };
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinReshape:
        if (version == 1 && node->inputs->size == 2) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            return ANEURALNETWORKS_RESHAPE;
          };
        } else if (version == 1 && node->inputs->size == 1) {
          /// M: NeuroPilot {@
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            auto builtin = reinterpret_cast<TfLiteReshapeParams*>(
                            mapping_args.node->builtin_data);
            mapping_args.builder->AddVectorInt32Operand(
                builtin->shape,
                static_cast<uint32_t>(builtin->num_dimensions));
            return ANEURALNETWORKS_RESHAPE;
          };
          /// M: NeuroPilot @}
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinSqueeze:
        if (version == 1 && kAndroidSdkVersion >= kMinSdkVersionForNNAPI11) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            auto builtin = reinterpret_cast<TfLiteSqueezeParams*>(
                mapping_args.node->builtin_data);
            // Note that we add the squeeze dimensions even if the dimensions
            // were unspecified (empty), as NNAPI requires the operand.
            mapping_args.builder->AddVectorInt32Operand(
                builtin->squeeze_dims,
                static_cast<uint32_t>(builtin->num_squeeze_dims));
            return ANEURALNETWORKS_SQUEEZE;
          };
        } else {
          return nullptr;
        }
      case kTfLiteBuiltinL2Normalization: {
        auto builtin =
            reinterpret_cast<TfLiteL2NormParams*>(node->builtin_data);
        if (builtin->activation != kTfLiteActNone) {
          // NNAPI does not support activations
          return nullptr;
        }
        return [](const NNAPIOpMappingArgs& mapping_args)
                   -> ANeuralNetworksOperationType {
          return ANEURALNETWORKS_L2_NORMALIZATION;
        };
      }
      case kTfLiteBuiltinLocalResponseNormalization:
        if (version == 1) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            auto builtin = reinterpret_cast<TfLiteLocalResponseNormParams*>(
                mapping_args.node->builtin_data);
            mapping_args.builder->AddScalarInt32Operand(builtin->radius);
            mapping_args.builder->AddScalarFloat32Operand(builtin->bias);
            mapping_args.builder->AddScalarFloat32Operand(builtin->alpha);
            mapping_args.builder->AddScalarFloat32Operand(builtin->beta);
            return ANEURALNETWORKS_LOCAL_RESPONSE_NORMALIZATION;
          };
        } else {
          // TODO(miaowang): clean-up code and return early in the unsupported
          // case.
          return nullptr;
        }
        break;
      case kTfLiteBuiltinLshProjection:
        if (version == 1) {
          // NNAPI does not support sparse projection correctly (b/111751836).
          if (reinterpret_cast<TfLiteLSHProjectionParams*>(node->builtin_data)
                  ->type == kTfLiteLshProjectionSparse) {
            return nullptr;
          }
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            auto builtin = reinterpret_cast<TfLiteLSHProjectionParams*>(
                mapping_args.node->builtin_data);
            mapping_args.builder->AddScalarInt32Operand(builtin->type);
            return ANEURALNETWORKS_LSH_PROJECTION;
          };
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinConcatenation:
        if (version == 1 &&
            reinterpret_cast<TfLiteConcatenationParams*>(node->builtin_data)
                    ->activation == kTfLiteActNone) {
/*
          if (context->tensors[node->inputs->data[0]].type == kTfLiteUInt8) {
            // NNAPI only support concatenating quantized tensor of the same
            // scale and offset.
            auto first_param = context->tensors[node->inputs->data[0]].params;
            for (int i = 0; i < node->inputs->size; i++) {
              auto curr_param = context->tensors[node->inputs->data[i]].params;
              if (curr_param.scale != first_param.scale ||
                  curr_param.zero_point != first_param.zero_point) {
                return nullptr;
              }
            }
          }
*/
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            auto builtin = reinterpret_cast<TfLiteConcatenationParams*>(
                mapping_args.node->builtin_data);
            if (builtin->axis < 0) {
              // Support negative axis
              TfLiteTensor tensor =
                mapping_args.context->tensors[mapping_args.node->inputs->data[0]];
              builtin->axis += tensor.dims->size;
            }
            mapping_args.builder->AddScalarInt32Operand(builtin->axis);
            return ANEURALNETWORKS_CONCATENATION;
          };
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinDequantize:
        if (version == 1) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            return ANEURALNETWORKS_DEQUANTIZE;
          };
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinFloor:
        if (version == 1) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            return ANEURALNETWORKS_FLOOR;
          };
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinRelu:
        if (version == 1) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            return ANEURALNETWORKS_RELU;
          };
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinReluN1To1:
        if (version == 1) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            return ANEURALNETWORKS_RELU1;
          };
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinRelu6:
        if (version == 1) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            return ANEURALNETWORKS_RELU6;
          };
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinLogistic:
        if (version == 1) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            return ANEURALNETWORKS_LOGISTIC;
          };
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinTanh:
        // TODO(miaowang): add additional checks for the parameters.
        if (version == 1 &&
            context->tensors[node->inputs->data[0]].type == kTfLiteFloat32) {
          // NNAPI only support float tanh.
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            return ANEURALNETWORKS_TANH;
          };
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinSub:
/*
        if (version == 1 && kAndroidSdkVersion >= kMinSdkVersionForNNAPI11 &&
            context->tensors[node->inputs->data[0]].type == kTfLiteFloat32) {
          // NNAPI only support float sub.
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            auto builtin = reinterpret_cast<TfLiteSubParams*>(
                mapping_args.node->builtin_data);
            mapping_args.builder->AddScalarInt32Operand(builtin->activation);
            return ANEURALNETWORKS_SUB;
          };
*/
        /// M: NeuroPilot {@
        // NeuroPilot supports both float and uint8 sub
        if (version == 1) {
            return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            auto builtin = reinterpret_cast<TfLiteSubParams*>(
                mapping_args.node->builtin_data);
            mapping_args.builder->AddScalarInt32Operand(builtin->activation);
            return ANEURALNETWORKS_SUB;
          };
        /// M: NeuroPilot @}
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinDiv:
/*
        if (version == 1 && kAndroidSdkVersion >= kMinSdkVersionForNNAPI11 &&
            context->tensors[node->inputs->data[0]].type == kTfLiteFloat32) {
          // NNAPI only support float div.
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            auto builtin = reinterpret_cast<TfLiteDivParams*>(
                mapping_args.node->builtin_data);
            mapping_args.builder->AddScalarInt32Operand(builtin->activation);
            return ANEURALNETWORKS_DIV;
          };
*/
        /// M: NeuroPilot {@
        // NeuroPilot supports both float and uint8 div
        if (version == 1) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            auto builtin = reinterpret_cast<TfLiteDivParams*>(
                mapping_args.node->builtin_data);
            mapping_args.builder->AddScalarInt32Operand(builtin->activation);
            return ANEURALNETWORKS_DIV;
          };
        /// M: NeuroPilot @}
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinPad:
/*
        if (version == 1 && kAndroidSdkVersion >= kMinSdkVersionForNNAPI11 &&
            node->inputs->size == 2 &&
            context->tensors[node->inputs->data[0]].type == kTfLiteFloat32) {
          // NNAPI does not support specifying the padding value.
          // NNAPI pads physical zero for quantized tensors, so only delegate
          // float pad to NNAPI.
*/
        /// M: NeuroPilot {@
        // NeuroPilot supports both float and uint8 pad
        if (version == 1 && node->inputs->size == 2) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            return ANEURALNETWORKS_PAD;
          };
        /// M: NeuroPilot @}
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinSpaceToBatchNd:
        if (version == 1 && kAndroidSdkVersion >= kMinSdkVersionForNNAPI11) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            return ANEURALNETWORKS_SPACE_TO_BATCH_ND;
          };
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinStridedSlice:
        if (version == 1 && kAndroidSdkVersion >= kMinSdkVersionForNNAPI11) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            auto builtin = reinterpret_cast<TfLiteStridedSliceParams*>(
                mapping_args.node->builtin_data);
            mapping_args.builder->AddScalarInt32Operand(builtin->begin_mask);
            mapping_args.builder->AddScalarInt32Operand(builtin->end_mask);
            mapping_args.builder->AddScalarInt32Operand(
                builtin->shrink_axis_mask);
            return ANEURALNETWORKS_STRIDED_SLICE;
          };
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinTranspose:
        // Note that the permutation input tensor value dictates the output
        // dimensions.
        // TODO(b/110888333): Support dynamically-sized tensors in delegates.
        if ((version == 1) &&
            (kAndroidSdkVersion >= kMinSdkVersionForNNAPI11) &&
            (node->inputs->size > 1) &&
            (context->tensors[node->inputs->data[1]].allocation_type ==
             kTfLiteMmapRo)) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            return ANEURALNETWORKS_TRANSPOSE;
          };
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinRnn:
        // NNAPI only support float32 weights.
        if (version == 1 && node->inputs->size == 5 &&
            context->tensors[node->inputs->data[/*kWeightsTensor*/ 1]].type ==
                kTfLiteFloat32) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            // NNAPI need both state_in and state_out.
            int ann_index;
            mapping_args.builder->AddStateFloat32Tensor(
                mapping_args.node->inputs->data[/*kHiddenStateTensor*/ 4],
                &ann_index);
            mapping_args.model_state_outputs->push_back(ann_index);
            mapping_args.model_state_tfl_inputs->push_back(
                mapping_args.node->inputs->data[/*kHiddenStateTensor*/ 4]);
            auto builtin = reinterpret_cast<TfLiteRNNParams*>(
                mapping_args.node->builtin_data);
            mapping_args.builder->AddScalarInt32Operand(builtin->activation);
            return ANEURALNETWORKS_RNN;
          };
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinSvdf:
        // NNAPI only support float32 weights.
        if (version == 1 && node->inputs->size == 5 &&
            context->tensors[node->inputs->data[/*kWeightsFeatureTensor*/ 1]]
                    .type == kTfLiteFloat32) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            // NNAPI need both state_in and state_out.
            int ann_index;
            mapping_args.builder->AddStateFloat32Tensor(
                mapping_args.node->inputs
                    ->data[/*kInputActivationStateTensor*/ 4],
                &ann_index);
            mapping_args.model_state_outputs->push_back(ann_index);
            mapping_args.model_state_tfl_inputs->push_back(
                mapping_args.node->inputs
                    ->data[/*kInputActivationStateTensor*/ 4]);

            auto builtin = reinterpret_cast<TfLiteSVDFParams*>(
                mapping_args.node->builtin_data);
            mapping_args.builder->AddScalarInt32Operand(builtin->rank);
            mapping_args.builder->AddScalarInt32Operand(builtin->activation);
            return ANEURALNETWORKS_SVDF;
          };
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinLstm:
        // NNAPI only support float32 weights.
        // TODO(miaowang): add loggings to indicate why the op is rejected.
        if (version == 1 && node->inputs->size == 20 &&
            context->tensors[node->inputs
                                 ->data[/*kInputToOutputWeightsTensor*/ 4]]
                    .type == kTfLiteFloat32) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            auto builtin = reinterpret_cast<TfLiteLSTMParams*>(
                mapping_args.node->builtin_data);
            mapping_args.builder->AddScalarInt32Operand(builtin->activation);
            mapping_args.builder->AddScalarFloat32Operand(builtin->cell_clip);
            mapping_args.builder->AddScalarFloat32Operand(builtin->proj_clip);

            // Current NNAPI implementation requires the sratch_buffer as
            // output.
            mapping_args.builder->AddAdditionalFloat32OutputTensor(2);

            // NNAPI need both state_in and state_out for cell_state and
            // output_state.
            int ann_index;
            mapping_args.builder->AddStateFloat32Tensor(
                mapping_args.node->inputs
                    ->data[/*kInputActivationStateTensor*/ 18],
                &ann_index);
            mapping_args.model_state_outputs->push_back(ann_index);
            mapping_args.model_state_tfl_inputs->push_back(
                mapping_args.node->inputs
                    ->data[/*kInputActivationStateTensor*/ 18]);
            mapping_args.builder->AddStateFloat32Tensor(
                mapping_args.node->inputs->data[/*kInputCellStateTensor*/ 19],
                &ann_index);
            mapping_args.model_state_outputs->push_back(ann_index);
            mapping_args.model_state_tfl_inputs->push_back(
                mapping_args.node->inputs->data[/*kInputCellStateTensor*/ 19]);

            return ANEURALNETWORKS_LSTM;
          };
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinMean:
/*
        // NNAPI does not support generating a scalar as output for MEAN.
        if (version == 1 && kAndroidSdkVersion >= kMinSdkVersionForNNAPI11 &&
            context->tensors[node->inputs->data[0]].type == kTfLiteFloat32 &&
            context->tensors[node->outputs->data[0]].dims->size > 0) {
*/
        if (version == 1) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            auto builtin = reinterpret_cast<TfLiteReducerParams*>(
                mapping_args.node->builtin_data);
            int32_t keep_dims = 0;
            if (builtin->keep_dims) keep_dims = 1;
            mapping_args.builder->AddScalarInt32Operand(keep_dims);
            return ANEURALNETWORKS_MEAN;
          };
        } else {
          return nullptr;
        }
      case kTfLiteBuiltinEmbeddingLookup:
        // NNAPI only support float32 values.
        if (version == 1 &&
            context->tensors[node->inputs->data[1]].type == kTfLiteFloat32) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            return ANEURALNETWORKS_EMBEDDING_LOOKUP;
          };
        } else {
          return nullptr;
        }
        break;
      case kTfLiteBuiltinHashtableLookup:
        // NNAPI only support float32 output.
        if (version == 1 &&
            context->tensors[node->outputs->data[0]].type == kTfLiteFloat32) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            return ANEURALNETWORKS_HASHTABLE_LOOKUP;
          };
        } else {
          return nullptr;
        }
        break;

      /// M: NeuroPilot {@
      case kTfLiteBuiltinResizeBilinear:
        if (version == 1) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            mapping_args.builder->PopbackInput();
            TfLiteTensor size_tensor =
              mapping_args.context->tensors[mapping_args.node->inputs->data[1]];
            TfLiteTensor output_tensor =
              mapping_args.context->tensors[mapping_args.node->inputs->data[0]];
            if (size_tensor.dims->size == 1 &&
                size_tensor.type == kTfLiteInt32 &&
                size_tensor.bytes / sizeof(int32_t) == 2) {
              mapping_args.builder->AddScalarInt32Operand(
                                                  size_tensor.data.i32[0]);
              mapping_args.builder->AddScalarInt32Operand(
                                                  size_tensor.data.i32[1]);
              TFLITE_MTK_LOG_INFO("Resize bilinear height=%d, width=%d from size tensor",
                    size_tensor.data.i32[0], size_tensor.data.i32[1]);
            } else if (output_tensor.dims->size == 4) {
              mapping_args.builder->AddScalarInt32Operand(
                                                  output_tensor.dims->data[1]);
              mapping_args.builder->AddScalarInt32Operand(
                                                  output_tensor.dims->data[2]);
              TFLITE_MTK_LOG_INFO("Resize bilinear height=%d, width=%d from output tensor",
                    output_tensor.dims->data[1], output_tensor.dims->data[2]);
            }
            // M: ALPS04682836: Add Custom Bilinear Uint8 @{
            if(kAndroidSdkVersion >= 29 && output_tensor.type == kTfLiteUInt8) {
                int32_t op_hash = mtk::Hash("ResizeBilinearMtk");
                mapping_args.builder->AddOemScalarStringOperand(op_hash,
                                                                "ResizeBilinearMtk");
                return ANEURALNETWORKS_OEM_OPERATION;
            } else {
                return ANEURALNETWORKS_RESIZE_BILINEAR;
            }
            // @}
          };
        } else {
          return nullptr;
        }
        break;

      case kTfLiteBuiltinBatchToSpaceNd:
        if (version == 1) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            // O1 NeuroPilot support BATCH_TO_SPACE_ND with 3 inputs.
            // Only change the input for P.
            if (kAndroidSdkVersion >= 28) {
              mapping_args.builder->PopbackInput();
            }
            return ANEURALNETWORKS_BATCH_TO_SPACE_ND;
          };
        } else {
          return nullptr;
        }
        break;

      case kTfLiteBuiltinPrelu:
        if (version == 1) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            // Set is_channel_shared to 0
            mapping_args.builder->AddScalarInt32Operand(0);
            int32_t op_hash = mtk::Hash("prelumtk");
            if (kUseOemScaler) {
              mapping_args.builder->AddOemScalarStringOperand(op_hash,
                                "prelumtk");
            } else {
              mapping_args.builder->AddScalarInt32Operand(op_hash);
            }
            return ANEURALNETWORKS_OEM_OPERATION;
          };
        } else {
          return nullptr;
        }
        break;

      case kTfLiteBuiltinMinimum:
        if (version == 1) {
          return [](const NNAPIOpMappingArgs& mapping_args)
                     -> ANeuralNetworksOperationType {
            int32_t op_hash = mtk::Hash("minimummtk");
            if (kUseOemScaler) {
              mapping_args.builder->AddOemScalarStringOperand(op_hash,
                                "minimummtk");
            } else {
              mapping_args.builder->AddScalarInt32Operand(op_hash);
            }
            return ANEURALNETWORKS_OEM_OPERATION;
          };
        } else {
          return nullptr;
        }
        break;

      case kTfLiteBuiltinCustom:
        if (registration->custom_name == nullptr) {
          TFLITE_MTK_LOG_ERROR(
            "Empty custom name form a kTfLiteBuiltinCustom OP");
          return nullptr;
        }
        if (version == 1) {
          if (strcmp(registration->custom_name, "MTK_OPT") == 0) {
            return [](const NNAPIOpMappingArgs& mapping_args)
                      -> ANeuralNetworksOperationType {
              return ANEURALNETWORKS_OEM_OPERATION;
            };
          } else {
            mtk::CustomOpHelper& helper = mtk::CustomOpHelper::GetInstance();
            mtk::CustomOpHelper::ParameterFunc custom_op_param_func =
              helper.GetParamFunc(registration->custom_name);
            mtk::CustomOpHelper::MtkExtOpParameterFunc mtk_op_param_func =
              helper.GetMtkExtOpParamFunc(registration->custom_name);
            if (mtk_op_param_func != nullptr) {
              // MTK OP
              if (strcmp(registration->custom_name,
                         "MTK_DEPTH_TO_SPACE") == 0) {
                return [](const NNAPIOpMappingArgs& mapping_args)
                          -> ANeuralNetworksOperationType {
                  uint32_t next_id =
                    mapping_args.builder->GetNextAnnOperandIdx();
                  mtk::CustomOpHelper::MtkExtOpParameterFunc mtk_op_param_func =
                    mapping_args.custom_op_helper.GetMtkExtOpParamFunc(
                      mapping_args.registration->custom_name);
                  mtk_op_param_func(mapping_args.builder->GetNnModel(),
                                    mapping_args.builder->GetAugmentedInputs(),
                                    next_id,
                                    mapping_args.node->user_data);
                  return ANEURALNETWORKS_DEPTH_TO_SPACE;
                };
              } else {
                return [](const NNAPIOpMappingArgs& mapping_args)
                          -> ANeuralNetworksOperationType {
                  uint32_t next_id =
                    mapping_args.builder->GetNextAnnOperandIdx();
                  mtk::CustomOpHelper::MtkExtOpParameterFunc mtk_op_param_func =
                    mapping_args.custom_op_helper.GetMtkExtOpParamFunc(
                      mapping_args.registration->custom_name);
                  int32_t op_hash =
                    mtk_op_param_func(mapping_args.builder->GetNnModel(),
                                    mapping_args.builder->GetAugmentedInputs(),
                                    next_id,
                                    mapping_args.node->user_data);
                  mapping_args.builder->UpdateAnnOperandIdx(next_id);
                  if (kUseOemScaler) {
                    mapping_args.builder->AddOemScalarStringOperand(op_hash,
                                      mapping_args.registration->custom_name);
                  } else {
                    mapping_args.builder->AddScalarInt32Operand(op_hash);
                  }
                  return ANEURALNETWORKS_OEM_OPERATION;
                };
              }
            } else if (custom_op_param_func != nullptr) {
              // 3rd party or customer customized OP
              return [](const NNAPIOpMappingArgs& mapping_args)
                        -> ANeuralNetworksOperationType {
                uint32_t next_id =
                  mapping_args.builder->GetNextAnnOperandIdx();
                mtk::CustomOpHelper::ParameterFunc custom_op_param_func =
                  mapping_args.custom_op_helper.GetParamFunc(
                    mapping_args.registration->custom_name);
                custom_op_param_func(mapping_args.node->user_data,
                                mapping_args.builder->GetNnModel(),
                                mapping_args.builder->GetAugmentedInputs(),
                                next_id);
                mapping_args.builder->UpdateAnnOperandIdx(next_id);
                int32_t op_hash =
                  mapping_args.custom_op_helper.GetCustomOpHash(
                    mapping_args.registration->custom_name);
                if (kUseOemScaler) {
#if 0
                  const char* full_name =
                    mapping_args.custom_op_helper.GetCustomOpFullName(
                      mapping_args.registration->custom_name);
#endif
                  mapping_args.builder->AddOemScalarStringOperand(op_hash,
                                    mapping_args.registration->custom_name);
                } else {
                  mapping_args.builder->AddScalarInt32Operand(op_hash);
                }
                return ANEURALNETWORKS_OEM_OPERATION;
              };
            }
          }
          return nullptr;
        } else {
          return nullptr;
        }
        break;
      /// M: NeuroPilot @}

      default:
        return nullptr;
    }
  }

  // Initialize the kernel (a NN model).
  TfLiteStatus Init(TfLiteContext* context,
                    const TfLiteDelegateParams* params) {
    for (auto node_index : TfLiteIntArrayView(params->nodes_to_replace)) {
      nodes_.push_back(node_index);
    }

    if (!nn_model_) {
      ANeuralNetworksModel* model;
      CHECK_NN(context, ANeuralNetworksModel_create(&model));
      nn_model_.reset(model);

      TF_LITE_ENSURE_STATUS(
          BuildGraph(context, params->input_tensors, params->output_tensors));
    }

    if (!nn_compilation_) {
      ANeuralNetworksCompilation* compilation;
      CHECK_NN(context, ANeuralNetworksCompilation_create(nn_model_.get(),
                                                          &compilation));
      CHECK_NN(context, ANeuralNetworksCompilation_finish(compilation));
      nn_compilation_.reset(compilation);
    }
    return kTfLiteOk;
  }

  TfLiteStatus Invoke(TfLiteContext* context, TfLiteNode* node) {
    ANeuralNetworksExecution* execution = nullptr;
    CHECK_NN(context, ANeuralNetworksExecution_create(nn_compilation_.get(),
                                                      &execution));

    // Set the input tensor buffers. Note: we access tflite tensors using
    // absolute indices but NN api indices inputs by relative indices.
    int relative_input_index = 0;
#ifdef USE_ASHMEM
    size_t input_offset = 0;
#endif
    for (auto absolute_input_index : TfLiteIntArrayView(node->inputs)) {
      if (absolute_input_index == kOptionalTensor) {
        continue;
      }
      TfLiteTensor* tensor = &context->tensors[absolute_input_index];

      // TODO(miaowang): make sure the delegation works with dequantized weights
      // as intermediate tensors.
      if (tensor->allocation_type != kTfLiteMmapRo) {
#ifdef USE_ASHMEM
        // copy data to pre-allocated shared memory.
        memcpy(nn_input_memory_->get_data_ptr() + input_offset,
               tensor->data.raw, tensor->bytes);
        CHECK_NN(context, ANeuralNetworksExecution_setInputFromMemory(
                              execution, relative_input_index, nullptr,
                              nn_input_memory_->get_handle(), input_offset,
                              tensor->bytes));
        input_offset += tensor->bytes;
#else
        CHECK_NN(context, ANeuralNetworksExecution_setInput(
                              execution, relative_input_index, nullptr,
                              tensor->data.raw, tensor->bytes));
#endif
        relative_input_index++;
      }
    }

    // Set the output tensor buffers.
    int relative_output_index = 0;
#ifdef USE_ASHMEM
    size_t output_offset = 0;
#endif
    for (auto output_index : TfLiteIntArrayView(node->outputs)) {
      TfLiteTensor* tensor = &context->tensors[output_index];
#ifdef USE_ASHMEM
      CHECK_NN(context, ANeuralNetworksExecution_setOutputFromMemory(
                            execution, relative_output_index, nullptr,
                            nn_output_memory_->get_handle(), output_offset,
                            tensor->bytes));
      output_offset += tensor->bytes;
#else
      CHECK_NN(context, ANeuralNetworksExecution_setOutput(
                            execution, relative_output_index, nullptr,
                            tensor->data.raw, tensor->bytes));
#endif
      relative_output_index++;
    }

    // The state_out of previous invocation need to be mapped to state_in of
    // current invocation.
    for (size_t i = 0; i < model_state_tfl_inputs_.size(); i++) {
      int state_tensor_idx = model_state_tfl_inputs_[i];
      TfLiteTensor* tensor = &context->tensors[state_tensor_idx];
      // Here we are using a deep copy for state_in tensors so that we are not
      // reading and writing into the same buffer during a invocation.
      // TODO(110369471): using double shared buffer to minimize the copies.
      CHECK_NN(context, ANeuralNetworksExecution_setOutput(
                            execution, relative_output_index, nullptr,
                            tensor->data.raw, tensor->bytes));
      relative_output_index++;
    }
    // Invoke ANN in blocking fashion.
    ANeuralNetworksEvent* event = nullptr;
    CHECK_NN(context, ANeuralNetworksExecution_startCompute(execution, &event));
    CHECK_NN(context, ANeuralNetworksEvent_wait(event));
    ANeuralNetworksEvent_free(event);
    ANeuralNetworksExecution_free(execution);
#ifdef USE_ASHMEM
    // copy results from shared memory to the destination.
    output_offset = 0;
    for (auto output_index : TfLiteIntArrayView(node->outputs)) {
      TfLiteTensor* tensor = &context->tensors[output_index];
      memcpy(tensor->data.raw,
             nn_output_memory_->get_data_ptr() + output_offset, tensor->bytes);
      output_offset += tensor->bytes;
    }
#endif
    return kTfLiteOk;
  }

  static mtk::CustomOpHelper::ParameterFunc GetParamFunc(std::string op_name) {
    return mtk::CustomOpHelper::GetInstance().GetParamFunc(op_name);
  }

  static mtk::CustomOpHelper::MtkExtOpParameterFunc GetMtkExtOpParamFunc(
                                                          std::string op_name) {
    return mtk::CustomOpHelper::GetInstance().GetMtkExtOpParamFunc(op_name);
  }

 private:
  // ANN API state.
  std::unique_ptr<ANeuralNetworksModel, NNFreeModel> nn_model_;
  std::unique_ptr<ANeuralNetworksCompilation, NNFreeCompilation>
      nn_compilation_;
  // Node indices that this delegate is responsible for. Indices here
  // indexes into the nodes array in the TfLiteContext.
  std::vector<int> nodes_;
  // Track indices we use
  OperandMapping operand_mapping_;

  std::vector<int> model_state_outputs_;
  std::vector<int> model_state_tfl_inputs_;

  std::unique_ptr<NNMemory> nn_input_memory_;
  std::unique_ptr<NNMemory> nn_output_memory_;

  /// M: NeuroPilot @{
  // List of the MTK OPs to be fused with other OPs
  std::vector<TfLiteNode> fusion_mtk_op_list;
  /// M: NeuroPilot @}

  TfLiteStatus AddOpsAndTensors(TfLiteContext* context) {
    // The operand builder allows creating a single op. We create it at this
    // reduced power position rather than in the for loop to avoid reallocating
    // the vectors.
    NNAPIOpBuilder builder(context, &operand_mapping_, nn_model_.get());
    // Add Tensors
    // allocate outside to avoid realloc
    for (auto node_index : nodes_) {
      // Obtain the op and registration.
      TfLiteNode* node;
      TfLiteRegistration* reg;
      context->GetNodeAndRegistration(context, node_index, &node, &reg);

      /// M: NeuroPilot @{
      TFLITE_MTK_LOG_INFO("Process builtin OP(%d):%s", reg->builtin_code,
        EnumNameBuiltinOperator(
          static_cast<BuiltinOperator>(reg->builtin_code)));
      TFLITE_MTK_LOG_INFO("Num of input:%d", node->inputs->size);
      for (int l = 0; l <node->inputs->size; l++) {
        TFLITE_MTK_LOG_INFO("Input(%d)'s index: %d", l, node->inputs->data[l]);
      }
      TFLITE_MTK_LOG_INFO("Num of output:%d", node->outputs->size);
      for (int m = 0; m <node->outputs->size; m++) {
        TFLITE_MTK_LOG_INFO("Output(%d)'s index: %d", m, node->outputs->data[m]);
      }

      bool skip = false;
      if (static_cast<TfLiteBuiltinOperator>(reg->builtin_code) ==
          kTfLiteBuiltinCustom &&
          reg->custom_name != nullptr &&
          strcmp(reg->custom_name, "MTK_OPT") == 0) {
        for (size_t idx = node_index + 1; idx < nodes_.size(); idx++) {
          TfLiteNode* target_node;
          TfLiteRegistration* target_reg;
          context->GetNodeAndRegistration(context, idx,
                                          &target_node, &target_reg);
          // If the MTK OP is connected by multiple OPs
          // ==> The MTK OP node should be put into fusion list multiple times.
          for (auto m = 0; m < target_node->inputs->size; m++) {
            if (node->outputs->data[0] == target_node->inputs->data[m]) {
              fusion_mtk_op_list.push_back(*node);
            }
          }
        }
        skip = true;
      } else {
        if (!fusion_mtk_op_list.empty()) {
          bool erased = false;
          std::vector<TfLiteNode>::iterator it = fusion_mtk_op_list.begin();
          while (it != fusion_mtk_op_list.end()) {
            erased = false;
            for (auto j = 0; j < node->inputs->size; j++) {
              if (it->outputs->data[0] == node->inputs->data[j]) {
                TFLITE_MTK_LOG_INFO("Change builtin OP(%d):%s's input index:%d to index:%d",
                    reg->builtin_code,
                    EnumNameBuiltinOperator(
                      static_cast<BuiltinOperator>(reg->builtin_code)),
                    node->inputs->data[j],
                    it->inputs->data[0]);
                // Set the input tensor of mtk op as the input tensor of conv/pool/concatenation
                node->inputs->data[j] = it->inputs->data[0];
                it = fusion_mtk_op_list.erase(it);
                erased = true;
              }
            }
            if (!erased) {
              ++it;
            }
          }
        }
      }

      if (skip) {
        continue;
      }
      /// M: NeuroPilot @}

      // Map inputs to NN API tensor indices.
      for (auto input_index : TfLiteIntArrayView(node->inputs)) {
        if (input_index == kOptionalTensor &&
            (reg->builtin_code == kTfLiteBuiltinLstm ||
             reg->builtin_code == kTfLiteBuiltinSvdf)) {
          // properly handle the optional tensor for LSTM and SVDF.
          // currently only support float32.
          // TODO(miaowang): make sure this is also able to handle quantized
          // tensor when supported by NNAPI.
          TF_LITE_ENSURE_STATUS(builder.AddVectorFloat32Operand(nullptr, 0));
        } else {
          TF_LITE_ENSURE_STATUS(builder.AddTensorInput(input_index));
        }
      }
      // Get op type and operands
      int nn_op_type = Map(context, reg->builtin_code, reg->version, node, reg)(
          {context, &builder, node, &model_state_outputs_,
           &model_state_tfl_inputs_, reg,
           mtk::CustomOpHelper::GetInstance()});
      // Map outputs to NN API tensor indices.
      for (auto output_index : TfLiteIntArrayView(node->outputs)) {
        TF_LITE_ENSURE_STATUS(builder.AddTensorOutput(output_index));
      }

      builder.FinalizeAddOperation(nn_op_type);
    }
    return kTfLiteOk;
  }

  TfLiteStatus BuildGraph(TfLiteContext* context,
                          const TfLiteIntArray* input_tensors,
                          const TfLiteIntArray* output_tensors) {
    // Build the ops and tensors.
    TF_LITE_ENSURE_STATUS(AddOpsAndTensors(context));
    // Map input and output tensor indices to ANN
    std::vector<uint32_t> inputs;
    inputs.reserve(input_tensors->size);
    std::vector<uint32_t> outputs;
    outputs.reserve(output_tensors->size);

    size_t total_input_byte_size = 0;
    // Make the TensorFlow lite inputs and outputs to ann_indices.
    for (int i : TfLiteIntArrayView(input_tensors)) {
      // Constant tensors are not NNAPI inputs.
      if (i != kOptionalTensor &&
          context->tensors[i].allocation_type != kTfLiteMmapRo) {
        inputs.push_back(operand_mapping_.lite_index_to_ann(i));
        total_input_byte_size += context->tensors[i].bytes;
      }
    }

    size_t total_output_byte_size = 0;
    for (int i : TfLiteIntArrayView(output_tensors)) {
      outputs.push_back(operand_mapping_.lite_index_to_ann(i));
      total_output_byte_size += context->tensors[i].bytes;
    }

    // Add state output tensors as model inputs
    for (int i : model_state_outputs_) {
      outputs.push_back(i);
    }

    // Tell ANN to declare inputs/outputs
    CHECK_NN(context, ANeuralNetworksModel_identifyInputsAndOutputs(
                          nn_model_.get(), inputs.size(), inputs.data(),
                          outputs.size(), outputs.data()));

    // Set relaxed computation mode for fp32 if possible.
    if (kAndroidSdkVersion >= kMinSdkVersionForNNAPI11) {
      CHECK_NN(context,
               ANeuralNetworksModel_relaxComputationFloat32toFloat16(
                   nn_model_.get(), context->allow_fp32_relax_to_fp16));
    }

    // Finalize the model
    CHECK_NN(context, ANeuralNetworksModel_finish(nn_model_.get()));
#ifdef USE_ASHMEM
    // Create shared memory pool for inputs and outputs.
    nn_input_memory_.reset(new NNMemory("input_pool", total_input_byte_size));
    nn_output_memory_.reset(
        new NNMemory("output_pool", total_output_byte_size));
#endif
    return kTfLiteOk;
  }
};

}  // namespace

MtkNnApiDelegateP::MtkNnApiDelegateP() {
  delegate_ = {
    .data_ = nullptr,
    .Prepare = [](TfLiteContext* context,
                  TfLiteDelegate* delegate) -> TfLiteStatus {
      // Do not check nodes_ if NN API is unavailable.
      if (kAndroidSdkVersion < kMinSdkVersionForNNAPI || !NNAPIExists()) {
        return kTfLiteOk;
      }

      std::vector<int> supported_nodes(1);
      // We don't care about all nodes_, we only care about ones in the
      // current plan.
      TfLiteIntArray* plan;
      TF_LITE_ENSURE_STATUS(context->GetExecutionPlan(context, &plan));
      int total_supported_nodes = 0;

      // Check for every node if it is supported
      // TODO(b/80625235): Fix this to do more careful checking of versioning.
      for (int node_index : TfLiteIntArrayView(plan)) {
        TfLiteNode* node;
        TfLiteRegistration* registration;
        TF_LITE_ENSURE_STATUS(context->GetNodeAndRegistration(
            context, node_index, &node, &registration));
        NNAPIDelegateKernel dummy_kernel;
        if (dummy_kernel.Map(context, registration->builtin_code,
                             registration->version, node, registration)) {
          supported_nodes.push_back(node_index);
        } else {
          context->ReportError(context,
            "Node at index: %d(%s)(version: %d) is not delegated to NNAPI\n",
            node_index,
            EnumNameBuiltinOperator(
              static_cast<BuiltinOperator>(registration->builtin_code)),
            registration->version);
        }
        total_supported_nodes += 1;
      }
      // Put the size at the beginning of the array.
      supported_nodes[0] = supported_nodes.size() - 1;

      // NN API Delegate Registration (the pseudo kernel that will invoke NN
      // API subgraphs)
      static const TfLiteRegistration nnapi_delegate_kernel = {
          .init = [](TfLiteContext* context, const char* buffer,
                     size_t length) -> void* {
            const TfLiteDelegateParams* params =
                reinterpret_cast<const TfLiteDelegateParams*>(buffer);
            NNAPIDelegateKernel* kernel_state = new NNAPIDelegateKernel;
            kernel_state->Init(context, params);
            return kernel_state;
          },

          .free = [](TfLiteContext* context, void* buffer) -> void {
            delete reinterpret_cast<NNAPIDelegateKernel*>(buffer);
          },

          .prepare = [](TfLiteContext* context,
                        TfLiteNode* node) -> TfLiteStatus {
            // Since the underlying resize happened ahead of delegation
            // worked. This does nothing.
            return kTfLiteOk;
          },

          .invoke = [](TfLiteContext* context,
                       TfLiteNode* node) -> TfLiteStatus {
            NNAPIDelegateKernel* state =
                reinterpret_cast<NNAPIDelegateKernel*>(node->user_data);
            return state->Invoke(context, node);
          },

          .builtin_code = kTfLiteBuiltinDelegate,
      };

      // Request TFLite to partition the graph and make kernels
      // for each independent subgraph a new nnapi_delegate_kernel.
      context->ReplaceNodeSubsetsWithDelegateKernels(
          context, nnapi_delegate_kernel,
          reinterpret_cast<TfLiteIntArray*>(supported_nodes.data()),
          delegate);
      return kTfLiteOk;
    }};
}

MtkNnApiDelegateP::~MtkNnApiDelegateP() {
}

}  // namespace tflite
