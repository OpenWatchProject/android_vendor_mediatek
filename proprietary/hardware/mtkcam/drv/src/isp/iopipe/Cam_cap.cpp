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
#define LOG_TAG "cam_capi"

#include <Cam/ICam_capibility.h>
#include <cam/cam_capibility.h>
#include <cutils/log.h>

#ifndef NULL
#define NULL 0
#endif

CAM_CAPIBILITY::CAM_CAPIBILITY()
{
    m_hwModule = NSCam::NSIoPipe::NSCamIOPipe::ENPipe_UNKNOWN;
    memset((void*)m_Name, 0, sizeof(m_Name));
}
CAM_CAPIBILITY::~CAM_CAPIBILITY()
{}

static CAM_CAPIBILITY QueryObj[NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_MAX];

CAM_CAPIBILITY* CAM_CAPIBILITY::CreateInsatnce(char const* szCallerName,NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM module)
{
    MUINT32 num;
    if(szCallerName == NULL){
        ALOGE(LOG_TAG"[%s]ERR(%5d):no user name\n" , __FUNCTION__, __LINE__);
        return NULL;
    }

    switch(module){
        case NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_A:
            num = 1;
            break;
        case NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_B:
            num = 2;
            break;
        case NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_C:
            num = 3;
            break;
        case NSCam::NSIoPipe::NSCamIOPipe::ENPipe_UNKNOWN:
            num = 0;
            break;
        default:
            ALOGE(LOG_TAG"[%s]ERR(%5d):user:%s out of module range,%d\n" , __FUNCTION__, __LINE__, szCallerName, module);
            return NULL;
            break;
    }
    if(num >= NSCam::NSIoPipe::NSCamIOPipe::ENPipe_CAM_MAX){
        ALOGE(LOG_TAG"[%s]ERR(%5d):user:%s out of module range,%d\n" , __FUNCTION__, __LINE__, szCallerName, module);
        return NULL;
    }

    QueryObj[num].m_hwModule = module;
    std::strncpy((char*)QueryObj[num].m_Name,(char const*)szCallerName,sizeof(QueryObj[num].m_Name)-1);

    return (CAM_CAPIBILITY*)&QueryObj[num];

}

void CAM_CAPIBILITY::DestroyInstance(char const* szCallerName)
{
    if(szCallerName == NULL){
        ALOGE(LOG_TAG"[%s]ERR(%5d):no user name\n" , __FUNCTION__, __LINE__);
        return;
    }
}


MBOOL CAM_CAPIBILITY::GetCapibility(MUINT32 portId,E_CAM_Query e_Op,CAM_Queryrst &QueryRst)
{
    if(this->m_hwModule == NSCam::NSIoPipe::NSCamIOPipe::ENPipe_UNKNOWN){
        capibility capObj;
        return capObj.GetCapibility(portId, e_Op, QueryRst);
    }
    else{
        //share  this compile flag - SUPPORTED_SEN_NUM for ISP50
        #ifdef SUPPORTED_SEN_NUM
        capibility capObj(this->m_hwModule);
        return capObj.GetCapibility(portId, e_Op, QueryRst);
        #else
        capibility capObj;
        return capObj.GetCapibility(portId, e_Op, QueryRst);
        #endif
    }
}

