#ifndef HWC_COLOR_H_
#define HWC_COLOR_H_

// In this file, including any file under libhwcompsoer folder is prohibited!!!
#include <vector>
#include <system/graphics.h>
#include <utils/RefBase.h>
#include <utils/LightRefBase.h>

struct ColorTransform : public android::LightRefBase<ColorTransform>
{
    ColorTransform(const float* in_matrix, const int32_t& in_hint, const bool& in_dirty);

    ColorTransform(const std::vector<std::vector<float> >& in_matrix, const int32_t& in_hint, const bool& in_dirty);

    ColorTransform(const int32_t& hint, const bool& in_dirty);

    std::vector<std::vector<float> > matrix;

    int32_t hint;

    bool dirty;
};

const int32_t COLOR_MATRIX_DIM = 4;
#endif // HWC_COLOR_H_
