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

#define LOG_TAG "NormalPipe"
//
#include <vector>
#include <algorithm>
//
#include <mtkcam/utils/std/Log.h>
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#include <mtkcam/drv/iopipe/CamIO/Cam_QueryDef.h>
//
#include <Cam/ICam_capibility.h>
//
#include <ispio_utility.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NSImageio;
using namespace NSIspio;


/******************************************************************************
 *  Global Function Declaration.
 ******************************************************************************/
extern "C" INormalPipe* createInstance_INormalPipe(MUINT32 SensorIndex, char const* szCallerName);


/******************************************************************************
 *  Module Version
 ******************************************************************************/
/**
 * The supported module API version.
 */
#define MY_MODULE_API_VERSION       MTKCAM_MAKE_API_VERSION(1, 0)

/**
 * The supported sub-module API versions in ascending order.
 * Assume: all cameras support the same sub-module api versions.
 */
#define MY_SUB_MODULE_API_VERSION   {MTKCAM_MAKE_API_VERSION(1, 0), }
static const std::vector<MUINT32>   gSubModuleApiVersion = []() {
    static std::vector<MUINT32> v(MY_SUB_MODULE_API_VERSION);
    std::sort(v.begin(), v.end());
    return v;
} ();


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
/******************************************************************************
 *
 ******************************************************************************/
namespace {
class NormalPipeModule : public INormalPipeModule
{
public:     ////                    Operations.
                                    NormalPipeModule();

public:     ////                    Operations.

    virtual MERROR                  createSubModule(
                                        MUINT32 sensorIndex,
                                        char const* szCallerName,
                                        MUINT32 apiVersion,
                                        MVOID** ppInstance,
                                        MUINT32 pipeSelect = EPipeSelect_Normal
                                    );

    virtual MBOOL                   query(
                                        MUINT32 portIdx,
                                        MUINT32 eCmd,
                                        MINT imgFmt,
                                        NormalPipe_QueryIn const& input,
                                        NormalPipe_QueryInfo &queryInfo
                                    )   const;
    virtual MBOOL                   query(
                                        MUINT32 eCmd,
                                        MUINTPTR pIO_struct
                                    )   const override;
};
};


/******************************************************************************
 *
 ******************************************************************************/
NormalPipeModule*
getNormalPipeModule()
{
    static NormalPipeModule singleton;
    return &singleton;
}


/******************************************************************************
 *
 ******************************************************************************/
extern "C"
mtkcam_module* get_mtkcam_module_iopipe_CamIO_NormalPipe()
{
    return static_cast<mtkcam_module*>(getNormalPipeModule());
}


/******************************************************************************
 *
 ******************************************************************************/
