/**************************************************************
Copyright 2018 Sony Semiconductor Solutions Corporation

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, 
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, 
this list of conditions and the following disclaimer in the documentation 
and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors 
may be used to endorse or promote products derived from this software without 
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS 
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **************************************************************/

/**************************************************************/
/*        include                                             */
/**************************************************************/

/******************************/
#include "SonyIMX519PdafTransCoord.h"
#include "ReadDefine.h"


/**************************************************************/
/*        macro                                               */
/**************************************************************/

#define UNMATCH_1(a, b)             ((a) != (b))
#define UNMATCH_2(a, b, c)          (((a) != (b)) && ((a) != (c)))
#define UNMATCH_3(a, b, c, d)       (((a) != (b)) && ((a) != (c)) && ((a) != (d)))
#define UNMATCH_4(a, b, c, d, e)    (((a) != (b)) && ((a) != (c)) && ((a) != (d)) && ((a) != (e)))
#define OUT_RANGE(a, b, c)          (((a) < (b)) || ((a) > (c)))

/**************************************************************/
/*        struct                                              */
/**************************************************************/
typedef struct tagReadDefine {
    unsigned int    img_orientation_h_msb;
    unsigned int    img_orientation_h_lsb;
    unsigned int    img_orientation_v_msb;
    unsigned int    img_orientation_v_lsb;
    unsigned int    hdr_mode_en_msb;
    unsigned int    hdr_mode_en_lsb;
    unsigned int    hdr_reso_redu_h_msb;
    unsigned int    hdr_reso_redu_h_lsb;
    unsigned int    hdr_reso_redu_v_msb;
    unsigned int    hdr_reso_redu_v_lsb;
    unsigned int    x_add_sta_1_msb;
    unsigned int    x_add_sta_1_lsb;
    unsigned int    x_add_sta_2_msb;
    unsigned int    x_add_sta_2_lsb;
    unsigned int    y_add_sta_1_msb;
    unsigned int    y_add_sta_1_lsb;
    unsigned int    y_add_sta_2_msb;
    unsigned int    y_add_sta_2_lsb;
    unsigned int    x_add_end_1_msb;
    unsigned int    x_add_end_1_lsb;
    unsigned int    x_add_end_2_msb;
    unsigned int    x_add_end_2_lsb;
    unsigned int    y_add_end_1_msb;
    unsigned int    y_add_end_1_lsb;
    unsigned int    y_add_end_2_msb;
    unsigned int    y_add_end_2_lsb;
    unsigned int    x_out_size_1_msb;
    unsigned int    x_out_size_1_lsb;
    unsigned int    x_out_size_2_msb;
    unsigned int    x_out_size_2_lsb;
    unsigned int    y_out_size_1_msb;
    unsigned int    y_out_size_1_lsb;
    unsigned int    y_out_size_2_msb;
    unsigned int    y_out_size_2_lsb;
    unsigned int    x_evn_inc_msb;
    unsigned int    x_evn_inc_lsb;
    unsigned int    x_odd_inc_msb;
    unsigned int    x_odd_inc_lsb;
    unsigned int    y_evn_inc_msb;
    unsigned int    y_evn_inc_lsb;
    unsigned int    y_odd_inc_msb;
    unsigned int    y_odd_inc_lsb;
    unsigned int    scale_mode_msb;
    unsigned int    scale_mode_lsb;
    unsigned int    scale_m_1_msb;
    unsigned int    scale_m_1_lsb;
    unsigned int    scale_m_2_msb;
    unsigned int    scale_m_2_lsb;
    unsigned int    scale_n;
    unsigned int    dig_crop_x_offset_1_msb;
    unsigned int    dig_crop_x_offset_1_lsb;
    unsigned int    dig_crop_x_offset_2_msb;
    unsigned int    dig_crop_x_offset_2_lsb;
    unsigned int    dig_crop_y_offset_1_msb;
    unsigned int    dig_crop_y_offset_1_lsb;
    unsigned int    dig_crop_y_offset_2_msb;
    unsigned int    dig_crop_y_offset_2_lsb;
    unsigned int    dig_crop_image_width_1_msb;
    unsigned int    dig_crop_image_width_1_lsb;
    unsigned int    dig_crop_image_width_2_msb;
    unsigned int    dig_crop_image_width_2_lsb;
    unsigned int    dig_crop_image_height_1_msb;
    unsigned int    dig_crop_image_height_1_lsb;
    unsigned int    dig_crop_image_height_2_msb;
    unsigned int    dig_crop_image_height_2_lsb;
    unsigned int    binning_mode_msb;
    unsigned int    binning_mode_lsb;
    unsigned int    binning_type_h_msb;
    unsigned int    binning_type_h_lsb;
    unsigned int    binning_type_v_msb;
    unsigned int    binning_type_v_lsb;
    unsigned int    binning_type_ext_en_msb;
    unsigned int    binning_type_ext_en_lsb;
    unsigned int    binning_type_h_ext_msb;
    unsigned int    binning_type_h_ext_lsb;
    unsigned int    pattern_img_orientation_h;
    unsigned int    pattern_img_orientation_v;
    unsigned int    pattern_hdr_mode_en;
    unsigned int    pattern_hdr_reso_redu_h;
    unsigned int    pattern_hdr_reso_redu_v;
    unsigned int    pattern_hdr_reso_redu_same;
    unsigned int    pattern_x_evn_inc;
    unsigned int    pattern_x_odd_inc;
    unsigned int    pattern_y_evn_inc;
    unsigned int    pattern_y_odd_inc;
    unsigned int    pattern_evn_inc_same;
    unsigned int    pattern_odd_inc_same;
    unsigned int    pattern_scale_mode;
    unsigned int    pattern_scale_m;
    unsigned int    pattern_binning_mode;
    unsigned int    pattern_binning_type_h;
    unsigned int    pattern_binning_type_v;
    unsigned int    pattern_binning_type_ext_en;
    unsigned int    pattern_binning_type_h_ext;
} ReadDefine_t;

