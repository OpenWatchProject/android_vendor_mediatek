/*
 *   Copyright Statement:
 *
 *     This software/firmware and related documentation ("MediaTek Software") are
 *     protected under relevant copyright laws. The information contained herein is
 *     confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *     the prior written permission of MediaTek inc. and/or its licensors, any
 *     reproduction, modification, use or disclosure of MediaTek Software, and
 *     information contained herein, in whole or in part, shall be strictly
 *     prohibited.
 *
 *     MediaTek Inc. (C) 2016. All rights reserved.
 *
 *     BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *    THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *     RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *     ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *     WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *     WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *     NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *     RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *     TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *     RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *     OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *     SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *     RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *     STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *     ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *     RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *     MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *     CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     The following software/firmware and/or related documentation ("MediaTek
 *     Software") have been modified by MediaTek Inc. All revisions are subject to
 *     any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.camera.common.mode.video.videoui;

import android.view.View;

import javax.annotation.Nonnull;
/**
 * interface IVideoUi use for video UI.
 * other platform or other feature which is video relate can
 * implement it to realize new UI.
 */

public interface IVideoUI {
    int VIDEO_BAD_PERFORMANCE_AUTO_STOP = 1;
    int VIDEO_RECORDING_NOT_AVAILABLE = 2;
    int VIDEO_RECORDING_ERROR = 4;
    int REACH_SIZE_LIMIT = 5;
    /**
     * VideoUIState provide five state used to update video ui.
     */
    enum VideoUIState {
        /**
         * usually used for video preview
         * and the implement will use it to show or hide recording ui.
         * for video ui if state is STATE_PREVIEW then will see video normal
         * preview.
         */
        STATE_PREVIEW,
        /**
         * usually used after video shutter button click and before real recording.
         * to show recording time 00:00 for performance, vss/pause icon will show
         * if they are supported.and before set STATE_PRE_RECORDING the ui state must
         * be STATE_PREVIEW or will throw IllegalStateException.
         */
        STATE_PRE_RECORDING,
        /**
         * used for to show recording ui.
         * in this state recording time will update
         * before set STATE_RECORDING the ui state must be STATE_PRE_RECORDING
         * or will throw throw IllegalStateException.
         */
        STATE_RECORDING,
        /**
         * used for to show pause ui.
         * in this state pause icon will change to resume icon.
         * recording time will not update
         * before set STATE_PAUSE_RECORDING the ui state must be STATE_RECORDING
         * or will throw IllegalStateException.
         */
        STATE_PAUSE_RECORDING,
        /**
         * used for resume recording.
         * this should call after STATE_PAUSE_RECORDING
         * recording will update and video ui will same with STATE_RECORDING.
         * before set STATE_RESUME_RECORDING the ui state must be STATE_PAUSE_RECORDING
         * or will throw IllegalStateException.
         */
        STATE_RESUME_RECORDING,
        /**
         * used for burst recording.
         * this should call after STATE_RECORDING, in this state pause/stop/burst icon is disabled
         */
        STATE_BURST_RECORDING,
        /**
         * used for resume normal recording after burst recording.
         * this should call after STATE_RESUME_RECORDING_AFTER_BURST.
         * recording will update and video ui will same with STATE_RECORDING.
         */
        STATE_RESUME_RECORDING_AFTER_BURST,
    }

    /**
     * use spec to config video ui.
     * @param spec config video ui
     */
    void initVideoUI(@Nonnull UISpec spec);

    /**
     * video feature use updateUIState to control video ui.
     * and the state is only use VideoUiState.
     * @param state the state use to control video ui.
     */
    void updateUIState(VideoUIState state);

    /**
     * update orientation to video ui,maybe use to rotate.
     * @param orientation the orientation want to set,the value only supported
     * 0,90,180,270 and other value will throw IllegalArgumentException.
     */
    void updateOrientation(int orientation);

    /**
     * the function mainly use for mms to recording video who
     * want to know the current record file size.
     * use the function the ui spec.recordingTotalSize must greater than zero and
     * the size must in range [0,recordingTotalSize] otherwise will
     * throw IllegalArgumentException.
     * @param size current file size unit is byte
     */
    void updateRecordingSize(long size);

    /**
     * enable burst icon after complete burst recording processing
     */
    void enableBurst();

    /**
     * unInitializeVideoUI video ui.
     * means remove video ui from root view.
     */
    void unInitVideoUI();

    /**
     * this function used to show info when recording.
     * @param infoId the info id which tell implement show which info.
     */
    void showInfo(int infoId);
    /**
     * the class UISpec is used for video feature to config Video ui.
     * the fields can be extend by future features.
     * each instance of IVideoUI will just care about what it will use of the fields.
     */
    static class UISpec {
        /**
         * isSupportedVss used for decide whether need show video snap shot button.
         * default value is false means not show it.
         */
        public boolean isSupportedVss = false;
        /**
         * isSupportedPause used for decide whether need show pause/resume button.
         * default value is false means not show it.
         */
        public boolean isSupportedPause = false;

        /**
         * isSupportedBurst is used for decide whether need show burst button.
         * isSupportedVss and isSupportedBurst is not allowed to be true at the same time, or else
         * UI will show abnormally.
         * default value is false meas not show it.
         */
        public boolean isSupportedBurst = false;

        /**
         * recordingTotalSize used to set progress when recording.
         * default value is 0 means not show it, and if it greater than zero means
         * should show recording progress and total size in recording ui.
         * if it negative will not show ui and not deal it.
         * the unit is byte.
         */
        public long recordingTotalSize = 0;

        /**
         * this is used for monitor ui's vss icon click
         * and notify those who want know it.
         */
        public View.OnClickListener vssListener = null;

        /**
         * this is used for monitor ui's burst icon click
         * and notify those who want know it.
         */
        public View.OnClickListener burstListener = null;

        /**
         * this is used for monitor ui's pause resuem icon click
         * and notify those who want know it.
         */
        public View.OnClickListener pauseResumeListener = null;

        /**
         * this is used for monitor ui's shutter button click
         * and notify those who want know it.
         */
        public View.OnClickListener stopListener = null;

        /**
         * used for burst recording.
         * it means the normal recording fps in burst mode.
         */
        public int normalFps = 0;

        /**
         * used for burst recording.
         * it means the burst recording fps in burst mode.
         */
        public int burstFps = 0;

    }
}
