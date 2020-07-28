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

#include <cstdint>
#include <initializer_list>
#include <vector>

namespace armnn_wrapper {


struct ActivationDescriptor
{
    ActivationDescriptor() : m_Function(ActivationFunction::Sigmoid), m_A(0), m_B(0) {};

    ActivationFunction m_Function;
    float              m_A;
    float              m_B;
};

struct SoftmaxDescriptor
{
    SoftmaxDescriptor() : m_Beta(1.0f) {};

    float              m_Beta;
};

struct Pooling2dDescriptor
{
    Pooling2dDescriptor()
    : m_PoolType(PoolingAlgorithm::Max)
    , m_PadLeft(0)
    , m_PadRight(0)
    , m_PadTop(0)
    , m_PadBottom(0)
    , m_PoolWidth(0)
    , m_PoolHeight(0)
    , m_StrideX(0)
    , m_StrideY(0)
    , m_OutputShapeRounding(OutputShapeRounding::Floor)
    , m_PaddingMethod(PaddingMethod::Exclude)
    , m_DataLayout(DataLayout::NCHW)
    {};

    PoolingAlgorithm    m_PoolType;
    uint32_t            m_PadLeft;
    uint32_t            m_PadRight;
    uint32_t            m_PadTop;
    uint32_t            m_PadBottom;
    uint32_t            m_PoolWidth;
    uint32_t            m_PoolHeight;
    uint32_t            m_StrideX;
    uint32_t            m_StrideY;
    OutputShapeRounding m_OutputShapeRounding;
    PaddingMethod       m_PaddingMethod;
    DataLayout   m_DataLayout;
};

struct FullyConnectedDescriptor
{
    FullyConnectedDescriptor()
    : m_BiasEnabled(false)
    , m_TransposeWeightMatrix(false)
    {};

    bool m_BiasEnabled;
    bool m_TransposeWeightMatrix;
};

struct Convolution2dDescriptor
{
    Convolution2dDescriptor()
    : m_PadLeft(0)
    , m_PadRight(0)
    , m_PadTop(0)
    , m_PadBottom(0)
    , m_StrideX(0)
    , m_StrideY(0)
    , m_BiasEnabled(false)
    , m_DataLayout(DataLayout::NCHW)
    {};

    uint32_t             m_PadLeft;
    uint32_t             m_PadRight;
    uint32_t             m_PadTop;
    uint32_t             m_PadBottom;
    uint32_t             m_StrideX;
    uint32_t             m_StrideY;
    bool                 m_BiasEnabled;
    DataLayout           m_DataLayout;
};

struct DepthwiseConvolution2dDescriptor
{
    DepthwiseConvolution2dDescriptor()
    :   m_PadLeft(0)
    ,   m_PadRight(0)
    ,   m_PadTop(0)
    ,   m_PadBottom(0)
    ,   m_StrideX(0)
    ,   m_StrideY(0)
    ,   m_BiasEnabled(false)
    ,   m_DataLayout(DataLayout::NCHW)
    {}

    uint32_t   m_PadLeft;
    uint32_t   m_PadRight;
    uint32_t   m_PadTop;
    uint32_t   m_PadBottom;
    uint32_t   m_StrideX;
    uint32_t   m_StrideY;
    bool       m_BiasEnabled;
    DataLayout m_DataLayout;
};


struct NormalizationDescriptor
{
    NormalizationDescriptor()
    : m_NormChannelType(NormalizationAlgorithmChannel::Across)
    , m_NormMethodType(NormalizationAlgorithmMethod::LocalBrightness)
    , m_NormSize(0)
    , m_Alpha(0.f)
    , m_Beta(0.f)
    , m_K(0.f)
    , m_DataLayout(DataLayout::NCHW)
    {}

    NormalizationAlgorithmChannel m_NormChannelType;
    NormalizationAlgorithmMethod  m_NormMethodType;
    uint32_t                      m_NormSize;
    float                         m_Alpha;
    float                         m_Beta;
    float                         m_K;
    DataLayout                    m_DataLayout;
};

struct L2NormalizationDescriptor
{
    L2NormalizationDescriptor()
        : m_DataLayout(DataLayout::NCHW)
    {}

    DataLayout m_DataLayout;
};

struct BatchNormalizationDescriptor
{
    BatchNormalizationDescriptor()
    : m_Eps(0.0001f)
    , m_DataLayout(DataLayout::NCHW)
    {}

    float m_Eps;
    DataLayout m_DataLayout;
};

struct BatchToSpaceNdDescriptor
{
    BatchToSpaceNdDescriptor()
        : m_Crops({{0, 0}, {0, 0}})
        , m_DataLayout(DataLayout::NCHW)
    {}

