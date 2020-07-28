/****************************************************************
Copyright 2014-2015 Sony Corporation
****************************************************************/

#ifndef __SONY_PDAF_TRANS_COORD_H__
#define __SONY_PDAF_TRANS_COORD_H__


#define D_SONY_PD_LIB_REGDATA_IS_OK				(0)
#define D_SONY_PD_LIB_REGDATA_IS_NG				(1)

#define D_SONY_PD_LIB_COORD_ERROR				(-1)


typedef struct
{
    unsigned int		reg_addr_0x0101;
    unsigned int		reg_addr_0x0220;
    unsigned int		reg_addr_0x0221;
    unsigned int		reg_addr_0x0344;
    unsigned int		reg_addr_0x0345;
    unsigned int		reg_addr_0x0346;
    unsigned int		reg_addr_0x0347;
    unsigned int		reg_addr_0x0348;
    unsigned int		reg_addr_0x0349;
    unsigned int		reg_addr_0x034A;
    unsigned int		reg_addr_0x034B;
    unsigned int		reg_addr_0x034C;
    unsigned int		reg_addr_0x034D;
    unsigned int		reg_addr_0x034E;
    unsigned int		reg_addr_0x034F;
    unsigned int		reg_addr_0x0381;
    unsigned int		reg_addr_0x0383;
    unsigned int		reg_addr_0x0385;
    unsigned int		reg_addr_0x0387;
    unsigned int		reg_addr_0x0401;
    unsigned int		reg_addr_0x0404;
    unsigned int		reg_addr_0x0405;
    unsigned int		reg_addr_0x0408;
    unsigned int		reg_addr_0x0409;
    unsigned int		reg_addr_0x040A;
    unsigned int		reg_addr_0x040B;
    unsigned int		reg_addr_0x040C;
    unsigned int		reg_addr_0x040D;
    unsigned int		reg_addr_0x040E;
    unsigned int		reg_addr_0x040F;
    unsigned int		reg_addr_0x0900;
    unsigned int		reg_addr_0x0901;
} SonyPdLibSensorCoordRegData_t;

typedef struct _SonyPdLibSensorCoordSetting_t
{
    unsigned int		img_orientation_h;
    unsigned int		img_orientation_v;
    unsigned int		hdr_mode_en;
    unsigned int		hdr_reso_redu_h;
    unsigned int		hdr_reso_redu_v;
    unsigned int		x_add_sta;
    unsigned int		y_add_sta;
    unsigned int		x_add_end;
    unsigned int		y_add_end;
    unsigned int		x_out_size;
    unsigned int		y_out_size;
    unsigned int		x_evn_inc;
    unsigned int		x_odd_inc;
    unsigned int		y_evn_inc;
    unsigned int		y_odd_inc;
    unsigned int		scale_mode;
    unsigned int		scale_m;
    unsigned int		dig_crop_x_offset;
    unsigned int		dig_crop_y_offset;
    unsigned int		dig_crop_image_width;
    unsigned int		dig_crop_image_height;
    unsigned int		binning_mode;
    unsigned int		binning_type_h;
    unsigned int		binning_type_v;
} SonyPdLibSensorCoordSetting_t;

typedef struct
{
    signed int			x;
    signed int			y;
} SonyPdLibPoint_t;

typedef struct
{
    SonyPdLibPoint_t	sta;
    SonyPdLibPoint_t	end;
} SonyPdLibRect_t;

#ifdef __cplusplus
extern "C" {
#endif

extern int SonyPdLibInterpretRegData(SonyPdLibSensorCoordRegData_t *p_regData, SonyPdLibSensorCoordSetting_t *p_setting);

extern SonyPdLibPoint_t SonyPdLibTransOutputPointToPdafPoint(SonyPdLibPoint_t onOutImage, SonyPdLibSensorCoordSetting_t *p_setting);

extern SonyPdLibRect_t SonyPdLibTransOutputRectToPdafRect(SonyPdLibRect_t onOutImage, SonyPdLibSensorCoordSetting_t *p_setting);


#ifdef __cplusplus
}
#endif




#endif // __SONY_PDAF_TRANS_COORD_H__
