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

#ifndef MTK_TENSORFLOW_LITE_HELPER_H_
#define MTK_TENSORFLOW_LITE_HELPER_H_

#include "tensorflow/lite/c/c_api_internal.h"
#include "tensorflow/lite/nnapi/NeuralNetworksShim.h"

#include <stdlib.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <mutex>

// A union of pointers that points to memory for a given tensor.
typedef union {
  int32_t i32;
  int64_t i64;
  float f;
  // Placeholder for 16b float type. Use uint16* in the pointer union for now.
  TfLiteFloat16 f16;
  uint8_t uint8;
  bool b;
  int16_t i16;
  TfLiteComplex64 c64;
  int8_t int8;
} CustomOpParamUnion;

typedef struct {
  TfLiteType type;
  CustomOpParamUnion param;
} CustomOpParam;

namespace tflite {
namespace mtk {

class CustomOpHelper {
 public:
  /*========== Start of legacy API ==========*/
  typedef TfLiteStatus (*ParameterFunc)(void*, ANeuralNetworksModel*,
                                        std::vector<uint32_t>&, uint32_t&);
  typedef int32_t (*MtkExtOpParameterFunc)(ANeuralNetworksModel*,
                                        std::vector<uint32_t>&, uint32_t&, void*);
  static CustomOpHelper& GetInstance() {
    static CustomOpHelper instance; // Guaranteed to be destroyed.
                                    // Instantiated on first use.
    return instance;
  }

  CustomOpHelper(CustomOpHelper const&) = delete;
  void operator=(CustomOpHelper const&) = delete;

  void SetParamsFunc(const char* op_name, const char* target_name,
    const char* vendor_name, ParameterFunc add_params);

  ParameterFunc GetParamFunc(std::string op_name) {
    std::lock_guard<std::mutex> lock(index_addparams_mutex);
    std::unordered_map<std::string, ParameterFunc>::const_iterator got =
        index_addparams_.find(op_name);
    if (got != index_addparams_.end()) {
      return got->second;
    } else {
      return nullptr;
    }
  }

  void SetMtkExtOpParameterFunc(std::string op_name, MtkExtOpParameterFunc add_params) {
    std::lock_guard<std::mutex> lock(mtk_ext_op_addparams_mutex);
    mtk_ext_op_addparams_.insert(make_pair(op_name, add_params));
  }

  MtkExtOpParameterFunc GetMtkExtOpParamFunc(std::string op_name) {
    std::lock_guard<std::mutex> lock(mtk_ext_op_addparams_mutex);
    std::unordered_map<std::string, MtkExtOpParameterFunc>::const_iterator got =
        mtk_ext_op_addparams_.find(op_name);
    if (got != mtk_ext_op_addparams_.end()) {
      return got->second;
    } else {
      return nullptr;
    }
  }

  int32_t GetCustomOpHash(std::string op_name) {
    std::lock_guard<std::mutex> lock(index_hash_mutex);
    std::unordered_map<std::string, int32_t>::const_iterator got =
      index_hash_.find(op_name);
    if (got != index_hash_.end()) {
      return got->second;
    } else {
      return -1;
    }
  }

  const char* GetCustomOpFullName(std::string op_name) {
    std::lock_guard<std::mutex> lock(index_vendor_mutex);
    std::unordered_map<std::string, std::string>::const_iterator got =
      index_vendor_.find(op_name);
    if (got != index_vendor_.end()) {
      return got->second.c_str();
    } else {
      return nullptr;
    }
  }
  /*========== End of legacy API ==========*/

  TfLiteStatus SetOutputDims(const char* op_name, int output_idx,
                            int dims_size, int* dims);

  void RegisterTarget(const char* op_name, const char* target_name,
    const char* vendor_name) {
    std::string full_name = std::string(op_name) +
                            std::string(":") +
                            std::string(target_name) +
                            std::string(":") +
                            std::string(vendor_name);
    std::lock_guard<std::mutex> lock(index_vendor_mutex);
    index_vendor_.insert(std::make_pair(op_name, full_name));
  }

  const char* FindTarget(const char* op_name) {
    std::lock_guard<std::mutex> lock(index_vendor_mutex);
    std::unordered_map<std::string, std::string>::const_iterator got =
      index_vendor_.find(std::string(op_name));
    if (got != index_vendor_.end()) {
      return got->second.c_str();
    } else {
      return nullptr;
    }
  }

  TfLiteStatus GetOutputRank(const char* op_name, int output_idx, int* rank);

  TfLiteStatus GetOutputDims(const char* op_name, int output_idx, int* dims);

  TfLiteStatus AddParam(const char* op_name, TfLiteType type,
                        const void* param);

  std::vector<CustomOpParam>* GetParams(const char* op_name);

 private:
  CustomOpHelper() {}
  std::mutex index_addparams_mutex;
  std::mutex mtk_ext_op_addparams_mutex;
  std::mutex index_hash_mutex;
  std::mutex index_vendor_mutex;
  std::mutex index_output_dims_mutex;
  std::mutex index_params_mutex;
  std::unordered_map<std::string, ParameterFunc> index_addparams_;
  std::unordered_map<std::string, MtkExtOpParameterFunc> mtk_ext_op_addparams_;
  std::unordered_map<std::string, int32_t> index_hash_;  // Map custom op name with hash value
  std::unordered_map<std::string, std::string> index_vendor_;  // Map custom op name with vendor
  std::unordered_map<std::string,
      std::vector<std::vector<int>>> index_output_dims_;  // Map custom op name with output dimensions
  std::unordered_map<std::string, std::vector<CustomOpParam>> index_params_;  // Map custom op name with parameters
};

}  // namespace mtk
}  // namespace tflite

#endif  // MTK_TENSORFLOW_LITE_HELPER_H_
