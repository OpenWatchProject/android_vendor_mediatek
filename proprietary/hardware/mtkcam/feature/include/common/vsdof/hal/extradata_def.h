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
#ifndef EXTRA_DATA_DEF_H_
#define EXTRA_DATA_DEF_H_

//    "JPS_size": {
//        "width": 4352,
//        "height": 1152
//    },
#define EXTRA_DATA_JPS_SIZE "JPS_size"
#define EXTRA_DATA_WIDTH "width"
#define EXTRA_DATA_HEIGHT "height"

//    "output_image_size" : {
//        "width": 2176,
//        "height": 1152
//    },
#define EXTRA_DATA_OUTPUT_IMAGE_SIZE "output_image_size"

//    "main_cam_align_shift" : {
//        "x": 30,
//        "y": 10
//    },
#define EXTRA_DATA_MAIN_CAM_ALIGN_SHIFT "main_cam_align_shift"
#define EXTRA_DATA_X "x"
#define EXTRA_DATA_Y "y"

//    "input_image_size": {
//        "width": 1920,
//        "height": 1080
//    },
#define EXTRA_DATA_INPUT_IMAGE_SIZE "input_image_size"

//    "capture_orientation": {
//        "orientations_values": ["0: none", "1: flip_horizontal", "2: flip_vertical", "4: 90", "3: 180", "7: 270"],
//        "orientation": 0
//    },
#define EXTRA_DATA_CAPTURE_ORIENTATION "capture_orientation"
#define EXTRA_DATA_ORIENTATIONS_VALUES "orientations_values"
#define EXTRA_DATA_ORIENTATION "orientation"

//    "depthmap_orientation": {
//        "depthmap_orientation_values": ["0: none", "90: 90 degrees CW", "180: 180 degrees CW", "270: 270 degrees CW"],
//        "orientation": 0
//    },
#define EXTRA_DATA_DEPTHMAP_ORIENTATION "depthmap_orientation"
#define EXTRA_DATA_DEPTHMAP_ORIENTATION_VALUES "depthmap_orientation_values"

//    "sensor_relative_position": {
//        "relative_position_values": ["0: main-minor", "1: minor-main"],
//        "relative_position": 0
//    },
#define EXTRA_DATA_SENSOR_RELATIVE_POSITION "sensor_relative_position"
#define EXTRA_DATA_RELATIVE_POSITION_VALUES "relative_position_values"
#define EXTRA_DATA_RELATIVE_POSITION "relative_position"

//    "focus_roi": {
//        "top": 0,
//        "left": 10,
//        "bottom": 10,
//        "right": 30
//    },
#define EXTRA_DATA_FOCUS_ROI "focus_roi"
#define EXTRA_DATA_TOP "top"
#define EXTRA_DATA_LEFT "left"
#define EXTRA_DATA_BOTTOM "bottom"
#define EXTRA_DATA_RIGHT "right"

//    "focus_info": {
//        "is_face": 0,
//        "face_ratio": 0.07,
//        "dac_cur": 200,
//        "dac_min": 100,
//        "dac_max": 300,
//        "focus_type": 3
//    }
#define EXTRA_DATA_FOCUS_INFO   "focus_info"
#define EXTRA_DATA_IS_FACE "is_face"
#define EXTRA_DATA_FACE_RATIO "face_ratio"
#define EXTRA_DATA_DAC_CUR "dac_cur"
#define EXTRA_DATA_DAC_MIN "dac_min"
#define EXTRA_DATA_DAC_MAX "dac_max"
#define EXTRA_DATA_FOCUS_TYPE "focus_type"

//    "verify_geo_data": {
//        "quality_level_values": ["0: PASS","1: Cond.Pass","2: FAIL"],
//        "quality_level": 0,
//        "statistics": [0,0,0,0,0,0]
//    },
#define EXTRA_DATA_VERIFY_GEO_DATA "verify_geo_data"
#define EXTRA_DATA_QUALITY_LEVEL_VALUES "quality_level_values"
#define EXTRA_DATA_QUALITY_LEVEL "quality_level"
#define EXTRA_DATA_STATISTICS "statistics"

