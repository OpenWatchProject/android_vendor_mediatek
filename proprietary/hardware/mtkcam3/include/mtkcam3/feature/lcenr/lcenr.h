#ifndef __LCENR_H__
#define __LCENR_H__

#include <mtkcam/drv/def/Dip_Notify_datatype.h>

inline void lcenr_srz_settings(int rrz_in_wd, int rrz_in_ht, int rrz_out_wd, int rrz_out_ht, int rrz_xoff, int rrz_yoff, int rrz_crop_in_wd, int rrz_crop_in_ht, int lce_full_slm_wd, int lce_full_slm_ht,
                unsigned int &srz_in_wd, unsigned int &srz_in_ht, unsigned long &srz_in_crop_wd, unsigned long &srz_in_crop_ht, unsigned int &srz_out_wd, unsigned int &srz_out_ht, unsigned int &srz_crop_x, unsigned int &srz_crop_y, unsigned int &srz_crop_floatx, unsigned int &srz_crop_floaty)
{
    srz_in_wd = lce_full_slm_wd;
    srz_in_ht = lce_full_slm_ht;
    srz_in_crop_wd = (lce_full_slm_wd * rrz_crop_in_wd + (rrz_in_wd >> 1)) / rrz_in_wd;
    srz_in_crop_ht = (lce_full_slm_ht * rrz_crop_in_ht + (rrz_in_ht >> 1)) / rrz_in_ht;
    srz_out_wd = rrz_out_wd;
    srz_out_ht = rrz_out_ht;
    // Calculate int offset and float offset.
    unsigned long long tmp_x = ((unsigned long long)rrz_xoff * lce_full_slm_wd);
    unsigned long long tmp_y = ((unsigned long long)rrz_yoff * lce_full_slm_ht);
    unsigned long long tmp_crop_x = ((tmp_x << 31) + (rrz_in_wd >> 1)) / rrz_in_wd;
    unsigned long long tmp_crop_y = ((tmp_y << 31) + (rrz_in_ht >> 1)) / rrz_in_ht;
    srz_crop_x = (unsigned int)(tmp_crop_x >> 31);
    srz_crop_y = (unsigned int)(tmp_crop_y >> 31);
    srz_crop_floatx = (unsigned int)(tmp_crop_x & ((unsigned long long)(1 << 31) - 1));
    srz_crop_floaty = (unsigned int)(tmp_crop_y & ((unsigned long long)(1 << 31) - 1));
}

struct LCENR_IN_PARAMS
{
    MBOOL resized;
    NSCam::MSize p2_in;
    NSCam::MSize rrz_in;
    NSCam::MRect rrz_crop_in;
    NSCam::MSize rrz_out;
    NSCam::MSize lce_full;
};

struct LCENR_OUT_PARAMS
{
    _SRZ_SIZE_INFO_ srz4Param;
};

inline void calculateLCENRConfig(LCENR_IN_PARAMS in, LCENR_OUT_PARAMS &out)
{
    if( in.resized )
    {
        lcenr_srz_settings(
            in.rrz_in.w,
            in.rrz_in.h,
            in.rrz_out.w,
            in.rrz_out.h,
            in.rrz_crop_in.p.x,
            in.rrz_crop_in.p.y,
            in.rrz_crop_in.s.w,
            in.rrz_crop_in.s.h,
            in.lce_full.w,
            in.lce_full.h,
            out.srz4Param.in_w,
            out.srz4Param.in_h,
            out.srz4Param.crop_w,
            out.srz4Param.crop_h,
            out.srz4Param.out_w,
            out.srz4Param.out_h,
            out.srz4Param.crop_x,
            out.srz4Param.crop_y,
            out.srz4Param.crop_floatX,
            out.srz4Param.crop_floatY);
    }
    else
    {
        out.srz4Param.in_w = in.lce_full.w;
        out.srz4Param.in_h = in.lce_full.h;
        out.srz4Param.crop_floatX = 0;
        out.srz4Param.crop_floatY = 0;
        out.srz4Param.crop_x = 0;
        out.srz4Param.crop_y = 0;
        out.srz4Param.crop_w = in.lce_full.w;
        out.srz4Param.crop_h = in.lce_full.h;
        out.srz4Param.out_w = in.p2_in.w;
        out.srz4Param.out_h = in.p2_in.h;
    }
}

struct FACENR_IN_PARAMS
{
    NSCam::MSize p2_in;
    NSCam::MSize face_map;
};

struct FACENR_OUT_PARAMS
{
    _SRZ_SIZE_INFO_ srz3Param;
};

inline void calculateFACENRConfig(FACENR_IN_PARAMS in, FACENR_OUT_PARAMS &out)
{
    out.srz3Param.in_w = in.face_map.w;
    out.srz3Param.in_h = in.face_map.h;
    out.srz3Param.crop_floatX = 0;
    out.srz3Param.crop_floatY = 0;
    out.srz3Param.crop_x = 0;
    out.srz3Param.crop_y = 0;
    out.srz3Param.crop_w = in.face_map.w;
    out.srz3Param.crop_h = in.face_map.h;
    out.srz3Param.out_w = in.p2_in.w;
    out.srz3Param.out_h = in.p2_in.h;
}

#endif /* __LCENR_H__ */