/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

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
#define LOG_TAG "HalIspAttributes"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#define HALISP_LOG_SET_P1       (1<<0)
#define HALISP_LOG_SET_P2       (2<<0)
#define HALISP_LOG_SETPARAM_P2  (3<<0)
#define HALISP_LOG_GET_P1       (4<<0)
#define HALISP_LOG_GET_P2       (5<<0)
#define HALISP_LOG_CONVERT_P2   (6<<0)

#include "HalIspImp.h"

#include <faces.h>

using namespace NSIspTuning;

#define ABS(a)    ((a) > 0 ? (a) : -(a))

static std::map<MINT32,NSCamHW::HwMatrix>       mMapMat;

MVOID
HalIspImp::
notifyRPGEnable(RAWIspCamInfo &rP1CamInfo, MBOOL const fgEnable)
{
    CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1, "[+notifyRPGEnable](%d, %d)", rP1CamInfo.fgRPGEnable, fgEnable);

    rP1CamInfo.fgRPGEnable = fgEnable;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
HalIspImp::
setIspProfile(RAWIspCamInfo &rP1CamInfo, EIspProfile_T const eIspProfile)
{
    CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1, "[+setIspProfile](old, new)=(%d, %d)", rP1CamInfo.rMapping_Info.eIspProfile, eIspProfile);

    rP1CamInfo.rMapping_Info.eIspProfile = eIspProfile;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
HalIspImp::
setZoomRatio(RAWIspCamInfo &rP1CamInfo, MINT32 const i4ZoomRatio_x100)
{
    CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1, "[+setZoomRatio](old, new)=(%d, %d)", rP1CamInfo.i4ZoomRatio_x100, i4ZoomRatio_x100);
    rP1CamInfo.i4ZoomRatio_x100 = i4ZoomRatio_x100;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
HalIspImp::
setAEInfo2ISP(RAWIspCamInfo &rP1CamInfo, AE_ISP_INFO_T  const &rAEInfo_)
{
    CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1, "setAEInfo2ISP()");

    AE_ISP_INFO_T rAEInfo = rAEInfo_;

    MINT32 iso;
    getPropInt("vendor.isp.aeinfo.iso", &iso, -1); // -1: default, others: set ISO value
    if (iso != -1) rAEInfo.u4P1RealISOValue = iso;

    MINT32 stopUpdate = 0x00;
    getPropInt("vendor.isp.aeinfo.ctrl", &stopUpdate, 0); // 0: default, 1: bypass AE info

    if (!stopUpdate) rP1CamInfo.rAEInfo = rAEInfo;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
HalIspImp::
setAWBInfo2ISP(RAWIspCamInfo &rP1CamInfo, AWB_ISP_INFO_T  const &rAWBInfo)
{
    CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1, "setAWBInfo2ISP()");

    rP1CamInfo.rAWBInfo = rAWBInfo;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
