/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License
 */

package com.android.incallui.call;

import android.annotation.TargetApi;
import android.app.Notification;
import android.bluetooth.BluetoothDevice;
import android.content.ActivityNotFoundException;
import android.content.Intent;
import android.os.Looper;
import android.support.annotation.MainThread;
import android.support.annotation.VisibleForTesting;
import android.telecom.InCallService;
import com.android.dialer.common.Assert;
import com.android.dialer.common.LogUtil;
import com.android.incallui.InCallPresenter;

import com.mediatek.incallui.compat.InCallUiCompat;
import com.mediatek.incallui.hangup.HangupOptions;
import com.mediatek.incallui.video.VideoFeatures;


import mediatek.telecom.MtkTelecomHelper;

import java.util.ArrayList;
import java.util.List;

/** Wrapper around Telecom APIs. */
public class TelecomAdapter implements InCallServiceListener {

  private static final String ADD_CALL_MODE_KEY = "add_call_mode";

  private static TelecomAdapter instance;
  private InCallService inCallService;

  private TelecomAdapter() {}

  @MainThread
  public static TelecomAdapter getInstance() {
    if (!Looper.getMainLooper().isCurrentThread()) {
      throw new IllegalStateException();
    }
    if (instance == null) {
      instance = new TelecomAdapter();
    }
    return instance;
  }

  @VisibleForTesting(otherwise = VisibleForTesting.NONE)
  public static void setInstanceForTesting(TelecomAdapter telecomAdapter) {
    instance = telecomAdapter;
  }

  @Override
  public void setInCallService(InCallService inCallService) {
    this.inCallService = inCallService;
  }

  @Override
  public void clearInCallService() {
    inCallService = null;
  }

  private android.telecom.Call getTelecomCallById(String callId) {
    DialerCall call = CallList.getInstance().getCallById(callId);
    return call == null ? null : call.getTelecomCall();
  }

  public void mute(boolean shouldMute) {
    if (inCallService != null) {
      inCallService.setMuted(shouldMute);
    } else {
      LogUtil.e("TelecomAdapter.mute", "mInCallService is null");
    }
  }

  public void setAudioRoute(int route) {
    if (inCallService != null) {
      inCallService.setAudioRoute(route);
    } else {
      LogUtil.e("TelecomAdapter.setAudioRoute", "mInCallService is null");
    }
  }

  public void merge(String callId) {
    android.telecom.Call call = getTelecomCallById(callId);
    if (call != null) {
      List<android.telecom.Call> conferenceable = call.getConferenceableCalls();
      if (!conferenceable.isEmpty()) {
        call.conference(conferenceable.get(0));
        // It's safe to clear restrict count for merge action.
        DialerCall.clearRestrictedCount();
      } else {
        if (call.getDetails().can(android.telecom.Call.Details.CAPABILITY_MERGE_CONFERENCE)) {
          call.mergeConference();
          // It's safe to clear restrict count for merge action.
          DialerCall.clearRestrictedCount();
        }
      }
      /// M: it's for AT&T RTT feature. Show rtt will drop to audio only when RTT call is merged
      /// into conference.  @{
      if (call.isRttActive()
          || (!conferenceable.isEmpty() && conferenceable.get(0).isRttActive())) {
          InCallPresenter.getInstance().showMessage(com.android.incallui.
                  R.string.rtt_drop_to_audio_when_merge);
      }
      /// @}
    } else {
      LogUtil.e("TelecomAdapter.merge", "call not in call list " + callId);
    }
  }

  public void swap(String callId) {
    android.telecom.Call call = getTelecomCallById(callId);
    if (call != null) {
      if (call.getDetails().can(android.telecom.Call.Details.CAPABILITY_SWAP_CONFERENCE)) {
        call.swapConference();
      }
    } else {
      LogUtil.e("TelecomAdapter.swap", "call not in call list " + callId);
    }
  }

