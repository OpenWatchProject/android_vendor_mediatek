/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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
*     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
*     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
*     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
************************************************************************************************/
#ifndef CAMERA_CUSTOM_STEREO_SETTING_H_
#define CAMERA_CUSTOM_STEREO_SETTING_H_
const char *DEFAULT_STEREO_SETTING =
"{"
    "\"Sensors\": ["
        "{"
            "\"Name\": \"IMX398_MIPI_RAW\","
            "\"Capture Size\": {"
                "\"4:3\": ["
                    "\"4656x3492\""
                "],"
                "\"16:9\": ["
                    "\"4608x2592\""
                "]"
            "},"
            "\"IMGOYUV Size\": {"
                "\"4:3\": \"4656x3492\","
                "\"16:9\": \"4608x2592\""
            "},"
            "\"RRZOYUV Size\": {"
                "\"4:3\": \"2328x1746\","
                "\"16:9\": \"2304x1296\""
            "},"
            "\"SensorScenarioZSD\": {"
                "\"VSDoF\": \"Preview\","
                "\"3rdParty\": \"Preview\","
                "\"Zoom\": \"Capture\","
                "\"MtkDepthmap\": \"Preview\","
                "\"Multicam\": \"Capture\""
            "},"
             "\"SensorScenarioRecording\": {"
                "\"VSDoF\": \"Preview\","
                "\"3rdParty\": \"Preview\","
                "\"Zoom\": \"Preview\","
                "\"MtkDepthmap\": \"Preview\","
                "\"Multicam\": \"Preview\""
            "},"
            "\"FOV\": {"
                "\"H\": 66.13,"
                "\"V\": 52.1"
            "},"
            "\"Calibration\": {"
                "\"Macro Distance\": 100,"
                "\"Infinite Distance\": 5000"
            "}"
        "},"
        "{"
            "\"Name\": \"IMX350_MIPI_RAW\","
            "\"Capture Size\": {"
                "\"4:3\": ["
                    "\"5120x3840\""
                "],"
                "\"16:9\": ["
                    "\"5120x2880\""
                "]"
            "},"
            "\"IMGOYUV Size\": {"
                "\"4:3\": \"5120x3840\","
                "\"16:9\": \"5120x2880\""
            "},"
            "\"RRZOYUV Size\": {"
                "\"4:3\": \"2560x1920\","
                "\"16:9\": \"2560x1440\""
            "},"
            "\"SensorScenarioZSD\": {"
                "\"VSDoF\": \"Capture\","
                "\"3rdParty\": \"Capture\","
                "\"Zoom\": \"Preview\","
                "\"MtkDepthmap\": \"Capture\","
                "\"Multicam\": \"Preview\""
            "},"
             "\"SensorScenarioRecording\": {"
                "\"VSDoF\": \"Capture\","
                "\"3rdParty\": \"Capture\","
                "\"Zoom\": \"Preview\","
                "\"MtkDepthmap\": \"Capture\","
                "\"Multicam\": \"Preview\""
            "},"
            "\"FOV\": {"
                "\"H\": 51.85,"
                "\"V\": 40.11"
            "},"
            "\"Calibration\": {"
                "\"Macro Distance\": 300,"
                "\"Infinite Distance\": 5000"
            "}"
        "},"
        "{"
            "\"Name\": \"IMX386_MIPI_RAW\","
            "\"Capture Size\": {"
                "\"4:3\": ["
                    "\"3968x2976\""
                "],"
                "\"16:9\": ["
                    "\"4032x2268\""
                "]"
            "},"
            "\"IMGOYUV Size\": {"
                "\"4:3\": \"4032x3016\","
                "\"16:9\": \"4032x2268\""
            "},"
            "\"RRZOYUV Size\": {"
                "\"4:3\": \"2016x1508\","
                "\"16:9\": \"2016x1134\""
            "},"
            "\"SensorScenarioZSD\": {"
                "\"VSDoF\": \"Capture\","
                "\"3rdParty\": \"Capture\","
                "\"MtkDepthmap\": \"Capture\","
                "\"Multicam\": \"Capture\""
            "},"
            "\"FOV\": {"
                "\"H\": 66.0,"
                "\"V\": 52.2"
            "},"
            "\"Calibration\": {"
                "\"Macro Distance\": 100,"
                "\"Infinite Distance\": 5000"
            "}"
        "},"
        "{"
            "\"Name\": \"IMX386_MIPI_MONO\","
            "\"IMGOYUV Size\": {"
                "\"4:3\": \"4032x3016\","
                "\"16:9\": \"4032x2268\""
            "},"
            "\"RRZOYUV Size\": {"
                "\"4:3\": \"2016x1508\","
                "\"16:9\": \"2016x1134\""
            "},"
            "\"SensorScenarioZSD\": {"
                "\"VSDoF\": \"Preview\","
                "\"3rdParty\": \"Preview\","
                "\"MtkDepthmap\": \"Preview\","
                "\"Multicam\": \"Capture\""
            "},"
            "\"FOV\": {"
                "\"H\": 66.0,"
                "\"V\": 52.2"
            "},"
            "\"Calibration\": {"
                "\"Macro Distance\": 100,"
                "\"Infinite Distance\": 5000"
            "}"
        "},"
        "{"
            "\"Name\": \"HM1062_MIPI_MONO\","
            "\"FOV\": {"
                "\"H\": 71.39,"
                "\"V\": 44.01"
            "},"
            "\"SensorScenarioZSD\": {"
                "\"VSDoF\": \"Capture\","
                "\"ActiveStereo\": \"Capture\","
                "\"MtkDepthmap\": \"Capture\""
            "}"
        "},"
        "{"
            "\"Name\": \"HM1062_2_MIPI_MONO\","
            "\"Capture Size\": {"
                "\"4:3\": ["
                    "\"960x720\""
                "],"
                "\"16:9\": ["
                    "\"1280x720\""
                "]"
            "},"
            "\"FOV\": {"
                "\"H\": 71.39,"
                "\"V\": 44.01"
            "},"
            "\"SensorScenarioZSD\": {"
                "\"VSDoF\": \"Capture\","
                "\"ActiveStereo\": \"Capture\","
                "\"MtkDepthmap\": \"Capture\""
            "}"
        "},"
        "{"
            "\"Name\": \"S5K4E6_MIPI_RAW\""
        "}"
    "],"
    "\"Sensor Combinations\": ["
        "{"
            "\"Logical Device\":{"
                "\"Name\": \"dualzoom\","
                "\"Features\": [\"Zoom\"]"
            "},"
            "\"Sensors\": [\"IMX398_MIPI_RAW\", \"IMX350_MIPI_RAW\"],"
            "\"Module Type\": 3,"
            "\"Baseline\": 0.97,"
            "\"Depthmap Size\": {"
                "\"4:3\": \"480x360\","
                "\"16:9\": \"480x272\""
            "},"
            "\"LDC\": [\"0\"]"
        "},"
        "{"
            "\"Logical Device\":{"
                "\"Name\": \"vsdof\","
                "\"Features\": [\"VSDoF\"]"
            "},"
            "\"Sensors\": [\"IMX350_MIPI_RAW\", \"IMX398_MIPI_RAW\"],"
            "\"Module Type\": 3,"
            "\"Baseline\": 0.97,"
            "\"Depthmap Size\": {"
                "\"4:3\": \"480x360\","
                "\"16:9\": \"480x272\""
            "},"
            "\"LDC\": [\"0\"]"
        "},"
        "{"
            "\"Logical Device\":{"
                "\"Name\": \"bayermono\","
                "\"Features\": [\"VSDoF\", \"Denoise\"]"
            "},"
            "\"Sensors\": [ \"IMX386_MIPI_RAW\", \"IMX386_MIPI_MONO\"],"
            "\"Module Type\": 1,"
            "\"Baseline\": 0.95,"
            "\"Depthmap Size\": {"
                "\"4:3\": \"480x360\","
                "\"16:9\": \"480x272\""
            "},"
            "\"LDC\": [\"0\"]"
        "},"
        "{"
            "\"Logical Device\":{"
                "\"Name\": \"activestereo\","
                "\"Features\": [\"VSDoF\"]"
            "},"
            "\"Sensors\": [\"HM1062_2_MIPI_MONO\", \"HM1062_MIPI_MONO\"],"
            "\"Module Type\": 1,"
            "\"Baseline\": 2.5,"
            "\"LDC\": [\"0\"]"
        "},"
        "{"
            "\"Logical Device\":{"
                "\"Name\": \"securecamera\","
                "\"Features\": [\"SecureCamera\"]"
            "},"
            "\"Sensors\": [\"S5K4E6_MIPI_RAW\"]"
        "}"
    "],"
    "\"Callback Buffer List\": {"
        "\"//\": [\"ci: Clean Image, bi: Bokeh Image, mbd: MTK Bokeh Depth, mdb: MTK Debug Buffer, mbm: MTK Bokeh Metadata, rbi: Relighting Bokeh Image\","
           "\"Value must <= 91 bytes\"],"
        "\"Value\": \"bi\""
    "}"
"}";
#endif
