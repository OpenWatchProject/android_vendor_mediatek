/****************************************************************
Copyright 2014-2015 Sony Corporation
****************************************************************/

/****************************************************************/
/*                          include                             */
/****************************************************************/

#include "SonyPdafTransCoord.h"
#include <aaa_log.h>


#define LOG_TAG "SonyPdafTransCoord"

/****************************************************************/
/*                 local function declaration                   */
/****************************************************************/

// pick up a bit field. (When 'MSBpos' is the same as 'LSBpos', single bit is extracted.)
static unsigned int bitPick(unsigned int x, int MSBpos, int LSBpos)
{
    return (x >> LSBpos) & (~(~0 << (MSBpos - LSBpos + 1)));
}

/****************************************************************/
/*                      external function                       */
/****************************************************************/

// this function translates the 1st argument into the 2nd one.
// 1st argument is the set of sensor register data, 2nd argument is the semantic of sensor settings.
// return value: the set of register data is    1= wrong (not allowed),    0= correct
int SonyPdLibInterpretRegData(SonyPdLibSensorCoordRegData_t *p_regData, SonyPdLibSensorCoordSetting_t *p_setting)
{
    int ret = D_SONY_PD_LIB_REGDATA_IS_OK;

    p_setting->img_orientation_h = bitPick(p_regData->reg_addr_0x0101, 0, 0);
    p_setting->img_orientation_v = bitPick(p_regData->reg_addr_0x0101, 1, 1);

    p_setting->hdr_mode_en = bitPick(p_regData->reg_addr_0x0220, 0, 0);
    p_setting->hdr_reso_redu_h = bitPick(p_regData->reg_addr_0x0221, 7, 4);
    p_setting->hdr_reso_redu_v = bitPick(p_regData->reg_addr_0x0221, 3, 0);

    p_setting->x_add_sta = (bitPick(p_regData->reg_addr_0x0344, 4, 0) << 8) + bitPick(p_regData->reg_addr_0x0345, 7, 0);
    p_setting->y_add_sta = (bitPick(p_regData->reg_addr_0x0346, 3, 0) << 8) + bitPick(p_regData->reg_addr_0x0347, 7, 0);
    p_setting->x_add_end = (bitPick(p_regData->reg_addr_0x0348, 4, 0) << 8) + bitPick(p_regData->reg_addr_0x0349, 7, 0);
    p_setting->y_add_end = (bitPick(p_regData->reg_addr_0x034A, 3, 0) << 8) + bitPick(p_regData->reg_addr_0x034B, 7, 0);

    p_setting->x_out_size = (bitPick(p_regData->reg_addr_0x034C, 4, 0) << 8) + bitPick(p_regData->reg_addr_0x034D, 7, 0);
    p_setting->y_out_size = (bitPick(p_regData->reg_addr_0x034E, 4, 0) << 8) + bitPick(p_regData->reg_addr_0x034F, 7, 0);

    p_setting->x_evn_inc = bitPick(p_regData->reg_addr_0x0381, 2, 0);
    p_setting->x_odd_inc = bitPick(p_regData->reg_addr_0x0383, 2, 0);
    p_setting->y_evn_inc = bitPick(p_regData->reg_addr_0x0385, 3, 0);
    p_setting->y_odd_inc = bitPick(p_regData->reg_addr_0x0387, 3, 0);

    p_setting->scale_mode = bitPick(p_regData->reg_addr_0x0401, 1, 0);
    p_setting->scale_m = (bitPick(p_regData->reg_addr_0x0404, 0, 0) << 8) + bitPick(p_regData->reg_addr_0x0405, 7, 0);

    p_setting->dig_crop_x_offset     = (bitPick(p_regData->reg_addr_0x0408, 4, 0) << 8) + bitPick(p_regData->reg_addr_0x0409, 7, 0);
    p_setting->dig_crop_y_offset     = (bitPick(p_regData->reg_addr_0x040A, 3, 0) << 8) + bitPick(p_regData->reg_addr_0x040B, 7, 0);
    p_setting->dig_crop_image_width  = (bitPick(p_regData->reg_addr_0x040C, 4, 0) << 8) + bitPick(p_regData->reg_addr_0x040D, 7, 0);
    p_setting->dig_crop_image_height = (bitPick(p_regData->reg_addr_0x040E, 3, 0) << 8) + bitPick(p_regData->reg_addr_0x040F, 7, 0);

    p_setting->binning_mode = bitPick(p_regData->reg_addr_0x0900, 0, 0);
    p_setting->binning_type_h = bitPick(p_regData->reg_addr_0x0901, 7, 4);
    p_setting->binning_type_v = bitPick(p_regData->reg_addr_0x0901, 3, 0);


    if (p_setting->hdr_reso_redu_h != p_setting->hdr_reso_redu_v) {
        ret = D_SONY_PD_LIB_REGDATA_IS_NG;
    }
    if ((p_setting->y_evn_inc != 1) || (p_setting->x_evn_inc != 1)) {
        ret = D_SONY_PD_LIB_REGDATA_IS_NG;
    }
    if (p_setting->y_odd_inc != p_setting->x_odd_inc) {
        ret = D_SONY_PD_LIB_REGDATA_IS_NG;
    }
    if (p_setting->binning_type_h != p_setting->binning_type_v) {
        ret = D_SONY_PD_LIB_REGDATA_IS_NG;
    }

    return ret;
}


