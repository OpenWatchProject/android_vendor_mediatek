/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "cam_capibitiliy"

#include "cam_capibility.h"

#include <mtkcam/drv/def/ICam_type.h>
#include <mtkcam/drv/def/ispio_port_index.h>
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#include <mtkcam/def/ImageFormat.h>
#include "isp_drv_stddef.h"

#include <cutils/properties.h>  // For property_get().
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.

DECLARE_DBG_LOG_VARIABLE(cam_cap);
//EXTERN_DBG_LOG_VARIABLE(func_cam);

// Clear previous define, use our own define.
#undef CAM_CAP_DBG
#undef CAM_CAP_INF
#undef CAM_CAP_WRN
#undef CAM_CAP_ERR
#define CAM_CAP_DBG(fmt, arg...)        do {\
    if (cam_cap_DbgLogEnable_DEBUG  ) { \
        BASE_LOG_DBG("[%d]:" fmt,__LINE__, ##arg); \
    }\
} while(0)

#define CAM_CAP_INF(fmt, arg...)        do {\
    if (cam_cap_DbgLogEnable_INFO  ) { \
        BASE_LOG_INF("[%d]:" fmt,__LINE__, ##arg); \
    }\
} while(0)

#define CAM_CAP_WRN(fmt, arg...)        do {\
    if (cam_cap_DbgLogEnable_WARN   ) { \
        BASE_LOG_WRN("[%d]:" fmt,__LINE__, ##arg); \
    }\
} while(0)

#define CAM_CAP_ERR(fmt, arg...)        do {\
    if (cam_cap_DbgLogEnable_ERROR  ) { \
        BASE_LOG_ERR("" fmt, ##arg); \
    }\
} while(0)


capibility::capibility(NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM module)
{
    DBG_LOG_CONFIG(imageio, cam_cap);
    switch(module){
        case NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_A:
            m_hwModule = CAM_A;
            break;
        case NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_B:
            m_hwModule = CAM_B;
            break;
        case NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_C:
            m_hwModule = CAM_C;
            break;
        default:
            m_hwModule = CAM_MAX;
            CAM_CAP_ERR("unsupported module:%d",module);
            break;
    }
}

capibility::capibility(ISP_HW_MODULE module)
{
    DBG_LOG_CONFIG(imageio, cam_cap);
    switch(module){
        case CAM_A:
        case CAM_B:
        case CAM_C:
            m_hwModule = module;
            break;
        default:
            m_hwModule = CAM_MAX;
            CAM_CAP_ERR("unsupported module:%d",module);
            break;
    }
}


capibility::D_TWIN_CFG capibility::m_DTwin;
capibility::D_IQ_Version capibility::m_IQVer;


MBOOL capibility::GetCapibility(MUINT32 portId,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd e_Op,
                                        NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo inputInfo,tCAM_rst &QueryRst,E_CAM_Query_OP inerOP)
{
    MBOOL ret = MTRUE;

    if (inerOP == E_CAM_UNKNOWNN) {
        MUINT32 valid_cmd_cap = (
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_BS_RATIO |
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_QUERY_FMT |
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_PIPELINE_BITDEPTH|
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_D_Twin|
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_MAX_SEN_NUM|
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_D_BayerEnc|
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_IQ_LEVEL|
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_DYNAMIC_PAK|
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_BURST_NUM|
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_MAX_PREVIEW_SIZE|
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_SUPPORT_PATTERN|
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_NEED_CFG_SENSOR_BY_MW|
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_RSV);
        MUINT32 valid_cmd_constraint = (
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_PIX |
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_BYTE |
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_X_PIX |
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_X_BYTE |
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_START_X |
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_PIX |
                    NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE);

        if((e_Op & valid_cmd_cap) && (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_BS_RATIO)){
            if( (QueryRst.ratio = this->GetRatio(portId)) == 0 )
                ret = MFALSE;
        }

        if((e_Op & valid_cmd_cap) && (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_QUERY_FMT)){
            if( this->GetFormat(portId, QueryRst) == MFALSE )
                ret = MFALSE;
        }

        if((e_Op & valid_cmd_cap) && (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_PIPELINE_BITDEPTH)){
            QueryRst.pipelinebitdepth = this->GetPipelineBitdepth();
        }

        if(e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_SUPPORT_PATTERN){
            QueryRst.pattern = this->GetSupportPattern();
        }

        if((e_Op & valid_cmd_cap) && (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_MAX_PREVIEW_SIZE)){
            QueryRst.rrzMaxSize = this->GetRRZSize();
        }

        if((e_Op & valid_cmd_cap) && (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_BURST_NUM)){
            QueryRst.burstNum = this->GetSupportBurstNum();
        }

        if((e_Op & valid_cmd_cap) && (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_D_Twin)){
            //donothing, just use default vaule
            //QueryRst.D_TWIN;
        }

        if((e_Op & valid_cmd_cap) && (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_D_BayerEnc)){
            //donothing, just use default vaule
            //QueryRst.D_UF;
        }

        if((e_Op & valid_cmd_cap) && (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_DYNAMIC_PAK)){
            //donothing, just use default vaule
            //QueryRst.D_Pak;
        }

        if((e_Op & valid_cmd_cap) && (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_IQ_LEVEL)){
            QueryRst.IQlv = MTRUE; // support iq query
        }

        if((e_Op & valid_cmd_cap) && (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_MAX_SEN_NUM)){
            QueryRst.Sen_Num = this->GetMaxSenNum();
        }

        if ((e_Op & valid_cmd_cap) == NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_NEED_CFG_SENSOR_BY_MW) {
            QueryRst.needCfgSensorByMW = 1;
            CAM_CAP_INF("QueryRst.needCfgSensorByMW = %d", QueryRst.needCfgSensorByMW);
        }

        if((e_Op & valid_cmd_cap) && (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_RSV)){
            ret = this->GetHWModule(QueryRst);
        }

        if (e_Op & valid_cmd_constraint) {
            if (this->GetConstrainedSize(portId, e_Op, inputInfo, QueryRst) == MFALSE) {
                ret = MFALSE;
            }
        }

        if(e_Op & (~(valid_cmd_cap|valid_cmd_constraint))){
            ret = MFALSE;
            CAM_CAP_ERR("some query cmd(0x%x) is not supported. valid cmd(0x%x)\n",e_Op,(valid_cmd_cap|valid_cmd_constraint));
        }
    }
    else
        return this->GetCapibility_drv(portId,inerOP,QueryRst,inputInfo.pixelMode);

    return ret;
}


MBOOL capibility::GetCapibility(const tCapInPut& input,tCAM_rst &QueryRst)
{

    if((input.e_Op != NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN)||(input.inerOP == E_CAM_UNKNOWNN)){
        CAM_CAP_ERR("this api is supported yet,cmd(%d_%d)\n",input.e_Op,input.inerOP);
        return MFALSE;
    }

    if(input.inerOP & E_CAM_pipeline_size){
        if( (QueryRst.pipeSize = this->GetPipeSize((const tPipeLineSize*)input.pInput)) == 0){
            CAM_CAP_ERR("E_CAM_pipeline_size get fail\n");
            return MFALSE;
        }
    }

    return MTRUE;
}