/**************************************************************/
/*        prototype                                           */
/**************************************************************/
static unsigned int BitPick(unsigned int data, unsigned int msb, unsigned int lsb);
static int CallReadDefine(ReadDefine_t *data);

/**************************************************************/
/*        function : PdLibInterpretRegData                    */
/**************************************************************/
/* this function translates the 1st argument into the 2nd one. */
/* 1st argument is the set of sensor register data, 2nd argument is the semantic of sensor settings. */
/* return value: the set of register data is    1= wrong (not allowed),    0= correct */
extern int PdLibInterpretRegData(PdLibSensorCoordRegData_t *reg_data, PdLibSensorCoordSetting_t *setting)
{
    int             ret = D_PD_LIB_REGDATA_IS_OK;
    ReadDefine_t    data;

    if (CallReadDefine(&data) == DEFINE_OK) {

        setting->img_orientation_h      =  BitPick(reg_data->reg_addr_0x0101, data.img_orientation_h_msb        , data.img_orientation_h_lsb        );
        setting->img_orientation_v      =  BitPick(reg_data->reg_addr_0x0101, data.img_orientation_v_msb        , data.img_orientation_v_lsb        );

        setting->hdr_mode_en            =  BitPick(reg_data->reg_addr_0x0220, data.hdr_mode_en_msb              , data.hdr_mode_en_lsb              );
        setting->hdr_reso_redu_h        =  BitPick(reg_data->reg_addr_0x0221, data.hdr_reso_redu_h_msb          , data.hdr_reso_redu_h_lsb          );
        setting->hdr_reso_redu_v        =  BitPick(reg_data->reg_addr_0x0221, data.hdr_reso_redu_v_msb          , data.hdr_reso_redu_v_lsb          );

        setting->x_add_sta              = (BitPick(reg_data->reg_addr_0x0344, data.x_add_sta_1_msb              , data.x_add_sta_1_lsb              ) << 8)
                                         + BitPick(reg_data->reg_addr_0x0345, data.x_add_sta_2_msb              , data.x_add_sta_2_lsb              );
        setting->y_add_sta              = (BitPick(reg_data->reg_addr_0x0346, data.y_add_sta_1_msb              , data.y_add_sta_1_lsb              ) << 8)
                                         + BitPick(reg_data->reg_addr_0x0347, data.y_add_sta_2_msb              , data.y_add_sta_2_lsb              );

        setting->x_add_end              = (BitPick(reg_data->reg_addr_0x0348, data.x_add_end_1_msb              , data.x_add_end_1_lsb              ) << 8)
                                         + BitPick(reg_data->reg_addr_0x0349, data.x_add_end_2_msb              , data.x_add_end_2_lsb              );
        setting->y_add_end              = (BitPick(reg_data->reg_addr_0x034A, data.y_add_end_1_msb              , data.y_add_end_1_lsb              ) << 8)
                                         + BitPick(reg_data->reg_addr_0x034B, data.y_add_end_2_msb              , data.y_add_end_2_lsb              );

        setting->x_out_size             = (BitPick(reg_data->reg_addr_0x034C, data.x_out_size_1_msb             , data.x_out_size_1_lsb             ) << 8)
                                         + BitPick(reg_data->reg_addr_0x034D, data.x_out_size_2_msb             , data.x_out_size_2_lsb             );
        setting->y_out_size             = (BitPick(reg_data->reg_addr_0x034E, data.y_out_size_1_msb             , data.y_out_size_1_lsb             ) << 8)
                                         + BitPick(reg_data->reg_addr_0x034F, data.y_out_size_2_msb             , data.y_out_size_2_lsb             );

        setting->x_evn_inc              =  BitPick(reg_data->reg_addr_0x0381, data.x_evn_inc_msb                , data.x_evn_inc_lsb                );
        setting->x_odd_inc              =  BitPick(reg_data->reg_addr_0x0383, data.x_odd_inc_msb                , data.x_odd_inc_lsb                );
        setting->y_evn_inc              =  BitPick(reg_data->reg_addr_0x0385, data.y_evn_inc_msb                , data.y_evn_inc_lsb                );
        setting->y_odd_inc              =  BitPick(reg_data->reg_addr_0x0387, data.y_odd_inc_msb                , data.y_odd_inc_lsb                );

        setting->scale_mode             =  BitPick(reg_data->reg_addr_0x0401, data.scale_mode_msb               , data.scale_mode_lsb               );
        setting->scale_m                = (BitPick(reg_data->reg_addr_0x0404, data.scale_m_1_msb                , data.scale_m_1_lsb                ) << 8)
                                         + BitPick(reg_data->reg_addr_0x0405, data.scale_m_2_msb                , data.scale_m_2_lsb                );
        setting->scale_n                = data.scale_n;

        setting->dig_crop_x_offset      = (BitPick(reg_data->reg_addr_0x0408, data.dig_crop_x_offset_1_msb      , data.dig_crop_x_offset_1_lsb      ) << 8)
                                         + BitPick(reg_data->reg_addr_0x0409, data.dig_crop_x_offset_2_msb      , data.dig_crop_x_offset_2_lsb      );
        setting->dig_crop_y_offset      = (BitPick(reg_data->reg_addr_0x040A, data.dig_crop_y_offset_1_msb      , data.dig_crop_y_offset_1_lsb      ) << 8)
                                         + BitPick(reg_data->reg_addr_0x040B, data.dig_crop_y_offset_2_msb      , data.dig_crop_y_offset_2_lsb      );

        setting->dig_crop_image_width   = (BitPick(reg_data->reg_addr_0x040C, data.dig_crop_image_width_1_msb   , data.dig_crop_image_width_1_lsb   ) << 8)
                                         + BitPick(reg_data->reg_addr_0x040D, data.dig_crop_image_width_2_msb   , data.dig_crop_image_width_2_lsb   );
        setting->dig_crop_image_height  = (BitPick(reg_data->reg_addr_0x040E, data.dig_crop_image_height_1_msb  , data.dig_crop_image_height_1_lsb  ) << 8)
                                         + BitPick(reg_data->reg_addr_0x040F, data.dig_crop_image_height_2_msb  , data.dig_crop_image_height_2_lsb  );

        setting->binning_mode           =  BitPick(reg_data->reg_addr_0x0900, data.binning_mode_msb             , data.binning_mode_lsb             );
        setting->binning_type_h         =  BitPick(reg_data->reg_addr_0x0901, data.binning_type_h_msb           , data.binning_type_h_lsb           );
        setting->binning_type_v         =  BitPick(reg_data->reg_addr_0x0901, data.binning_type_v_msb           , data.binning_type_v_lsb           );
        setting->binning_type_ext_en    =  BitPick(reg_data->reg_addr_0x3F42, data.binning_type_ext_en_msb      , data.binning_type_ext_en_lsb      );
        setting->binning_type_h_ext     =  BitPick(reg_data->reg_addr_0x3F43, data.binning_type_h_ext_msb       , data.binning_type_h_ext_lsb       );

        if (ret == D_PD_LIB_REGDATA_IS_OK) {
            switch (data.pattern_img_orientation_h) {
            case  1:    if (UNMATCH_1(setting->img_orientation_h, 0))       ret = -D_EREG_IMG_ORI_H;    break;
            case  2:    if (UNMATCH_2(setting->img_orientation_h, 0, 1))    ret = -D_EREG_IMG_ORI_H;    break;
            default:                                                        ret = -D_EREG_DEFINE;       break;
            };
        }

        if (ret == D_PD_LIB_REGDATA_IS_OK) {
            switch (data.pattern_img_orientation_v) {
            case  1:    if (UNMATCH_1(setting->img_orientation_v, 0))       ret = -D_EREG_IMG_ORI_V;    break;
            case  2:    if (UNMATCH_2(setting->img_orientation_v, 0, 1))    ret = -D_EREG_IMG_ORI_V;    break;
            default:                                                        ret = -D_EREG_DEFINE;       break;
            };
        }

        if (ret == D_PD_LIB_REGDATA_IS_OK) {
            switch (data.pattern_hdr_mode_en) {
            case  1:    if (UNMATCH_1(setting->hdr_mode_en, 0))     ret = -D_EREG_HDR_MODE; break;
            case  2:    if (UNMATCH_2(setting->hdr_mode_en, 0, 1))  ret = -D_EREG_HDR_MODE; break;
            default:                                                ret = -D_EREG_DEFINE;   break;
            };
        }

        if (ret == D_PD_LIB_REGDATA_IS_OK) {
            switch (data.pattern_binning_mode) {
            case  1:    if (UNMATCH_1(setting->binning_mode, 0))    ret = -D_EREG_BINN_MODE;    break;
            case  2:    if (UNMATCH_2(setting->binning_mode, 0, 1)) ret = -D_EREG_BINN_MODE;    break;
            default:                                                ret = -D_EREG_DEFINE;       break;
            };
        }

        if (setting->hdr_mode_en == 1) {

            if (ret == D_PD_LIB_REGDATA_IS_OK) {
                switch (data.pattern_hdr_reso_redu_h) {
                case  1:    if (UNMATCH_1(setting->hdr_reso_redu_h, 1))     ret = -D_EREG_HDR_RESO_H;   break;
                case  2:    if (UNMATCH_2(setting->hdr_reso_redu_h, 1, 2))  ret = -D_EREG_HDR_RESO_H;   break;
                default:                                                    ret = -D_EREG_DEFINE;       break;
                };
            }

            if (ret == D_PD_LIB_REGDATA_IS_OK) {
                switch (data.pattern_hdr_reso_redu_v) {
                case  1:    if (UNMATCH_1(setting->hdr_reso_redu_v, 1))     ret = -D_EREG_HDR_RESO_V;   break;
                case  2:    if (UNMATCH_2(setting->hdr_reso_redu_v, 1, 2))  ret = -D_EREG_HDR_RESO_V;   break;
                default:                                                    ret = -D_EREG_DEFINE;       break;
                };
            }

            if (ret == D_PD_LIB_REGDATA_IS_OK) {
                switch (data.pattern_hdr_reso_redu_same) {
                case  1:    /* Do Nothing */                                                                                    break;
                case  2:    if (UNMATCH_1(setting->hdr_reso_redu_h, setting->hdr_reso_redu_v))  ret = -D_EREG_HDR_RESO_SAME;    break;
                default:                                                                        ret = -D_EREG_DEFINE;           break;
                };
            }

        } else {

            if (ret == D_PD_LIB_REGDATA_IS_OK) {
                switch (data.pattern_x_evn_inc) {
                case  1:    if (UNMATCH_1(setting->x_evn_inc, 1))           ret = -D_EREG_X_EVN_INC;    break;
                case  2:    if (UNMATCH_2(setting->x_evn_inc, 1, 3))        ret = -D_EREG_X_EVN_INC;    break;
                case  3:    if (UNMATCH_3(setting->x_evn_inc, 1, 3, 7))     ret = -D_EREG_X_EVN_INC;    break;
                case  4:    if (UNMATCH_4(setting->x_evn_inc, 1, 3, 7, 15)) ret = -D_EREG_X_EVN_INC;    break;
                default:                                                    ret = -D_EREG_DEFINE;       break;
                };
            }

            if (ret == D_PD_LIB_REGDATA_IS_OK) {
                switch (data.pattern_x_odd_inc) {
                case  1:    if (UNMATCH_1(setting->x_odd_inc, 1))           ret = -D_EREG_X_ODD_INC;    break;
                case  2:    if (UNMATCH_2(setting->x_odd_inc, 1, 3))        ret = -D_EREG_X_ODD_INC;    break;
                case  3:    if (UNMATCH_3(setting->x_odd_inc, 1, 3, 7))     ret = -D_EREG_X_ODD_INC;    break;
                case  4:    if (UNMATCH_4(setting->x_odd_inc, 1, 3, 7, 15)) ret = -D_EREG_X_ODD_INC;    break;
                default:                                                    ret = -D_EREG_DEFINE;       break;
                };
            }

            if (ret == D_PD_LIB_REGDATA_IS_OK) {
                switch (data.pattern_y_evn_inc) {
                case  1:    if (UNMATCH_1(setting->y_evn_inc, 1))           ret = -D_EREG_Y_EVN_INC;    break;
                case  2:    if (UNMATCH_2(setting->y_evn_inc, 1, 3))        ret = -D_EREG_Y_EVN_INC;    break;
                case  3:    if (UNMATCH_3(setting->y_evn_inc, 1, 3, 7))     ret = -D_EREG_Y_EVN_INC;    break;
                case  4:    if (UNMATCH_4(setting->y_evn_inc, 1, 3, 7, 15)) ret = -D_EREG_Y_EVN_INC;    break;
                default:                                                    ret = -D_EREG_DEFINE;       break;
                };
            }

            if (ret == D_PD_LIB_REGDATA_IS_OK) {
                switch (data.pattern_y_odd_inc) {
                case  1:    if (UNMATCH_1(setting->y_odd_inc, 1))           ret = -D_EREG_Y_ODD_INC;    break;
                case  2:    if (UNMATCH_2(setting->y_odd_inc, 1, 3))        ret = -D_EREG_Y_ODD_INC;    break;
                case  3:    if (UNMATCH_3(setting->y_odd_inc, 1, 3, 7))     ret = -D_EREG_Y_ODD_INC;    break;
                case  4:    if (UNMATCH_4(setting->y_odd_inc, 1, 3, 7, 15)) ret = -D_EREG_Y_ODD_INC;    break;
                default:                                                    ret = -D_EREG_DEFINE;       break;
                };
            }

            if (ret == D_PD_LIB_REGDATA_IS_OK) {
                switch (data.pattern_evn_inc_same) {
                case  1:    /* Do Nothing */                                                                    break;
                case  2:    if (UNMATCH_1(setting->x_evn_inc, setting->y_evn_inc))  ret = -D_EREG_EVN_INC_SAME; break;
                default:                                                            ret = -D_EREG_DEFINE;       break;
                };
            }

            if (ret == D_PD_LIB_REGDATA_IS_OK) {
                switch (data.pattern_odd_inc_same) {
                case  1:    /* Do Nothing */                                                                    break;
                case  2:    if (UNMATCH_1(setting->x_odd_inc, setting->y_odd_inc))  ret = -D_EREG_ODD_INC_SAME; break;
                default:                                                            ret = -D_EREG_DEFINE;       break;
                };
            }

            if (setting->binning_mode == 1) {
                if (ret == D_PD_LIB_REGDATA_IS_OK) {
                    unsigned int tmp_pattern_binning_type_h = (setting->binning_type_ext_en) ? 1 : data.pattern_binning_type_h;

                    switch (tmp_pattern_binning_type_h) {
                    case  1:    /* Do Nothing */                                                                break;
                    case  2:    if (UNMATCH_1(setting->binning_type_h, 1))          ret = -D_EREG_BINN_TYPE_H;  break;
                    case  3:    if (UNMATCH_1(setting->binning_type_h, 2))          ret = -D_EREG_BINN_TYPE_H;  break;
                    case  4:    if (UNMATCH_1(setting->binning_type_h, 4))          ret = -D_EREG_BINN_TYPE_H;  break;
                    case  5:    if (UNMATCH_2(setting->binning_type_h, 1, 2))       ret = -D_EREG_BINN_TYPE_H;  break;
                    case  6:    if (UNMATCH_2(setting->binning_type_h, 1, 4))       ret = -D_EREG_BINN_TYPE_H;  break;
                    case  7:    if (UNMATCH_2(setting->binning_type_h, 2, 4))       ret = -D_EREG_BINN_TYPE_H;  break;
                    case  8:    if (UNMATCH_3(setting->binning_type_h, 1, 2, 4))    ret = -D_EREG_BINN_TYPE_H;  break;
                    case  9:    if (UNMATCH_4(setting->binning_type_h, 1, 2, 4, 8)) ret = -D_EREG_BINN_TYPE_H;  break;
                    default:                                                        ret = -D_EREG_DEFINE;       break;
                    };
                }

                if (ret == D_PD_LIB_REGDATA_IS_OK) {
                    switch (data.pattern_binning_type_v) {
                    case  1:    /* Do Nothing */                                                                break;
                    case  2:    if (UNMATCH_1(setting->binning_type_v, 1))          ret = -D_EREG_BINN_TYPE_V;  break;
                    case  3:    if (UNMATCH_1(setting->binning_type_v, 2))          ret = -D_EREG_BINN_TYPE_V;  break;
                    case  4:    if (UNMATCH_1(setting->binning_type_v, 4))          ret = -D_EREG_BINN_TYPE_V;  break;
                    case  5:    if (UNMATCH_2(setting->binning_type_v, 1, 2))       ret = -D_EREG_BINN_TYPE_V;  break;
                    case  6:    if (UNMATCH_2(setting->binning_type_v, 1, 4))       ret = -D_EREG_BINN_TYPE_V;  break;
                    case  7:    if (UNMATCH_2(setting->binning_type_v, 2, 4))       ret = -D_EREG_BINN_TYPE_V;  break;
                    case  8:    if (UNMATCH_3(setting->binning_type_v, 1, 2, 4))    ret = -D_EREG_BINN_TYPE_V;  break;
                    case  9:    if (UNMATCH_4(setting->binning_type_v, 1, 2, 4, 8)) ret = -D_EREG_BINN_TYPE_V;  break;
                    default:                                                        ret = -D_EREG_DEFINE;       break;
                    };
                }

                if (ret == D_PD_LIB_REGDATA_IS_OK) {
                    switch (data.pattern_binning_type_ext_en) {
                    case  1:    if (UNMATCH_1(setting->binning_type_ext_en, 0))     ret = -D_EREG_BINN_TYPE_EXT_EN; break;
                    case  2:    if (UNMATCH_2(setting->binning_type_ext_en, 0, 1))  ret = -D_EREG_BINN_TYPE_EXT_EN; break;
                    default:                                                        ret = -D_EREG_DEFINE;           break;
                    };
                }

                if (setting->binning_type_ext_en == 1) {
                    if (ret == D_PD_LIB_REGDATA_IS_OK) {
                        switch (data.pattern_binning_type_h_ext) {
                        case  1:    if (UNMATCH_1(setting->binning_type_h_ext, 0))      ret = -D_EREG_BINN_TYPE_H_EXT;  break;
                        case  2:    if (UNMATCH_2(setting->binning_type_h_ext, 0, 1))   ret = -D_EREG_BINN_TYPE_H_EXT;  break;
                        default:                                                        ret = -D_EREG_DEFINE;           break;
                        };
                    }
                }
            }
        }

        if (ret == D_PD_LIB_REGDATA_IS_OK) {
            switch (data.pattern_scale_mode) {
            case  1:    if (UNMATCH_1(setting->scale_mode, 0))          ret = -D_EREG_SCALE_MODE;   break;
            case  2:    if (UNMATCH_2(setting->scale_mode, 0, 1))       ret = -D_EREG_SCALE_MODE;   break;
            case  3:    if (UNMATCH_3(setting->scale_mode, 0, 1, 2))    ret = -D_EREG_SCALE_MODE;   break;
            default:                                                    ret = -D_EREG_DEFINE;       break;
            };
        }

        if (setting->scale_mode != 0) {
            if (ret == D_PD_LIB_REGDATA_IS_OK) {
                switch (data.pattern_scale_m) {
                case  1:    if (OUT_RANGE(setting->scale_m,  16,  511))     ret = -D_EREG_SCALE_M;  break;
                case  2:    if (OUT_RANGE(setting->scale_m, 256, 8191))     ret = -D_EREG_SCALE_M;  break;
                case  3:    if (UNMATCH_3(setting->scale_m, 16, 24, 32))    ret = -D_EREG_SCALE_M;  break;
                default:                                                    ret = -D_EREG_DEFINE;   break;
                };
            }

            if (ret == D_PD_LIB_REGDATA_IS_OK) {
                if (setting->scale_n == 0) {
                    ret = -D_EREG_SCALE_N;
                }
            }
        }

    } else {
        ret = -D_EREG_DEFINE;
    }

    return ret;
}

