#ifndef __ISP_DATATYPES_H__
#define __ISP_DATATYPES_H__

/**
    this header is for imageio internal-use
*/

#include "camera_isp.h" //For ISP Function ID,and Kernel Flag
#include <isp_reg.h>
//#include "crz_drv.h"
//-----------------------------------------------------------------------------
#include <mtkcam/def/BuiltinTypes.h>    // For type definitions.
#include <isp_drv.h>        // for IspDrv class.
//-----------------------------------------------------------------------------
using namespace android;

#include <mtkcam/def/common.h>
using namespace NSCam;


/*-----------------------------------------------------------------------------
    ISP ERROR CODE
  -----------------------------------------------------------------------------*/

#define ISP_ERROR_CODE_OK                      (0)
#define ISP_ERROR_CODE_FAIL                    (-1)

#define ISP_ERROR_CODE_FAIL_00                 (-1 & ~(0x1<<0))  /*-2 , (0xFFFFFFFE)*/
#define ISP_ERROR_CODE_FAIL_01                 (-1 & ~(0x1<<1))  /*-3 , (0xFFFFFFFD)*/
#define ISP_ERROR_CODE_FAIL_02                 (-1 & ~(0x1<<2))  /*-5 , (0xFFFFFFFB)*/
#define ISP_ERROR_CODE_FAIL_03                 (-1 & ~(0x1<<3))  /*-9 , (0xFFFFFFF7)*/
#define ISP_ERROR_CODE_FAIL_04                 (-1 & ~(0x1<<4))  /*-17 , (0xFFFFFFEF)*/
#define ISP_ERROR_CODE_LOCK_RESOURCE_FAIL      (-1 & ~(0x1<<5))  /*-33 , (0xFFFFFFDF)*/
#define ISP_ERROR_CODE_FAIL_06                 (-1 & ~(0x1<<6))  /*-65 , (0xFFFFFFBF)*/
#define ISP_ERROR_CODE_FAIL_07                 (-1 & ~(0x1<<7))  /*-129 , (0xFFFFFF7F)*/
#define ISP_ERROR_CODE_FAIL_08                 (-1 & ~(0x1<<8))  /*-257 , (0xFFFFFEFF)*/
#define ISP_ERROR_CODE_FAIL_09                 (-1 & ~(0x1<<9))  /*-513 , (0xFFFFFDFF)*/
#define ISP_ERROR_CODE_FAIL_10                 (-1 & ~(0x1<<10))  /*-1025 , (0xFFFFFBFF)*/
#define ISP_ERROR_CODE_FAIL_11                 (-1 & ~(0x1<<11))  /*-2049 , (0xFFFFF7FF)*/
#define ISP_ERROR_CODE_FAIL_12                 (-1 & ~(0x1<<12))  /*-4097 , (0xFFFFEFFF)*/
#define ISP_ERROR_CODE_FAIL_13                 (-1 & ~(0x1<<13))  /*-8193 , (0xFFFFDFFF)*/
#define ISP_ERROR_CODE_FAIL_14                 (-1 & ~(0x1<<14))  /*-16385 , (0xFFFFBFFF)*/
#define ISP_ERROR_CODE_FAIL_15                 (-1 & ~(0x1<<15))  /*-32769 , (0xFFFF7FFF)*/
#define ISP_ERROR_CODE_FAIL_16                 (-1 & ~(0x1<<16))  /*-65537 , (0xFFFEFFFF)*/
#define ISP_ERROR_CODE_FAIL_17                 (-1 & ~(0x1<<17))  /*-131073 , (0xFFFDFFFF)*/
#define ISP_ERROR_CODE_FAIL_18                 (-1 & ~(0x1<<18))  /*-262145 , (0xFFFBFFFF)*/
#define ISP_ERROR_CODE_FAIL_19                 (-1 & ~(0x1<<19))  /*-524289 , (0xFFF7FFFF)*/
#define ISP_ERROR_CODE_FAIL_20                 (-1 & ~(0x1<<20))  /*-1048577 , (0xFFEFFFFF)*/
#define ISP_ERROR_CODE_FAIL_21                 (-1 & ~(0x1<<21))  /*-2097153 , (0xFFDFFFFF)*/
#define ISP_ERROR_CODE_FAIL_22                 (-1 & ~(0x1<<22))  /*-4194305 , (0xFFBFFFFF)*/
#define ISP_ERROR_CODE_FAIL_23                 (-1 & ~(0x1<<23))  /*-8388609 , (0xFF7FFFFF)*/
#define ISP_ERROR_CODE_FAIL_24                 (-1 & ~(0x1<<24))  /*-16777217 , (0xFEFFFFFF)*/
#define ISP_ERROR_CODE_FAIL_25                 (-1 & ~(0x1<<25))  /*-33554433 , (0xFDFFFFFF)*/
#define ISP_ERROR_CODE_FAIL_26                 (-1 & ~(0x1<<26))  /*-67108865 , (0xFBFFFFFF)*/
#define ISP_ERROR_CODE_FAIL_27                 (-1 & ~(0x1<<27))  /*-134217729 , (0xF7FFFFFF)*/
#define ISP_ERROR_CODE_FAIL_28                 (-1 & ~(0x1<<28))  /*-268435457 , (0xEFFFFFFF)*/
#define ISP_ERROR_CODE_FAIL_29                 (-1 & ~(0x1<<29))  /*-536870913 , (0xDFFFFFFF)*/
#define ISP_ERROR_CODE_FAIL_30                 (-1 & ~(0x1<<30))  /*-1073741825 , (0xBFFFFFFF)*/


