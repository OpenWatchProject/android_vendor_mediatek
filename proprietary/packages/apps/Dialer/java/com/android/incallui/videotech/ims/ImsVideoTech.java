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

package com.android.incallui.videotech.ims;

import android.content.Context;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.annotation.VisibleForTesting;
import android.telecom.Call;
import android.telecom.Call.Details;
import android.telecom.InCallService.VideoCall;
import android.telecom.PhoneAccountHandle;
import android.telecom.VideoProfile;
import com.android.dialer.common.Assert;
import com.android.dialer.common.LogUtil;
import com.android.dialer.logging.DialerImpression;
import com.android.dialer.logging.LoggingBindings;
import com.android.dialer.util.CallUtil;
import com.android.incallui.video.protocol.VideoCallScreen;
import com.android.incallui.video.protocol.VideoCallScreenDelegate;
import com.android.incallui.InCallPresenter;
import com.android.incallui.videotech.VideoTech;
import com.android.incallui.videotech.utils.SessionModificationState;
import com.android.incallui.R;
import com.mediatek.incallui.video.VideoSessionController;

/** ViLTE implementation */
public class ImsVideoTech implements VideoTech {
  private final LoggingBindings logger;
  private final Call call;
  private final VideoTechListener listener;
  @VisibleForTesting ImsVideoCallCallback callback;
  private @SessionModificationState int sessionModificationState =
      SessionModificationState.NO_REQUEST;
  private int previousVideoState = VideoProfile.STATE_AUDIO_ONLY;
  private boolean paused = false;
  private String savedCameraId;
  /// M: ALPS03515698 Save the previous video call object to call VideoCall.registerCallback
  /// when videocall change. @{
  private VideoCall oldVideCall = null;
  /// @}

  // Hold onto a flag of whether or not stopTransmission was called but resumeTransmission has not
  // been. This is needed because there is time between calling stopTransmission and
  // call.getDetails().getVideoState() reflecting the change. During that time, pause() and
  // unpause() will send the incorrect VideoProfile.
  private boolean transmissionStopped = false;

  public ImsVideoTech(LoggingBindings logger, VideoTechListener listener, Call call) {
    this.logger = logger;
    this.listener = listener;
    this.call = call;
  }

  @Override
  public boolean isAvailable(Context context, PhoneAccountHandle phoneAccountHandle) {
    if (call.getVideoCall() == null) {
      LogUtil.d("ImsVideoCall.isAvailable", "null video call");
      return false;
    }

    // We are already in an IMS video call
    if (VideoProfile.isVideo(call.getDetails().getVideoState())) {
      LogUtil.d("ImsVideoCall.isAvailable", "already video call");
      return true;
    }

    /// M: [Performance] Improve volte mo call performance. @{
    /// Google code:
    /// // The user has disabled IMS video calling in system settings
    /// if (!CallUtil.isVideoEnabled(context)) {
    ///   return false;
    /// }
    /// @}

    // The current call doesn't support transmitting video
    if (!call.getDetails().can(Call.Details.CAPABILITY_SUPPORTS_VT_LOCAL_TX)) {
      LogUtil.d("ImsVideoCall.isAvailable", "no TX");
      return false;
    }

    // The current call remote device doesn't support receiving video
    if (!call.getDetails().can(Call.Details.CAPABILITY_SUPPORTS_VT_REMOTE_RX)) {
      LogUtil.d("ImsVideoCall.isAvailable", "no RX");
      return false;
    }
    LogUtil.d("ImsVideoCall.isAvailable", "available");
    return true;
  }

  @Override
  public boolean isTransmittingOrReceiving() {
    return VideoProfile.isVideo(call.getDetails().getVideoState());
  }

  @Override
  public boolean isSelfManagedCamera() {
    // Return false to indicate that the answer UI shouldn't open the camera itself.
    // For IMS Video the modem is responsible for opening the camera.
    return false;
  }