// this function transforms a point from coordinate system of the sensor output image to the one of the PDAF library.
// 1st argument is coordinates in the output image, 2nd argument is the sensor settings.
// return value: coordinates in the PDAF library
SonyPdLibPoint_t SonyPdLibTransOutputPointToPdafPoint(SonyPdLibPoint_t onOutImage, SonyPdLibSensorCoordSetting_t *p_setting)
{
    SonyPdLibPoint_t ret;
    int x4, y4;
    int x3, y3;
    int x2, y2;
    int x1, y1;
    int x0, y0;
    int mag_scaleX, mag_scaleY, mag_binsub;

    // not need to think the output crop in this transformation
    x4 = onOutImage.x;
    y4 = onOutImage.y;

    // inverse scaling
    switch (p_setting->scale_mode) {
    case 1:
        mag_scaleX = p_setting->scale_m;
        mag_scaleY = 16;
        break;
    case 2:
        mag_scaleX = p_setting->scale_m;
        mag_scaleY = p_setting->scale_m;
        break;
    default:
        mag_scaleX = 16;
        mag_scaleY = 16;
        break;
    }
    x3 = x4 * mag_scaleX / 16 + (mag_scaleX / 16) / 2; // the fraction is compensation for round off
    y3 = y4 * mag_scaleY / 16 + (mag_scaleY / 16) / 2; // the fraction is compensation for round off

    // inverse digital crop
    x2 = x3 + p_setting->dig_crop_x_offset;
    y2 = y3 + p_setting->dig_crop_y_offset;

    // inverse binning and sub-sampling
    if (p_setting->hdr_mode_en == 0) {
        // Normal capture mode
        // check only binning_type_v, because binning_type_h should be the same value.
        mag_binsub = p_setting->binning_type_v;
        // check only y_odd_inc, because x_odd_inc should be the same value.
        if (p_setting->y_odd_inc == 3) {
            mag_binsub *= 2;
        }
    } else {
        // HDR capture mode
        // check only hdr_reso_redu_v, because hdr_reso_redu_h should be the same value.
        mag_binsub = p_setting->hdr_reso_redu_v;
    }
    x1 = x2 * mag_binsub + (mag_binsub / 2); // the fraction is compensation for round off
    y1 = y2 * mag_binsub + (mag_binsub / 2); // the fraction is compensation for round off

    // inverse analog crop
    if (p_setting->img_orientation_h == 0) {
        x0 = x1 + p_setting->x_add_sta;
    } else {
        // mirroring
        x0 = p_setting->x_add_end - x1;
    }
    if (p_setting->img_orientation_v == 0) {
        y0 = y1 + p_setting->y_add_sta;
    } else {
        // flipping
        y0 = p_setting->y_add_end - y1;
    }

    ret.x = x0;
    ret.y = y0;
	MY_LOG("x: %4d  %4d  %4d  %4d  %4d\n", x4, x3, x2, x1, x0);
	MY_LOG("y: %4d  %4d  %4d  %4d  %4d\n", y4, y3, y2, y1, y0);

    return ret;
}

// this function transforms a rectangle from coordinate system of the sensor output image to the one of the PDAF library.
// 1st argument is coordinates in the output image, 2nd argument is the sensor settings.
// return value: coordinates in the PDAF library
SonyPdLibRect_t SonyPdLibTransOutputRectToPdafRect(SonyPdLibRect_t onOutImage, SonyPdLibSensorCoordSetting_t *p_setting)
{
    SonyPdLibRect_t ret;

    if ((onOutImage.end.x < onOutImage.sta.x) || (onOutImage.end.y < onOutImage.sta.y)) {
        // should be 'end' >= 'sta'
        ret.sta.x = ret.sta.y = D_SONY_PD_LIB_COORD_ERROR;
        ret.end.x = ret.end.y = D_SONY_PD_LIB_COORD_ERROR;
    } else {
        ret.sta = SonyPdLibTransOutputPointToPdafPoint(onOutImage.sta, p_setting);
        ret.end = SonyPdLibTransOutputPointToPdafPoint(onOutImage.end, p_setting);
    }

    return ret;
}