MBOOL CAM_CAPIBILITY::GetCapibility(MUINT32 portId,NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd e_Op,
                                            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_InputInfo inputInfo,
                                            NSCam::NSIoPipe::NSCamIOPipe::NormalPipe_QueryInfo &QueryRst)
{
    MBOOL rst = MTRUE;
    tCAM_rst camrst;
    capibility* ptr = NULL;

    if(this->m_hwModule == NSCam::NSIoPipe::NSCamIOPipe::ENPipe_UNKNOWN){
        ptr = new capibility;
    }
    else{
        //share  this compile flag - SUPPORTED_SEN_NUM for ISP50
        #ifdef SUPPORTED_SEN_NUM
        ptr = new capibility(this->m_hwModule);
        #else
        ptr = new capibility;
        #endif
    }
    rst = ptr->GetCapibility(portId, e_Op, inputInfo, camrst, E_CAM_UNKNOWNN);

    if(rst == MTRUE){
        if (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_BS_RATIO) {
            QueryRst.bs_ratio = camrst.ratio;
        }
        if (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_QUERY_FMT) {
            QueryRst.query_fmt = camrst.Queue_fmt;
        }
        if (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_PIPELINE_BITDEPTH) {
            QueryRst.pipelinebitdepth = camrst.pipelinebitdepth;
        }

        if (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_MAX_SEN_NUM) {
            //share  this compile flag - SUPPORTED_SEN_NUM for ISP50
            #ifdef SUPPORTED_SEN_NUM
            QueryRst.sen_num = camrst.Sen_Num;
            QueryRst.function.Bits.SensorNum = camrst.Sen_Num;
            #endif
        }
        if (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_BURST_NUM) {
            QueryRst.burstNum = camrst.burstNum;
        }
        if (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_SUPPORT_PATTERN) {
            QueryRst.pattern = camrst.pattern;
        }
        if (e_Op & (NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_PIX|NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_X_PIX)) {
            QueryRst.x_pix = camrst.x_pix;
        }
        if (e_Op & (NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_X_BYTE|NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_X_BYTE)) {
            //share  this compile flag - SUPPORTED_SEN_NUM for ISP50
            #ifdef SUPPORTED_SEN_NUM
            QueryRst.xsize_byte = camrst.xsize_byte[0];
            #else
            QueryRst.xsize_byte = camrst.xsize_byte;
            #endif
        }
        if (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_CROP_START_X) {
            QueryRst.crop_x = camrst.crop_x;
        }
        if (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_PIX) {
            QueryRst.stride_pix = camrst.stride_pix;
        }
        if (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_STRIDE_BYTE) {
            //share  this compile flag - SUPPORTED_SEN_NUM for ISP50
            #ifdef SUPPORTED_SEN_NUM
            QueryRst.stride_byte = camrst.stride_byte[0];
            QueryRst.stride_B[0] = camrst.stride_byte[0];
            QueryRst.stride_B[1] = camrst.stride_byte[1];
            QueryRst.stride_B[2] = camrst.stride_byte[2];
            #else
            QueryRst.stride_byte = camrst.stride_byte;
            QueryRst.stride_B[0] = camrst.stride_byte;
            #endif

        }
        if (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_D_Twin) {
            QueryRst.D_TWIN = camrst.D_TWIN;
            QueryRst.function.Bits.D_TWIN = camrst.D_TWIN;
        }
        if (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_IQ_LEVEL) {
            #ifdef SUPPORTED_IQ_LV
            QueryRst.IQlv = camrst.IQlv;
            #else
            QueryRst.IQlv = MFALSE;
            #endif
        }
        if(e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_IQ_SW_VER) {
            #ifdef SUPPORTED_IQ_LV
            if( ptr->m_IQVer.GetInfo((MUINT32*)&QueryRst.IQ_SW_Ver) == MFALSE){
                ALOGE(LOG_TAG"[%s]ERR(%5d):user:%s query fail(module:%d)\n" , __FUNCTION__, __LINE__,this->m_Name,this->m_hwModule);
            }
            #else
            ALOGE(LOG_TAG"[%s]ERR(%5d):user:%s query fail(module:%d)\n" , __FUNCTION__, __LINE__,this->m_Name,this->m_hwModule);
            #endif
        }
        if (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_D_BayerEnc) {
            //share  this compile flag - SUPPORTED_SEN_NUM for ISP50
            #ifdef SUPPORTED_SEN_NUM
            QueryRst.function.Bits.D_BayerENC = camrst.D_UF;
            #endif
        }

        if (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_NEED_CFG_SENSOR_BY_MW) {
            #ifdef SUPPORTED_CFG_SENSOR_BY_MW
            QueryRst.function.Bits.needCfgSensorByMW = camrst.needCfgSensorByMW;
            #else
            QueryRst.function.Bits.needCfgSensorByMW = MFALSE;
            #endif
        }

        if (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_UNI_NUM) {
            QueryRst.uni_num = camrst.uni_num;
        }

        if (e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_DYNAMIC_PAK) {
            //share  this compile flag - SUPPORT_DYNAMIC_PAK for ISP50
            #ifdef SUPPORTED_DYNAMIC_PAK
            QueryRst.D_Pak = camrst.D_Pak;
            #else
            QueryRst.D_Pak = MFALSE;
            #endif
        }

        if ((e_Op & NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_FUNC)
            == NSCam::NSIoPipe::NSCamIOPipe::ENPipeQueryCmd_FUNC) {
            //if no simple raw c -> this cmd is not supported  -> all functions below will be set to 0.
            //flk/lmv/rss r not use this cmd at lagecy IC
            #ifdef SUPPORTED_SIMPLE_RAW_C
            QueryRst.function.Bits.isFLK = camrst.bSupportedModule.bFlk;
            QueryRst.function.Bits.isLMV = camrst.bSupportedModule.bLMV;
            QueryRst.function.Bits.isPDE = camrst.bSupportedModule.bPDE;
            QueryRst.function.Bits.isRSS = camrst.bSupportedModule.bRSS;
            QueryRst.function.Bits.isFull_dir_YUV = camrst.bSupportedModule.bFullYUV;
            QueryRst.function.Bits.isScaled_Y = camrst.bSupportedModule.bScaledY;
            QueryRst.function.Bits.isScaled_YUV = camrst.bSupportedModule.bScaledYUV;
            QueryRst.function.Bits.isG_Scaled_YUV = camrst.bSupportedModule.bGScaledYUV;
            QueryRst.function.Bits.isRAWI = camrst.bSupportedModule.bRAWI;
            #elif defined SUPPORTED_DYUV
            QueryRst.function.Bits.isFLK = 1;
            QueryRst.function.Bits.isLMV = 1;
            QueryRst.function.Bits.isPDE = 1;
            QueryRst.function.Bits.isRSS = 1;
            QueryRst.function.Bits.isFull_dir_YUV = 1;
            QueryRst.function.Bits.isScaled_Y = 1;
            QueryRst.function.Bits.isScaled_YUV = 1;
            QueryRst.function.Bits.isG_Scaled_YUV = 1;
            QueryRst.function.Bits.isRAWI = 1;
            #else
            QueryRst.function.Bits.isFLK = 1;
            QueryRst.function.Bits.isLMV = 1;
            QueryRst.function.Bits.isPDE = 1;
            QueryRst.function.Bits.isRSS = 1;
            QueryRst.function.Bits.isFull_dir_YUV = 0;
            QueryRst.function.Bits.isScaled_Y = 0;
            QueryRst.function.Bits.isScaled_YUV = 0;
            QueryRst.function.Bits.isG_Scaled_YUV = 0;
            QueryRst.function.Bits.isRAWI = 1;
            #endif
        }

    }
    else
        ALOGE(LOG_TAG"[%s]ERR(%5d):user:%s query fail(module:%d)\n" , __FUNCTION__, __LINE__,this->m_Name,this->m_hwModule);

    delete ptr;

    return rst;
}