  @Override
  public boolean shouldUseSurfaceView() {
    return false;
  }

  @Override
  public boolean isPaused() {
    return paused;
  }

  @Override
  public VideoCallScreenDelegate createVideoCallScreenDelegate(
      Context context, VideoCallScreen videoCallScreen) {
    // TODO move creating VideoCallPresenter here
    throw Assert.createUnsupportedOperationFailException();
  }

  @Override
  public void onCallStateChanged(
      Context context, int newState, PhoneAccountHandle phoneAccountHandle) {
    if (!isAvailable(context, phoneAccountHandle)) {
      return;
    }
    /// M: ALPS03515698 Video conference will change videocall when it merge with active call,
    /// So need register callback again. @{
    /// google code:
    /*if (callback == null) {
        callback = new ImsVideoCallCallback(logger, call, this, listener,context);
        //call.getVideoCall().registerCallback(callback);
      }*/
    if (call.getVideoCall() != null) {
      if (callback == null) {
        callback = new ImsVideoCallCallback(logger, call, this, listener, context);
      }
      if (oldVideCall != call.getVideoCall()) {
        call.getVideoCall().registerCallback(callback);
        oldVideCall = call.getVideoCall();
        LogUtil.i("ImsVideoTech.onCallStateChanged", "register callback to videocall");
      }
    }
    /// @}
    if (getSessionModificationState()
            == SessionModificationState.WAITING_FOR_UPGRADE_TO_VIDEO_RESPONSE
        && isTransmittingOrReceiving()) {
      // We don't clear the session modification state right away when we find out the video upgrade
      // request was accepted to avoid having the UI switch from video to voice to video.
      // Once the underlying telecom call updates to video mode it's safe to clear the state.
      LogUtil.i(
          "ImsVideoTech.onCallStateChanged",
          "upgraded to video, clearing session modification state");
      setSessionModificationState(SessionModificationState.NO_REQUEST);
    }

    // Determines if a received upgrade to video request should be cancelled. This can happen if
    // another InCall UI responds to the upgrade to video request.
    int newVideoState = call.getDetails().getVideoState();
    if (newVideoState != previousVideoState
        && sessionModificationState == SessionModificationState.RECEIVED_UPGRADE_TO_VIDEO_REQUEST
        /// M: ALPS03582976: Only cancel upgrade request if video state change to bidirectional.
        /// During TX -> RXTX request, device going to background, then TX -> TX Pause, the request
        /// will be cancelled incorrectly, user has no chance to response this request. @{
        && VideoProfile.isBidirectional(newVideoState)) {
        /// @}
      LogUtil.i("ImsVideoTech.onCallStateChanged", "cancelling upgrade notification");
      setSessionModificationState(SessionModificationState.NO_REQUEST);
    }
    previousVideoState = newVideoState;
  }

  @Override
  public void onRemovedFromCallList() {}

  @Override
  public int getSessionModificationState() {
    return sessionModificationState;
  }

  /// M: ALPS04188332 clear session modification state after SRVCC.
  @Override
  public void setSessionModificationState(@SessionModificationState int state) {
    /// M: ALPS03460044 Auto decline timer, stop timer when SESSION_MODIFICATION_STATE_NO_REQUEST. @{
    if (state == SessionModificationState.NO_REQUEST) {
      com.mediatek.incallui.video.VideoSessionController.getInstance().stopTiming();
    }
    /// @}
    if (state != sessionModificationState) {
      LogUtil.i(
          "ImsVideoTech.setSessionModificationState", "%d -> %d", sessionModificationState, state);
      sessionModificationState = state;
      listener.onSessionModificationStateChanged();
    }
  }

