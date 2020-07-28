/*
 * Copyright Statement:
 *
 *   This software/firmware and related documentation ("MediaTek Software") are
 *   protected under relevant copyright laws. The information contained herein is
 *   confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *   the prior written permission of MediaTek inc. and/or its licensors, any
 *   reproduction, modification, use or disclosure of MediaTek Software, and
 *   information contained herein, in whole or in part, shall be strictly
 *   prohibited.
 *
 *   MediaTek Inc. (C) 2016. All rights reserved.
 *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *   RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *   ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *   WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *   NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *   RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *   INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *   TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *   RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *   OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *   SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *   RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *   STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *   ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *   RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *   MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *   CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *   The following software/firmware and/or related documentation ("MediaTek
 *   Software") have been modified by MediaTek Inc. All revisions are subject to
 *   any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.common.setting;

import android.hardware.camera2.CaptureRequest;
import android.view.Surface;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.v2.Camera2CaptureSessionProxy;
import com.mediatek.camera.common.bgservice.CaptureSurface;

import javax.annotation.Nonnull;

/**
 * An implementation of SettingDevice2Requester used by setting notify mode.
 */

public class SettingDevice2RequesterProxy implements ISettingManager.SettingDevice2Requester {
    private static final LogUtil.Tag TAG
            = new LogUtil.Tag(SettingDevice2RequesterProxy.class.getSimpleName());

    private ISettingManager.SettingDevice2Requester mModeDevice2RequesterImpl;

    @Override
    public void createAndChangeRepeatingRequest() {
        synchronized (this) {
            if (mModeDevice2RequesterImpl != null) {
                mModeDevice2RequesterImpl.createAndChangeRepeatingRequest();
            }
        }
    }

    @Override
    public CaptureRequest.Builder createAndConfigRequest(int templateType) {
        synchronized (this) {
            if (mModeDevice2RequesterImpl != null) {
                return mModeDevice2RequesterImpl.createAndConfigRequest(templateType);
            }
        }
        return null;
    }

    @Override
    public Camera2CaptureSessionProxy getCurrentCaptureSession() {
        synchronized (this) {
            if (mModeDevice2RequesterImpl != null) {
                return mModeDevice2RequesterImpl.getCurrentCaptureSession();
            }
        }
        return null;
    }

    @Override
    public void requestRestartSession() {
        synchronized (this) {
            if (mModeDevice2RequesterImpl != null) {
                mModeDevice2RequesterImpl.requestRestartSession();
            }
        }
    }

    @Override
    public int getRepeatingTemplateType() {
        synchronized (this) {
            if (mModeDevice2RequesterImpl != null) {
                return mModeDevice2RequesterImpl.getRepeatingTemplateType();
            }
        }
        return -1;
    }

    public void updateModeDevice2Requester(
            @Nonnull ISettingManager.SettingDevice2Requester settingDevice2Requester) {
        synchronized (this) {
            mModeDevice2RequesterImpl = settingDevice2Requester;
        }
    }

    @Override
    public CaptureSurface getModeSharedCaptureSurface() throws IllegalStateException {
        CaptureSurface captureSurface = mModeDevice2RequesterImpl.getModeSharedCaptureSurface();
        return captureSurface;
    }

    @Override
    public Surface getModeSharedPreviewSurface() throws IllegalStateException {
        Surface previewSurface = mModeDevice2RequesterImpl.getModeSharedPreviewSurface();
        return previewSurface;
    }

    @Override
    public Surface getModeSharedThumbnailSurface() throws IllegalStateException {
        Surface thumbnailSurface = mModeDevice2RequesterImpl.getModeSharedThumbnailSurface();
        return thumbnailSurface;
    }
}
