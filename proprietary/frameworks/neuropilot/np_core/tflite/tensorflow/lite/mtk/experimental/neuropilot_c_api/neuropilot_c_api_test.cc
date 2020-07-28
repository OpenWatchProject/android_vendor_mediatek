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

#include "tensorflow/lite/nnapi/NeuralNetworksShim.h"
#include "tensorflow/lite/mtk/experimental/neuropilot_c_api/NeuroPilotTFLiteShim.h"
#include "gtest/gtest.h"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <stdlib.h>

// Float model with single AvgPool OP
// Input [10, 784]
// Output [10, 14, 14, 1]
static constexpr int kInputRank = 2;
static constexpr int kInputDims[kInputRank] = {10, 784};
static constexpr size_t kInputSize = (10 * 784 * sizeof(float));
static constexpr int kOutputRank = 4;
static constexpr int kOutputDims[kOutputRank] = {10, 14, 14, 1};
static constexpr size_t kOutputSize = (10 * 14 * 14 * 1 * sizeof(float));
static constexpr char kAvgPoolModel[] = {
  0x18, 0x00, 0x00, 0x00, 0x54, 0x46, 0x4c, 0x33, 0x00, 0x00, 0x0e, 0x00, 0x18,
  0x00, 0x04, 0x00, 0x08, 0x00, 0x0c, 0x00, 0x10, 0x00, 0x14, 0x00, 0x0e, 0x00,
  0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x54, 0x01, 0x00, 0x00, 0x0c, 0x00, 0x00,
  0x00, 0x10, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x68, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x54, 0x4f, 0x43, 0x4f, 0x20,
  0x43, 0x6f, 0x6e, 0x76, 0x65, 0x72, 0x74, 0x65, 0x64, 0x2e, 0x00, 0x05, 0x00,
  0x00, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00,
  0x00, 0x0c, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x8c, 0xfd, 0xff, 0xff,
  0x90, 0xfd, 0xff, 0xff, 0x94, 0xfd, 0xff, 0xff, 0x1e, 0xff, 0xff, 0xff, 0x04,
  0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x1c, 0x00,
  0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xb4, 0xfd, 0xff,
  0xff, 0x0c, 0x00, 0x14, 0x00, 0x04, 0x00, 0x08, 0x00, 0x0c, 0x00, 0x10, 0x00,
  0x0c, 0x00, 0x00, 0x00, 0x0c, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0xf4,
  0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x70, 0x00,
  0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x14, 0x00, 0x00,
  0x00, 0x08, 0x00, 0x0c, 0x00, 0x07, 0x00, 0x10, 0x00, 0x0e, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x05, 0x0c, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x24,
  0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x16, 0x00, 0x00,
  0x00, 0x04, 0x00, 0x08, 0x00, 0x0c, 0x00, 0x10, 0x00, 0x0e, 0x00, 0x00, 0x00,
  0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x18, 0x00, 0x08, 0x00, 0x0c, 0x00,
  0x10, 0x00, 0x07, 0x00, 0x14, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x11, 0x01, 0x00, 0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00,
  0x20, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,
  0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x06, 0x00, 0x08, 0x00, 0x04, 0x00, 0x06, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00,
  0x00, 0x04, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0x1c, 0x00, 0x00, 0x00,
  0x1c, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x10,
  0x00, 0x00, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x00, 0x06, 0x00,
  0x05, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x01, 0x06, 0x00, 0x08, 0x00, 0x07,
  0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0x01, 0x00, 0x00, 0x00,
  0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04,
  0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0x00, 0xec, 0x00,
  0x00, 0x00, 0xa0, 0x00, 0x00, 0x00, 0x72, 0xff, 0xff, 0xff, 0x10, 0x00, 0x00,
  0x00, 0x04, 0x00, 0x00, 0x00, 0x14, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00,
  0x02, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x10, 0x03, 0x00, 0x00, 0x0b,
  0x00, 0x00, 0x00, 0x50, 0x6c, 0x61, 0x63, 0x65, 0x68, 0x6f, 0x6c, 0x64, 0x65,
  0x72, 0x00, 0x08, 0x00, 0x0c, 0x00, 0x04, 0x00, 0x08, 0x00, 0x08, 0x00, 0x00,
  0x00, 0x10, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x7f, 0x43, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc6,
  0xff, 0xff, 0xff, 0x10, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x1c, 0x00,
  0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00,
  0x00, 0x1c, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
  0x07, 0x00, 0x00, 0x00, 0x52, 0x65, 0x73, 0x68, 0x61, 0x70, 0x65, 0x00, 0xbc,
  0xff, 0xff, 0xff, 0x00, 0x00, 0x0e, 0x00, 0x14, 0x00, 0x04, 0x00, 0x00, 0x00,
  0x08, 0x00, 0x0c, 0x00, 0x10, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00,
  0x00, 0x02, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00,
  0x04, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x0e,
  0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x79, 0x73,
  0x00, 0x00, 0x04, 0x00, 0x06, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e,
  0x00, 0x18, 0x00, 0x08, 0x00, 0x07, 0x00, 0x0c, 0x00, 0x10, 0x00, 0x14, 0x00,
  0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x10, 0x00, 0x00, 0x00, 0x01,
  0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x01, 0x00,
  0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x0d, 0x00, 0x00, 0x00, 0x52, 0x65, 0x73,
  0x68, 0x61, 0x70, 0x65, 0x2f, 0x73, 0x68, 0x61, 0x70, 0x65, 0x00, 0x00, 0x00,
  0x04, 0x00, 0x04, 0x00, 0x04, 0x00, 0x00, 0x00
};

