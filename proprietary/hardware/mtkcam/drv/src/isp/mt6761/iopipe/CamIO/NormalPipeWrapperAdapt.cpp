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

#define LOG_TAG "NormalPipeModule"

#include <mtkcam/def/common.h>
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#include <iopipe/CamIO/CamIoWrapperPublic.h>
#include <iopipe/CamIO/NormalPipeModuleBase.h>
#include <mtkcam/drv/iopipe/CamIO/Cam_QueryDef.h>
#include <imageio/ispio_utility.h> //todo: it's under isp_30, it will be in mt6739.
#include <Cam/ICam_capibility.h>

#include "NormalPipe_FrmB.h"
#include <drv/utils.h>
#include <mtkcam/utils/std/ULog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_DRV_NORMAL_PIPE);


using namespace android;
using namespace NSCam;
using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe::Wrapper;
using namespace NSImageioIsp3;
using namespace NSImageioIsp3::NSIspio;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)

namespace NSCam {
namespace NSIoPipe {
namespace NSCamIOPipe {
namespace Wrapper {


class DefaultDriverFactory : public IDriverFactory
{
public:
    virtual INormalPipeIsp3 *createNormalPipeIsp3(MUINT32 sensorIndex, char const* szCallerName, MINT32 burstQnum) override {
        MUINT32 sensorIndexArr[1] = { sensorIndex };
        return INormalPipeIsp3::createInstance(sensorIndexArr, 1, szCallerName, burstQnum);
    }

    virtual IspDrv *createIspDrvIsp3() override {
        return IspDrv::createInstance();
    }
};


class NormalPipeModule : public NormalPipeModuleBase
{
public:
    NormalPipeModule() : NormalPipeModuleBase() {
        get_module_extension = get_module_extension_impl;
    }

    virtual MERROR                  createSubModule(
                                        MUINT32 sensorIndex,
                                        char const* szCallerName,
                                        MUINT32 apiVersion,
                                        MVOID** ppInstance,
                                        MUINT32 pipeSelect = EPipeSelect_Normal
                                    );

    static NormalPipeModule* getSingleton() {
        return &sSingleton;
    }

    /*New query API for after ISP5.0*/
    virtual MBOOL                   query(
                                        MUINT32 eCmd,
                                        MUINTPTR pIO_struct
                                    )   const ;

private:
    static NormalPipeModule sSingleton;

    static void* get_module_extension_impl();
};


NormalPipeModule NormalPipeModule::sSingleton;


MERROR NormalPipeModule::createSubModule(
    MUINT32 sensorIndex,
    char const* szCallerName,
    MUINT32 apiVersion,
    MVOID** ppInstance,
    MUINT32 pipeSelect)
{
    UNUSED(pipeSelect);
    INormalPipe *instance = NormalPipeWrapper_createInstance(sensorIndex, szCallerName, apiVersion, new DefaultDriverFactory);
    *ppInstance = instance;
    return (instance != NULL) ? OK : BAD_INDEX;
}


void* NormalPipeModule::get_module_extension_impl()
{
    return static_cast<void*>(getSingleton());
}


}
}
}
}


