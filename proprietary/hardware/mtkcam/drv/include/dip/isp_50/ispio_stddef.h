/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _ISPIO_STDDEF_H_
#define _ISPIO_STDDEF_H_

#include <mtkcam/def/common.h>
#include <utils/Mutex.h>    // For android::Mutex.
using namespace NSCam;
#include <mtkcam/drv/def/ispio_sw_scenario.h>
//#include "utils/Mutex.h"    // For android::Mutex.


/*******************************************************************************
*
*******************************************************************************/
namespace NSImageio {
namespace NSIspio   {
////////////////////////////////////////////////////////////////////////////////
/*******************************************************************************
* pipe support command list
********************************************************************************/
enum EPIPECmd {
    EPIPECmd_NONE             = 0x0,
    EPIPECmd_SET_VENC_DRLINK            = 0x110C,
    EPIPECmd_RELEASE_VENC_DRLINK        = 0x110D,
    EPIPECmd_SET_JPEG_CFG               = 0x1303,
    EPIPECmd_SET_JPEG_WORKBUF_SIZE      = 0x1304,
    EPIPECmd_MAX                        = 0xFFFF
};

enum EPIPE_P2BUFQUECmd {
    EPIPE_P2BUFQUECmd_ENQUE_FRAME         = 0x0000,   //enque frame
    EPIPE_P2BUFQUECmd_WAIT_DEQUE          = 0x0001,   //wait deque for deque thread
    EPIPE_P2BUFQUECmd_DEQUE_SUCCESS       = 0x0002,   //deque done
    EPIPE_P2BUFQUECmd_DEQUE_FAIL          = 0x0003,   //deque done
    EPIPE_P2BUFQUECmd_WAIT_FRAME          = 0x0004,   //wait frame for user
    EPIPE_P2BUFQUECmd_WAKE_WAITFRAME      = 0x0005,   //wake up user that wait for a frame to check
    EPIPE_P2BUFQUECmd_CLAER_ALL           = 0x0006    //clear all stored buffer in kernel list
};

enum EPIPE_P2engine {
    EPIPE_P2engine_DIP         = 0x0000,   //
    EPIPE_P2engine_Warp          = 0x0001,   //
};

/*******************************************************************************
* Image Rotation.
********************************************************************************/
enum EImageRotation
{
    eImgRot_0      = 0, //
    eImgRot_90,         //90 CW
    eImgRot_180,
    eImgRot_270
};
/*******************************************************************************
* Image Rotation.
********************************************************************************/
enum EImageFlip
{
    eImgFlip_OFF     = 0, //
    eImgFlip_ON      = 1, //
};

/*******************************************************************************
* raw image pixel ID
********************************************************************************/
enum ERawPxlID
{
    ERawPxlID_B   = 0,  // B Gb Gr R
    ERawPxlID_Gb,       // Gb B R Gr
    ERawPxlID_Gr,       // Gr R B Gb
    ERawPxlID_R         // R Gr Gb B
};

/*******************************************************************************
* raw data memory footprint bit per pixel
********************************************************************************/
enum ERAW_MEM_FP_BPP
{
    ERAW8_MEM_FP_BPP   = 8,
    ERAW10_MEM_FP_BPP  = 10,
    ERAW12_MEM_FP_BPP  = 12,
};

/*******************************************************************************
* STRIDE INFO INDEX
********************************************************************************/
enum EIMAGE_STRIDE
{
    ESTRIDE_1ST_PLANE   = 0,
    ESTRIDE_2ND_PLANE   = 1,
    ESTRIDE_3RD_PLANE   = 2
};




/*******************************************************************************
* raw image Type setting
********************************************************************************/
enum ERawFmt
{
    eRawFmt_Processed          = 0x0000,   // processed Raw
    eRawFmt_Pure               = 0x0001,   // Pure Raw
};

/*******************************************************************************
* TG pixel sampling mode setting
********************************************************************************/
enum EPxlMode
{
    ePxlMode_One_   = 0x0000,   // 1 pixel mode
    ePxlMode_Two_   = 0x0001,   // 2 pixel mode
    ePxlMode_Four_  = 0x0002,   // 4 pixel mode
    ePxlMode_RSVD   = 0x000FF
};


/*******************************************************************************
* image resizer
********************************************************************************/
struct STImgResize{
    public:
        MUINT32 src_x;
        MUINT32 src_y;
        MUINT32 src_w;
        MUINT32 src_h;
        MUINT32 tar_x;
        MUINT32 tar_y;
        MUINT32 tar_w;
        MUINT32 tar_h;
    public:
        STImgResize()
            :src_x(0)
            ,src_y(0)
            ,src_w(0)
            ,src_h(0)
            ,tar_x(0)
            ,tar_y(0)
            ,tar_w(0)
            ,tar_h(0)
        {
        }
};

/*******************************************************************************
* image crop
********************************************************************************/
struct STImgCrop{
public:
    MUINT32 x;
    MUINT32 y;
    MUINT32 floatX; /* x float precise - 32 bit */
    MUINT32 floatY; /* y float precise - 32 bit */
    MUINT32 w;
    MUINT32 h;
    MUINT32 floatW; /* W float precise - 32 bit */
    MUINT32 floatH; /* H float precise - 32 bit */
public:
    STImgCrop()
        :x(0)
        ,y(0)
        ,floatX(0)
        ,floatY(0)
        ,w(0)
        ,h(0)
        ,floatW(0)
        ,floatH(0)
    {
    }
};

/*******************************************************************************
* Image Info.
********************************************************************************/
struct ImgInfo
{
public:     //// fields.
    typedef EImageFormat EImgFmt_t;
    typedef EImageRotation EImgRot_t;
    typedef EImageFlip EImgFlip_t;
    typedef ERawPxlID ERawPxlID_t;
    typedef EPxlMode EPxlMode_t;
    EImgFmt_t   eImgFmt;        //  Image Pixel Format
    EImgRot_t   eImgRot;        //  Image Rotation degree in CW
    EImgFlip_t  eImgFlip;       //  Image Flip ON/OFF
    ERawPxlID_t eRawPxlID;      //  raw data pixel ID
    EPxlMode_t  ePxlMode;       //  seninf/tg sampling mode

