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
#include <cstdarg>
#include <gtest/gtest.h>
#include "flatbuffers/flexbuffers.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/kernels/test_util.h"
#include "tensorflow/lite/model.h"

namespace tflite {
namespace ops {
namespace mtk {

TfLiteRegistration* Register_MTK_OPT();

namespace {

using ::testing::ElementsAreArray;

class BaseMtkOptModel : public SingleOpModel {
 public:
  BaseMtkOptModel(const TensorData& input,
                 const TensorData& output) {
    input_ = AddInput(input);
    output_ = AddOutput(output);

    flexbuffers::Builder fbb;
    fbb.Map([&]() {
    });
    fbb.Finish();
    SetCustomOp("MTK_OPT", fbb.GetBuffer(), Register_MTK_OPT);

    BuildInterpreter({GetShape(input_)});
  }

  int input() { return input_; }

  std::vector<int> GetOutputShape() { return GetTensorShape(output_); }

 private:
  int input_;

 protected:
  int output_;
};

class MtkOptModel : public BaseMtkOptModel {
 public:
  using BaseMtkOptModel::BaseMtkOptModel;
  MtkOptModel(std::initializer_list<int> input_shape)
                         : BaseMtkOptModel(
                             {TensorType_FLOAT32, input_shape},
                             {TensorType_FLOAT32, {}})
                         {}

  std::vector<float> GetOutput() { return ExtractVector<float>(output_); }
};

class QuantizedMtkOptModel : public BaseMtkOptModel {
 public:
  using BaseMtkOptModel::BaseMtkOptModel;

  std::vector<float> GetDequantizedOutput() {
    return Dequantize<uint8_t>(ExtractVector<uint8_t>(output_),
                               GetScale(output_), GetZeroPoint(output_));
  }
};

// for quantized MtkTransposeConv
float GetTolerance(float min, float max) {
//  return 2 * (max - min) / 255.0;
  return 0.0;
}

// Test case:
// output = tf.nn.conv2d_backprop_input(
//     tf.constant([ 1, 4, 4, 1 ]),
//     tf.constant(np.arange(1, 10), shape=[ 3, 3, 1, 1 ], dtype=tf.float32),
//     tf.constant(np.arange(1, 17), shape=[ 1, 4, 4, 1 ], dtype=tf.float32),
//     [1, 1, 1, 1 ],
//     "SAME")
TEST(MtkOptModelTest, SimpleTest) {
  MtkOptModel m({1, 4, 4, 1});
  m.PopulateTensor<float>(
      m.input(), {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16});
  m.Invoke();

  EXPECT_THAT(m.GetOutput(),
              ElementsAreArray({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16}));
  EXPECT_THAT(m.GetOutputShape(), ElementsAreArray({1, 4, 4, 1}));
}

TEST(MtkOptModelTest, QuantizeSimpleTest) {
  float kQuantizedTolerance = GetTolerance(0, 255);
  QuantizedMtkOptModel m({TensorType_UINT8, {1, 4, 4, 1}, 0, 255},
                        {TensorType_UINT8, {}, 0, 255});
  m.QuantizeAndPopulate<uint8_t>(
      m.input(), {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16});
  m.Invoke();

  EXPECT_THAT(m.GetDequantizedOutput(),
              ElementsAreArray(
                ArrayFloatNear({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16},
                               kQuantizedTolerance)));
  EXPECT_THAT(m.GetOutputShape(), ElementsAreArray({1, 4, 4, 1}));
}


}  // namespace
}  // namespace mtk
}  // namespace ops
}  // namespace tflite

int main(int argc, char** argv) {
  ::tflite::LogToStderr();
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}