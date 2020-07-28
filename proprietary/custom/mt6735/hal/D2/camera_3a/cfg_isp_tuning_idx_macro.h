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
#ifndef _CFG_ISP_TUNING_IDX_MACRO_H_
#define _CFG_ISP_TUNING_IDX_MACRO_H_


#define CAPTURE_SCENE_MODE_CONFIG(sensor, scene)\
    IDX_MODE_NormalCapture(sensor, scene, eIDX_ISO_100)\
    IdxSet<EIspProfile_NormalCapture, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_100>::idx;\
    IDX_MODE_NormalCapture(sensor, scene, eIDX_ISO_200)\
    IdxSet<EIspProfile_NormalCapture, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_200>::idx;\
    IDX_MODE_NormalCapture(sensor, scene, eIDX_ISO_400)\
    IdxSet<EIspProfile_NormalCapture, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_400>::idx;\
    IDX_MODE_NormalCapture(sensor, scene, eIDX_ISO_800)\
    IdxSet<EIspProfile_NormalCapture, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_800>::idx;\
    IDX_MODE_NormalCapture(sensor, scene, eIDX_ISO_1200)\
    IdxSet<EIspProfile_NormalCapture, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_1200>::idx;\
    IDX_MODE_NormalCapture(sensor, scene, eIDX_ISO_1600)\
    IdxSet<EIspProfile_NormalCapture, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_1600>::idx;\
    IDX_MODE_NormalCapture(sensor, scene, eIDX_ISO_2000)\
    IdxSet<EIspProfile_NormalCapture, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_2000>::idx;\
    IDX_MODE_NormalCapture(sensor, scene, eIDX_ISO_2400)\
    IdxSet<EIspProfile_NormalCapture, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_2400>::idx;\
    IDX_MODE_NormalCapture(sensor, scene, eIDX_ISO_2800)\
    IdxSet<EIspProfile_NormalCapture, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_2800>::idx;\
    IDX_MODE_NormalCapture(sensor, scene, eIDX_ISO_3200)\
    IdxSet<EIspProfile_NormalCapture, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_3200>::idx;\



#define PREVIEW_SCENE_MODE_CONFIG(sensor, scene)\
    IDX_MODE_NormalPreview(sensor, scene, eIDX_ISO_100)\
    IdxSet<EIspProfile_NormalPreview, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_100>::idx;\
    IDX_MODE_NormalPreview(sensor, scene, eIDX_ISO_200)\
    IdxSet<EIspProfile_NormalPreview, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_200>::idx;\
    IDX_MODE_NormalPreview(sensor, scene, eIDX_ISO_400)\
    IdxSet<EIspProfile_NormalPreview, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_400>::idx;\
    IDX_MODE_NormalPreview(sensor, scene, eIDX_ISO_800)\
    IdxSet<EIspProfile_NormalPreview, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_800>::idx;\
    IDX_MODE_NormalPreview(sensor, scene, eIDX_ISO_1200)\
    IdxSet<EIspProfile_NormalPreview, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_1200>::idx;\
    IDX_MODE_NormalPreview(sensor, scene, eIDX_ISO_1600)\
    IdxSet<EIspProfile_NormalPreview, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_1600>::idx;\
    IDX_MODE_NormalPreview(sensor, scene, eIDX_ISO_2000)\
    IdxSet<EIspProfile_NormalPreview, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_2000>::idx;\
    IDX_MODE_NormalPreview(sensor, scene, eIDX_ISO_2400)\
    IdxSet<EIspProfile_NormalPreview, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_2400>::idx;\
    IDX_MODE_NormalPreview(sensor, scene, eIDX_ISO_2800)\
    IdxSet<EIspProfile_NormalPreview, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_2800>::idx;\
    IDX_MODE_NormalPreview(sensor, scene, eIDX_ISO_3200)\
    IdxSet<EIspProfile_NormalPreview, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_3200>::idx;\