MBOOL capibility::GetCapibility_drv(MUINT32 portId,E_CAM_Query_OP inerOP,tCAM_rst &QueryRst,E_CamPixelMode pixMode)
{
    MBOOL ret = MTRUE;
    portId;
    if(inerOP & (E_CAM_SRAM_SEP|E_CAM_SRAM_MRG)){
        if(this->GetDLPSramSize(&QueryRst.dlp) == 0){
            CAM_CAP_ERR("dlp sram size error\n");
            ret = MFALSE;
        }
        if(this->GetDLPLineBuffer(&QueryRst.dlp) == 0){
            CAM_CAP_ERR("dlp linebuffer error\n");
            ret = MFALSE;
        }
    }



    if(inerOP & E_CAM_pipeline_size){
        //not work at isp60. plz use capibility::GetCapibility(const tCapInPut&,tCAM_rst &)
        ret = MFALSE;
    }

    if(inerOP & E_CAM_BS_Max_size){
        if( (QueryRst.bs_info.bs_max_size = this->GetRRZSize()) == 0)
            ret = MFALSE;
    }
    else if(inerOP & E_CAM_BS_Alignment){
        if( (QueryRst.bs_info.bs_alignment = this->GetRLB_SRAM_Alignment()) == 0)
            ret = MFALSE;
    }

    if(inerOP & E_CAM_HEADER_size){
        if( (QueryRst.HeaderSize = this->GetHeaderSize()) == 0)
            ret = MFALSE;
    }

    if (inerOP & E_CAM_pipeline_min_size) {
        this->GetPipelineMinSize(&QueryRst.pipelineMinSize, pixMode);
    }

    return ret;
}

MUINT32 capibility::GetFormat(MUINT32 portID, tCAM_rst &QueryRst)
{

    if(QueryRst.Queue_fmt.size()){
        CAM_CAP_ERR("current portID(0x%x) Queue_fmt need init\n",portID);
        return MFALSE;
    }

    switch(portID){
        case NSImageio::NSIspio::EPortIndex_RAWI:
        case NSImageio::NSIspio::EPortIndex_IMGO:
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_BAYER8);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_BAYER10);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_BAYER12);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_BAYER16_APPLY_LSC);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_BAYER10_MIPI);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_BAYER8_UNPAK);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_BAYER10_UNPAK);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_BAYER12_UNPAK);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_BAYER15_UNPAK);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_UFO_BAYER10);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_UFO_BAYER12);
            break;
        case NSImageio::NSIspio::EPortIndex_RRZO:
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_FG_BAYER8);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_FG_BAYER10);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_FG_BAYER12);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_UFO_FG_BAYER10);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_UFO_FG_BAYER12);
            break;
        case NSImageio::NSIspio::EPortIndex_YUVO:
            //8bit 1p
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_YUY2);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_YVYU);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_UYVY);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_VYUY);
            //8bit 2p
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_NV16);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_NV61);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_NV21);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_NV12);
            //8bit 3p
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_I422);
            //QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_YV16);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_I420);
            //QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_YV12);
            //10bit 1p
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_MTK_YUYV_Y210);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_MTK_YVYU_Y210);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_MTK_UYVY_Y210);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_MTK_VYUY_Y210);
            //10bit 2p
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_MTK_YUV_P210);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_MTK_YVU_P210);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_MTK_YUV_P010);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_MTK_YVU_P010);
            //10bit 3p
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_MTK_YUV_P210_3PLANE);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_MTK_YUV_P010_3PLANE);
            //16bit 1p
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_YUYV_Y210);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_YVYU_Y210);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_UYVY_Y210);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_VYUY_Y210);
            //16bit 2p
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_YUV_P210);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_YVU_P210);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_YUV_P010);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_YVU_P010);
            //16bit 3p
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_YUV_P210_3PLANE);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_YUV_P010_3PLANE);
            break;
        case NSImageio::NSIspio::EPortIndex_CRZO:
            //8bit 1p
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_YUY2);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_YVYU);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_UYVY);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_VYUY);
            //8bit 2p
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_NV16);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_NV61);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_NV21);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_NV12);
            break;
        case NSImageio::NSIspio::EPortIndex_CRZO_R2:
            //8bit 1p
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_YUY2);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_YVYU);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_UYVY);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_VYUY);
            //8bit 2p
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_NV16);
            QueryRst.Queue_fmt.push_back(NSCam::eImgFmt_NV61);
            break;
        default:
            CAM_CAP_ERR("current portID(0x%x) are not supported in query\n",portID);
            return MFALSE;
            break;
    }
    return MTRUE;
}

MUINT32 capibility::GetRatio(MUINT32 portID)
{
    #define MAX_SCALING_DWON_RATIO  (6)        //unit:%

    switch(portID){
        case NSImageio::NSIspio::EPortIndex_RRZO:
            return MAX_SCALING_DWON_RATIO;
            break;
        default:
            CAM_CAP_ERR("curent portID(0x%x) are no scaler\n",portID);
            break;

    }
    return 100;
}

MUINT32 capibility::GetPipelineBitdepth()
{
    return (NSCam::NSIoPipe::NSCamIOPipe::CAM_Pipeline_12BITS);
}

MVOID capibility::GetPipelineMinSize(minSize* size, E_CamPixelMode pixMode)
{
    #define CAM_PIPELINE_MIN_WIDTH_SIZE  (120*4) //awb win_w is fixed in 120, 4 is for 1 block need 4 pixels.
    #define CAM_PIPELINE_MIN_HEIGHT_SIZE (90*2) //awb win_h is fixed in 90, 2 is for 1 block need 2 pixels.

    if (pixMode == ePixMode_4) { // 2 pixels for qbin_acc 1
        size->w = (CAM_PIPELINE_MIN_WIDTH_SIZE<<2);
        size->h = (CAM_PIPELINE_MIN_HEIGHT_SIZE<<2);
    }
    else if (pixMode == ePixMode_2) { // 2 pixels for qbin_acc 1
        size->w = (CAM_PIPELINE_MIN_WIDTH_SIZE<<1);
        size->h = (CAM_PIPELINE_MIN_HEIGHT_SIZE<<1);
    }
    else {
        size->w = CAM_PIPELINE_MIN_WIDTH_SIZE;
        size->h = CAM_PIPELINE_MIN_HEIGHT_SIZE;
    }
}

MUINT32 capibility::GetPipeSize(const tPipeLineSize* input)
{
    #define CAM_A_MAX_LINE_BUFFER_IN_PIXEL  (5504)
    #define CAM_B_MAX_LINE_BUFFER_IN_PIXEL  (5504)
    #define CAM_C_MAX_LINE_BUFFER_IN_PIXEL  (3624)
    MUINT32 value;

    switch(this->m_hwModule){
        case CAM_A:
            if(input->bBin){
                value = 8080;
            }
            else if(input->bFBN){
                value = 6623;
            }
            else{
                value = CAM_A_MAX_LINE_BUFFER_IN_PIXEL;
            }

            break;
        case CAM_B:
            if(input->bBin){
                value = 8080;
            }
            else if(input->bFBN){
                value = 6623;
            }
            else{
                value = CAM_B_MAX_LINE_BUFFER_IN_PIXEL;
            }
            break;
        case CAM_C:
            if(input->bBin){
                value = 8080;
            }
            else if(input->bFBN){
                value = 6623;
            }
            else{
                value = CAM_C_MAX_LINE_BUFFER_IN_PIXEL;
            }
            break;
        default:
            CAM_CAP_ERR("need to assign a hw module, like CAM_A ...etc.\n");
            value = 0;
            break;
    }

    return value;
}


