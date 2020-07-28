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

#ifndef _MTK_CUSTOM_PROJECT_HAL_SENINDEPFEATURE_CONFIGFTBL_H_
#define _MTK_CUSTOM_PROJECT_HAL_SENINDEPFEATURE_CONFIGFTBL_H_
#if 1

/*******************************************************************************
 *
 ******************************************************************************/
#define CUSTOM_SENINDEPFEATURE   "senindepfeature"
FTABLE_DEFINITION(CUSTOM_SENINDEPFEATURE)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FTABLE_SCENE_INDEP()
//==========================================================================

#if 1
    //  3DNR ON/OFF
#if (1 == NR3D_SUPPORTED)
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_3DNR_MODE), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::ON), 
            ITEM_AS_VALUES_(            
                MtkCameraParameters::ON,
                MtkCameraParameters::OFF
            )
        ), 
    )
#else
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_3DNR_MODE), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::OFF), 
            ITEM_AS_VALUES_(
                MtkCameraParameters::OFF
            )
        ), 
    )
#endif
#endif
//==========================================================================

    //  Video Face Beauty
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_VIDEO_FACE_BEAUTY_SUPPORTED), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::TRUE), 
            ITEM_AS_VALUES_(            
                MtkCameraParameters::TRUE
            )
        ), 
    )

    // Extend strength tuning to 25 levels
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_FB_SMOOTH_LEVEL_MIN), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("-12"), 
            ITEM_AS_VALUES_(            
                "-12"
            )
        ), 
    )
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_FB_SMOOTH_LEVEL_MAX), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("12"), 
            ITEM_AS_VALUES_(            
                "12"
            )
        ), 
    )
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_FB_SKIN_COLOR_MIN), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("-12"), 
            ITEM_AS_VALUES_(            
                "-12"
            )
        ), 
    )
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_FB_SKIN_COLOR_MAX), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("12"), 
            ITEM_AS_VALUES_(            
                "12"
            )
        ), 
    )
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_FB_SHARP_MIN), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("0"), 
            ITEM_AS_VALUES_(            
                "0"
            )
        ), 
    )
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_FB_SHARP_MAX), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("0"), 
            ITEM_AS_VALUES_(            
                "0"
            )
        ), 
    )
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_FB_SLIM_FACE_MIN), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("-12"), 
            ITEM_AS_VALUES_(            
                "-12"
            )
        ), 
    )
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_FB_SLIM_FACE_MAX), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_("12"), 
            ITEM_AS_VALUES_(            
                "12"
            )
        ), 
    )

//==========================================================================

#if 1
    //  Gesture Shot
#if (1 == GESTURE_SHOT_SUPPORTED)    
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_SUPPORTED(
        KEY_AS_(MtkCameraParameters::KEY_GESTURE_SHOT), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::FALSE), 
            ITEM_AS_SUPPORTED_(           
                MtkCameraParameters::TRUE            
            )
        ), 
    )
#else
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_SUPPORTED(
        KEY_AS_(MtkCameraParameters::KEY_GESTURE_SHOT), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::FALSE), 
            ITEM_AS_SUPPORTED_(            
                MtkCameraParameters::FALSE
            )
        ), 
    )
#endif
#endif
//==========================================================================

#if 1
    //  Native PIP ON/OFF
#if (1 == PIP_SUPPORTED)
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_SUPPORTED(
        KEY_AS_(MtkCameraParameters::KEY_NATIVE_PIP), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::FALSE), 
            ITEM_AS_SUPPORTED_(           
                MtkCameraParameters::TRUE            
            )
        ), 
    )
#else
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_SUPPORTED(
        KEY_AS_(MtkCameraParameters::KEY_NATIVE_PIP), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::FALSE), 
            ITEM_AS_SUPPORTED_(            
                MtkCameraParameters::FALSE
            )
        ), 
    )
#endif
#endif
//==========================================================================

#if 0
    //  Depth-based AF ON/OFF
#if (1 == DEPTH_AF_SUPPORTED)
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_STEREO_DEPTHAF_MODE), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::OFF), 
            ITEM_AS_VALUES_(            
                MtkCameraParameters::ON,
                MtkCameraParameters::OFF
            )
        ), 
    )
#else
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_STEREO_DEPTHAF_MODE), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::OFF), 
            ITEM_AS_VALUES_(
                MtkCameraParameters::OFF
            )
        ), 
    )
#endif
#endif
//==========================================================================

#if 0
    //  Distance Measurement ON/OFF
#if (1 == DISTANCE_MEASURE_SUPPORTED)
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_STEREO_DISTANCE_MODE), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::OFF), 
            ITEM_AS_VALUES_(            
                MtkCameraParameters::ON,
                MtkCameraParameters::OFF
            )
        ), 
    )
#else
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_STEREO_DISTANCE_MODE), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::OFF), 
            ITEM_AS_VALUES_(
                MtkCameraParameters::OFF
            )
        ), 
    )
#endif
#endif
//==========================================================================

#if 0
    //  STEREO REFOCUS ON/OFF
#if (1 == IMAGE_REFOCUS_SUPPORTED)
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_STEREO_REFOCUS_MODE), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::OFF), 
            ITEM_AS_VALUES_(            
                MtkCameraParameters::ON,
                MtkCameraParameters::OFF
            )
        ), 
    )
#else
    FTABLE_CONFIG_AS_TYPE_OF_DEFAULT_VALUES(
        KEY_AS_(MtkCameraParameters::KEY_STEREO_REFOCUS_MODE), 
        SCENE_AS_DEFAULT_SCENE(
            ITEM_AS_DEFAULT_(MtkCameraParameters::OFF), 
            ITEM_AS_VALUES_(
                MtkCameraParameters::OFF
            )
        ), 
    )
#endif
#endif
//==========================================================================
END_FTABLE_SCENE_INDEP()
//------------------------------------------------------------------------------
END_FTABLE_DEFINITION()


#endif
#endif //_MTK_CUSTOM_PROJECT_HAL_CAMERASHOT_CONFIGFTBLSHOT_H_