HalIspImp::
setFDEnabletoISP(RAWIspCamInfo &rP1CamInfo, MBOOL const FD_enable)
{
    CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1, "[%s] FDEnalbe = %d", __FUNCTION__, FD_enable);
    if(FD_enable){
        rP1CamInfo.fgFDEnable = MTRUE;
    }
    else{
        rP1CamInfo.fgFDEnable = MFALSE;
    }

    if(!rP1CamInfo.fgFDEnable){
        rP1CamInfo.rFdInfo.FaceNum = 0;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
inline static
CameraArea_T _transformArea(const MUINT32 i4SensorIdx, const MINT32 i4SensorMode, const CameraArea_T& rArea)
{
    if (rArea.i4Left == 0 && rArea.i4Top == 0 && rArea.i4Right == 0 && rArea.i4Bottom == 0)
    {
        return rArea;
    }

    CameraArea_T rOut;
    MSize size(rArea.i4Right- rArea.i4Left,rArea.i4Bottom- rArea.i4Top);
    MPoint point(rArea.i4Left, rArea.i4Top);
    MRect input(point,size);
    MRect output;

    HwTransHelper helper(i4SensorIdx);
    HwMatrix mat;
    if(!helper.getMatrixToActive(i4SensorMode, mat))
        CAM_LOGE("Get hw matrix failed");
    mat.transform(input, output);

    rOut.i4Left   = output.p.x;
    rOut.i4Right  = output.p.x + output.s.w;
    rOut.i4Top    = output.p.y;
    rOut.i4Bottom = output.p.y + output.s.h;
    rOut.i4Weight = rArea.i4Weight;
/*
    ALOGD("[%s] in(%d,%d,%d,%d), out(%d,%d,%d,%d) sensormode(%d)", __FUNCTION__,
        input.p.x, input.p.y, input.s.w, input.s.h,
        output.p.x, output.p.y, output.s.w, output.s.h,
        i4SensorMode);
    ALOGD("[%s] rArea(%d,%d,%d,%d), rOut(%d,%d,%d,%d)", __FUNCTION__,
        rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom,
        rOut.i4Left, rOut.i4Top, rOut.i4Right, rOut.i4Bottom);
*/
    return rOut;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

inline static
CameraArea_T _clipArea(const MINT32 i4TgWidth, const MINT32 i4TgHeight, const CameraArea_T& rArea)
{
    if (rArea.i4Left == 0 && rArea.i4Top == 0 && rArea.i4Right == 0 && rArea.i4Bottom == 0)
    {
        return rArea;
    }
    if (i4TgWidth == 0 && i4TgHeight == 0)
    {
        return rArea;
    }
    CameraArea_T rOut;
    MINT32 i4AreaWidth  = rArea.i4Right - rArea.i4Left;
    MINT32 i4AreaHeight = rArea.i4Bottom - rArea.i4Top;
    MINT32 xOffset = 0;
    MINT32 yOffset = 0;

    if(i4AreaWidth > i4TgWidth)
        xOffset = (i4AreaWidth - i4TgWidth) / 2;
    if(i4AreaHeight > i4TgHeight)
        yOffset = (i4AreaHeight - i4TgHeight) / 2;

    rOut.i4Left   = rArea.i4Left + xOffset;
    rOut.i4Top    = rArea.i4Top + yOffset;
    rOut.i4Right  = rArea.i4Right - xOffset;
    rOut.i4Bottom = rArea.i4Bottom - yOffset;
    rOut.i4Weight = rArea.i4Weight;
/*
    CAM_LOGD("[%s] rArea(%d,%d,%d,%d), rOut(%d,%d,%d,%d) offset(%d,%d)", __FUNCTION__,
        rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom,
        rOut.i4Left, rOut.i4Top, rOut.i4Right, rOut.i4Bottom,
        xOffset, yOffset);
*/
    return rOut;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
HalIspImp::
setFDInfotoISP(RAWIspCamInfo &rP1CamInfo, MVOID* prFaces)
{
    CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1, "[%s] m_bFaceDetectEnable(%d)", __FUNCTION__, rP1CamInfo.fgFDEnable);
    if (rP1CamInfo.fgFDEnable) {
        MtkCameraFaceMetadata *pFaces = (MtkCameraFaceMetadata *)prFaces;
        CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1, "[%s] m_i4faceNum(%d)", __FUNCTION__, pFaces->number_of_faces);

        CAMERA_TUNING_FD_INFO_T fd_tuning_info;
        memset(&fd_tuning_info, 0, sizeof(CAMERA_TUNING_FD_INFO_T));
        fd_tuning_info.FaceNum = pFaces->number_of_faces;

        static_assert( sizeof(pFaces->YUVsts) <= sizeof(fd_tuning_info.YUVsts),"face struct YUVsts size error");
        static_assert( sizeof(pFaces->GenderLabel) <= sizeof(fd_tuning_info.fld_GenderLabel),"face struct fld_GenderLabel size error");
        static_assert( sizeof(pFaces->fld_GenderInfo) <= sizeof(fd_tuning_info.fld_GenderInfo),"face struct fld_GenderInfo size error");
        static_assert( sizeof(pFaces->fld_rop) <= sizeof(fd_tuning_info.fld_rop),"face struct fld_rop size error");

        memcpy(&(fd_tuning_info.YUVsts), &(pFaces->YUVsts), sizeof(pFaces->YUVsts));
        memcpy(&(fd_tuning_info.fld_GenderLabel), &(pFaces->GenderLabel), sizeof(pFaces->GenderLabel));
        memcpy(&(fd_tuning_info.fld_GenderInfo), &(pFaces->fld_GenderInfo), sizeof(pFaces->fld_GenderInfo));
        memcpy(&(fd_tuning_info.fld_rop), &(pFaces->fld_rop), sizeof(pFaces->fld_rop));
        memcpy(&(fd_tuning_info.Landmark_CV), &(pFaces->fa_cv), sizeof(pFaces->fa_cv));

        fd_tuning_info.FaceNum = pFaces->number_of_faces;
        fd_tuning_info.GenderNum = pFaces->genderNum;
        fd_tuning_info.LandmarkNum = pFaces->poseNum;

        fd_tuning_info.FaceGGM_Idx = pFaces->gmIndex;

        if((pFaces->gmSize) == GGM_LUT_SIZE)
            for(int i=0; i< (pFaces->gmSize); i++ ){
                fd_tuning_info.FaceGGM.lut[i].bits.GGM_R
                    = fd_tuning_info.FaceGGM.lut[i].bits.GGM_G
                    = fd_tuning_info.FaceGGM.lut[i].bits.GGM_B
                    = pFaces->gmData[i];
        }

        MUINT32 FaceArea = 0;
        MUINT32 temp_area = 0;

        if (pFaces->number_of_faces != 0)
        {
            // calculate face TG size
            int i = 0;
            CameraArea_T rArea;

            for(i = 0; i < pFaces->number_of_faces; i++) {
                fd_tuning_info.fld_rip[i] = pFaces->posInfo[i].rip_dir;

                // face
                rArea.i4Left   = pFaces->faces[i].rect[0];  //Left
                rArea.i4Top    = pFaces->faces[i].rect[1];  //Top
                rArea.i4Right  = pFaces->faces[i].rect[2];  //Right
                rArea.i4Bottom = pFaces->faces[i].rect[3];  //Bottom
                rArea.i4Weight = 0;

                //TG Domain
                temp_area = ABS(rArea.i4Right - rArea.i4Left)* ABS(rArea.i4Bottom - rArea.i4Top);
                FaceArea += temp_area;

                rArea = _transformArea(m_i4SensorIdx, m_i4SensorMode, rArea);
                rArea = _clipArea(m_i4TgWidth, m_i4TgHeight, rArea);

                CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1,"After _transformArea x0(%d), y0(%d), x1(%d), y1(%d)", rArea.i4Left, rArea.i4Top, rArea.i4Right, rArea.i4Bottom);

                fd_tuning_info.rect[i][0] = rArea.i4Left;
                fd_tuning_info.rect[i][1] = rArea.i4Top;
                fd_tuning_info.rect[i][2] = rArea.i4Right;
                fd_tuning_info.rect[i][3] = rArea.i4Bottom;

                if(pFaces->fa_cv[i] > 0) // 0 is invalid value
                {
                    // left eye
                    rArea.i4Left   = pFaces->leyex0[i];  //Left
                    rArea.i4Top    = pFaces->leyey0[i];  //Top
                    rArea.i4Right  = pFaces->leyex1[i];  //Right
                    rArea.i4Bottom = pFaces->leyey1[i];  //Bottom
                    rArea = _transformArea(m_i4SensorIdx, m_i4SensorMode, rArea);
                    rArea = _clipArea(m_i4TgWidth, m_i4TgHeight, rArea);

                    fd_tuning_info.Face_Leye[i][0] = rArea.i4Left;
                    fd_tuning_info.Face_Leye[i][1] = rArea.i4Top;
                    fd_tuning_info.Face_Leye[i][2] = rArea.i4Right;
                    fd_tuning_info.Face_Leye[i][3] = rArea.i4Bottom;

                    // right eye
                    rArea.i4Left   = pFaces->reyex0[i];  //Left
                    rArea.i4Top    = pFaces->reyey0[i];  //Top
                    rArea.i4Right  = pFaces->reyex1[i];  //Right
                    rArea.i4Bottom = pFaces->reyey1[i];  //Bottom
                    rArea = _transformArea(m_i4SensorIdx, m_i4SensorMode, rArea);
                    rArea = _clipArea(m_i4TgWidth, m_i4TgHeight, rArea);
                    fd_tuning_info.Face_Reye[i][0] = rArea.i4Left;
                    fd_tuning_info.Face_Reye[i][1] = rArea.i4Top;
                    fd_tuning_info.Face_Reye[i][2] = rArea.i4Right;
                    fd_tuning_info.Face_Reye[i][3] = rArea.i4Bottom;
                }
             }
        }

        //Face Ratio
        MUINT32 TGSize = m_i4TgWidth * m_i4TgHeight;
        float FDRatio = (float)FaceArea / (float)TGSize;
        if( FDRatio > 1.0 ){
            FDRatio = 1.0;
        }
        fd_tuning_info.FDRatio = FDRatio;

        CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1,"%s: SensorDev(%d), TGSize(%d), FD_ratio = %f\n", __FUNCTION__, m_i4SensorDev, TGSize, FDRatio);

        MINT32 ret = 0;
        MINT32 i = 0, j=0;

        memcpy(&(rP1CamInfo.rFdInfo), &fd_tuning_info, sizeof(CAMERA_TUNING_FD_INFO_T));
        for(i = 0; i < rP1CamInfo.rFdInfo.FaceNum; i++) {
            CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1,"[%s] fld_GenderLabel: %d ",
                __FUNCTION__, rP1CamInfo.rFdInfo.fld_GenderLabel[i]);

            for(j = 0; j < 5; j++) {
                CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1,"[%s] faceNum(%d), rP1CamInfo.rFdInfo.YUVsts[%d][%d]: %d ",
                    __FUNCTION__, rP1CamInfo.rFdInfo.FaceNum, i, j, rP1CamInfo.rFdInfo.YUVsts[i][j]);
            }
        }

        if(rP1CamInfo.rFdInfo.FaceNum != 0){
            rP1CamInfo.rMapping_Info.eFaceDetection = EFaceDetection_Yes;
            CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1,"[%s] EFaceDetection_Yes",__FUNCTION__);
        }
        else{
            rP1CamInfo.rMapping_Info.eFaceDetection = EFaceDetection_No;
            CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1,"[%s] EFaceDetection_No",__FUNCTION__);
        }
    }
    else {
        memset(&(rP1CamInfo.rFdInfo), 0, sizeof(CAMERA_TUNING_FD_INFO_T));
    }

    rP1CamInfo.rFdInfo.FD_source = m_target_source;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
