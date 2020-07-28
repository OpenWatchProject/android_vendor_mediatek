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

package com.mediatek.op12.ims;

import android.telecom.Connection;
import android.telecom.VideoProfile;
import android.util.Log;
// for async the action to anthor thread
import com.android.internal.os.SomeArgs;

import com.mediatek.ims.internal.ImsVTProvider;
import com.mediatek.ims.internal.ImsVTProviderUtil;

public class Op12ImsVTProvider extends ImsVTProvider {
    private static final String                 TAG = "Op12ImsVT";

    private boolean                             mNeedSendPause = false;
    private boolean                             mNeedSendResume = false;
    private VideoProfile                        mPauseResumeProfile = null;
    private boolean                             mIsResumedDuringBG = false;

    public Op12ImsVTProvider() {
        super();
        Log.d(TAG, "Create Op12ImsVTProvider done");
    }

    @Override
    protected void sendFgBgSessionModifyRequestInternal(VideoProfile fromProfile, VideoProfile toProfile, int mode, boolean needNotify) {

        Log.d(TAG, "[ID=" + mId + "] [sendFgBgSessionModifyRequestInternal] Start mode = " + mode);
        Log.d(TAG, "[ID=" + mId + "] [sendFgBgSessionModifyRequestInternal] fromProfile:" + fromProfile.toString());
        Log.d(TAG, "[ID=" + mId + "] [sendFgBgSessionModifyRequestInternal] toProfile:" + toProfile.toString());

        super.sendFgBgSessionModifyRequestInternal(fromProfile, toProfile, mode, false);

        // Don't send pause when Data off/Roaming/MA crashed downgrade.
        if (isDuringNotAppDowngrade()) {
            Log.d(TAG, "[ID=" + mId + "] [sendFgBgSessionModifyRequestInternal] During Not App Downgrade");
            return;
        }

        if (mode == ImsVTProviderUtil.UI_MODE_BG) {
            mNeedSendPause = true;
            mPauseResumeProfile = toProfile;
        } else if (mode == ImsVTProviderUtil.UI_MODE_FG) {
            // Pause haven't sent yet, just cancel it.
            if (mNeedSendPause) {
                mNeedSendPause = false;
                mPauseResumeProfile = null;
                Log.d(TAG, "[ID=" + mId + "] [sendFgBgSessionModifyRequestInternal] Cancel send Pause");
                return;
            }
            mNeedSendResume = true;
            mPauseResumeProfile = toProfile;
        }

        // If set Bg/Fg during session modify, the pause/resume will be sent after session modify complete.
        if (isDuringSessionModify()) {
            Log.d(TAG, "[ID=" + mId + "] [sendFgBgSessionModifyRequestInternal] Return due to during session modify");
            return;
        } else {
            sendBgPauseOrFgResume();
        }

        Log.d(TAG, "[ID=" + mId + "] [sendFgBgSessionModifyRequestInternal] Finish");
    }

    // Local session modify done
    @Override
    public void receiveSessionModifyResponseInternal(int status, VideoProfile requestedProfile, VideoProfile responseProfile) {
        super.receiveSessionModifyResponseInternal(status, requestedProfile, responseProfile);

        if (status == Connection.VideoProvider.SESSION_MODIFY_REQUEST_SUCCESS) {
            mCurrentProfile = responseProfile;
        }
        sendBgPauseOrFgResume();
    }

    // Remote session modify done
    @Override
    public void sendSessionModifyResponseInternal(VideoProfile responseProfile) {
        super.sendSessionModifyResponseInternal(responseProfile);

        mCurrentProfile = responseProfile;
        sendBgPauseOrFgResume();
    }

    @Override
    public void notifyResume() {
        if (mMode == ImsVTProviderUtil.UI_MODE_BG) {
            // This flag is used to notify resume when in BG
            // In this case InCallUI will send unPaused when call swap
            Log.d(TAG, "[ID=" + mId + "] [notifyResume] Receive resume when in BG");
            mIsResumedDuringBG = true;
        }
    }