  public void addCall() {
    if (inCallService != null) {
      Intent intent = new Intent(Intent.ACTION_DIAL);
      intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

      // when we request the dialer come up, we also want to inform
      // it that we're going through the "add call" option from the
      // InCallScreen / PhoneUtils.
      intent.putExtra(ADD_CALL_MODE_KEY, true);
      try {
        LogUtil.d("TelecomAdapter.addCall", "Sending the add DialerCall intent");
        inCallService.startActivity(intent);
      } catch (ActivityNotFoundException e) {
        // This is rather rare but possible.
        // Note: this method is used even when the phone is encrypted. At that moment
        // the system may not find any Activity which can accept this Intent.
        LogUtil.e("TelecomAdapter.addCall", "Activity for adding calls isn't found.", e);
      }
    }
  }

  public void playDtmfTone(String callId, char digit) {
    android.telecom.Call call = getTelecomCallById(callId);
    if (call != null) {
      call.playDtmfTone(digit);
    } else {
      LogUtil.e("TelecomAdapter.playDtmfTone", "call not in call list " + callId);
    }
  }

  public void stopDtmfTone(String callId) {
    android.telecom.Call call = getTelecomCallById(callId);
    if (call != null) {
      call.stopDtmfTone();
    } else {
      LogUtil.e("TelecomAdapter.stopDtmfTone", "call not in call list " + callId);
    }
  }

  public void postDialContinue(String callId, boolean proceed) {
    android.telecom.Call call = getTelecomCallById(callId);
    if (call != null) {
      call.postDialContinue(proceed);
    } else {
      LogUtil.e("TelecomAdapter.postDialContinue", "call not in call list " + callId);
    }
  }

  public boolean canAddCall() {
    if (inCallService != null) {
      return inCallService.canAddCall();
    }
    return false;
  }

  /**
   * Start a foreground notification. Calling it multiple times with the same id only updates the
   * existing notification. Whoever called this function are responsible for calling {@link
   * #stopForegroundNotification()} to remove the notification.
   */
  public void startForegroundNotification(int id, Notification notification) {
    Assert.isNotNull(
        inCallService, "No inCallService available for starting foreground notification");
    inCallService.startForeground(id, notification);
  }

  /**
   * Stop a started foreground notification. This does not stop {@code mInCallService} from running.
   */
  public void stopForegroundNotification() {
    if (inCallService != null) {
      inCallService.stopForeground(true /*removeNotification*/);
    } else {
      LogUtil.e(
          "TelecomAdapter.stopForegroundNotification",
          "no inCallService available for stopping foreground notification");
    }
  }

  @TargetApi(28)
  public void requestBluetoothAudio(BluetoothDevice bluetoothDevice) {
    if (inCallService != null) {
      inCallService.requestBluetoothAudio(bluetoothDevice);
    } else {
      LogUtil.e("TelecomAdapter.requestBluetoothAudio", "inCallService is null");
    }
  }

  /// M: ------------------------- MediaTek features ------------------------
  /**
   * M: [Voice Record] Toggle voice recording.
   * @param start if to record
   */
  public void toggleVoiceRecording(boolean start) {
    if (inCallService != null && InCallUiCompat.isMtkTelecomCompat()) {
      if (start) {
        inCallService.doMtkAction(
            MtkTelecomHelper.MtkInCallServiceHelper.buildParamsForStartVoiceRecording());
      } else {
        inCallService.doMtkAction(
            MtkTelecomHelper.MtkInCallServiceHelper.buildParamsForStopVoiceRecording());
      }
    } else {
      LogUtil.e("TelecomAdapter.toggleVoiceRecording",
          "error startVoiceRecording, mInCallService is null or no mtk telecom available!");
    }
  }

  /**
   * M: [Hang Up] Hang up all/hold.
   * @param option hang up code
   */
  public void hangupByOption(@HangupOptions int option) {
    if (inCallService != null && InCallUiCompat.isMtkTelecomCompat()) {
      if (HangupOptions.HANGUP_ALL == option) {
        inCallService.doMtkAction(
            MtkTelecomHelper.MtkInCallServiceHelper.buildParamsForHangupAll());
      } else if (HangupOptions.HANGUP_HOLD == option) {
        inCallService.doMtkAction(
            MtkTelecomHelper.MtkInCallServiceHelper.buildParamsForHangupHold());
      }
    } else {
      LogUtil.e("TelecomAdapter.hangupByOption",
          "error startVoiceRecording, mInCallService is null or no mtk telecom available!");
    }
  }

