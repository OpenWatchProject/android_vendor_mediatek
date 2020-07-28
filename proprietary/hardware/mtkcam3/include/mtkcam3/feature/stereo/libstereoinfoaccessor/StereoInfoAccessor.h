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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#ifndef STEREO_INFO_ACCESSOR_H
#define STEREO_INFO_ACCESSOR_H

#include <utils/RefBase.h>
#include "IncludeStruct.h"

namespace stereo {

/**
 * StereoInfoAccessor, provide simple utility methods for third application,
 * to read or write some xmp or stereo related information.
 */
class StereoInfoAccessor : public virtual android::RefBase {
public:
    /**
     * write stereo capture info to file buffer.
     * @param captureInfo
     *            StereoCaptureInfo
     * @param [out]jpgBuffer
     */
    void writeStereoCaptureInfo(StereoCaptureInfo &captureInfo, StereoBuffer_t &jpgBuffer);

    /**
     * write stereo depth info to file buffer.
     * @param filePath
     *            file path
     * @param depthInfo
     *            StereoDepthInfo
     */
    void writeStereoDepthInfo(const StereoString &filePath, StereoDepthInfo &depthInfo);

    /**
     * write segment and mask info to file buffer.
     * @param filePath
     *            file path
     * @param maskInfo
     *            SegmentMaskInfo
     */
    void writeSegmentMaskInfo(const StereoString &filePath, SegmentMaskInfo &maskInfo);

    /**
     * write refocus info to file buffer.
     * @param filePath
     *            file path
     * @param configInfo
     *            StereoConfigInfo
     * @param blurImage
     *            buffer
     */
    void writeRefocusImage(const StereoString &filePath, StereoConfigInfo &configInfo,
                                  StereoBuffer_t &blurImage);

    /**
     * Read depth information of stereo image.
     * @param filePath
     *            stereo image file path
     * @return StereoDepthInfo
     */
    StereoDepthInfo* readStereoDepthInfo(const StereoString & filePath);

    /**
     * Read segment mask information of stereo image.
     * @param filePath
     *            stereo image file path
     * @return SegmentMaskInfo
     */
    SegmentMaskInfo* readSegmentMaskInfo(const StereoString & filePath);

    /**
     * Read stereo buffer information of stereo image.
     * @param filePath
     *            stereo image file path
     * @return StereoBufferInfo
     */
    StereoBufferInfo* readStereoBufferInfo(const StereoString & filePath);

    /**
     * Read stereo config information of stereo image.
     * @param filePath
     *            stereo image file path
     * @return SegmentMaskInfo
     */
    StereoConfigInfo* readStereoConfigInfo(const StereoString & filePath);

    /**
     * Read google stereo information of stereo image.
     * @param filePath
     *            stereo image file path
     * @return GoogleStereoInfo
     */
     GoogleStereoInfo* readGoogleStereoInfo(const StereoString & filePath);

    /**
     * Get Geo verify level, debug tool's using.
     * @param configBuffer
     *            json config buffer
     * @return Geo verify level
     */
     int getGeoVerifyLevel(const StereoBuffer_t &configBuffer);

    /**
     * Get Pho verify level, debug tool's using.
     * @param configBuffer
     *            json config buffer
     * @return Pho verify level.
     */
    int getPhoVerifyLevel(const StereoBuffer_t &configBuffer);

    /**
     * Get Cha verify level, debug tool's using.
     * @param configBuffer
     *            json config buffer
     * @return Cha verify level.
     */
    int getMtkChaVerifyLevel(const StereoBuffer_t &configBuffer);

public:
    virtual ~StereoInfoAccessor();

private:
    void serialize(PackInfo &info, IParser &parser);
};

}

#endif