#define ISP_IS_ERROR_CODE( _retval_ , _errorcode_ ) \
            (   _retval_ >= 0    ?    0    :    ( (( _retval_|_errorcode_)==_errorcode_) ? 1 : 0 )     )



/*-----------------------------------------------------------------------------
    DATA STRUCTURE
  -----------------------------------------------------------------------------*/


class IspSize
{
public:
    unsigned long w;
    unsigned long h;
    unsigned long stride;
    unsigned long xsize;    //unit:byte
public:
    IspSize():
        w(0),h(0), stride(0), xsize(0)
        {};


    IspSize(unsigned long _w, unsigned long _h )
        {
            w = _w; h = _h;
        };
};

class IspPoint
{
public:
    long    x;
    long    y;

public:
    IspPoint():
        x(0),y(0)
        {};

   IspPoint(unsigned long _x, unsigned long _y )
       {
           x = _x; y = _y;
       };
};

class IspRect
{
public:
    MUINT32         x;
    MUINT32         y;
    MUINT32         floatX; /* x float precise - 32 bit */
    MUINT32         floatY; /* y float precise - 32 bit */
    unsigned long   w;
    unsigned long   h;


public:
    IspRect():
        x(0),y(0),floatX(0),floatY(0),w(0),h(0)
        {};

   IspRect(long _x, long _y, long _floatX, long _floatY, unsigned long _w, unsigned long _h )
        {
            x = _x; y = _y; floatX = _floatX; floatY = _floatY; w = _w; h = _h;
        };

};

class IspYuvAddr
{
public:
    unsigned long   y;
    unsigned long   u;
    unsigned long   v;

    /*User need not fill in the data below!-----------------------------------------------------------*/
    /*Below data is auto fill by ISP driver calculation!----------------------------------------------*/

    unsigned long   y_buffer_size;
    unsigned long   u_buffer_size;
    unsigned long   v_buffer_size;

public:
    IspYuvAddr():
        y(0), u(0), v(0),
        y_buffer_size(0), u_buffer_size(0), v_buffer_size(0)
        {};
};


class IspMemBuffer
{
public:
    MUINT32 size;
    MUINTPTR base_vAddr;
    MUINTPTR base_pAddr;
    MUINT32 ofst_addr;
    MUINT32 alignment;
public:
    IspMemBuffer():
        size(0),base_vAddr(0),base_pAddr(0),ofst_addr(0), alignment(16)
        {};
};

class IspOffset
{
public:
    MUINT32 x;
    MUINT32 y;
public:
    IspOffset():
        x(0),y(0)
        {};
};