MUINT32 capibility::GetRRZSize()
{
    #undef CAM_RRZ_MAX_LINE_BUFFER_IN_PIXEL
    #define CAM_RRZ_MAX_LINE_BUFFER_IN_PIXEL ((6144))

    return CAM_RRZ_MAX_LINE_BUFFER_IN_PIXEL;
}

MUINT32 capibility::GetSupportPattern()
{
    MUINT32 pattern;
    /* ISP 6.0 support pattern */
    pattern = 0x1 << eCAM_NORMAL         |
              0x1 << eCAM_DUAL_PIX       |
              0x1 << eCAM_QuadCode       |
              0x1 << eCAM_4CELL          |
              0x1 << eCAM_MONO           |
              0x1 << eCAM_IVHDR          |
              0x1 << eCAM_ZVHDR          |
              0x1 << eCAM_DUAL_PIX_IVHDR |
              0x1 << eCAM_DUAL_PIX_ZVHDR |
              0x1 << eCAM_YUV            |
              0x1 << eCAM_NORMAL_PD;

    return pattern;
}

MUINT32 capibility::GetSupportBurstNum()
{
    /*
    return support burst number
    0x0 : not support       // ISP 3.0
    0x2 : support 2
    0x6 : support 2|4
    0xE : support 2|4|8
    0x1E: support 2|4|8|16  // ISP 4.0 / ISP 6.0
    */
    return 0x1E;
}


MUINT32 capibility::GetRLB_SRAM_Alignment()
{
    #define RLB_ALIGNMENT (8)

    return RLB_ALIGNMENT;
}


MUINT32 capibility::GetDLPSramSize(tDlp* pDlp)
{
    //unit: word
    #define SRAM_SEP        (544)

    #define SRAM_MRG_R1     (228)
    #define SRAM_MRG_R2     (228)
    #define SRAM_MRG_R3     (8)
    #define SRAM_MRG_R4     (228)
    #define SRAM_MRG_R5     (228)
    #define SRAM_MRG_R7     (16)
    #define SRAM_MRG_R8     (300)
    #define SRAM_MRG_R10    (228)
    #define SRAM_MRG_R11    (16)
    #define SRAM_MRG_R12    (8)
    #define SRAM_MRG_R13    (16)
    #define SRAM_MRG_R14    (16)

    switch(this->m_hwModule){
        case CAM_A:
        case CAM_B:
            pDlp->module.sram_sep = SRAM_SEP;
            break;
        case CAM_C:
            pDlp->module.sram_sep = 506;
            break;
        default:
            CAM_CAP_ERR("need to assign a hw module, like CAM_A ...etc.\n");
            return 0;
            break;
    }

    pDlp->module.sram_mrg_r1 = SRAM_MRG_R1;
    pDlp->module.sram_mrg_r2 = SRAM_MRG_R2;
    pDlp->module.sram_mrg_r3 = SRAM_MRG_R3;
    pDlp->module.sram_mrg_r4 = SRAM_MRG_R4;
    pDlp->module.sram_mrg_r5 = SRAM_MRG_R5;
    pDlp->module.sram_mrg_r7 = SRAM_MRG_R7;
    pDlp->module.sram_mrg_r8 = SRAM_MRG_R8;
    pDlp->module.sram_mrg_r10 = SRAM_MRG_R10;
    pDlp->module.sram_mrg_r11 = SRAM_MRG_R11;
    pDlp->module.sram_mrg_r12 = SRAM_MRG_R12;
    pDlp->module.sram_mrg_r13 = SRAM_MRG_R13;
    pDlp->module.sram_mrg_r14 = SRAM_MRG_R14;

    return 1;
}

MUINT32 capibility::GetDLPLineBuffer(tDlp* pDlp)
{
#if 0
    #define DUAL_SEP_WD     (3456)//(6804)
    #define DUAL_MRG_R1_WD  (1728)


    pDlp->line_sep    = DUAL_SEP_WD;
    pDlp->line_mrg_r1 = DUAL_MRG_R1_WD;
    pDlp->line_mrg_r8 = this->GetRRZSize();
#endif
    return 1;
}

MUINT32 capibility::GetHeaderSize()
{
    #define spare_num   (16)
    return spare_num * 4;
}

MUINT32 capibility::GetMaxSenNum()
{
    #define CAM_TG_NUM      (3)
    #define CAMSV_TG_NUM    (6)
    switch(this->m_hwModule){
        case CAM_A:
        case CAM_B:
        case CAM_C:
            return CAM_TG_NUM;
            break;
        case CAMSV_0:
        case CAMSV_1:
        case CAMSV_2:
        case CAMSV_3:
        case CAMSV_4:
        case CAMSV_5:
            return CAMSV_TG_NUM;
            break;
        default:
            CAM_CAP_ERR("need to assign a hw module, like CAM_X,CAM_SV_X..etc.\n");
            break;
    }
    return 0;
}

#define _query_p2_stride_constraint( stride){\
    stride = (stride + 0x3) & (~0x3);\
}

#define query_rrzo_constraint(xsize,pixMode,imgFmt){\
    switch(pixMode){\
        case ePixMode_NONE:\
        case ePixMode_4:\
            xsize = (xsize + 0xf) & (~0xf);\
            break;\
        case ePixMode_2:\
            xsize = (xsize + 0x7) & (~0x7);\
            break;\
        case ePixMode_1:\
            if(imgFmt == NSCam::eImgFmt_FG_BAYER10)\
                xsize = (xsize + 0x3) & (~0x3);\
            else\
                xsize = (xsize + 0x7) & (~0x7);\
            break;\
        default:\
            xsize = (xsize + 0xf) & (~0xf);\
        break;\
    }\
}


static MUINT32 _query_fg_constraint(MUINT32 size)
{
    return (size&0x3)? 0 : 1;
}

static MUINT32 _query_fg_align_size(MUINT32 size)
{
    return ((size + 0x3) & ~0x3);
}