#define VIDEO_SCENE_MODE_CONFIG(sensor, scene)\
    IDX_MODE_VideoPreview(sensor, scene, eIDX_ISO_100)\
    IdxSet<EIspProfile_VideoPreview, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_100>::idx;\
    IDX_MODE_VideoPreview(sensor, scene, eIDX_ISO_200)\
    IdxSet<EIspProfile_VideoPreview, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_200>::idx;\
    IDX_MODE_VideoPreview(sensor, scene, eIDX_ISO_400)\
    IdxSet<EIspProfile_VideoPreview, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_400>::idx;\
    IDX_MODE_VideoPreview(sensor, scene, eIDX_ISO_800)\
    IdxSet<EIspProfile_VideoPreview, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_800>::idx;\
    IDX_MODE_VideoPreview(sensor, scene, eIDX_ISO_1200)\
    IdxSet<EIspProfile_VideoPreview, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_1200>::idx;\
    IDX_MODE_VideoPreview(sensor, scene, eIDX_ISO_1600)\
    IdxSet<EIspProfile_VideoPreview, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_1600>::idx;\
    IDX_MODE_VideoPreview(sensor, scene, eIDX_ISO_2000)\
    IdxSet<EIspProfile_VideoPreview, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_2000>::idx;\
    IDX_MODE_VideoPreview(sensor, scene, eIDX_ISO_2400)\
    IdxSet<EIspProfile_VideoPreview, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_2400>::idx;\
    IDX_MODE_VideoPreview(sensor, scene, eIDX_ISO_2800)\
    IdxSet<EIspProfile_VideoPreview, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_2800>::idx;\
    IDX_MODE_VideoPreview(sensor, scene, eIDX_ISO_3200)\
    IdxSet<EIspProfile_VideoPreview, sensor, MTK_CONTROL_SCENE_MODE_DISABLED, eIDX_ISO_3200>::idx;\



#define LINK_ONE_SCENE_ISOs(link, scene)\
    link(scene, eIDX_ISO_100);\
    link(scene, eIDX_ISO_200);\
    link(scene, eIDX_ISO_400);\
    link(scene, eIDX_ISO_800);\
    link(scene, eIDX_ISO_1200);\
    link(scene, eIDX_ISO_1600);\
    link(scene, eIDX_ISO_2000);\
    link(scene, eIDX_ISO_2400);\
    link(scene, eIDX_ISO_2800);\
    link(scene, eIDX_ISO_3200);


#define LINK_SCENEs_ISOs(link)\
    LINK_ONE_SCENE_ISOs(link, MTK_CONTROL_SCENE_MODE_DISABLED);\
    LINK_ONE_SCENE_ISOs(link, MTK_CONTROL_SCENE_MODE_FACE_PRIORITY);\
    LINK_ONE_SCENE_ISOs(link, MTK_CONTROL_SCENE_MODE_ACTION);\
    LINK_ONE_SCENE_ISOs(link, MTK_CONTROL_SCENE_MODE_PORTRAIT);\
    LINK_ONE_SCENE_ISOs(link, MTK_CONTROL_SCENE_MODE_LANDSCAPE);\
    LINK_ONE_SCENE_ISOs(link, MTK_CONTROL_SCENE_MODE_NIGHT);\
    LINK_ONE_SCENE_ISOs(link, MTK_CONTROL_SCENE_MODE_NIGHT_PORTRAIT);\
    LINK_ONE_SCENE_ISOs(link, MTK_CONTROL_SCENE_MODE_THEATRE);\
    LINK_ONE_SCENE_ISOs(link, MTK_CONTROL_SCENE_MODE_BEACH);\
    LINK_ONE_SCENE_ISOs(link, MTK_CONTROL_SCENE_MODE_SNOW);\
    LINK_ONE_SCENE_ISOs(link, MTK_CONTROL_SCENE_MODE_SUNSET);\
    LINK_ONE_SCENE_ISOs(link, MTK_CONTROL_SCENE_MODE_STEADYPHOTO);\
    LINK_ONE_SCENE_ISOs(link, MTK_CONTROL_SCENE_MODE_FIREWORKS);\
    LINK_ONE_SCENE_ISOs(link, MTK_CONTROL_SCENE_MODE_SPORTS);\
    LINK_ONE_SCENE_ISOs(link, MTK_CONTROL_SCENE_MODE_PARTY);\
    LINK_ONE_SCENE_ISOs(link, MTK_CONTROL_SCENE_MODE_CANDLELIGHT);\
    LINK_ONE_SCENE_ISOs(link, MTK_CONTROL_SCENE_MODE_BARCODE);\
    LINK_ONE_SCENE_ISOs(link, MTK_CONTROL_SCENE_MODE_HIGH_SPEED_VIDEO);\
    LINK_ONE_SCENE_ISOs(link, MTK_CONTROL_SCENE_MODE_NORMAL);\
    LINK_ONE_SCENE_ISOs(link, MTK_CONTROL_SCENE_MODE_HDR)

#define LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, scene)\
    link(sensor, scene, eIDX_ISO_100);\
    link(sensor, scene, eIDX_ISO_200);\
    link(sensor, scene, eIDX_ISO_400);\
    link(sensor, scene, eIDX_ISO_800);\
    link(sensor, scene, eIDX_ISO_1200);\
    link(sensor, scene, eIDX_ISO_1600);\
    link(sensor, scene, eIDX_ISO_2000);\
    link(sensor, scene, eIDX_ISO_2400);\
    link(sensor, scene, eIDX_ISO_2800);\
    link(sensor, scene, eIDX_ISO_3200);