TEST(TFLiteCApiTest, NullInstance) {
  const char* model_path = "/data/local/tmp/model_test.lite";
  int ret = ANeuroPilotTFLiteWrapper_makeTFLite(nullptr, model_path);
  ASSERT_EQ(ANEURALNETWORKS_UNEXPECTED_NULL, ret);
}

TEST(TFLiteCApiTest, NullModelPath) {
  ANeuralNetworksTFLite* tflite = nullptr;
  int ret = ANeuroPilotTFLiteWrapper_makeTFLite(&tflite, nullptr);
  ASSERT_EQ(ANEURALNETWORKS_UNEXPECTED_NULL, ret);
}

TEST(TFLiteCApiTest, CreateNonExistentModel) {
  // Try to load a non-existent model.
  const char* model_path = "/data/local/tmp/model_test.lite";
  ANeuralNetworksTFLite* tflite = nullptr;
  int ret = ANeuroPilotTFLiteWrapper_makeTFLite(&tflite, model_path);
  ANeuroPilotTFLiteWrapper_free(tflite);
  ASSERT_EQ(ANEURALNETWORKS_BAD_DATA, ret);
}

TEST(TFLiteCApiTest, CreateWithBuffer) {
  ANeuralNetworksTFLite* tflite = nullptr;
  int ret =
      ANeuroPilotTFLiteWrapper_makeTFLiteWithBuffer(&tflite,
                                                    kAvgPoolModel,
                                                    sizeof(kAvgPoolModel));
  ANeuroPilotTFLiteWrapper_free(tflite);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);
}

TEST(TFLiteCApiTest, Create) {
  remove("/data/local/tmp/model.lite");
  std::ofstream tflite_file("/data/local/tmp/model.lite",
    std::ios::out | std::ios::app | std::ios::binary);
  ASSERT_TRUE(tflite_file.is_open());

  tflite_file.write(kAvgPoolModel, sizeof(kAvgPoolModel));
  tflite_file.close();

  ANeuralNetworksTFLite* tflite = nullptr;
  int ret = ANeuroPilotTFLiteWrapper_makeTFLite(&tflite,
                                                "/data/local/tmp/model.lite");
  ANeuroPilotTFLiteWrapper_free(tflite);

  remove("/data/local/tmp/model.lite");
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);
}

TEST(TFLiteCApiTest, GetTensorCount) {
  ANeuralNetworksTFLite* tflite = nullptr;
  int ret =
      ANeuroPilotTFLiteWrapper_makeTFLiteWithBuffer(&tflite,
                                                    kAvgPoolModel,
                                                    sizeof(kAvgPoolModel));
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);
  int32_t count = 0;
  ret = ANeuroPilotTFLiteWrapper_getTensorCount(tflite,
                                                TFLITE_BUFFER_TYPE_INPUT,
                                                &count);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);
  ASSERT_EQ(count, 1);
  ret = ANeuroPilotTFLiteWrapper_getTensorCount(tflite,
                                                TFLITE_BUFFER_TYPE_OUTPUT,
                                                &count);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);
  ASSERT_EQ(count, 1);
  ANeuroPilotTFLiteWrapper_free(tflite);
}