static MUINT32  _queryBitPerPix(MUINT32 const imgFmt)
{
    MUINT32 pixDepth;

    switch (imgFmt) {
        case NSCam::eImgFmt_BAYER8:          //= 0x0001,   //Bayer format, 8-bit
        case NSCam::eImgFmt_Y8:              //weighting table
        case NSCam::eImgFmt_FG_BAYER8:
            pixDepth = 8;
            break;
        case NSCam::eImgFmt_BAYER10:         //= 0x0002,   //Bayer format, 10-bit
        case NSCam::eImgFmt_FG_BAYER10:
            pixDepth = 10;
            break;
        case NSCam::eImgFmt_BAYER12:         //= 0x0004,   //Bayer format, 12-bit
        case NSCam::eImgFmt_FG_BAYER12:
            pixDepth = 12;
            break;
        case NSCam::eImgFmt_BAYER14:         //= 0x0008,   //Bayer format, 14-bit
        case NSCam::eImgFmt_BAYER16_APPLY_LSC:         //= 0x221A,   //Bayer format, 16-bit
            pixDepth = 16;//?
            break;
        case NSCam::eImgFmt_BAYER8_UNPAK:
        case NSCam::eImgFmt_BAYER10_UNPAK:
        case NSCam::eImgFmt_BAYER12_UNPAK:
        case NSCam::eImgFmt_BAYER14_UNPAK:
        case NSCam::eImgFmt_BAYER15_UNPAK:
            pixDepth = 16;
            break;
        case NSCam::eImgFmt_YUY2:            //= 0x0100,   //422 format, 1 plane (YUYV)
        case NSCam::eImgFmt_UYVY:            //= 0x0200,   //422 format, 1 plane (UYVY)
        case NSCam::eImgFmt_YVYU:            //= 0x080000,   //422 format, 1 plane (YVYU)
        case NSCam::eImgFmt_VYUY:            //= 0x100000,   //422 format, 1 plane (VYUY)
            pixDepth = 16;
            break;
        case NSCam::eImgFmt_NV16:  //user would set format_nv16 to all of 2 planes if multi-plane(img3o~img3bo)
        case NSCam::eImgFmt_NV12:
        case NSCam::eImgFmt_YV12:  //user would set format_yv12 to all of 3 planes if multi-plane(img3o~img3co)
        case NSCam::eImgFmt_I420:
            pixDepth = 8;   //temp use pixDepth 8
            break;
        case NSCam::eImgFmt_RGB565:
        case NSCam::eImgFmt_STA_2BYTE:
            pixDepth = 16;
            break;
        case NSCam::eImgFmt_RGB888:
            pixDepth = 24;
            break;
        case NSCam::eImgFmt_JPEG:
            pixDepth = 8;
            break;
        default:
            CAM_CAP_ERR("eImgFmt:[%d]NOT Support",imgFmt);
            return -1;
    }
    if( imgFmt == NSCam::eImgFmt_FG_BAYER8 ||
        imgFmt == NSCam::eImgFmt_FG_BAYER10 ||
        imgFmt == NSCam::eImgFmt_FG_BAYER12)
    {
        pixDepth = (pixDepth*3)/2;
    }
    return pixDepth;
}


static MUINT32 _queryCropStart(MUINT32 portId,NSCam::EImageFormat imgFmt,MUINT32 input, tCAM_rst &QueryRst,
                                        E_CamPixelMode e_PixMode)
{
    switch(portId){
    case NSImageio::NSIspio::EPortIndex_IMGO:
        switch(imgFmt){
        case NSCam::eImgFmt_BAYER10:
            switch(e_PixMode){
            case ePixMode_1:
                //if bus size 16bit, the inputer must be 8 alignment,
                QueryRst.crop_x = (input >> 3) << 3;
                break;
            case ePixMode_2:
                //if bus size 16bit, the inputer must be 16 alignment,
                QueryRst.crop_x = (input >> 4) << 4;
                break;
            default:
            case ePixMode_4:
                //if bus size 16bit, the inputer must be 32 alignment,
                QueryRst.crop_x = (input >> 5) << 5;
                break;
            }
            break;
        case NSCam::eImgFmt_BAYER12:
            switch(e_PixMode){
            case ePixMode_1:
                //if bus size 16bit, the inputer must be 4 alignment,
                QueryRst.crop_x = (input >> 2) << 2;
                break;
            case ePixMode_2:
                //if bus size 16bit, the inputer must be 8 alignment,
                QueryRst.crop_x = (input >> 3) << 3;
                break;
            default:
            case ePixMode_4:
                //if bus size 16bit, the inputer must be 16 alignment,
                QueryRst.crop_x = (input >> 4) << 4;
                break;
            }
            break;
        case NSCam::eImgFmt_BAYER8:
        case NSCam::eImgFmt_FG_BAYER8:
        case NSCam::eImgFmt_FG_BAYER10:
        case NSCam::eImgFmt_FG_BAYER12:
            switch(e_PixMode){
            case ePixMode_1:
                //if bus size 16bit, the inputer must be 2 alignment,
                QueryRst.crop_x = (input >> 1) << 1;
                break;
            case ePixMode_2:
                //if bus size 16bit, the inputer must be 4 alignment,
                QueryRst.crop_x = (input >> 2) << 2;
                break;
            default:
            case ePixMode_4:
                //if bus size 16bit, the inputer must be 8 alignment,
                QueryRst.crop_x = (input >> 3) << 3;
                break;
            }
            break;
        case NSCam::eImgFmt_BAYER8_UNPAK:
        case NSCam::eImgFmt_BAYER10_UNPAK:
        case NSCam::eImgFmt_BAYER12_UNPAK:
        case NSCam::eImgFmt_BAYER14_UNPAK:
        case NSCam::eImgFmt_BAYER15_UNPAK:
            QueryRst.crop_x = 0;
            break;
        default:
            QueryRst.crop_x = 0;
            CAM_CAP_ERR("NOT SUPPORT imgFmt(%d)",imgFmt);
            return 0;
            break;
        }
        break;
    case NSImageio::NSIspio::EPortIndex_RRZO://rrz support only rrz_in crop, not dma crop
        input -= (input&0x1);
        QueryRst.crop_x = input;
        break;
    default:
        CAM_CAP_ERR("NOT SUPPORT port(%d)",portId);
        break;
    }

    return 1;
}


static MUINT32 _query_p2_constraint(MUINT32 size)
{
    return (size&0x1) ? 0 : 1;
}

static MUINT32 _query_pix_mode_constraint(MUINT32 size, MUINT32 pixMode){
    /*
    1 pix mode => 2n
    2 pix mode => 4n
    but current flow , user have no pix mode information, so fix 4n for safety
    */
    MUINT32 divisor = 0x3;
    switch(pixMode){
        case ePixMode_NONE:
        case ePixMode_4:
            divisor = 0x7;
            break;
        case ePixMode_2:
            divisor = 0x3;
            break;
        case ePixMode_1:
            divisor = 0x1;
            break;
        default:
            CAM_CAP_ERR("pix mode error(%d)",pixMode);
            break;
    }
    if(size & divisor)
        return 0;
    else
        return 1;
}
static MUINT32 _query_pix_mode_align_size(MUINT32 size, MUINT32 pixMode)
{
    MUINT32 divisor = 0x7, align_size = size;

    switch (pixMode) {
        case ePixMode_NONE:
        case ePixMode_4:
            divisor = 0x7;
            break;
        case ePixMode_2:
            divisor = 0x3;
            break;
        case ePixMode_1:
            divisor = 0x1;
            break;
        default:
            CAM_CAP_ERR("pix mode error(%d)", pixMode);
            break;
    }

    align_size = ((align_size + divisor) & ~divisor);

    return align_size;
}