//    "verify_pho_data": {
//        "quality_level_values": ["0: PASS","1: Cond.Pass","2: FAIL"],
//        "quality_level": 0,
//        "statistics": [0,0,0,0]
//    },
#define EXTRA_DATA_VERIFY_PHO_DATA "verify_pho_data"
#define EXTRA_DATA_QUALITY_LEVEL_VALUES "quality_level_values"
#define EXTRA_DATA_QUALITY_LEVEL "quality_level"

//    "verify_mtk_cha": {
//        "check_values": ["0: PASS","1: FAIL"],
//        "check": 0,
//        "score": 0,
//        "distance": 0
//    },
#define EXTRA_DATA_VERIFY_MTK_CHA "verify_mtk_cha"
#define EXTRA_DATA_CHECK_VALUES "check_values"
#define EXTRA_DATA_CHECK "check"
#define EXTRA_DATA_SCORE "score"
#define EXTRA_DATA_DISTANCE "distance"

//    "depth_buffer_size": {
//        "width": 480,
//        "height": 270
//    },
#define EXTRA_DATA_DEPTH_BUFFER_SIZE "depth_buffer_size"

//    "ldc_size": {
//        "width": 272,
//        "height": 144
//    },
#define EXTRA_DATA_LDC_SIZE "ldc_size"

//    "GFocus": {
//        "BlurAtInfinity": 0.014506519,
//        "FocalDistance": 25.612852,
//        "FocalPointX": 0.5,
//        "FocalPointY": 0.5
//    },
#define EXTRA_DATA_GFOCUS "GFocus"
#define EXTRA_DATA_BLURATINFINITY "BlurAtInfinity"
#define EXTRA_DATA_FOCALDISTANCE "FocalDistance"
#define EXTRA_DATA_FOCALPOINTX "FocalPointX"
#define EXTRA_DATA_FOCALPOINTY "FocalPointY"

//    "GImage" : {
//        "Mime": "image/jpeg"
//    },
#define EXTRA_DATA_GIMAGE "GImage"
#define EXTRA_DATA_MIME "Mime"

//    "GDepth": {
//        "Format": "RangeInverse",
//        "Near": 15.12828254699707,
//        "Far": 97.0217514038086,
//        "Mime": "image/png"
//    },
#define EXTRA_DATA_GDEPTH "GDepth"
#define EXTRA_DATA_FORMAT "Format"
#define EXTRA_DATA_NEAR "Near"
#define EXTRA_DATA_FAR "Far"

//    "mask_info" : {
//        "width":2176,
//        "height":1152,
//        "mask description": "Data(0xFF), format: [offset,length]",
//        "mask": [[28,1296],[1372,1296],[2716,1296],...]
//    },
#define EXTRA_DATA_MASK_INFO "mask_info"
#define EXTRA_DATA_MASK_DESCRIPTION "mask description"
#define EXTRA_DATA_MASK "mask"

//    "face_detections" : {
//        "left":12
//        "top":34
//        "right":56
//        "bottom":78,
//        "rotation-in-plane":9
//    },
#define EXTRA_DATA_FACE_DETECTIONS "face_detections"
#define EXTRA_DATA_ROTATION_IN_PLANE "rotation-in-plane"

//    {"dof_level":8},
#define EXTRA_DATA_DOF_LEVEL "dof_level"

//    "buffer_list": [
//        {
//          "name": "MV_Y",
//          "image_format": "Y8",
//          "image_size": "480x272",
//          "buffer_size": 130560
//        },
//        {
//          "name": "SV_Y",
//          "image_format": "Y8",
//          "image_size": "480x272",
//          "buffer_size": 130560
//        },
//        {
//          "name": "MASK_M",
//          "image_format": "Y8",
//          "image_size": "480x272",
//          "buffer_size": 130560
//        },
//        {
//          "name": "MASK_S",
//          "image_format": "Y8",
//          "image_size": "480x272",
//          "buffer_size": 130560
//        }
//      ]
#define EXTRA_DATA_BUFFER_LIST "buffer_list"
#define EXTRA_DATA_NAME "name"
#define EXTRA_DATA_IMAGE_FORMAT "image_format"
#define EXTRA_DATA_IMAGE_SIZE "image_size"
#define EXTRA_DATA_BUFFER_SIZE "buffer_size"
#define EXTRA_DATA_COMPRESSED "compressed"

#endif  // EXTRA_DATA_DEF_H_