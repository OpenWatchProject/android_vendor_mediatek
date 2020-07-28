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

#include <mtkcam3/feature/stereo/hal/stereo_size_provider.h>
#include "ImgParamSetting.h"

typedef IImageBufferAllocator::ImgParam ImgParam;

StereoSizeProvider* gpSizePrvder = StereoSizeProvider::getInstance();
MUINT32 FULL_RAW_BOUNDARY[3] = {0, 0, 0};

ImgParam getImgParam_FD()
{
    Pass2SizeInfo pass2SizeInfo;
    gpSizePrvder->getPass2SizeInfo(PASS2A, eSTEREO_SCENARIO_RECORD, pass2SizeInfo);
    MSize P2A_FD_IMG_SIZE = pass2SizeInfo.areaIMG2O;
    MUINT32 FD_STRIDES[3] = {(MUINT32) P2A_FD_IMG_SIZE.w*2, 0, 0};
    return ImgParam(eImgFmt_YUY2, P2A_FD_IMG_SIZE, FD_STRIDES, FULL_RAW_BOUNDARY, 1);
}

ImgParam getImgParam_MV_F()
{
    Pass2SizeInfo pass2SizeInfo;
    gpSizePrvder->getPass2SizeInfo(PASS2A, eSTEREO_SCENARIO_RECORD, pass2SizeInfo);
    MSize VSDOF_MV_F_SIZE = pass2SizeInfo.areaWDMA.size;

    MUINT32 MAINIMG_STRIDES[3] = {(MUINT32)VSDOF_MV_F_SIZE.w, (MUINT32)VSDOF_MV_F_SIZE.w/2, (MUINT32)VSDOF_MV_F_SIZE.w/2};

    return ImgParam(eImgFmt_YV12, VSDOF_MV_F_SIZE, MAINIMG_STRIDES, FULL_RAW_BOUNDARY, 3);
}

ImgParam getImgParam_MV_F_CAP()
{
    Pass2SizeInfo pass2SizeInfo;
    gpSizePrvder->getPass2SizeInfo(PASS2A, eSTEREO_SCENARIO_CAPTURE, pass2SizeInfo);
    MSize VSDOF_MV_F_CAP_SIZE = pass2SizeInfo.areaWDMA.size;

    MUINT32 MAINIMG_STRIDES[3] = {(MUINT32)VSDOF_MV_F_CAP_SIZE.w, (MUINT32)VSDOF_MV_F_CAP_SIZE.w/2, (MUINT32)VSDOF_MV_F_CAP_SIZE.w/2};

    return ImgParam(eImgFmt_YV12, VSDOF_MV_F_CAP_SIZE, MAINIMG_STRIDES, FULL_RAW_BOUNDARY, 3);
}

ImgParam getImgParam_WARPING_MATRIX()
{
    MUINT32 iWarpingMatrixSize = StereoSettingProvider::getMaxWarpingMatrixBufferSizeInBytes();

    MUINT32 MAINIMG_STRIDES[3] = {iWarpingMatrixSize, 0, 0};

    return ImgParam(eImgFmt_Y8, MSize(iWarpingMatrixSize, 1), MAINIMG_STRIDES, FULL_RAW_BOUNDARY, 1);
}

ImgParam getImgParam_DMP()
{
    MSize szDMP_SIZE = gpSizePrvder->getBufferSize(E_DMP_H, eSTEREO_SCENARIO_CAPTURE);
    // Y16 : stride 1 = w*2
    MUINT32 MAINIMG_STRIDES[3] = {(MUINT32)szDMP_SIZE.w*2, 0, 0};

    return ImgParam(eImgFmt_Y16, szDMP_SIZE, MAINIMG_STRIDES, FULL_RAW_BOUNDARY, 1);
}

ImgParam getImgParam_DMW()
{
    MSize DMW_SIZE = gpSizePrvder->getBufferSize(E_DMW, eSTEREO_SCENARIO_RECORD);
    MUINT32 DMW_STRIDES[3] = {(MUINT32)DMW_SIZE.w, 0, 0};

    return ImgParam(eImgFmt_Y8, DMW_SIZE, DMW_STRIDES, FULL_RAW_BOUNDARY, 1);
}

