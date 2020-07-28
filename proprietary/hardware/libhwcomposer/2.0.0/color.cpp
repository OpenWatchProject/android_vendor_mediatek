#define DEBUG_LOG_TAG "COL"
#include "color.h"
#include "utils/debug.h"

ColorTransform::ColorTransform(const float* in_matrix, const int32_t& in_hint, const bool& in_dirty)
    : hint(in_hint) , dirty(in_dirty)
{
    matrix.resize(COLOR_MATRIX_DIM);
    for (auto& row : matrix)
    {
        row.resize(COLOR_MATRIX_DIM);
    }

    for (int32_t i = 0; i < COLOR_MATRIX_DIM; ++i)
    {
        for (int32_t j = 0; j < COLOR_MATRIX_DIM; ++j)
        {
            matrix[i][j] = in_matrix[i * COLOR_MATRIX_DIM + j];
        }
    }
}

ColorTransform::ColorTransform(
    const std::vector<std::vector<float> >& in_matrix,
    const int32_t& in_hint,
    const bool& in_dirty)
    : matrix(in_matrix), hint(in_hint) , dirty(in_dirty)
{}

ColorTransform::ColorTransform(const int32_t& in_hint, const bool& in_dirty)
    : hint(in_hint), dirty(in_dirty)
{
    switch (hint)
    {
        case HAL_COLOR_TRANSFORM_IDENTITY:
            matrix.resize(COLOR_MATRIX_DIM);
            for (auto& row : matrix)
            {
                row.resize(COLOR_MATRIX_DIM);
            }

            for (int32_t i = 0; i < COLOR_MATRIX_DIM; ++i)
            {
                matrix[i][i] = 1.f;
            }
            break;

        default:
            HWC_LOGE("Constructor of ColorTransform does not accept other hints exclude identity");
    }
}
