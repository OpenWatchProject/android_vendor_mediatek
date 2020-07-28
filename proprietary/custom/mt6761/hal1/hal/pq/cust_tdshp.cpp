#include "cust_tdshp.h"

const DISPLAY_TDSHP_T tdshpindex =
{
.entry =
{

{//0
        0x18, //TDS_COR_GAIN       1500B000[07:00]
        0x20, //TDS_GAIN_HIGH      1500B000[15:08]
        0x10, //TDS_GAIN_MID       1500B000[23:16]
        0x0,  //TDS_INK_SEL        1500B000[25:24]
        0x0,  //TDS_BYPASS_HIGH    1500B000[29:29]
        0x0,  //TDS_BYPASS_MID     1500B000[30:30]
        0x0,  //TDS_EN             1500B000[31:31]
        0x8,  //TDS_LIMIT_RATIO    1500B004[03:00]
        0x30, //TDS_GAIN           1500B004[11:04]
        0x2,  //TDS_COR_ZERO       1500B004[23:16]
        0x4,  //TDS_COR_THR        1500B004[31:24]
        0x3,  //TDS_COR_VALUE      1500B008[15:08]
        0x40, //TDS_BOUND          1500B008[23:16]
        0x20, //TDS_LIMIT          1500B008[31:24]
        0x8,  //TDS_SAT_PROC       1500B00C[05:00]
        0x2,  //TDS_AC_LPF_COE     1500B00C[11:08]
        0x8,  //TDS_CLIP_THR       1500B00C[23:16]
        0x4,  //TDS_CLIP_RATIO     1500B00C[28:24]
        0x1,  //TDS_CLIP_EN        1500B00C[31:31]
        0x48, //TDS_YLEV_P048      1500B014[07:00]
        0x30, //TDS_YLEV_P032      1500B014[15:08]
        0x18, //TDS_YLEV_P016      1500B014[23:16]
        0x0,  //TDS_YLEV_P000      1500B014[31:24]
        0x78, //TDS_YLEV_P112      1500B018[07:00]
        0x70, //TDS_YLEV_P096      1500B018[15:08]
        0x68, //TDS_YLEV_P080      1500B018[23:16]
        0x58, //TDS_YLEV_P064      1500B018[31:24]
        0x70, //TDS_YLEV_P176      1500B01C[07:00]
        0x76, //TDS_YLEV_P160      1500B01C[15:08]
        0x7B, //TDS_YLEV_P144      1500B01C[23:16]
        0x80, //TDS_YLEV_P128      1500B01C[31:24]
        0x5B, //TDS_YLEV_P240      1500B020[07:00]
        0x60, //TDS_YLEV_P224      1500B020[15:08]
        0x66, //TDS_YLEV_P208      1500B020[23:16]
        0x6B, //TDS_YLEV_P192      1500B020[31:24]
        0x1,  //TDS_YLEV_EN        1500B024[14:14]
        0x10, //TDS_YLEV_ALPHA     1500B024[21:16]
        0x56, //TDS_YLEV_P256      1500B024[31:24]

        0x0C, //PBC1_RADIUS_R      1500B040[05:00]
        0x0C, //PBC1_THETA_R       1500B040[11:06]
        0x27, //PBC1_RSLOPE_1      1500B040[21:12]
        0x30, //PBC1_GAIN          1500B040[29:22]
        0x0,  //PBC1_LPF_EN        1500B040[30:30]
        0x1,  //PBC1_EN            1500B040[31:31]
        0x10, //PBC1_LPF_GAIN      1500B044[05:00]
        0x55, //PBC1_TSLOPE        1500B044[15:06]
        0x1A, //PBC1_RADIUS_C      1500B044[23:16]
        0x7F, //PBC1_THETA_C       1500B044[31:24]
        0x20, //PBC1_EDGE_SLOPE    1500B048[05:00]
        0xC,  //PBC1_EDGE_THR      1500B048[13:08]
        0x0,  //PBC1_EDGE_EN       1500B048[14:14]
        0x1,  //PBC1_CONF_GAIN     1500B048[19:16]
        0x55, //PBC1_RSLOPE        1500B048[31:22]

        0x18, //PBC2_RADIUS_R      1500B04C[05:00]
        0x18, //PBC2_THETA_R       1500B04C[11:06]
        0x20, //PBC2_RSLOPE_1      1500B04C[21:12]
        0x10, //PBC2_GAIN          1500B04C[29:22]
        0x1,  //PBC2_LPF_EN        1500B04C[30:30]
        0x1,  //PBC2_EN            1500B04C[31:31]
        0x10, //PBC1_LPF_GAIN      1500B050[05:00]
        0x2B, //PBC2_TSLOPE        1500B050[15:06]
        0x20, //PBC2_RADIUS_C      1500B050[23:16]
        0x60, //PBC2_THETA_C       1500B050[31:24]
        0x20, //PBC2_EDGE_SLOPE    1500B054[05:00]
        0xC,  //PBC2_EDGE_THR      1500B054[13:08]
        0x1,  //PBC2_EDGE_EN       1500B054[14:14]
        0x5,  //PBC2_CONF_GAIN     1500B054[19:16]
        0x2B, //PBC2_RSLOPE        1500B054[31:22]

        0x20, //PBC3_RADIUS_R      1500B058[05:00]
        0x10, //PBC3_THETA_R       1500B058[11:06]
        0x11, //PBC3_RSLOPE_1      1500B058[21:12]
        0x30, //PBC3_GAIN          1500B058[29:22]
        0x0,  //PBC3_LPF_EN        1500B058[30:30]
        0x1,  //PBC3_EN            1500B058[31:31]
        0x10, //PBC3_LPF_GAIN      1500B05C[05:00]
        0x40, //PBC3_TSLOPE        1500B05C[15:06]
        0x3C, //PBC3_RADIUS_C      1500B05C[23:16]
        0xE0, //PBC3_THETA_C       1500B05C[31:24]
        0x20, //PBC3_EDGE_SLOPE    1500B060[05:00]
        0xC,  //PBC3_EDGE_THR      1500B060[13:08]
        0x0,  //PBC3_EDGE_EN       1500B060[14:14]
        0x1,  //PBC3_CONF_GAIN     1500B060[19:16]
        0x20,  //PBC3_RSLOPE       1500B060[31:22]

        /*TDSHP_2_0*/
        0x8,  //TDS_MID_LIMIT_RATIO
        0x0,  //TDS_MID_COR_ZERO
        0x0,  //TDS_MID_COR_THR
        0x08, //TDS_MID_COR_GAIN
        0x0,  //TDS_MID_COR_VALUE
        0x60, //TDS_MID_BOUND
        0x30, //TDS_MID_LIMIT
        0x8,  //TDS_HIGH_LIMIT_RATIO
        0x2,  //TDS_HIGH_COR_ZERO
        0x4,  //TDS_HIGH_COR_THR
        0x10, //TDS_HIGH_COR_GAIN
        0x3,  //TDS_HIGH_COR_VALUE
        0x60, //TDS_HIGH_BOUND
        0x30, //TDS_HIGH_LIMIT
        0x2,  //EDF_CLIP_RATIO_INC
        0x08, //EDF_EDGE_GAIN
        0x20, //EDF_DETAIL_GAIN
        0x02, //EDF_FLAT_GAIN
        0x1,  //EDF_GAIN_EN
        0xE0, //EDF_EDGE_TH
        0xE0, //EDF_DETAIL_FALL_TH
        0x10, //EDF_DETAIL_RISE_TH
        0x10, //EDF_FLAT_TH
        0xA,  //EDF_EDGE_SLOPE
        0xA,  //EDF_DETAIL_FALL_SLOPE
        0x5,  //EDF_DETAIL_RISE_SLOPE
        0x4,  //EDF_FLAT_SLOPE
        0x7,  //EDF_EDGE_MONO_SLOPE
        0x30, //EDF_EDGE_MONO_TH
        0x8,  //EDF_EDGE_MAG_SLOPE
        0x0,  //EDF_EDGE_MAG_TH
        0x8,  //EDF_EDGE_TREND_FLAT_MAG
        0x10, //EDF_EDGE_TREND_SLOPE
        0x20, //EDF_EDGE_TREND_TH
        0x20, //EDF_BLD_WGT_MAG
        0x20, //EDF_BLD_WGT_MONO
        0x20, //EDF_BLD_WGT_TREND
        0xFF, //TDSHP_CBOOST_LMT_U
        0x80, //TDSHP_CBOOST_LMT_L
        0x01, //TDSHP_CBOOST_EN
        0x80, //TDSHP_CBOOST_GAIN
        0x10, //TDSHP_CBOOST_YCONST
        0x0,  //TDSHP_CBOOST_YOFFSET_SEL
        0x0,  //TDSHP_CBOOST_YOFFSET

        /*TDSHP_3_0*/
        0x48, //TDSHP_POST_YLEV_P048
        0x30, //TDSHP_POST_YLEV_P032
        0x18, //TDSHP_POST_YLEV_P016
        0x0,  //TDSHP_POST_YLEV_P000
        0x78, //TDSHP_POST_YLEV_P112
        0x70, //TDSHP_POST_YLEV_P096
        0x68, //TDSHP_POST_YLEV_P080
        0x60, //TDSHP_POST_YLEV_P064
        0x70, //TDSHP_POST_YLEV_P176
        0x76, //TDSHP_POST_YLEV_P160
        0x7B, //TDSHP_POST_YLEV_P144
        0x80, //TDSHP_POST_YLEV_P128
        0x5B, //TDSHP_POST_YLEV_P240
        0x60, //TDSHP_POST_YLEV_P224
        0x66, //TDSHP_POST_YLEV_P208
        0x6B, //TDSHP_POST_YLEV_P192
        0x0,  //TDSHP_POST_YLEV_EN
        0x10, //TDSHP_POST_YLEV_ALPHA
        0x56  //TDSHP_POST_YLEV_256
}
,
{//1
        0x18, //TDS_COR_GAIN       1500B000[07:00]
        0x10, //TDS_GAIN_HIGH      1500B000[15:08]
        0x0,  //TDS_GAIN_MID       1500B000[23:16]
        0x0,  //TDS_INK_SEL        1500B000[25:24]
        0x0,  //TDS_BYPASS_HIGH    1500B000[29:29]
        0x0,  //TDS_BYPASS_MID     1500B000[30:30]
        0x1,  //TDS_EN             1500B000[31:31]
        0x8,  //TDS_LIMIT_RATIO    1500B004[03:00]
        0x30, //TDS_GAIN           1500B004[11:04]
        0x2,  //TDS_COR_ZERO       1500B004[23:16]
        0x4,  //TDS_COR_THR        1500B004[31:24]
        0x3,  //TDS_COR_VALUE      1500B008[15:08]
        0x40, //TDS_BOUND          1500B008[23:16]
        0x20, //TDS_LIMIT          1500B008[31:24]
        0x8,  //TDS_SAT_PROC       1500B00C[05:00]
        0x2,  //TDS_AC_LPF_COE     1500B00C[11:08]
        0x8,  //TDS_CLIP_THR       1500B00C[23:16]
        0x4,  //TDS_CLIP_RATIO     1500B00C[28:24]
        0x1,  //TDS_CLIP_EN        1500B00C[31:31]
        0x48, //TDS_YLEV_P048      1500B014[07:00]
        0x30, //TDS_YLEV_P032      1500B014[15:08]
        0x18, //TDS_YLEV_P016      1500B014[23:16]
        0x0,  //TDS_YLEV_P000      1500B014[31:24]
        0x78, //TDS_YLEV_P112      1500B018[07:00]
        0x70, //TDS_YLEV_P096      1500B018[15:08]
        0x68, //TDS_YLEV_P080      1500B018[23:16]
        0x58, //TDS_YLEV_P064      1500B018[31:24]
        0x70, //TDS_YLEV_P176      1500B01C[07:00]
        0x76, //TDS_YLEV_P160      1500B01C[15:08]
        0x7B, //TDS_YLEV_P144      1500B01C[23:16]
        0x80, //TDS_YLEV_P128      1500B01C[31:24]
        0x5B, //TDS_YLEV_P240      1500B020[07:00]
        0x60, //TDS_YLEV_P224      1500B020[15:08]
        0x66, //TDS_YLEV_P208      1500B020[23:16]
        0x6B, //TDS_YLEV_P192      1500B020[31:24]
        0x1,  //TDS_YLEV_EN        1500B024[14:14]
        0x10, //TDS_YLEV_ALPHA     1500B024[21:16]
        0x56, //TDS_YLEV_P256      1500B024[31:24]

        0x0C, //PBC1_RADIUS_R      1500B040[05:00]
        0x0C, //PBC1_THETA_R       1500B040[11:06]
        0x27, //PBC1_RSLOPE_1      1500B040[21:12]
        0x30, //PBC1_GAIN          1500B040[29:22]
        0x0,  //PBC1_LPF_EN        1500B040[30:30]
        0x1,  //PBC1_EN            1500B040[31:31]
        0x10, //PBC1_LPF_GAIN      1500B044[05:00]
        0x55, //PBC1_TSLOPE        1500B044[15:06]
        0x1A, //PBC1_RADIUS_C      1500B044[23:16]
        0x7F, //PBC1_THETA_C       1500B044[31:24]
        0x20, //PBC1_EDGE_SLOPE    1500B048[05:00]
        0xC,  //PBC1_EDGE_THR      1500B048[13:08]
        0x0,  //PBC1_EDGE_EN       1500B048[14:14]
        0x1,  //PBC1_CONF_GAIN     1500B048[19:16]
        0x55, //PBC1_RSLOPE        1500B048[31:22]

        0x18, //PBC2_RADIUS_R      1500B04C[05:00]
        0x18, //PBC2_THETA_R       1500B04C[11:06]
        0x20, //PBC2_RSLOPE_1      1500B04C[21:12]
        0x10, //PBC2_GAIN          1500B04C[29:22]
        0x1,  //PBC2_LPF_EN        1500B04C[30:30]
        0x1,  //PBC2_EN            1500B04C[31:31]
        0x10, //PBC1_LPF_GAIN      1500B050[05:00]
        0x2B, //PBC2_TSLOPE        1500B050[15:06]
        0x20, //PBC2_RADIUS_C      1500B050[23:16]
        0x60, //PBC2_THETA_C       1500B050[31:24]
        0x20, //PBC2_EDGE_SLOPE    1500B054[05:00]
        0xC,  //PBC2_EDGE_THR      1500B054[13:08]
        0x1,  //PBC2_EDGE_EN       1500B054[14:14]
        0x5,  //PBC2_CONF_GAIN     1500B054[19:16]
        0x2B, //PBC2_RSLOPE        1500B054[31:22]

        0x20, //PBC3_RADIUS_R      1500B058[05:00]
        0x10, //PBC3_THETA_R       1500B058[11:06]
        0x11, //PBC3_RSLOPE_1      1500B058[21:12]
        0x30, //PBC3_GAIN          1500B058[29:22]
        0x0,  //PBC3_LPF_EN        1500B058[30:30]
        0x1,  //PBC3_EN            1500B058[31:31]
        0x10, //PBC3_LPF_GAIN      1500B05C[05:00]
        0x40, //PBC3_TSLOPE        1500B05C[15:06]
        0x3C, //PBC3_RADIUS_C      1500B05C[23:16]
        0xE0, //PBC3_THETA_C       1500B05C[31:24]
        0x20, //PBC3_EDGE_SLOPE    1500B060[05:00]
        0xC,  //PBC3_EDGE_THR      1500B060[13:08]
        0x0,  //PBC3_EDGE_EN       1500B060[14:14]
        0x1,  //PBC3_CONF_GAIN     1500B060[19:16]
        0x20, //PBC3_RSLOPE        1500B060[31:22]

        /*TDSHP_2_0*/
        0x8,  //TDS_MID_LIMIT_RATIO
        0x0,  //TDS_MID_COR_ZERO
        0x0,  //TDS_MID_COR_THR
        0x08, //TDS_MID_COR_GAIN
        0x0,  //TDS_MID_COR_VALUE
        0x60, //TDS_MID_BOUND
        0x30, //TDS_MID_LIMIT
        0x8,  //TDS_HIGH_LIMIT_RATIO
        0x2,  //TDS_HIGH_COR_ZERO
        0x4,  //TDS_HIGH_COR_THR
        0x10, //TDS_HIGH_COR_GAIN
        0x3,  //TDS_HIGH_COR_VALUE
        0x60, //TDS_HIGH_BOUND
        0x30, //TDS_HIGH_LIMIT
        0x2,  //EDF_CLIP_RATIO_INC
        0x08, //EDF_EDGE_GAIN
        0x20, //EDF_DETAIL_GAIN
        0x02, //EDF_FLAT_GAIN
        0x1,  //EDF_GAIN_EN
        0xE0, //EDF_EDGE_TH
        0xE0, //EDF_DETAIL_FALL_TH
        0x10, //EDF_DETAIL_RISE_TH
        0x10, //EDF_FLAT_TH
        0xA,  //EDF_EDGE_SLOPE
        0xA,  //EDF_DETAIL_FALL_SLOPE
        0x5,  //EDF_DETAIL_RISE_SLOPE
        0x4,  //EDF_FLAT_SLOPE
        0x7,  //EDF_EDGE_MONO_SLOPE
        0x30, //EDF_EDGE_MONO_TH
        0x8,  //EDF_EDGE_MAG_SLOPE
        0x0,  //EDF_EDGE_MAG_TH
        0x8,  //EDF_EDGE_TREND_FLAT_MAG
        0x10, //EDF_EDGE_TREND_SLOPE
        0x20, //EDF_EDGE_TREND_TH
        0x20, //EDF_BLD_WGT_MAG
        0x20, //EDF_BLD_WGT_MONO
        0x20, //EDF_BLD_WGT_TREND
        0xFF, //TDSHP_CBOOST_LMT_U
        0x80, //TDSHP_CBOOST_LMT_L
        0x01, //TDSHP_CBOOST_EN
        0x80, //TDSHP_CBOOST_GAIN
        0x10, //TDSHP_CBOOST_YCONST
        0x0,  //TDSHP_CBOOST_YOFFSET_SEL
        0x0,  //TDSHP_CBOOST_YOFFSET

        /*TDSHP_3_0*/
        0x48, //TDSHP_POST_YLEV_P048
        0x30, //TDSHP_POST_YLEV_P032
        0x18, //TDSHP_POST_YLEV_P016
        0x0,  //TDSHP_POST_YLEV_P000
        0x78, //TDSHP_POST_YLEV_P112
        0x70, //TDSHP_POST_YLEV_P096
        0x68, //TDSHP_POST_YLEV_P080
        0x60, //TDSHP_POST_YLEV_P064
        0x70, //TDSHP_POST_YLEV_P176
        0x76, //TDSHP_POST_YLEV_P160
        0x7B, //TDSHP_POST_YLEV_P144
        0x80, //TDSHP_POST_YLEV_P128
        0x5B, //TDSHP_POST_YLEV_P240
        0x60, //TDSHP_POST_YLEV_P224
        0x66, //TDSHP_POST_YLEV_P208
        0x6B, //TDSHP_POST_YLEV_P192
        0x0,  //TDSHP_POST_YLEV_EN
        0x10, //TDSHP_POST_YLEV_ALPHA
        0x56  //TDSHP_POST_YLEV_256
},
{//2
        0x18, //TDS_COR_GAIN       1500B000[07:00]
        0x23, //TDS_GAIN_HIGH      1500B000[15:08]
        0x13, //TDS_GAIN_MID       1500B000[23:16]
        0x0,  //TDS_INK_SEL        1500B000[25:24]
        0x0,  //TDS_BYPASS_HIGH    1500B000[29:29]
        0x0,  //TDS_BYPASS_MID     1500B000[30:30]
        0x1,  //TDS_EN             1500B000[31:31]
        0x8,  //TDS_LIMIT_RATIO    1500B004[03:00]
        0x30, //TDS_GAIN           1500B004[11:04]
        0x2,  //TDS_COR_ZERO       1500B004[23:16]
        0x4,  //TDS_COR_THR        1500B004[31:24]
        0x3,  //TDS_COR_VALUE      1500B008[15:08]
        0x40, //TDS_BOUND          1500B008[23:16]
        0x20, //TDS_LIMIT          1500B008[31:24]
        0x8,  //TDS_SAT_PROC       1500B00C[05:00]
        0x2,  //TDS_AC_LPF_COE     1500B00C[11:08]
        0x8,  //TDS_CLIP_THR       1500B00C[23:16]
        0x4,  //TDS_CLIP_RATIO     1500B00C[28:24]
        0x1,  //TDS_CLIP_EN        1500B00C[31:31]
        0x48, //TDS_YLEV_P048      1500B014[07:00]
        0x30, //TDS_YLEV_P032      1500B014[15:08]
        0x18, //TDS_YLEV_P016      1500B014[23:16]
        0x0,  //TDS_YLEV_P000      1500B014[31:24]
        0x78, //TDS_YLEV_P112      1500B018[07:00]
        0x70, //TDS_YLEV_P096      1500B018[15:08]
        0x68, //TDS_YLEV_P080      1500B018[23:16]
        0x58, //TDS_YLEV_P064      1500B018[31:24]
        0x70, //TDS_YLEV_P176      1500B01C[07:00]
        0x76, //TDS_YLEV_P160      1500B01C[15:08]
        0x7B, //TDS_YLEV_P144      1500B01C[23:16]
        0x80, //TDS_YLEV_P128      1500B01C[31:24]
        0x5B, //TDS_YLEV_P240      1500B020[07:00]
        0x60, //TDS_YLEV_P224      1500B020[15:08]
        0x66, //TDS_YLEV_P208      1500B020[23:16]
        0x6B, //TDS_YLEV_P192      1500B020[31:24]
        0x1,  //TDS_YLEV_EN        1500B024[14:14]
        0x10, //TDS_YLEV_ALPHA     1500B024[21:16]
        0x56, //TDS_YLEV_P256      1500B024[31:24]

        0x0C, //PBC1_RADIUS_R      1500B040[05:00]
        0x0C, //PBC1_THETA_R       1500B040[11:06]
        0x27, //PBC1_RSLOPE_1      1500B040[21:12]
        0x30, //PBC1_GAIN          1500B040[29:22]
        0x0,  //PBC1_LPF_EN        1500B040[30:30]
        0x1,  //PBC1_EN            1500B040[31:31]
        0x10, //PBC1_LPF_GAIN      1500B044[05:00]
        0x55, //PBC1_TSLOPE        1500B044[15:06]
        0x1A, //PBC1_RADIUS_C      1500B044[23:16]
        0x7F, //PBC1_THETA_C       1500B044[31:24]
        0x20, //PBC1_EDGE_SLOPE    1500B048[05:00]
        0xC,  //PBC1_EDGE_THR      1500B048[13:08]
        0x0,  //PBC1_EDGE_EN       1500B048[14:14]
        0x1,  //PBC1_CONF_GAIN     1500B048[19:16]
        0x55, //PBC1_RSLOPE        1500B048[31:22]

        0x18, //PBC2_RADIUS_R      1500B04C[05:00]
        0x18, //PBC2_THETA_R       1500B04C[11:06]
        0x20, //PBC2_RSLOPE_1      1500B04C[21:12]
        0x10, //PBC2_GAIN          1500B04C[29:22]
        0x1,  //PBC2_LPF_EN        1500B04C[30:30]
        0x1,  //PBC2_EN            1500B04C[31:31]
        0x10, //PBC1_LPF_GAIN      1500B050[05:00]
        0x2B, //PBC2_TSLOPE        1500B050[15:06]
        0x20, //PBC2_RADIUS_C      1500B050[23:16]
        0x60, //PBC2_THETA_C       1500B050[31:24]
        0x20, //PBC2_EDGE_SLOPE    1500B054[05:00]
        0xC,  //PBC2_EDGE_THR      1500B054[13:08]
        0x1,  //PBC2_EDGE_EN       1500B054[14:14]
        0x5,  //PBC2_CONF_GAIN     1500B054[19:16]
        0x2B, //PBC2_RSLOPE        1500B054[31:22]

        0x20, //PBC3_RADIUS_R      1500B058[05:00]
        0x10, //PBC3_THETA_R       1500B058[11:06]
        0x11, //PBC3_RSLOPE_1      1500B058[21:12]
        0x30, //PBC3_GAIN          1500B058[29:22]
        0x0,  //PBC3_LPF_EN        1500B058[30:30]
        0x1,  //PBC3_EN            1500B058[31:31]
        0x10, //PBC3_LPF_GAIN      1500B05C[05:00]
        0x40, //PBC3_TSLOPE        1500B05C[15:06]
        0x3C, //PBC3_RADIUS_C      1500B05C[23:16]
        0xE0, //PBC3_THETA_C       1500B05C[31:24]
        0x20, //PBC3_EDGE_SLOPE    1500B060[05:00]
        0xC,  //PBC3_EDGE_THR      1500B060[13:08]
        0x0,  //PBC3_EDGE_EN       1500B060[14:14]
        0x1,  //PBC3_CONF_GAIN     1500B060[19:16]
        0x20, //PBC3_RSLOPE        1500B060[31:22]

        /*TDSHP_2_0*/
        0x8,  //TDS_MID_LIMIT_RATIO
        0x0,  //TDS_MID_COR_ZERO
        0x0,  //TDS_MID_COR_THR
        0x08, //TDS_MID_COR_GAIN
        0x0,  //TDS_MID_COR_VALUE
        0x60, //TDS_MID_BOUND
        0x30, //TDS_MID_LIMIT
        0x8,  //TDS_HIGH_LIMIT_RATIO
        0x2,  //TDS_HIGH_COR_ZERO
        0x4,  //TDS_HIGH_COR_THR
        0x10, //TDS_HIGH_COR_GAIN
        0x3,  //TDS_HIGH_COR_VALUE
        0x60, //TDS_HIGH_BOUND
        0x30, //TDS_HIGH_LIMIT
        0x2,  //EDF_CLIP_RATIO_INC
        0x08, //EDF_EDGE_GAIN
        0x20, //EDF_DETAIL_GAIN
        0x02, //EDF_FLAT_GAIN
        0x1,  //EDF_GAIN_EN
        0xE0, //EDF_EDGE_TH
        0xE0, //EDF_DETAIL_FALL_TH
        0x10, //EDF_DETAIL_RISE_TH
        0x10, //EDF_FLAT_TH
        0xA,  //EDF_EDGE_SLOPE
        0xA,  //EDF_DETAIL_FALL_SLOPE
        0x5,  //EDF_DETAIL_RISE_SLOPE
        0x4,  //EDF_FLAT_SLOPE
        0x7,  //EDF_EDGE_MONO_SLOPE
        0x30, //EDF_EDGE_MONO_TH
        0x8,  //EDF_EDGE_MAG_SLOPE
        0x0,  //EDF_EDGE_MAG_TH
        0x8,  //EDF_EDGE_TREND_FLAT_MAG
        0x10, //EDF_EDGE_TREND_SLOPE
        0x20, //EDF_EDGE_TREND_TH
        0x20, //EDF_BLD_WGT_MAG
        0x20, //EDF_BLD_WGT_MONO
        0x20, //EDF_BLD_WGT_TREND
        0xFF, //TDSHP_CBOOST_LMT_U
        0x80, //TDSHP_CBOOST_LMT_L
        0x01, //TDSHP_CBOOST_EN
        0x80, //TDSHP_CBOOST_GAIN
        0x10, //TDSHP_CBOOST_YCONST
        0x0,  //TDSHP_CBOOST_YOFFSET_SEL
        0x0,  //TDSHP_CBOOST_YOFFSET

        /*TDSHP_3_0*/
        0x48, //TDSHP_POST_YLEV_P048
        0x30, //TDSHP_POST_YLEV_P032
        0x18, //TDSHP_POST_YLEV_P016
        0x0,  //TDSHP_POST_YLEV_P000
        0x78, //TDSHP_POST_YLEV_P112
        0x70, //TDSHP_POST_YLEV_P096
        0x68, //TDSHP_POST_YLEV_P080
        0x60, //TDSHP_POST_YLEV_P064
        0x70, //TDSHP_POST_YLEV_P176
        0x76, //TDSHP_POST_YLEV_P160
        0x7B, //TDSHP_POST_YLEV_P144
        0x80, //TDSHP_POST_YLEV_P128
        0x5B, //TDSHP_POST_YLEV_P240
        0x60, //TDSHP_POST_YLEV_P224
        0x66, //TDSHP_POST_YLEV_P208
        0x6B, //TDSHP_POST_YLEV_P192
        0x0,  //TDSHP_POST_YLEV_EN
        0x10, //TDSHP_POST_YLEV_ALPHA
        0x56  //TDSHP_POST_YLEV_256
}
,
{//3
        0x19, //TDS_COR_GAIN       1500B000[07:00]
        0x26, //TDS_GAIN_HIGH      1500B000[15:08]
        0x16, //TDS_GAIN_MID       1500B000[23:16]
        0x0,  //TDS_INK_SEL        1500B000[25:24]
        0x0,  //TDS_BYPASS_HIGH    1500B000[29:29]
        0x0,  //TDS_BYPASS_MID     1500B000[30:30]
        0x1,  //TDS_EN             1500B000[31:31]
        0x8,  //TDS_LIMIT_RATIO    1500B004[03:00]
        0x33, //TDS_GAIN           1500B004[11:04]
        0x2,  //TDS_COR_ZERO       1500B004[23:16]
        0x4,  //TDS_COR_THR        1500B004[31:24]
        0x3,  //TDS_COR_VALUE      1500B008[15:08]
        0x40, //TDS_BOUND          1500B008[23:16]
        0x20, //TDS_LIMIT          1500B008[31:24]
        0x8,  //TDS_SAT_PROC       1500B00C[05:00]
        0x2,  //TDS_AC_LPF_COE     1500B00C[11:08]
        0x8,  //TDS_CLIP_THR       1500B00C[23:16]
        0x4,  //TDS_CLIP_RATIO     1500B00C[28:24]
        0x1,  //TDS_CLIP_EN        1500B00C[31:31]
        0x48, //TDS_YLEV_P048      1500B014[07:00]
        0x30, //TDS_YLEV_P032      1500B014[15:08]
        0x18, //TDS_YLEV_P016      1500B014[23:16]
        0x0,  //TDS_YLEV_P000      1500B014[31:24]
        0x78, //TDS_YLEV_P112      1500B018[07:00]
        0x70, //TDS_YLEV_P096      1500B018[15:08]
        0x68, //TDS_YLEV_P080      1500B018[23:16]
        0x58, //TDS_YLEV_P064      1500B018[31:24]
        0x70, //TDS_YLEV_P176      1500B01C[07:00]
        0x76, //TDS_YLEV_P160      1500B01C[15:08]
        0x7B, //TDS_YLEV_P144      1500B01C[23:16]
        0x80, //TDS_YLEV_P128      1500B01C[31:24]
        0x5B, //TDS_YLEV_P240      1500B020[07:00]
        0x60, //TDS_YLEV_P224      1500B020[15:08]
        0x66, //TDS_YLEV_P208      1500B020[23:16]
        0x6B, //TDS_YLEV_P192      1500B020[31:24]
        0x1,  //TDS_YLEV_EN        1500B024[14:14]
        0x10, //TDS_YLEV_ALPHA     1500B024[21:16]
        0x56, //TDS_YLEV_P256      1500B024[31:24]

        0x0C, //PBC1_RADIUS_R      1500B040[05:00]
        0x0C, //PBC1_THETA_R       1500B040[11:06]
        0x27, //PBC1_RSLOPE_1      1500B040[21:12]
        0x30, //PBC1_GAIN          1500B040[29:22]
        0x0,  //PBC1_LPF_EN        1500B040[30:30]
        0x1,  //PBC1_EN            1500B040[31:31]
        0x10, //PBC1_LPF_GAIN      1500B044[05:00]
        0x55, //PBC1_TSLOPE        1500B044[15:06]
        0x1A, //PBC1_RADIUS_C      1500B044[23:16]
        0x7F, //PBC1_THETA_C       1500B044[31:24]
        0x20, //PBC1_EDGE_SLOPE    1500B048[05:00]
        0xC,  //PBC1_EDGE_THR      1500B048[13:08]
        0x0,  //PBC1_EDGE_EN       1500B048[14:14]
        0x1,  //PBC1_CONF_GAIN     1500B048[19:16]
        0x55, //PBC1_RSLOPE        1500B048[31:22]

        0x18, //PBC2_RADIUS_R      1500B04C[05:00]
        0x18, //PBC2_THETA_R       1500B04C[11:06]
        0x20, //PBC2_RSLOPE_1      1500B04C[21:12]
        0x10, //PBC2_GAIN          1500B04C[29:22]
        0x1,  //PBC2_LPF_EN        1500B04C[30:30]
        0x1,  //PBC2_EN            1500B04C[31:31]
        0x10, //PBC1_LPF_GAIN      1500B050[05:00]
        0x2B, //PBC2_TSLOPE        1500B050[15:06]
        0x20, //PBC2_RADIUS_C      1500B050[23:16]
        0x60, //PBC2_THETA_C       1500B050[31:24]
        0x20, //PBC2_EDGE_SLOPE    1500B054[05:00]
        0xC,  //PBC2_EDGE_THR      1500B054[13:08]
        0x1,  //PBC2_EDGE_EN       1500B054[14:14]
        0x5,  //PBC2_CONF_GAIN     1500B054[19:16]
        0x2B, //PBC2_RSLOPE        1500B054[31:22]

        0x20, //PBC3_RADIUS_R      1500B058[05:00]
        0x10, //PBC3_THETA_R       1500B058[11:06]
        0x11, //PBC3_RSLOPE_1      1500B058[21:12]
        0x30, //PBC3_GAIN          1500B058[29:22]
        0x0,  //PBC3_LPF_EN        1500B058[30:30]
        0x1,  //PBC3_EN            1500B058[31:31]
        0x10, //PBC3_LPF_GAIN      1500B05C[05:00]
        0x40, //PBC3_TSLOPE        1500B05C[15:06]
        0x3C, //PBC3_RADIUS_C      1500B05C[23:16]
        0xE0, //PBC3_THETA_C       1500B05C[31:24]
        0x20, //PBC3_EDGE_SLOPE    1500B060[05:00]
        0xC,  //PBC3_EDGE_THR      1500B060[13:08]
        0x0,  //PBC3_EDGE_EN       1500B060[14:14]
        0x1,  //PBC3_CONF_GAIN     1500B060[19:16]
        0x20, //PBC3_RSLOPE        1500B060[31:22]

        /*TDSHP_2_0*/
        0x8,  //TDS_MID_LIMIT_RATIO
        0x0,  //TDS_MID_COR_ZERO
        0x0,  //TDS_MID_COR_THR
        0x08, //TDS_MID_COR_GAIN
        0x0,  //TDS_MID_COR_VALUE
        0x60, //TDS_MID_BOUND
        0x30, //TDS_MID_LIMIT
        0x8,  //TDS_HIGH_LIMIT_RATIO
        0x2,  //TDS_HIGH_COR_ZERO
        0x4,  //TDS_HIGH_COR_THR
        0x10, //TDS_HIGH_COR_GAIN
        0x3,  //TDS_HIGH_COR_VALUE
        0x60, //TDS_HIGH_BOUND
        0x30, //TDS_HIGH_LIMIT
        0x2,  //EDF_CLIP_RATIO_INC
        0x08, //EDF_EDGE_GAIN
        0x20, //EDF_DETAIL_GAIN
        0x02, //EDF_FLAT_GAIN
        0x1,  //EDF_GAIN_EN
        0xE0, //EDF_EDGE_TH
        0xE0, //EDF_DETAIL_FALL_TH
        0x10, //EDF_DETAIL_RISE_TH
        0x10, //EDF_FLAT_TH
        0xA,  //EDF_EDGE_SLOPE
        0xA,  //EDF_DETAIL_FALL_SLOPE
        0x5,  //EDF_DETAIL_RISE_SLOPE
        0x4,  //EDF_FLAT_SLOPE
        0x7,  //EDF_EDGE_MONO_SLOPE
        0x30, //EDF_EDGE_MONO_TH
        0x8,  //EDF_EDGE_MAG_SLOPE
        0x0,  //EDF_EDGE_MAG_TH
        0x8,  //EDF_EDGE_TREND_FLAT_MAG
        0x10, //EDF_EDGE_TREND_SLOPE
        0x20, //EDF_EDGE_TREND_TH
        0x20, //EDF_BLD_WGT_MAG
        0x20, //EDF_BLD_WGT_MONO
        0x20, //EDF_BLD_WGT_TREND
        0xFF, //TDSHP_CBOOST_LMT_U
        0x80, //TDSHP_CBOOST_LMT_L
        0x01, //TDSHP_CBOOST_EN
        0x80, //TDSHP_CBOOST_GAIN
        0x10, //TDSHP_CBOOST_YCONST
        0x0,  //TDSHP_CBOOST_YOFFSET_SEL
        0x0,  //TDSHP_CBOOST_YOFFSET

        /*TDSHP_3_0*/
        0x48, //TDSHP_POST_YLEV_P048
        0x30, //TDSHP_POST_YLEV_P032
        0x18, //TDSHP_POST_YLEV_P016
        0x0,  //TDSHP_POST_YLEV_P000
        0x78, //TDSHP_POST_YLEV_P112
        0x70, //TDSHP_POST_YLEV_P096
        0x68, //TDSHP_POST_YLEV_P080
        0x60, //TDSHP_POST_YLEV_P064
        0x70, //TDSHP_POST_YLEV_P176
        0x76, //TDSHP_POST_YLEV_P160
        0x7B, //TDSHP_POST_YLEV_P144
        0x80, //TDSHP_POST_YLEV_P128
        0x5B, //TDSHP_POST_YLEV_P240
        0x60, //TDSHP_POST_YLEV_P224
        0x66, //TDSHP_POST_YLEV_P208
        0x6B, //TDSHP_POST_YLEV_P192
        0x0,  //TDSHP_POST_YLEV_EN
        0x10, //TDSHP_POST_YLEV_ALPHA
        0x56  //TDSHP_POST_YLEV_256
}
,
{//4
        0x22, //TDS_COR_GAIN       1500B000[07:00]
        0x30, //TDS_GAIN_HIGH      1500B000[15:08]
        0x20, //TDS_GAIN_MID       1500B000[23:16]
        0x0,  //TDS_INK_SEL        1500B000[25:24]
        0x0,  //TDS_BYPASS_HIGH    1500B000[29:29]
        0x0,  //TDS_BYPASS_MID     1500B000[30:30]
        0x1,  //TDS_EN             1500B000[31:31]
        0x8,  //TDS_LIMIT_RATIO    1500B004[03:00]
        0x43, //TDS_GAIN           1500B004[11:04]
        0x2,  //TDS_COR_ZERO       1500B004[23:16]
        0x4,  //TDS_COR_THR        1500B004[31:24]
        0x3,  //TDS_COR_VALUE      1500B008[15:08]
        0x40, //TDS_BOUND          1500B008[23:16]
        0x20, //TDS_LIMIT          1500B008[31:24]
        0x8,  //TDS_SAT_PROC       1500B00C[05:00]
        0x2,  //TDS_AC_LPF_COE     1500B00C[11:08]
        0x8,  //TDS_CLIP_THR       1500B00C[23:16]
        0x4,  //TDS_CLIP_RATIO     1500B00C[28:24]
        0x1,  //TDS_CLIP_EN        1500B00C[31:31]
        0x48, //TDS_YLEV_P048      1500B014[07:00]
        0x30, //TDS_YLEV_P032      1500B014[15:08]
        0x18, //TDS_YLEV_P016      1500B014[23:16]
        0x0,  //TDS_YLEV_P000      1500B014[31:24]
        0x78, //TDS_YLEV_P112      1500B018[07:00]
        0x70, //TDS_YLEV_P096      1500B018[15:08]
        0x68, //TDS_YLEV_P080      1500B018[23:16]
        0x58, //TDS_YLEV_P064      1500B018[31:24]
        0x70, //TDS_YLEV_P176      1500B01C[07:00]
        0x76, //TDS_YLEV_P160      1500B01C[15:08]
        0x7B, //TDS_YLEV_P144      1500B01C[23:16]
        0x80, //TDS_YLEV_P128      1500B01C[31:24]
        0x5B, //TDS_YLEV_P240      1500B020[07:00]
        0x60, //TDS_YLEV_P224      1500B020[15:08]
        0x66, //TDS_YLEV_P208      1500B020[23:16]
        0x6B, //TDS_YLEV_P192      1500B020[31:24]
        0x1,  //TDS_YLEV_EN        1500B024[14:14]
        0x10, //TDS_YLEV_ALPHA     1500B024[21:16]
        0x56, //TDS_YLEV_P256      1500B024[31:24]

        0x0C, //PBC1_RADIUS_R      1500B040[05:00]
        0x0C, //PBC1_THETA_R       1500B040[11:06]
        0x27, //PBC1_RSLOPE_1      1500B040[21:12]
        0x30, //PBC1_GAIN          1500B040[29:22]
        0x0,  //PBC1_LPF_EN        1500B040[30:30]
        0x1,  //PBC1_EN            1500B040[31:31]
        0x10, //PBC1_LPF_GAIN      1500B044[05:00]
        0x55, //PBC1_TSLOPE        1500B044[15:06]
        0x1A, //PBC1_RADIUS_C      1500B044[23:16]
        0x7F, //PBC1_THETA_C       1500B044[31:24]
        0x20, //PBC1_EDGE_SLOPE    1500B048[05:00]
        0xC,  //PBC1_EDGE_THR      1500B048[13:08]
        0x0,  //PBC1_EDGE_EN       1500B048[14:14]
        0x1,  //PBC1_CONF_GAIN     1500B048[19:16]
        0x55, //PBC1_RSLOPE        1500B048[31:22]

        0x18, //PBC2_RADIUS_R      1500B04C[05:00]
        0x18, //PBC2_THETA_R       1500B04C[11:06]
        0x20, //PBC2_RSLOPE_1      1500B04C[21:12]
        0x10, //PBC2_GAIN          1500B04C[29:22]
        0x1,  //PBC2_LPF_EN        1500B04C[30:30]
        0x1,  //PBC2_EN            1500B04C[31:31]
        0x10, //PBC1_LPF_GAIN      1500B050[05:00]
        0x2B, //PBC2_TSLOPE        1500B050[15:06]
        0x20, //PBC2_RADIUS_C      1500B050[23:16]
        0x60, //PBC2_THETA_C       1500B050[31:24]
        0x20, //PBC2_EDGE_SLOPE    1500B054[05:00]
        0xC,  //PBC2_EDGE_THR      1500B054[13:08]
        0x1,  //PBC2_EDGE_EN       1500B054[14:14]
        0x5,  //PBC2_CONF_GAIN     1500B054[19:16]
        0x2B, //PBC2_RSLOPE        1500B054[31:22]

        0x20, //PBC3_RADIUS_R      1500B058[05:00]
        0x10, //PBC3_THETA_R       1500B058[11:06]
        0x11, //PBC3_RSLOPE_1      1500B058[21:12]
        0x30, //PBC3_GAIN          1500B058[29:22]
        0x0,  //PBC3_LPF_EN        1500B058[30:30]
        0x1,  //PBC3_EN            1500B058[31:31]
        0x10, //PBC3_LPF_GAIN      1500B05C[05:00]
        0x40, //PBC3_TSLOPE        1500B05C[15:06]
        0x3C, //PBC3_RADIUS_C      1500B05C[23:16]
        0xE0, //PBC3_THETA_C       1500B05C[31:24]
        0x20, //PBC3_EDGE_SLOPE    1500B060[05:00]
        0xC,  //PBC3_EDGE_THR      1500B060[13:08]
        0x0,  //PBC3_EDGE_EN       1500B060[14:14]
        0x1,  //PBC3_CONF_GAIN     1500B060[19:16]
        0x20, //PBC3_RSLOPE        1500B060[31:22]

        /*TDSHP_2_0*/
        0x8,  //TDS_MID_LIMIT_RATIO
        0x0,  //TDS_MID_COR_ZERO
        0x0,  //TDS_MID_COR_THR
        0x08, //TDS_MID_COR_GAIN
        0x0,  //TDS_MID_COR_VALUE
        0x60, //TDS_MID_BOUND
        0x30, //TDS_MID_LIMIT
        0x8,  //TDS_HIGH_LIMIT_RATIO
        0x2,  //TDS_HIGH_COR_ZERO
        0x4,  //TDS_HIGH_COR_THR
        0x10, //TDS_HIGH_COR_GAIN
        0x3,  //TDS_HIGH_COR_VALUE
        0x60, //TDS_HIGH_BOUND
        0x30, //TDS_HIGH_LIMIT
        0x2,  //EDF_CLIP_RATIO_INC
        0x08, //EDF_EDGE_GAIN
        0x20, //EDF_DETAIL_GAIN
        0x02, //EDF_FLAT_GAIN
        0x1,  //EDF_GAIN_EN
        0xE0, //EDF_EDGE_TH
        0xE0, //EDF_DETAIL_FALL_TH
        0x10, //EDF_DETAIL_RISE_TH
        0x10, //EDF_FLAT_TH
        0xA,  //EDF_EDGE_SLOPE
        0xA,  //EDF_DETAIL_FALL_SLOPE
        0x5,  //EDF_DETAIL_RISE_SLOPE
        0x4,  //EDF_FLAT_SLOPE
        0x7,  //EDF_EDGE_MONO_SLOPE
        0x30, //EDF_EDGE_MONO_TH
        0x8,  //EDF_EDGE_MAG_SLOPE
        0x0,  //EDF_EDGE_MAG_TH
        0x8,  //EDF_EDGE_TREND_FLAT_MAG
        0x10, //EDF_EDGE_TREND_SLOPE
        0x20, //EDF_EDGE_TREND_TH
        0x20, //EDF_BLD_WGT_MAG
        0x20, //EDF_BLD_WGT_MONO
        0x20, //EDF_BLD_WGT_TREND
        0xFF, //TDSHP_CBOOST_LMT_U
        0x80, //TDSHP_CBOOST_LMT_L
        0x01, //TDSHP_CBOOST_EN
        0x80, //TDSHP_CBOOST_GAIN
        0x10, //TDSHP_CBOOST_YCONST
        0x0,  //TDSHP_CBOOST_YOFFSET_SEL
        0x0,  //TDSHP_CBOOST_YOFFSET

        /*TDSHP_3_0*/
        0x48, //TDSHP_POST_YLEV_P048
        0x30, //TDSHP_POST_YLEV_P032
        0x18, //TDSHP_POST_YLEV_P016
        0x0,  //TDSHP_POST_YLEV_P000
        0x78, //TDSHP_POST_YLEV_P112
        0x70, //TDSHP_POST_YLEV_P096
        0x68, //TDSHP_POST_YLEV_P080
        0x60, //TDSHP_POST_YLEV_P064
        0x70, //TDSHP_POST_YLEV_P176
        0x76, //TDSHP_POST_YLEV_P160
        0x7B, //TDSHP_POST_YLEV_P144
        0x80, //TDSHP_POST_YLEV_P128
        0x5B, //TDSHP_POST_YLEV_P240
        0x60, //TDSHP_POST_YLEV_P224
        0x66, //TDSHP_POST_YLEV_P208
        0x6B, //TDSHP_POST_YLEV_P192
        0x0,  //TDSHP_POST_YLEV_EN
        0x10, //TDSHP_POST_YLEV_ALPHA
        0x56  //TDSHP_POST_YLEV_256
}
,
{//5
        0x22, //TDS_COR_GAIN       1500B000[07:00]
        0x40, //TDS_GAIN_HIGH      1500B000[15:08]
        0x30, //TDS_GAIN_MID       1500B000[23:16]
        0x0,  //TDS_INK_SEL        1500B000[25:24]
        0x0,  //TDS_BYPASS_HIGH    1500B000[29:29]
        0x0,  //TDS_BYPASS_MID     1500B000[30:30]
        0x1,  //TDS_EN             1500B000[31:31]
        0x8,  //TDS_LIMIT_RATIO    1500B004[03:00]
        0x43, //TDS_GAIN           1500B004[11:04]
        0x2,  //TDS_COR_ZERO       1500B004[23:16]
        0x4,  //TDS_COR_THR        1500B004[31:24]
        0x3,  //TDS_COR_VALUE      1500B008[15:08]
        0x40, //TDS_BOUND          1500B008[23:16]
        0x20, //TDS_LIMIT          1500B008[31:24]
        0x8,  //TDS_SAT_PROC       1500B00C[05:00]
        0x2,  //TDS_AC_LPF_COE     1500B00C[11:08]
        0x8,  //TDS_CLIP_THR       1500B00C[23:16]
        0x4,  //TDS_CLIP_RATIO     1500B00C[28:24]
        0x1,  //TDS_CLIP_EN        1500B00C[31:31]
        0x48, //TDS_YLEV_P048      1500B014[07:00]
        0x30, //TDS_YLEV_P032      1500B014[15:08]
        0x18, //TDS_YLEV_P016      1500B014[23:16]
        0x0,  //TDS_YLEV_P000      1500B014[31:24]
        0x78, //TDS_YLEV_P112      1500B018[07:00]
        0x70, //TDS_YLEV_P096      1500B018[15:08]
        0x68, //TDS_YLEV_P080      1500B018[23:16]
        0x58, //TDS_YLEV_P064      1500B018[31:24]
        0x70, //TDS_YLEV_P176      1500B01C[07:00]
        0x76, //TDS_YLEV_P160      1500B01C[15:08]
        0x7B, //TDS_YLEV_P144      1500B01C[23:16]
        0x80, //TDS_YLEV_P128      1500B01C[31:24]
        0x5B, //TDS_YLEV_P240      1500B020[07:00]
        0x60, //TDS_YLEV_P224      1500B020[15:08]
        0x66, //TDS_YLEV_P208      1500B020[23:16]
        0x6B, //TDS_YLEV_P192      1500B020[31:24]
        0x1,  //TDS_YLEV_EN        1500B024[14:14]
        0x10, //TDS_YLEV_ALPHA     1500B024[21:16]
        0x56, //TDS_YLEV_P256      1500B024[31:24]

        0x0C, //PBC1_RADIUS_R      1500B040[05:00]
        0x0C, //PBC1_THETA_R       1500B040[11:06]
        0x27, //PBC1_RSLOPE_1      1500B040[21:12]
        0x30, //PBC1_GAIN          1500B040[29:22]
        0x0,  //PBC1_LPF_EN        1500B040[30:30]
        0x1,  //PBC1_EN            1500B040[31:31]
        0x10, //PBC1_LPF_GAIN      1500B044[05:00]
        0x55, //PBC1_TSLOPE        1500B044[15:06]
        0x1A, //PBC1_RADIUS_C      1500B044[23:16]
        0x7F, //PBC1_THETA_C       1500B044[31:24]
        0x20, //PBC1_EDGE_SLOPE    1500B048[05:00]
        0xC,  //PBC1_EDGE_THR      1500B048[13:08]
        0x0,  //PBC1_EDGE_EN       1500B048[14:14]
        0x1,  //PBC1_CONF_GAIN     1500B048[19:16]
        0x55, //PBC1_RSLOPE        1500B048[31:22]

        0x18, //PBC2_RADIUS_R      1500B04C[05:00]
        0x18, //PBC2_THETA_R       1500B04C[11:06]
        0x20, //PBC2_RSLOPE_1      1500B04C[21:12]
        0x10, //PBC2_GAIN          1500B04C[29:22]
        0x1,  //PBC2_LPF_EN        1500B04C[30:30]
        0x1,  //PBC2_EN            1500B04C[31:31]
        0x10, //PBC1_LPF_GAIN      1500B050[05:00]
        0x2B, //PBC2_TSLOPE        1500B050[15:06]
        0x20, //PBC2_RADIUS_C      1500B050[23:16]
        0x60, //PBC2_THETA_C       1500B050[31:24]
        0x20, //PBC2_EDGE_SLOPE    1500B054[05:00]
        0xC,  //PBC2_EDGE_THR      1500B054[13:08]
        0x1,  //PBC2_EDGE_EN       1500B054[14:14]
        0x5,  //PBC2_CONF_GAIN     1500B054[19:16]
        0x2B, //PBC2_RSLOPE        1500B054[31:22]

        0x20, //PBC3_RADIUS_R      1500B058[05:00]
        0x10, //PBC3_THETA_R       1500B058[11:06]
        0x11, //PBC3_RSLOPE_1      1500B058[21:12]
        0x30, //PBC3_GAIN          1500B058[29:22]
        0x0,  //PBC3_LPF_EN        1500B058[30:30]
        0x1,  //PBC3_EN            1500B058[31:31]
        0x10, //PBC3_LPF_GAIN      1500B05C[05:00]
        0x40, //PBC3_TSLOPE        1500B05C[15:06]
        0x3C, //PBC3_RADIUS_C      1500B05C[23:16]
        0xE0, //PBC3_THETA_C       1500B05C[31:24]
        0x20, //PBC3_EDGE_SLOPE    1500B060[05:00]
        0xC,  //PBC3_EDGE_THR      1500B060[13:08]
        0x0,  //PBC3_EDGE_EN       1500B060[14:14]
        0x1,  //PBC3_CONF_GAIN     1500B060[19:16]
        0x20, //PBC3_RSLOPE        1500B060[31:22]

        /*TDSHP_2_0*/
        0x8,  //TDS_MID_LIMIT_RATIO
        0x0,  //TDS_MID_COR_ZERO
        0x0,  //TDS_MID_COR_THR
        0x08, //TDS_MID_COR_GAIN
        0x0,  //TDS_MID_COR_VALUE
        0x60, //TDS_MID_BOUND
        0x30, //TDS_MID_LIMIT
        0x8,  //TDS_HIGH_LIMIT_RATIO
        0x2,  //TDS_HIGH_COR_ZERO
        0x4,  //TDS_HIGH_COR_THR
        0x10, //TDS_HIGH_COR_GAIN
        0x3,  //TDS_HIGH_COR_VALUE
        0x60, //TDS_HIGH_BOUND
        0x30, //TDS_HIGH_LIMIT
        0x2,  //EDF_CLIP_RATIO_INC
        0x08, //EDF_EDGE_GAIN
        0x20, //EDF_DETAIL_GAIN
        0x02, //EDF_FLAT_GAIN
        0x1,  //EDF_GAIN_EN
        0xE0, //EDF_EDGE_TH
        0xE0, //EDF_DETAIL_FALL_TH
        0x10, //EDF_DETAIL_RISE_TH
        0x10, //EDF_FLAT_TH
        0xA,  //EDF_EDGE_SLOPE
        0xA,  //EDF_DETAIL_FALL_SLOPE
        0x5,  //EDF_DETAIL_RISE_SLOPE
        0x4,  //EDF_FLAT_SLOPE
        0x7,  //EDF_EDGE_MONO_SLOPE
        0x30, //EDF_EDGE_MONO_TH
        0x8,  //EDF_EDGE_MAG_SLOPE
        0x0,  //EDF_EDGE_MAG_TH
        0x8,  //EDF_EDGE_TREND_FLAT_MAG
        0x10, //EDF_EDGE_TREND_SLOPE
        0x20, //EDF_EDGE_TREND_TH
        0x20, //EDF_BLD_WGT_MAG
        0x20, //EDF_BLD_WGT_MONO
        0x20, //EDF_BLD_WGT_TREND
        0xFF, //TDSHP_CBOOST_LMT_U
        0x80, //TDSHP_CBOOST_LMT_L
        0x01, //TDSHP_CBOOST_EN
        0x80, //TDSHP_CBOOST_GAIN
        0x10, //TDSHP_CBOOST_YCONST
        0x0,  //TDSHP_CBOOST_YOFFSET_SEL
        0x0,  //TDSHP_CBOOST_YOFFSET

        /*TDSHP_3_0*/
        0x48, //TDSHP_POST_YLEV_P048
        0x30, //TDSHP_POST_YLEV_P032
        0x18, //TDSHP_POST_YLEV_P016
        0x0,  //TDSHP_POST_YLEV_P000
        0x78, //TDSHP_POST_YLEV_P112
        0x70, //TDSHP_POST_YLEV_P096
        0x68, //TDSHP_POST_YLEV_P080
        0x60, //TDSHP_POST_YLEV_P064
        0x70, //TDSHP_POST_YLEV_P176
        0x76, //TDSHP_POST_YLEV_P160
        0x7B, //TDSHP_POST_YLEV_P144
        0x80, //TDSHP_POST_YLEV_P128
        0x5B, //TDSHP_POST_YLEV_P240
        0x60, //TDSHP_POST_YLEV_P224
        0x66, //TDSHP_POST_YLEV_P208
        0x6B, //TDSHP_POST_YLEV_P192
        0x0,  //TDSHP_POST_YLEV_EN
        0x10, //TDSHP_POST_YLEV_ALPHA
        0x56  //TDSHP_POST_YLEV_256
}
,
{//6
        0x2A, //TDS_COR_GAIN       1500B000[07:00]
        0x50, //TDS_GAIN_HIGH      1500B000[15:08]
        0x40, //TDS_GAIN_MID       1500B000[23:16]
        0x0,  //TDS_INK_SEL        1500B000[25:24]
        0x0,  //TDS_BYPASS_HIGH    1500B000[29:29]
        0x0,  //TDS_BYPASS_MID     1500B000[30:30]
        0x1,  //TDS_EN             1500B000[31:31]
        0x8,  //TDS_LIMIT_RATIO    1500B004[03:00]
        0x54, //TDS_GAIN           1500B004[11:04]
        0x2,  //TDS_COR_ZERO       1500B004[23:16]
        0x4,  //TDS_COR_THR        1500B004[31:24]
        0x3,  //TDS_COR_VALUE      1500B008[15:08]
        0x40, //TDS_BOUND          1500B008[23:16]
        0x20, //TDS_LIMIT          1500B008[31:24]
        0x8,  //TDS_SAT_PROC       1500B00C[05:00]
        0x2,  //TDS_AC_LPF_COE     1500B00C[11:08]
        0x8,  //TDS_CLIP_THR       1500B00C[23:16]
        0x4,  //TDS_CLIP_RATIO     1500B00C[28:24]
        0x1,  //TDS_CLIP_EN        1500B00C[31:31]
        0x48, //TDS_YLEV_P048      1500B014[07:00]
        0x30, //TDS_YLEV_P032      1500B014[15:08]
        0x18, //TDS_YLEV_P016      1500B014[23:16]
        0x0,  //TDS_YLEV_P000      1500B014[31:24]
        0x78, //TDS_YLEV_P112      1500B018[07:00]
        0x70, //TDS_YLEV_P096      1500B018[15:08]
        0x68, //TDS_YLEV_P080      1500B018[23:16]
        0x58, //TDS_YLEV_P064      1500B018[31:24]
        0x70, //TDS_YLEV_P176      1500B01C[07:00]
        0x76, //TDS_YLEV_P160      1500B01C[15:08]
        0x7B, //TDS_YLEV_P144      1500B01C[23:16]
        0x80, //TDS_YLEV_P128      1500B01C[31:24]
        0x5B, //TDS_YLEV_P240      1500B020[07:00]
        0x60, //TDS_YLEV_P224      1500B020[15:08]
        0x66, //TDS_YLEV_P208      1500B020[23:16]
        0x6B, //TDS_YLEV_P192      1500B020[31:24]
        0x1,  //TDS_YLEV_EN        1500B024[14:14]
        0x10, //TDS_YLEV_ALPHA     1500B024[21:16]
        0x56, //TDS_YLEV_P256      1500B024[31:24]

        0x0C, //PBC1_RADIUS_R      1500B040[05:00]
        0x0C, //PBC1_THETA_R       1500B040[11:06]
        0x27, //PBC1_RSLOPE_1      1500B040[21:12]
        0x30, //PBC1_GAIN          1500B040[29:22]
        0x0,  //PBC1_LPF_EN        1500B040[30:30]
        0x1,  //PBC1_EN            1500B040[31:31]
        0x10, //PBC1_LPF_GAIN      1500B044[05:00]
        0x55, //PBC1_TSLOPE        1500B044[15:06]
        0x1A, //PBC1_RADIUS_C      1500B044[23:16]
        0x7F, //PBC1_THETA_C       1500B044[31:24]
        0x20, //PBC1_EDGE_SLOPE    1500B048[05:00]
        0xC,  //PBC1_EDGE_THR      1500B048[13:08]
        0x0,  //PBC1_EDGE_EN       1500B048[14:14]
        0x1,  //PBC1_CONF_GAIN     1500B048[19:16]
        0x55, //PBC1_RSLOPE        1500B048[31:22]

        0x18, //PBC2_RADIUS_R      1500B04C[05:00]
        0x18, //PBC2_THETA_R       1500B04C[11:06]
        0x20, //PBC2_RSLOPE_1      1500B04C[21:12]
        0x10, //PBC2_GAIN          1500B04C[29:22]
        0x1,  //PBC2_LPF_EN        1500B04C[30:30]
        0x1,  //PBC2_EN            1500B04C[31:31]
        0x10, //PBC1_LPF_GAIN      1500B050[05:00]
        0x2B, //PBC2_TSLOPE        1500B050[15:06]
        0x20, //PBC2_RADIUS_C      1500B050[23:16]
        0x60, //PBC2_THETA_C       1500B050[31:24]
        0x20, //PBC2_EDGE_SLOPE    1500B054[05:00]
        0xC,  //PBC2_EDGE_THR      1500B054[13:08]
        0x1,  //PBC2_EDGE_EN       1500B054[14:14]
        0x5,  //PBC2_CONF_GAIN     1500B054[19:16]
        0x2B, //PBC2_RSLOPE        1500B054[31:22]

        0x20, //PBC3_RADIUS_R      1500B058[05:00]
        0x10, //PBC3_THETA_R       1500B058[11:06]
        0x11, //PBC3_RSLOPE_1      1500B058[21:12]
        0x30, //PBC3_GAIN          1500B058[29:22]
        0x0,  //PBC3_LPF_EN        1500B058[30:30]
        0x1,  //PBC3_EN            1500B058[31:31]
        0x10, //PBC3_LPF_GAIN      1500B05C[05:00]
        0x40, //PBC3_TSLOPE        1500B05C[15:06]
        0x3C, //PBC3_RADIUS_C      1500B05C[23:16]
        0xE0, //PBC3_THETA_C       1500B05C[31:24]
        0x20, //PBC3_EDGE_SLOPE    1500B060[05:00]
        0xC,  //PBC3_EDGE_THR      1500B060[13:08]
        0x0,  //PBC3_EDGE_EN       1500B060[14:14]
        0x1,  //PBC3_CONF_GAIN     1500B060[19:16]
        0x20, //PBC3_RSLOPE        1500B060[31:22]

        /*TDSHP_2_0*/
        0x8,  //TDS_MID_LIMIT_RATIO
        0x0,  //TDS_MID_COR_ZERO
        0x0,  //TDS_MID_COR_THR
        0x08, //TDS_MID_COR_GAIN
        0x0,  //TDS_MID_COR_VALUE
        0x60, //TDS_MID_BOUND
        0x30, //TDS_MID_LIMIT
        0x8,  //TDS_HIGH_LIMIT_RATIO
        0x2,  //TDS_HIGH_COR_ZERO
        0x4,  //TDS_HIGH_COR_THR
        0x10, //TDS_HIGH_COR_GAIN
        0x3,  //TDS_HIGH_COR_VALUE
        0x60, //TDS_HIGH_BOUND
        0x30, //TDS_HIGH_LIMIT
        0x2,  //EDF_CLIP_RATIO_INC
        0x08, //EDF_EDGE_GAIN
        0x20, //EDF_DETAIL_GAIN
        0x02, //EDF_FLAT_GAIN
        0x1,  //EDF_GAIN_EN
        0xE0, //EDF_EDGE_TH
        0xE0, //EDF_DETAIL_FALL_TH
        0x10, //EDF_DETAIL_RISE_TH
        0x10, //EDF_FLAT_TH
        0xA,  //EDF_EDGE_SLOPE
        0xA,  //EDF_DETAIL_FALL_SLOPE
        0x5,  //EDF_DETAIL_RISE_SLOPE
        0x4,  //EDF_FLAT_SLOPE
        0x7,  //EDF_EDGE_MONO_SLOPE
        0x30, //EDF_EDGE_MONO_TH
        0x8,  //EDF_EDGE_MAG_SLOPE
        0x0,  //EDF_EDGE_MAG_TH
        0x8,  //EDF_EDGE_TREND_FLAT_MAG
        0x10, //EDF_EDGE_TREND_SLOPE
        0x20, //EDF_EDGE_TREND_TH
        0x20, //EDF_BLD_WGT_MAG
        0x20, //EDF_BLD_WGT_MONO
        0x20, //EDF_BLD_WGT_TREND
        0xFF, //TDSHP_CBOOST_LMT_U
        0x80, //TDSHP_CBOOST_LMT_L
        0x01, //TDSHP_CBOOST_EN
        0x80, //TDSHP_CBOOST_GAIN
        0x10, //TDSHP_CBOOST_YCONST
        0x0,  //TDSHP_CBOOST_YOFFSET_SEL
        0x0,  //TDSHP_CBOOST_YOFFSET

        /*TDSHP_3_0*/
        0x48, //TDSHP_POST_YLEV_P048
        0x30, //TDSHP_POST_YLEV_P032
        0x18, //TDSHP_POST_YLEV_P016
        0x0,  //TDSHP_POST_YLEV_P000
        0x78, //TDSHP_POST_YLEV_P112
        0x70, //TDSHP_POST_YLEV_P096
        0x68, //TDSHP_POST_YLEV_P080
        0x60, //TDSHP_POST_YLEV_P064
        0x70, //TDSHP_POST_YLEV_P176
        0x76, //TDSHP_POST_YLEV_P160
        0x7B, //TDSHP_POST_YLEV_P144
        0x80, //TDSHP_POST_YLEV_P128
        0x5B, //TDSHP_POST_YLEV_P240
        0x60, //TDSHP_POST_YLEV_P224
        0x66, //TDSHP_POST_YLEV_P208
        0x6B, //TDSHP_POST_YLEV_P192
        0x0,  //TDSHP_POST_YLEV_EN
        0x10, //TDSHP_POST_YLEV_ALPHA
        0x56  //TDSHP_POST_YLEV_256
}
,
{//7
        0x2A, //TDS_COR_GAIN       1500B000[07:00]
        0x60, //TDS_GAIN_HIGH      1500B000[15:08]
        0x50, //TDS_GAIN_MID       1500B000[23:16]
        0x0,  //TDS_INK_SEL        1500B000[25:24]
        0x0,  //TDS_BYPASS_HIGH    1500B000[29:29]
        0x0,  //TDS_BYPASS_MID     1500B000[30:30]
        0x1,  //TDS_EN             1500B000[31:31]
        0x8,  //TDS_LIMIT_RATIO    1500B004[03:00]
        0x54, //TDS_GAIN           1500B004[11:04]
        0x2,  //TDS_COR_ZERO       1500B004[23:16]
        0x4,  //TDS_COR_THR        1500B004[31:24]
        0x3,  //TDS_COR_VALUE      1500B008[15:08]
        0x40, //TDS_BOUND          1500B008[23:16]
        0x20, //TDS_LIMIT          1500B008[31:24]
        0x8,  //TDS_SAT_PROC       1500B00C[05:00]
        0x2,  //TDS_AC_LPF_COE     1500B00C[11:08]
        0x8,  //TDS_CLIP_THR       1500B00C[23:16]
        0x4,  //TDS_CLIP_RATIO     1500B00C[28:24]
        0x1,  //TDS_CLIP_EN        1500B00C[31:31]
        0x48, //TDS_YLEV_P048      1500B014[07:00]
        0x30, //TDS_YLEV_P032      1500B014[15:08]
        0x18, //TDS_YLEV_P016      1500B014[23:16]
        0x0,  //TDS_YLEV_P000      1500B014[31:24]
        0x78, //TDS_YLEV_P112      1500B018[07:00]
        0x70, //TDS_YLEV_P096      1500B018[15:08]
        0x68, //TDS_YLEV_P080      1500B018[23:16]
        0x58, //TDS_YLEV_P064      1500B018[31:24]
        0x70, //TDS_YLEV_P176      1500B01C[07:00]
        0x76, //TDS_YLEV_P160      1500B01C[15:08]
        0x7B, //TDS_YLEV_P144      1500B01C[23:16]
        0x80, //TDS_YLEV_P128      1500B01C[31:24]
        0x5B, //TDS_YLEV_P240      1500B020[07:00]
        0x60, //TDS_YLEV_P224      1500B020[15:08]
        0x66, //TDS_YLEV_P208      1500B020[23:16]
        0x6B, //TDS_YLEV_P192      1500B020[31:24]
        0x1,  //TDS_YLEV_EN        1500B024[14:14]
        0x10, //TDS_YLEV_ALPHA     1500B024[21:16]
        0x56, //TDS_YLEV_P256      1500B024[31:24]

        0x0C, //PBC1_RADIUS_R      1500B040[05:00]
        0x0C, //PBC1_THETA_R       1500B040[11:06]
        0x27, //PBC1_RSLOPE_1      1500B040[21:12]
        0x30, //PBC1_GAIN          1500B040[29:22]
        0x0,  //PBC1_LPF_EN        1500B040[30:30]
        0x1,  //PBC1_EN            1500B040[31:31]
        0x10, //PBC1_LPF_GAIN      1500B044[05:00]
        0x55, //PBC1_TSLOPE        1500B044[15:06]
        0x1A, //PBC1_RADIUS_C      1500B044[23:16]
        0x7F, //PBC1_THETA_C       1500B044[31:24]
        0x20, //PBC1_EDGE_SLOPE    1500B048[05:00]
        0xC,  //PBC1_EDGE_THR      1500B048[13:08]
        0x0,  //PBC1_EDGE_EN       1500B048[14:14]
        0x1,  //PBC1_CONF_GAIN     1500B048[19:16]
        0x55, //PBC1_RSLOPE        1500B048[31:22]

        0x18, //PBC2_RADIUS_R      1500B04C[05:00]
        0x18, //PBC2_THETA_R       1500B04C[11:06]
        0x20, //PBC2_RSLOPE_1      1500B04C[21:12]
        0x10, //PBC2_GAIN          1500B04C[29:22]
        0x1,  //PBC2_LPF_EN        1500B04C[30:30]
        0x1,  //PBC2_EN            1500B04C[31:31]
        0x10, //PBC1_LPF_GAIN      1500B050[05:00]
        0x2B, //PBC2_TSLOPE        1500B050[15:06]
        0x20, //PBC2_RADIUS_C      1500B050[23:16]
        0x60, //PBC2_THETA_C       1500B050[31:24]
        0x20, //PBC2_EDGE_SLOPE    1500B054[05:00]
        0xC,  //PBC2_EDGE_THR      1500B054[13:08]
        0x1,  //PBC2_EDGE_EN       1500B054[14:14]
        0x5,  //PBC2_CONF_GAIN     1500B054[19:16]
        0x2B, //PBC2_RSLOPE        1500B054[31:22]

        0x20, //PBC3_RADIUS_R      1500B058[05:00]
        0x10, //PBC3_THETA_R       1500B058[11:06]
        0x11, //PBC3_RSLOPE_1      1500B058[21:12]
        0x30, //PBC3_GAIN          1500B058[29:22]
        0x0,  //PBC3_LPF_EN        1500B058[30:30]
        0x1,  //PBC3_EN            1500B058[31:31]
        0x10, //PBC3_LPF_GAIN      1500B05C[05:00]
        0x40, //PBC3_TSLOPE        1500B05C[15:06]
        0x3C, //PBC3_RADIUS_C      1500B05C[23:16]
        0xE0, //PBC3_THETA_C       1500B05C[31:24]
        0x20, //PBC3_EDGE_SLOPE    1500B060[05:00]
        0xC,  //PBC3_EDGE_THR      1500B060[13:08]
        0x0,  //PBC3_EDGE_EN       1500B060[14:14]
        0x1,  //PBC3_CONF_GAIN     1500B060[19:16]
        0x20, //PBC3_RSLOPE        1500B060[31:22]

        /*TDSHP_2_0*/
        0x8,  //TDS_MID_LIMIT_RATIO
        0x0,  //TDS_MID_COR_ZERO
        0x0,  //TDS_MID_COR_THR
        0x08, //TDS_MID_COR_GAIN
        0x0,  //TDS_MID_COR_VALUE
        0x60, //TDS_MID_BOUND
        0x30, //TDS_MID_LIMIT
        0x8,  //TDS_HIGH_LIMIT_RATIO
        0x2,  //TDS_HIGH_COR_ZERO
        0x4,  //TDS_HIGH_COR_THR
        0x10, //TDS_HIGH_COR_GAIN
        0x3,  //TDS_HIGH_COR_VALUE
        0x60, //TDS_HIGH_BOUND
        0x30, //TDS_HIGH_LIMIT
        0x2,  //EDF_CLIP_RATIO_INC
        0x08, //EDF_EDGE_GAIN
        0x20, //EDF_DETAIL_GAIN
        0x02, //EDF_FLAT_GAIN
        0x1,  //EDF_GAIN_EN
        0xE0, //EDF_EDGE_TH
        0xE0, //EDF_DETAIL_FALL_TH
        0x10, //EDF_DETAIL_RISE_TH
        0x10, //EDF_FLAT_TH
        0xA,  //EDF_EDGE_SLOPE
        0xA,  //EDF_DETAIL_FALL_SLOPE
        0x5,  //EDF_DETAIL_RISE_SLOPE
        0x4,  //EDF_FLAT_SLOPE
        0x7,  //EDF_EDGE_MONO_SLOPE
        0x30, //EDF_EDGE_MONO_TH
        0x8,  //EDF_EDGE_MAG_SLOPE
        0x0,  //EDF_EDGE_MAG_TH
        0x8,  //EDF_EDGE_TREND_FLAT_MAG
        0x10, //EDF_EDGE_TREND_SLOPE
        0x20, //EDF_EDGE_TREND_TH
        0x20, //EDF_BLD_WGT_MAG
        0x20, //EDF_BLD_WGT_MONO
        0x20, //EDF_BLD_WGT_TREND
        0xFF, //TDSHP_CBOOST_LMT_U
        0x80, //TDSHP_CBOOST_LMT_L
        0x01, //TDSHP_CBOOST_EN
        0x80, //TDSHP_CBOOST_GAIN
        0x10, //TDSHP_CBOOST_YCONST
        0x0,  //TDSHP_CBOOST_YOFFSET_SEL
        0x0,  //TDSHP_CBOOST_YOFFSET

        /*TDSHP_3_0*/
        0x48, //TDSHP_POST_YLEV_P048
        0x30, //TDSHP_POST_YLEV_P032
        0x18, //TDSHP_POST_YLEV_P016
        0x0,  //TDSHP_POST_YLEV_P000
        0x78, //TDSHP_POST_YLEV_P112
        0x70, //TDSHP_POST_YLEV_P096
        0x68, //TDSHP_POST_YLEV_P080
        0x60, //TDSHP_POST_YLEV_P064
        0x70, //TDSHP_POST_YLEV_P176
        0x76, //TDSHP_POST_YLEV_P160
        0x7B, //TDSHP_POST_YLEV_P144
        0x80, //TDSHP_POST_YLEV_P128
        0x5B, //TDSHP_POST_YLEV_P240
        0x60, //TDSHP_POST_YLEV_P224
        0x66, //TDSHP_POST_YLEV_P208
        0x6B, //TDSHP_POST_YLEV_P192
        0x0,  //TDSHP_POST_YLEV_EN
        0x10, //TDSHP_POST_YLEV_ALPHA
        0x56  //TDSHP_POST_YLEV_256
}
,
{//8
        0x2A, //TDS_COR_GAIN       1500B000[07:00]
        0x70, //TDS_GAIN_HIGH      1500B000[15:08]
        0x60, //TDS_GAIN_MID       1500B000[23:16]
        0x0,  //TDS_INK_SEL        1500B000[25:24]
        0x0,  //TDS_BYPASS_HIGH    1500B000[29:29]
        0x0,  //TDS_BYPASS_MID     1500B000[30:30]
        0x1,  //TDS_EN             1500B000[31:31]
        0x8,  //TDS_LIMIT_RATIO    1500B004[03:00]
        0x54, //TDS_GAIN           1500B004[11:04]
        0x2,  //TDS_COR_ZERO       1500B004[23:16]
        0x4,  //TDS_COR_THR        1500B004[31:24]
        0x3,  //TDS_COR_VALUE      1500B008[15:08]
        0x40, //TDS_BOUND          1500B008[23:16]
        0x20, //TDS_LIMIT          1500B008[31:24]
        0x8,  //TDS_SAT_PROC       1500B00C[05:00]
        0x2,  //TDS_AC_LPF_COE     1500B00C[11:08]
        0x8,  //TDS_CLIP_THR       1500B00C[23:16]
        0x4,  //TDS_CLIP_RATIO     1500B00C[28:24]
        0x1,  //TDS_CLIP_EN        1500B00C[31:31]
        0x48, //TDS_YLEV_P048      1500B014[07:00]
        0x30, //TDS_YLEV_P032      1500B014[15:08]
        0x18, //TDS_YLEV_P016      1500B014[23:16]
        0x0,  //TDS_YLEV_P000      1500B014[31:24]
        0x78, //TDS_YLEV_P112      1500B018[07:00]
        0x70, //TDS_YLEV_P096      1500B018[15:08]
        0x68, //TDS_YLEV_P080      1500B018[23:16]
        0x58, //TDS_YLEV_P064      1500B018[31:24]
        0x70, //TDS_YLEV_P176      1500B01C[07:00]
        0x76, //TDS_YLEV_P160      1500B01C[15:08]
        0x7B, //TDS_YLEV_P144      1500B01C[23:16]
        0x80, //TDS_YLEV_P128      1500B01C[31:24]
        0x5B, //TDS_YLEV_P240      1500B020[07:00]
        0x60, //TDS_YLEV_P224      1500B020[15:08]
        0x66, //TDS_YLEV_P208      1500B020[23:16]
        0x6B, //TDS_YLEV_P192      1500B020[31:24]
        0x1,  //TDS_YLEV_EN        1500B024[14:14]
        0x10, //TDS_YLEV_ALPHA     1500B024[21:16]
        0x56, //TDS_YLEV_P256      1500B024[31:24]

        0x0C, //PBC1_RADIUS_R      1500B040[05:00]
        0x0C, //PBC1_THETA_R       1500B040[11:06]
        0x27, //PBC1_RSLOPE_1      1500B040[21:12]
        0x30, //PBC1_GAIN          1500B040[29:22]
        0x0,  //PBC1_LPF_EN        1500B040[30:30]
        0x1,  //PBC1_EN            1500B040[31:31]
        0x10, //PBC1_LPF_GAIN      1500B044[05:00]
        0x55, //PBC1_TSLOPE        1500B044[15:06]
        0x1A, //PBC1_RADIUS_C      1500B044[23:16]
        0x7F, //PBC1_THETA_C       1500B044[31:24]
        0x20, //PBC1_EDGE_SLOPE    1500B048[05:00]
        0xC,  //PBC1_EDGE_THR      1500B048[13:08]
        0x0,  //PBC1_EDGE_EN       1500B048[14:14]
        0x1,  //PBC1_CONF_GAIN     1500B048[19:16]
        0x55, //PBC1_RSLOPE        1500B048[31:22]

        0x18, //PBC2_RADIUS_R      1500B04C[05:00]
        0x18, //PBC2_THETA_R       1500B04C[11:06]
        0x20, //PBC2_RSLOPE_1      1500B04C[21:12]
        0x10, //PBC2_GAIN          1500B04C[29:22]
        0x1,  //PBC2_LPF_EN        1500B04C[30:30]
        0x1,  //PBC2_EN            1500B04C[31:31]
        0x10, //PBC1_LPF_GAIN      1500B050[05:00]
        0x2B, //PBC2_TSLOPE        1500B050[15:06]
        0x20, //PBC2_RADIUS_C      1500B050[23:16]
        0x60, //PBC2_THETA_C       1500B050[31:24]
        0x20, //PBC2_EDGE_SLOPE    1500B054[05:00]
        0xC,  //PBC2_EDGE_THR      1500B054[13:08]
        0x1,  //PBC2_EDGE_EN       1500B054[14:14]
        0x5,  //PBC2_CONF_GAIN     1500B054[19:16]
        0x2B, //PBC2_RSLOPE        1500B054[31:22]

        0x20, //PBC3_RADIUS_R      1500B058[05:00]
        0x10, //PBC3_THETA_R       1500B058[11:06]
        0x11, //PBC3_RSLOPE_1      1500B058[21:12]
        0x30, //PBC3_GAIN          1500B058[29:22]
        0x0,  //PBC3_LPF_EN        1500B058[30:30]
        0x1,  //PBC3_EN            1500B058[31:31]
        0x10, //PBC3_LPF_GAIN      1500B05C[05:00]
        0x40, //PBC3_TSLOPE        1500B05C[15:06]
        0x3C, //PBC3_RADIUS_C      1500B05C[23:16]
        0xE0, //PBC3_THETA_C       1500B05C[31:24]
        0x20, //PBC3_EDGE_SLOPE    1500B060[05:00]
        0xC,  //PBC3_EDGE_THR      1500B060[13:08]
        0x0,  //PBC3_EDGE_EN       1500B060[14:14]
        0x1,  //PBC3_CONF_GAIN     1500B060[19:16]
        0x20, //PBC3_RSLOPE        1500B060[31:22]

        /*TDSHP_2_0*/
        0x8,  //TDS_MID_LIMIT_RATIO
        0x0,  //TDS_MID_COR_ZERO
        0x0,  //TDS_MID_COR_THR
        0x08, //TDS_MID_COR_GAIN
        0x0,  //TDS_MID_COR_VALUE
        0x60, //TDS_MID_BOUND
        0x30, //TDS_MID_LIMIT
        0x8,  //TDS_HIGH_LIMIT_RATIO
        0x2,  //TDS_HIGH_COR_ZERO
        0x4,  //TDS_HIGH_COR_THR
        0x10, //TDS_HIGH_COR_GAIN
        0x3,  //TDS_HIGH_COR_VALUE
        0x60, //TDS_HIGH_BOUND
        0x30, //TDS_HIGH_LIMIT
        0x2,  //EDF_CLIP_RATIO_INC
        0x08, //EDF_EDGE_GAIN
        0x20, //EDF_DETAIL_GAIN
        0x02, //EDF_FLAT_GAIN
        0x1,  //EDF_GAIN_EN
        0xE0, //EDF_EDGE_TH
        0xE0, //EDF_DETAIL_FALL_TH
        0x10, //EDF_DETAIL_RISE_TH
        0x10, //EDF_FLAT_TH
        0xA,  //EDF_EDGE_SLOPE
        0xA,  //EDF_DETAIL_FALL_SLOPE
        0x5,  //EDF_DETAIL_RISE_SLOPE
        0x4,  //EDF_FLAT_SLOPE
        0x7,  //EDF_EDGE_MONO_SLOPE
        0x30, //EDF_EDGE_MONO_TH
        0x8,  //EDF_EDGE_MAG_SLOPE
        0x0,  //EDF_EDGE_MAG_TH
        0x8,  //EDF_EDGE_TREND_FLAT_MAG
        0x10, //EDF_EDGE_TREND_SLOPE
        0x20, //EDF_EDGE_TREND_TH
        0x20, //EDF_BLD_WGT_MAG
        0x20, //EDF_BLD_WGT_MONO
        0x20, //EDF_BLD_WGT_TREND
        0xFF, //TDSHP_CBOOST_LMT_U
        0x80, //TDSHP_CBOOST_LMT_L
        0x01, //TDSHP_CBOOST_EN
        0x80, //TDSHP_CBOOST_GAIN
        0x10, //TDSHP_CBOOST_YCONST
        0x0,  //TDSHP_CBOOST_YOFFSET_SEL
        0x0,  //TDSHP_CBOOST_YOFFSET

        /*TDSHP_3_0*/
        0x48, //TDSHP_POST_YLEV_P048
        0x30, //TDSHP_POST_YLEV_P032
        0x18, //TDSHP_POST_YLEV_P016
        0x0,  //TDSHP_POST_YLEV_P000
        0x78, //TDSHP_POST_YLEV_P112
        0x70, //TDSHP_POST_YLEV_P096
        0x68, //TDSHP_POST_YLEV_P080
        0x60, //TDSHP_POST_YLEV_P064
        0x70, //TDSHP_POST_YLEV_P176
        0x76, //TDSHP_POST_YLEV_P160
        0x7B, //TDSHP_POST_YLEV_P144
        0x80, //TDSHP_POST_YLEV_P128
        0x5B, //TDSHP_POST_YLEV_P240
        0x60, //TDSHP_POST_YLEV_P224
        0x66, //TDSHP_POST_YLEV_P208
        0x6B, //TDSHP_POST_YLEV_P192
        0x0,  //TDSHP_POST_YLEV_EN
        0x10, //TDSHP_POST_YLEV_ALPHA
        0x56  //TDSHP_POST_YLEV_256
}
,
{//9
        0x2A, //TDS_COR_GAIN       1500B000[07:00]
        0x88, //TDS_GAIN_HIGH      1500B000[15:08]
        0x78, //TDS_GAIN_MID       1500B000[23:16]
        0x0,  //TDS_INK_SEL         1500B000[25:24]
        0x0,  //TDS_BYPASS_HIGH     1500B000[29:29]
        0x0,  //TDS_BYPASS_MID      1500B000[30:30]
        0x1,  //TDS_EN              1500B000[31:31]
        0x8,  //TDS_LIMIT_RATIO     1500B004[03:00]
        0x54, //TDS_GAIN           1500B004[11:04]
        0x2,  //TDS_COR_ZERO        1500B004[23:16]
        0x4,  //TDS_COR_THR         1500B004[31:24]
        0x3,  //TDS_COR_VALUE       1500B008[15:08]
        0x40, //TDS_BOUND          1500B008[23:16]
        0x20, //TDS_LIMIT          1500B008[31:24]
        0x8,  //TDS_SAT_PROC       1500B00C[05:00]
        0x2,  //TDS_AC_LPF_COE      1500B00C[11:08]
        0x8,  //TDS_CLIP_THR        1500B00C[23:16]
        0x4,  //TDS_CLIP_RATIO      1500B00C[28:24]
        0x1,  //TDS_CLIP_EN         1500B00C[31:31]
        0x48, //TDS_YLEV_P048      1500B014[07:00]
        0x30, //TDS_YLEV_P032      1500B014[15:08]
        0x18, //TDS_YLEV_P016      1500B014[23:16]
        0x0,  //TDS_YLEV_P000      1500B014[31:24]
        0x78, //TDS_YLEV_P112      1500B018[07:00]
        0x70, //TDS_YLEV_P096      1500B018[15:08]
        0x68, //TDS_YLEV_P080      1500B018[23:16]
        0x58, //TDS_YLEV_P064      1500B018[31:24]
        0x70, //TDS_YLEV_P176      1500B01C[07:00]
        0x76, //TDS_YLEV_P160      1500B01C[15:08]
        0x7B, //TDS_YLEV_P144      1500B01C[23:16]
        0x80, //TDS_YLEV_P128      1500B01C[31:24]
        0x5B, //TDS_YLEV_P240      1500B020[07:00]
        0x60, //TDS_YLEV_P224      1500B020[15:08]
        0x66, //TDS_YLEV_P208      1500B020[23:16]
        0x6B, //TDS_YLEV_P192      1500B020[31:24]
        0x1,  //TDS_YLEV_EN        1500B024[14:14]
        0x10, //TDS_YLEV_ALPHA     1500B024[21:16]
        0x56, //TDS_YLEV_P256      1500B024[31:24]

        0x0C, //PBC1_RADIUS_R      1500B040[05:00]
        0x0C, //PBC1_THETA_R       1500B040[11:06]
        0x27, //PBC1_RSLOPE_1      1500B040[21:12]
        0x30, //PBC1_GAIN          1500B040[29:22]
        0x0,  //PBC1_LPF_EN        1500B040[30:30]
        0x1,  //PBC1_EN            1500B040[31:31]
        0x10, //PBC1_LPF_GAIN      1500B044[05:00]
        0x55, //PBC1_TSLOPE        1500B044[15:06]
        0x1A, //PBC1_RADIUS_C      1500B044[23:16]
        0x7F, //PBC1_THETA_C       1500B044[31:24]
        0x20, //PBC1_EDGE_SLOPE    1500B048[05:00]
        0xC,  //PBC1_EDGE_THR      1500B048[13:08]
        0x0,  //PBC1_EDGE_EN       1500B048[14:14]
        0x1,  //PBC1_CONF_GAIN     1500B048[19:16]
        0x55, //PBC1_RSLOPE        1500B048[31:22]

        0x18, //PBC2_RADIUS_R      1500B04C[05:00]
        0x18, //PBC2_THETA_R       1500B04C[11:06]
        0x20, //PBC2_RSLOPE_1      1500B04C[21:12]
        0x10, //PBC2_GAIN          1500B04C[29:22]
        0x1,  //PBC2_LPF_EN        1500B04C[30:30]
        0x1,  //PBC2_EN            1500B04C[31:31]
        0x10, //PBC1_LPF_GAIN      1500B050[05:00]
        0x2B, //PBC2_TSLOPE        1500B050[15:06]
        0x20, //PBC2_RADIUS_C      1500B050[23:16]
        0x60, //PBC2_THETA_C       1500B050[31:24]
        0x20, //PBC2_EDGE_SLOPE    1500B054[05:00]
        0xC,  //PBC2_EDGE_THR      1500B054[13:08]
        0x1,  //PBC2_EDGE_EN       1500B054[14:14]
        0x5,  //PBC2_CONF_GAIN     1500B054[19:16]
        0x2B, //PBC2_RSLOPE        1500B054[31:22]

        0x20, //PBC3_RADIUS_R      1500B058[05:00]
        0x10, //PBC3_THETA_R       1500B058[11:06]
        0x11, //PBC3_RSLOPE_1      1500B058[21:12]
        0x30, //PBC3_GAIN          1500B058[29:22]
        0x0,  //PBC3_LPF_EN        1500B058[30:30]
        0x1,  //PBC3_EN            1500B058[31:31]
        0x10, //PBC3_LPF_GAIN      1500B05C[05:00]
        0x40, //PBC3_TSLOPE        1500B05C[15:06]
        0x3C, //PBC3_RADIUS_C      1500B05C[23:16]
        0xE0, //PBC3_THETA_C       1500B05C[31:24]
        0x20, //PBC3_EDGE_SLOPE    1500B060[05:00]
        0xC,  //PBC3_EDGE_THR      1500B060[13:08]
        0x0,  //PBC3_EDGE_EN       1500B060[14:14]
        0x1,  //PBC3_CONF_GAIN     1500B060[19:16]
        0x20, //PBC3_RSLOPE        1500B060[31:22]

        /*TDSHP_2_0*/
        0x8,  //TDS_MID_LIMIT_RATIO
        0x0,  //TDS_MID_COR_ZERO
        0x0,  //TDS_MID_COR_THR
        0x08, //TDS_MID_COR_GAIN
        0x0,  //TDS_MID_COR_VALUE
        0x60, //TDS_MID_BOUND
        0x30, //TDS_MID_LIMIT
        0x8,  //TDS_HIGH_LIMIT_RATIO
        0x2,  //TDS_HIGH_COR_ZERO
        0x4,  //TDS_HIGH_COR_THR
        0x10, //TDS_HIGH_COR_GAIN
        0x3,  //TDS_HIGH_COR_VALUE
        0x60, //TDS_HIGH_BOUND
        0x30, //TDS_HIGH_LIMIT
        0x2,  //EDF_CLIP_RATIO_INC
        0x08, //EDF_EDGE_GAIN
        0x20, //EDF_DETAIL_GAIN
        0x02, //EDF_FLAT_GAIN
        0x1,  //EDF_GAIN_EN
        0xE0, //EDF_EDGE_TH
        0xE0, //EDF_DETAIL_FALL_TH
        0x10, //EDF_DETAIL_RISE_TH
        0x10, //EDF_FLAT_TH
        0xA,  //EDF_EDGE_SLOPE
        0xA,  //EDF_DETAIL_FALL_SLOPE
        0x5,  //EDF_DETAIL_RISE_SLOPE
        0x4,  //EDF_FLAT_SLOPE
        0x7,  //EDF_EDGE_MONO_SLOPE
        0x30, //EDF_EDGE_MONO_TH
        0x8,  //EDF_EDGE_MAG_SLOPE
        0x0,  //EDF_EDGE_MAG_TH
        0x8,  //EDF_EDGE_TREND_FLAT_MAG
        0x10, //EDF_EDGE_TREND_SLOPE
        0x20, //EDF_EDGE_TREND_TH
        0x20, //EDF_BLD_WGT_MAG
        0x20, //EDF_BLD_WGT_MONO
        0x20, //EDF_BLD_WGT_TREND
        0xFF, //TDSHP_CBOOST_LMT_U
        0x80, //TDSHP_CBOOST_LMT_L
        0x01, //TDSHP_CBOOST_EN
        0x80, //TDSHP_CBOOST_GAIN
        0x10, //TDSHP_CBOOST_YCONST
        0x0,  //TDSHP_CBOOST_YOFFSET_SEL
        0x0,  //TDSHP_CBOOST_YOFFSET

        /*TDSHP_3_0*/
        0x48, //TDSHP_POST_YLEV_P048
        0x30, //TDSHP_POST_YLEV_P032
        0x18, //TDSHP_POST_YLEV_P016
        0x0,  //TDSHP_POST_YLEV_P000
        0x78, //TDSHP_POST_YLEV_P112
        0x70, //TDSHP_POST_YLEV_P096
        0x68, //TDSHP_POST_YLEV_P080
        0x60, //TDSHP_POST_YLEV_P064
        0x70, //TDSHP_POST_YLEV_P176
        0x76, //TDSHP_POST_YLEV_P160
        0x7B, //TDSHP_POST_YLEV_P144
        0x80, //TDSHP_POST_YLEV_P128
        0x5B, //TDSHP_POST_YLEV_P240
        0x60, //TDSHP_POST_YLEV_P224
        0x66, //TDSHP_POST_YLEV_P208
        0x6B, //TDSHP_POST_YLEV_P192
        0x0,  //TDSHP_POST_YLEV_EN
        0x10, //TDSHP_POST_YLEV_ALPHA
        0x56  //TDSHP_POST_YLEV_256
}
,
{//10, weaker sharpness for video on UR
        0x30, //TDS_COR_GAIN       1500B000[07:00]
        0x10, //TDS_GAIN_HIGH      1500B000[15:08]
        0x10, //TDS_GAIN_MID       1500B000[23:16]
        0x0, //TDS_INK_SEL        1500B000[25:24]
        0x0, //TDS_BYPASS_HIGH    1500B000[29:29]
        0x0, //TDS_BYPASS_MID     1500B000[30:30]
        0x1, //TDS_EN             1500B000[31:31]
        0x6, //TDS_LIMIT_RATIO    1500B004[03:00]
        0x60, //TDS_GAIN           1500B004[11:04]
        0x2, //TDS_COR_ZERO       1500B004[23:16]
        0xC, //TDS_COR_THR        1500B004[31:24]
        0x7, //TDS_COR_VALUE      1500B008[15:08]
        0x40, //TDS_BOUND          1500B008[23:16]
        0x20, //TDS_LIMIT          1500B008[31:24]
        0x8,  //TDS_SAT_PROC       1500B00C[05:00]
        0x8, //TDS_AC_LPF_COE     1500B00C[11:08]
        0xC, //TDS_CLIP_THR       1500B00C[23:16]
        0xA, //TDS_CLIP_RATIO     1500B00C[28:24]
        0x1, //TDS_CLIP_EN        1500B00C[31:31]
        0x48, //TDS_YLEV_P048      1500B014[07:00]
        0x30, //TDS_YLEV_P032      1500B014[15:08]
        0x18, //TDS_YLEV_P016      1500B014[23:16]
        0x0,  //TDS_YLEV_P000      1500B014[31:24]
        0x78, //TDS_YLEV_P112      1500B018[07:00]
        0x70, //TDS_YLEV_P096      1500B018[15:08]
        0x68, //TDS_YLEV_P080      1500B018[23:16]
        0x58, //TDS_YLEV_P064      1500B018[31:24]
        0x70, //TDS_YLEV_P176      1500B01C[07:00]
        0x76, //TDS_YLEV_P160      1500B01C[15:08]
        0x7B, //TDS_YLEV_P144      1500B01C[23:16]
        0x80, //TDS_YLEV_P128      1500B01C[31:24]
        0x5B, //TDS_YLEV_P240      1500B020[07:00]
        0x60, //TDS_YLEV_P224      1500B020[15:08]
        0x66, //TDS_YLEV_P208      1500B020[23:16]
        0x6B, //TDS_YLEV_P192      1500B020[31:24]
        0x1,  //TDS_YLEV_EN        1500B024[14:14]
        0x10, //TDS_YLEV_ALPHA     1500B024[21:16]
        0x56, //TDS_YLEV_P256      1500B024[31:24]

        0x0C, //PBC1_RADIUS_R      1500B040[05:00]
        0x0C, //PBC1_THETA_R       1500B040[11:06]
        0x27, //PBC1_RSLOPE_1      1500B040[21:12]
        0x30, //PBC1_GAIN          1500B040[29:22]
        0x0,  //PBC1_LPF_EN        1500B040[30:30]
        0x1,  //PBC1_EN            1500B040[31:31]
        0x10, //PBC1_LPF_GAIN      1500B044[05:00]
        0x55, //PBC1_TSLOPE        1500B044[15:06]
        0x1A, //PBC1_RADIUS_C      1500B044[23:16]
        0x7F, //PBC1_THETA_C       1500B044[31:24]
        0x20, //PBC1_EDGE_SLOPE    1500B048[05:00]
        0xC,  //PBC1_EDGE_THR      1500B048[13:08]
        0x0,  //PBC1_EDGE_EN       1500B048[14:14]
        0x1,  //PBC1_CONF_GAIN     1500B048[19:16]
        0x55, //PBC1_RSLOPE        1500B048[31:22]

        0x18, //PBC2_RADIUS_R      1500B04C[05:00]
        0x18, //PBC2_THETA_R       1500B04C[11:06]
        0x20, //PBC2_RSLOPE_1      1500B04C[21:12]
        0x08, //PBC2_GAIN          1500B04C[29:22]
        0x1,  //PBC2_LPF_EN        1500B04C[30:30]
        0x1,  //PBC2_EN            1500B04C[31:31]
        0x10, //PBC1_LPF_GAIN      1500B050[05:00]
        0x2B, //PBC2_TSLOPE        1500B050[15:06]
        0x20, //PBC2_RADIUS_C      1500B050[23:16]
        0x60, //PBC2_THETA_C       1500B050[31:24]
        0x20, //PBC2_EDGE_SLOPE    1500B054[05:00]
        0xC,  //PBC2_EDGE_THR      1500B054[13:08]
        0x1,  //PBC2_EDGE_EN       1500B054[14:14]
        0x5,  //PBC2_CONF_GAIN     1500B054[19:16]
        0x2B, //PBC2_RSLOPE        1500B054[31:22]

        0x20, //PBC3_RADIUS_R      1500B058[05:00]
        0x10, //PBC3_THETA_R       1500B058[11:06]
        0x11, //PBC3_RSLOPE_1      1500B058[21:12]
        0x30, //PBC3_GAIN          1500B058[29:22]
        0x0,  //PBC3_LPF_EN        1500B058[30:30]
        0x0,  //PBC3_EN            1500B058[31:31]
        0x10, //PBC3_LPF_GAIN      1500B05C[05:00]
        0x40, //PBC3_TSLOPE        1500B05C[15:06]
        0x3C, //PBC3_RADIUS_C      1500B05C[23:16]
        0xE0, //PBC3_THETA_C       1500B05C[31:24]
        0x20, //PBC3_EDGE_SLOPE    1500B060[05:00]
        0xC,  //PBC3_EDGE_THR      1500B060[13:08]
        0x0,  //PBC3_EDGE_EN       1500B060[14:14]
        0x1,  //PBC3_CONF_GAIN     1500B060[19:16]
        0x20,  //PBC3_RSLOPE        1500B060[31:22]

        /*TDSHP_2_0*/
        0x8,  //TDS_MID_LIMIT_RATIO
        0x0,  //TDS_MID_COR_ZERO
        0x4,  //TDS_MID_COR_THR
        0x10, //TDS_MID_COR_GAIN
        0x2,  //TDS_MID_COR_VALUE
        0x40, //TDS_MID_BOUND
        0x20, //TDS_MID_LIMIT
        0x8,  //TDS_HIGH_LIMIT_RATIO
        0x2,  //TDS_HIGH_COR_ZERO
        0x4,  //TDS_HIGH_COR_THR
        0x18, //TDS_HIGH_COR_GAIN
        0x3,  //TDS_HIGH_COR_VALUE
        0x60, //TDS_HIGH_BOUND
        0x30, //TDS_HIGH_LIMIT
        0x2,  //EDF_CLIP_RATIO_INC
        0x05, //EDF_EDGE_GAIN
        0x18, //EDF_DETAIL_GAIN
        0x02, //EDF_FLAT_GAIN
        0x1,  //EDF_GAIN_EN
        0x70, //EDF_EDGE_TH
        0x70, //EDF_DETAIL_FALL_TH
        0x10, //EDF_DETAIL_RISE_TH
        0x4, //EDF_FLAT_TH
        0xA,  //EDF_EDGE_SLOPE
        0xA,  //EDF_DETAIL_FALL_SLOPE
        0x5,  //EDF_DETAIL_RISE_SLOPE
        0x4,  //EDF_FLAT_SLOPE
        0x5,  //EDF_EDGE_MONO_SLOPE
        0x30, //EDF_EDGE_MONO_TH
        0x8,  //EDF_EDGE_MAG_SLOPE
        0x0,  //EDF_EDGE_MAG_TH
        0x8,  //EDF_EDGE_TREND_FLAT_MAG
        0x10, //EDF_EDGE_TREND_SLOPE
        0x20, //EDF_EDGE_TREND_TH
        0x20, //EDF_BLD_WGT_MAG
        0x20, //EDF_BLD_WGT_MONO
        0x20, //EDF_BLD_WGT_TREND
        0xFF, //TDSHP_CBOOST_LMT_U
        0x80, //TDSHP_CBOOST_LMT_L
        0x01, //TDSHP_CBOOST_EN
        0x80, //TDSHP_CBOOST_GAIN
        0x10, //TDSHP_CBOOST_YCONST
        0x0,  //TDSHP_CBOOST_YOFFSET_SEL
        0x0,  //TDSHP_CBOOST_YOFFSET

        /*TDSHP_3_0*/
        0x48, //TDSHP_POST_YLEV_P048
        0x30, //TDSHP_POST_YLEV_P032
        0x18, //TDSHP_POST_YLEV_P016
        0x0,  //TDSHP_POST_YLEV_P000
        0x78, //TDSHP_POST_YLEV_P112
        0x70, //TDSHP_POST_YLEV_P096
        0x68, //TDSHP_POST_YLEV_P080
        0x60, //TDSHP_POST_YLEV_P064
        0x70, //TDSHP_POST_YLEV_P176
        0x76, //TDSHP_POST_YLEV_P160
        0x7B, //TDSHP_POST_YLEV_P144
        0x80, //TDSHP_POST_YLEV_P128
        0x5B, //TDSHP_POST_YLEV_P240
        0x60, //TDSHP_POST_YLEV_P224
        0x66, //TDSHP_POST_YLEV_P208
        0x6B, //TDSHP_POST_YLEV_P192
        0x0,  //TDSHP_POST_YLEV_EN
        0x10, //TDSHP_POST_YLEV_ALPHA
        0x56  //TDSHP_POST_YLEV_256
}
,
{//11
        0x18, //TDS_COR_GAIN       1500B000[07:00]
        0x18, //TDS_GAIN_HIGH      1500B000[15:08]
        0x8,  //TDS_GAIN_MID       1500B000[23:16]
        0x0,  //TDS_INK_SEL        1500B000[25:24]
        0x0,  //TDS_BYPASS_HIGH    1500B000[29:29]
        0x0,  //TDS_BYPASS_MID     1500B000[30:30]
        0x1,  //TDS_EN             1500B000[31:31]
        0x8,  //TDS_LIMIT_RATIO    1500B004[03:00]
        0x30, //TDS_GAIN           1500B004[11:04]
        0x2,  //TDS_COR_ZERO       1500B004[23:16]
        0x4,  //TDS_COR_THR        1500B004[31:24]
        0x3,  //TDS_COR_VALUE      1500B008[15:08]
        0x40, //TDS_BOUND          1500B008[23:16]
        0x20, //TDS_LIMIT          1500B008[31:24]
        0x8,  //TDS_SAT_PROC       1500B00C[05:00]
        0x2,  //TDS_AC_LPF_COE     1500B00C[11:08]
        0x8,  //TDS_CLIP_THR       1500B00C[23:16]
        0x4,  //TDS_CLIP_RATIO     1500B00C[28:24]
        0x1,  //TDS_CLIP_EN        1500B00C[31:31]
        0x48, //TDS_YLEV_P048      1500B014[07:00]
        0x30, //TDS_YLEV_P032      1500B014[15:08]
        0x18, //TDS_YLEV_P016      1500B014[23:16]
        0x0,  //TDS_YLEV_P000      1500B014[31:24]
        0x78, //TDS_YLEV_P112      1500B018[07:00]
        0x70, //TDS_YLEV_P096      1500B018[15:08]
        0x68, //TDS_YLEV_P080      1500B018[23:16]
        0x58, //TDS_YLEV_P064      1500B018[31:24]
        0x70, //TDS_YLEV_P176      1500B01C[07:00]
        0x76, //TDS_YLEV_P160      1500B01C[15:08]
        0x7B, //TDS_YLEV_P144      1500B01C[23:16]
        0x80, //TDS_YLEV_P128      1500B01C[31:24]
        0x5B, //TDS_YLEV_P240      1500B020[07:00]
        0x60, //TDS_YLEV_P224      1500B020[15:08]
        0x66, //TDS_YLEV_P208      1500B020[23:16]
        0x6B, //TDS_YLEV_P192      1500B020[31:24]
        0x1,  //TDS_YLEV_EN        1500B024[14:14]
        0x10, //TDS_YLEV_ALPHA     1500B024[21:16]
        0x56, //TDS_YLEV_P256      1500B024[31:24]

        0x0C, //PBC1_RADIUS_R      1500B040[05:00]
        0x0C, //PBC1_THETA_R       1500B040[11:06]
        0x27, //PBC1_RSLOPE_1      1500B040[21:12]
        0x30, //PBC1_GAIN          1500B040[29:22]
        0x0,  //PBC1_LPF_EN        1500B040[30:30]
        0x1,  //PBC1_EN            1500B040[31:31]
        0x10, //PBC1_LPF_GAIN      1500B044[05:00]
        0x55, //PBC1_TSLOPE        1500B044[15:06]
        0x1A, //PBC1_RADIUS_C      1500B044[23:16]
        0x7F, //PBC1_THETA_C       1500B044[31:24]
        0x20, //PBC1_EDGE_SLOPE    1500B048[05:00]
        0xC,  //PBC1_EDGE_THR      1500B048[13:08]
        0x0,  //PBC1_EDGE_EN       1500B048[14:14]
        0x1,  //PBC1_CONF_GAIN     1500B048[19:16]
        0x55, //PBC1_RSLOPE        1500B048[31:22]

        0x18, //PBC2_RADIUS_R      1500B04C[05:00]
        0x18, //PBC2_THETA_R       1500B04C[11:06]
        0x20, //PBC2_RSLOPE_1      1500B04C[21:12]
        0x10, //PBC2_GAIN          1500B04C[29:22]
        0x1,  //PBC2_LPF_EN        1500B04C[30:30]
        0x1,  //PBC2_EN            1500B04C[31:31]
        0x10, //PBC1_LPF_GAIN      1500B050[05:00]
        0x2B, //PBC2_TSLOPE        1500B050[15:06]
        0x20, //PBC2_RADIUS_C      1500B050[23:16]
        0x60, //PBC2_THETA_C       1500B050[31:24]
        0x20, //PBC2_EDGE_SLOPE    1500B054[05:00]
        0xC,  //PBC2_EDGE_THR      1500B054[13:08]
        0x1,  //PBC2_EDGE_EN       1500B054[14:14]
        0x5,  //PBC2_CONF_GAIN     1500B054[19:16]
        0x2B, //PBC2_RSLOPE        1500B054[31:22]

        0x20, //PBC3_RADIUS_R      1500B058[05:00]
        0x10, //PBC3_THETA_R       1500B058[11:06]
        0x11, //PBC3_RSLOPE_1      1500B058[21:12]
        0x30, //PBC3_GAIN          1500B058[29:22]
        0x0,  //PBC3_LPF_EN        1500B058[30:30]
        0x1,  //PBC3_EN            1500B058[31:31]
        0x10, //PBC3_LPF_GAIN      1500B05C[05:00]
        0x40, //PBC3_TSLOPE        1500B05C[15:06]
        0x3C, //PBC3_RADIUS_C      1500B05C[23:16]
        0xE0, //PBC3_THETA_C       1500B05C[31:24]
        0x20, //PBC3_EDGE_SLOPE    1500B060[05:00]
        0xC,  //PBC3_EDGE_THR      1500B060[13:08]
        0x0,  //PBC3_EDGE_EN       1500B060[14:14]
        0x1,  //PBC3_CONF_GAIN     1500B060[19:16]
        0x20, //PBC3_RSLOPE        1500B060[31:22]

        /*TDSHP_2_0*/
        0x8,  //TDS_MID_LIMIT_RATIO
        0x0,  //TDS_MID_COR_ZERO
        0x0,  //TDS_MID_COR_THR
        0x08, //TDS_MID_COR_GAIN
        0x0,  //TDS_MID_COR_VALUE
        0x60, //TDS_MID_BOUND
        0x30, //TDS_MID_LIMIT
        0x8,  //TDS_HIGH_LIMIT_RATIO
        0x2,  //TDS_HIGH_COR_ZERO
        0x4,  //TDS_HIGH_COR_THR
        0x10, //TDS_HIGH_COR_GAIN
        0x3,  //TDS_HIGH_COR_VALUE
        0x60, //TDS_HIGH_BOUND
        0x30, //TDS_HIGH_LIMIT
        0x2,  //EDF_CLIP_RATIO_INC
        0x08, //EDF_EDGE_GAIN
        0x20, //EDF_DETAIL_GAIN
        0x02, //EDF_FLAT_GAIN
        0x1,  //EDF_GAIN_EN
        0xE0, //EDF_EDGE_TH
        0xE0, //EDF_DETAIL_FALL_TH
        0x10, //EDF_DETAIL_RISE_TH
        0x10, //EDF_FLAT_TH
        0xA,  //EDF_EDGE_SLOPE
        0xA,  //EDF_DETAIL_FALL_SLOPE
        0x5,  //EDF_DETAIL_RISE_SLOPE
        0x4,  //EDF_FLAT_SLOPE
        0x7,  //EDF_EDGE_MONO_SLOPE
        0x30, //EDF_EDGE_MONO_TH
        0x8,  //EDF_EDGE_MAG_SLOPE
        0x0,  //EDF_EDGE_MAG_TH
        0x8,  //EDF_EDGE_TREND_FLAT_MAG
        0x10, //EDF_EDGE_TREND_SLOPE
        0x20, //EDF_EDGE_TREND_TH
        0x20, //EDF_BLD_WGT_MAG
        0x20, //EDF_BLD_WGT_MONO
        0x20, //EDF_BLD_WGT_TREND
        0xFF, //TDSHP_CBOOST_LMT_U
        0x80, //TDSHP_CBOOST_LMT_L
        0x01, //TDSHP_CBOOST_EN
        0x80, //TDSHP_CBOOST_GAIN
        0x10, //TDSHP_CBOOST_YCONST
        0x0,  //TDSHP_CBOOST_YOFFSET_SEL
        0x0,  //TDSHP_CBOOST_YOFFSET

        /*TDSHP_3_0*/
        0x48, //TDSHP_POST_YLEV_P048
        0x30, //TDSHP_POST_YLEV_P032
        0x18, //TDSHP_POST_YLEV_P016
        0x0,  //TDSHP_POST_YLEV_P000
        0x78, //TDSHP_POST_YLEV_P112
        0x70, //TDSHP_POST_YLEV_P096
        0x68, //TDSHP_POST_YLEV_P080
        0x60, //TDSHP_POST_YLEV_P064
        0x70, //TDSHP_POST_YLEV_P176
        0x76, //TDSHP_POST_YLEV_P160
        0x7B, //TDSHP_POST_YLEV_P144
        0x80, //TDSHP_POST_YLEV_P128
        0x5B, //TDSHP_POST_YLEV_P240
        0x60, //TDSHP_POST_YLEV_P224
        0x66, //TDSHP_POST_YLEV_P208
        0x6B, //TDSHP_POST_YLEV_P192
        0x0,  //TDSHP_POST_YLEV_EN
        0x10, //TDSHP_POST_YLEV_ALPHA
        0x56  //TDSHP_POST_YLEV_256
}
}
};