extern "C"
mtkcam_module* get_mtkcam_module_iopipe_CamIO_NormalPipe()
{
    return static_cast<mtkcam_module*>(NormalPipeModule::getSingleton());
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipeModule::
query(
    MUINT32 eCmd,
    MUINTPTR pIO_struct
)   const
{
    MBOOL ret = MTRUE;
    MUINT32 portIdx = 0;
    NormalPipe_InputInfo  qryInput;
    NormalPipe_QueryInfo  queryInfo;

    switch(eCmd){
        case ENPipeQueryCmd_X_PIX:
            {
                sCAM_QUERY_X_PIX *pQueryStruct = (sCAM_QUERY_X_PIX*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }

                portIdx             = pQueryStruct->QueryInput.portId;
                qryInput.format     = pQueryStruct->QueryInput.format;
                qryInput.width      = pQueryStruct->QueryInput.width;
                qryInput.pixelMode  = pQueryStruct->QueryInput.pixelMode;

                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.x_pix;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_CROP_X_PIX:
            {
                sCAM_QUERY_CROP_X_PIX *pQueryStruct = (sCAM_QUERY_CROP_X_PIX*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }

                portIdx             = pQueryStruct->QueryInput.portId;
                qryInput.format     = pQueryStruct->QueryInput.format;
                qryInput.width      = pQueryStruct->QueryInput.width;
                qryInput.pixelMode  = pQueryStruct->QueryInput.pixelMode;

                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.x_pix;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_X_BYTE:
            {
                sCAM_QUERY_X_BYTE *pQueryStruct = (sCAM_QUERY_X_BYTE*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }

                portIdx             = pQueryStruct->QueryInput.portId;
                qryInput.format     = pQueryStruct->QueryInput.format;
                qryInput.width      = pQueryStruct->QueryInput.width;
                qryInput.pixelMode  = pQueryStruct->QueryInput.pixelMode;

                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.xsize_byte;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_CROP_X_BYTE:
            {
                sCAM_QUERY_CROP_X_BYTE *pQueryStruct = (sCAM_QUERY_CROP_X_BYTE*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }

                portIdx             = pQueryStruct->QueryInput.portId;
                qryInput.format     = pQueryStruct->QueryInput.format;
                qryInput.width      = pQueryStruct->QueryInput.width;
                qryInput.pixelMode  = pQueryStruct->QueryInput.pixelMode;

                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.xsize_byte;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_CROP_START_X:
            {
                sCAM_QUERY_CROP_START_X *pQueryStruct = (sCAM_QUERY_CROP_START_X*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }

                portIdx             = pQueryStruct->QueryInput.portId;
                qryInput.format     = pQueryStruct->QueryInput.format;
                qryInput.width      = pQueryStruct->QueryInput.width;
                qryInput.pixelMode  = pQueryStruct->QueryInput.pixelMode;

                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.crop_x;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_STRIDE_PIX:
            {
                sCAM_QUERY_STRIDE_PIX *pQueryStruct = (sCAM_QUERY_STRIDE_PIX*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }

                portIdx             = pQueryStruct->QueryInput.portId;
                qryInput.format     = pQueryStruct->QueryInput.format;
                qryInput.width      = pQueryStruct->QueryInput.width;
                qryInput.pixelMode  = pQueryStruct->QueryInput.pixelMode;

                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }
                pQueryStruct->QueryOutput = queryInfo.stride_pix;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_STRIDE_BYTE:
            {
                sCAM_QUERY_STRIDE_BYTE *pQueryStruct = (sCAM_QUERY_STRIDE_BYTE*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }

                portIdx             = pQueryStruct->QueryInput.portId;
                qryInput.format     = pQueryStruct->QueryInput.format;
                qryInput.width      = pQueryStruct->QueryInput.width;
                qryInput.pixelMode  = pQueryStruct->QueryInput.pixelMode;

                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.stride_byte;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_MAX_SEN_NUM:
            {
                sCAM_QUERY_MAX_SEN_NUM *pQueryStruct = (sCAM_QUERY_MAX_SEN_NUM*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }
                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                //pQueryStruct->QueryOutput = queryInfo.sen_num;
                pQueryStruct->QueryOutput = 0; /* ISP 3.0 is not support for forcing twin off*/
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_PIPELINE_BITDEPTH:
            {
                sCAM_QUERY_PIPELINE_BITDEPTH *pQueryStruct = (sCAM_QUERY_PIPELINE_BITDEPTH*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }
                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.pipelinebitdepth;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_QUERY_FMT:
            {
                sCAM_QUERY_QUERY_FMT *pQueryStruct = (sCAM_QUERY_QUERY_FMT*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }

                portIdx = pQueryStruct->QueryInput.portId;

                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.query_fmt;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
         case ENPipeQueryCmd_BURST_NUM:
            {
                sCAM_QUERY_BURST_NUM *pQueryStruct = (sCAM_QUERY_BURST_NUM*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }
                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.burstNum;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_SUPPORT_PATTERN:
            {
                sCAM_QUERY_SUPPORT_PATTERN *pQueryStruct = (sCAM_QUERY_SUPPORT_PATTERN*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }
                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.pattern;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_BS_RATIO:
            {
                sCAM_QUERY_BS_RATIO *pQueryStruct = (sCAM_QUERY_BS_RATIO*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }

                portIdx = pQueryStruct->QueryInput.portId;

                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.bs_ratio;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_D_Twin:
            {
                sCAM_QUERY_D_Twin *pQueryStruct = (sCAM_QUERY_D_Twin*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }
                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.D_TWIN;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_D_BayerEnc:
            {
                sCAM_QUERY_D_BAYERENC *pQueryStruct = (sCAM_QUERY_D_BAYERENC*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }
                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.function.Bits.D_BayerENC;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_IQ_LEVEL:
            {
                sCAM_QUERY_IQ_LEVEL *pQueryStruct = (sCAM_QUERY_IQ_LEVEL*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                if(pQueryStruct->Cmd != eCmd){
                    MY_LOGE("cmd is not match with struct(0x%x_0x%x)\n",pQueryStruct->Cmd, eCmd);
                    return MFALSE;
                }
                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.IQlv;
                pinfo->DestroyInstance(LOG_TAG);

            }
            break;
        case ENPipeQueryCmd_PDO_AVAILABLE:
            {
                sCAM_QUERY_PDO_AVAILABLE *pQueryStruct = (sCAM_QUERY_PDO_AVAILABLE*)pIO_struct;
                pQueryStruct->QueryOutput = MFALSE; //~isp3.0:pdo not supported, isp4.0~:pdo supported
            }
            break;
        case ENPipeQueryCmd_MAX_PREVIEW_SIZE:
            {
                sCAM_QUERY_MAX_PREVIEW_SIZE *pQueryStruct = (sCAM_QUERY_MAX_PREVIEW_SIZE*)pIO_struct;
                pQueryStruct->QueryOutput = 2304; //isp3.0:max rrzo width 2304
            }
            break;
        default:
            ret = MFALSE;
            MY_LOGE("unsupport cmd(0x%x)\n",eCmd);
            break;
    }

    return ret;
}


// Used by StatisticPipe.cpp & CamSvStatisticPipe.cpp of ISP 4
// Add this function to pass link error
extern "C"
INormalPipe* createInstance_INormalPipe(MUINT32 SensorIndex, char const* szCallerName)
{
    INormalPipe *instance = NormalPipeWrapper_createInstance(SensorIndex, szCallerName, MTKCAM_MAKE_API_VERSION(1, 0), new DefaultDriverFactory);
    return instance;
}


