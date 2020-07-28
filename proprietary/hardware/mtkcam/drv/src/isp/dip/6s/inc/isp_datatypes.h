#ifndef __ISP_DATATYPES_H__
#define __ISP_DATATYPES_H__

/**
    this header is for imageio internal-use
*/

#include "camera_dip.h" //For ISP Function ID,and Kernel Flag
#include <dip_reg.h>
#include "crz_drv.h"
#include <mtkcam/def/common.h>
using namespace NSCam;

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
    int             yuv_bit;   //0: 8 bits, 1: 10 bits
    int             yuv_dng;   //0: disable loose(pak,10b/10b), 1: enable loose(unp,10b/16b msb align)
    EImageFormat    lIspColorfmt;  //SL TEST_MDP_YUV
    MUINT32         capbility;      //port capbility
    MUINT32         secureTag;
    IspDMACfg():
        pixel_byte(1),swap(0), format_en(0),format(0),bus_size_en(0),bus_size(0),memBuf_c_ofst(0),memBuf_v_ofst(0),v_flip_en(0),yuv_bit(0),yuv_dng(0),lIspColorfmt(eImgFmt_IMPLEMENTATION_DEFINED),capbility(0x00),secureTag(0)
        {};
};

class MdpRotDMACfg
{
public:
    IspMemBuffer    memBuf;
    IspSize         size;
    IspMemBuffer    memBuf_c;
    IspSize         size_c;
    IspMemBuffer    memBuf_v;
    IspSize         size_v;
    int             crop_en;    //dma crop
    IspRect         crop;       //dma crop
    int             pixel_byte;
    int             uv_plane_swap;
    EImageFormat    Format; /* DISPO_FORMAT_1 */
    MUINT32         Plane;  /* DISPO_FORMAT_3 */ /* 0: 1 plane; 1: 2 plane; 2: 3 plane */
    MUINT32         Rotation; /* 0: 0; 1: 90; 2:180; 3:270 */
    /* crop information for the hw module before the dma(if support) */
    MBOOL withCropM;                    // there is crop module before the dma port or not
    MBOOL enSrcCrop;                    // enable src crop in the previous hw module
    MUINT32 srcCropX;                       //! X integer start position for cropping
    MUINT32 srcCropFloatX;                  //! X float start position for cropping
    MUINT32 srcCropY;                       //! Y integer start position for crpping
    MUINT32 srcCropFloatY;                  //! Y float start position for cropping
    MUINT32 srcCropW;                       //! width integer of cropped image
    MUINT32 srcCropFloatW;                  //! width float of cropped image
    MUINT32 srcCropH;                       //! height integer of cropped image
    MUINT32 srcCropFloatH;                  //! height float of cropped image
    MBOOL Flip;
    MUINT32         capbility;      //port capbility
    MUINT32 crop_group;	// 0 for MDP_CROP; 1 for MDP_CROP2
    MUINT32         secureTag;
    public:
        MdpRotDMACfg():
            crop_en(0),pixel_byte(1),uv_plane_swap(0),Format(eImgFmt_IMPLEMENTATION_DEFINED),Plane(0),Rotation(0),withCropM(0),enSrcCrop(0),srcCropX(0),srcCropFloatX(0),srcCropY(0),srcCropFloatY(0),srcCropW(0),srcCropFloatW(0),srcCropH(0),srcCropFloatH(0),Flip(0),capbility(0x00),crop_group(0),secureTag(0)
            {};
};


//[isp_reg review] need DIP owner to review this code
typedef struct ST_DIP_TOP_CTRL {
    DIPCTL_REG_D1A_DIPCTL_RGB_EN1 RGB_EN;
    DIPCTL_REG_D1A_DIPCTL_RGB_EN2 RGB2_EN;
    DIPCTL_REG_D1A_DIPCTL_YUV_EN1 YUV_EN;
    DIPCTL_REG_D1A_DIPCTL_YUV_EN2 YUV2_EN;
    DIPCTL_REG_D1A_DIPCTL_YUV_EN3 YUV3_EN;
    DIPCTL_REG_D1A_DIPCTL_DMA_EN1 DMA_EN;
    DIPCTL_REG_D1A_DIPCTL_DMA_EN2 DMA2_EN;
    DIPCTL_REG_D1A_DIPCTL_MUX_SEL1 MUX_SEL;
    DIPCTL_REG_D1A_DIPCTL_MUX_SEL2 MUX2_SEL;
    DIPCTL_REG_D1A_DIPCTL_FMT_SEL1 FMT_SEL;
    DIPCTL_REG_D1A_DIPCTL_FMT_SEL2 FMT2_SEL;
    DIPCTL_REG_D1A_DIPCTL_MISC_SEL MISC_SEL;
//    DIPCTL_REG_D1A_DIPCTL_INT1_EN INT_EN;
//    DIPCTL_REG_D1A_DIPCTL_INT2_EN INT2_EN;
//    DIPCTL_REG_D1A_DIPCTL_INT3_EN INT3_EN;
}ST_DIP_TOP_CTRL_t;



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


#endif


