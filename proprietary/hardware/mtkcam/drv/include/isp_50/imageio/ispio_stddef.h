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
#include <mtkcam/drv/def/ICam_type.h>

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
    EPIPECmd_GET_STT_CUR_BUF            = 0x0812,

    EPipeCmd_DTwin_INFO                 = 0x1001,

    EPIPECmd_SET_RRZ                    = 0x101A,
    EPIPECmd_SET_P1_UPDATE              = 0x101B,
    EPIPECmd_SET_IMGO                   = 0x101C,
    EPIPECmd_SET_STT_BA                 = 0x101D,
    EPIPECmd_SET_FRM_TIME               = 0x101E,
    EPIPECmd_SET_UFEO                   = 0x101F,

    EPIPECmd_AE_SMOOTH                  = 0x1105,
    EPIPECmd_HIGHSPEED_AE               = 0x1106,

    EPIPECmd_SET_TG_INT_LINE            = 0x110E,

    EPIPECmd_GET_HBIN_INFO              = 0x1111,
    EPIPECmd_GET_RCP_INFO               = 0x1112,
    EPIPECmd_GET_UNI_INFO               = 0x1113,
    EPIPECmd_GET_BIN_INFO               = 0x1114,
    EPIPECmd_GET_CQUPDATECNT            = 0x1115,
    EPIPECmd_GET_IMGO_INFO              = 0x111C,

    EPIPECmd_GET_CUR_FRM_STATUS         = 0x111D,
    EPIPECmd_GET_CUR_SOF_IDX            = 0x111E,

    EPIPECmd_GET_FLK_INFO               = 0x111F,

    EPIPECmd_GET_AFO_CONS               = 0x1120,
    EPIPECmd_GET_AAO_CONS               = 0x1121,
    EPIPECmd_GET_FLKO_CONS              = 0x1122,
    EPipeCmd_GET_MAGIC_REG_ADDR         = 0x1123,
    EPIPECmd_GET_PDO_CONS               = 0x1124,
    EPipeCmd_GET_TWIN_REG_ADDR          = 0x1125,
    EPIPECmd_GET_PSO_CONS               = 0x1126,
    EPIPECmd_GET_HEADER_SIZE            = 0x1127,//get header size by input dma
    EPIPECmd_GET_RMB_INFO               = 0x113D,

    EPIPECmd_ALLOC_UNI                  = 0x1200,
    EPIPECmd_DEALLOC_UNI                = 0x1201,

    EPIPECmd_SET_EIS_CBFP               = 0X1301,
    EPIPECmd_SET_LCS_CBFP               = 0X1302,
    EPIPECmd_SET_SGG2_CBFP              = 0X1303,
    EPIPECmd_SET_RSS_CBFP               = 0X1304,
    EPIPECmd_SET_RRZ_CBFP               = 0x1305,
    EPIPECmd_SET_TUNING_CBFP            = 0x1306,
    EPIPECmd_SET_REGDUMP_CBFP           = 0x1307,
    EPIPECmd_SET_IQ_CBFP                = 0x1308,
    EPIPECmd_SET_AWB_CBFP               = 0x1309,

    EPIPECmd_SET_PM_QOS_INFO            = 0x1408,
    EPIPECmd_SET_PM_QOS_RESET           = 0x1409,

    EPIPECmd_UNI_SWITCHOUT              = 0x1411,
    EPIPECmd_UNI_SWITCHOUTRIG           = 0x1412,
    EPIPECmd_GET_UNI_SWITCHOUTRIG_DONE  = 0x1413,
    EPIPECmd_UNI_SWITCHIN               = 0x1414,
    EPIPECmd_UNI_SWITCHINTRIG           = 0x1415,
    EPIPECmd_GET_UNI_ATTACH_MODULE      = 0x1416,

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
* interrupr event
********************************************************************************/
enum EPipeIRQ {
    EPIPEIRQ_VSYNC      = 0,
    EPIPEIRQ_SOF,
    EPIPEIRQ_PATH_DONE
};


