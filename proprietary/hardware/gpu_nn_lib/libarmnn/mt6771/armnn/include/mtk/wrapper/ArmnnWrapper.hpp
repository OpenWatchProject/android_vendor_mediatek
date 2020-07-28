/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include "Types.hpp"
#include "Descriptors.hpp"

namespace armnn_wrapper {

class ArmnnWrapper {
public:
    ArmnnWrapper();
    ~ArmnnWrapper();

private:
    class ArmnnImpl;
    ArmnnImpl *mImpl;

public:
    // Add OP Layers
    void AddInputLayer(TensorInfo& input, uint32_t& inputId);

    bool AddActivationLayer(TensorInfo& input, TensorInfo& output, ActivationDescriptor& desc);

    bool AddConv2dLayer(TensorInfo& input, TensorInfo& output, StaticData& weight,
                        StaticData& bias, Convolution2dDescriptor& desc);

    bool AddDwConv2dLayer(TensorInfo& input, TensorInfo& output, StaticData& weight,
                          StaticData& bias, DepthwiseConvolution2dDescriptor& desc);

    bool AddPoolingLayer(TensorInfo& input, TensorInfo& output, Pooling2dDescriptor& desc);

    bool AddReshapeLayer(TensorInfo& input, TensorInfo& output);

    bool AddSoftmaxLayer(TensorInfo& input, TensorInfo& output, SoftmaxDescriptor& desc);

    void AddOutputLayer(TensorInfo& output, uint32_t& outputId);

public:
    // Control ArmNN Flow
    void Optimize();

    bool LoadNetwork();

    void SetInput(uint32_t index, std::vector<float>& input);

    void SetOutput(uint32_t index, std::vector<float>& output);

    bool EnqueueWorkload();

    bool UnloadNetwork();
};

}  // namespace armnn_wrapper