    BatchToSpaceNdDescriptor(std::vector<unsigned int> blockShape,
                             std::vector<std::pair<unsigned int, unsigned int>> crops)
        : m_BlockShape(blockShape)
        , m_Crops(crops)
        , m_DataLayout(DataLayout::NCHW)
    {}

    std::vector<unsigned int> m_BlockShape;
    std::vector<std::pair<unsigned int, unsigned int>> m_Crops;
    DataLayout m_DataLayout;
};

struct FakeQuantizationDescriptor
{
    FakeQuantizationDescriptor()
    : m_Min(-6.0f)
    , m_Max(6.0f)
    {}

    float m_Min;
    float m_Max;
};

struct ResizeBilinearDescriptor
{
    ResizeBilinearDescriptor()
    : m_TargetWidth(0)
    , m_TargetHeight(0)
    , m_DataLayout(DataLayout::NCHW)
    {}

    uint32_t          m_TargetWidth;
    uint32_t          m_TargetHeight;
    DataLayout m_DataLayout;
};

struct ReshapeDescriptor
{
    ReshapeDescriptor()
    : m_TargetShape()
    {}

    ReshapeDescriptor(const uint32_t& dimension)
    : m_TargetShape(dimension)
    {}

    uint32_t m_TargetShape;
};

struct SpaceToBatchNdDescriptor
{
    SpaceToBatchNdDescriptor()
    : m_DataLayout(DataLayout::NCHW)
    {}

    SpaceToBatchNdDescriptor(const std::vector<unsigned int>& blockShape,
                             const std::vector<std::pair<unsigned int, unsigned int>>& padList)
    : m_BlockShape(blockShape)
    , m_PadList(padList)
    , m_DataLayout(DataLayout::NCHW)
    {}

    std::vector<unsigned int> m_BlockShape;
    std::vector<std::pair<unsigned int, unsigned int>> m_PadList;
    DataLayout m_DataLayout;
};

// temporary descriptor for Lstm
struct LstmDescriptor
{
    LstmDescriptor()
    : m_ActivationFunc(1) // 0: None, 1: Relu, 3: Relu6, 4: Tanh, 6: Sigmoid
    , m_ClippingThresCell(0.0)
    , m_ClippingThresProj(0.0)
    , m_CifgEnabled(true)
    , m_PeepholeEnabled(false)
    , m_ProjectionEnabled(false)
    {}

    uint32_t m_ActivationFunc;
    float m_ClippingThresCell;
    float m_ClippingThresProj;
    bool m_CifgEnabled;
    bool m_PeepholeEnabled;
    bool m_ProjectionEnabled;
};

struct MeanDescriptor
{
    MeanDescriptor()
    : m_Axis()
    , m_KeepDims(false)
    {}

    MeanDescriptor(const std::vector<unsigned int>& axis, bool keepDims)
    : m_Axis(axis)
    , m_KeepDims(keepDims)
    {}

    std::vector<unsigned int> m_Axis;
    bool m_KeepDims;
};

struct PadDescriptor
{
    PadDescriptor()
    {}

    PadDescriptor(const std::vector<std::pair<unsigned int, unsigned int>>& padList)
    : m_PadList(padList)
    {}

    // first is number of values to add before the tensor in the dimension,
    // second is the number of values to add after the tensor in the dimension
    // the number of pairs should match the number of dimensions in the input tensor.
    std::vector<std::pair<unsigned int, unsigned int>> m_PadList;
};

struct StridedSliceDescriptor
{
    StridedSliceDescriptor()
    : m_DataLayout(DataLayout::NCHW)
    {}

    StridedSliceDescriptor(const std::vector<int>& begin,
                           const std::vector<int>& end,
                           const std::vector<int>& stride)
    : m_Begin(begin)
    , m_End(end)
    , m_Stride(stride)
    , m_BeginMask(0)
    , m_EndMask(0)
    , m_ShrinkAxisMask(0)
    , m_EllipsisMask(0)
    , m_NewAxisMask(0)
    , m_DataLayout(DataLayout::NCHW)
    {}

    std::vector<int> m_Begin;
    std::vector<int> m_End;
    std::vector<int> m_Stride;

    int32_t m_BeginMask;
    int32_t m_EndMask;
    int32_t m_ShrinkAxisMask;
    int32_t m_EllipsisMask;
    int32_t m_NewAxisMask;

    DataLayout m_DataLayout;
};

}  // namespace armnn_wrapper