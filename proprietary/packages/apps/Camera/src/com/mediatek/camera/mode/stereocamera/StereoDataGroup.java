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

package com.mediatek.camera.mode.stereocamera;

/**
 * A stereo data group containing stereo Jps/Clear image/
 * Mask/Ldc/DepthMap.
 *
 */
public class StereoDataGroup {
    private static final String TAG = "StereoDataGroup";

    private String mPictureName;
    private byte[] mJpsData;
    private byte[] mMaskAndConfigData;
    private byte[] mDepthMap;
    private byte[] mClearImage;
    private byte[] mLdcData;
    private byte[] mDepthWrapper;
    private byte[] mN3dDebugData;
    private byte[] mOriginalJpegData;

    /**
     * Create Stereo data group object.
     * @param pictureName picture name
     * @param originalJpegData originalJpegData
     * @param jpsData jps Data
     * @param maskData mask Data
     * @param depthData depth Data
     * @param clearImage clear Image
     * @param ldcData ldc Data
     * @param n3dDebugData n3d debug data
     * @param depthWrapper depth wrapper
     */
    public StereoDataGroup(String pictureName, byte[] originalJpegData,
            byte[] jpsData, byte[] maskData, byte[] depthData,
            byte[] clearImage, byte[] ldcData, byte[] n3dDebugData, byte[] depthWrapper) {
        mPictureName = pictureName;
        mOriginalJpegData = originalJpegData;
        mJpsData = jpsData;
        mMaskAndConfigData = maskData;
        mDepthMap = depthData;
        mClearImage = clearImage;
        mLdcData = ldcData;
        mN3dDebugData = n3dDebugData;
        mDepthWrapper = depthWrapper;
    }

    /**
     * Get picture name.
     * @return picture name
     */
    public String getPictureName() {
       return mPictureName;
    }

    /**
     * Get jps data.
     * @return jps data
     */
    public byte[] getJpsData() {
        return mJpsData;
    }

    /**
     * Get mask data.
     * @return mask data
     */
    public byte[] getMaskAndConfigData() {
        return mMaskAndConfigData;
    }

    /**
     * Get depth map.
     * @return depth data
     */
    public byte[] getDepthMap() {
        return mDepthMap;
    }

    /**
     * Get clear image.
     * @return clear data
     */
    public byte[] getClearImage() {
        return mClearImage;
    }

    /**
     * Get ldc data.
     * @return ldc data
     */
    public byte[] getLdcData() {
        return mLdcData;
    }

    /**
     * Get Original data.
     * @return Original data
     */
    public byte[] getOriginalJpegData() {
        return mOriginalJpegData;
    }


    /**
     * Get n3d debug data.
     * @return n3d debug data
     */
    public byte[] getN3dDebugData() {
        return mN3dDebugData;
    }

    /**
     * Get depth wrapper data.
     * @return depth wrapper debug data
     */
    public byte[] getDepthWrapper() {
        return mDepthWrapper;
    }
}