/*******************************************************************************
* CQ
********************************************************************************/
enum EPipeCQ {
    EPIPE_CQ_NONE = (-1),
    EPIPE_PASS2_CQ1,
    EPIPE_PASS2_CQ1_SYNC,
    EPIPE_PASS2_CQ2,
    EPIPE_PASS2_CQ2_SYNC,
    EPIPE_PASS2_CQ3
};
//
enum EPipeCQTriger {
    EPIPECQ_TRIGGER_SINGLE_IMMEDIATE = 0,
    EPIPECQ_TRIGGER_SINGLE_EVENT,
    EPIPECQ_TRIGGER_CONTINUOUS_EVENT
};
//
enum EPipeCQStart {
    EPIPECQ_TRIG_BY_START = 0,
    EPIPECQ_TRIG_BY_PASS1_DONE,
    EPIPECQ_TRIG_BY_PASS2_DONE,
    EPIPECQ_TRIG_BY_IMGO_DONE,
    EPIPECQ_TRIG_BY_IMG2O_DONE,
};

/*******************************************************************************
* buffer source type
********************************************************************************/
enum EBufType
{
    eBufType_PMEM       = 0,  //= ISP_BUF_TYPE_PMEM,
    eBufType_STD_M4U,         //= ISP_BUF_TYPE_STD_M4U,
    eBufType_ION,             //= ISP_BUF_TYPE_ION,
};
/*******************************************************************************
* free buffer mode.
********************************************************************************/
enum EFreeBufMode {
    eFreeBufMode_SINGLE         = 0x0000,   //
    eFreeBufMode_ALL            = 0x0001,   //
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
* Muti-Plane Buffer Index
********************************************************************************/
typedef enum
{
    ePlane_1st = 0,
    ePlane_2nd,
    ePlane_3rd,
    ePlane_max,
}E_BufPlaneID;

/*******************************************************************************
* raw image Type setting
********************************************************************************/
enum ERawFmt
{
    eRawFmt_Processed          = 0x0000,   // processed Raw
    eRawFmt_Pure               = 0x0001,   // Pure Raw
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
    MUINT32      x;
    MUINT32      y;
    MUINT32      floatX; /* x float precise - 32 bit */
    MUINT32      floatY; /* y float precise - 32 bit */
    MUINT32      w;
    MUINT32      h;
    EImageFormat img_fmt;
    MUINT32      enqueue_img_stride;
public:
    STImgCrop()
    : x(0)
    , y(0)
    , floatX(0)
    , floatY(0)
    , w(0)
    , h(0)
    , img_fmt(eImgFmt_UNKNOWN)
    , enqueue_img_stride(0)
    {}
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
    EImgFmt_t       eImgFmt;        //  Image Pixel Format
    EImgRot_t       eImgRot;        //  Image Rotation degree in CW
    EImgFlip_t      eImgFlip;       //  Image Flip ON/OFF
    ERawPxlID_t     eRawPxlID;      //  raw data pixel ID
    E_CamPixelMode  ePxlMode;       //  seninf/tg sampling mode

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
    MUINT32     u4Stride[ePlane_max];    //  Image line byte size,0 for one or Y plae/1 for u or uv plane/2 for v plane
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
        E_CamPixelMode const _ePxlMode = ePixMode_1,
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

/**
buffer extension info
*/
struct BufInfo_Ext
{
public:
    MBOOL       bReplace;
    MUINT32     u4BufSize[ePlane_max];
    MUINTPTR    u4BufVA[ePlane_max];
    MUINTPTR    u4BufPA[ePlane_max];
    MINT32      memID[ePlane_max];
    MINT32      bufSecu[ePlane_max];
    MINT32      bufCohe[ePlane_max];
    BufInfo_Ext()
        : bReplace(0)
    {
        memset((void*)u4BufSize, 0, sizeof(u4BufSize));
        memset((void*)u4BufVA, 0, sizeof(u4BufVA));
        memset((void*)u4BufPA, 0, sizeof(u4BufPA));
        memset((void*)memID, 0, sizeof(memID));
        memset((void*)bufSecu, 0, sizeof(bufSecu));
        memset((void*)bufCohe, 0, sizeof(bufCohe));
    }
};

/*******************************************************************************
* Buffer Info.
********************************************************************************/
//need to mark this compile option once if the format of timestamp if sec + usec.  not usec only
    //undef -> format of timestamp is sec + usec
    //defed -> format of timestamp is only usec
#define TS_PERF
struct BufInfo  //buffer for each dma port (size/va/pa/memid should be enlarge to 3 for multi-plane support in one dma port)
{
public:     //// fields.
    MUINT32     u4BufSize[ePlane_max];  //  Per buffer size
    MUINTPTR    u4BufVA[ePlane_max];    //  Vir Address of pool
    MUINTPTR    u4BufPA[ePlane_max];    //  Phy Address of pool
    MUINTPTR    u4BufOffset[ePlane_max];
    MINT32      memID[ePlane_max];      //  memory ID
    MINT32      bufSecu[ePlane_max];
    MINT32      bufCohe[ePlane_max];
    BufInfo_Ext replace;
    BufInfo_Ext Frame_Header;
    //EBufType    eBufType;   // buffer type, by ION or other interface
    //
    MUINT64     i4TimeStamp_us;     //mono  time stamp ,unit:us;
    MUINT64     i4TimeStamp_us_B;   //boot  time stamp ,unit:us;
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
    MBOOL       m_highlightData;
    E_CamIQLevel  eIQlv;
    MBOOL        bUF_DataFmt;
    //
public:     //// constructors.
    BufInfo()
        : i4TimeStamp_us(0)
        , i4TimeStamp_us_B(0)
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
            memset((void*)u4BufSize, 0, sizeof(u4BufSize));
            memset((void*)u4BufVA, 0, sizeof(u4BufVA));
            memset((void*)u4BufPA, 0, sizeof(u4BufPA));
            memset((void*)memID, 0, sizeof(memID));
            memset((void*)bufSecu, 0, sizeof(bufSecu));
            memset((void*)bufCohe, 0, sizeof(bufCohe));
            crop_win = MRect(MPoint(0,0),MSize(0,0));
            DstSize  = MSize(0,0);
            m_pPrivate = NULL;
            raw_type = 0;
            jpg_size = 0;
            eIQlv = eCamIQ_MAX;
            bUF_DataFmt = MFALSE;
    }
    //

public: ////    operations.
    inline MINT64   getTimeStamp_ns() const
    {
        return  i4TimeStamp_us * 1000LL;
    }
    inline MINT64   getTimeStamp_B_ns() const
    {
        return  i4TimeStamp_us_B * 1000LL;
    }
};

/*******************************************************************************
* irq
********************************************************************************/
class Irq_t
{
public:
    typedef enum
    {
        _CLEAR_NONE,     //non-clear wait, clear after wait
        _CLEAR_WAIT,     //clear wait, clear before and after wait
    }E_TYPE;
    E_TYPE  Type;
    typedef enum
    {
        _SIGNAL_INT_    = 0,
        _DMA_INT_       = 1,
    }E_STType;
    E_STType    StatusType;
    typedef enum{
        _VSYNC_ = 0,
        _SOF_,
        _EOF_,
        _AF_DONE_,
        _TG_INT_,
    }E_STATUS;
    E_STATUS    Status;         // reference enum:ENUM_CAM_INT when using p1
    MINT32      UserKey;
    MUINT32     Timeout;