#define LINK_ONE_SCENE_SENSORS_ISOs(link, scene)\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, ESensorMode_Preview, scene);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, ESensorMode_Video, scene);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, ESensorMode_Capture, scene);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, ESensorMode_SlimVideo1, scene);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, ESensorMode_SlimVideo2, scene);

#define LINK_ONE_SENSOR_SCENEs_ISOs(link, sensor)\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_DISABLED);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_NORMAL);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_ACTION);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_PORTRAIT);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_LANDSCAPE);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_NIGHT);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_NIGHT_PORTRAIT);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_THEATRE);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_BEACH);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_SNOW);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_SUNSET);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_STEADYPHOTO);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_FIREWORKS);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_SPORTS);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_PARTY);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_CANDLELIGHT);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_HDR);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_FACE_PRIORITY);\
    LINK_ONE_SENSOR_ONE_SCENE_ISOs(link, sensor, MTK_CONTROL_SCENE_MODE_BARCODE);

#define LINK_SENSORs_SCENEs_ISOs(link)\
    LINK_ONE_SENSOR_SCENEs_ISOs(link, ESensorMode_Preview);\
    LINK_ONE_SENSOR_SCENEs_ISOs(link, ESensorMode_Video);\
    LINK_ONE_SENSOR_SCENEs_ISOs(link, ESensorMode_Capture);\
    LINK_ONE_SENSOR_SCENEs_ISOs(link, ESensorMode_SlimVideo1);\
    LINK_ONE_SENSOR_SCENEs_ISOs(link, ESensorMode_SlimVideo2);\
    LINK_ONE_SENSOR_SCENEs_ISOs(link, ESensorMode_Custom1);\
    LINK_ONE_SENSOR_SCENEs_ISOs(link, ESensorMode_Custom2);\
    LINK_ONE_SENSOR_SCENEs_ISOs(link, ESensorMode_Custom3);\
    LINK_ONE_SENSOR_SCENEs_ISOs(link, ESensorMode_Custom4);\
    LINK_ONE_SENSOR_SCENEs_ISOs(link, ESensorMode_Custom5);

#define LINK_CAPTURE(sensor, scene, iso)\
m_pNormalCapture[sensor][scene][iso] = &IdxSet<EIspProfile_NormalCapture, sensor, scene, iso>::idx

#define LINK_VIDEO(sensor, scene, iso)\
m_pVideoPreview[sensor][scene][iso] = &IdxSet<EIspProfile_VideoPreview, sensor, scene, iso>::idx

#define LINK_PREVIEW(sensor, scene, iso)\
m_pNormalPreview[sensor][scene][iso] = &IdxSet<EIspProfile_NormalPreview, sensor, scene, iso>::idx

/*
#define LINK_NORMAL_PREVIEW(scene, iso)\
m_pNormalPreview[scene][iso] = &IdxSet<EIspProfile_NormalPreview, scene, iso>::idx
*/

#define LINK_ZSD_PREVIEW_CC(scene, iso)\
m_pZsdPreview_CC[scene][iso] = &IdxSet<EIspProfile_ZsdPreview_CC, scene, iso>::idx

#define LINK_ZSD_PREVIEW_NCC(scene, iso)\
m_pZsdPreview_NCC[scene][iso] = &IdxSet<EIspProfile_ZsdPreview_NCC, scene, iso>::idx

/*
#define LINK_NORMAL_CAPTURE(scene, iso)\
m_pNormalCapture[scene][iso] = &IdxSet<EIspProfile_NormalCapture, scene, iso>::idx
*/

/*
#define LINK_VIDEO_PREVIEW(scene, iso)\
m_pVideoPreview [iso] = &IdxSet<EIspProfile_VideoPreview, iso>::idx;
*/
#define LINK_VIDEO_CAPTURE(scene, iso)\
m_pVideoCapture [iso] = &IdxSet<EIspProfile_VideoCapture, iso>::idx;

#define LINK_MF_CAPTURE_PASS1(scene, iso)\
m_pMFCapturePass1 [iso] = &IdxSet<EIspProfile_MFCapPass1, iso>::idx;

#define LINK_MF_CAPTURE_PASS2(scene, iso)\
m_pMFCapturePass2 [iso] = &IdxSet<EIspProfile_MFCapPass2, iso>::idx;



#endif //  _CFG_ISP_TUNING_IDX_MACRO_H_