/**************************************************************/
/*        function : PdLibTransOutputPointToPdafPoint         */
/**************************************************************/
/* this function transforms a point from coordinate system of the sensor output image to the one of the PDAF library. */
/* 1st argument is coordinates in the output image, 2nd argument is the sensor settings. */
/* return value: coordinates in the PDAF library */
extern PdLibPoint_t PdLibTransOutputPointToPdafPoint(PdLibPoint_t point, PdLibSensorCoordSetting_t *setting)
{
    PdLibPoint_t    ret;
    unsigned int    x4, y4;
    unsigned int    x3, y3;
    unsigned int    x2, y2;
    unsigned int    x1, y1;
    unsigned int    x0, y0;
    unsigned int    mag_scaleX, mag_scaleY, mag_binsubX = 1, mag_binsubY = 1;
    unsigned int    scale_n = (setting->scale_n > 0) ? setting->scale_n : 1;

    /* not need to think the output crop in this transformation */
    x4 = (unsigned int)point.x;
    y4 = (unsigned int)point.y;

    /* inverse scaling */
    switch (setting->scale_mode) {
    case 1:
        mag_scaleX = setting->scale_m;
        mag_scaleY = scale_n;
        break;
    case 2:
        mag_scaleX = setting->scale_m;
        mag_scaleY = setting->scale_m;
        break;
    default:
        mag_scaleX = scale_n;
        mag_scaleY = scale_n;
        break;
    }
    x3 = x4 * mag_scaleX / scale_n + (mag_scaleX / scale_n) / 2; /* the fraction is compensation for round off */
    y3 = y4 * mag_scaleY / scale_n + (mag_scaleY / scale_n) / 2; /* the fraction is compensation for round off */

    /* inverse digital crop */
    x2 = x3 + setting->dig_crop_x_offset;
    y2 = y3 + setting->dig_crop_y_offset;

    /* inverse binning and sub-sampling */
    if (setting->hdr_mode_en == 0) {
        /* Normal capture mode */
        if (setting->binning_mode == 1) {
            unsigned int tmp_binning_type_h;

            if (setting->binning_type_ext_en == 1) {
                tmp_binning_type_h = (setting->binning_type_h_ext << 4) + setting->binning_type_h;
            } else {
                tmp_binning_type_h = setting->binning_type_h;
            }

            mag_binsubX = ((tmp_binning_type_h == 0) ? 1 : tmp_binning_type_h);
            mag_binsubY = ((setting->binning_type_v == 0) ? 1 : setting->binning_type_v);
        }

        mag_binsubX *= ((setting->x_odd_inc + setting->x_evn_inc) / 2);
        mag_binsubY *= ((setting->y_odd_inc + setting->y_evn_inc) / 2);
    } else {
        /* HDR capture mode */
        mag_binsubX = setting->hdr_reso_redu_h;
        mag_binsubY = setting->hdr_reso_redu_v;
    }
    x1 = x2 * mag_binsubX + (mag_binsubX / 2); /* the fraction is compensation for round off */
    y1 = y2 * mag_binsubY + (mag_binsubY / 2); /* the fraction is compensation for round off */

    /* inverse analog crop */
    if (setting->img_orientation_h == 0) {
        x0 = x1 + setting->x_add_sta;
    } else {
        /* mirroring */
        x0 = setting->x_add_end - x1;
    }
    if (setting->img_orientation_v == 0) {
        y0 = y1 + setting->y_add_sta;
    } else {
        /* flipping */
        y0 = setting->y_add_end - y1;
    }

    ret.x = (signed int)x0;
    ret.y = (signed int)y0;

    return ret;
}

