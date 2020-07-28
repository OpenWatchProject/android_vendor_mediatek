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

#define LOG_TAG "MtkSubgraph"
#include "tensorflow/lite/builtin_ops.h"
#include "tensorflow/lite/core/api/error_reporter.h"
#include "tensorflow/lite/mtk/core/mtk_subgraph.h"
#include "tensorflow/lite/mtk/mtk_helper.h"
#include "tensorflow/lite/mtk/mtk_minimal_logging.h"

#include <string>

namespace tflite {

MtkSubgraph::MtkSubgraph(ErrorReporter* error_reporter,
           TfLiteExternalContext** external_contexts,
           std::vector<std::unique_ptr<Subgraph>>* subgraphs,
           ResourceVariableMap* resource_variables) :
           Subgraph(error_reporter, external_contexts,
                    subgraphs, resource_variables) {
  TFLITE_MTK_LOG_INFO("MtkSubgraph()");
}

MtkSubgraph::~MtkSubgraph() {
}

void* MtkSubgraph::OpInit(const TfLiteRegistration& op_reg, const char* buffer,
                          size_t length) {
  // Let CustomOpHerper process the registration first.
  return Subgraph::OpInit(op_reg, buffer, length);
}

void MtkSubgraph::OpFree(const TfLiteRegistration& op_reg, void* buffer) {
  return Subgraph::OpFree(op_reg, buffer);
}

TfLiteStatus MtkSubgraph::OpPrepare(const TfLiteRegistration& op_reg,
                                    TfLiteNode* node) {
  TfLiteStatus status = kTfLiteError;
  if (op_reg.builtin_code == kTfLiteBuiltinCustom) {
    TFLITE_MTK_LOG_INFO("OpPrepare() %s", op_reg.custom_name);
    int rank;
    mtk::CustomOpHelper& helper = mtk::CustomOpHelper::GetInstance();
    for (int i = 0; i < node->outputs->size; i++) {
      status = helper.GetOutputRank(op_reg.custom_name, i, &rank);
      if (status != kTfLiteOk) {
        break;
      }
      int dims[rank];
      status = helper.GetOutputDims(op_reg.custom_name, i, dims);
      if (status != kTfLiteOk) {
        break;
      }
      // Resize the output tensor
      TfLiteTensor* output = &context_.tensors[node->outputs->data[i]];
      TfLiteIntArray* output_size = TfLiteIntArrayCreate(rank);
      for (int j = 0; j < rank; j++) {
        output_size->data[j] = dims[j];
      }
      context_.ResizeTensor(&context_, output, output_size);
    }
  }

  // Fallback to original OpPrepare
  if (status != kTfLiteOk) {
    status = Subgraph::OpPrepare(op_reg, node);
  }
  return status;
}

}  // namespace tflite