  @Override
  public void upgradeToVideo(@NonNull Context context) {
    LogUtil.enterBlock("ImsVideoTech.upgradeToVideo");
    /// M: ALPS03710396 Video call may be removed before click button, timing issue. @{
    if (call.getVideoCall() == null) {
      LogUtil.i(
          "ImsVideoTech.upgradeToVideo", "Video call already change to null");
      return;
    }
    /// @}

    int unpausedVideoState = getUnpausedVideoState(call.getDetails().getVideoState());
    call.getVideoCall()
        .sendSessionModifyRequest(
            new VideoProfile(unpausedVideoState | VideoProfile.STATE_BIDIRECTIONAL));
    setSessionModificationState(SessionModificationState.WAITING_FOR_UPGRADE_TO_VIDEO_RESPONSE);
    logger.logImpression(DialerImpression.Type.IMS_VIDEO_UPGRADE_REQUESTED);
  }

  @Override
  public void acceptVideoRequest(@NonNull Context context) {
    /// M: ALPS03710396 Video call may be removed before click button, timing issue. @{
    if (call.getVideoCall() == null) {
      LogUtil.i(
          "ImsVideoTech.acceptVideoRequest", "Video call already change to null");
      return;
    }
    /// @}
    int requestedVideoState = callback.getRequestedVideoState();
    Assert.checkArgument(requestedVideoState != VideoProfile.STATE_AUDIO_ONLY);
    LogUtil.i("ImsVideoTech.acceptUpgradeRequest", "videoState: " + requestedVideoState);
    call.getVideoCall().sendSessionModifyResponse(new VideoProfile(requestedVideoState));
    // M: ALPS03967612 clear session state after response.
    setSessionModificationState(SessionModificationState.NO_REQUEST);
    // Telecom manages audio route for us
    listener.onUpgradedToVideo(false /* switchToSpeaker */);
    logger.logImpression(DialerImpression.Type.IMS_VIDEO_REQUEST_ACCEPTED);
  }

  @Override
  public void acceptVideoRequestAsAudio() {
    LogUtil.enterBlock("ImsVideoTech.acceptVideoRequestAsAudio");
    call.getVideoCall().sendSessionModifyResponse(new VideoProfile(VideoProfile.STATE_AUDIO_ONLY));
    // M: ALPS03967612 clear session state after response.
    setSessionModificationState(SessionModificationState.NO_REQUEST);
    logger.logImpression(DialerImpression.Type.IMS_VIDEO_REQUEST_ACCEPTED_AS_AUDIO);
  }

  @Override
  public void declineVideoRequest() {
    LogUtil.enterBlock("ImsVideoTech.declineUpgradeRequest");
    /// M: ALPS04188332 clear session modification state after SRVCC. @{
    if (call.getVideoCall() == null) {
      LogUtil.i(
          "ImsVideoTech.upgradeToVideo", "Video call already change to null");
      return;
    }
    /// @}
    call.getVideoCall()
        .sendSessionModifyResponse(new VideoProfile(call.getDetails().getVideoState()));
    setSessionModificationState(SessionModificationState.NO_REQUEST);
    logger.logImpression(DialerImpression.Type.IMS_VIDEO_REQUEST_DECLINED);
  }

  @Override
  public boolean isTransmitting() {
    return VideoProfile.isTransmissionEnabled(call.getDetails().getVideoState());
  }

  @Override
  public void stopTransmission() {
    LogUtil.enterBlock("ImsVideoTech.stopTransmission");

    transmissionStopped = true;

    int unpausedVideoState = getUnpausedVideoState(call.getDetails().getVideoState());
    call.getVideoCall()
        .sendSessionModifyRequest(
            new VideoProfile(unpausedVideoState & ~VideoProfile.STATE_TX_ENABLED));
  }

  @Override
  public void resumeTransmission(@NonNull Context context) {
    LogUtil.enterBlock("ImsVideoTech.resumeTransmission");

    transmissionStopped = false;

    int unpausedVideoState = getUnpausedVideoState(call.getDetails().getVideoState());
    call.getVideoCall()
        .sendSessionModifyRequest(
            new VideoProfile(unpausedVideoState | VideoProfile.STATE_TX_ENABLED));
    setSessionModificationState(SessionModificationState.WAITING_FOR_RESPONSE);
  }

