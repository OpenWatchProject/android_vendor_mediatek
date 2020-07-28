/*
 * Copyright (C) 2014 The Android Open Source Project
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
import android.content.Intent;
import android.net.Uri;
import android.os.IBinder;
import android.os.Trace;
import android.telecom.Call;
import android.telecom.CallAudioState;
import android.telecom.InCallService;

import com.android.contacts.common.compat.CallCompat;
import com.android.dialer.blocking.FilteredNumberAsyncQueryHandler;
import com.android.dialer.common.LogUtil;
import com.android.dialer.feedback.FeedbackComponent;
import com.android.dialer.notification.NotificationChannelManager;
import com.android.incallui.audiomode.AudioModeProvider;
import com.android.incallui.call.CallList;
import com.android.incallui.call.ExternalCallList;
import com.android.incallui.call.TelecomAdapter;
import com.android.incallui.speakeasy.SpeakEasyCallManager;
import com.android.incallui.speakeasy.SpeakEasyComponent;
import com.mediatek.incallui.plugin.ExtensionManager;
import com.mediatek.incallui.utils.InCallUtils;

/**
 * Used to receive updates about calls from the Telecom component. This service is bound to Telecom
 * while there exist calls which potentially require UI. This includes ringing (incoming), dialing
 * (outgoing), and active calls. When the last call is disconnected, Telecom will unbind to the
 * service triggering InCallActivity (via CallList) to finish soon after.
 */
public class InCallServiceImpl extends InCallService {

  private ReturnToCallController returnToCallController;
  private CallList.Listener feedbackListener;
  // We only expect there to be one speakEasyCallManager to be instantiated at a time.
  // We did not use a singleton SpeakEasyCallManager to avoid holding on to state beyond the
  // lifecycle of this service, because the singleton is associated with the state of the
  // Application, not this service.
  private SpeakEasyCallManager speakEasyCallManager;

  @Override
  public void onCallAudioStateChanged(CallAudioState audioState) {
    Trace.beginSection("InCallServiceImpl.onCallAudioStateChanged");
    AudioModeProvider.getInstance().onAudioStateChanged(audioState);
    Trace.endSection();
  }

  @Override
  public void onBringToForeground(boolean showDialpad) {
    Trace.beginSection("InCallServiceImpl.onBringToForeground");
    InCallPresenter.getInstance().onBringToForeground(showDialpad);
    Trace.endSection();
  }

  @Override
  public void onCallAdded(Call call) {
    Trace.beginSection("InCallServiceImpl.onCallAdded");
    /**
     * M: When in upgrade progress or in requesting for VILTE call,
     * It should reject the incoming call and disconnect other calls,
     * except the emergency call.
     * @{
     */
    if ((CallList.getInstance().getVideoUpgradeRequestCall() != null ||
        CallList.getInstance().getSendingVideoUpgradeRequestCall() != null ||
        /// M: When is cancel upgrade progress,we can't add another call in calllist. @{
        CallList.getInstance().getSendingCancelUpgradeRequestCall() != null)
        ///@}
        && !isEmergency(call)
        /// M: ALPS03795769 Fix merge and upgrade conflict issue @{
        && !call.getDetails().hasProperty(CallCompat.Details.PROPERTY_CONFERENCE_PARTICIPANT)) {
        ///@}
      if (call.getState() == Call.STATE_RINGING) {
        call.reject(false, null);
      } else {
        call.disconnect();
      }
      LogUtil.d("InCallServiceImpl.onCallAdded","[Debug][CC][InCallUI][OP][Hangup][null][null]" +
          "auto disconnect call while upgrading to video");
      InCallUtils.showOutgoingFailMsg(getApplicationContext(), call);
    } else {
      InCallPresenter.getInstance().onCallAdded(call);
    }
    /** @} */
    Trace.endSection();
  }

  @Override
  public void onCallRemoved(Call call) {
    Trace.beginSection("InCallServiceImpl.onCallRemoved");
    speakEasyCallManager.onCallRemoved(CallList.getInstance().getDialerCallFromTelecomCall(call));

    InCallPresenter.getInstance().onCallRemoved(call);
    Trace.endSection();
  }