static MBOOL _queryXsize_imgo(NSCam::EImageFormat imgFmt, MUINT32 xsize[3], MUINT32& outWidth,
                                        E_CamPixelMode pixMode, MBOOL bCrop)
{
    if(outWidth&0x3)
        outWidth += (4- (outWidth&0x3));

    switch(imgFmt){
    case NSCam::eImgFmt_BAYER8:
    case NSCam::eImgFmt_JPEG:
        xsize[0] = outWidth;

        //expand stride, instead of shrink width
        if (!(_query_pix_mode_constraint(xsize[0], pixMode) && _query_p2_constraint(xsize[0]))) {
            MUINT32 aligned = _query_pix_mode_align_size(xsize[0], pixMode);

            if (_query_p2_constraint(aligned)) {
                CAM_CAP_ERR("constraint: align xsize(%d/%d)\n", xsize[0], aligned);
            }
            else {
                CAM_CAP_ERR("constraint: cannot pass p2 constraint(%d)\n", xsize[0]);
            }
            xsize[0] = aligned;
        }
        break;
    case NSCam::eImgFmt_BAYER10_MIPI:
    case NSCam::eImgFmt_BAYER10:
        if (bCrop == MFALSE) {//do nothing
        }
        else {
            //width need 4 alignment to make sure xsize is 8-alignment
            if (outWidth & 0x2) {
                CAM_CAP_WRN("WARNING: constraint: 4 align for outWidth(%d)\n", outWidth);
                outWidth -= 2;
            }
        }
        xsize[0] = (outWidth*10 + 7) / 8;
        xsize[0] += (xsize[0] & 0x1);

        //expand stride, instead of shrink width
        if (!((_query_pix_mode_constraint(xsize[0], pixMode) && _query_p2_constraint(xsize[0])))) {
            MUINT32 aligned = _query_pix_mode_align_size(xsize[0], pixMode);

            if (_query_p2_constraint(aligned)) {
                CAM_CAP_DBG("constraint: align xsize(%d/%d)\n", xsize[0], aligned);
            }
            else {
                CAM_CAP_ERR("constraint: cannot pass p2 constraint(%d)\n", xsize[0]);
            }
            xsize[0] = aligned;
        }

        break;
    case NSCam::eImgFmt_BAYER12:
        //no need to patch under cropping , because outWidth must be at leaset 2-alignment
        xsize[0] = (outWidth*12 + 7) / 8;
        xsize[0] += (xsize[0] & 0x1);

        //expand stride, instead of shrink width
        if (!((_query_pix_mode_constraint(xsize[0], pixMode) && _query_p2_constraint(xsize[0])))) {
            MUINT32 aligned = _query_pix_mode_align_size(xsize[0], pixMode);

            if (_query_p2_constraint(aligned)) {
                CAM_CAP_DBG("constraint: align xsize(%d/%d)\n", xsize[0], aligned);
            }
            else {
                CAM_CAP_ERR("constraint: cannot pass p2 constraint(%d)\n", xsize[0]);
            }
            xsize[0] = aligned;
        }
        break;
    case NSCam::eImgFmt_BAYER8_UNPAK:
    case NSCam::eImgFmt_BAYER10_UNPAK:
    case NSCam::eImgFmt_BAYER12_UNPAK:
    case NSCam::eImgFmt_BAYER15_UNPAK:
    case NSCam::eImgFmt_BAYER16_APPLY_LSC:
        xsize[0] = outWidth * 2; //fixed 16 bit
        xsize[0] += (xsize[0] & 0x1);

        if (!((_query_pix_mode_constraint(xsize[0], pixMode) && _query_p2_constraint(xsize[0])))) {
            MUINT32 aligned = _query_pix_mode_align_size(xsize[0], pixMode);

            if( _query_p2_constraint(xsize[0]))
                CAM_CAP_DBG("constraint: align xsize(%d/%d)\n", xsize[0], aligned);
            else {
                CAM_CAP_ERR("constraint: cannot pass p2 constraint(%d)\n", xsize[0]);
            }
            xsize[0] = aligned;
        }
        break;
    case NSCam::eImgFmt_YUY2:
    case NSCam::eImgFmt_UYVY:
    case NSCam::eImgFmt_YVYU:
    case NSCam::eImgFmt_VYUY:
        xsize[0] = outWidth * 2;
        //expand stride, instead of shrink width
        if (!((_query_pix_mode_constraint(xsize[0], pixMode) && _query_p2_constraint(xsize[0])))) {
            MUINT32 aligned = _query_pix_mode_align_size(xsize[0], pixMode);

            if (_query_p2_constraint(aligned)) {
                CAM_CAP_DBG("constraint: align xsize(%d/%d)\n", xsize[0], aligned);
            }
            else {
                CAM_CAP_ERR("constraint: cannot pass p2 constraint(%d)\n", xsize[0]);
            }
            xsize[0] = aligned;
        }
        break;
    case NSCam::eImgFmt_RGB888:
    case NSCam::eImgFmt_RGB565:
        xsize[0] = outWidth * 2;
        //expand stride, instead of shrink width
        if (!(_query_pix_mode_constraint(xsize[0], pixMode) && _query_p2_constraint(xsize[0]))) {
            MUINT32 aligned = _query_pix_mode_align_size(xsize[0], pixMode);

            if (_query_p2_constraint(aligned)) {
                CAM_CAP_DBG("constraint: align xsize(%d/%d)\n", xsize[0], aligned);
            }
            else {
                CAM_CAP_ERR("constraint: cannot pass p2 constraint(%d)\n", xsize[0]);
            }
            xsize[0] = aligned;
        }
        CAM_CAP_ERR("NOT SUPPORT imgFmt(%d)",imgFmt);
        break;
    case NSCam::eImgFmt_BLOB:
        xsize[0] = outWidth;
        break;
    case NSCam::eImgFmt_UFO_BAYER8:
        xsize[0] = ((((outWidth+63)>>6)<<6)<<3)>>3;
        xsize[1] = ((((outWidth+63)>>6) + 7)>>3)<<3;
        xsize[2] = sizeof(UFDG_META_INFO);
        break;
    case NSCam::eImgFmt_UFO_BAYER10:
        xsize[0] = ((((outWidth+63)>>6)<<6)*10)>>3;
        xsize[1] = ((((outWidth+63)>>6) + 7)>>3)<<3;
        xsize[2] = sizeof(UFDG_META_INFO);
        break;
    case NSCam::eImgFmt_UFO_BAYER12:
        xsize[0] = ((((outWidth+63)>>6)<<6)*12)>>3;
        xsize[1] = ((((outWidth+63)>>6) + 7)>>3)<<3;
        xsize[2] = sizeof(UFDG_META_INFO);
        break;
    default:
        xsize[0] = outWidth = 0;
        CAM_CAP_ERR("NOT SUPPORT imgFmt(%d)",imgFmt);
        return MFALSE;
        break;
    }
    return MTRUE;
}