  @Override
  public void pause() {
    if (call.getState() != Call.STATE_ACTIVE) {
      LogUtil.i("ImsVideoTech.pause", "not pausing because call is not active");
      return;
    }

    if (!isTransmittingOrReceiving()) {
      LogUtil.i("ImsVideoTech.pause", "not pausing because this is not a video call");
      return;
    }
    /// M: ALPS03538275 pause video call fail.video call still keep pause flag when it downgrade
    /// to voice and upgrade video again.so when video call state is audio tx rx,but pause video
    /// fail because of pause flag is true. add check whether the call has in pause state. @{
    /// google original code:
    /// if (paused)
    if (paused && VideoProfile.isPaused(call.getDetails().getVideoState())) {
    /// @}
      LogUtil.i("ImsVideoTech.pause", "already paused");
      return;
    }

    paused = true;

    if (canPause()) {
      LogUtil.i("ImsVideoTech.pause", "sending pause request");
      int pausedVideoState = call.getDetails().getVideoState() | VideoProfile.STATE_PAUSED;
      /// M: ALPS03617139 pause video fail when call change from rx to audio and upgrade to video
      /// again. @{
      /// google code:
      /*if (transmissionStopped && VideoProfile.isTransmissionEnabled(pausedVideoState)) {
        LogUtil.i("ImsVideoTech.pause", "overriding TX to false due to user request");
        pausedVideoState &= ~VideoProfile.STATE_TX_ENABLED;
      }*/
      /// @}
      call.getVideoCall().sendSessionModifyRequest(new VideoProfile(pausedVideoState));
    } else {
      // This video call does not support pause so we fall back to disabling the camera
      LogUtil.i("ImsVideoTech.pause", "disabling camera");
      call.getVideoCall().setCamera(null);
    }
  }

  @Override
  public void unpause() {
    if (call.getState() != Call.STATE_ACTIVE) {
      LogUtil.i("ImsVideoTech.unpause", "not unpausing because call is not active");
      return;
    }

    if (!isTransmittingOrReceiving()) {
      LogUtil.i("ImsVideoTech.unpause", "not unpausing because this is not a video call");
      return;
    }
    /// M: ALPS03527853 unpause video conference call.the normal video call becomes conference call
    /// in background,the conference call will have pause state in videostate.But conference call
    /// can't send unpause because of wrong flag. add check whether the call has in pause state. @{
    /// google original code:
    /// if (!paused) {
    if (!paused && !VideoProfile.isPaused(call.getDetails().getVideoState())) {
    /// @
      LogUtil.i("ImsVideoTech.unpause", "already unpaused");
      return;
    }

    paused = false;

    if (canPause()) {
      LogUtil.i("ImsVideoTech.unpause", "sending unpause request");
      int unpausedVideoState = getUnpausedVideoState(call.getDetails().getVideoState());
      /// M: ALPS03617139 pause video fail  when call change from rx to audio and upgrade to video
      /// again. @{
      /// google code:
      /*if (transmissionStopped && VideoProfile.isTransmissionEnabled(unpausedVideoState)) {
        LogUtil.i("ImsVideoTech.unpause", "overriding TX to false due to user request");
        unpausedVideoState &= ~VideoProfile.STATE_TX_ENABLED;
      }*/
      /// @}
      call.getVideoCall().sendSessionModifyRequest(new VideoProfile(unpausedVideoState));
    } else {
      // This video call does not support pause so we fall back to re-enabling the camera
      LogUtil.i("ImsVideoTech.pause", "re-enabling camera");
      setCamera(savedCameraId);
    }
  }

  @Override
  public void setCamera(@Nullable String cameraId) {
    savedCameraId = cameraId;
    if (call.getVideoCall() == null) {
      LogUtil.w("ImsVideoTech.setCamera", "video call no longer exist");
      return;
    }
    call.getVideoCall().setCamera(cameraId);
    call.getVideoCall().requestCameraCapabilities();
  }