  @Override
  public void onCanAddCallChanged(boolean canAddCall) {
    Trace.beginSection("InCallServiceImpl.onCanAddCallChanged");
    InCallPresenter.getInstance().onCanAddCallChanged(canAddCall);
    Trace.endSection();
  }

  @Override
  public void onCreate() {
    super.onCreate();
    this.speakEasyCallManager = SpeakEasyComponent.get(this).speakEasyCallManager();
  }

  @Override
  public IBinder onBind(Intent intent) {
    Trace.beginSection("InCallServiceImpl.onBind");
    LogUtil.d("InCallServiceImpl.onBind", "onBind");
    final Context context = getApplicationContext();
    /// M: [Plugin Host] register context @{
    ExtensionManager.registerApplicationContext(context);
    /// @}
    final ContactInfoCache contactInfoCache = ContactInfoCache.getInstance(context);
    AudioModeProvider.getInstance().initializeAudioState(this);
    /// M: ALPS04880502, dialer crash during make outgoing call after reset data in Setting.
    /// Need recreate notification channel if call channels do not exist. @{
    if (NotificationChannelManager.shouldInitChannels(context)) {
        NotificationChannelManager.initChannels(context);
    }
    /// @}
    InCallPresenter.getInstance()
        .setUp(
            context,
            CallList.getInstance(),
            new ExternalCallList(),
            new StatusBarNotifier(context, contactInfoCache),
            new ExternalCallNotifier(context, contactInfoCache),
            contactInfoCache,
            new ProximitySensor(
                context, AudioModeProvider.getInstance(), new AccelerometerListener(context)),
            new FilteredNumberAsyncQueryHandler(context),
            speakEasyCallManager);
    InCallPresenter.getInstance().onServiceBind();
    InCallPresenter.getInstance().maybeStartRevealAnimation(intent);
    TelecomAdapter.getInstance().setInCallService(this);
    returnToCallController =
        new ReturnToCallController(this, ContactInfoCache.getInstance(context));
    feedbackListener = FeedbackComponent.get(context).getCallFeedbackListener();
    CallList.getInstance().addListener(feedbackListener);

    /// M: for operator pct vilte auto test on instrument @{
    ExtensionManager.getVilteAutoTestHelperExt().registerReceiver(context,
        InCallPresenter.getInstance(),TelecomAdapter.getInstance());
    /// @}
    IBinder iBinder = super.onBind(intent);
    Trace.endSection();
    return iBinder;
  }

  @Override
  public boolean onUnbind(Intent intent) {
    Trace.beginSection("InCallServiceImpl.onUnbind");
    super.onUnbind(intent);

    InCallPresenter.getInstance().onServiceUnbind();
    tearDown();

    Trace.endSection();
    return false;
  }

  private void tearDown() {
    Trace.beginSection("InCallServiceImpl.tearDown");
    Log.v(this, "tearDown");
    // Tear down the InCall system
    InCallPresenter.getInstance().tearDown();
    TelecomAdapter.getInstance().clearInCallService();
    if (returnToCallController != null) {
      returnToCallController.tearDown();
      returnToCallController = null;
    }
    if (feedbackListener != null) {
      CallList.getInstance().removeListener(feedbackListener);
      feedbackListener = null;
    }
    /// M: for operator pct vilte auto test on instrument @{
    ExtensionManager.getVilteAutoTestHelperExt().unregisterReceiver();
    /// @}
    Trace.endSection();
  }

  /// M: ---------------- MediaTek feature -------------------
  /// M: fix CR:ALPS02696713,can not dial ECC when requesting for vilte call. @{
  private boolean isEmergency(Call call) {
    Uri handle = call.getDetails().getHandle();
    return android.telephony.PhoneNumberUtils.isEmergencyNumber(
        handle == null ? "" : handle.getSchemeSpecificPart());
  }
  /// @}
}