static MBOOL _queryXsize_rrzo(NSCam::EImageFormat imgFmt, MUINT32 xsize[3], MUINT32& outWidth,
                                        E_CamPixelMode pixMode)
{
    if(outWidth&0x3)
        outWidth += (4- (outWidth&0x3));

    switch(imgFmt){
    case NSCam::eImgFmt_FG_BAYER8:
        xsize[0] = outWidth*3>>1;
        query_rrzo_constraint(xsize[0],pixMode,imgFmt);

        //expand stride, instead of shrink width
        if (!_query_fg_constraint(xsize[0])) {
            MUINT32 aligned = _query_fg_align_size(xsize[0]);

            CAM_CAP_INF("constraint: align fg xsize(%d/%d)\n", xsize[0], aligned);
            xsize[0] = aligned;
        }
        break;
    case NSCam::eImgFmt_FG_BAYER10:
        xsize[0] = outWidth*3>>1;
        xsize[0] = (xsize[0]*10 + 7) / 8;
        query_rrzo_constraint(xsize[0],pixMode,imgFmt);

        //expand stride, instead of shrink width
        if (!_query_fg_constraint(xsize[0])) {
            MUINT32 aligned = _query_fg_align_size(xsize[0]);

            CAM_CAP_INF("constraint: align fg xsize(%d/%d)\n", xsize[0], aligned);
            xsize[0] = aligned;
        }
        break;
    case NSCam::eImgFmt_FG_BAYER12:
        xsize[0] = outWidth*3>>1;
        xsize[0] = (xsize[0]*12 + 7) / 8;
        query_rrzo_constraint(xsize[0],pixMode,imgFmt);

        //expand stride, instead of shrink width
        if (!_query_fg_constraint(xsize[0])) {
            MUINT32 aligned = _query_fg_align_size(xsize[0]);

            CAM_CAP_INF("constraint: align fg xsize(%d/%d)\n", xsize[0], aligned);
            xsize[0] = aligned;
        }
        break;
    case NSCam::eImgFmt_UFO_FG_BAYER8:
        xsize[0] = ((((outWidth+63)>>6)<<6)*3/2*8)>>3;
        xsize[1] = ((outWidth+63)>>6);
        xsize[2] = sizeof(UFDG_META_INFO);
        break;
    case NSCam::eImgFmt_UFO_FG_BAYER10:
        xsize[0] = ((((outWidth+63)>>6)<<6)*3/2*10)>>3;
        xsize[1] = ((outWidth+63)>>6);
        xsize[2] = sizeof(UFDG_META_INFO);
        break;
    case NSCam::eImgFmt_UFO_FG_BAYER12:
        xsize[0] = ((((outWidth+63)>>6)<<6)*3/2*12)>>3;
        xsize[1] = ((outWidth+63)>>6);
        xsize[2] = sizeof(UFDG_META_INFO);
        break;
    default:
        xsize[0] = outWidth = 0;
        CAM_CAP_ERR("rrzo NOT SUPPORT imgFmt(%d)",imgFmt);
        return MFALSE;
        break;
    }
    return MTRUE;
}

class yuvo_xsize
{
    public:
        virtual ~yuvo_xsize(){}
    public:
        virtual MBOOL constraint(NSCam::EImageFormat imgFmt, MUINT32 xsize[3], MUINT32& outWidth,E_CamPixelMode pixMode);
    protected:
                MBOOL _constraint(NSCam::EImageFormat const &imgFmt, MUINT32 xsize[3], MUINT32& outWidth,E_CamPixelMode const& pixMode);
                        //4-byte alignment
                MUINT32 _alignment(MUINT32 input,MUINT32 _align);
        virtual MUINT32 x_align(void) {return 4;}
        virtual MUINT32 max_width(void);
};

class crzo_xsize : public yuvo_xsize
{
    public:
        virtual ~crzo_xsize(){}
    public:
        virtual MBOOL constraint(NSCam::EImageFormat imgFmt, MUINT32 xsize[3], MUINT32& outWidth,E_CamPixelMode pixMode);
    protected:
        //2-byte alignment
        virtual MUINT32 x_align(void) {return 2;}
        virtual MUINT32 max_width(void);
};

class crzo_r2_xsize : public crzo_xsize
{
    public:
        virtual ~crzo_r2_xsize(){}
    public:
        virtual MBOOL constraint(NSCam::EImageFormat imgFmt, MUINT32 xsize[3], MUINT32& outWidth,E_CamPixelMode pixMode);
    protected:
        virtual MUINT32 max_width(void);
};

MBOOL yuvo_xsize::constraint(NSCam::EImageFormat imgFmt, MUINT32 xsize[3], MUINT32& outWidth,E_CamPixelMode pixMode)
{
    MBOOL rst = MTRUE;

    if(this->_constraint(imgFmt,xsize, outWidth,pixMode) == MFALSE){
        switch(imgFmt){
            //8-bit
            //2-plane
            case NSCam::eImgFmt_NV21:
            case NSCam::eImgFmt_NV12:
                xsize[0] = this->_alignment(outWidth,this->x_align());
                xsize[1] = this->_alignment((outWidth>>1),this->x_align());
                break;
            //3-plane
            case NSCam::eImgFmt_I422:
            //case NSCam::eImgFmt_YV16:
                xsize[0] = this->_alignment(outWidth,this->x_align());
                xsize[1] = this->_alignment((outWidth>>1),this->x_align());
                xsize[2] = xsize[1];
                break;
            case NSCam::eImgFmt_I420:
            //case NSCam::eImgFmt_YV12:
                xsize[0] = this->_alignment(outWidth,this->x_align());
                xsize[1] = this->_alignment((outWidth>>1),this->x_align());
                xsize[2] = xsize[1];
                break;
            //10-bit
            //1-plane
            case NSCam::eImgFmt_MTK_YUYV_Y210:
            case NSCam::eImgFmt_MTK_YVYU_Y210:
            case NSCam::eImgFmt_MTK_UYVY_Y210:
            case NSCam::eImgFmt_MTK_VYUY_Y210:
                xsize[0] = (outWidth*2*10 + 7) / 8;
                xsize[0] = this->_alignment(xsize[0],this->x_align());
                break;
            //2-plane
            case NSCam::eImgFmt_MTK_YUV_P210:
            case NSCam::eImgFmt_MTK_YVU_P210:
                xsize[0] = (outWidth*10 + 7) / 8;
                xsize[0] = this->_alignment(xsize[0],this->x_align());
                xsize[1] = xsize[0];
                break;
            case NSCam::eImgFmt_MTK_YUV_P010:
            case NSCam::eImgFmt_MTK_YVU_P010:
                xsize[0] = (outWidth*10 + 7) / 8;
                xsize[0] = this->_alignment(xsize[0],this->x_align());
                xsize[1] = xsize[0];
                break;
            //3-plane
            case NSCam::eImgFmt_MTK_YUV_P210_3PLANE:
                xsize[0] = (outWidth*10 + 7) / 8;
                xsize[0] = this->_alignment(xsize[0],this->x_align());
                xsize[1] = (outWidth*5 + 7) / 8;
                xsize[1] = this->_alignment(xsize[1],this->x_align());
                xsize[2] = xsize[1];
                break;
            case NSCam::eImgFmt_MTK_YUV_P010_3PLANE:
                xsize[0] = (outWidth*10 + 7) / 8;
                xsize[0] = this->_alignment(xsize[0],this->x_align());
                xsize[1] = (outWidth*5 + 7) / 8;
                xsize[1] = this->_alignment(xsize[1],this->x_align());
                xsize[2] = xsize[1];
                break;
            //16-bit
            //1-plane
            case NSCam::eImgFmt_YUYV_Y210:
            case NSCam::eImgFmt_YVYU_Y210:
            case NSCam::eImgFmt_UYVY_Y210:
            case NSCam::eImgFmt_VYUY_Y210:
                xsize[0] = this->_alignment((outWidth << 2),this->x_align());
                break;
            //2-plane
            case NSCam::eImgFmt_YUV_P210:
            case NSCam::eImgFmt_YVU_P210:
                xsize[0] = this->_alignment((outWidth<<1),this->x_align());
                xsize[1] = xsize[0];
                break;
            case NSCam::eImgFmt_YUV_P010:
            case NSCam::eImgFmt_YVU_P010:
                xsize[0] = this->_alignment((outWidth<<1),this->x_align());
                xsize[1] = xsize[0];
                break;
            //3-plane
            case NSCam::eImgFmt_YUV_P210_3PLANE:
                xsize[0] = this->_alignment((outWidth<<1),this->x_align());
                xsize[1] = this->_alignment(outWidth,this->x_align());
                xsize[2] = xsize[1];
                break;
            case NSCam::eImgFmt_YUV_P010_3PLANE:
                xsize[0] = this->_alignment((outWidth<<1),this->x_align());
                xsize[1] = this->_alignment(outWidth,this->x_align());
                xsize[2] = xsize[1];
                break;
            default:
                rst = MFALSE;
                CAM_CAP_ERR("NOT supported imgFmt(%d)",imgFmt);
                break;
        }
    }
    return rst;
}

