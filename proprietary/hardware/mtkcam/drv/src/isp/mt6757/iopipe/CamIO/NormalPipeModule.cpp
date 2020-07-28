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
                                        MUINTPTR IO_struct
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
    ISP_QUERY_RST   queryRst;
    E_ISP_PIXMODE   _defPixMode = ISP_QUERY_4_PIX_MODE;
    MUINT32         qCmd = 0;
    CAM_Queryrst    rst;

    switch(input.pixMode){
        case _1_PIX_MODE:
            _defPixMode = ISP_QUERY_1_PIX_MODE;
            break;
        case _2_PIX_MODE:
            _defPixMode = ISP_QUERY_2_PIX_MODE;
            break;
        case _4_PIX_MODE:
        default:
            _defPixMode = ISP_QUERY_4_PIX_MODE;
            break;
    }

    if (eCmd & ENPipeQueryCmd_X_PIX) {
        qCmd |= ISP_QUERY_X_PIX;
    }
    if (eCmd & ENPipeQueryCmd_X_BYTE) {
        qCmd |= ISP_QUERY_XSIZE_BYTE;
    }
    if (eCmd & ENPipeQueryCmd_CROP_X_PIX) {
        qCmd |= ISP_QUERY_CROP_X_PIX;
    }
    if (eCmd & ENPipeQueryCmd_CROP_X_BYTE) {
        qCmd |= ISP_QUERY_CROP_X_BYTE;
    }
    if (eCmd & ENPipeQueryCmd_CROP_START_X) {
        qCmd |= ISP_QUERY_CROP_START_X;
    }
    if (eCmd & ENPipeQueryCmd_STRIDE_PIX) {
        qCmd |= ISP_QUERY_STRIDE_PIX;
    }
    if (eCmd & ENPipeQueryCmd_STRIDE_BYTE) {
        qCmd |= ISP_QUERY_STRIDE_BYTE;
    }

    if (eCmd & ENPipeQueryCmd_CROP_START_X) {
        if (ISP_QuerySize(_BY_PASSS_PORT, qCmd, (EImageFormat)imgFmt, input.width, queryRst, _defPixMode) == 0) {
            return MFALSE;
        }
    }
    else {
        if (ISP_QuerySize(portIdx, qCmd, (EImageFormat)imgFmt, input.width, queryRst, _defPixMode) == 0) {
            return MFALSE;
        }
    }

    qCmd = 0;
    qCmd |= (eCmd & ENPipeQueryCmd_QUERY_FMT)   ? (ECamio_valid_fmt):(0);
    qCmd |= (eCmd & ENPipeQueryCmd_BS_RATIO)    ? (ECamio_BS_ratio):(0);
    qCmd |= (eCmd & ENPipeQueryCmd_PIPELINE_BITDEPTH) ? (ECamio_pipeline_bitdepth):(0);
    qCmd |= (eCmd & ENPipeQueryCmd_D_Twin) ? (ECamio_pipeline_d_twin):(0);
    qCmd |= (eCmd & ENPipeQueryCmd_UNI_NUM) ? (ECamio_pipeline_uni_num):(0);

    if(qCmd){
        CAM_CAPIBILITY* ptr = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);
        if(ptr->GetCapibility(portIdx,(E_CAM_Query)qCmd,rst) == MFALSE){
            MY_LOGE("some query op fail\n");
        }
        ptr->DestroyInstance(LOG_TAG);
    }

    if (eCmd & ENPipeQueryCmd_X_PIX) {
        queryInfo.x_pix = queryRst.x_pix;
    }
    if (eCmd & ENPipeQueryCmd_X_BYTE) {
        queryInfo.xsize_byte = queryRst.xsize_byte;
    }
    if (eCmd & ENPipeQueryCmd_CROP_X_PIX) {
        queryInfo.x_pix = queryRst.x_pix;
    }
    if (eCmd & ENPipeQueryCmd_CROP_X_BYTE) {
        queryInfo.xsize_byte = queryRst.xsize_byte;
    }
    if (eCmd & ENPipeQueryCmd_CROP_START_X) {
        queryInfo.crop_x = queryRst.crop_x;
    }
    if (eCmd & ENPipeQueryCmd_STRIDE_PIX) {
        queryInfo.stride_pix = queryRst.stride_pix;
    }
    if (eCmd & ENPipeQueryCmd_STRIDE_BYTE) {
        queryInfo.stride_byte = queryRst.stride_byte;
    }

    queryInfo.query_fmt = (eCmd & ENPipeQueryCmd_QUERY_FMT)?(rst.Queue_fmt):(queryInfo.query_fmt);
    queryInfo.bs_ratio = (eCmd & ENPipeQueryCmd_BS_RATIO)?(rst.ratio):(queryInfo.bs_ratio);
    queryInfo.pipelinebitdepth = (eCmd & ENPipeQueryCmd_PIPELINE_BITDEPTH)?((E_CAM_PipelineBitDepth_SEL)rst.pipelinebitdepth):(queryInfo.pipelinebitdepth);

    return MTRUE;
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

    switch(eCmd){
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

    return ret;
}