NormalPipeModule::
NormalPipeModule()
    : INormalPipeModule()
{
    //// + mtkcam_module initialization

    get_module_api_version      = []()->uint32_t{ return MY_MODULE_API_VERSION; };

    get_module_extension        = []()->void*   { return getNormalPipeModule(); };

    get_sub_module_api_version  = [](uint32_t const** versions, size_t* count, int index)->int
    {
        if  ( ! versions || ! count ) {
            MY_LOGE("[%#x] invalid arguments - versions:%p count:%p", index, versions, count);
            return -EINVAL;
        }

        //Assume: all cameras support the same sub-module api versions.
        *versions = gSubModuleApiVersion.data();
        *count = gSubModuleApiVersion.size();
        return 0;
    };

    //// - mtkcam_module initialization

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
NormalPipeModule::
createSubModule(
    MUINT32 sensorIndex,
    char const* szCallerName,
    MUINT32 apiVersion,
    MVOID** ppInstance,
    MUINT32 pipeSelect
)
{
    MERROR status = OK;

    if  ( ! ppInstance ) {
        MY_LOGE("[%s:%d] ppInstance==0", szCallerName, sensorIndex);
        return BAD_VALUE;
    }

    if  ( ! std::binary_search(gSubModuleApiVersion.begin(), gSubModuleApiVersion.end(), apiVersion) ) {
        MY_LOGE("[%s:%d] Unsupported sub-module api version:%#x", szCallerName, sensorIndex, apiVersion);
        return BAD_VALUE;
    }

    switch  (MTKCAM_GET_MAJOR_API_VERSION(apiVersion))
    {
        //Version = (1, 0)-(1, 0xFF)
        case 1:{
            *ppInstance = static_cast<INormalPipe*>(createInstance_INormalPipe(sensorIndex, szCallerName));
            break;
        }
        //
        default:{
            MY_LOGE(
                "[%s:%d] Not implement for sub-module api version:%#x",
                szCallerName, sensorIndex, apiVersion
            );
            status = INVALID_OPERATION;
            break;
        }
    }

    return status;
}

inline MUINT32 getNPipeVersion(MUINT32 sensorIdx)
{
    MUINT32 selectedVersion = 0;
    auto pModule = getNormalPipeModule();

    if  ( ! pModule ) {
        MY_LOGE("getNormalPipeModule() fail");
        return selectedVersion;
    }

    MUINT32 const* version = NULL;
    size_t count = 0;
    int err = pModule->get_sub_module_api_version(&version, &count, sensorIdx);
    if  ( err < 0 || ! count || ! version ) {
        MY_LOGE(
            "[%d] INormalPipeModule::get_sub_module_api_version - err:%#x count:%zu version:%p",
            sensorIdx, err, count, version
        );
        return selectedVersion;
    }

    selectedVersion = *(version + count - 1); //Select max. version
    MY_LOGI("[%d] count:%zu Selected CamIO Version:%0#x", sensorIdx, count, selectedVersion);
    return selectedVersion;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
NormalPipeModule::
query(
    MUINT32 portIdx,
    MUINT32 eCmd,
    MINT imgFmt,
    NormalPipe_QueryIn const& input,
    NormalPipe_QueryInfo &queryInfo
)   const
{
    MBOOL ret = MTRUE;

    if(eCmd){
        NormalPipe_InputInfo  qryInput;
        CAM_CAPIBILITY*       pinfo = NULL;

        pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);
        qryInput.format     = (EImageFormat)imgFmt;
        qryInput.width      = input.width;
        qryInput.pixelMode  = input.pixMode;

        if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
            MY_LOGE("some query op fail\n");
            ret = MFALSE;
        }
        pinfo->DestroyInstance(LOG_TAG);
    }

    return ret;
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

#define CheckCapCmd(lhs, rhs) \
    if (lhs != rhs) { \
        MY_LOGE("cmd mismatch(0x%x_0x%x)\n",lhs, rhs); \
        return MFALSE; \
    }

    switch(eCmd){
        case ENPipeQueryCmd_X_PIX:
            {
                sCAM_QUERY_X_PIX *pQueryStruct = (sCAM_QUERY_X_PIX*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                CheckCapCmd(pQueryStruct->Cmd, eCmd);

                portIdx             = pQueryStruct->QueryInput.portId;
                qryInput.format     = pQueryStruct->QueryInput.format;
                qryInput.width      = pQueryStruct->QueryInput.width;
                qryInput.pixelMode  = pQueryStruct->QueryInput.pixelMode;

                if((ret = pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo)) == MFALSE){
                    MY_LOGE("some query op fail\n");
                }

                pQueryStruct->QueryOutput = queryInfo.x_pix;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_CROP_X_PIX:
            {
                sCAM_QUERY_CROP_X_PIX *pQueryStruct = (sCAM_QUERY_CROP_X_PIX*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                CheckCapCmd(pQueryStruct->Cmd, eCmd);

                portIdx             = pQueryStruct->QueryInput.portId;
                qryInput.format     = pQueryStruct->QueryInput.format;
                qryInput.width      = pQueryStruct->QueryInput.width;
                qryInput.pixelMode  = pQueryStruct->QueryInput.pixelMode;

                if((ret = pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo)) == MFALSE){
                    MY_LOGE("some query op fail\n");
                }

                pQueryStruct->QueryOutput = queryInfo.x_pix;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_X_BYTE:
            {
                sCAM_QUERY_X_BYTE *pQueryStruct = (sCAM_QUERY_X_BYTE*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                CheckCapCmd(pQueryStruct->Cmd, eCmd);

                portIdx             = pQueryStruct->QueryInput.portId;
                qryInput.format     = pQueryStruct->QueryInput.format;
                qryInput.width      = pQueryStruct->QueryInput.width;
                qryInput.pixelMode  = pQueryStruct->QueryInput.pixelMode;

                if((ret = pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo)) == MFALSE){
                    MY_LOGE("some query op fail\n");
                }

                pQueryStruct->QueryOutput = queryInfo.xsize_byte;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_CROP_X_BYTE:
            {
                sCAM_QUERY_CROP_X_BYTE *pQueryStruct = (sCAM_QUERY_CROP_X_BYTE*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                CheckCapCmd(pQueryStruct->Cmd, eCmd);

                portIdx             = pQueryStruct->QueryInput.portId;
                qryInput.format     = pQueryStruct->QueryInput.format;
                qryInput.width      = pQueryStruct->QueryInput.width;
                qryInput.pixelMode  = pQueryStruct->QueryInput.pixelMode;

                if((ret = pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo)) == MFALSE){
                    MY_LOGE("some query op fail\n");
                }

                pQueryStruct->QueryOutput = queryInfo.xsize_byte;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_CROP_START_X:
            {
                sCAM_QUERY_CROP_START_X *pQueryStruct = (sCAM_QUERY_CROP_START_X*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                CheckCapCmd(pQueryStruct->Cmd, eCmd);

                portIdx             = pQueryStruct->QueryInput.portId;
                qryInput.format     = pQueryStruct->QueryInput.format;
                qryInput.width      = pQueryStruct->QueryInput.width;
                qryInput.pixelMode  = pQueryStruct->QueryInput.pixelMode;

                if((ret = pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo)) == MFALSE){
                    MY_LOGE("some query op fail\n");
                }

                pQueryStruct->QueryOutput = queryInfo.crop_x;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_STRIDE_PIX:
            {
                sCAM_QUERY_STRIDE_PIX *pQueryStruct = (sCAM_QUERY_STRIDE_PIX*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                CheckCapCmd(pQueryStruct->Cmd, eCmd);

                portIdx             = pQueryStruct->QueryInput.portId;
                qryInput.format     = pQueryStruct->QueryInput.format;
                qryInput.width      = pQueryStruct->QueryInput.width;
                qryInput.pixelMode  = pQueryStruct->QueryInput.pixelMode;

                if((ret = pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo)) == MFALSE){
                    MY_LOGE("some query op fail\n");
                }
                pQueryStruct->QueryOutput = queryInfo.stride_pix;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_STRIDE_BYTE:
            {
                sCAM_QUERY_STRIDE_BYTE *pQueryStruct = (sCAM_QUERY_STRIDE_BYTE*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);
                CheckCapCmd(pQueryStruct->Cmd, eCmd);

                portIdx             = pQueryStruct->QueryInput.portId;
                qryInput.format     = pQueryStruct->QueryInput.format;
                qryInput.width      = pQueryStruct->QueryInput.width;
                qryInput.pixelMode  = pQueryStruct->QueryInput.pixelMode;

                if((ret = pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo)) == MFALSE){
                    MY_LOGE("some query op fail\n");
                }

                pQueryStruct->QueryOutput = queryInfo.stride_byte;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_MAX_SEN_NUM:
            {
                sCAM_QUERY_MAX_SEN_NUM *pQueryStruct = (sCAM_QUERY_MAX_SEN_NUM*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                CheckCapCmd(pQueryStruct->Cmd, eCmd);

                if((ret = pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo)) == MFALSE){
                    MY_LOGE("some query op fail\n");
                }

                pQueryStruct->QueryOutput = queryInfo.sen_num;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_PIPELINE_BITDEPTH:
            {
                sCAM_QUERY_PIPELINE_BITDEPTH *pQueryStruct = (sCAM_QUERY_PIPELINE_BITDEPTH*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                CheckCapCmd(pQueryStruct->Cmd, eCmd);

               if((ret = pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo)) == MFALSE){
                    MY_LOGE("some query op fail\n");
                }

                pQueryStruct->QueryOutput = queryInfo.pipelinebitdepth;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_QUERY_FMT:
            {
                sCAM_QUERY_QUERY_FMT *pQueryStruct = (sCAM_QUERY_QUERY_FMT*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                CheckCapCmd(pQueryStruct->Cmd, eCmd);

                portIdx = pQueryStruct->QueryInput.portId;

                if((ret = pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo)) == MFALSE){
                    MY_LOGE("some query op fail\n");
                }

                pQueryStruct->QueryOutput = queryInfo.query_fmt;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
         case ENPipeQueryCmd_BURST_NUM:
            {
                sCAM_QUERY_BURST_NUM *pQueryStruct = (sCAM_QUERY_BURST_NUM*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                CheckCapCmd(pQueryStruct->Cmd, eCmd);
                if((ret = pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo)) == MFALSE){
                    MY_LOGE("some query op fail\n");
                }

                pQueryStruct->QueryOutput = queryInfo.burstNum;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_SUPPORT_PATTERN:
            {
                sCAM_QUERY_SUPPORT_PATTERN *pQueryStruct = (sCAM_QUERY_SUPPORT_PATTERN*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                CheckCapCmd(pQueryStruct->Cmd, eCmd);
                if((ret = pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo)) == MFALSE){
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

                CheckCapCmd(pQueryStruct->Cmd, eCmd);

                portIdx = pQueryStruct->QueryInput.portId;

                if((ret = pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo)) == MFALSE){
                    MY_LOGE("some query op fail\n");
                }

                pQueryStruct->QueryOutput = queryInfo.bs_ratio;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_D_Twin:
            {
                sCAM_QUERY_D_Twin *pQueryStruct = (sCAM_QUERY_D_Twin*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                CheckCapCmd(pQueryStruct->Cmd, eCmd);

                if((ret = pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo)) == MFALSE){
                    MY_LOGE("some query op fail\n");
                }

                pQueryStruct->QueryOutput = queryInfo.D_TWIN;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_D_BayerEnc:
            {
                sCAM_QUERY_D_BAYERENC *pQueryStruct = (sCAM_QUERY_D_BAYERENC*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                CheckCapCmd(pQueryStruct->Cmd, eCmd);

                if((ret = pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo)) == MFALSE){
                    MY_LOGE("some query op fail\n");
                }

                pQueryStruct->QueryOutput = queryInfo.function.Bits.D_BayerENC;
                pinfo->DestroyInstance(LOG_TAG);
            }
            break;
        case ENPipeQueryCmd_IQ_LEVEL:
            #ifdef SUPPORTED_IQ_LV
            {
                sCAM_QUERY_IQ_LEVEL *pQueryStruct = (sCAM_QUERY_IQ_LEVEL*)pIO_struct;
                CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

                CheckCapCmd(pQueryStruct->Cmd, eCmd);

                if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
                    MY_LOGE("some query op fail\n");
                    ret = MFALSE;
                }

                pQueryStruct->QueryOutput = queryInfo.IQlv;
                pinfo->DestroyInstance(LOG_TAG);

                if (queryInfo.IQlv) {
                    IQ_CAPABILITY* pIQinfo = IQ_CAPABILITY::CreateInsatnce(LOG_TAG);
                    if(pIQinfo->queryIQTable(pQueryStruct->QueryInput.vInData, pQueryStruct->QueryInput.vOutData) == MFALSE){
                        MY_LOGE("queryIQTable fail\n");
                        return MFALSE;
                    }
                    pIQinfo->DestroyInstance(LOG_TAG);
                }
            }
            #endif
            break;
        case ENPipeQueryCmd_ISP_RES:
            {
                #define szCallerName "NpipeIsExe"
                INormalPipe* pNPipe = NULL;
                MBOOL bIsExe = MFALSE;
                sCAM_QUERY_ISP_RES *pQueryStruct = (sCAM_QUERY_ISP_RES*)pIO_struct;

                CheckCapCmd(pQueryStruct->Cmd, eCmd);

                getNormalPipeModule()->createSubModule(pQueryStruct->QueryInput.sensorIdx,
                    szCallerName, getNPipeVersion(pQueryStruct->QueryInput.sensorIdx), (MVOID**)&pNPipe);

                if(pNPipe == NULL){
                    MY_LOGE("createSubModule fail\n");
                    return MFALSE;
                }

                qryInput.sensorIdx  = pQueryStruct->QueryInput.sensorIdx;
                qryInput.scenarioId = pQueryStruct->QueryInput.scenarioId;
                qryInput.rrz_out_w  = pQueryStruct->QueryInput.rrz_out_w;
                qryInput.pattern    = pQueryStruct->QueryInput.pattern;
                qryInput.bin_off    = pQueryStruct->QueryInput.bin_off;

                if((ret = pNPipe->get_isExecutable(qryInput, bIsExe)) == MFALSE) {
                    MY_LOGE("get_isExecutable fail\n");
                }
                else{
                    pQueryStruct->QueryOutput = bIsExe;
                }
                pNPipe->destroyInstance(szCallerName);
            }
            break;
        case ENPipeQueryCmd_PDO_AVAILABLE:
            {
                sCAM_QUERY_PDO_AVAILABLE *pQueryStruct = (sCAM_QUERY_PDO_AVAILABLE*)pIO_struct;
                pQueryStruct->QueryOutput = MTRUE; //~isp3.0:pdo not supported, isp4.0~:pdo supported
            }
            break;
        default:
            ret = MFALSE;
            MY_LOGE("unsupport cmd(0x%x)\n",eCmd);
            break;
    }
#undef CheckCapCmd
    return ret;
}

