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
package com.mediatek.camera.feature.setting.hdr;

/**
 * Classes implementing this interface will received config update from device.
 */
interface IHdr {
    String MODE_DEVICE_STATE_OPENED = "opened";

    /**
     * Define the mode type to distinguish mode is photo mode
     * or video mode.
     */
    enum HdrModeType {
        SCENE_MODE_DEFAULT,
        ZVHDR_PHOTO,
        MVHDR_PHOTP,
        NONVHDR_PHOTO,
        ZVHDR_VIDEO,
        MVHDR_VIDEO,
        NONVHDR_VIDEO
    }

    /**
     * Classes implementing this interface will be called when contact device to do hdr related
     * things.
     */
    interface Listener {
        /**
         * Notify preview has been started or started or stopped.
         *
         * @param isPreviewStarted true if preview started.Otherwise false.
         */
        void onPreviewStateChanged(boolean isPreviewStarted);

        /**
         * Used to reponse the user change vaule.
         */
        void onHdrValueChanged();

        /**
         * update mode device state to hdr.
         * @param newState the device state.
         */
        void updateModeDeviceState(String newState);

        void setCameraId(int id);

        /**
         * Whether hdr can work together with zsd.
         *
         * @return True if zsd can work with hdr,false otherwise,when false,zsd will be close
         * when hdr on.
         */
        boolean isZsdHdrSupported();

        /**
         * Whether M-Stream HDR supported
         * @return True if M-Stream HDR supported, false otherwise. When supported, make zsd as on
         * when hdr on.
         */
        boolean isMStreamHDRSupported();
    }
}