class IspDMACfg
{
public:
    IspMemBuffer    memBuf;
    IspSize         size;
    IspOffset       offset;
    IspRect         crop;
    int             pixel_byte;
    int             swap;
    int             format_en;
    int             format;
    int             bus_size_en;
    int             bus_size;
    int             memBuf_c_ofst;
    int             memBuf_v_ofst;
    int             v_flip_en;
    MUINT32         capbility;      //port capbility
    EImageFormat    lIspColorfmt;  //SL TEST_MDP_YUV
    IspDMACfg():
        pixel_byte(1),swap(0), format_en(0),format(0),bus_size_en(0),bus_size(0),
        memBuf_c_ofst(0),memBuf_v_ofst(0),v_flip_en(0),capbility(0x00),
        lIspColorfmt(eImgFmt_IMPLEMENTATION_DEFINED)
        {};
};

class IspCapTdriCfg
{
public:
    int isCalculateTpipe;
    MUINT32 setSimpleConfIdxNumVa;
    MUINT32 segSimpleConfBufVa;
};


//
typedef struct _ST_CAM_TOP_CTRL {
    CAM_REG_CTL_EN            FUNC_EN;
    CAM_REG_CTL_FMT_SEL       FMT_SEL;
    CAM_REG_CTL_SEL           CTRL_SEL;
}ST_CAM_TOP_CTRL;

typedef struct _ST_UNI_TOP_CTRL{
    CAM_UNI_REG_TOP_MOD_EN      FUNC_EN;
    CAM_UNI_REG_TOP_FMT_SEL     FMT_SEL;
}ST_UNI_TOP_CTRL;

/* _CAM_SV */
struct ST_CAMSV_TOP_CTRL {
    CAMSV_REG_MODULE_EN            FUNC_EN;
    CAMSV_REG_FMT_SEL              FMT_SEL;
};

/* [Everest isp_reg review] need DIP owner to review this code
struct ST_DIP_TOP_CTRL {
    DIP_A_REG_CTL_YUV_EN    YUV_EN;
    DIP_A_REG_CTL_YUV2_EN   YUV2_EN;
    DIP_A_REG_CTL_RGB_EN    RGB_EN;
    DIP_A_REG_CTL_FMT_SEL   FMT_SEL;
    DIP_A_REG_CTL_PATH_SEL  PATH_SEL;
    DIP_A_REG_CTL_INT_EN    INT_EN;
};
*/

typedef struct stIspOBCCtl {
    MUINT32              CAM_OBC_OFFST0;                 //4500
    MUINT32              CAM_OBC_OFFST1;                 //4504
    MUINT32              CAM_OBC_OFFST2;                 //4508
    MUINT32              CAM_OBC_OFFST3;                 //450C
    MUINT32               CAM_OBC_GAIN0;                  //4510
    MUINT32               CAM_OBC_GAIN1;                  //4514
    MUINT32               CAM_OBC_GAIN2;                  //4518
    MUINT32               CAM_OBC_GAIN3;                  //451C
}stIspOBCCtl_t;

typedef struct stIspBPCCtl {
    MUINT32 dmmy;
}stIspBPCCtl_t;

typedef struct stIspRMMCtl {
    MUINT32 dmmy;
}stIspRMMCtl_t;

typedef struct stIspNR1Ctl {
    MUINT32 dmmy;

}stIspNR1Ctl_t;

typedef struct stIspLSCCtl {
    MUINT32 dmmy;

}stIspLSCCtl_t;

typedef struct stIspRPGCtl {
    MUINT32 dmmy;

}stIspRPGCtl_t;

typedef struct stIspAECtl {
    MUINT32 dmmy;

}stIspAECtl_t;

typedef struct stIspAWBCtl {
    MUINT32 dmmy;

}stIspAWBCtl_t;

typedef struct stIspSGG1Ctl {
    MUINT32 dmmy;

}stIspSGG1Ctl_t;

typedef struct stIspFLKCtl {
    MUINT32 dmmy;

}stIspFLKCtl_t;