    struct{
        MUINT32    tLastSig_sec;                       /* time stamp of the latest occuring signal*/
        MUINT32    tLastSig_usec;                  /* time stamp of the latest occuring signal*/
        MUINT32    tMark2WaitSig_sec;            /* time period from marking a signal to user try to wait and get the signal*/
        MUINT32    tMark2WaitSig_usec;            /* time period from marking a signal to user try to wait and get the signal*/
        MUINT32    tLastSig2GetSig_sec;         /* time period from latest occuring signal to user try to wait and get the signal*/
        MUINT32    tLastSig2GetSig_usec;         /* time period from latest occuring signal to user try to wait and get the signal*/
        MUINT32    passedbySigcnt;          /* the count for the signal passed by  */
    }TimeInfo;
public:     //// constructors.
    Irq_t(
        MINT32  const   _UserKey = -1,
        MUINT32 const   _Timeout = 0,
        MUINT32 const   _irq_TStamp = 0
    )
        : UserKey(_UserKey)
        , Timeout(_Timeout)
    {
        Status = Irq_t::_VSYNC_;
        Type = Irq_t::_CLEAR_NONE;
        TimeInfo.tLastSig_sec = _irq_TStamp;
        TimeInfo.tLastSig_usec = _irq_TStamp;
        TimeInfo.tMark2WaitSig_sec = _irq_TStamp;
        TimeInfo.tMark2WaitSig_usec = _irq_TStamp;
        TimeInfo.tLastSig2GetSig_sec = _irq_TStamp;
        TimeInfo.tLastSig2GetSig_usec = _irq_TStamp;
        TimeInfo.passedbySigcnt = _irq_TStamp;
        StatusType = _SIGNAL_INT_;
    }

};

typedef struct
{
    MUINT32     Addr;
    MUINT32     Data;
}ISPIO_REG_CFG;

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio
#endif // _ISPIO_STDDEF_H_

