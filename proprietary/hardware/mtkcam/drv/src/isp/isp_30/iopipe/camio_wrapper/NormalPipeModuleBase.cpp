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

#define LOG_TAG "NormalPipeModuleBase"

#include <mtkcam/def/common.h>
#include <mtkcam/drv/iopipe/CamIO/INormalPipe.h>
#include <imageio/ispio_utility.h> //todo: it's under isp_30, it will be in mt6739.
#include <iopipe/CamIO/NormalPipeModuleBase.h>
#include "NormalPipeWrapper.h"
#include <Cam/ICam_capibility.h>
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
//
/******************************************************************************
 *
 ******************************************************************************/

uint32_t NormalPipeModuleBase::get_module_api_version_impl()
{
    return MTKCAM_MAKE_API_VERSION(1, 0);
}


int NormalPipeModuleBase::get_sub_module_api_version_impl(
    uint32_t const** versions,
    size_t* count,
    int // index
)
{
    static const uint32_t API_VERSIONS[] = {
        MTKCAM_MAKE_API_VERSION(1, 0)
    };

    *versions = API_VERSIONS;
    *count = sizeof(API_VERSIONS) / sizeof(API_VERSIONS[0]);

    return 0; // success
}


NormalPipeModuleBase::NormalPipeModuleBase()
{
    get_module_api_version      = get_module_api_version_impl;
    get_module_extension        = NULL;
    get_sub_module_api_version  = get_sub_module_api_version_impl;
}


MBOOL NormalPipeModuleBase::query(
    MUINT32 portIdx,
    MUINT32 eCmd,
    MINT imgFmt,
    NormalPipe_QueryIn const& input,
    NormalPipe_QueryInfo &queryInfo
) const
{

    ISP_QUERY_RST queryRst;
    E_ISP_PIXMODE _defPixMode = ISP_QUERY_2_PIX_MODE;
    MUINT32 qCmd = 0;
    MBOOL ret = MTRUE;
    NormalPipe_InputInfo  qryInput;

    switch(input.pixMode){
        case _1_PIX_MODE:
            _defPixMode = ISP_QUERY_1_PIX_MODE;
            break;
        case _2_PIX_MODE:
        default:
            _defPixMode = ISP_QUERY_2_PIX_MODE;
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


    if(eCmd){
        CAM_CAPIBILITY* pinfo = CAM_CAPIBILITY::CreateInsatnce(LOG_TAG);

        qryInput.format     = (EImageFormat)imgFmt;
        qryInput.width      = input.width;
        qryInput.pixelMode  = input.pixMode;

        if(pinfo->GetCapibility(portIdx, (ENPipeQueryCmd)eCmd, qryInput, queryInfo) == MFALSE){
            MY_LOGE("some query op fail\n");
            ret = MFALSE;
        }
        pinfo->DestroyInstance(LOG_TAG);
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

    return ret;
}

