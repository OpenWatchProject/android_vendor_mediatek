/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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
 */

package com.mediatek.incallui.video;

import android.os.SystemClock;

import android.telecom.VideoProfile;

import com.android.dialer.common.LogUtil;
import com.android.incallui.call.DialerCall;
import com.android.incallui.call.CallList;
import com.android.incallui.videotech.VideoTech;
import com.android.incallui.videotech.utils.SessionModificationState;
import com.android.incallui.InCallPresenter;
import com.android.incallui.Log;
import com.mediatek.incallui.CallTimer;

import com.mediatek.incallui.plugin.ExtensionManager;
import mediatek.telephony.MtkCarrierConfigManager;

/**
 * M: [Video Call] A helper to downgrade video call if necessary.
 * Especially downgrade when UI in background or quit.
 */
public class VideoSessionController implements InCallPresenter.InCallStateListener,
    InCallPresenter.IncomingCallListener {
  private static final boolean DEBUG = true;
  private static final int DEFAULT_COUNT_DOWN_SECONDS = 20;
  ///M:add for upgrade recevied timeout
  private static final int COUNT_DOWN_SECONDS_FOR_RECEVICE_UPGRADE_WITH_PRECONDITION = 15;
  private static final long MILLIS_PER_SECOND = 1000;
  private static VideoSessionController sInstance;
  private InCallPresenter inCallPresenter;
  private DialerCall primaryCall;
  private AutoDeclineTimer autoDeclineTimer = new AutoDeclineTimer();

  //M:the event to start timer of cancel upgrade
  public static final int SESSION_EVENT_NOTIFY_START_TIMER_20S = 1013;

  private VideoSessionController() {
        // do nothing
  }

  /**
   * M: get the VideoSessionController instance.
   * @return the instance.
   */
  public static VideoSessionController getInstance() {
    if (sInstance == null) {
      sInstance = new VideoSessionController();
    }
    return sInstance;
  }

  /**
   * M: setup when InCallPresenter setUp.
   * @param inCallPresenter the InCallPresenter instance.
   */
  public void setUp(InCallPresenter presenter) {
    LogUtil.d("VideoSessionController.setUp", null);
    if (presenter == null) {
      return;
    }
    inCallPresenter = presenter;
    inCallPresenter.addListener(this);
    inCallPresenter.addIncomingCallListener(this);
  }

  /**
   * M: tearDown when InCallPresenter tearDown.
   */
  public void tearDown() {
    LogUtil.d("VideoSessionController.tearDown", null);
    inCallPresenter.removeListener(this);
    inCallPresenter.removeIncomingCallListener(this);

    clear();
  }

  /**
   * M: get the countdown second number.
   * @return countdown number.
   */
  public long getAutoDeclineCountdownSeconds() {
    return autoDeclineTimer.getAutoDeclineCountdown();
  }

  @Override
  public void onStateChange(InCallPresenter.InCallState oldState,
      InCallPresenter.InCallState newState, CallList callList) {
    DialerCall call;
    if (newState == InCallPresenter.InCallState.INCOMING) {
      call = callList.getIncomingCall();
    } else if (newState == InCallPresenter.InCallState.WAITING_FOR_ACCOUNT) {
      call = callList.getWaitingForAccountCall();
    } else if (newState == InCallPresenter.InCallState.PENDING_OUTGOING) {
      call = callList.getPendingOutgoingCall();
    } else if (newState == InCallPresenter.InCallState.OUTGOING) {
      call = callList.getOutgoingCall();
    } else {
      call = callList.getActiveOrBackgroundCall();
    }

    if (!DialerCall.areSame(call, primaryCall)) {
      onPrimaryCallChanged(call);
    }
  }

  @Override
  public void onIncomingCall(InCallPresenter.InCallState oldState,
      InCallPresenter.InCallState newState, DialerCall call) {
      if (!DialerCall.areSame(call, primaryCall)) {
        onPrimaryCallChanged(call);
      }
  }

  /**
   * M: When upgrade request received, start timing.
   * @param call the call upgrading.
   */
  public void startTimingForAutoDecline(DialerCall call) {
    LogUtil.d("VideoSessionController.startTimingForAutoDecline", "for call: " + getId(call));
    if (!DialerCall.areSame(call, primaryCall)) {
      LogUtil.e("VideoSessionController.startTimingForAutoDecline", "Abnormal case for a" +
            "non-primary call receiving upgrade request.");
      onPrimaryCallChanged(call);
    }
    if (call.getVideoFeatures().isSupportAutoDeclineUpgradeRequest()) {
      autoDeclineTimer.startTiming();
    }
  }

  /**
   * M: stop timing when the request accepted or declined.
   */
  public void stopTiming() {
    autoDeclineTimer.stopTiming();
  }

  private void onPrimaryCallChanged(DialerCall call) {
    LogUtil.d("VideoSessionController.onPrimaryCallChanged", "change : " + getId(primaryCall) +
        " -> " + getId(call));
    if (call != null && primaryCall != null && primaryCall.getVideoTech().
        getSessionModificationState()
        == SessionModificationState.RECEIVED_UPGRADE_TO_VIDEO_REQUEST) {
      /**
       * force decline upgrade request if primary call changed.
        */
      inCallPresenter.declineUpgradeRequest(inCallPresenter.getContext());
    }
    primaryCall = call;
  }

  private void clear() {
    inCallPresenter = null;
    // when mInCallPresenter is null ,eg peer disconnect call,
    // local should stop timer.
    stopTiming();
  }

  private void logd(String msg) {
    if (DEBUG) {
      Log.d(this, msg);
    }
  }

  private void logw(String msg) {
    if (DEBUG) {
      Log.w(this, msg);
    }
  }

  private void logi(String msg) {
    Log.i(this, msg);
  }

  private static String getId(DialerCall call) {
    return call == null ? "null" : call.getId();
  }

  public void onDowngradeToAudio(DialerCall call) {
    LogUtil.d("VideoSessionController.onDowngradeToAudio", "for callId: " + getId(call));
    if (call == null) {
      LogUtil.w("VideoSessionController.onDowngradeToAudio", "the current call is nul");
      return;
    }
    //reset hide preview flag
    call.setHidePreview(false);
    //show message when downgrade to voice
    InCallPresenter.getInstance().showMessage(
    com.android.incallui.R.string.video_call_downgrade_to_voice_call);
    /// @}
    /// M: add for OP18 plugin. @{
    ExtensionManager.getInCallExt().maybeDismissBatteryDialog();
    /// @}
  }

  /**
   * M: Timer to countdown.
   */
  private class AutoDeclineTimer {
    private int countdownSeconds = DEFAULT_COUNT_DOWN_SECONDS;
    private CallTimer timer;
    private long timingStartMillis;
    private long remainSecondsBeforeDecline = -1;

    AutoDeclineTimer() {
      timer = new CallTimer(new Runnable() {
        @Override
        public void run() {
          updateCountdown();
        }
      });
    }

    public void startTiming() {
      //TODO: customer might need some other value for this.
      ///M: change timer value for AT&T
      /// TODO: API Depedency
      ///countdownSeconds = ExtensionManager.getVideoCallExt().getDeclineTimer();
      /// M: CMCC upgrade with precondition needs differenct timer.the timer of UE that sends
      /// upgrade request is 20 seconds.the timer of UE that receives upgrade request is 15 seconds.
      /// M: [ALPS04013714] Get the decline timer or cancel upgrade timer by Carrier Config way. @{
      if (primaryCall != null && primaryCall.getVideoTech().
          getSessionModificationState() ==
          SessionModificationState.RECEIVED_UPGRADE_TO_VIDEO_REQUEST) {
        countdownSeconds = primaryCall.getVideoFeatures().getCountDownTimer(
            MtkCarrierConfigManager.MTK_KEY_TIMER_FOR_DECLINE_UPGRADE_INT);
      } else {
        countdownSeconds = primaryCall.getVideoFeatures().getCountDownTimer(
            MtkCarrierConfigManager.MTK_KEY_TIMER_FOR_CANCEL_UPGRADE_INT);
      }
      /// @}

      remainSecondsBeforeDecline = countdownSeconds;
      timingStartMillis = SystemClock.uptimeMillis();
      timer.start(MILLIS_PER_SECOND);
    }

    public long getAutoDeclineCountdown() {
      return remainSecondsBeforeDecline;
    }

    public void stopTiming() {
      timer.cancel();
      remainSecondsBeforeDecline = -1;
    }

    private void updateCountdown() {
      long currentMillis = SystemClock.uptimeMillis();
      long elapsedSeconds = (currentMillis - timingStartMillis) / MILLIS_PER_SECOND;
      if (elapsedSeconds > countdownSeconds) {
        if(inCallPresenter == null) {
          LogUtil.d("VideoSessionController.updateCountdown","inCallPresenter is null return");
          return;
        }

        //M: When call is in cancel progress, the timeout requires send cancel
        /// upgrade request out. @{
        if (primaryCall != null && primaryCall.getVideoTech().
            getSessionModificationState() ==
            SessionModificationState.WAITING_FOR_UPGRADE_TO_VIDEO_RESPONSE ) {
         inCallPresenter.cancelUpgradeRequest(inCallPresenter.getContext());
        ///@}
        } else {
          inCallPresenter.declineUpgradeRequest(inCallPresenter.getContext());
        }
      } else {
        remainSecondsBeforeDecline = countdownSeconds - elapsedSeconds;
        /// M: When call is in cancel progress, it doesn't need to update UI. @{
        if (primaryCall != null
            && primaryCall.getVideoTech().getSessionModificationState() !=
              SessionModificationState.RECEIVED_UPGRADE_TO_VIDEO_REQUEST) {
          LogUtil.d("VideoSessionController.updateCountdown", "it didn't need show updateUI");
          return;
        }
        ///@}
        updateRelatedUi();
      }
    }

    private void updateRelatedUi() {
      LogUtil.d("VideoSessionController.updateRelatedUi", "remain seconds: " +
          remainSecondsBeforeDecline);
      if(inCallPresenter == null) {
        LogUtil.d("VideoSessionController.updateRelatedUi", "inCallPresenter is null return");
        return;
      }
      inCallPresenter.onAutoDeclineCountdownChanged();
    }
  }
}
