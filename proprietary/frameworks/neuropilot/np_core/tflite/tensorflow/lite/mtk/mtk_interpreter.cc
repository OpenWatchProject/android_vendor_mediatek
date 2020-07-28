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

#define LOG_TAG "MtkInterpreter"
#include "tensorflow/lite/core/api/error_reporter.h"
#include "tensorflow/lite/mtk/delegates/nnapi/mtk_nnapi_delegate.h"
#include "tensorflow/lite/mtk/mtk_interpreter.h"
#include "tensorflow/lite/mtk/mtk_minimal_logging.h"
#include "tensorflow/lite/mtk/core/mtk_subgraph.h"
#include "tensorflow/lite/nnapi/NnModelEncryptionShim.h"

#include <string>

namespace tflite {

MtkInterpreter::MtkInterpreter(ErrorReporter* error_reporter) {
  TFLITE_LOG_PROD_ONCE(TFLITE_LOG_INFO,
                      "Initialized MTK TensorFlow Lite runtime.");
  error_reporter_ = error_reporter ? error_reporter : DefaultErrorReporter();
  // There's always at least 1 subgraph which is the primary subgraph.
  AddSubgraphs(1);
  context_ = primary_subgraph().context();

  // Reserve some space for the tensors to avoid excessive resizing.
  for (int i = 0; i < kTfLiteMaxExternalContexts; ++i) {
    external_contexts_[i] = nullptr;
  }

  // This operation is cheap because we allocate the CPU context resources (i.e.
  // threads) lazily.
  own_external_cpu_backend_context_.reset(new ExternalCpuBackendContext());
  external_contexts_[kTfLiteCpuBackendContext] =
      own_external_cpu_backend_context_.get();

  UseNNAPI(false);
}

MtkInterpreter::~MtkInterpreter() {
}

void MtkInterpreter::AddSubgraphs(int subgraphs_to_add,
                  int* first_new_subgraph_index) {
  TFLITE_MTK_LOG_INFO("AddSubgraphs()");
  const size_t base_index = subgraphs_.size();
  if (first_new_subgraph_index) *first_new_subgraph_index = base_index;

  subgraphs_.reserve(base_index + subgraphs_to_add);
  for (int i = 0; i < subgraphs_to_add; ++i) {
    Subgraph* subgraph =
        new MtkSubgraph(error_reporter_, external_contexts_, &subgraphs_,
                        &resource_variables_);
    subgraphs_.emplace_back(subgraph);
  }
}

void MtkInterpreter::SetCacheDir(const char* cache_dir) {
  if(!cache_dir) {
      TFLITE_MTK_LOG_INFO("cache directory is null");
      return;
  }
  cache_dir_ = cache_dir;
  for (auto& subgraph : subgraphs_) {
    subgraph->context()->cache_dir = cache_dir_.c_str();
  }
}

void MtkInterpreter::SetEncryptionLevel(int encryption_level) {
  if (encryption_level != 0 && encryption_level != ENCRYPTION_LEVEL_FASTEST &&
      encryption_level != ENCRYPTION_LEVEL_GOOD && encryption_level != ENCRYPTION_LEVEL_SAFEST) {
      TFLITE_MTK_LOG_INFO("input encryption level not defined");
      return;
  }
  for (auto& subgraph : subgraphs_) {
    subgraph->context()->encryption_level = encryption_level;
  }
}

}  // namespace tflite
