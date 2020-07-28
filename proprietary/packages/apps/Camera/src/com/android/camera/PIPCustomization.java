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
 * MediaTek Inc. (C) 2014. All rights reserved.
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
package com.android.camera;

public class PIPCustomization {
    private static final String TAG = "PIPCustomization";
    public static final String MAIN_CAMERA = "main_camera";
    public static final String SUB_CAMERA = "sub_camera";
    // scale
    public static final float TOP_GRAPHIC_MAX_SCALE_VALUE = 1.8f;
    public static final float TOP_GRAPHIC_MIN_SCALE_VALUE = 0.6f;
    // rotate
    public static final float TOP_GRAPHIC_MAX_ROTATE_VALUE = 180f;
    // top graphic edge, default is min(width, height) / 2
    public static final float TOP_GRAPHIC_DEFAULT_EDGE_RELATIVE_VALUE = 1f / 2;
    // edit button edge, default is min(width, height) / 10
    public static final int TOP_GRAPHIC_EDIT_BUTTON_RELATIVE_VALUE = 10;
    public static final float TOP_GRAPHIC_LEFT_TOP_RELATIVE_VALUE =
            TOP_GRAPHIC_DEFAULT_EDGE_RELATIVE_VALUE - 1f / TOP_GRAPHIC_EDIT_BUTTON_RELATIVE_VALUE;
    // top graphic crop preview position
    public static final float TOP_GRAPHIC_CROP_RELATIVE_POSITION_VALUE = 3f / 4;
    // which camera enable FD, default is main camera support fd
    public static final String ENABLE_FACE_DETECTION = MAIN_CAMERA;
    // when take picture, whether sub camera need mirror
    public static final boolean SUB_CAMERA_NEED_HORIZONTAL_FLIP = true;

    private PIPCustomization() {
    }

    public static boolean isMainCameraEnableFD() {
        boolean enable = false;
        enable = ENABLE_FACE_DETECTION.endsWith(MAIN_CAMERA);
        Log.i(TAG, "isMainCameraEnableFD enable = " + enable);
        return enable;
    }
}