ImgParam getImgParam_MY_S(ENUM_STEREO_SCENARIO scen)
{
    MSize MY_S_SIZE = gpSizePrvder->getBufferSize(E_MY_S, scen);
    MUINT32 MY_S_STRIDES[3] = {(MUINT32)MY_S_SIZE.w, (MUINT32)MY_S_SIZE.w/2 , (MUINT32)MY_S_SIZE.w/2};
    return ImgParam(eImgFmt_YV12, MY_S_SIZE, MY_S_STRIDES, FULL_RAW_BOUNDARY, 3);
}

ImgParam getImgParam_DMBG()
{
    MSize DMBG_SIZE = gpSizePrvder->getBufferSize(E_DMBG, eSTEREO_SCENARIO_RECORD);
    MUINT32 DMBG_STRIDES[3] = {(MUINT32)DMBG_SIZE.w, 0, 0};
    return ImgParam(eImgFmt_Y8, DMBG_SIZE, DMBG_STRIDES, FULL_RAW_BOUNDARY, 3);
}

ImgParam getImgParam_DEPTHMAP()
{
    MSize DEPTHMAP_SIZE = gpSizePrvder->getBufferSize(E_DEPTH_MAP, eSTEREO_SCENARIO_CAPTURE);
    MUINT32 DEPTHMAP_STRIDES[3] = {(MUINT32)DEPTHMAP_SIZE.w, 0, 0};
    return ImgParam(eImgFmt_Y8, DEPTHMAP_SIZE, DEPTHMAP_STRIDES, FULL_RAW_BOUNDARY, 3);
}

ImgParam getImgParam_JPSMain()
{
    Pass2SizeInfo pass2SizeInfo;
    gpSizePrvder->getPass2SizeInfo(PASS2A_P_2, eSTEREO_SCENARIO_CAPTURE, pass2SizeInfo);
    MSize JPS_SIZE = pass2SizeInfo.areaWDMA.size;
    MUINT32 JPS_STRIDES[3] = {(MUINT32)JPS_SIZE.w, (MUINT32)JPS_SIZE.w/2, (MUINT32)JPS_SIZE.w/2};
    return ImgParam(eImgFmt_YV12, JPS_SIZE, JPS_STRIDES, FULL_RAW_BOUNDARY, 3);
}

ImgParam getImgParam_LDC()
{
    MSize LDC_SIZE = gpSizePrvder->getBufferSize(E_LDC, eSTEREO_SCENARIO_CAPTURE);
    MUINT32 LDC_STRIDES[3] = {(MUINT32)LDC_SIZE.w, 0, 0};
    return ImgParam(eImgFmt_Y8, LDC_SIZE, LDC_STRIDES, FULL_RAW_BOUNDARY, 3);
}

ImgParam getImgParam_OCC(ENUM_STEREO_SCENARIO sce)
{
    MSize OCC_SIZE = gpSizePrvder->getBufferSize(E_OCC, sce);
    MUINT32 OCC_STRIDES[3] = {(MUINT32)OCC_SIZE.w, 0, 0};
    return ImgParam(eImgFmt_Y8, OCC_SIZE, OCC_STRIDES, FULL_RAW_BOUNDARY, 3);
}

ImgParam getImgParam_NOC(ENUM_STEREO_SCENARIO sce)
{
    MSize NOC_SIZE = gpSizePrvder->getBufferSize(E_NOC, sce);
    MUINT32 NOC_STRIDES[3] = {(MUINT32)NOC_SIZE.w, 0, 0};
    return ImgParam(eImgFmt_Y8, NOC_SIZE, NOC_STRIDES, FULL_RAW_BOUNDARY, 3);
}

ImgParam getImgParam_DMW(ENUM_STEREO_SCENARIO sce)
{
    MSize DMW_SIZE = gpSizePrvder->getBufferSize(E_DMW, sce);
    MUINT32 DMW_STRIDES[3] = {(MUINT32)DMW_SIZE.w, 0, 0};
    return ImgParam(eImgFmt_Y8, DMW_SIZE, DMW_STRIDES, FULL_RAW_BOUNDARY, 3);
}

IImageBufferAllocator::ImgParam getImgParam_DEPTH_DBG(ENUM_STEREO_SCENARIO sce)
{
    MSize DEPTHDBG_SIZE = gpSizePrvder->getBufferSize(E_DMW, sce) * 4;
    MUINT32 DEPTHDBG_STRIDES[3] = {(MUINT32)DEPTHDBG_SIZE.w, 0, 0};
    return ImgParam(eImgFmt_Y8, DEPTHDBG_SIZE, DEPTHDBG_STRIDES, FULL_RAW_BOUNDARY, 3);
}