  @Override
  public void setDeviceOrientation(int rotation) {
    /// M:ALPS03571427 Video call released before set. @{
    if (call == null || call.getVideoCall() == null) {
      LogUtil.i(
          "ImsVideoTech.setDeviceOrientation",
          "video call already released");
      return;
    }
    ///@}
    call.getVideoCall().setDeviceOrientation(rotation);
  }

  @Override
  public void becomePrimary() {
    listener.onImpressionLoggingNeeded(
        DialerImpression.Type.UPGRADE_TO_VIDEO_CALL_BUTTON_SHOWN_FOR_IMS);
  }

  @Override
  public com.android.dialer.logging.VideoTech.Type getVideoTechType() {
    return com.android.dialer.logging.VideoTech.Type.IMS_VIDEO_TECH;
  }

  private boolean canPause() {
    return call.getDetails().can(Details.CAPABILITY_CAN_PAUSE_VIDEO);
  }

  static int getUnpausedVideoState(int videoState) {
    return videoState & (~VideoProfile.STATE_PAUSED);
  }

  /// M:--------------MediaTek features-------------
  @Override
  public void downgradeToAudio() {
    LogUtil.enterBlock("ImsVideoTech.downgradeToAudio");

    if (call == null) {
      LogUtil.w("ImsVideoTech.unpause", "downgradeToAudio failed");
      return;
    }

    call.getVideoCall().sendSessionModifyRequest(
        new VideoProfile(VideoProfile.STATE_AUDIO_ONLY));
    setSessionModificationState(SessionModificationState.WAITING_FOR_RESPONSE);
  }

  /// M: [One way video] Send one way video upgrade request with transmission enabled. @{
  @Override
  public void upgradeToOneWayVideo() {
    LogUtil.enterBlock("ImsVideoTech.upgradeToOneWayVideo");
    if (call == null) {
      LogUtil.w("ImsVideoTech.upgradeToOneWayVideo", "upgrade failed");
      return;
    }
    int unpausedVideoState = getUnpausedVideoState(call.getDetails().getVideoState());
    call.getVideoCall()
        .sendSessionModifyRequest(
            new VideoProfile(unpausedVideoState | VideoProfile.STATE_TX_ENABLED));
    setSessionModificationState(SessionModificationState.WAITING_FOR_UPGRADE_TO_VIDEO_RESPONSE);
    logger.logImpression(DialerImpression.Type.IMS_VIDEO_UPGRADE_REQUESTED);
  }

  @Override
  public void acceptVideoRequestAsState(int videoState) {
    LogUtil.enterBlock("ImsVideoTech.acceptVideoRequestWithState");
    call.getVideoCall().sendSessionModifyResponse(new VideoProfile(videoState));
    setSessionModificationState(SessionModificationState.NO_REQUEST);
  }
  /// @}

 /// M:send cancel upgrade request and if the timer of cancel upgrade has started ,should
 /// stop the timer . @{
 public void cancelUpgradeVideoRequest() {
   LogUtil.enterBlock("ImsVideoTech.cancelUpgradeVideoRequest");
   if (call == null) {
     LogUtil.w("ImsVideoTech.cancelUpgradeVideoRequest", "cancelUpgradeVideoRequest failed");
     return;
   }
   VideoCall videoCall = call.getVideoCall();
   if (videoCall == null) {
     return;
   }

   if (sessionModificationState ==
       SessionModificationState.WAITING_FOR_CANCEL_UPGRADE_RESPONSE) {
     return;
   }

   videoCall.sendSessionModifyRequest(new VideoProfile(
       mediatek.telecom.MtkVideoProfile.STATE_CANCEL_UPGRADE));
   setSessionModificationState(SessionModificationState.WAITING_FOR_CANCEL_UPGRADE_RESPONSE);
   VideoSessionController.getInstance().stopTiming();
 }
 /// @}
}