    //
    MFLOAT      rrzRatio;       //  for sl2
    MUINT32     rrzCropX;       //  for sl2
    MUINT32     rrzCropY;       //  for sl2
    MUINT32     u4MagicNum;     //  magic number for tuning queue
    MUINT32     u4PureRaw;
    MUINT32     u4PureRawPak;
    MUINT32     u4ImgWidth;     //  Image Width
    MUINT32     u4ImgHeight;    //  Image Height
    MUINT32     u4Offset;       //  Image offset byte size
    MUINT32     u4Stride[3];    //  Image line byte size,0 for one or Y plae/1 for u or uv plane/2 for v plane
    STImgCrop   crop1;           // image crop info. (ring buffer use curz to run crop)
    STImgResize resize1;
    STImgCrop   crop2;           // image crop info. (ring buffer use curz to run crop)
    STImgResize resize2;
    STImgCrop   crop3;           // image crop info. (ring buffer use curz to run crop)
    STImgResize resize3;
    //
public:     //// constructors.
    ImgInfo(
        EImgFmt_t const _eImgFmt = eImgFmt_UNKNOWN,
        EImgRot_t const _eImgFot = eImgRot_0,
        EImgFlip_t const _eImgFlip = eImgFlip_OFF,
        EPxlMode_t const _ePxlMode = ePxlMode_One_,
        MUINT32 const _u4PureRaw = 0,
        MUINT32 const _u4PureRawPak = 0,
        MUINT32 const _u4ImgWidth = 0,
        MUINT32 const _u4ImgHeight = 0
    )
        : eImgFmt(_eImgFmt)
        , eImgRot(_eImgFot)
        , eImgFlip(_eImgFlip)
        , eRawPxlID(ERawPxlID_B)
        , ePxlMode(_ePxlMode)
        , rrzRatio(0)
        , rrzCropX(0)
        , rrzCropY(0)
        , u4MagicNum(0)
        , u4PureRaw(_u4PureRaw)
        , u4PureRawPak(_u4PureRawPak)
        , u4ImgWidth(_u4ImgWidth)
        , u4ImgHeight(_u4ImgHeight)
        , u4Offset(0)
        , crop1()
    {
    }
};

#if 1

/**
buffer extension info
*/
struct BufInfo_Ext
{
public:
    MBOOL       bReplace;
    MUINT32     u4BufSize;
    MUINTPTR    u4BufVA;
    MUINTPTR    u4BufPA;
    MINT32      memID;
    MINT32      bufSecu;
    MINT32      bufCohe;
    BufInfo_Ext()
        : bReplace(0)
        , u4BufSize(0)
        , u4BufVA(0)
        , u4BufPA(0)
        , memID(0)
        , bufSecu(0)
        , bufCohe(0)
    {}
};
/*******************************************************************************
* Buffer Info.
********************************************************************************/
struct BufInfo  //buffer for each dma port (size/va/pa/memid should be enlarge to 3 for multi-plane support in one dma port)
{
public:     //// fields.
    MUINT32     u4BufSize[3];  //  Per buffer size
    MUINTPTR    u4BufVA[3];    //  Vir Address of pool
    MUINTPTR    u4BufPA[3];    //  Phy Address of pool
    MUINTPTR    u4BufOffset[3];
    MINT32      memID[3];      //  memory ID
    MINT32      bufSecu[3];
    MINT32      bufCohe[3];
    BufInfo_Ext replace;
    BufInfo_Ext Frame_Header;
    //EBufType    eBufType;   // buffer type, by ION or other interface
    //
    MUINT32     i4TimeStamp_sec;//  time stamp in seconds.
    MUINT32     i4TimeStamp_us; //  time stamp in microseconds
    //
    MUINT32     img_w;
    MUINT32     img_h;
    MUINT32     img_stride;
    MUINT32     img_fmt;
    MUINT32     img_pxl_id;
    MUINT32     m_num;
    MUINT32     frm_cnt;
    MUINT32     raw_type;   //0:proc, 1:pure, 2 before lsc
    MUINT32     jpg_size;