    private void sendBgPauseOrFgResume () {
        Log.d(TAG, "[ID=" + mId + "] [sendBgPauseOrFgResume] Start mIsAudioCall: " + mIsAudioCall);
        Log.d(TAG, "[ID=" + mId + "] [sendBgPauseOrFgResume] mPauseResumeProfile:" + mPauseResumeProfile);

        if (mIsAudioCall) {
            mPauseResumeProfile = null;
            mNeedSendPause = false;
            mNeedSendResume = false;
            Log.d(TAG, "[ID=" + mId + "] [sendBgPauseOrFgResume] return due to mIsAudioCall: " + mIsAudioCall);
            return;
        }

        if (mPauseResumeProfile == null) {
            Log.d(TAG, "[ID=" + mId + "] [sendBgPauseOrFgResume] return due to mPauseResumeProfile: " + mPauseResumeProfile);
            return;
        }

        Log.d(TAG, "[ID=" + mId + "] [sendBgPauseOrFgResume] mMode: " + mMode + " mNeedSendPause: " + mNeedSendPause
                + " mNeedSendResume: " + mNeedSendResume + " mCurrentProfile: " + mCurrentProfile);
        if (ImsVTProviderUtil.UI_MODE_BG == mMode && mNeedSendPause) {
            VideoProfile pauseProfile = mPauseResumeProfile;
            if (mCurrentProfile != null) {
                int realToState = mCurrentProfile.getVideoState();
                realToState |= VideoProfile.STATE_PAUSED;
                pauseProfile = new VideoProfile(realToState, pauseProfile.getQuality());
            }
            setDuringSessionRequest(true);
            mLastRequestVideoProfile = pauseProfile;
            nRequestPeerConfig(mId, mVTProviderUtil.packFromVdoProfile(pauseProfile));
            mNeedSendPause = false;
            mPauseResumeProfile = null;

        } else if (ImsVTProviderUtil.UI_MODE_FG == mMode && mNeedSendResume) {
            VideoProfile resumeProfile = mPauseResumeProfile;
            if (mCurrentProfile != null) {
                int realToState = mCurrentProfile.getVideoState();
                realToState &= ~VideoProfile.STATE_PAUSED;
                resumeProfile = new VideoProfile(realToState, resumeProfile.getQuality());
            }

            // In VzW request, we need to send different direction when FG
            // Case 1:
            //  If it is triggered by swap call,
            //  because remote phone may change direction when we in BG (UA won't post to AP),
            //  so we need use the latest state given by call session proxy
            // Case 2:
            //  If it is simple BG/FG switch, we turn off camera in past
            //  we need to send Rx to present FG
            // Case 3:
            //  If it is simple BG/FG switch, we need to send TxRx to present FG
            //
            // So we need to known if this FG is cause by call swap
            // and we also need the latest state as well.
            if (mIsResumedDuringBG) {
                Log.d(TAG, "[ID=" + mId + "] [sendBgPauseOrFgResume] No need to add Rx when in call swap");
                mIsResumedDuringBG = false;

            } else {
                int toState = resumeProfile.getVideoState();
                if (!VideoProfile.isReceptionEnabled(toState)) {
                    int realToState = (toState | VideoProfile.STATE_RX_ENABLED);

                    resumeProfile = new VideoProfile(
                                                realToState,
                                                resumeProfile.getQuality());
                }
            }

            setDuringSessionRequest(true);
            mLastRequestVideoProfile = resumeProfile;
            nRequestPeerConfig(mId, mVTProviderUtil.packFromVdoProfile(resumeProfile));
            mNeedSendResume = false;
            mPauseResumeProfile = null;
        }
        Log.d(TAG, "[ID=" + mId + "] [sendBgPauseOrFgResume] Finish");
    }
}