/**************************************************************/
/*        function : PdLibTransOutputRectToPdafRect           */
/**************************************************************/
/* this function transforms a rectangle from coordinate system of the sensor output image to the one of the PDAF library. */
/* 1st argument is coordinates in the output image, 2nd argument is the sensor settings. */
/* return value: coordinates in the PDAF library */
extern PdLibRect_t PdLibTransOutputRectToPdafRect(PdLibRect_t rect, PdLibSensorCoordSetting_t *setting)
{
    PdLibRect_t ret;

    if ((rect.end.x < rect.sta.x) ||
        (rect.end.y < rect.sta.y)) {
        /* should be 'end' >= 'sta' */
        ret.sta.x = ret.sta.y = D_PD_LIB_COORD_ERROR;
        ret.end.x = ret.end.y = D_PD_LIB_COORD_ERROR;
    } else {
        /* mirroring */
        if (setting->img_orientation_h == 1) {
            int x0, x1;

            x0 = rect.end.x;
            x1 = rect.sta.x;
            rect.sta.x = x0;
            rect.end.x = x1;
        }

        /* flipping */
        if (setting->img_orientation_v == 1) {
            int y0, y1;

            y0 = rect.end.y;
            y1 = rect.sta.y;
            rect.sta.y = y0;
            rect.end.y = y1;
        }

        ret.sta = PdLibTransOutputPointToPdafPoint(rect.sta, setting);
        ret.end = PdLibTransOutputPointToPdafPoint(rect.end, setting);
    }

    return ret;
}