    MUINT32     xoffset;    //starting x-offset of dma
    MUINT32     yoffset;    //starting y-offset of dma
    MRect       crop_win;   //crop windon, IN TG coordinate axis
    MSize       DstSize;    // image w/h on dram

    MUINT32*    m_pPrivate; //reserved pointer
    MBOOL         m_highlightData;
    //
public:     //// constructors.
    BufInfo()
        : i4TimeStamp_sec(0)
        , i4TimeStamp_us(0)
        , img_w(0)
        , img_h(0)
        , img_stride(0)
        , img_fmt(0)
        , img_pxl_id(0)
        , m_num(0)
        , frm_cnt(0)
        , raw_type(0)
        , jpg_size(0)
        , xoffset(0)
        , yoffset(0)
        , m_highlightData(0)
    {
            for(int i=0;i<3;i++){
                u4BufSize[i] = 0;
                u4BufVA[i] = 0;
                u4BufPA[i] = 0;
                memID[i] = 0;
                bufSecu[i] = 0;
                bufCohe[i] = 0;
            }
            crop_win = MRect(MPoint(0,0),MSize(0,0));
            DstSize  = MSize(0,0);
            m_pPrivate = NULL;
            raw_type = 0;
            jpg_size = 0;
    }
    //

public: ////    operations.
    inline MINT64   getTimeStamp_ns() const
    {
        return  i4TimeStamp_sec * 1000000000LL + i4TimeStamp_us * 1000LL;
    }
    //
/*
    inline MBOOL    setTimeStamp()
    {
        struct timeval tv;
        if  ( 0 == ::gettimeofday(&tv, NULL) )
        {
            i4TimeStamp_sec = tv.tv_sec;
            i4TimeStamp_us  = tv.tv_usec;
            return  MTRUE;
        }
        return  MFALSE;
    }
*/
    //
};

#endif

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio
#endif // _ISPIO_STDDEF_H_

