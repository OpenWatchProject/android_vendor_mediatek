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

#define LOG_TAG "MtkHelper"
#include "tensorflow/lite/c/c_api_internal.h"
#include "tensorflow/lite/mtk/mtk_helper.h"
#include "tensorflow/lite/mtk/mtk_minimal_logging.h"
#include "tensorflow/lite/mtk/mtk_utils.h"

#include <string>
#include <vector>

namespace tflite {
namespace mtk {

void CustomOpHelper::SetParamsFunc(const char* op_name,
  const char* target_name,
  const char* vendor_name, ParameterFunc add_params) {
  TFLITE_MTK_LOG_INFO("SetParamsFunc, OP: %s, Vendor: %s",
                      op_name, vendor_name);
  std::string full_name = std::string(op_name) +
                          std::string(":") +
                          std::string(target_name) +
                          std::string(":") +
                          std::string(vendor_name);

  int32_t op_hash = mtk::Hash(full_name.c_str());
  TFLITE_MTK_LOG_INFO("Hash value in 32bit: %d", op_hash);
  {
      std::lock_guard<std::mutex> lock(index_addparams_mutex);
      index_addparams_.insert(std::make_pair(op_name, add_params));
  }
  {
      std::lock_guard<std::mutex> lock(index_hash_mutex);
      index_hash_.insert(std::make_pair(op_name, op_hash));
  }
  {
      std::lock_guard<std::mutex> lock(index_vendor_mutex);
      index_vendor_.insert(std::make_pair(op_name, full_name));
  }
}

TfLiteStatus CustomOpHelper::SetOutputDims(const char* op_name,
                                          int output_idx,
                                          int dims_size,
                                          int* dims) {
  TFLITE_MTK_LOG_INFO("SetOutputDims, OP: %s", op_name);
  // 1st element is the output index
  // 2nd element is the dimension size.
  // Followed with the dimensions from 3rd element
  std::vector<int> dims_data(dims, dims + dims_size);
  dims_data.emplace(dims_data.begin(), dims_size);
  dims_data.emplace(dims_data.begin(), output_idx);

  std::lock_guard<std::mutex> lock(index_output_dims_mutex);
  std::unordered_map<std::string,
                    std::vector<std::vector<int>>>::iterator got =
      index_output_dims_.find(std::string(op_name));
  if (got != index_output_dims_.end()) {
    got->second.push_back(dims_data);
  } else {
    std::vector<std::vector<int>> dims_array;
    dims_array.push_back(dims_data);
    index_output_dims_.insert(std::make_pair(op_name, dims_array));
  }
  return kTfLiteOk;
}

TfLiteStatus CustomOpHelper::GetOutputRank(const char* op_name,
                                          int output_idx,
                                          int* rank) {
  std::lock_guard<std::mutex> lock(index_output_dims_mutex);
  std::unordered_map<std::string,
                    std::vector<std::vector<int>>>::iterator got =
      index_output_dims_.find(std::string(op_name));
  if (got != index_output_dims_.end()) {
    for (std::vector<std::vector<int>>::iterator it = got->second.begin();
        it != got->second.end(); ++it) {
      if (it->at(0) == output_idx) {
        *rank = it->at(1);
        return kTfLiteOk;
      }
    }
  }
  return kTfLiteError;
}

TfLiteStatus CustomOpHelper::GetOutputDims(const char* op_name,
                                          int output_idx,
                                          int* dims) {
  std::lock_guard<std::mutex> lock(index_output_dims_mutex);
  std::unordered_map<std::string,
                    std::vector<std::vector<int>>>::iterator got =
      index_output_dims_.find(std::string(op_name));
  if (got != index_output_dims_.end()) {
    for (std::vector<std::vector<int>>::iterator it = got->second.begin();
        it != got->second.end(); ++it) {
      if (it->at(0) == output_idx) {
        for (auto i = 0; i < it->at(1); i++) {
          dims[i] = it->at(i + 2);
        }
        return kTfLiteOk;
      }
    }
  }
  return kTfLiteError;
}

TfLiteStatus CustomOpHelper::AddParam(const char* op_name,
                                      TfLiteType type, const void* param) {
  TFLITE_MTK_LOG_INFO("AddParam, OP: %s, type: %d", op_name, type);
  TfLiteStatus ret = kTfLiteOk;
  CustomOpParam op_param;
  op_param.type = type;
  switch (type) {
    case kTfLiteNoType:
      ret = kTfLiteError;
      break;
    case kTfLiteFloat32:
      op_param.param.f = *static_cast<const float*>(param);
      break;
    case kTfLiteInt32:
      op_param.param.i32 = *static_cast<const int32_t*>(param);
      break;
    case kTfLiteUInt8:
      op_param.param.uint8 = *static_cast<const uint8_t*>(param);
      break;
    case kTfLiteInt64:
      op_param.param.i64 = *static_cast<const int64_t*>(param);
      break;
    case kTfLiteString:
      ret = kTfLiteError;
      break;
    case kTfLiteBool:
      op_param.param.b = *static_cast<const bool*>(param);
      break;
    case kTfLiteInt16:
      op_param.param.i16 = *static_cast<const int16_t*>(param);
      break;
    case kTfLiteComplex64:
      op_param.param.c64 = *static_cast<const TfLiteComplex64*>(param);
      break;
    case kTfLiteInt8:
      op_param.param.int8 = *static_cast<const int8_t*>(param);
      break;
    case kTfLiteFloat16:
      op_param.param.f16 = *static_cast<const TfLiteFloat16*>(param);
      break;
    default:
      ret = kTfLiteError;
      break;
  }

  if (ret == kTfLiteOk) {
    std::lock_guard<std::mutex> lock(index_params_mutex);
    std::unordered_map<std::string, std::vector<CustomOpParam>>::iterator got =
        index_params_.find(std::string(op_name));
    if (got != index_params_.end()) {
      got->second.push_back(op_param);
    } else {
      std::vector<CustomOpParam> vec;
      vec.push_back(op_param);
      index_params_.insert(std::make_pair(std::string(op_name), vec));
    }
  }
  return ret;
}

std::vector<CustomOpParam>* CustomOpHelper::GetParams(const char* op_name) {
  TFLITE_MTK_LOG_INFO("GetParams, OP: %s", op_name);
  std::lock_guard<std::mutex> lock(index_params_mutex);
  std::unordered_map<std::string, std::vector<CustomOpParam>>::iterator got =
      index_params_.find(std::string(op_name));
  if (got != index_params_.end()) {
    return &(got->second);
  }
  return nullptr;
}
}  // namespace mtk

namespace ops {
namespace mtk {
namespace custom_op_helper {

void* Init(TfLiteContext* context, const char* buffer, size_t length) {
  return nullptr;
}

void Free(TfLiteContext* context, void* buffer) {
}

TfLiteStatus Prepare(TfLiteContext* context, TfLiteNode* node) {
  return kTfLiteOk;
}

}  // namespace custom_op_helper

TfLiteRegistration* Register_CUSTOM_OP_HELPER() {
  static TfLiteRegistration r = {custom_op_helper::Init,
                                 custom_op_helper::Free,
                                 custom_op_helper::Prepare,
                                 nullptr};
  return &r;
}

}  // namespace mtk
}  // namespace ops

}  // namespace tflite
