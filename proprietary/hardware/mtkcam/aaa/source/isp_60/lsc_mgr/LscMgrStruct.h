#ifndef _LSC_MGR_DEFAULT_STRUCT_H_
#define _LSC_MGR_DEFAULT_STRUCT_H_

typedef struct
{
    MUINT32 u4SensorPreviewWidth;
    MUINT32 u4SensorPreviewHeight;
    MUINT32 u4SensorCaptureWidth;
    MUINT32 u4SensorCaptureHeight;
    MUINT32 u4SensorVideoWidth;
    MUINT32 u4SensorVideoHeight;
    MUINT32 u4SensorVideo1Width;
    MUINT32 u4SensorVideo1Height;
    MUINT32 u4SensorVideo2Width;
    MUINT32 u4SensorVideo2Height;
    MUINT32 u4SensorCustom1Width;   // new for custom
    MUINT32 u4SensorCustom1Height;
    MUINT32 u4SensorCustom2Width;
    MUINT32 u4SensorCustom2Height;
    MUINT32 u4SensorCustom3Width;
    MUINT32 u4SensorCustom3Height;
    MUINT32 u4SensorCustom4Width;
    MUINT32 u4SensorCustom4Height;
    MUINT32 u4SensorCustom5Width;
    MUINT32 u4SensorCustom5Height;
} SENSOR_RESOLUTION_INFO_T;

struct SensorCropInfo_T
{
    MUINT32  w0;    // original full width
    MUINT32  h0;    // original full height
    MUINT32  x1;    // crop_1 x offset from full_0
    MUINT32  y1;    // crop_1 y offset from full_0
    MUINT32  w1;    // crop_1 width from full_0
    MUINT32  h1;    // crop_1 height from full_0
    MUINT32  w1r;   // scaled width from crop_1, w1 * r
    MUINT32  h1r;   // scaled height from crop_1, h1 * r
    MUINT32  x2;    // crop_2 x offset from scaled crop_1
    MUINT32  y2;    // crop_2 y offset from scaled crop_1
    MUINT32  w2;    // crop_2 width from scaled crop_1
    MUINT32  h2;    // crop_2 height from scaled crop_1
    MUINT32  u4W;   // input size of LSC, w2*r2, r2 must be 1
    MUINT32  u4H;   // input size of LSC, h2*r2, r2 must be 1
};

struct SensorCropCalculateVars_T
{
    MFLOAT rwc;
    MFLOAT rhc;
    MFLOAT x2c_;
    MFLOAT y2c_;

    MFLOAT rws;
    MFLOAT rhs;
    MFLOAT x2s_;
    MFLOAT y2s_;

    MFLOAT x2c__;
    MFLOAT y2c__;

    MFLOAT x_sc;
    MFLOAT y_sc;
    MFLOAT out_w;
    MFLOAT out_h;
    MFLOAT w2s;
    MFLOAT h2s;
};

#endif //_LSC_MGR_DEFAULT_STRUCT_H_