MBOOL yuvo_xsize::_constraint(NSCam::EImageFormat const &imgFmt, MUINT32 xsize[3], MUINT32& outWidth,E_CamPixelMode const& pixMode)
{
    MBOOL rst = MTRUE;

    if(pixMode != ePixMode_1){
        CAM_CAP_WRN("###   support only 1pix mode output   ###\n");
        //pix mode is useless here.
    }

    if(outWidth > this->max_width()){
        CAM_CAP_WRN("####   max supported size is %d(%d), clamp it   ###\n",this->max_width(),outWidth);
        outWidth = this->max_width();
    }
    else if(outWidth&0x1){
        CAM_CAP_WRN("###   width must be 2-alignment,shrink width   ###\n");
        outWidth--;
    }

    switch(imgFmt){
        //8-bit
        //1-plane
        case NSCam::eImgFmt_YUY2:
        case NSCam::eImgFmt_YVYU:
        case NSCam::eImgFmt_UYVY:
        case NSCam::eImgFmt_VYUY:
            xsize[0] = this->_alignment((outWidth << 1),this->x_align());
            break;
        //2-plane
        case NSCam::eImgFmt_NV16:
        case NSCam::eImgFmt_NV61:
            xsize[0] = this->_alignment(outWidth,this->x_align());
            xsize[1] = xsize[0];
            break;
        default:
            xsize[0] = outWidth = 0;
            CAM_CAP_DBG("NOT common imgFmt(%d)",imgFmt);
            rst = MFALSE;
            break;
    }
    return rst;
}


MUINT32 yuvo_xsize::_alignment(MUINT32 input,MUINT32 _align)
{
    MUINT32 rst = input;
    MUINT32 tmp = rst & (_align-1);
    if(tmp){
      rst = input + _align - tmp;
    }
    return rst;
}

MUINT32 yuvo_xsize::max_width(void)
{
    capibility cap(CAM_A);//workaround, just for stride check
    tCapInPut input;
    tCAM_rst QueryRst;
    tPipeLineSize size;

    input.inerOP = E_CAM_pipeline_size;
    input.e_Op = NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNKNOWN;
    input.pInput = &size;
    size.bBin = size.bFBN = MFALSE;
    if(!cap.GetCapibility(input,QueryRst))
        CAM_CAP_ERR("GetCapibility failed");

    return QueryRst.pipeSize;
}

MUINT32 crzo_xsize::max_width(void)
{
    return 640;
}

MBOOL crzo_xsize::constraint(NSCam::EImageFormat imgFmt, MUINT32 xsize[3], MUINT32& outWidth,E_CamPixelMode pixMode)
{
    MBOOL rst = MTRUE;

    if(this->_constraint(imgFmt,xsize, outWidth,pixMode) == MFALSE){
        switch(imgFmt){
            case NSCam::eImgFmt_NV21:
            case NSCam::eImgFmt_NV12:
                xsize[0] = this->_alignment(outWidth,this->x_align());
                xsize[1] = xsize[0];
                break;
            default:
                rst = MFALSE;
                CAM_CAP_ERR("NOT supported imgFmt(%d)",imgFmt);
                break;
        }
    }

    return rst;
}

MUINT32 crzo_r2_xsize::max_width(void)
{
    return 768;
}

MBOOL crzo_r2_xsize::constraint(NSCam::EImageFormat imgFmt, MUINT32 xsize[3], MUINT32& outWidth,E_CamPixelMode pixMode)
{
    MBOOL rst = MTRUE;

    if( this->_constraint(imgFmt,xsize, outWidth,pixMode) == MFALSE){
        CAM_CAP_ERR("NOT supported imgFmt(%d)",imgFmt);
        rst = MFALSE;
    }

    return rst;
}

