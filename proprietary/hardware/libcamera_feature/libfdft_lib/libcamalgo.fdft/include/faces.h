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
#ifndef _MHAL_MTK_FACES_H_
#define _MHAL_MTK_FACES_H_

typedef int int32_t;
typedef float float32_t;
typedef long long int MTEE_INT64;
/******************************************************************************
 *  The information of a face from camera face detection.
 ******************************************************************************/
 struct MtkCameraFace {
    /**
     * Bounds of the face [left, top, right, bottom]. (-1000, -1000) represents
     * the top-left of the camera field of view, and (1000, 1000) represents the
     * bottom-right of the field of view. The width and height cannot be 0 or
     * negative. This is supported by both hardware and software face detection.
     *
     * The direction is relative to the sensor orientation, that is, what the
     * sensor sees. The direction is not affected by the rotation or mirroring
     * of CAMERA_CMD_SET_DISPLAY_ORIENTATION.
     */
    int32_t rect[4];

    /**
     * The confidence level of the face. The range is 1 to 100. 100 is the
     * highest confidence. This is supported by both hardware and software
     * face detection.
     */
    int32_t score;

    /**
     * An unique id per face while the face is visible to the tracker. If
     * the face leaves the field-of-view and comes back, it will get a new
     * id. If the value is 0, id is not supported.
     */
    int32_t id;

    /**
     * The coordinates of the center of the left eye. The range is -1000 to
     * 1000. -2000, -2000 if this is not supported.
     */
    int32_t left_eye[2];

    /**
     * The coordinates of the center of the right eye. The range is -1000 to
     * 1000. -2000, -2000 if this is not supported.
     */
    int32_t right_eye[2];

    /**
     * The coordinates of the center of the mouth. The range is -1000 to 1000.
     * -2000, -2000 if this is not supported.
     */
    int32_t mouth[2];
};


/******************************************************************************
 *   FD Pose Information: ROP & RIP
 *****************************************************************************/
struct MtkFaceInfo {

    int32_t rop_dir;
    int32_t rip_dir;
};

/******************************************************************************
 *   CNN FD Information
 *****************************************************************************/
struct MtkCNNFaceInfo {
    int32_t PortEnable;
    int32_t IsTrueFace;
    float32_t CnnResult0;
    float32_t CnnResult1;
};

/******************************************************************************
 *  The metadata of the frame data.
 *****************************************************************************/
struct MtkCameraFaceMetadata {
    /**
     * The number of detected faces in the frame.
     */
    int32_t number_of_faces;

    /**
     * An array of the detected faces. The length is number_of_faces.
     */
#if defined(_SIM_PC_)
    MtkCameraFace   *faces;
    MtkFaceInfo     *posInfo;
#else
    MtkCameraFace   faces[15];
    MtkFaceInfo     posInfo[15];
#endif
    int32_t         faces_type[15];
    int32_t         motion[15][2];

    int32_t ImgWidth;
    int32_t ImgHeight;

    int32_t gmIndex;
    union{
    int32_t* gmData;
        MTEE_INT64 TEMP64_2;
    };
    int32_t gmSize;
    int32_t genderNum;
    int32_t poseNum;
    int32_t landmarkNum;
    int32_t leyex0[15];
    int32_t leyey0[15];
    int32_t leyex1[15];
    int32_t leyey1[15];
    int32_t reyex0[15];
    int32_t reyey0[15];
    int32_t reyex1[15];
    int32_t reyey1[15];
    int32_t nosex[15];
    int32_t nosey[15];
    int32_t mouthx0[15];
    int32_t mouthy0[15];
    int32_t mouthx1[15];
    int32_t mouthy1[15];
    int32_t leyeux[15];
    int32_t leyeuy[15];
    int32_t leyedx[15];
    int32_t leyedy[15];
    int32_t reyeux[15];
    int32_t reyeuy[15];
    int32_t reyedx[15];
    int32_t reyedy[15];
    int32_t fa_cv[15];
    int32_t fld_rip[15];
    int32_t fld_rop[15];
    int32_t YUVsts[15][5];
    uint8_t fld_GenderLabel[15];
    int32_t fld_GenderInfo[15];
    uint8_t GenderLabel[15];
    uint8_t oGenderLabel[15];
    uint8_t GenderCV[15];
    uint8_t RaceLabel[15];
    uint8_t RaceCV[15][4];
    uint8_t visible_percent[15]; //the visible percentage of each face in preview FOV: 0~100
    int16_t poseinfo[15][10];

    /**
     * Timestamp of source yuv frame
     */
    int64_t timestamp;
    /**
     * for CNN face
     */
    MtkCNNFaceInfo CNNFaces;
    MtkCameraFaceMetadata(): number_of_faces(0), gmIndex(0), gmData(nullptr), gmSize(0)
    {
    }
};


#endif