/**************************************************************/
/*        function : BitPick                                  */
/**************************************************************/
/* pick up a bit field. (When 'MSBpos' is the same as 'LSBpos', single bit is extracted.) */
static unsigned int BitPick(unsigned int data, unsigned int msb, unsigned int lsb)
{
    return (data >> lsb) & (~(~0U << (msb - lsb + 1)));
}

/**************************************************************/
/*        function : CallReadDefine                           */
/**************************************************************/
static int CallReadDefine(ReadDefine_t *data)
{
    int             ret = DEFINE_OK;

	data->img_orientation_h_msb 		= 0x0;
	data->img_orientation_h_lsb 		= 0x0;
	data->img_orientation_v_msb 		= 0x1;
	data->img_orientation_v_lsb 		= 0x1;
	data->hdr_mode_en_msb				= 0x0;
	data->hdr_mode_en_lsb				= 0x0;
	data->hdr_reso_redu_h_msb			= 0x7;
	data->hdr_reso_redu_h_lsb			= 0x4;
	data->hdr_reso_redu_v_msb			= 0x3;
	data->hdr_reso_redu_v_lsb			= 0x0;
	data->x_add_sta_1_msb				= 0x4;
	data->x_add_sta_1_lsb				= 0x0;
	data->x_add_sta_2_msb				= 0x7;
	data->x_add_sta_2_lsb				= 0x0;
	data->y_add_sta_1_msb				= 0x3;
	data->y_add_sta_1_lsb				= 0x0;
	data->y_add_sta_2_msb				= 0x7;
	data->y_add_sta_2_lsb				= 0x0;
	data->x_add_end_1_msb				= 0x4;
	data->x_add_end_1_lsb				= 0x0;
	data->x_add_end_2_msb				= 0x7;
	data->x_add_end_2_lsb				= 0x0;
	data->y_add_end_1_msb				= 0x3;
	data->y_add_end_1_lsb				= 0x0;
	data->y_add_end_2_msb				= 0x7;
	data->y_add_end_2_lsb				= 0x0;
	data->x_out_size_1_msb				= 0x4;
	data->x_out_size_1_lsb				= 0x0;
	data->x_out_size_2_msb				= 0x7;
	data->x_out_size_2_lsb				= 0x0;
	data->y_out_size_1_msb				= 0x3;
	data->y_out_size_1_lsb				= 0x0;
	data->y_out_size_2_msb				= 0x7;
	data->y_out_size_2_lsb				= 0x0;
	data->x_evn_inc_msb 				= 0x2;
	data->x_evn_inc_lsb 				= 0x0;
	data->x_odd_inc_msb 				= 0x2;
	data->x_odd_inc_lsb 				= 0x0;
	data->y_evn_inc_msb 				= 0x3;
	data->y_evn_inc_lsb 				= 0x0;
	data->y_odd_inc_msb 				= 0x3;
	data->y_odd_inc_lsb 				= 0x0;
	data->scale_mode_msb				= 0x1;
	data->scale_mode_lsb				= 0x0;
	data->scale_m_1_msb 				= 0x0;
	data->scale_m_1_lsb 				= 0x0;
	data->scale_m_2_msb 				= 0x7;
	data->scale_m_2_lsb 				= 0x0;
	data->scale_n						= 0x10;
	data->dig_crop_x_offset_1_msb		= 0x4;
	data->dig_crop_x_offset_1_lsb		= 0x0;
	data->dig_crop_x_offset_2_msb		= 0x7;
	data->dig_crop_x_offset_2_lsb		= 0x0;
	data->dig_crop_y_offset_1_msb		= 0x3;
	data->dig_crop_y_offset_1_lsb		= 0x0;
	data->dig_crop_y_offset_2_msb		= 0x7;
	data->dig_crop_y_offset_2_lsb		= 0x0;
	data->dig_crop_image_width_1_msb	= 0x4;
	data->dig_crop_image_width_1_lsb	= 0x0;
	data->dig_crop_image_width_2_msb	= 0x7;
	data->dig_crop_image_width_2_lsb	= 0x0;
	data->dig_crop_image_height_1_msb	= 0x3;
	data->dig_crop_image_height_1_lsb	= 0x0;
	data->dig_crop_image_height_2_msb	= 0x7;
	data->dig_crop_image_height_2_lsb	= 0x0;
	data->binning_mode_msb				= 0x0;
	data->binning_mode_lsb				= 0x0;
	data->binning_type_h_msb			= 0x7;
	data->binning_type_h_lsb			= 0x4;
	data->binning_type_v_msb			= 0x3;
	data->binning_type_v_lsb			= 0x0;
	data->binning_type_ext_en_msb		= 0x0;
	data->binning_type_ext_en_lsb		= 0x0;
	data->binning_type_h_ext_msb		= 0x0;
	data->binning_type_h_ext_lsb		= 0x0;
	data->pattern_img_orientation_h 	= 0x2;
	data->pattern_img_orientation_v 	= 0x2;
	data->pattern_hdr_mode_en			= 0x2;
	data->pattern_hdr_reso_redu_h		= 0x1;
	data->pattern_hdr_reso_redu_v		= 0x2;
	data->pattern_hdr_reso_redu_same	= 0x1;
	data->pattern_x_evn_inc 			= 0x1;
	data->pattern_x_odd_inc 			= 0x1;
	data->pattern_y_evn_inc 			= 0x1;
	data->pattern_y_odd_inc 			= 0x1;
	data->pattern_evn_inc_same			= 0x2;
	data->pattern_odd_inc_same			= 0x2;
	data->pattern_scale_mode			= 0x3;
	data->pattern_scale_m				= 0x1;
	data->pattern_binning_mode			= 0x2;
	data->pattern_binning_type_h		= 0x5;
	data->pattern_binning_type_v		= 0x5;
	data->pattern_binning_type_ext_en	= 0x1;
	data->pattern_binning_type_h_ext	= 0x1;

	return ret;
}