HalIspImp::
setSensorMode(RAWIspCamInfo &rP1CamInfo, MINT32 const i4SensorMode, MBOOL const bFrontalBin, MUINT32 const u4RawWidth, MUINT32 const u4RawHeight)
{
    CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1,"%s (%d), FBin: %d, Raw: (%d, %d)\n", __FUNCTION__, i4SensorMode, bFrontalBin, u4RawWidth, u4RawHeight);

    NSCam::MSize RawSize;
    RawSize.w = u4RawWidth;
    RawSize.h = u4RawHeight;

    HwTransHelper helper(m_i4SensorIdx);
    NSCamHW::HwMatrix mat;
    if(!helper.getMatrixFromActive(m_i4LogEn & HALISP_LOG_SET_P1, mat))
        MY_ERR("Get hw matrix failed");
    if(m_i4LogEn & HALISP_LOG_SET_P1)
        mat.dump(__FUNCTION__);
    mMapMat[m_i4SensorIdx] = mat;

    MINT32 ret = 0;

    CAM_LOGD_IF((rP1CamInfo.rMapping_Info.eSensorMode != i4SensorMode), "[+setSensorMode](old, new)=(%d, %d)",
        rP1CamInfo.rMapping_Info.eSensorMode, i4SensorMode);
    rP1CamInfo.rMapping_Info.eSensorMode = static_cast<ESensorMode_T>(i4SensorMode);

    if(bFrontalBin){
        rP1CamInfo.rMapping_Info.eFrontBin = EFrontBin_Yes;
        rP1CamInfo.rCropRzInfo.fgFBinOnOff = MTRUE;
    }
    else{
        rP1CamInfo.rMapping_Info.eFrontBin = EFrontBin_No;
        rP1CamInfo.rCropRzInfo.fgFBinOnOff = MFALSE;
    }

    rP1CamInfo.rCropRzInfo.sRRZin.w = RawSize.w;
    rP1CamInfo.rCropRzInfo.sRRZin.h = RawSize.h;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