typedef struct stIspAFCtl {
    MUINT32 dmmy;

}stIspAFCtl_t;

typedef struct stIspSGG2Ctl {
    MUINT32 dmmy;

}stIspSGG2Ctl_t;

typedef struct stIspEISCtl {
    MUINT32 dmmy;

}stIspEISCtl_t;

typedef struct stIspLCSCtl {
    MUINT32 dmmy;

}stIspLCSCtl_t;

class IspBnrCfg
{
public:
    int bpc_en;
    int bpc_tbl_en;
};

class IspLscCfg
{
public:
    int sdblk_width;
    int sdblk_xnum;
    int sdblk_last_width;
    int sdblk_height;
    int sdblk_ynum;
    int sdblk_last_height;
};
class IspLceCfg
{
public:
    int lce_bc_mag_kubnx;
    int lce_offset_x;
    int lce_bias_x;
    int lce_slm_width;
    int lce_bc_mag_kubny;
    int lce_offset_y;
    int lce_bias_y;
    int lce_slm_height;
};
class IspNbcCfg
{
public:
    int anr_eny;
    int anr_enc;
    int anr_iir_mode;
    int anr_scale_mode;
};
class IspSeeeCfg
{
public:
    int se_edge;
    int usm_over_shrink_en;
};
class IspImgoCfg
{
public:
    int imgo_stride;
    int imgo_crop_en;
};
class IspEsfkoCfg
{
public:
    int esfko_stride;
};
class IspAaoCfg
{
public:
    int aao_stride;
};
class IspLcsoCfg
{
public:
    int lcso_stride;
    int lcso_crop_en;
};

class IspRrzCfg
{
public:
    //unsigned long lsc_en;
    //unsigned long sd_lwidth;
    //unsigned long sd_xnum;
    //unsigned long twin_lsc_crop_offx;
    //unsigned long twin_lsc_crop_offy;
    IspRect      rrz_in_roi;
    IspSize      rrz_out_size;
	EImageFormat img_fmt;
public:
    IspRrzCfg()
    : img_fmt(eImgFmt_UNKNOWN)
    {}
};

class IspP1TuningCfg{
public:
    MUINT32 magic;
    MUINT32 SenDev;
public:
    IspP1TuningCfg()
    {
        magic = 0xffffffff;
        SenDev = 0xffffffff;
    }
};


class IspMdpCropCfg
{
public:
    MBOOL crz_enable;
    IspSize crz_in;
    IspRect crz_crop;
    IspSize crz_out;
    MBOOL mdp_enable;
    EImageFormat mdp_src_fmt;
    IspSize mdp_Yin;
    IspSize mdp_UVin;
    MUINT32 mdp_src_size;
    MUINT32 mdp_src_Csize;
    MUINT32 mdp_src_Vsize;
    MUINT32 mdp_src_Plane_Num;
    IspRect mdp_crop;
    IspSize mdp_out;
};


#if 1

class IspCrzCfg
{
public:
    int crz_input_crop_width;
    int crz_input_crop_height;
    int crz_output_width;
    int crz_output_height;
    int crz_horizontal_integer_offset;/* pixel base */
    int crz_horizontal_subpixel_offset;/* 20 bits base */
    int crz_vertical_integer_offset;/* pixel base */
    int crz_vertical_subpixel_offset;/* 20 bits base */
    int crz_horizontal_luma_algorithm;
    int crz_vertical_luma_algorithm;
    int crz_horizontal_coeff_step;
    int crz_vertical_coeff_step;
};
class IspCurzCfg
{
public:
    int curz_input_crop_width;
    int curz_input_crop_height;
    int curz_output_width;
    int curz_output_height;
    int curz_horizontal_integer_offset;/* pixel base */
    int curz_horizontal_subpixel_offset;/* 20 bits base */
    int curz_vertical_integer_offset;/* pixel base */
    int curz_vertical_subpixel_offset;/* 20 bits base */
    int curz_horizontal_coeff_step;
    int curz_vertical_coeff_step;
};

