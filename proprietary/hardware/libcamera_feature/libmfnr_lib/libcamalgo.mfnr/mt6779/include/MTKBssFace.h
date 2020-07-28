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
#ifndef _MTK_BSS_FACES_H_
#define _MTK_BSS_FACES_H_

#include "MTKBssType.h"

//typedef int MINT32;
//typedef float float32_t;
/******************************************************************************
 *  The information of a face from camera face detection.
 ******************************************************************************/
 struct BssFace {
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
    MINT32 rect[4];

    /**
     * The confidence level of the face. The range is 1 to 100. 100 is the
     * highest confidence. This is supported by both hardware and software
     * face detection.
     */
    MINT32 score;

    /**
     * An unique id per face while the face is visible to the tracker. If
     * the face leaves the field-of-view and comes back, it will get a new
     * id. If the value is 0, id is not supported.
     */
    MINT32 id;

    /**
     * The coordinates of the center of the left eye. The range is -1000 to
     * 1000. -2000, -2000 if this is not supported.
     */
    MINT32 left_eye[2];

    /**
     * The coordinates of the center of the right eye. The range is -1000 to
     * 1000. -2000, -2000 if this is not supported.
     */
    MINT32 right_eye[2];

    /**
     * The coordinates of the center of the mouth. The range is -1000 to 1000.
     * -2000, -2000 if this is not supported.
     */
    MINT32 mouth[2];
};


/******************************************************************************
 *   FD Pose Information: ROP & RIP
 *****************************************************************************/
struct BssFaceInfo {

    MINT32 rop_dir;
    MINT32 rip_dir;
};

/******************************************************************************
 *   CNN FD Information
 *****************************************************************************/
struct BssCNNFaceInfo {
    MINT32 PortEnable;
    MINT32 IsTrueFace;
    float CnnResult0;
    float CnnResult1;
};

/******************************************************************************
 *  The metadata of the frame data.
 *****************************************************************************/
struct BssFaceMetadata {
    /**
     * The number of detected faces in the frame.
     */
    MINT32 number_of_faces;

    /**
     * An array of the detected faces. The length is number_of_faces.
     */
    BssFace   *faces;
    BssFaceInfo     *posInfo;
    MINT32         faces_type[15];
    MINT32         motion[15][2];

    MINT32 ImgWidth;
    MINT32 ImgHeight;

    MINT32 gmIndex;
    MINT32* gmData;
    MINT32 gmSize;
    MINT32 genderNum;
    MINT32 poseNum;
    MINT32 landmarkNum;
    MINT32 leyex0[15];
    MINT32 leyey0[15];
    MINT32 leyex1[15];
    MINT32 leyey1[15];
    MINT32 reyex0[15];
    MINT32 reyey0[15];
    MINT32 reyex1[15];
    MINT32 reyey1[15];
    MINT32 nosex[15];
    MINT32 nosey[15];
    MINT32 mouthx0[15];
    MINT32 mouthy0[15];
    MINT32 mouthx1[15];
    MINT32 mouthy1[15];
    MINT32 leyeux[15];
    MINT32 leyeuy[15];
    MINT32 leyedx[15];
    MINT32 leyedy[15];
    MINT32 reyeux[15];
    MINT32 reyeuy[15];
    MINT32 reyedx[15];
    MINT32 reyedy[15];
    MINT32 fa_cv[15];
    MINT32 fld_rip[15];
    MINT32 fld_rop[15];
    MINT32 YUVsts[15][5];
    MUINT8 fld_GenderLabel[15];
    MINT32 fld_GenderInfo[15];
    MUINT8 GenderLabel[15];
    MUINT8 oGenderLabel[15];
    MUINT8 GenderCV[15];
    MUINT8 RaceLabel[15];
    MUINT8 RaceCV[15][4];

    int16_t poseinfo[15][10];
    /**
     * Timestamp of source yuv frame
     */
    MINT64 timestamp;
    /**
     * for CNN face
     */
    BssCNNFaceInfo CNNFaces;
    BssFaceMetadata(): number_of_faces(0), faces(0), posInfo(0), gmIndex(0), gmData(NULL), gmSize(0)
    {
    }
};


#endif