TEST(TFLiteCApiTest, GetInputTensor) {
  ANeuralNetworksTFLite* tflite = nullptr;
  int ret =
      ANeuroPilotTFLiteWrapper_makeTFLiteWithBuffer(&tflite,
                                                    kAvgPoolModel,
                                                    sizeof(kAvgPoolModel));
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);
  int rank = 0;
  ret = ANeuroPilotTFLiteWrapper_getTensorRank(tflite,
                                    TFLITE_BUFFER_TYPE_INPUT,
                                    0,
                                    &rank);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);
  ASSERT_EQ(rank, kInputRank);
  int diemssions[rank];
  ret = ANeuroPilotTFLiteWrapper_getTensorDimensions(tflite,
                                    TFLITE_BUFFER_TYPE_INPUT,
                                    0,
                                    diemssions);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);
  for (auto i = 0; i < rank; i++) {
    ASSERT_EQ(kInputDims[i], diemssions[i]);
  }
  size_t buffer_size = 0;
  ret = ANeuroPilotTFLiteWrapper_getTensorByteSize(tflite,
                                    TFLITE_BUFFER_TYPE_INPUT,
                                    0,
                                    &buffer_size);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);
  ASSERT_EQ(kInputSize, buffer_size);
  TFLiteTensorType tensr_type = TFLITE_TENSOR_TYPE_NONE;
  ret = ANeuroPilotTFLiteWrapper_getTensorType(tflite,
                                    TFLITE_BUFFER_TYPE_INPUT,
                                    0,
                                    &tensr_type);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);
  ASSERT_EQ(TFLITE_TENSOR_TYPE_FLOAT, tensr_type);
  ANeuroPilotTFLiteWrapper_free(tflite);
}

TEST(TFLiteCApiTest, GetOutputTensor) {
  ANeuralNetworksTFLite* tflite = nullptr;
  int ret =
      ANeuroPilotTFLiteWrapper_makeTFLiteWithBuffer(&tflite,
                                                    kAvgPoolModel,
                                                    sizeof(kAvgPoolModel));
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);
  int rank = 0;
  ret = ANeuroPilotTFLiteWrapper_getTensorRank(tflite,
                                    TFLITE_BUFFER_TYPE_OUTPUT,
                                    0,
                                    &rank);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);
  ASSERT_EQ(rank, kOutputRank);
  int diemssions[rank];
  ret = ANeuroPilotTFLiteWrapper_getTensorDimensions(tflite,
                                    TFLITE_BUFFER_TYPE_OUTPUT,
                                    0,
                                    diemssions);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);
  for (auto i = 0; i < rank; i++) {
    ASSERT_EQ(kOutputDims[i], diemssions[i]);
  }
  size_t buffer_size = 0;
  ret = ANeuroPilotTFLiteWrapper_getTensorByteSize(tflite,
                                    TFLITE_BUFFER_TYPE_OUTPUT,
                                    0,
                                    &buffer_size);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);
  ASSERT_EQ(kOutputSize, buffer_size);
  TFLiteTensorType tensr_type = TFLITE_TENSOR_TYPE_NONE;
  ret = ANeuroPilotTFLiteWrapper_getTensorType(tflite,
                                    TFLITE_BUFFER_TYPE_OUTPUT,
                                    0,
                                    &tensr_type);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);
  ASSERT_EQ(TFLITE_TENSOR_TYPE_FLOAT, tensr_type);
  ANeuroPilotTFLiteWrapper_free(tflite);
}