class IspFeCfg
{
public:
    int fem_harris_tpipe_mode;
};
class IspImg2oCfg
{
public:
    int img2o_stride;
    int img2o_crop_en;
    int img2o_xsize; /* byte size - 1 */
    int img2o_ysize; /* byte size - 1 */
};
class IspPrzCfg
{
public:
    int prz_input_crop_width;
    int prz_input_crop_height;
    int prz_output_width;
    int prz_output_height;
    int prz_horizontal_integer_offset;/* pixel base */
    int prz_horizontal_subpixel_offset;/* 20 bits base */
    int prz_vertical_integer_offset;/* pixel base */
    int prz_vertical_subpixel_offset;/* 20 bits base */
    int prz_horizontal_luma_algorithm;
    int prz_vertical_luma_algorithm;
    int prz_horizontal_coeff_step;
    int prz_vertical_coeff_step;
};

class IspMfbCfg
{
public:
    int bld_mode;
    int bld_deblock_en;
    int bld_brz_en;
};


class IspFlkiCfg
{
public:
    int flki_stride;
};
class IspCfaCfg
{
public:
    int bayer_bypass;
};

/**
*@brief  ISP sl2 configure struct
*/
class IspSl2Cfg
{
    public:
          int sl2_hrz_comp;
};

#endif

class IspTopCfg
{
public:
    unsigned int enable1;
    unsigned int enable2;
    unsigned int dma;
};

#if 0
class IspTdriCfg
{
public:
    //enable table
    IspTopCfg   en_Top;

    /*===DMA===*/
    IspDMACfg imgi;
    IspDMACfg vipi;
    IspDMACfg vip2i;
    IspDMACfg imgci;
    IspDMACfg lcei;
    IspDMACfg lsci;
    MdpRotDMACfg wroto;
    MdpRotDMACfg wdmao;
    MdpRotDMACfg imgxo;

    IspCapTdriCfg capTdriCfg;
    IspBnrCfg bnr;
    IspLscCfg lsc;
    IspLceCfg lce;
    IspNbcCfg nbc;
    IspSeeeCfg seee;
    IspImgoCfg imgo;
    IspEsfkoCfg esfko;
    IspAaoCfg aao;
    IspLcsoCfg lcso;

//    IspCdrzCfg cdrz;
//    IspCurzCfg curz;
//    IspFeCfg fe;
//    IspImg2oCfg img2o;
//    IspPrzCfg prz;
//    IspMfbCfg mfb;
//    IspFlkiCfg flki;
//    IspCfaCfg cfa;
};
#endif

/*******************************************************************************
* JPG Info.
********************************************************************************/
class JPGCfg
{
public:
    int             soi_en;
    MUINT32         fQuality;
public:
    JPGCfg():
        soi_en(0),fQuality(0)
        {};
};

/*******************************************************************************
* RRZ Info.
********************************************************************************/

class DMACfg
{
public:
    IspRect      crop;
    IspSize      out;
    MUINT32      rawType;    //0:proc, 1:pure, 2 before lsc
    EImageFormat img_fmt;
    EImageFormat img_fmt_configpipe;
    MUINT32      tgCropW;    //tgCropWidth
    MUINT32      enqueue_img_stride;
    MINT32       pixel_byte;
public:
    DMACfg()
    : rawType(0)
    , img_fmt(eImgFmt_UNKNOWN)
    , img_fmt_configpipe(eImgFmt_UNKNOWN)
    , tgCropW(0)
    , enqueue_img_stride(0)
    , pixel_byte(0)
    {};
};

typedef DMACfg RRZCfg;

typedef int (*pfCBFuncCfg)(void);

/*-----------------------------------------------------------------------------
    UTILITY MACRO
  -----------------------------------------------------------------------------*/
#define ISP_IS_ALIGN( _number_, _power_of_2_ )      ( ( _number_ & ( (0x1<<_power_of_2_)-1 ) ) ? 0 : 1 )
#define ISP_ROUND_UP( _number_, _power_of_2_ )      _number_ = ( ( (_number_ + (( 0x1 << _power_of_2_ )-1)) >> _power_of_2_ ) << _power_of_2_ )

#endif


