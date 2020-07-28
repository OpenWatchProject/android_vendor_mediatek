/*
 * Copyright (C) 2013 The Android Open Source Project
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

package com.android.incallui;

import android.content.Context;
import android.os.Bundle;
import android.os.Trace;
import android.os.UserHandle;
import android.support.v4.app.Fragment;
import android.support.v4.os.UserManagerCompat;
import android.telecom.CallAudioState;
import android.telecom.PhoneAccountHandle;
import android.telecom.VideoProfile;

import com.android.contacts.common.compat.CallCompat;
import com.android.dialer.common.Assert;
import com.android.dialer.common.LogUtil;
import com.android.dialer.common.concurrent.DialerExecutorComponent;
import com.android.dialer.logging.DialerImpression;
import com.android.dialer.logging.DialerImpression.Type;
import com.android.dialer.logging.Logger;
import com.android.dialer.telecom.TelecomUtil;
import com.android.incallui.InCallCameraManager.Listener;
import com.android.incallui.InCallPresenter.CanAddCallListener;
import com.android.incallui.InCallPresenter.InCallDetailsListener;
import com.android.incallui.InCallPresenter.InCallState;
import com.android.incallui.InCallPresenter.InCallStateListener;
import com.android.incallui.InCallPresenter.IncomingCallListener;
import com.android.incallui.InCallPresenter.SessionStateListener;
import com.android.incallui.audiomode.AudioModeProvider;
import com.android.incallui.audiomode.AudioModeProvider.AudioModeListener;
import com.android.incallui.call.CallList;
import com.android.incallui.call.DialerCall;
import com.android.incallui.call.DialerCall.CameraDirection;
import com.android.incallui.call.TelecomAdapter;
import com.android.incallui.call.state.DialerCallState;
import com.android.incallui.incall.protocol.InCallButtonIds;
import com.android.incallui.incall.protocol.InCallButtonUi;
import com.android.incallui.incall.protocol.InCallButtonUiDelegate;
import com.android.incallui.multisim.SwapSimWorker;
import com.android.incallui.videotech.VideoTech;
import com.android.incallui.videotech.utils.SessionModificationState;
import com.android.incallui.videotech.utils.VideoUtils;
import com.mediatek.incallui.blindect.AddTransferNumberScreenController;
import com.mediatek.incallui.compat.InCallUiCompat;
import com.mediatek.incallui.plugin.ExtensionManager;
import com.mediatek.incallui.utils.InCallUtils;
import com.mediatek.incallui.video.VideoSessionController;

import mediatek.telecom.MtkCall;

/** Logic for call buttons. */
public class CallButtonPresenter
    implements InCallStateListener,
        AudioModeListener,
        IncomingCallListener,
        InCallDetailsListener,
        CanAddCallListener,
        Listener,
        InCallButtonUiDelegate,
        /// M: ALPS03449185 Update session state to refresh UI. @{
        SessionStateListener {
        /// @}

  private static final String KEY_AUTOMATICALLY_MUTED_BY_ADD_CALL =
      "incall_key_automatically_muted_by_add_call";
  private static final String KEY_PREVIOUS_MUTE_STATE = "incall_key_previous_mute_state";

  private final Context context;
  private InCallButtonUi inCallButtonUi;
  private DialerCall call;
  private boolean automaticallyMutedByAddCall = false;
  private boolean previousMuteState = false;
  private boolean isInCallButtonUiReady;
  private PhoneAccountHandle otherAccount;

  public CallButtonPresenter(Context context) {
    LogUtil.d("CallButtonPresenter.CallButtonPresenter", "previousMuteState:" + previousMuteState);
    this.context = context.getApplicationContext();
  }

  @Override
  public void onInCallButtonUiReady(InCallButtonUi ui) {
    Assert.checkState(!isInCallButtonUiReady);
    inCallButtonUi = ui;
    AudioModeProvider.getInstance().addListener(this);

    // register for call state changes last
    final InCallPresenter inCallPresenter = InCallPresenter.getInstance();
    inCallPresenter.addListener(this);
    inCallPresenter.addIncomingCallListener(this);
    /// M: ALPS03504365, improve MO/MT performance @{
    /// inCallPresenter.addDetailsListener(this);
    /// @}
    inCallPresenter.addCanAddCallListener(this);
    inCallPresenter.getInCallCameraManager().addCameraSelectionListener(this);
    /// M: ALPS03449185 Update session state to refresh UI. @{
    inCallPresenter.addSessionListener(this);
    /// @}

    // Update the buttons state immediately for the current call
    onStateChange(InCallState.NO_CALLS, inCallPresenter.getInCallState(), CallList.getInstance());
    isInCallButtonUiReady = true;
  }

  @Override
  public void onInCallButtonUiUnready() {
    Assert.checkState(isInCallButtonUiReady);
    inCallButtonUi = null;
    InCallPresenter.getInstance().removeListener(this);
    AudioModeProvider.getInstance().removeListener(this);
    InCallPresenter.getInstance().removeIncomingCallListener(this);
    InCallPresenter.getInstance().removeDetailsListener(this);
    InCallPresenter.getInstance().getInCallCameraManager().removeCameraSelectionListener(this);
    InCallPresenter.getInstance().removeCanAddCallListener(this);
    /// M: ALPS03449185 Update session state to refresh UI. @{
    InCallPresenter.getInstance().removeSessionListener(this);
    /// @}
    isInCallButtonUiReady = false;
  }

  @Override
  public void onStateChange(InCallState oldState, InCallState newState, CallList callList) {
    Trace.beginSection("CallButtonPresenter.onStateChange");
    if (newState == InCallState.OUTGOING) {
      call = callList.getOutgoingCall();
    } else if (newState == InCallState.INCALL) {
      call = callList.getActiveOrBackgroundCall();

      // When connected to voice mail, automatically shows the dialpad.
      // (On previous releases we showed it when in-call shows up, before waiting for
      // OUTGOING.  We may want to do that once we start showing "Voice mail" label on
      // the dialpad too.)
      if (oldState == InCallState.OUTGOING && call != null) {
        if (call.isVoiceMailNumber() && getActivity() != null) {
          getActivity().showDialpadFragment(true /* show */, true /* animate */);
        }
      }
    } else if (newState == InCallState.INCOMING) {
      if (getActivity() != null) {
        getActivity().showDialpadFragment(false /* show */, true /* animate */);
      }
      call = callList.getIncomingCall();
    } else {
      call = null;
    }
    updateUi(newState, call);
    Trace.endSection();
  }

  /**
   * Updates the user interface in response to a change in the details of a call. Currently handles
   * changes to the call buttons in response to a change in the details for a call. This is
   * important to ensure changes to the active call are reflected in the available buttons.
   *
   * @param call The active call.
   * @param details The call details.
   */
  @Override
  public void onDetailsChanged(DialerCall call, android.telecom.Call.Details details) {
    // Only update if the changes are for the currently active call
    if (inCallButtonUi != null && call != null && call.equals(this.call)) {
      updateButtonsState(call);
    }
  }

  @Override
  public void onIncomingCall(InCallState oldState, InCallState newState, DialerCall call) {
    onStateChange(oldState, newState, CallList.getInstance());
  }

  @Override
  public void onCanAddCallChanged(boolean canAddCall) {
    LogUtil.d("CallButtonPresenter.onCanAddCallChanged", "canAddCall = " + canAddCall);
    if (inCallButtonUi != null && call != null) {
      updateButtonsState(call);
    }
  }

  @Override
  public void onAudioStateChanged(CallAudioState audioState) {
    if (inCallButtonUi != null) {
      inCallButtonUi.setAudioState(audioState);
    }
  }

  @Override
  public CallAudioState getCurrentAudioState() {
    return AudioModeProvider.getInstance().getAudioState();
  }

  @Override
  public void setAudioRoute(int route) {
    LogUtil.i(
        "CallButtonPresenter.setAudioRoute",
        "sending new audio route: " + CallAudioState.audioRouteToString(route));
    TelecomAdapter.getInstance().setAudioRoute(route);
  }

  /** Function assumes that bluetooth is not supported. */
  @Override
  public void toggleSpeakerphone() {
    // This function should not be called if bluetooth is available.
    CallAudioState audioState = getCurrentAudioState();
    if (0 != (CallAudioState.ROUTE_BLUETOOTH & audioState.getSupportedRouteMask())) {
      // It's clear the UI is wrong, so update the supported mode once again.
      LogUtil.e(
          "CallButtonPresenter", "toggling speakerphone not allowed when bluetooth supported.");
      inCallButtonUi.setAudioState(audioState);
      return;
    }

    /// M: ALPS03664609 Fixed no pointer exception. @{
    if (call == null) {
      LogUtil.e("CallButtonPresenter.toggleSpeakerphone", "Call is null");
      return;
    }
    /// @}

    int newRoute;
    if (audioState.getRoute() == CallAudioState.ROUTE_SPEAKER) {
      newRoute = CallAudioState.ROUTE_WIRED_OR_EARPIECE;
      Logger.get(context)
          .logCallImpression(
              DialerImpression.Type.IN_CALL_SCREEN_TURN_ON_WIRED_OR_EARPIECE,
              call.getUniqueCallId(),
              call.getTimeAddedMs());
    } else {
      newRoute = CallAudioState.ROUTE_SPEAKER;
      Logger.get(context)
          .logCallImpression(
              DialerImpression.Type.IN_CALL_SCREEN_TURN_ON_SPEAKERPHONE,
              call.getUniqueCallId(),
              call.getTimeAddedMs());
    }

    setAudioRoute(newRoute);
  }

  @Override
  public void muteClicked(boolean checked, boolean clickedByUser) {
    LogUtil.i(
        "CallButtonPresenter", "turning on mute: %s, clicked by user: %s", checked, clickedByUser);
    if (clickedByUser) {
      Logger.get(context)
          .logCallImpression(
              checked
                  ? DialerImpression.Type.IN_CALL_SCREEN_TURN_ON_MUTE
                  : DialerImpression.Type.IN_CALL_SCREEN_TURN_OFF_MUTE,
              call.getUniqueCallId(),
              call.getTimeAddedMs());
    }
    TelecomAdapter.getInstance().mute(checked);
  }

  @Override
  public void holdClicked(boolean checked) {
    if (call == null) {
      return;
    }
    if (checked) {
      LogUtil.i("CallButtonPresenter", "putting the call on hold: " + call);
      call.hold();
    } else {
      LogUtil.i("CallButtonPresenter", "removing the call from hold: " + call);
      call.unhold();
    }
  }

  @Override
  public void swapClicked() {
    if (call == null) {
      return;
    }

    LogUtil.i("CallButtonPresenter", "swapping the call: " + call);
    TelecomAdapter.getInstance().swap(call.getId());
  }

  @Override
  public void mergeClicked() {
    Logger.get(context)
        .logCallImpression(
            DialerImpression.Type.IN_CALL_MERGE_BUTTON_PRESSED,
            call.getUniqueCallId(),
            call.getTimeAddedMs());
    TelecomAdapter.getInstance().merge(call.getId());
    /// M: ALPS03881758, disable hangup all and hangup hold button when click merge. @{
    DialerCall bgCall = CallList.getInstance().getBackgroundCall();
    if ((bgCall != null
            && bgCall.isConferenceCall()
            && !bgCall.hasProperty(MtkCall.MtkDetails.MTK_PROPERTY_VOLTE)
            && bgCall.hasConferencePermission()
            && !InCallUtils.isCdmaCall(bgCall))
        || (call != null
            && call.isConferenceCall()
            && !call.hasProperty(MtkCall.MtkDetails.MTK_PROPERTY_VOLTE)
            && call.hasConferencePermission()
            && !InCallUtils.isCdmaCall(call))) {
      call.setConferenceMergeStatus(true);
      updateExtButtonUI(call);
    }
    /// @}
  }

  @Override
  public void addCallClicked() {
    Logger.get(context)
        .logCallImpression(
            DialerImpression.Type.IN_CALL_ADD_CALL_BUTTON_PRESSED,
            call.getUniqueCallId(),
            call.getTimeAddedMs());
    if (automaticallyMutedByAddCall) {
      // Since clicking add call button brings user to MainActivity and coming back refreshes mute
      // state, add call button should only be clicked once during InCallActivity shows. Otherwise,
      // we set previousMuteState wrong.
      return;
    }
    // Automatically mute the current call
    automaticallyMutedByAddCall = true;
    previousMuteState = AudioModeProvider.getInstance().getAudioState().isMuted();
    // Simulate a click on the mute button
    muteClicked(true /* checked */, false /* clickedByUser */);
    TelecomAdapter.getInstance().addCall();
  }

  @Override
  public void showDialpadClicked(boolean checked) {
    Logger.get(context)
        .logCallImpression(
            DialerImpression.Type.IN_CALL_SHOW_DIALPAD_BUTTON_PRESSED,
            call.getUniqueCallId(),
            call.getTimeAddedMs());
    LogUtil.v("CallButtonPresenter", "show dialpad " + String.valueOf(checked));
    getActivity().showDialpadFragment(checked /* show */, true /* animate */);
  }

  @Override
  public void changeToVideoClicked() {
    LogUtil.enterBlock("CallButtonPresenter.changeToVideoClicked");

    ///M: ALPS03758801 Should reset the hide preview status when call change to audio by network.
    ///Better reset the status when click upgrade button or receive upgrade request. @{
    call.setHidePreview(false);
    /// @}

    Logger.get(context)
        .logCallImpression(
            DialerImpression.Type.VIDEO_CALL_UPGRADE_REQUESTED,
            call.getUniqueCallId(),
            call.getTimeAddedMs());
    /// M: check if Video call over WIFI is allowed or not. @{
    if (call.hasProperty(android.telecom.Call.Details.PROPERTY_WIFI) &&
        call.getVideoFeatures().disableVideoCallOverWifi()) {
      InCallPresenter.getInstance().showMessage(
          com.android.incallui.R.string.video_over_wifi_not_available);
      return;
    }
    /// @}
    call.getVideoTech().upgradeToVideo(context);

    /// M: [ALPS04013714] Refer to CT NS-IOT 3009 case requirement,auto start cancel timer after
    /// sending upgrade on CT lab test. @{
    if(call.getVideoFeatures().supportsCancelUpgradeVideo()
        && call.getVideoFeatures().supportsAutoStartTimerForCancelUpgrade()
        && InCallUtils.MTK_CT_VILTE_LAB_TEST) {
      LogUtil.i("CallButtonPresenter.changeToVideoClicked", "auto start timer for cancel");
      VideoSessionController.getInstance().startTimingForAutoDecline(call);
    }
    /// @}
  }

  @Override
  public void changeToRttClicked() {
    LogUtil.enterBlock("CallButtonPresenter.changeToRttClicked");
    call.sendRttUpgradeRequest();
  }

  @Override
  public void onEndCallClicked() {
    LogUtil.i("CallButtonPresenter.onEndCallClicked", "call: " + call);
    if (call != null) {
      call.disconnect();
    }
  }

  @Override
  public void showAudioRouteSelector() {
    /// M: ALPS03762461 If activity is not resumed, it shall not show new fragment @{
    if (!getActivity().isResumed()) {
      LogUtil.i("CallButtonPresenter.showAudioRouteSelector", "Activity is not resume");
      return;
    }
    /// @}
    inCallButtonUi.showAudioRouteSelector();
  }

  @Override
  public void swapSimClicked() {
    LogUtil.enterBlock("CallButtonPresenter.swapSimClicked");
    Logger.get(getContext()).logImpression(Type.DUAL_SIM_CHANGE_SIM_PRESSED);
    SwapSimWorker worker =
        new SwapSimWorker(
            getContext(),
            call,
            InCallPresenter.getInstance().getCallList(),
            otherAccount,
            InCallPresenter.getInstance().acquireInCallUiLock("swapSim"));
    DialerExecutorComponent.get(getContext())
        .dialerExecutorFactory()
        .createNonUiTaskBuilder(worker)
        .build()
        .executeParallel(null);
  }

  /**
   * Switches the camera between the front-facing and back-facing camera.
   *
   * @param useFrontFacingCamera True if we should switch to using the front-facing camera, or false
   *     if we should switch to using the back-facing camera.
   */
  @Override
  public void switchCameraClicked(boolean useFrontFacingCamera) {
    updateCamera(useFrontFacingCamera);
  }

  @Override
  public void toggleCameraClicked() {
    LogUtil.i("CallButtonPresenter.toggleCameraClicked", "");
    if (call == null) {
      return;
    }
    Logger.get(context)
        .logCallImpression(
            DialerImpression.Type.IN_CALL_SCREEN_SWAP_CAMERA,
            call.getUniqueCallId(),
            call.getTimeAddedMs());
    switchCameraClicked(
        !InCallPresenter.getInstance().getInCallCameraManager().isUsingFrontFacingCamera());
  }

  /**
   * Stop or start client's video transmission.
   *
   * @param pause True if pausing the local user's video, or false if starting the local user's
   *     video.
   */
  @Override
  public void pauseVideoClicked(boolean pause) {
    LogUtil.i("CallButtonPresenter.pauseVideoClicked", "%s", pause ? "pause" : "unpause");

    Logger.get(context)
        .logCallImpression(
            pause
                ? DialerImpression.Type.IN_CALL_SCREEN_TURN_OFF_VIDEO
                : DialerImpression.Type.IN_CALL_SCREEN_TURN_ON_VIDEO,
            call.getUniqueCallId(),
            call.getTimeAddedMs());

    /// M: pause the video without sending sip message feature check @{
    if (pauseVideoWithoutSipMessage(pause)) {
      // video is paused by making camera as null so no need to do anything else
      return;
    }
    /// @}
    if (pause) {
      /// M: ALPS03593227 can't set camera again after pause request fail. We will remove set camera
      /// is null when send pause video request. And videocallpresenter will set correct value to
      /// vtservice accroding to video state. @{
      /// google original code:
      /// call.getVideoTech().setCamera(null);
      /// @}
      call.getVideoTech().stopTransmission();
    } else {
      /// M: ALPS03593227 can't set camera again after pause request fail. We will remove set camera
      /// is null when send pause video request. And videocallpresenter will set correct value to
      /// vtservice accroding to video state. @{
      /// google original code:
      /// updateCamera(
      ///    InCallPresenter.getInstance().getInCallCameraManager().isUsingFrontFacingCamera());
      /// @}
      call.getVideoTech().resumeTransmission(context);
    }

    inCallButtonUi.setVideoPaused(pause);
    inCallButtonUi.enableButton(InCallButtonIds.BUTTON_PAUSE_VIDEO, false);
  }

  private void updateCamera(boolean useFrontFacingCamera) {
    /// M: ALPS03462464 fixed no pointer exception @{
    if (call == null) {
      LogUtil.w("CallButtonPresenter.updateCamera", "call is already null");
      return;
    }
    /// @}
    InCallCameraManager cameraManager = InCallPresenter.getInstance().getInCallCameraManager();
    cameraManager.setUseFrontFacingCamera(useFrontFacingCamera);

    /// M: if video paused by turning of camera, then dont enable camera @{
    if ((call != null) && (call.getVideoPauseState() == true)) {
      LogUtil.i("CallButtonPresenter.updateCamera", " Video is paused by making camera null");
      return;
    }
    /// @}
    String cameraId = cameraManager.getActiveCameraId();
    if (cameraId != null) {
      final int cameraDir =
          cameraManager.isUsingFrontFacingCamera()
              ? CameraDirection.CAMERA_DIRECTION_FRONT_FACING
              : CameraDirection.CAMERA_DIRECTION_BACK_FACING;
      call.setCameraDir(cameraDir);
      call.getVideoTech().setCamera(cameraId);
    }
  }

  private void updateUi(InCallState state, DialerCall call) {
    LogUtil.v("CallButtonPresenter", "updating call UI for call: %s", call);

    if (inCallButtonUi == null) {
      return;
    }

    if (call != null) {
      inCallButtonUi.updateInCallButtonUiColors(
          InCallPresenter.getInstance().getThemeColorManager().getSecondaryColor());
    }

    final boolean isEnabled =
        state.isConnectingOrConnected() && !state.isIncoming() && call != null;
    inCallButtonUi.setEnabled(isEnabled);

    if (call == null) {
      /// M: ALPS04047396, Dismiss voice recording icon when no call exist. @{
      inCallButtonUi.updateRecordStateUi(false);
      /// @}
      return;
    }

    updateButtonsState(call);
  }

  /**
   * Updates the buttons applicable for the UI.
   *
   * @param call The active call.
   */
  @SuppressWarnings(value = {"MissingPermission"})
  private void updateButtonsState(DialerCall call) {
    LogUtil.v("CallButtonPresenter.updateButtonsState", "");
    final boolean isVideo = call.isVideoCall();

    // Common functionality (audio, hold, etc).
    // Show either HOLD or SWAP, but not both. If neither HOLD or SWAP is available:
    //     (1) If the device normally can hold, show HOLD in a disabled state.
    //     (2) If the device doesn't have the concept of hold/swap, remove the button.
    boolean showSwap = call.can(android.telecom.Call.Details.CAPABILITY_SWAP_CONFERENCE);

    /// M: Fix ALPS03419203 hold shall has even if it is cdma call.
    /// boolean showHold =
    ///  !showSwap
    ///         && call.can(android.telecom.Call.Details.CAPABILITY_SUPPORT_HOLD)
    ///         && call.can(android.telecom.Call.Details.CAPABILITY_HOLD);
    boolean showHold =
        call.can(android.telecom.Call.Details.CAPABILITY_SUPPORT_HOLD)
            && call.can(android.telecom.Call.Details.CAPABILITY_HOLD);
    /// @}

    final boolean isCallOnHold = call.getState() == DialerCallState.ONHOLD;

    /// M: Add log to print user canAddCall value. @{
    /// Google code:
    /// boolean showAddCall =
    ///     TelecomAdapter.getInstance().canAddCall() && UserManagerCompat.isUserUnlocked(context);
    boolean canAddCall = TelecomAdapter.getInstance().canAddCall();
    LogUtil.d("CallButtonPresenter.updateButtonsState", "canAddCall = " + canAddCall);
    boolean showAddCall = canAddCall && UserManagerCompat.isUserUnlocked(context);
    /// @}

    // There can only be two calls so don't show the ability to merge when one of them
    // is a speak easy call.
    final boolean showMerge =
        InCallPresenter.getInstance()
                .getCallList()
                .getAllCalls()
                .stream()
                .noneMatch(c -> c != null && c.isSpeakEasyCall())
            && InCallPresenter.getInstance().getCallList().getAllCalls().size() > 1
            && call.can(android.telecom.Call.Details.CAPABILITY_MERGE_CONFERENCE);

    /// M: Should check canUpgradeToVideoCall. Because in CMCC NW, can not start
    /// upgrade when have 1A1H calls. ALPS03454000
    /// And should enable the upgrade button in oneway video status. @{
    /// Google code:
    /// boolean showUpgradeToVideo = !isVideo && (hasVideoCallCapabilities(call));
    boolean showUpgradeToVideo = (!isVideo
            || ExtensionManager.getInCallButtonExt().shouldShowUpgradeButton(call))
        && hasVideoCallCapabilities(call)
        && call.getVideoFeatures().canUpgradeToVideoCall();
    /// @}

    boolean showDowngradeToAudio = isVideo && isDowngradeToAudioSupported(call);
    final boolean showMute = call.can(android.telecom.Call.Details.CAPABILITY_MUTE);

    final boolean hasCameraPermission =
        isVideo && VideoUtils.hasCameraPermissionAndShownPrivacyToast(context);
    // Disabling local video doesn't seem to work when dialing. See a bug.
    boolean showPauseVideo =
        isVideo
            && call.getState() != DialerCallState.DIALING
            && call.getState() != DialerCallState.CONNECTING;
    /// M: Handle some special case, e.g. 3G Video not support hold, downgrade. @{
    boolean showDialpad = false;
    if (isVideo) {
      showHold &= call.getVideoFeatures().supportsHold();
      showDowngradeToAudio &= call.getVideoFeatures().supportsDowngrade();
      showPauseVideo &= call.getVideoFeatures().supportsPauseVideo();
      /// For video call, only show some special OP.
      showDialpad = call.getVideoFeatures().supportShowVideoDialpad();
    } else {
      showDialpad = true;
    }
    /// @}

    /// M: ALPS03450799 hide some video call buttons when dialing/incoming or held @{
    boolean currentHeldState = call.isRemotelyHeld();
    boolean isCallActive = call.getState() == DialerCallState.ACTIVE;
    if (currentHeldState || !isCallActive) {
      showDowngradeToAudio = false;
      showUpgradeToVideo = false;
      showPauseVideo = false;
      // showHideLocalVideoBtn in updateExtButtonUI(call)
      // hide BUTTON_SWITCH_CAMERA below
      LogUtil.v("CallButtonPresenter.updateButtonsState", "call isHeld:" + currentHeldState);
    }
    /// @}

    /// M: [Performance] Do not caculate otherAccount first. @{
    /// Google code:
    /// otherAccount = TelecomUtil.getOtherAccount(getContext(), call.getAccountHandle());
    /// boolean showSwapSim =
    ///     !call.isEmergencyCall()
    ///         && otherAccount != null
    ///         && !call.isVoiceMailNumber()
    ///         && DialerCallState.isDialing(call.getState())
    ///         // Most devices cannot make calls on 2 SIMs at the same time.
    ///         && InCallPresenter.getInstance().getCallList().getAllCalls().size() == 1;
    boolean showSwapSim =
        !call.isEmergencyCall()
            && !call.isVoiceMailNumber()
            && DialerCallState.isDialing(call.getState())
            // Most devices cannot make calls on 2 SIMs at the same time.
            && InCallPresenter.getInstance().getCallList().getAllCalls().size() == 1;
    if (showSwapSim) {
      otherAccount = TelecomUtil.getOtherAccount(getContext(), call.getAccountHandle());
      showSwapSim = showSwapSim && (otherAccount != null);
    }
    /// @}

    boolean showUpgradeToRtt = call.canUpgradeToRttCall();
    boolean enableUpgradeToRtt = showUpgradeToRtt && call.getState() == DialerCallState.ACTIVE;

    /*
     * M: ALPS03175530 Porting N's criteria.
     * add canEnableVideoBtn flag to control showing hold button is avoid this case:
     * when there was a volte call, we can do some video action, during this action,
     * we can't show hold button. by another way if there was a voice call, it's
     * SessionModificationState always is no_request, so meet the requestment. @{
     */
    if (isSesssionProgressCall(call)) {
      showHold = false;
      showUpgradeToVideo = false;
      showAddCall = false;
      showDowngradeToAudio = false;
      showPauseVideo = false;
      showSwap = false;
      LogUtil.v("CallButtonPresenter.updateButtonsState", "isSesssionProgressCall, disable hold," +
          " upgrade, add call, downgrade, pause video and swap button.");
    }
    final boolean isCameraOff = call.getVideoState() == VideoProfile.STATE_RX_ENABLED;
    /* @} */

    /// M: for RTT feature. when only exists one RTT call, show hold button. More than one RTT call,
    /// show swap button instead. @{
    if (call.isActiveRttCall()) {
      showHold &= InCallPresenter.getInstance().getCallList().getAllCalls().size() == 1;
    }
    /// @}
    inCallButtonUi.showButton(InCallButtonIds.BUTTON_AUDIO, true);
    inCallButtonUi.showButton(InCallButtonIds.BUTTON_SWAP, showSwap);
    inCallButtonUi.showButton(InCallButtonIds.BUTTON_HOLD, showHold);
    inCallButtonUi.setHold(isCallOnHold);
    inCallButtonUi.showButton(InCallButtonIds.BUTTON_MUTE, showMute);
    inCallButtonUi.showButton(InCallButtonIds.BUTTON_SWAP_SIM, showSwapSim);
    inCallButtonUi.showButton(InCallButtonIds.BUTTON_ADD_CALL, true);
    inCallButtonUi.enableButton(InCallButtonIds.BUTTON_ADD_CALL, showAddCall);
    inCallButtonUi.showButton(InCallButtonIds.BUTTON_UPGRADE_TO_VIDEO, showUpgradeToVideo);
    inCallButtonUi.showButton(InCallButtonIds.BUTTON_UPGRADE_TO_RTT, showUpgradeToRtt);
    inCallButtonUi.enableButton(InCallButtonIds.BUTTON_UPGRADE_TO_RTT, enableUpgradeToRtt);
    inCallButtonUi.showButton(InCallButtonIds.BUTTON_DOWNGRADE_TO_AUDIO, showDowngradeToAudio);
    inCallButtonUi.showButton(
        InCallButtonIds.BUTTON_SWITCH_CAMERA,
        isVideo && hasCameraPermission && call.getVideoTech().isTransmitting()
        /// M: when pause video make one way and only can receive video, we can't show
        /// switch camera button (ALPS03175530).
        /// and call is held (ALPS03450799).
        && !isSesssionProgressCall(call) && !isCameraOff && !currentHeldState);
    inCallButtonUi.showButton(InCallButtonIds.BUTTON_PAUSE_VIDEO, showPauseVideo);
    /// M: ALPS03452869, enable pause button
    inCallButtonUi.enableButton(InCallButtonIds.BUTTON_PAUSE_VIDEO, showPauseVideo);

    if (isVideo) {
      inCallButtonUi.setVideoPaused(!call.getVideoTech().isTransmitting() || !hasCameraPermission
        /// M: Check if video was paused by disabling camera only without session modification @ {
        || (call.getVideoPauseState() == true));
        /// @}
    }
    /// M: For video only show diaplad for some special OP.
    inCallButtonUi.showButton(InCallButtonIds.BUTTON_DIALPAD, showDialpad);
    inCallButtonUi.showButton(InCallButtonIds.BUTTON_MERGE, showMerge);

    /// M: MediaTek extension buttons.
    updateExtButtonUI(call);

    inCallButtonUi.updateButtonStates();
  }

  private boolean hasVideoCallCapabilities(DialerCall call) {
    return call.getVideoTech().isAvailable(context, call.getAccountHandle());
  }

  /**
   * Determines if downgrading from a video call to an audio-only call is supported. In order to
   * support downgrade to audio, the SDK version must be >= N and the call should NOT have the
   * {@link android.telecom.Call.Details#CAPABILITY_CANNOT_DOWNGRADE_VIDEO_TO_AUDIO}.
   *
   * @param call The call.
   * @return {@code true} if downgrading to an audio-only call from a video call is supported.
   */
  private boolean isDowngradeToAudioSupported(DialerCall call) {
    // TODO(a bug): If there is an RCS video share session, return true here
    return !call.can(CallCompat.Details.CAPABILITY_CANNOT_DOWNGRADE_VIDEO_TO_AUDIO);
  }

  @Override
  public void refreshMuteState() {
    // Restore the previous mute state
    if (automaticallyMutedByAddCall
        && AudioModeProvider.getInstance().getAudioState().isMuted() != previousMuteState) {
      if (inCallButtonUi == null) {
        return;
      }
      muteClicked(previousMuteState, false /* clickedByUser */);
    }
    automaticallyMutedByAddCall = false;
  }

  @Override
  public void onSaveInstanceState(Bundle outState) {
    outState.putBoolean(KEY_AUTOMATICALLY_MUTED_BY_ADD_CALL, automaticallyMutedByAddCall);
    outState.putBoolean(KEY_PREVIOUS_MUTE_STATE, previousMuteState);
  }

  @Override
  public void onRestoreInstanceState(Bundle savedInstanceState) {
    automaticallyMutedByAddCall =
        savedInstanceState.getBoolean(
            KEY_AUTOMATICALLY_MUTED_BY_ADD_CALL, automaticallyMutedByAddCall);
    previousMuteState = savedInstanceState.getBoolean(KEY_PREVIOUS_MUTE_STATE, previousMuteState);
  }

  @Override
  public void onCameraPermissionGranted() {
    if (call != null) {
      updateButtonsState(call);
    }
  }

  @Override
  public void onActiveCameraSelectionChanged(boolean isUsingFrontFacingCamera) {
    if (inCallButtonUi == null) {
      return;
    }
    inCallButtonUi.setCameraSwitched(!isUsingFrontFacingCamera);
  }

  @Override
  public Context getContext() {
    return context;
  }

  private InCallActivity getActivity() {
    if (inCallButtonUi != null) {
      Fragment fragment = inCallButtonUi.getInCallButtonUiFragment();
      if (fragment != null) {
        return (InCallActivity) fragment.getActivity();
      }
    }
    return null;
  }

  /// M: ------------------ MediaTek features ---------------------
  // Downgrade to audio call.
  @Override
  public void changeToAudioClicked() {
    LogUtil.enterBlock("CallButtonPresenter.changeToAudioClicked");
    if (call == null) {
      LogUtil.w("CallButtonPresenter.changeToAudioClicked", "downgradeToAudio failed");
      return;
    }
    call.getVideoTech().downgradeToAudio();
    //reset hide preview flag
    call.setHidePreview(false);
    InCallPresenter.getInstance().showMessage(R.string.video_call_downgrade_request);
  }

  /**
   * M: ALPS03449185 Update session state to refresh UI.
   * @param state SessionState
   */
  @Override
  public void onSessionModificationStateChanged(int state) {
    if (call == null) {
        return;
    }
    updateButtonsState(call);
  }

  /**
   * Hide preview.
   */
  @Override
  public void hidePreviewClicked(boolean hide) {
    LogUtil.enterBlock("CallButtonPresenter.hidePreviewClicked: " + hide);
    InCallPresenter.getInstance().notifyHideLocalVideoChanged(hide);
  }

  /// M:send cancel upgrade request . @{
  public void cancelUpgradeClicked() {
    LogUtil.enterBlock("CallButtonPresenter.cancelUpgradeClicked");
    if (call == null) {
       LogUtil.w("CallButtonPresenter.cancelUpgradeClicked", "cancelUpgradeVideoRequest failed");
       return;
    }
    call.getVideoTech().cancelUpgradeVideoRequest();
  }
  ///@}

  private void updateExtButtonUI(DialerCall call) {
    final boolean isVideo = call.isVideoCall();
    boolean isCallActive = call.getState() == DialerCallState.ACTIVE;

    /// M: ALPS03949921, disable switch to secondary call because this button will be
    /// enable first before. Even if callcard presenter have disable this button, callbutton
    /// presenter may enable it at last due to the unfixed order of all presenter. @{
    if (call.getState() == DialerCallState.DIALING) {
      inCallButtonUi.enableButton(InCallButtonIds.BUTTON_SWITCH_TO_SECONDARY, false);
    }
    /// @}

    // find whether it's held state, when held state can't do video operation
    boolean currentHeldState = call.isRemotelyHeld();
    final boolean hasCameraPermission =
        isVideo && VideoUtils.hasCameraPermissionAndShownPrivacyToast(context);

    /// hide preview. except : camera off, held call, 3G video call, without permission.
    final boolean showHideLocalVideoBtn = isVideo
        && call.getVideoFeatures().supportsHidePreview()
        && isCallActive && !currentHeldState
        && hasCameraPermission
        && !isSesssionProgressCall(call)
        && call.getVideoState() != VideoProfile.STATE_RX_ENABLED;
    inCallButtonUi.showButton(InCallButtonIds.BUTTON_HIDE_PREVIEW, showHideLocalVideoBtn);

    /// M: [Voice Record] check if should display record @{
    final boolean isUserUnlocked = UserManagerCompat.isUserUnlocked(context);
    final boolean canRecordVoice = call.can(MtkCall.MtkDetails.MTK_CAPABILITY_CALL_RECORDING)
        && !isVideo
        && isUserUnlocked
        && InCallUiCompat.isMtkTelecomCompat();
    final boolean showRecording = UserHandle.myUserId() == UserHandle.USER_OWNER
        && canRecordVoice;
    inCallButtonUi.showButton(InCallButtonIds.BUTTON_SWITCH_VOICE_RECORD, showRecording);
    inCallButtonUi.updateRecordStateUi(call.isRecording());
    /// @}

    /// M: [Hang Up] hang up all/hold calls.
    inCallButtonUi.showButton(InCallButtonIds.BUTTON_HANG_UP_ALL,
        InCallUtils.canHangupAllCalls());
    inCallButtonUi.showButton(InCallButtonIds.BUTTON_HANG_UP_HOLD,
        InCallUtils.canHangupAllHoldCalls());

    /// M: ALPS03881758, disable hangup all and hangup hold button when click merge. @{
    inCallButtonUi.enableButton(InCallButtonIds.BUTTON_HANG_UP_ALL,
            !call.getConferenceMergeStatus());
    inCallButtonUi.enableButton(InCallButtonIds.BUTTON_HANG_UP_HOLD,
            !call.getConferenceMergeStatus());
    /// @}

    /// M: [ECT(blind)]
    inCallButtonUi.showButton(InCallButtonIds.BUTTON_ECT, InCallUtils.canEct());
    inCallButtonUi.showButton(InCallButtonIds.BUTTON_BLIND_ECT, InCallUtils.canBlindEct(call));

    /// M: [One way video] one way video update.
    inCallButtonUi.showButton(InCallButtonIds.BUTTON_ONE_WAY_VIDEO,
        ExtensionManager.getInCallButtonExt().isOneWayVideoSupportedForCall(call));
    ///@}

    /// M: [Device Switch]
    inCallButtonUi.showButton(InCallButtonIds.BUTTON_DEVICE_SWITCH,
        ExtensionManager.getInCallButtonExt().isDeviceSwitchSupported(call));

    /// M:show cancel button after upgrade request is sent @{
    final boolean showCancelUpgrade = !isVideo && isCallActive
        && (call.getVideoTech().getSessionModificationState()
            == SessionModificationState.WAITING_FOR_UPGRADE_TO_VIDEO_RESPONSE)
        && call.getVideoFeatures().supportsCancelUpgradeVideo();

    LogUtil.d("CallButtonPresenter.updateExtButtonUI", "showCancelUpgrade: " + showCancelUpgrade
            + ", session_state: " + call.getVideoTech().getSessionModificationState());
    /// M:now in real network isn't support this feature,so disable cancel button on UI.
    inCallButtonUi.showButton(InCallButtonIds.BUTTON_CANCEL_UPGRADE, false);
    ///@}

    /// M: Add downgrade RTT button for RTT feature. @{
    final boolean showDowngradeRtt = isCallActive
        && call.isActiveRttCall()
        && call.getVideoFeatures().supportsDowngradeRtt();
    inCallButtonUi.showButton(InCallButtonIds.BUTTON_DOWNGRADE_RTT, showDowngradeRtt);
    /// @}

  }

  private boolean isSesssionProgressCall(DialerCall call) {
    //we will set VideoBtn Enable except the instantaneous state
    int sessionState = call.getVideoTech().getSessionModificationState();
    boolean sessionProgress =
        sessionState == SessionModificationState.WAITING_FOR_UPGRADE_TO_VIDEO_RESPONSE
        || sessionState == SessionModificationState.RECEIVED_UPGRADE_TO_VIDEO_REQUEST
        || sessionState == SessionModificationState.WAITING_FOR_RESPONSE
        /// M: for cancel upgrade @{
        || sessionState == SessionModificationState.WAITING_FOR_CANCEL_UPGRADE_RESPONSE;
        /// @}
    return sessionProgress;
  }

  /**
   * M: [Voice Record] Start or stop voice record.
   * @param checked True if start voice recording.
   */
  @Override
  public void toggleVoiceRecord(boolean checked) {
    LogUtil.i("CallButtonPresenter.toggleVoiceRecord", "%s", checked ? "start" : "stop");
    TelecomAdapter.getInstance().toggleVoiceRecording(checked);
  }

  /**
   * M: [ECT(blind)].
   */
  @Override
  public void onBlindOrAssuredEctClicked() {
    if (call == null) {
      return;
    }
    if (context != null) {
      AddTransferNumberScreenController.getInstance().showAddTransferNumberDialog(
          context, call.getId());
    }
  }

  /**
   * M: [ECT(blind)].
   */
  @Override
  public void onConsultativeEctClicked() {
    final DialerCall bgCall = CallList.getInstance().getBackgroundCall();
    if (bgCall != null && bgCall.can(
        mediatek.telecom.MtkCall.MtkDetails.MTK_CAPABILITY_CONSULTATIVE_ECT)) {
      TelecomAdapter.getInstance().explicitCallTransfer(
          bgCall.getTelecomCall().getDetails().getTelecomCallId());
    }
  }

  /// M: [One way video] @{
  @Override
  public void changeToOneWayVideoClicked() {
    if (call == null) {
      return;
    }

    call.getVideoTech().upgradeToOneWayVideo();
  }
  /// @}

  /**
   * M: [Device Switch].
   */
  @Override
  public void onDeviceSwitchClicked() {
    if (call == null) {
      return;
    }

    ExtensionManager.getInCallButtonExt().onMenuItemClick(InCallButtonIds.BUTTON_DEVICE_SWITCH);
  }

  /**
   * M: check and perform pause video without sending SIP message.
   *
   * @param pause Pause/Unpause state of video
   * @return true if supported, false otherwise
   */
  public boolean pauseVideoWithoutSipMessage(boolean pause) {
    if (call == null) {
      return false;
    }
    boolean isSupported = call.getVideoFeatures().isSupportPauseVideoWithoutSipMessage();
    LogUtil.d("CallButtonPresenter.pauseVideoWithoutSipMessage ", " supported:" + isSupported);
    if (!isSupported) {
      return false;
    }
    if (pause) {
      call.getVideoTech().setCamera(null);
      call.setVideoPauseState(pause);
    } else {
      InCallCameraManager cameraManager = InCallPresenter.getInstance().getInCallCameraManager();
      String cameraId = cameraManager.getActiveCameraId();
      call.getVideoTech().setCamera(cameraId);
      call.setVideoPauseState(pause);
    }
    inCallButtonUi.setVideoPaused(pause);
    InCallPresenter.getInstance().notifyPauseLocalVideoChanged(pause);
    return true;
  }

  /// M: Downgrade rtt call. @{
  @Override
  public void downgradeRttClicked() {
    LogUtil.enterBlock("CallButtonPresenter.downgradeRttClicked");
    if (call == null) {
      LogUtil.w("CallButtonPresenter.downgradeRttClicked", "downgradeRttClicked failed");
      return;
    }
    call.sendRttDowngradeRequest();
  }
  /// @}

}