TEST(TFLiteCApiTest, Invoke) {
  ANeuralNetworksTFLite* tflite = nullptr;
  int ret =
      ANeuroPilotTFLiteWrapper_makeTFLiteWithBuffer(&tflite,
                                                    kAvgPoolModel,
                                                    sizeof(kAvgPoolModel));
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);

  size_t in_buffer_size = 0;
  ret = ANeuroPilotTFLiteWrapper_getTensorByteSize(tflite,
                                    TFLITE_BUFFER_TYPE_INPUT,
                                    0,
                                    &in_buffer_size);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);

  void* input_buffer = reinterpret_cast<void*>(calloc(1, in_buffer_size));
  float* p = reinterpret_cast<float*>(input_buffer);
  for (auto i = 0; i < in_buffer_size / sizeof(float); i++) {
    *p = rand() % 256;
    p++;
  }
  ret = ANeuroPilotTFLiteWrapper_setInputTensorData(tflite,
                                                    0,
                                                    input_buffer,
                                                    in_buffer_size);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);

  ret = ANeuroPilotTFLiteWrapper_invoke(tflite);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);

  int rank = 0;
  ret = ANeuroPilotTFLiteWrapper_getTensorRank(tflite,
                                    TFLITE_BUFFER_TYPE_OUTPUT,
                                    0,
                                    &rank);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);
  ASSERT_EQ(rank, kOutputRank);

  int diemssions[rank];
  ret = ANeuroPilotTFLiteWrapper_getTensorDimensions(tflite,
                                    TFLITE_BUFFER_TYPE_OUTPUT,
                                    0,
                                    diemssions);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);

  size_t out_buffer_size = 0;
  ret = ANeuroPilotTFLiteWrapper_getTensorByteSize(tflite,
                                    TFLITE_BUFFER_TYPE_OUTPUT,
                                    0,
                                    &out_buffer_size);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);

  void* output_buffer = reinterpret_cast<void*>(calloc(1, out_buffer_size));
  ASSERT_NE(output_buffer, nullptr);
  ret = ANeuroPilotTFLiteWrapper_getOutputTensorData(tflite,
                                                     0,
                                                     output_buffer,
                                                     out_buffer_size);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);
  if (input_buffer != nullptr) {
    free(input_buffer);
  }
  if (output_buffer != nullptr) {
    free(output_buffer);
  }

  ANeuroPilotTFLiteWrapper_free(tflite);
}

TEST(TFLiteCApiTest, ResizeInputTensor) {
  ANeuralNetworksTFLite* tflite = nullptr;
  ANeuralNetworksTFLiteOptions* options = nullptr;
  int ret = ANeuralNetworksTFLiteOptions_create(&options);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);

  static constexpr int dims[kInputRank] = {8, 784};
  ret = ANeuralNetworksTFLiteOptions_resizeInputTensor(options,
                                                      0,
                                                      dims,
                                                      kInputRank);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);
  ret = ANeuroPilotTFLiteWrapper_makeAdvTFLiteWithBuffer(&tflite,
                                                    kAvgPoolModel,
                                                    sizeof(kAvgPoolModel),
                                                    options);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);
  int rank = 0;
  ret = ANeuroPilotTFLiteWrapper_getTensorRank(tflite,
                                    TFLITE_BUFFER_TYPE_INPUT,
                                    0,
                                    &rank);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);
  ASSERT_EQ(rank, kInputRank);
  int diemssions[rank];
  ret = ANeuroPilotTFLiteWrapper_getTensorDimensions(tflite,
                                    TFLITE_BUFFER_TYPE_INPUT,
                                    0,
                                    diemssions);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);
  for (auto i = 0; i < rank; i++) {
    ASSERT_EQ(dims[i], diemssions[i]);
  }
  ANeuralNetworksTFLiteOptions_free(options);
  ANeuroPilotTFLiteWrapper_free(tflite);
}

TEST(TFLiteCApiTest, AccelerationModeCpu) {
  ANeuralNetworksTFLite* tflite = nullptr;
  ANeuralNetworksTFLiteOptions* options = nullptr;
  int ret = ANeuralNetworksTFLiteOptions_create(&options);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);

  ret = ANeuralNetworksTFLiteOptions_setAccelerationMode(options,
                                                         NP_ACCELERATION_CPU);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);

  ret = ANeuroPilotTFLiteWrapper_makeAdvTFLiteWithBuffer(
      &tflite, kAvgPoolModel, sizeof(kAvgPoolModel), options);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);

  ret = ANeuroPilotTFLiteWrapper_invoke(tflite);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);

  ANeuralNetworksTFLiteOptions_free(options);
  ANeuroPilotTFLiteWrapper_free(tflite);
}

TEST(TFLiteCApiTest, AccelerationModeNNAPI) {
  ANeuralNetworksTFLite* tflite = nullptr;
  ANeuralNetworksTFLiteOptions* options = nullptr;
  int ret = ANeuralNetworksTFLiteOptions_create(&options);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);

  ret = ANeuralNetworksTFLiteOptions_setAccelerationMode(options,
                                                         NP_ACCELERATION_NNAPI);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);

  ret = ANeuroPilotTFLiteWrapper_makeAdvTFLiteWithBuffer(
      &tflite, kAvgPoolModel, sizeof(kAvgPoolModel), options);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);

  ret = ANeuroPilotTFLiteWrapper_invoke(tflite);
  ASSERT_EQ(ANEURALNETWORKS_NO_ERROR, ret);

  ANeuralNetworksTFLiteOptions_free(options);
  ANeuroPilotTFLiteWrapper_free(tflite);
}

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  int ret = RUN_ALL_TESTS();
  return ret;
}