MUINT32 capibility::GetConstrainedSize(MUINT32 portId, NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd e_Op,
                                                    NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo inputInfo, tCAM_rst &QueryRst)
{
    MUINT32 outWidth = inputInfo.width;

    switch(portId){
    case NSImageio::NSIspio::EPortIndex_IMGO:
    case NSImageio::NSIspio::EPortIndex_RRZO:
    case NSImageio::NSIspio::EPortIndex_RAWI:
        {
            MUINT32 Xsize[3];
            MBOOL   _bCrop = MFALSE;
            if(portId == NSImageio::NSIspio::EPortIndex_RRZO){// rrzo
                if (_queryXsize_rrzo(inputInfo.format, Xsize, outWidth, inputInfo.pixelMode) == MFALSE) {
                    return MFALSE;
                }
            }
            else{
                if (e_Op & (NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_X_PIX|NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_X_BYTE)) {
                    _bCrop = MTRUE;
                }
                else {
                    _bCrop = MFALSE;
                }

                if (_queryXsize_imgo(inputInfo.format, Xsize, outWidth, inputInfo.pixelMode, _bCrop) == MFALSE) {
                    return MFALSE;
                }
            }


            if (e_Op & (NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_PIX|NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_X_PIX)) {
                QueryRst.x_pix = outWidth;
            }
            if (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_PIX) {
                QueryRst.stride_pix = Xsize[0] / _queryBitPerPix(inputInfo.format);
            }
            if (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE) {
                QueryRst.stride_byte[0] = Xsize[0];
                switch(inputInfo.format){
                    case NSCam::eImgFmt_UFO_BAYER8:
                    case NSCam::eImgFmt_UFO_BAYER10:
                    case NSCam::eImgFmt_UFO_BAYER12:
                    case NSCam::eImgFmt_UFO_FG_BAYER8:
                    case NSCam::eImgFmt_UFO_FG_BAYER10:
                    case NSCam::eImgFmt_UFO_FG_BAYER12:
                        QueryRst.stride_byte[1] = ((Xsize[1] + 7)>>3)<<3;
                        QueryRst.stride_byte[2] = Xsize[2];
                        //16byte-align
                        QueryRst.stride_byte[0] = ((QueryRst.stride_byte[0] + 15)>>4)<<4;
                        break;
                    default:
                        break;
                }
                _query_p2_stride_constraint(QueryRst.stride_byte[0]);
            }
            if (e_Op & (NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_BYTE|NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_X_BYTE)) {
                QueryRst.xsize_byte[0] = Xsize[0];
                switch(inputInfo.format){
                    case NSCam::eImgFmt_UFO_BAYER8:
                    case NSCam::eImgFmt_UFO_BAYER10:
                    case NSCam::eImgFmt_UFO_BAYER12:
                    case NSCam::eImgFmt_UFO_FG_BAYER8:
                    case NSCam::eImgFmt_UFO_FG_BAYER10:
                    case NSCam::eImgFmt_UFO_FG_BAYER12:
                        QueryRst.xsize_byte[1] = Xsize[1];
                        break;
                    default:
                        break;
                }
            }
            if (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_START_X) {
                if (_queryCropStart(portId, inputInfo.format, inputInfo.width, QueryRst, inputInfo.pixelMode) == 0) {
                    CAM_CAP_ERR("unsupported format:0x%x\n",inputInfo.format);
                }
            }
        }
        break;
    case NSImageio::NSIspio::EPortIndex_CAMSV_IMGO:
        {
            MUINT32 xsize[3];
            MBOOL   _bCrop = MFALSE;
            if (e_Op & (NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_X_PIX|NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_X_BYTE)) {
                _bCrop = MTRUE;
            }
            else {
                _bCrop = MFALSE;
            }

            if (_queryXsize_imgo(inputInfo.format, xsize, outWidth, inputInfo.pixelMode, _bCrop) == MFALSE) {
                return MFALSE;
            }

            if (e_Op & (NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_PIX|NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_X_PIX)) {
                QueryRst.x_pix = outWidth;
            }
            if (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_PIX) {
                QueryRst.stride_pix = xsize[0] / _queryBitPerPix(inputInfo.format);
            }
            if (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE) {
                QueryRst.stride_byte[0] = xsize[0];
                _query_p2_stride_constraint(QueryRst.stride_byte[0]);
            }
            if (e_Op & (NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_BYTE|NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_X_BYTE)) {
                QueryRst.xsize_byte[0] = xsize[0];
            }
            if (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_START_X) {
                if (_queryCropStart(portId, inputInfo.format, inputInfo.width, QueryRst, inputInfo.pixelMode) == 0) {
                    CAM_CAP_ERR("unsupported format:0x%x\n",inputInfo.format);
                }
            }
        }
        break;
    case NSImageio::NSIspio::EPortIndex_YUVO:
    case NSImageio::NSIspio::EPortIndex_CRZO:
    case NSImageio::NSIspio::EPortIndex_CRZO_R2:
        {
            MUINT32 Xsize[3] = {0,0,0};

            if(portId == NSImageio::NSIspio::EPortIndex_YUVO){
                yuvo_xsize _yuvo;
                if (_yuvo.constraint(inputInfo.format, Xsize, outWidth, inputInfo.pixelMode) == MFALSE) {
                    return MFALSE;
                }
            }
            else if(portId == NSImageio::NSIspio::EPortIndex_CRZO){
                crzo_xsize _crzo;
                if (_crzo.constraint(inputInfo.format, Xsize, outWidth, inputInfo.pixelMode) == MFALSE) {
                    return MFALSE;
                }
            }
            else{
                crzo_r2_xsize _crzo_r2;
                if (_crzo_r2.constraint(inputInfo.format, Xsize, outWidth, inputInfo.pixelMode) == MFALSE) {
                    return MFALSE;
                }
            }

            if (e_Op & (NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_PIX|NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_X_PIX)) {
                QueryRst.x_pix = outWidth;
            }
            if (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_PIX) {
                CAM_CAP_ERR("unsupported cmd:ENPipeQueryCmd_STRIDE_PIX\n");
                QueryRst.stride_pix = 0;
            }
            if (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE) {
                QueryRst.stride_byte[0] = Xsize[0];
                QueryRst.stride_byte[1] = Xsize[1];
                QueryRst.stride_byte[2] = Xsize[2];
            }
            if (e_Op & (NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_BYTE|NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_X_BYTE)) {
                QueryRst.xsize_byte[0] = Xsize[0];
                QueryRst.xsize_byte[1] = Xsize[1];
                QueryRst.xsize_byte[2] = Xsize[2];
            }
            if (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_START_X) {
                QueryRst.crop_x = 0;
                CAM_CAP_WRN("crop is not supported, always crop from 0\n");
            }
        }
        break;
    case NSImageio::NSIspio::EPortIndex_YUVBO:
    case NSImageio::NSIspio::EPortIndex_YUVCO:
        if (e_Op == NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_START_X) {
            QueryRst.crop_x = 0;
            CAM_CAP_WRN("crop is not supported, always crop from 0\n");
        }
        else{
            CAM_CAP_ERR("current portID(0x%x) r not supported in query cmd:0x%x\n",portId,e_Op);
        }
        break;
    default:
        CAM_CAP_ERR("current portID(0x%x) r not supported in query\n",portId);
        QueryRst.x_pix = QueryRst.stride_pix = QueryRst.stride_byte[0] = 0;
        return MFALSE;
        break;
    }

    return MTRUE;
}

MBOOL capibility::D_TWIN_CFG::SetDTwin(MBOOL ON)
{
    CAM_CAP_INF("%s Dynamic twin!!!!\n", (ON ? "Enable" : "Disable"));
    return this->SetInfo((MUINT32*)&ON);
}

MBOOL capibility::D_TWIN_CFG::GetDTwin(void)
{
    MBOOL ON = MFALSE;
    if(this->GetInfo((MUINT32*)&ON) == MFALSE)
        CAM_CAP_ERR("Get Dynamic twin Fail!!!!\n");

    return ON;
}

MBOOL C_HW_CFG::SetInfo(MUINT32* pIn)
{
    if(pIn == NULL)
        return MFALSE;
    m_HW_Info = *pIn;
    return MTRUE;
}
MBOOL C_HW_CFG::GetInfo(MUINT32* pOut)
{
    if(pOut == NULL)
        return MFALSE;
    *pOut = m_HW_Info;
    return MTRUE;
}

MBOOL capibility::GetHWModule(tCAM_rst &output)
{
    switch(this->m_hwModule){
        case CAM_A:
            fill((MBOOL*)&(output.bSupportedModule), (MBOOL*)&(output.bSupportedModule) + (sizeof(tIsModule)/sizeof(MBOOL)), MTRUE);
            break;
        case CAM_B:
            fill((MBOOL*)&(output.bSupportedModule), (MBOOL*)&(output.bSupportedModule) + (sizeof(tIsModule)/sizeof(MBOOL)), MTRUE);
            output.bSupportedModule.bRAWI = MFALSE;
            break;
        case CAM_C:
            output.bSupportedModule.bPDE = MTRUE; //raw c could support pde
            break;
        case CAMSV_0:
        case CAMSV_1:
        case CAMSV_2:
        case CAMSV_3:
        case CAMSV_4:
        case CAMSV_5:
            //use default value.  default vaule is that function r not supported
            break;
        default:
            CAM_CAP_ERR("need to assign a hw module, like CAM_X,CAM_SV_X..etc.\n");
            return MFALSE;
            break;
    }


    return MTRUE;
}