HalIspImp::
setCCUInfo2ISP(RAWIspCamInfo &rP1CamInfo, ISP_CCU_RESULT_T const &rCCUInfo)
{
    CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1, "setCCUInfo2ISP()");

    rP1CamInfo.rCCU_Result=rCCUInfo;
    rP1CamInfo.rCCU_Result.LTM.LTM_Valid=CCU_VALID_LAST;
    rP1CamInfo.rCCU_Result.HLR.HLR_Valid=CCU_VALID_LAST; // set p1 caminfo alway LAST
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
HalIspImp::
setTGInfo(RAWIspCamInfo &rP1CamInfo, MINT32 const i4TGInfo, MINT32 const i4Width, MINT32 const i4Height)
{
    CAM_LOGD_IF(m_i4LogEn & HALISP_LOG_SET_P1, "[%s] i4TGInfo(%d) i4Width(%d), i4Height(%d)", __FUNCTION__, i4TGInfo, i4Width, i4Height);

    rP1CamInfo.rCropRzInfo.sTGout.w = i4Width;
    rP1CamInfo.rCropRzInfo.sTGout.h = i4Height;

    switch (i4TGInfo)
    {
    case CAM_TG_1:
        rP1CamInfo.eSensorTG = ESensorTG_1;
        break;
    case CAM_TG_2:
        rP1CamInfo.eSensorTG = ESensorTG_2;
        break;
    case CAM_TG_3:
        rP1CamInfo.eSensorTG = ESensorTG_3;
        break;
    default:
        CAM_LOGE("[%s] i4TGInfo = %d", __FUNCTION__, i4TGInfo);
    }
}