  /**
   * M: The all incoming calls will be sorted according to user's action,
   * since there are more than 1 incoming call exist user may touch to switch
   * any incoming call to the primary screen, the sequence of the incoming call
   * will be changed.
   */
  void setSortedIncomingCallList(ArrayList<String> incomingCallList) {
    if (inCallService != null && InCallUiCompat.isMtkTelecomCompat()) {
      inCallService.doMtkAction(MtkTelecomHelper.MtkInCallServiceHelper
          .buildParamsForSetSortedIncomingCallList(incomingCallList));
    } else {
      LogUtil.e("TelecomAdapter.setSortedIncomingCallList",
          "inCallService is null");
    }
  }

  /**
   * M: [Volte Conference] For VoLTE invitation.
   * @param conferenceCallId id
   * @param numbers the number list
   */
  public void inviteConferenceParticipants(String conferenceCallId, ArrayList<String> numbers) {
    android.telecom.Call conference = getTelecomCallById(conferenceCallId);
    if (inCallService != null && conference != null && InCallUiCompat.isMtkTelecomCompat()) {
      inCallService.doMtkAction(
          MtkTelecomHelper.MtkInCallServiceHelper.buildParamsForInviteConferenceParticipants(
              conference.getDetails().getTelecomCallId(), numbers));
    } else {
      LogUtil.e("TelecomAdapter.inviteConferenceParticipants", "inCallService is null");
    }
  }

  /**
   * [ECT(blind)]
   * Blind ECT
   * @param callId the call transfer from
   * @param number the number transfer to
   * @param type
   */
  public void blindExplicitCallTransfer(String callId, String number, int type) {
    if (inCallService != null && InCallUiCompat.isMtkTelecomCompat()) {
      inCallService.doMtkAction(
          MtkTelecomHelper.MtkInCallServiceHelper
              .buildParamsForBlindOrAssuredEct(callId, number, type));
    } else {
      LogUtil.e("TelecomAdapter.blindExplicitCallTransfer",
          "error blindEct, mInCallService is null or no mtk telecom available!");
    }
  }

  /**
   * [ECT(blind)]
   * ECT
   * @param callId the call transfer to
   */
  public void explicitCallTransfer(String callId) {
    if (inCallService != null && InCallUiCompat.isMtkTelecomCompat()) {
      inCallService.doMtkAction(
          MtkTelecomHelper.MtkInCallServiceHelper
              .buildParamsForExplicitCallTransfer(callId));
    } else {
      LogUtil.e("TelecomAdapter.explicitCallTransfer",
          "error ect, mInCallService is null or no mtk telecom available!");
    }
  }

  /**
   * M: [Device Switch]
   * Start Device Switch
   * @param callId the call transfer to
   * @param number the call number
   * @param deviceId the call deviceId
   */
  public void deviceSwitch(String callId, String number, String deviceId) {
    if (inCallService != null && InCallUiCompat.isMtkTelecomCompat()) {
      /// inCallService.doMtkAction(
          /// MtkTelecomHelper.MtkInCallServiceHelper
              /// .buildParamsForDeviceSwitch(callId, number, deviceId));
    } else {
      LogUtil.e("TelecomAdapter.deviceSwitch",
          "error deviceSwitch, mInCallService is null or no mtk telecom available!");
    }
  }

  /**
   * M: [Device Switch]
   * Cancel Device Switch
   * @param callId the call transfer to
   */
  public void cancelDeviceSwitch(String callId) {
    if (inCallService != null && InCallUiCompat.isMtkTelecomCompat()) {
      /// inCallService.doMtkAction(
          /// MtkTelecomHelper.MtkInCallServiceHelper
              /// .buildParamsForCancelDeviceSwitch(callId));
    } else {
      LogUtil.e("TelecomAdapter.cancelDeviceSwitch",
          "error deviceSwitch, mInCallService is null or no mtk telecom available!");
    }
  }
}
