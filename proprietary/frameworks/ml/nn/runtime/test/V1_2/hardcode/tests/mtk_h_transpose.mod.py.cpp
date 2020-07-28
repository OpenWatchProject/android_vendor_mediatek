// clang-format off
// Hardcode
#include "../../TestGenerated.h"

namespace mtk_h_transpose {
// Generated mtk_h_transpose test
#include "generated/examples/mtk_h_transpose.example.cpp"
// Generated model constructor
#include "generated/models/mtk_h_transpose.model.cpp"
} // namespace mtk_h_transpose

TEST_F(GeneratedTests, mtk_h_transpose_quant8_no_axis) {
    execute(mtk_h_transpose::CreateModel_quant8_no_axis,
            mtk_h_transpose::is_ignored,
            mtk_h_transpose::get_examples_quant8_no_axis());
}
TEST_AVAILABLE_SINCE(V1_1, mtk_h_transpose_quant8_no_axis, mtk_h_transpose::CreateModel_quant8_no_axis)

TEST_F(GeneratedTests, mtk_h_transpose_quant8_no_axis_simple) {
    execute(mtk_h_transpose::CreateModel_quant8_no_axis_simple,
            mtk_h_transpose::is_ignored,
            mtk_h_transpose::get_examples_quant8_no_axis_simple());
}
TEST_AVAILABLE_SINCE(V1_1, mtk_h_transpose_quant8_no_axis_simple, mtk_h_transpose::CreateModel_quant8_no_axis_simple)
