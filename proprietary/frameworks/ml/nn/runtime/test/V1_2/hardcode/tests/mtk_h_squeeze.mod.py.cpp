// clang-format off
// Hardcode file
#include "../../TestGenerated.h"

namespace mtk_h_squeeze {
// Generated mtk_h_squeeze test
#include "generated/examples/mtk_h_squeeze.example.cpp"
// Generated model constructor
#include "generated/models/mtk_h_squeeze.model.cpp"
} // namespace mtk_h_squeeze

TEST_F(GeneratedTests, mtk_h_squeeze_quant8_no_axis) {
    execute(mtk_h_squeeze::CreateModel_quant8_no_axis,
            mtk_h_squeeze::is_ignored,
            mtk_h_squeeze::get_examples_quant8_no_axis());
}
TEST_AVAILABLE_SINCE(V1_1, mtk_h_squeeze_quant8_no_axis, mtk_h_squeeze::CreateModel_quant8_no_axis)
