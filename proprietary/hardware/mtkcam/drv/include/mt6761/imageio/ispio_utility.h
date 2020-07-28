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
#ifndef _ISPIO_PIPE_UTILITY_ISP3_H_
#define _ISPIO_PIPE_UTILITY_ISP3_H_

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1

#include <imageio/ispio_pipe_ports.h>

/*******************************************************************************
*
********************************************************************************/
namespace NSImageioIsp3 {
namespace NSIspio {

/*******************************************************************************
*
********************************************************************************/

#include <cutils/log.h>

#include <mtkcam/def/ImageFormat.h>

#define ISPUTIL_MSG(fmt, arg...)    //ALOGD("[%s]"          fmt, __FUNCTION__,           ##arg)
#define ISPUTIL_WRN(fmt, arg...)    ALOGW("[%s]WRN(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)
#define ISPUTIL_ERR(fmt, arg...)    ALOGE("[%s]ERR(%5d):" fmt, __FUNCTION__, __LINE__, ##arg)

#define RRZ_LIMITATION  2304
#define _BY_PASSS_PORT  0xFFFF

typedef enum _QUERYOP {
    ISP_QUERY_X_PIX                 = 0x0001,
    ISP_QUERY_STRIDE_PIX            = 0x0002,
    ISP_QUERY_STRIDE_BYTE           = 0x0004,
    ISP_QUERY_XSIZE_BYTE            = 0x0008,
    ISP_QUERY_CROP_X_PIX            = 0x0010,   //query when using cropping, (cropping size != input size)
    ISP_QUERY_CROP_X_BYTE           = 0x0020,   //query when using cropping, (cropping size != input size)
    ISP_QUERY_CROP_START_X          = 0x0040,   //query when using cropping, the unit of crop start x is pixel.
} E_ISP_QUERY;

typedef enum _QUERY_PIXMODE {
    ISP_QUERY_UNKNOWN_PIX_MODE  = 0x0,      // default use 2 pixmode constraint if unknown pix mode being asignned
    ISP_QUERY_1_PIX_MODE        = 0x1,      // 1 pix mode
    ISP_QUERY_2_PIX_MODE        = 0x2,      // 2 pix mode
    ISP_QUERY_MAX_PIX_MODE      = 0x3
} E_ISP_PIXMODE;

typedef struct _ISP_QUERY_RST {
    MUINT32 x_pix;                  //horizontal resolution, unit:pix
    MUINT32 stride_pix;             //stride, uint:pix. this is a approximative value under pak mode
    MUINT32 stride_byte;            //stride, uint:byte
    MUINT32 xsize_byte;
    MUINT32 crop_x;                 //crop start point-x , unit:pix
    _ISP_QUERY_RST(
        MUINT32 _x_pix = 0,
        MUINT32 _stride_pix = 0,
        MUINT32 _stride_byte = 0,
        MUINT32 _xsize_byte = 0,
        MUINT32 _crop_x = 0)
        : x_pix(_x_pix)
        , stride_pix(_stride_pix)
        , stride_byte(_stride_byte)
        , xsize_byte(_xsize_byte)
        , crop_x(_crop_x)
    {}
} ISP_QUERY_RST,*PISP_QUERY_RST;

//only add this constraint at imgo is because of rrzo have 8-alignment constraint. but imgo have only 2-alignment under 1-pixmode
#define _query_p2_stride_constraint( stride){\
    stride = (stride + 0x3) & (~0x3);\
}

#define query_rrzo_constraint(xsize){\
    xsize = (xsize + 0x7) & (~0x7);\
}

MUINT32 ISP_QuerySize(MUINT32 portId,MUINT32 e_Op,EImageFormat imgFmt,MUINT32 QueryInput,ISP_QUERY_RST &QueryRst ,E_ISP_PIXMODE e_PixMode=ISP_QUERY_UNKNOWN_PIX_MODE);

};  //namespace NSIspio
};  //namespace NSImageioIsp3

#endif  //  _ISPIO_PIPE_UTILITY_ISP3_H_



