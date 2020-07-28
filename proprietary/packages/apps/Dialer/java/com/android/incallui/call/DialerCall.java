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
 * limitations under the License.
 */

package com.android.incallui.call;

import android.Manifest.permission;
import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.content.Context;
import android.content.pm.PackageManager;
import android.hardware.camera2.CameraCharacteristics;
import android.net.Uri;
import android.os.Build;
import android.os.Build.VERSION;
import android.os.Build.VERSION_CODES;
import android.os.Bundle;
import android.os.PersistableBundle;
import android.os.SystemClock;
import android.os.Trace;
import android.support.annotation.IntDef;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.annotation.VisibleForTesting;
import android.support.v4.content.ContextCompat;
import android.support.v4.os.BuildCompat;
import android.telecom.Call;
import android.telecom.Call.Details;
import android.telecom.Call.RttCall;
import android.telecom.CallAudioState;
import android.telecom.Connection;
import android.telecom.DisconnectCause;
import android.telecom.GatewayInfo;
import android.telecom.InCallService.VideoCall;
import android.telecom.PhoneAccount;
import android.telecom.PhoneAccountHandle;
import android.telecom.StatusHints;
import android.telecom.TelecomManager;
import android.telecom.VideoProfile;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.widget.Toast;
import com.android.contacts.common.compat.CallCompat;
import com.android.dialer.assisteddialing.ConcreteCreator;
import com.android.dialer.assisteddialing.TransformationInfo;
import com.android.dialer.blocking.FilteredNumbersUtil;
import com.android.dialer.callintent.CallInitiationType;
import com.android.dialer.callintent.CallIntentParser;
import com.android.dialer.callintent.CallSpecificAppData;
import com.android.dialer.common.Assert;
import com.android.dialer.common.LogUtil;
import com.android.dialer.common.concurrent.DefaultFutureCallback;
import com.android.dialer.compat.telephony.TelephonyManagerCompat;
import com.android.dialer.configprovider.ConfigProviderComponent;
import com.android.dialer.duo.DuoComponent;
import com.android.dialer.enrichedcall.EnrichedCallCapabilities;
import com.android.dialer.enrichedcall.EnrichedCallComponent;
import com.android.dialer.enrichedcall.EnrichedCallManager;
import com.android.dialer.enrichedcall.EnrichedCallManager.CapabilitiesListener;
import com.android.dialer.enrichedcall.EnrichedCallManager.Filter;
import com.android.dialer.enrichedcall.EnrichedCallManager.StateChangedListener;
import com.android.dialer.enrichedcall.Session;
import com.android.dialer.location.GeoUtil;
import com.android.dialer.logging.ContactLookupResult;
import com.android.dialer.logging.ContactLookupResult.Type;
import com.android.dialer.logging.DialerImpression;
import com.android.dialer.logging.Logger;
import com.android.dialer.preferredsim.PreferredAccountRecorder;
import com.android.dialer.preferredsim.suggestion.SuggestionProvider;
import com.android.dialer.rtt.RttTranscript;
import com.android.dialer.rtt.RttTranscriptUtil;
import com.android.dialer.spam.status.SpamStatus;
import com.android.dialer.telecom.TelecomCallUtil;
import com.android.dialer.telecom.TelecomUtil;
import com.android.dialer.theme.common.R;
import com.android.dialer.time.Clock;
import com.android.dialer.util.PermissionsUtil;
import com.android.incallui.audiomode.AudioModeProvider;
import com.android.incallui.call.state.DialerCallState;
import com.android.incallui.InCallPresenter;
import com.android.incallui.latencyreport.LatencyReport;
import com.android.incallui.rtt.protocol.RttChatMessage;
import com.android.incallui.videotech.VideoTech;
import com.android.incallui.videotech.VideoTech.VideoTechListener;
import com.android.incallui.videotech.duo.DuoVideoTech;
import com.android.incallui.videotech.empty.EmptyVideoTech;
import com.android.incallui.videotech.ims.ImsVideoTech;
import com.android.incallui.videotech.utils.VideoUtils;
import com.android.incallui.videotech.utils.SessionModificationState;
import com.google.common.base.Optional;
import com.google.common.util.concurrent.Futures;
import com.google.common.util.concurrent.MoreExecutors;
import com.mediatek.cta.CtaManager;
import com.mediatek.cta.CtaManagerFactory;
import com.mediatek.incallui.CallDetailChangeHandler;
import com.mediatek.incallui.plugin.ExtensionManager;
import com.mediatek.incallui.video.VideoFeatures;
import com.mediatek.incallui.video.VideoSessionController;
import com.mediatek.incallui.volte.ConferenceChildrenChangeHandler;
import com.mediatek.incallui.volte.InCallUIVolteUtils;

import mediatek.telecom.MtkCall;
import mediatek.telecom.MtkCall.MtkDetails;
import mediatek.telecom.MtkConnection.MtkVideoProvider;
import mediatek.telecom.MtkTelecomManager;


import java.io.IOException;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Locale;
import java.util.Objects;
import java.util.UUID;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.TimeUnit;



/** Describes a single call and its state. */
public class DialerCall implements VideoTechListener, StateChangedListener, CapabilitiesListener {

  public static final int CALL_HISTORY_STATUS_UNKNOWN = 0;
  public static final int CALL_HISTORY_STATUS_PRESENT = 1;
  public static final int CALL_HISTORY_STATUS_NOT_PRESENT = 2;

  // Hard coded property for {@code Call}. Upstreamed change from Motorola.
  // TODO(a bug): Move it to Telecom in framework.
  public static final int PROPERTY_CODEC_KNOWN = 0x04000000;

  private static final String ID_PREFIX = "DialerCall_";

  @VisibleForTesting
  public static final String CONFIG_EMERGENCY_CALLBACK_WINDOW_MILLIS =
      "emergency_callback_window_millis";

  private static int idCounter = 0;

  /**
   * A counter used to append to restricted/private/hidden calls so that users can identify them in
   * a conversation. This value is reset in {@link CallList#onCallRemoved(Context, Call)} when there
   * are no live calls.
   */
  private static int hiddenCounter;

  /**
   * The unique call ID for every call. This will help us to identify each call and allow us the
   * ability to stitch impressions to calls if needed.
   */
  private final String uniqueCallId = UUID.randomUUID().toString();

  private final Call telecomCall;
  private final LatencyReport latencyReport;
  private final String id;
  private final int hiddenId;
  private final List<String> childCallIds = new ArrayList<>();
  private final LogState logState = new LogState();
  private final Context context;
  private final DialerCallDelegate dialerCallDelegate;
  private final List<DialerCallListener> listeners = new CopyOnWriteArrayList<>();
  private final List<CannedTextResponsesLoadedListener> cannedTextResponsesLoadedListeners =
      new CopyOnWriteArrayList<>();
  private final VideoTechManager videoTechManager;

  private boolean isSpeakEasyCall;
  private boolean isEmergencyCall;
  private Uri handle;
  private int state = DialerCallState.INVALID;
  private DisconnectCause disconnectCause;

  private boolean hasShownLteToWiFiHandoverToast;
  private boolean hasShownWiFiToLteHandoverToast;
  private boolean doNotShowDialogForHandoffToWifiFailure;

  private String childNumber;
  private String lastForwardedNumber;
  private boolean isCallForwarded;
  private String callSubject;
  @Nullable private PhoneAccountHandle phoneAccountHandle;
  @CallHistoryStatus private int callHistoryStatus = CALL_HISTORY_STATUS_UNKNOWN;

  @Nullable private SpamStatus spamStatus;
  private boolean isBlocked;

  private boolean didShowCameraPermission;
  private boolean didDismissVideoChargesAlertDialog;
  private PersistableBundle carrierConfig;
  private String callProviderLabel;
  private String callbackNumber;
  private int cameraDirection = CameraDirection.CAMERA_DIRECTION_UNKNOWN;
  private EnrichedCallCapabilities enrichedCallCapabilities;
  private Session enrichedCallSession;

  private int answerAndReleaseButtonDisplayedTimes = 0;
  private boolean releasedByAnsweringSecondCall = false;
  // Times when a second call is received but AnswerAndRelease button is not shown
  // since it's not supported.
  private int secondCallWithoutAnswerAndReleasedButtonTimes = 0;
  private VideoTech videoTech;

  private com.android.dialer.logging.VideoTech.Type selectedAvailableVideoTechType =
      com.android.dialer.logging.VideoTech.Type.NONE;
  private boolean isVoicemailNumber;
  private List<PhoneAccountHandle> callCapableAccounts;
  private String countryIso;

  private volatile boolean feedbackRequested = false;

  private Clock clock = System::currentTimeMillis;

  @Nullable private PreferredAccountRecorder preferredAccountRecorder;
  private boolean isCallRemoved;

  public static String getNumberFromHandle(Uri handle) {
    return handle == null ? "" : handle.getSchemeSpecificPart();
  }

  /**
   * Whether the call is put on hold by remote party. This is different than the {@link
   * DialerCallState#ONHOLD} state which indicates that the call is being held locally on the
   * device.
   */
  private boolean isRemotelyHeld;

  /** Indicates whether this call is currently in the process of being merged into a conference. */
  private boolean isMergeInProcess;

  /**
   * Indicates whether the phone account associated with this call supports specifying a call
   * subject.
   */
  private boolean isCallSubjectSupported;

  public RttTranscript getRttTranscript() {
    return rttTranscript;
  }

  public void setRttTranscript(RttTranscript rttTranscript) {
    this.rttTranscript = rttTranscript;
  }

  private RttTranscript rttTranscript;

  /// M: [Modification for finishing Transparent InCall Screen if necessary]
  /// such as:ALPS03477988,select sim press home key, show select again.
  /// such as:ALPS02302461,occur JE when MT call arrive at some case. @{
  private int stateEx = DialerCallState.INVALID;
  /// @}

  private final Call.Callback telecomCallCallback =
      new Call.Callback() {
        @Override
        public void onStateChanged(Call call, int newState) {
          LogUtil.v("TelecomCallCallback.onStateChanged", "call=" + call + " newState=" + newState);
          /// M: [Performance] Reduce state change to improve performance. @{
          /// Google code: update();
          stateUpdate();
          /// @}
        }

        @Override
        public void onParentChanged(Call call, Call newParent) {
          LogUtil.v(
              "TelecomCallCallback.onParentChanged", "call=" + call + " newParent=" + newParent);
          /// M: [VoLTE Conference] Print parnet call and child call id mapping to telecom id @{
          printParnetChanged(call, newParent);
          /// @}
          update();

          /// M: ALPS03881758, reset status when parent change. @{
          duringConferenceMerge = false;
          /// @}
        }

        @Override
        public void onChildrenChanged(Call call, List<Call> children) {
          update();
          /// M: [VoLTE Conference] Print parnet call and child call id mapping to telecom id @{
          printChildrenChanged(call, children);
          handleChildrenChanged();
          /// @}

          /// M: ALPS03881758, reset status when children change. @{
          duringConferenceMerge = false;
          /// @}
        }

        @Override
        public void onDetailsChanged(Call call, Call.Details details) {
          LogUtil.v(
              "TelecomCallCallback.onDetailsChanged", " call=" + call + " details=" + details);

          /// M: [Performance] Check details change to increase performance. @{
          boolean forceUpdate = forceUpdateDetailByState();
          Call.Details newDetail = telecomCall.getDetails();
          if (!forceUpdate &&
              skipUpdateDetails(oldDetails, newDetail)) {
            LogUtil.d("TelecomCallCallback.onDetailsChanged", "Skip details change.");
            return;
          }

          if (translateState(telecomCall.getState()) == DialerCallState.DISCONNECTING) {
            LogUtil.d("TelecomCallCallback.onDetailsChanged", "Ignore disconnecting.");
            return;
          }
          /// @}

          update();
          /// M: [VoLTE Conference] @{
          handleDetailsChanged(newDetail);
          /// @}
        }

        @Override
        public void onCannedTextResponsesLoaded(Call call, List<String> cannedTextResponses) {
          LogUtil.v(
              "TelecomCallCallback.onCannedTextResponsesLoaded",
              "call=" + call + " cannedTextResponses=" + cannedTextResponses);
          for (CannedTextResponsesLoadedListener listener : cannedTextResponsesLoadedListeners) {
            listener.onCannedTextResponsesLoaded(DialerCall.this);
          }
        }

        @Override
        public void onPostDialWait(Call call, String remainingPostDialSequence) {
          LogUtil.v(
              "TelecomCallCallback.onPostDialWait",
              "call=" + call + " remainingPostDialSequence=" + remainingPostDialSequence);
          update();
        }

        @Override
        public void onVideoCallChanged(Call call, VideoCall videoCall) {
          LogUtil.v(
              "TelecomCallCallback.onVideoCallChanged", "call=" + call + " videoCall=" + videoCall);

          /// M: ALPS04188332 clear session modification state after SRVCC.when SRVCC happens
          ///before the user responses to upgrade to video request,the user will can't success to
          ///responses to the request.The end button don't show in this case.So the user will
          ///keep on receiving upgrade to video request state.the rootcase is videocall is set
          ///null after SRVCC. @{
          if (videoCall == null && videoTech != null && videoTech.getSessionModificationState() ==
              SessionModificationState.RECEIVED_UPGRADE_TO_VIDEO_REQUEST) {
            LogUtil.v(
              "TelecomCallCallback.onVideoCallChanged", "clear session modification");
            videoTech.setSessionModificationState(SessionModificationState.NO_REQUEST);
          }
          /// @}

          /// M: [Performance] Improve volte mo call performance. @{
          /// Google code: update();
          oldDetails = telecomCall.getDetails();
          update();
          /// @}
        }

        @Override
        public void onCallDestroyed(Call call) {
          LogUtil.v("TelecomCallCallback.onCallDestroyed", "call=" + call);
          unregisterCallback();
        }

        @Override
        public void onConferenceableCallsChanged(Call call, List<Call> conferenceableCalls) {
          LogUtil.v(
              "TelecomCallCallback.onConferenceableCallsChanged",
              "call %s, conferenceable calls: %d",
              call,
              conferenceableCalls.size());
          update();
        }

        @Override
        public void onRttModeChanged(Call call, int mode) {
          LogUtil.v("TelecomCallCallback.onRttModeChanged", "mode=%d", mode);
        }

        @Override
        public void onRttRequest(Call call, int id) {
          LogUtil.v("TelecomCallCallback.onRttRequest", "id=%d", id);
          /// M: Auto accept upgrade rtt request. @{
          if (videoFeatures!= null && videoFeatures.supportsAutoAcceptUpgradeRttRequest()) {
            LogUtil.v("TelecomCallCallback.onRttRequest", "auto accept upgrade rtt request");
            respondToRttRequest(true, id);
            return;
          }
          /// @}
          for (DialerCallListener listener : listeners) {
            listener.onDialerCallUpgradeToRtt(id);
          }
        }

        @Override
        public void onRttInitiationFailure(Call call, int reason) {
          LogUtil.v("TelecomCallCallback.onRttInitiationFailure", "reason=%d", reason);
          Toast.makeText(context, R.string.rtt_call_not_available_toast, Toast.LENGTH_LONG).show();
          update();
        }

        @Override
        public void onRttStatusChanged(Call call, boolean enabled, RttCall rttCall) {
          LogUtil.v("TelecomCallCallback.onRttStatusChanged", "enabled=%b", enabled);
          if (enabled) {
            Logger.get(context)
                .logCallImpression(
                    DialerImpression.Type.RTT_MID_CALL_ENABLED,
                    getUniqueCallId(),
                    getTimeAddedMs());
          }
          update();
        }

        @Override
        public void onConnectionEvent(android.telecom.Call call, String event, Bundle extras) {
          LogUtil.v(
              "TelecomCallCallback.onConnectionEvent",
              "Call: " + call + ", Event: " + event + ", Extras: " + extras);
          switch (event) {
              // The Previous attempt to Merge two calls together has failed in Telecom. We must
              // now update the UI to possibly re-enable the Merge button based on the number of
              // currently conferenceable calls available or Connection Capabilities.
            case android.telecom.Connection.EVENT_CALL_MERGE_FAILED:
              update();
              /// M: ALPS03881758, reset status when merge failed. @{
              duringConferenceMerge = false;
              /// @}
              break;
            case TelephonyManagerCompat.EVENT_HANDOVER_VIDEO_FROM_WIFI_TO_LTE:
              notifyWiFiToLteHandover();
              break;
            case TelephonyManagerCompat.EVENT_HANDOVER_VIDEO_FROM_LTE_TO_WIFI:
              onLteToWifiHandover();
              break;
            case TelephonyManagerCompat.EVENT_HANDOVER_TO_WIFI_FAILED:
              notifyHandoverToWifiFailed();
              break;
            case TelephonyManagerCompat.EVENT_CALL_REMOTELY_HELD:
              isRemotelyHeld = true;
              update();
              break;
            case TelephonyManagerCompat.EVENT_CALL_REMOTELY_UNHELD:
              isRemotelyHeld = false;
              update();
              break;
            case TelephonyManagerCompat.EVENT_NOTIFY_INTERNATIONAL_CALL_ON_WFC:
              notifyInternationalCallOnWifi();
              break;
            case TelephonyManagerCompat.EVENT_MERGE_START:
              LogUtil.i("DialerCall.onConnectionEvent", "merge start");
              isMergeInProcess = true;
              break;
            case TelephonyManagerCompat.EVENT_MERGE_COMPLETE:
              LogUtil.i("DialerCall.onConnectionEvent", "merge complete");
              isMergeInProcess = false;
              /// M: ALPS04765852  force update ui when hold call merge complete. @{
              if (translateState(telecomCall.getState()) == DialerCallState.ONHOLD) {
                LogUtil.d("TelecomCallCallback.onConnectionEvent", "update ui after hold call merge complete");
                update();
              }
              ///@}
              break;
            case TelephonyManagerCompat.EVENT_CALL_FORWARDED:
              // Only handle this event for P+ since it's unreliable pre-P.
              if (BuildCompat.isAtLeastP()) {
                isCallForwarded = true;
                update();
              }
              break;
            /// M: these toast is for TMO RTT feature. @{
            case mediatek.telecom.MtkConnection.EVENT_SRVCC:
              InCallPresenter.getInstance().showMessageWithLongDuration(com.android.incallui.
                  R.string.srvcc_happend_not_support_rtt);
              break;
            case mediatek.telecom.MtkConnection.EVENT_CSFB:
               if (isEmergencyCall()) {
                 InCallPresenter.getInstance().showMessageWithLongDuration(com.android.incallui.
                     R.string.not_support_rtt_emergency_redial);

               } else {
                 InCallPresenter.getInstance().showMessage(com.android.incallui.
                     R.string.csfb_happend);
               }
              break;
            case mediatek.telecom.MtkConnection.EVENT_RTT_UPDOWN_FAIL:
              InCallPresenter.getInstance().showMessage(com.android.incallui.
                  R.string.rtt_updown_fail);
              break;
            case mediatek.telecom.MtkConnection.EVENT_RTT_EMERGENCY_REDIAL:
              InCallPresenter.getInstance().showMessageWithLongDuration(com.android.incallui.
                  R.string.not_support_rtt_emergency_redial);
              break;
            /// M: ALPS03729639. Handle operation failed event.
            /// Handle operation failed event to check for hold call failed
            /// and update the hold call button state to unchecked. @{
            case mediatek.telecom.MtkConnection.EVENT_OPERATION_FAILED:
              LogUtil.i("DialerCall.onConnectionEvent", "Operation failed");
              update();
              break;
            /// @}
            default:
              break;
          }
          /// M: [Device Switch] @{
          ExtensionManager.getInCallExt().onConnectionEvent(call, event, extras);
          /// @}
        }
      };

  private long timeAddedMs;

  public DialerCall(
      Context context,
      DialerCallDelegate dialerCallDelegate,
      Call telecomCall,
      LatencyReport latencyReport,
      boolean registerCallback) {
    Assert.isNotNull(context);
    this.context = context;
    this.dialerCallDelegate = dialerCallDelegate;
    this.telecomCall = telecomCall;
    this.latencyReport = latencyReport;
    id = ID_PREFIX + Integer.toString(idCounter++);

    // Must be after assigning mTelecomCall
    videoTechManager = new VideoTechManager(this);

    /// M: [Performance] Check details change to increase performance. @{
    oldDetails = telecomCall.getDetails();
    ///@}

    updateFromTelecomCall();
    if (isHiddenNumber() && TextUtils.isEmpty(getNumber())) {
      hiddenId = ++hiddenCounter;
    } else {
      hiddenId = 0;
    }

    if (registerCallback) {
      this.telecomCall.registerCallback(telecomCallCallback);
    }

    timeAddedMs = System.currentTimeMillis();
    parseCallSpecificAppData();

    updateEnrichedCallSession();

    /// M: [voice call]manage video call features.
    videoFeatures = new VideoFeatures(this);

    /// M: ALPS03719925 Hide preview view when it's video conferene call.
    /// setHidePreview is only for video call, no need to check if it's video call. @{
    if (isConferenceCall()) {
        LogUtil.d("DialerCall.DialerCall", "Conference call");
        setHidePreview(true);
    }
    /// @}

    /// M: Add to filter bundle list in OP plugins. @{
    ExtensionManager.getInCallExt().addFilterBundle(allUsedBundleList, allUsedExtraBundleList);
    /// @}
  }

  private static int translateState(int state) {
    switch (state) {
      case Call.STATE_NEW:
      case Call.STATE_CONNECTING:
        return DialerCallState.CONNECTING;
      case Call.STATE_SELECT_PHONE_ACCOUNT:
        return DialerCallState.SELECT_PHONE_ACCOUNT;
      case Call.STATE_DIALING:
        return DialerCallState.DIALING;
      case Call.STATE_PULLING_CALL:
        return DialerCallState.PULLING;
      case Call.STATE_RINGING:
        return DialerCallState.INCOMING;
      case Call.STATE_ACTIVE:
        return DialerCallState.ACTIVE;
      case Call.STATE_HOLDING:
        return DialerCallState.ONHOLD;
      case Call.STATE_DISCONNECTED:
        return DialerCallState.DISCONNECTED;
      case Call.STATE_DISCONNECTING:
        return DialerCallState.DISCONNECTING;
      default:
        return DialerCallState.INVALID;
    }
  }

  public static boolean areSame(DialerCall call1, DialerCall call2) {
    if (call1 == null && call2 == null) {
      return true;
    } else if (call1 == null || call2 == null) {
      return false;
    }

    // otherwise compare call Ids
    return call1.getId().equals(call2.getId());
  }

  public void addListener(DialerCallListener listener) {
    Assert.isMainThread();
    listeners.add(listener);
  }

  public void removeListener(DialerCallListener listener) {
    Assert.isMainThread();
    listeners.remove(listener);
  }

  public void addCannedTextResponsesLoadedListener(CannedTextResponsesLoadedListener listener) {
    Assert.isMainThread();
    cannedTextResponsesLoadedListeners.add(listener);
  }

  public void removeCannedTextResponsesLoadedListener(CannedTextResponsesLoadedListener listener) {
    Assert.isMainThread();
    cannedTextResponsesLoadedListeners.remove(listener);
  }

  private void onLteToWifiHandover() {
    LogUtil.enterBlock("DialerCall.onLteToWifiHandover");
    if (hasShownLteToWiFiHandoverToast) {
      return;
    }

    Toast.makeText(context, R.string.video_call_lte_to_wifi_handover_toast, Toast.LENGTH_LONG)
        .show();
    hasShownLteToWiFiHandoverToast = true;
  }

  public void notifyWiFiToLteHandover() {
    LogUtil.i("DialerCall.notifyWiFiToLteHandover", "");
    for (DialerCallListener listener : listeners) {
      listener.onWiFiToLteHandover();
    }
  }

  public void notifyHandoverToWifiFailed() {
    LogUtil.i("DialerCall.notifyHandoverToWifiFailed", "");
    for (DialerCallListener listener : listeners) {
      listener.onHandoverToWifiFailure();
    }
  }

  public void notifyInternationalCallOnWifi() {
    LogUtil.enterBlock("DialerCall.notifyInternationalCallOnWifi");
    for (DialerCallListener dialerCallListener : listeners) {
      dialerCallListener.onInternationalCallOnWifi();
    }
  }

  /// M:/* package-private */
  public Call getTelecomCall() {
    return telecomCall;
  }

  public StatusHints getStatusHints() {
    return telecomCall.getDetails().getStatusHints();
  }

  public int getCameraDir() {
    return cameraDirection;
  }

  public void setCameraDir(int cameraDir) {
    if (cameraDir == CameraDirection.CAMERA_DIRECTION_FRONT_FACING
        || cameraDir == CameraDirection.CAMERA_DIRECTION_BACK_FACING) {
      cameraDirection = cameraDir;
    } else {
      cameraDirection = CameraDirection.CAMERA_DIRECTION_UNKNOWN;
    }
  }

  public boolean wasParentCall() {
    return logState.conferencedCalls != 0;
  }

  public boolean isVoiceMailNumber() {
    return isVoicemailNumber;
  }

  public List<PhoneAccountHandle> getCallCapableAccounts() {
    return callCapableAccounts;
  }

  public String getCountryIso() {
    return countryIso;
  }

  private void updateIsVoiceMailNumber() {
    if (getHandle() != null && PhoneAccount.SCHEME_VOICEMAIL.equals(getHandle().getScheme())) {
      isVoicemailNumber = true;
      return;
    }

    if (!PermissionsUtil.hasPermission(context, permission.READ_PHONE_STATE)) {
      isVoicemailNumber = false;
      return;
    }

    isVoicemailNumber = TelecomUtil.isVoicemailNumber(context, getAccountHandle(), getNumber());
  }

  private void update() {
    Trace.beginSection("DialerCall.update");
    int oldState = getState();
    // Clear any cache here that could potentially change on update.
    videoTech = null;
    // We want to potentially register a video call callback here.
    updateFromTelecomCall();
    if (oldState != getState() && getState() == DialerCallState.DISCONNECTED) {
      for (DialerCallListener listener : listeners) {
        listener.onDialerCallDisconnect();
      }
      EnrichedCallComponent.get(context)
          .getEnrichedCallManager()
          .unregisterCapabilitiesListener(this);
      EnrichedCallComponent.get(context)
          .getEnrichedCallManager()
          .unregisterStateChangedListener(this);
    } else {
      for (DialerCallListener listener : listeners) {
        listener.onDialerCallUpdate();
      }
    }
    Trace.endSection();
  }

  @SuppressWarnings("MissingPermission")
  private void updateFromTelecomCall() {
    Trace.beginSection("DialerCall.updateFromTelecomCall");
    LogUtil.v("DialerCall.updateFromTelecomCall", telecomCall.toString());

    videoTechManager.dispatchCallStateChanged(telecomCall.getState(), getAccountHandle());

    final int translatedState = translateState(telecomCall.getState());
    if (state != DialerCallState.BLOCKED) {
      setState(translatedState);
      setDisconnectCause(telecomCall.getDetails().getDisconnectCause());
    }

    childCallIds.clear();
    final int numChildCalls = telecomCall.getChildren().size();
    for (int i = 0; i < numChildCalls; i++) {
      childCallIds.add(
          dialerCallDelegate
              .getDialerCallFromTelecomCall(telecomCall.getChildren().get(i))
              .getId());
    }

    // The number of conferenced calls can change over the course of the call, so use the
    // maximum number of conferenced child calls as the metric for conference call usage.
    logState.conferencedCalls = Math.max(numChildCalls, logState.conferencedCalls);

    updateFromCallExtras(telecomCall.getDetails().getExtras());

    /// M: [Performance] Check if need to update voice mail number. @{
    boolean updateVoiceNumber = false;
    /// @}

    // If the handle of the call has changed, update state for the call determining if it is an
    // emergency call.
    Uri newHandle = telecomCall.getDetails().getHandle();
    if (!Objects.equals(handle, newHandle)) {
      /// M: [Performance] Check if need to update voice mail number. @{
      updateVoiceNumber = true;
      /// @}
      handle = newHandle;
      updateEmergencyCallState();
    }

    TelecomManager telecomManager = context.getSystemService(TelecomManager.class);
    // If the phone account handle of the call is set, cache capability bit indicating whether
    // the phone account supports call subjects.
    PhoneAccountHandle newPhoneAccountHandle = telecomCall.getDetails().getAccountHandle();
    if (!Objects.equals(phoneAccountHandle, newPhoneAccountHandle)) {
      LogUtil.i("DialerCall.updateFromTelecomCall ",
            "phone account changed, newPhoneAccountHandle = " + newPhoneAccountHandle);

      phoneAccountHandle = newPhoneAccountHandle;
      /// M: [Performance] Check if need to update voice mail number. @{
      updateVoiceNumber = true;
      /// @}

      if (phoneAccountHandle != null) {
        PhoneAccount phoneAccount = telecomManager.getPhoneAccount(phoneAccountHandle);
        if (phoneAccount != null) {
          isCallSubjectSupported =
              phoneAccount.hasCapabilities(PhoneAccount.CAPABILITY_CALL_SUBJECT);

          /// M: [Performance] Improve volte mo call performance. @{
          phoneAccountColor = phoneAccount.getHighlightColor();
          LogUtil.d("DialerCall.updateFromTelecomCall", "phoneAccountColor = " + phoneAccountColor);
          /// @}

          /// M: ALPS03414316, update call provider lable if account changed.
          /// M: ALPS03525413, need reset lable when account number is only one.
          /// Cannot remove the null pointer check of callProviderLabel, beacuse getPhoneAccount is
          /// binder call, will cause lot of time if called frequence.
          /// Get callcapable account here to increase performance. @{
          callCapableAccounts = telecomManager.getCallCapablePhoneAccounts();
          if (callCapableAccounts != null) {
            if (callCapableAccounts.size() > 1) {
              callProviderLabel = phoneAccount.getLabel().toString();
            } else if (callCapableAccounts.size() == 1) {
              callProviderLabel = "";
            } else {
              LogUtil.d("DialerCall.updateFromTelecomCall", "reset label");
              callProviderLabel = null;
            }
          }
          /// @}

          if (phoneAccount.hasCapabilities(PhoneAccount.CAPABILITY_SIM_SUBSCRIPTION)) {
            cacheCarrierConfiguration(phoneAccountHandle);
          }

          /// M: ALPS03555059, record sub id for post call. If not having any subid, we should not
          /// send post call. And also need to give dialer the sub info. @{
          subId = TelephonyManager.getDefault().getSubIdForPhoneAccount(phoneAccount);
          /// @}
        }
        /// M: ALPS03604515, ECC display wrong carrier name @{
        else {
          LogUtil.d("DialerCall.updateFromTelecomCall ", "phoneAccount==null");
          callProviderLabel = null;
          /// M: [Performance] Improve volte mo call performance, reset color. @{
          phoneAccountColor = PhoneAccount.NO_HIGHLIGHT_COLOR;
          callCapableAccounts = telecomManager.getCallCapablePhoneAccounts();
          /// @}
        }
        /// @}
      }
      /// M: ALPS03604515, ECC display wrong carrier name @{
      else {
        LogUtil.d("DialerCall.updateFromTelecomCall ", "phoneAccountHandle==null");
        callProviderLabel = null;
        /// M: [Performance]
        /// Improve volte mo call performance, reset color.
        /// Get callcapable account here to increase performance. @{
        phoneAccountColor = PhoneAccount.NO_HIGHLIGHT_COLOR;
        callCapableAccounts = telecomManager.getCallCapablePhoneAccounts();
        /// @}
      }
      /// @}
    }
    if (PermissionsUtil.hasPermission(context, permission.READ_PHONE_STATE)) {
      /// M: [Performance] Check if need to update voice mail number.
      /// Do not update country ISO every time.
      /// No need to get account every time. @{
      /// Google code:
      /// updateIsVoiceMailNumber();
      /// callCapableAccounts = telecomManager.getCallCapablePhoneAccounts();
      /// countryIso = GeoUtil.getCurrentCountryIso(context);
      if (updateVoiceNumber || phoneAccountHandle == null) {
        LogUtil.d("DialerCall.updateFromTelecomCall ", "updateIsVoiceMailNumber()");
        updateIsVoiceMailNumber();
      }

      countryIso = GeoUtil.getCurrentCountryIso(context);
      /// @}
    }
    Trace.endSection();
  }

  /**
   * Caches frequently used carrier configuration locally.
   *
   * @param accountHandle The PhoneAccount handle.
   */
  @SuppressLint("MissingPermission")
  private void cacheCarrierConfiguration(PhoneAccountHandle accountHandle) {
    if (!PermissionsUtil.hasPermission(context, permission.READ_PHONE_STATE)) {
      return;
    }
    if (VERSION.SDK_INT < VERSION_CODES.O) {
      return;
    }
    // TODO(a bug): This may take several seconds to complete, revisit it to move it to worker
    // thread.
    carrierConfig =
        TelephonyManagerCompat.getTelephonyManagerForPhoneAccountHandle(context, accountHandle)
            .getCarrierConfig();
  }

  /**
   * Tests corruption of the {@code callExtras} bundle by calling {@link
   * Bundle#containsKey(String)}. If the bundle is corrupted a {@link IllegalArgumentException} will
   * be thrown and caught by this function.
   *
   * @param callExtras the bundle to verify
   * @return {@code true} if the bundle is corrupted, {@code false} otherwise.
   */
  protected boolean areCallExtrasCorrupted(Bundle callExtras) {
    /**
     * There's currently a bug in Telephony service (a bug) that could corrupt the extras
     * bundle, resulting in a IllegalArgumentException while validating data under {@link
     * Bundle#containsKey(String)}.
     */
    try {
      callExtras.containsKey(Connection.EXTRA_CHILD_ADDRESS);
      return false;
    } catch (IllegalArgumentException e) {
      LogUtil.e(
          "DialerCall.areCallExtrasCorrupted", "callExtras is corrupted, ignoring exception", e);
      return true;
    }
  }

  protected void updateFromCallExtras(Bundle callExtras) {
    if (callExtras == null || areCallExtrasCorrupted(callExtras)) {
      /**
       * If the bundle is corrupted, abandon information update as a work around. These are not
       * critical for the dialer to function.
       */
      return;
    }
    // Check for a change in the child address and notify any listeners.
    if (callExtras.containsKey(Connection.EXTRA_CHILD_ADDRESS)) {
      String childNumber = callExtras.getString(Connection.EXTRA_CHILD_ADDRESS);
      if (!Objects.equals(childNumber, this.childNumber)) {
        this.childNumber = childNumber;
        for (DialerCallListener listener : listeners) {
          listener.onDialerCallChildNumberChange();
        }
      }
    }

    // Last forwarded number comes in as an array of strings.  We want to choose the
    // last item in the array.  The forwarding numbers arrive independently of when the
    // call is originally set up, so we need to notify the the UI of the change.
    if (callExtras.containsKey(Connection.EXTRA_LAST_FORWARDED_NUMBER)) {
      ArrayList<String> lastForwardedNumbers =
          callExtras.getStringArrayList(Connection.EXTRA_LAST_FORWARDED_NUMBER);

      if (lastForwardedNumbers != null) {
        String lastForwardedNumber = null;
        if (!lastForwardedNumbers.isEmpty()) {
          lastForwardedNumber = lastForwardedNumbers.get(lastForwardedNumbers.size() - 1);
        }

        if (!Objects.equals(lastForwardedNumber, this.lastForwardedNumber)) {
          this.lastForwardedNumber = lastForwardedNumber;
          for (DialerCallListener listener : listeners) {
            listener.onDialerCallLastForwardedNumberChange();
          }
        }
      }
    }

    // DialerCall subject is present in the extras at the start of call, so we do not need to
    // notify any other listeners of this.
    if (callExtras.containsKey(Connection.EXTRA_CALL_SUBJECT)) {
      String callSubject = callExtras.getString(Connection.EXTRA_CALL_SUBJECT);
      if (!Objects.equals(this.callSubject, callSubject)) {
        this.callSubject = callSubject;
      }
    }
  }

  public String getId() {
    return id;
  }

  /**
   * @return name appended with a number if the number is restricted/unknown and the user has
   *     received more than one restricted/unknown call.
   */
  @Nullable
  public String updateNameIfRestricted(@Nullable String name) {
    if (name != null && isHiddenNumber() && hiddenId != 0 && hiddenCounter > 1) {
      return context.getString(R.string.unknown_counter, name, hiddenId);
    }
    return name;
  }

  public static void clearRestrictedCount() {
    hiddenCounter = 0;
  }

  private boolean isHiddenNumber() {
    return getNumberPresentation() == TelecomManager.PRESENTATION_RESTRICTED
        || getNumberPresentation() == TelecomManager.PRESENTATION_UNKNOWN;
  }

  public boolean hasShownWiFiToLteHandoverToast() {
    return hasShownWiFiToLteHandoverToast;
  }

  public void setHasShownWiFiToLteHandoverToast() {
    hasShownWiFiToLteHandoverToast = true;
  }

  public boolean showWifiHandoverAlertAsToast() {
    return doNotShowDialogForHandoffToWifiFailure;
  }

  public void setDoNotShowDialogForHandoffToWifiFailure(boolean bool) {
    doNotShowDialogForHandoffToWifiFailure = bool;
  }

  public boolean showVideoChargesAlertDialog() {
    if (carrierConfig == null) {
      return false;
    }
    return carrierConfig.getBoolean(
        TelephonyManagerCompat.CARRIER_CONFIG_KEY_SHOW_VIDEO_CALL_CHARGES_ALERT_DIALOG_BOOL);
  }

  public long getTimeAddedMs() {
    return timeAddedMs;
  }

  @Nullable
  public String getNumber() {
    return TelecomCallUtil.getNumber(telecomCall);
  }

  public void blockCall() {
    telecomCall.reject(false, null);
    setState(DialerCallState.BLOCKED);
  }

  @Nullable
  public Uri getHandle() {
    return telecomCall == null ? null : telecomCall.getDetails().getHandle();
  }

  public boolean isEmergencyCall() {
    return isEmergencyCall;
  }

  public boolean isPotentialEmergencyCallback() {
    // The property PROPERTY_EMERGENCY_CALLBACK_MODE is only set for CDMA calls when the system
    // is actually in emergency callback mode (ie data is disabled).
    if (hasProperty(Details.PROPERTY_EMERGENCY_CALLBACK_MODE)) {
      return true;
    }

    // Call.EXTRA_LAST_EMERGENCY_CALLBACK_TIME_MILLIS is available starting in O
    if (VERSION.SDK_INT < VERSION_CODES.O) {
      long timestampMillis = FilteredNumbersUtil.getLastEmergencyCallTimeMillis(context);
      return isInEmergencyCallbackWindow(timestampMillis);
    }

    // We want to treat any incoming call that arrives a short time after an outgoing emergency call
    // as a potential emergency callback.
    if (getExtras() != null
        && getExtras().getLong(Call.EXTRA_LAST_EMERGENCY_CALLBACK_TIME_MILLIS, 0) > 0) {
      long lastEmergencyCallMillis =
          getExtras().getLong(Call.EXTRA_LAST_EMERGENCY_CALLBACK_TIME_MILLIS, 0);
      if (isInEmergencyCallbackWindow(lastEmergencyCallMillis)) {
        return true;
      }
    }
    return false;
  }

  boolean isInEmergencyCallbackWindow(long timestampMillis) {
    long emergencyCallbackWindowMillis =
        ConfigProviderComponent.get(context)
            .getConfigProvider()
            .getLong(CONFIG_EMERGENCY_CALLBACK_WINDOW_MILLIS, TimeUnit.MINUTES.toMillis(5));
    return System.currentTimeMillis() - timestampMillis < emergencyCallbackWindowMillis;
  }

  public int getState() {
    /**
     * M: [VoLTE Conference] Due to the AOSP original design, the IMS connection would report
     * the conference participant to UI with state "new" but no parent when the participant
     * connection was created. In this scenario, UI would make a mistake, it would regard
     * this as a new outgoing-call had been triggered.
     * In order to avoid this error case, if a call had the capability
     * "PROPERTY_CONFERENCE_PARTICIPANT" and property "MTK_PROPERTY_VOLTE", it should be
     * regarded as a conference children call. @{
     */
    if (telecomCall != null && telecomCall.getParent() != null
        || (telecomCall != null
            && hasProperty(MtkCall.MtkDetails.PROPERTY_CONFERENCE_PARTICIPANT)
            && hasProperty(MtkCall.MtkDetails.MTK_PROPERTY_VOLTE))
        /** @} */
        ) {
      return DialerCallState.CONFERENCED;
    } else {
      return state;
    }
  }

  public int getNonConferenceState() {
    return state;
  }

  public void setState(int state) {
    /// M: [Modification for finishing Transparent InCall Screen if necessary]
    /// such as:ALPS03477988,select sim press home key, show select again.
    /// such as:ALPS02302461,occur JE when MT call arrive at some case. @{
    if (state != DialerCallState.SELECT_PHONE_ACCOUNT ||
      stateEx != DialerCallState.WAIT_ACCOUNT_RESPONSE) {
      stateEx = state;
    }
    if (state == DialerCallState.WAIT_ACCOUNT_RESPONSE) {
      LogUtil.d(
          "DialerCall.setState",
          "ignore set SELECT_PHONE_ACCOUNT stateEx: " + stateEx);
      return;
    }
    /// @}
    if (state == DialerCallState.INCOMING) {
      logState.isIncoming = true;
    }
    updateCallTiming(state);

    this.state = state;
  }

  private void updateCallTiming(int newState) {
    if (newState == DialerCallState.ACTIVE) {
      if (this.state == DialerCallState.ACTIVE) {
        LogUtil.i("DialerCall.updateCallTiming", "state is already active");
        return;
      }
      logState.dialerConnectTimeMillis = clock.currentTimeMillis();
      logState.dialerConnectTimeMillisElapsedRealtime = SystemClock.elapsedRealtime();
    }

    if (newState == DialerCallState.DISCONNECTED) {
      long newDuration =
          getConnectTimeMillis() == 0 ? 0 : clock.currentTimeMillis() - getConnectTimeMillis();
      if (this.state == DialerCallState.DISCONNECTED) {
        LogUtil.i(
            "DialerCall.setState",
            "ignoring state transition from DISCONNECTED to DISCONNECTED."
                + " Duration would have changed from %s to %s",
            logState.telecomDurationMillis,
            newDuration);
        return;
      }
      logState.telecomDurationMillis = newDuration;
      logState.dialerDurationMillis =
          logState.dialerConnectTimeMillis == 0
              ? 0
              : clock.currentTimeMillis() - logState.dialerConnectTimeMillis;
      logState.dialerDurationMillisElapsedRealtime =
          logState.dialerConnectTimeMillisElapsedRealtime == 0
              ? 0
              : SystemClock.elapsedRealtime() - logState.dialerConnectTimeMillisElapsedRealtime;
    }
  }

  @VisibleForTesting
  void setClock(Clock clock) {
    this.clock = clock;
  }

  public int getNumberPresentation() {
    return telecomCall == null ? -1 : telecomCall.getDetails().getHandlePresentation();
  }

  public int getCnapNamePresentation() {
    return telecomCall == null ? -1 : telecomCall.getDetails().getCallerDisplayNamePresentation();
  }

  @Nullable
  public String getCnapName() {
    return telecomCall == null ? null : getTelecomCall().getDetails().getCallerDisplayName();
  }

  public Bundle getIntentExtras() {
    return telecomCall.getDetails().getIntentExtras();
  }

  @Nullable
  public Bundle getExtras() {
    return telecomCall == null ? null : telecomCall.getDetails().getExtras();
  }

  /** @return The child number for the call, or {@code null} if none specified. */
  public String getChildNumber() {
    return childNumber;
  }

  /** @return The last forwarded number for the call, or {@code null} if none specified. */
  public String getLastForwardedNumber() {
    return lastForwardedNumber;
  }

  public boolean isCallForwarded() {
    return isCallForwarded;
  }

  /** @return The call subject, or {@code null} if none specified. */
  public String getCallSubject() {
    return callSubject;
  }

  /**
   * @return {@code true} if the call's phone account supports call subjects, {@code false}
   *     otherwise.
   */
  public boolean isCallSubjectSupported() {
    return isCallSubjectSupported;
  }

  /** Returns call disconnect cause, defined by {@link DisconnectCause}. */
  public DisconnectCause getDisconnectCause() {
    if (state == DialerCallState.DISCONNECTED || state == DialerCallState.IDLE) {
      return disconnectCause;
    }

    return new DisconnectCause(DisconnectCause.UNKNOWN);
  }

  public void setDisconnectCause(DisconnectCause disconnectCause) {
    this.disconnectCause = disconnectCause;
    logState.disconnectCause = this.disconnectCause;
  }

  /** Returns the possible text message responses. */
  public List<String> getCannedSmsResponses() {
    return telecomCall.getCannedTextResponses();
  }

  /** Checks if the call supports the given set of capabilities supplied as a bit mask. */
  @TargetApi(28)
  public boolean can(int capabilities) {
    int supportedCapabilities = telecomCall.getDetails().getCallCapabilities();
    /// M: AT&T allows RTT call merge into conference. But the AOSP design isnot support.
    /// Judge whether should support by CarrierConfig. @{
    boolean hasConferenceableCall = false;
    if ((capabilities & Call.Details.CAPABILITY_MERGE_CONFERENCE) != 0) {
      // RTT call is not conferenceable, it's a bug (a bug) in Telecom and we work around it
      // here before it's fixed in Telecom.
      if (!videoFeatures.supportsRttCallMerge()) {
        for (Call call : telecomCall.getConferenceableCalls()) {
          if (!(BuildCompat.isAtLeastP() && call.isRttActive())) {
            hasConferenceableCall = true;
            break;
          }
        }
      } else {
         if(!telecomCall.getConferenceableCalls().isEmpty()) {
           hasConferenceableCall = true;
         }
      }
      /// @}
      // We allow you to merge if the capabilities allow it or if it is a call with
      // conferenceable calls.
      if (!hasConferenceableCall
          && ((Call.Details.CAPABILITY_MERGE_CONFERENCE & supportedCapabilities) == 0)) {
          // Cannot merge calls if there are no calls to merge with.
          return false;
      }
      capabilities &= ~Call.Details.CAPABILITY_MERGE_CONFERENCE;
    }
    return (capabilities == (capabilities & supportedCapabilities));
  }

  public boolean hasProperty(int property) {
    return telecomCall.getDetails().hasProperty(property);
  }

  @NonNull
  public String getUniqueCallId() {
    return uniqueCallId;
  }

  /** Gets the time when the call first became active. */
  public long getConnectTimeMillis() {
    return telecomCall.getDetails().getConnectTimeMillis();
  }

  /**
   * Gets the time when the call is created (see {@link Details#getCreationTimeMillis()}). This is
   * the same time that is logged as the start time in the Call Log (see {@link
   * android.provider.CallLog.Calls#DATE}).
   */
  @TargetApi(VERSION_CODES.O)
  public long getCreationTimeMillis() {
    return telecomCall.getDetails().getCreationTimeMillis();
  }

  public boolean isConferenceCall() {
    return hasProperty(Call.Details.PROPERTY_CONFERENCE);
  }

  @Nullable
  public GatewayInfo getGatewayInfo() {
    return telecomCall == null ? null : telecomCall.getDetails().getGatewayInfo();
  }

  @Nullable
  public PhoneAccountHandle getAccountHandle() {
    return telecomCall == null ? null : telecomCall.getDetails().getAccountHandle();
  }

  /** @return The {@link VideoCall} instance associated with the {@link Call}. */
  public VideoCall getVideoCall() {
    return telecomCall == null ? null : telecomCall.getVideoCall();
  }

  public List<String> getChildCallIds() {
    return childCallIds;
  }

  public String getParentId() {
    Call parentCall = telecomCall.getParent();
    if (parentCall != null) {
      /// M: ALPS03861942 Fixed JE issue. @{
      // Google code:
      // return dialerCallDelegate.getDialerCallFromTelecomCall(parentCall).getId();
      DialerCall call = dialerCallDelegate.getDialerCallFromTelecomCall(parentCall);
      if (call != null) {
        return call.getId();
      }
      /// @}
    }
    return null;
  }

  public int getVideoState() {
    return telecomCall.getDetails().getVideoState();
  }

  public boolean isVideoCall() {
    return getVideoTech().isTransmittingOrReceiving() || VideoProfile.isVideo(getVideoState());
  }

  @TargetApi(28)
  public boolean isActiveRttCall() {
    if (BuildCompat.isAtLeastP()) {
      return getTelecomCall().isRttActive();
    } else {
      return false;
    }
  }

  @TargetApi(28)
  @Nullable
  public RttCall getRttCall() {
    if (!isActiveRttCall()) {
      return null;
    }
    return getTelecomCall().getRttCall();
  }

  @TargetApi(28)
  public boolean isPhoneAccountRttCapable() {
    PhoneAccount phoneAccount = getPhoneAccount();
    if (phoneAccount == null) {
      return false;
    }
    if (!phoneAccount.hasCapabilities(PhoneAccount.CAPABILITY_RTT)) {
      return false;
    }
    return true;
  }

  @TargetApi(28)
  public boolean canUpgradeToRttCall() {
    /// M: [Performance] Improve call performance. @{
    /// Google code:
    /// if (!isPhoneAccountRttCapable()) {
    ///   return false;
    /// }
    /// Only volte call support upgrade to RTT call. @{
    if (!hasProperty(mediatek.telecom.MtkCall.MtkDetails.MTK_PROPERTY_VOLTE)) {
      return false;
    }
    /// @}
    if (isActiveRttCall()) {
      return false;
    }
    if (isVideoCall()) {
      return false;
    }
    if (isConferenceCall()) {
      return false;
    }
    /// M: [Performance] Improve call performance. @{
    if (!isPhoneAccountRttCapable()) {
      return false;
    }
    /// @}
    if (CallList.getInstance().hasActiveRttCall()
        /// M: it's for TMO/AT&T/Vzw requirements. they can support multi RTT call. @{
        && !videoFeatures.supportsMultiRttCall()) {
        /// @}
      return false;
    }
    /// M: it's for Vzw requirements that can't upgrade RTT agian when the call has ever
    /// chang to video call. @{
    if (getExtras().getBoolean(MtkTelecomManager.EXTRA_WAS_VIDEO_CALL,false)
        && !videoFeatures.supportsRttVideoSwitch()) {
      return false;
    }
    /// @}
    return true;
  }

  @TargetApi(28)
  public void sendRttUpgradeRequest() {
    getTelecomCall().sendRttRequest();
  }

  @TargetApi(28)
  public void respondToRttRequest(boolean accept, int rttRequestId) {
    Logger.get(context)
        .logCallImpression(
            accept
                ? DialerImpression.Type.RTT_MID_CALL_ACCEPTED
                : DialerImpression.Type.RTT_MID_CALL_REJECTED,
            getUniqueCallId(),
            getTimeAddedMs());
    getTelecomCall().respondToRttRequest(rttRequestId, accept);
  }

  @TargetApi(28)
  private void saveRttTranscript() {
    if (!BuildCompat.isAtLeastP()) {
      return;
    }
    if (getRttCall() != null) {
      // Save any remaining text in the buffer that's not shown by UI yet.
      // This may happen when the call is switched to background before disconnect.
      try {
        String messageLeft = getRttCall().readImmediately();
        if (!TextUtils.isEmpty(messageLeft)) {
          rttTranscript =
              RttChatMessage.getRttTranscriptWithNewRemoteMessage(rttTranscript, messageLeft);
        }
      } catch (IOException e) {
        LogUtil.e("DialerCall.saveRttTranscript", "error when reading remaining message", e);
      }
    }
    // Don't save transcript if it's empty.
    if (rttTranscript.getMessagesCount() == 0) {
      return;
    }
    Futures.addCallback(
        RttTranscriptUtil.saveRttTranscript(context, rttTranscript),
        new DefaultFutureCallback<>(),
        MoreExecutors.directExecutor());
  }

  public boolean hasReceivedVideoUpgradeRequest() {
    return VideoUtils.hasReceivedVideoUpgradeRequest(getVideoTech().getSessionModificationState());
  }

  public boolean hasSentVideoUpgradeRequest() {
    return VideoUtils.hasSentVideoUpgradeRequest(getVideoTech().getSessionModificationState());
  }

  public boolean hasSentRttUpgradeRequest() {
    return false;
  }

  /**
   * Determines if the call handle is an emergency number or not and caches the result to avoid
   * repeated calls to isEmergencyNumber.
   */
  private void updateEmergencyCallState() {
    isEmergencyCall = TelecomCallUtil.isEmergencyCall(telecomCall);
    LogUtil.d("Dialer.updateEmergencyCallState", "isEmergencyCall = " + isEmergencyCall);
  }

  public LogState getLogState() {
    return logState;
  }

  /**
   * Determines if the call is an external call.
   *
   * <p>An external call is one which does not exist locally for the {@link
   * android.telecom.ConnectionService} it is associated with.
   *
   * @return {@code true} if the call is an external call, {@code false} otherwise.
   */
  boolean isExternalCall() {
    return hasProperty(CallCompat.Details.PROPERTY_IS_EXTERNAL_CALL);
  }

  /**
   * Determines if answering this call will cause an ongoing video call to be dropped.
   *
   * @return {@code true} if answering this call will drop an ongoing video call, {@code false}
   *     otherwise.
   */
  public boolean answeringDisconnectsForegroundVideoCall() {
    Bundle extras = getExtras();
    if (extras == null
        || !extras.containsKey(CallCompat.Details.EXTRA_ANSWERING_DROPS_FOREGROUND_CALL)) {
      return false;
    }
    return extras.getBoolean(CallCompat.Details.EXTRA_ANSWERING_DROPS_FOREGROUND_CALL);
  }

  private void parseCallSpecificAppData() {
    if (isExternalCall()) {
      return;
    }

    logState.callSpecificAppData = CallIntentParser.getCallSpecificAppData(getIntentExtras());
    if (logState.callSpecificAppData == null) {

      logState.callSpecificAppData =
          CallSpecificAppData.newBuilder()
              .setCallInitiationType(CallInitiationType.Type.EXTERNAL_INITIATION)
              .build();
    }
    if (getState() == DialerCallState.INCOMING) {
      logState.callSpecificAppData =
          logState
              .callSpecificAppData
              .toBuilder()
              .setCallInitiationType(CallInitiationType.Type.INCOMING_INITIATION)
              .build();
    }
  }

  @Override
  public String toString() {
    if (telecomCall == null) {
      // This should happen only in testing since otherwise we would never have a null
      // Telecom call.
      return String.valueOf(id);
    }

    return String.format(
        Locale.US,
        "[%s, %s, %s, %s, children:%s, parent:%s, "
            + "conferenceable:%s, videoState:%s, mSessionModificationState:%d, CameraDir:%s]",
        id,
        DialerCallState.toString(getState()),
        /// M: ALPS03400867 Use MtkDetail to dump more capability and property. @{
        /// Details.capabilitiesToString(telecomCall.getDetails().getCallCapabilities()),
        /// Details.propertiesToString(telecomCall.getDetails().getCallProperties()),
        MtkDetails.capabilitiesToStringShort(telecomCall.getDetails().getCallCapabilities()),
        MtkDetails.propertiesToStringShort(telecomCall.getDetails().getCallProperties()),
        /// @}
        childCallIds,
        getParentId(),
        this.telecomCall.getConferenceableCalls(),
        VideoProfile.videoStateToString(telecomCall.getDetails().getVideoState()),
        getVideoTech().getSessionModificationState(),
        getCameraDir());
  }

  public String toSimpleString() {
    return super.toString();
  }

  @CallHistoryStatus
  public int getCallHistoryStatus() {
    return callHistoryStatus;
  }

  public void setCallHistoryStatus(@CallHistoryStatus int callHistoryStatus) {
    this.callHistoryStatus = callHistoryStatus;
  }

  public boolean didShowCameraPermission() {
    return didShowCameraPermission;
  }

  public void setDidShowCameraPermission(boolean didShow) {
    didShowCameraPermission = didShow;
  }

  public boolean didDismissVideoChargesAlertDialog() {
    return didDismissVideoChargesAlertDialog;
  }

  public void setDidDismissVideoChargesAlertDialog(boolean didDismiss) {
    didDismissVideoChargesAlertDialog = didDismiss;
  }

  public void setSpamStatus(@Nullable SpamStatus spamStatus) {
    this.spamStatus = spamStatus;
  }

  public Optional<SpamStatus> getSpamStatus() {
    return Optional.fromNullable(spamStatus);
  }

  public boolean isSpam() {
    if (spamStatus == null || !spamStatus.isSpam()) {
      return false;
    }

    if (!isIncoming()) {
      return false;
    }

    if (isPotentialEmergencyCallback()) {
      return false;
    }

    return true;
  }

  public boolean isBlocked() {
    return isBlocked;
  }

  public void setBlockedStatus(boolean isBlocked) {
    this.isBlocked = isBlocked;
  }

  public boolean isRemotelyHeld() {
    return isRemotelyHeld;
  }

  public boolean isMergeInProcess() {
    return isMergeInProcess;
  }

  public boolean isIncoming() {
    return logState.isIncoming;
  }

  /**
   * Try and determine if the call used assisted dialing.
   *
   * <p>We will not be able to verify a call underwent assisted dialing until the Platform
   * implmentation is complete in P+.
   *
   * @return a boolean indicating assisted dialing may have been performed
   */
  public boolean isAssistedDialed() {
    if (getIntentExtras() != null) {
      // P and below uses the existence of USE_ASSISTED_DIALING to indicate assisted dialing
      // was used. The Dialer client is responsible for performing assisted dialing before
      // placing the outgoing call.
      //
      // The existence of the assisted dialing extras indicates that assisted dialing took place.
      if (getIntentExtras().getBoolean(TelephonyManagerCompat.USE_ASSISTED_DIALING, false)
          && getAssistedDialingExtras() != null
          && Build.VERSION.SDK_INT <= ConcreteCreator.BUILD_CODE_CEILING) {
        return true;
      }
    }

    return false;
  }

  @Nullable
  public TransformationInfo getAssistedDialingExtras() {
    if (getIntentExtras() == null) {
      return null;
    }

    if (getIntentExtras().getBundle(TelephonyManagerCompat.ASSISTED_DIALING_EXTRAS) == null) {
      return null;
    }

    // Used in N-OMR1
    return TransformationInfo.newInstanceFromBundle(
        getIntentExtras().getBundle(TelephonyManagerCompat.ASSISTED_DIALING_EXTRAS));
  }

  public LatencyReport getLatencyReport() {
    return latencyReport;
  }

  public int getAnswerAndReleaseButtonDisplayedTimes() {
    return answerAndReleaseButtonDisplayedTimes;
  }

  public void increaseAnswerAndReleaseButtonDisplayedTimes() {
    answerAndReleaseButtonDisplayedTimes++;
  }

  public boolean getReleasedByAnsweringSecondCall() {
    return releasedByAnsweringSecondCall;
  }

  public void setReleasedByAnsweringSecondCall(boolean releasedByAnsweringSecondCall) {
    this.releasedByAnsweringSecondCall = releasedByAnsweringSecondCall;
  }

  public int getSecondCallWithoutAnswerAndReleasedButtonTimes() {
    return secondCallWithoutAnswerAndReleasedButtonTimes;
  }

  public void increaseSecondCallWithoutAnswerAndReleasedButtonTimes() {
    secondCallWithoutAnswerAndReleasedButtonTimes++;
  }

  @Nullable
  public EnrichedCallCapabilities getEnrichedCallCapabilities() {
    return enrichedCallCapabilities;
  }

  public void setEnrichedCallCapabilities(
      @Nullable EnrichedCallCapabilities mEnrichedCallCapabilities) {
    this.enrichedCallCapabilities = mEnrichedCallCapabilities;
  }

  @Nullable
  public Session getEnrichedCallSession() {
    return enrichedCallSession;
  }

  public void setEnrichedCallSession(@Nullable Session mEnrichedCallSession) {
    this.enrichedCallSession = mEnrichedCallSession;
  }

  public void unregisterCallback() {
    telecomCall.unregisterCallback(telecomCallCallback);
  }

  public void phoneAccountSelected(PhoneAccountHandle accountHandle, boolean setDefault) {
    LogUtil.i(
        "DialerCall.phoneAccountSelected",
        "accountHandle: %s, setDefault: %b",
        accountHandle,
        setDefault);
    telecomCall.phoneAccountSelected(accountHandle, setDefault);
  }

  public void disconnect() {
    LogUtil.i("DialerCall.disconnect", "");
    setState(DialerCallState.DISCONNECTING);
    /// M: [Performance] Increase end call performance.
    /// Disconnect call just before update UI will save about 30 ~ 50ms. @{
    /// google original code:
    /// for (DialerCallListener listener : listeners) {
    ///   listener.onDialerCallUpdate();
    /// }
    telecomCall.disconnect();
    for (DialerCallListener listener : listeners) {
      listener.onDialerCallUpdate();
    }
    /// @}
  }

  public void hold() {
    LogUtil.i("DialerCall.hold", "");
    telecomCall.hold();
  }

  public void unhold() {
    LogUtil.i("DialerCall.unhold", "");
    telecomCall.unhold();
  }

  public void splitFromConference() {
    LogUtil.i("DialerCall.splitFromConference", "");
    telecomCall.splitFromConference();
  }

  public void answer(int videoState) {
    LogUtil.i("DialerCall.answer", "videoState: " + videoState);
    telecomCall.answer(videoState);
  }

  public void answer() {
    answer(telecomCall.getDetails().getVideoState());
  }

  public void reject(boolean rejectWithMessage, String message) {
    LogUtil.i("DialerCall.reject", "");
    telecomCall.reject(rejectWithMessage, message);
  }

  /** Return the string label to represent the call provider */
  public String getCallProviderLabel() {
    if (callProviderLabel == null) {
      /// M: [Performance]. Add permission check only when need. @{
      if (ContextCompat.checkSelfPermission(context, permission.READ_PHONE_STATE)
          != PackageManager.PERMISSION_GRANTED) {
        return null;
      }
      /// @}

      PhoneAccount account = getPhoneAccount();
      if (account != null && !TextUtils.isEmpty(account.getLabel())) {
        if (callCapableAccounts != null && callCapableAccounts.size() > 1) {
          callProviderLabel = account.getLabel().toString();
        }
      }
      if (callProviderLabel == null) {
        callProviderLabel = "";
      }
    }
    return callProviderLabel;
  }

  private PhoneAccount getPhoneAccount() {
    PhoneAccountHandle accountHandle = getAccountHandle();
    if (accountHandle == null) {
      return null;
    }
    return context.getSystemService(TelecomManager.class).getPhoneAccount(accountHandle);
  }

  public VideoTech getVideoTech() {
    if (videoTech == null
        /// M: ALPS04025126 videoTech only update when telecom notify updatecall.Fix get wrong
        /// videoTech when call's videocall has could get not null before telecom notify update. @{
        || videoTech == videoTechManager.getEmptyVideoTech()) {
        /// @}
      videoTech = videoTechManager.getVideoTech(getAccountHandle());

      // Only store the first video tech type found to be available during the life of the call.
      if (selectedAvailableVideoTechType == com.android.dialer.logging.VideoTech.Type.NONE) {
        // Update the video tech.
        selectedAvailableVideoTechType = videoTech.getVideoTechType();
      }
      /// M: ALPS04080325 Register callback to telecom when get the real videotech. @{
      if (videoTech != videoTechManager.getEmptyVideoTech()) {
        videoTech.onCallStateChanged(context, state, getAccountHandle());
      }
      /// @}
    }
    return videoTech;
  }

  public String getCallbackNumber() {
    if (callbackNumber == null) {
      // Show the emergency callback number if either:
      // 1. This is an emergency call.
      // 2. The phone is in Emergency Callback Mode, which means we should show the callback
      //    number.
      boolean showCallbackNumber = hasProperty(Details.PROPERTY_EMERGENCY_CALLBACK_MODE);

      if (isEmergencyCall() || showCallbackNumber) {
        callbackNumber =
            context.getSystemService(TelecomManager.class).getLine1Number(getAccountHandle());
      }

      if (callbackNumber == null) {
        callbackNumber = "";
      }
    }
    return callbackNumber;
  }

  public String getSimCountryIso() {
    String simCountryIso =
        TelephonyManagerCompat.getTelephonyManagerForPhoneAccountHandle(context, getAccountHandle())
            .getSimCountryIso();
    if (!TextUtils.isEmpty(simCountryIso)) {
      simCountryIso = simCountryIso.toUpperCase(Locale.US);
    }
    return simCountryIso;
  }

  @Override
  public void onVideoTechStateChanged() {
    update();
  }

  @Override
  public void onSessionModificationStateChanged() {
    Trace.beginSection("DialerCall.onSessionModificationStateChanged");
    ///M: ALPS03758801 Should reset the hide preview status when call change to audio by network.
    ///Better reset the status when click upgrade button or receive upgrade request. @{
    if (getVideoState() == VideoProfile.STATE_AUDIO_ONLY &&
        getVideoTech().getSessionModificationState() ==
        SessionModificationState.RECEIVED_UPGRADE_TO_VIDEO_REQUEST) {
      isHidePreview = false;
      LogUtil.d(
          "DialerCall.onSessionModificationStateChanged", "reset preview status");
    }
    /// @}

    for (DialerCallListener listener : listeners) {
      listener.onDialerCallSessionModificationStateChange();
    }
    Trace.endSection();
  }

  @Override
  public void onCameraDimensionsChanged(int width, int height) {
    InCallVideoCallCallbackNotifier.getInstance().cameraDimensionsChanged(this, width, height);
  }

  @Override
  public void onPeerDimensionsChanged(int width, int height) {
    InCallVideoCallCallbackNotifier.getInstance().peerDimensionsChanged(this, width, height);
  }

  @Override
  public void onVideoUpgradeRequestReceived() {
    LogUtil.enterBlock("DialerCall.onVideoUpgradeRequestReceived");

    for (DialerCallListener listener : listeners) {
      listener.onDialerCallUpgradeToVideo();
    }

    update();

    Logger.get(context)
        .logCallImpression(
            DialerImpression.Type.VIDEO_CALL_REQUEST_RECEIVED, getUniqueCallId(), getTimeAddedMs());
  }

  @Override
  public void onUpgradedToVideo(boolean switchToSpeaker) {
    LogUtil.enterBlock("DialerCall.onUpgradedToVideo");

    if (!switchToSpeaker) {
      return;
    }

    CallAudioState audioState = AudioModeProvider.getInstance().getAudioState();

    if (0 != (CallAudioState.ROUTE_BLUETOOTH & audioState.getSupportedRouteMask())) {
      LogUtil.e(
          "DialerCall.onUpgradedToVideo",
          "toggling speakerphone not allowed when bluetooth supported.");
      return;
    }

    if (audioState.getRoute() == CallAudioState.ROUTE_SPEAKER) {
      return;
    }

    TelecomAdapter.getInstance().setAudioRoute(CallAudioState.ROUTE_SPEAKER);
  }

  @Override
  public void onCapabilitiesUpdated() {
    if (getNumber() == null) {
      return;
    }
    EnrichedCallCapabilities capabilities =
        EnrichedCallComponent.get(context).getEnrichedCallManager().getCapabilities(getNumber());
    if (capabilities != null) {
      setEnrichedCallCapabilities(capabilities);
      update();
    }
  }

  @Override
  public void onEnrichedCallStateChanged() {
    updateEnrichedCallSession();
  }

  @Override
  public void onImpressionLoggingNeeded(DialerImpression.Type impressionType) {
    Logger.get(context).logCallImpression(impressionType, getUniqueCallId(), getTimeAddedMs());
    if (impressionType == DialerImpression.Type.LIGHTBRINGER_UPGRADE_REQUESTED) {
      if (getLogState().contactLookupResult == Type.NOT_FOUND) {
        Logger.get(context)
            .logCallImpression(
                DialerImpression.Type.LIGHTBRINGER_NON_CONTACT_UPGRADE_REQUESTED,
                getUniqueCallId(),
                getTimeAddedMs());
      }
    }
  }

  private void updateEnrichedCallSession() {
    if (getNumber() == null) {
      return;
    }
    if (getEnrichedCallSession() != null) {
      // State changes to existing sessions are currently handled by the UI components (which have
      // their own listeners). Someday instead we could remove those and just call update() here and
      // have the usual onDialerCallUpdate update the UI.
      dispatchOnEnrichedCallSessionUpdate();
      return;
    }

    EnrichedCallManager manager = EnrichedCallComponent.get(context).getEnrichedCallManager();

    Filter filter =
        isIncoming()
            ? manager.createIncomingCallComposerFilter()
            : manager.createOutgoingCallComposerFilter();

    Session session = manager.getSession(getUniqueCallId(), getNumber(), filter);
    if (session == null) {
      return;
    }

    session.setUniqueDialerCallId(getUniqueCallId());
    setEnrichedCallSession(session);

    LogUtil.i(
        "DialerCall.updateEnrichedCallSession",
        "setting session %d's dialer id to %s",
        session.getSessionId(),
        getUniqueCallId());

    dispatchOnEnrichedCallSessionUpdate();
  }

  private void dispatchOnEnrichedCallSessionUpdate() {
    for (DialerCallListener listener : listeners) {
      listener.onEnrichedCallSessionUpdate();
    }
  }

  void onRemovedFromCallList() {
    LogUtil.enterBlock("DialerCall.onRemovedFromCallList");
    // Ensure we clean up when this call is removed.
    if (videoTechManager != null) {
      videoTechManager.dispatchRemovedFromCallList();
    }
    // TODO(wangqi): Consider moving this to a DialerCallListener.
    if (rttTranscript != null && !isCallRemoved) {
      saveRttTranscript();
    }
    isCallRemoved = true;
  }

  public com.android.dialer.logging.VideoTech.Type getSelectedAvailableVideoTechType() {
    return selectedAvailableVideoTechType;
  }

  public void markFeedbackRequested() {
    feedbackRequested = true;
  }

  public boolean isFeedbackRequested() {
    return feedbackRequested;
  }

  /**
   * If the in call UI has shown the phone account selection dialog for the call, the {@link
   * PreferredAccountRecorder} to record the result from the dialog.
   */
  @Nullable
  public PreferredAccountRecorder getPreferredAccountRecorder() {
    return preferredAccountRecorder;
  }

  public void setPreferredAccountRecorder(PreferredAccountRecorder preferredAccountRecorder) {
    this.preferredAccountRecorder = preferredAccountRecorder;
  }

  /** Indicates the call is eligible for SpeakEasy */
  public boolean isSpeakEasyEligible() {

    PhoneAccount phoneAccount = getPhoneAccount();

    if (phoneAccount == null) {
      return false;
    }

    if (!phoneAccount.hasCapabilities(PhoneAccount.CAPABILITY_SIM_SUBSCRIPTION)) {
      return false;
    }

    return !isPotentialEmergencyCallback()
        && !isEmergencyCall()
        && !isActiveRttCall()
        && !isConferenceCall()
        && !isVideoCall()
        && !isVoiceMailNumber()
        && !hasReceivedVideoUpgradeRequest()
        && !isVoipCallNotSupportedBySpeakeasy();
  }

  private boolean isVoipCallNotSupportedBySpeakeasy() {
    Bundle extras = getIntentExtras();

    if (extras == null) {
      return false;
    }

    // Indicates an VOIP call.
    String callid = extras.getString("callid");

    if (TextUtils.isEmpty(callid)) {
      LogUtil.i("DialerCall.isVoipCallNotSupportedBySpeakeasy", "callid was empty");
      return false;
    }

    LogUtil.i("DialerCall.isVoipCallNotSupportedBySpeakeasy", "call is not eligible");
    return true;
  }

  /** Indicates the user has selected SpeakEasy */
  public boolean isSpeakEasyCall() {
    /// M: ALPS04667884 improve answer call performance. @{
    /// original code :
    /*if (!isSpeakEasyEligible()) {
      return false;
    }
    return isSpeakEasyCall ;*/
    return isSpeakEasyCall && isSpeakEasyEligible();
    /// @}
  }

  /** Sets the user preference for SpeakEasy */
  public void setIsSpeakEasyCall(boolean isSpeakEasyCall) {
    this.isSpeakEasyCall = isSpeakEasyCall;
    if (listeners != null) {
      for (DialerCallListener listener : listeners) {
        listener.onDialerCallSpeakEasyStateChange();
      }
    }
  }

  /**
   * Specifies whether a number is in the call history or not. {@link #CALL_HISTORY_STATUS_UNKNOWN}
   * means there is no result.
   */
  @IntDef({
    CALL_HISTORY_STATUS_UNKNOWN,
    CALL_HISTORY_STATUS_PRESENT,
    CALL_HISTORY_STATUS_NOT_PRESENT
  })
  @Retention(RetentionPolicy.SOURCE)
  public @interface CallHistoryStatus {}

  /** Camera direction constants */
  public static class CameraDirection {
    public static final int CAMERA_DIRECTION_UNKNOWN = -1;
    public static final int CAMERA_DIRECTION_FRONT_FACING = CameraCharacteristics.LENS_FACING_FRONT;
    public static final int CAMERA_DIRECTION_BACK_FACING = CameraCharacteristics.LENS_FACING_BACK;
  }

  /**
   * Tracks any state variables that is useful for logging. There is some amount of overlap with
   * existing call member variables, but this duplication helps to ensure that none of these logging
   * variables will interface with/and affect call logic.
   */
  public static class LogState {

    public DisconnectCause disconnectCause;
    public boolean isIncoming = false;
    public ContactLookupResult.Type contactLookupResult =
        ContactLookupResult.Type.UNKNOWN_LOOKUP_RESULT_TYPE;
    public CallSpecificAppData callSpecificAppData;
    // If this was a conference call, the total number of calls involved in the conference.
    public int conferencedCalls = 0;
    public boolean isLogged = false;

    // Result of subtracting android.telecom.Call.Details#getConnectTimeMillis from the current time
    public long telecomDurationMillis = 0;

    // Result of a call to System.currentTimeMillis when Dialer sees that a call
    // moves to the ACTIVE state
    long dialerConnectTimeMillis = 0;

    // Same as dialer_connect_time_millis, using SystemClock.elapsedRealtime
    // instead
    long dialerConnectTimeMillisElapsedRealtime = 0;

    // Result of subtracting dialer_connect_time_millis from System.currentTimeMillis
    public long dialerDurationMillis = 0;

    // Same as dialerDurationMillis, using SystemClock.elapsedRealtime instead
    public long dialerDurationMillisElapsedRealtime = 0;

    private static String lookupToString(ContactLookupResult.Type lookupType) {
      switch (lookupType) {
        case LOCAL_CONTACT:
          return "Local";
        case LOCAL_CACHE:
          return "Cache";
        case REMOTE:
          return "Remote";
        case EMERGENCY:
          return "Emergency";
        case VOICEMAIL:
          return "Voicemail";
        default:
          return "Not found";
      }
    }

    private static String initiationToString(CallSpecificAppData callSpecificAppData) {
      if (callSpecificAppData == null) {
        return "null";
      }
      switch (callSpecificAppData.getCallInitiationType()) {
        case INCOMING_INITIATION:
          return "Incoming";
        case DIALPAD:
          return "Dialpad";
        case SPEED_DIAL:
          return "Speed Dial";
        case REMOTE_DIRECTORY:
          return "Remote Directory";
        case SMART_DIAL:
          return "Smart Dial";
        case REGULAR_SEARCH:
          return "Regular Search";
        case CALL_LOG:
          return "DialerCall Log";
        case CALL_LOG_FILTER:
          return "DialerCall Log Filter";
        case VOICEMAIL_LOG:
          return "Voicemail Log";
        case CALL_DETAILS:
          return "DialerCall Details";
        case QUICK_CONTACTS:
          return "Quick Contacts";
        case EXTERNAL_INITIATION:
          return "External";
        case LAUNCHER_SHORTCUT:
          return "Launcher Shortcut";
        default:
          return "Unknown: " + callSpecificAppData.getCallInitiationType();
      }
    }

    @Override
    public String toString() {
      return String.format(
          Locale.US,
          "["
              + "%s, " // DisconnectCause toString already describes the object type
              + "isIncoming: %s, "
              + "contactLookup: %s, "
              + "callInitiation: %s, "
              + "duration: %s"
              + "]",
          disconnectCause,
          isIncoming,
          lookupToString(contactLookupResult),
          initiationToString(callSpecificAppData),
          telecomDurationMillis);
    }
  }

  /** Coordinates the available VideoTech implementations for a call. */
  @VisibleForTesting
  public static class VideoTechManager {
    private final Context context;
    private final EmptyVideoTech emptyVideoTech = new EmptyVideoTech();
    private final VideoTech rcsVideoShare;
    private final List<VideoTech> videoTechs;
    private VideoTech savedTech;

    @VisibleForTesting
    public VideoTechManager(DialerCall call) {
      this.context = call.context;

      String phoneNumber = call.getNumber();
      phoneNumber = phoneNumber != null ? phoneNumber : "";
      phoneNumber = phoneNumber.replaceAll("[^+0-9]", "");

      // Insert order here determines the priority of that video tech option
      videoTechs = new ArrayList<>();

      videoTechs.add(new ImsVideoTech(Logger.get(call.context), call, call.telecomCall));

      rcsVideoShare =
          EnrichedCallComponent.get(call.context)
              .getRcsVideoShareFactory()
              .newRcsVideoShare(
                  EnrichedCallComponent.get(call.context).getEnrichedCallManager(),
                  call,
                  phoneNumber);
      videoTechs.add(rcsVideoShare);

      videoTechs.add(
          new DuoVideoTech(
              DuoComponent.get(call.context).getDuo(), call, call.telecomCall, phoneNumber));

      savedTech = emptyVideoTech;
    }

    @VisibleForTesting
    public VideoTech getVideoTech(PhoneAccountHandle phoneAccountHandle) {
      if (savedTech == emptyVideoTech) {
        for (VideoTech tech : videoTechs) {
          if (tech.isAvailable(context, phoneAccountHandle)) {
            savedTech = tech;
            savedTech.becomePrimary();
            break;
          }
        }
      } else if (savedTech instanceof DuoVideoTech
          && rcsVideoShare.isAvailable(context, phoneAccountHandle)) {
        // RCS Video Share will become available after the capability exchange which is slower than
        // Duo reading local contacts for reachability. If Video Share becomes available and we are
        // not in the middle of any session changes, let it take over.
        savedTech = rcsVideoShare;
        rcsVideoShare.becomePrimary();
      }

      return savedTech;
    }

    /// M: ALPS04025126 videoTech only update when telecom notify updatecall.Fix get wrong videoTech
    /// when call's videocall has could get not null before telecom notify update. @{
    public VideoTech getEmptyVideoTech() {
      return emptyVideoTech;
    }
    /// @}

    @VisibleForTesting
    public void dispatchCallStateChanged(int newState, PhoneAccountHandle phoneAccountHandle) {
      for (VideoTech videoTech : videoTechs) {
        videoTech.onCallStateChanged(context, newState, phoneAccountHandle);
      }
    }

    void dispatchRemovedFromCallList() {
      for (VideoTech videoTech : videoTechs) {
        videoTech.onRemovedFromCallList();
      }
    }
  }

  /** Called when canned text responses have been loaded. */
  public interface CannedTextResponsesLoadedListener {
    void onCannedTextResponsesLoaded(DialerCall call);
  }

  /// M: ----------------- MediaTek features --------------------
  /**
   * M: For management of video call features.
   */
  private VideoFeatures videoFeatures;
  /// M: add isHidePreview to record user click hidepreview  button
  // when device rotate, we should accord to this state to restore. @{
  private boolean isHidePreview = false;

  /// M: ALPS03881758, disable hangup all and hangup hold button when click merge. @{
  private boolean duringConferenceMerge = false;
  /// @}

  /// M: ALPS04156002, need check if having conference permission. @{
  private static final String CTA_CONFERENCE_CALL = "com.mediatek.permission.CTA_CONFERENCE_CALL";
  /// @}

  /// M: [ALPS03837665] add for support local and peer swap video. @{
  private boolean isShowLargePreview = false;
  /// @}

  /// M: This flag is needed to check video pause status when pause video without SIP message
  /// feature is enabled. When feature is off, pause status can be checked from videoState.
  /// Keep it false by default as video is unpaused for each video call when started.
  private boolean isVideoPaused = false;

  /// M: ALPS03555059, record sub id for post call. If not having any subid, we should not
  /// send post call. And also need to give dialer the sub info. @{
  private int subId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
  /// @}

  /// M: [Performance] Improve volte mo call performance. @{
  private int phoneAccountColor = PhoneAccount.NO_HIGHLIGHT_COLOR;

  private static final String EXTRA_CALL_SPECIFIC_APP_DATA =
      "com.android.dialer.callintent.CALL_SPECIFIC_APP_DATA";

  private List<String> filterBundleList =
      Arrays.asList (
          EXTRA_CALL_SPECIFIC_APP_DATA,
          LatencyReport.EXTRA_CALL_CREATED_TIME_MILLIS,
          LatencyReport.EXTRA_CALL_TELECOM_ROUTING_START_TIME_MILLIS,
          LatencyReport.EXTRA_CALL_TELECOM_ROUTING_END_TIME_MILLIS,
          TelephonyManagerCompat.USE_ASSISTED_DIALING,
          TelephonyManagerCompat.ASSISTED_DIALING_EXTRAS
      );

  private List<String> allUsedBundleList = new ArrayList(
      Arrays.asList (
          Call.AVAILABLE_PHONE_ACCOUNTS,
          SuggestionProvider.EXTRA_SIM_SUGGESTION_REASON,
          InCallPresenter.CALL_CONFIGURATION_EXTRA,
          LatencyReport.EXTRA_CALL_CREATED_TIME_MILLIS,
          LatencyReport.EXTRA_CALL_TELECOM_ROUTING_START_TIME_MILLIS,
          LatencyReport.EXTRA_CALL_TELECOM_ROUTING_END_TIME_MILLIS,
          TelephonyManagerCompat.USE_ASSISTED_DIALING,
          TelephonyManagerCompat.ASSISTED_DIALING_EXTRAS
      ));

  private List<String> allUsedExtraBundleList = new ArrayList(
      Arrays.asList (
          MtkTelecomManager.EXTRA_WAS_VIDEO_CALL,
          CallCompat.Details.EXTRA_ANSWERING_DROPS_FOREGROUND_CALL,
          Call.EXTRA_LAST_EMERGENCY_CALLBACK_TIME_MILLIS,
          Connection.EXTRA_CHILD_ADDRESS,
          Connection.EXTRA_LAST_FORWARDED_NUMBER,
          Connection.EXTRA_CALL_SUBJECT
      ));
  /// @}

  public boolean isHidePreview() {
    return isHidePreview;
  }

  public void setHidePreview(boolean isHidePreview) {
    this.isHidePreview = isHidePreview;
  }
  ///@}

  /**
   * M: For management of video call features.
   * @return video features manager.
   */
  public VideoFeatures getVideoFeatures() {
    return videoFeatures;
  }

  /**
   * M: [Voice Record] Query phone recording state.
   */
  public boolean isRecording() {
    return hasProperty(MtkCall.MtkDetails.MTK_PROPERTY_VOICE_RECORDING);
  }

  @Override
  public void onDowngradeToAudio() {
    VideoSessionController.getInstance().onDowngradeToAudio(this);
  }

  @Override
  public void onCallSessionEvent(int event) {
    if (MtkVideoProvider.SESSION_EVENT_ERROR_CAMERA_CRASHED == event) {
      onCameraError(event);
    /// M: start 20 seconds timer for cancel upgrade @ {
    } else if (VideoSessionController.SESSION_EVENT_NOTIFY_START_TIMER_20S == event) {
      if (getVideoTech().getSessionModificationState() ==
            SessionModificationState.WAITING_FOR_UPGRADE_TO_VIDEO_RESPONSE
          && getVideoFeatures().supportsCancelUpgradeVideo()
          && !getVideoFeatures().supportsAutoStartTimerForCancelUpgrade()) {
        LogUtil.i("DialerCall.onCallSessionEvent", "start timer for cancel");
        VideoSessionController.getInstance().startTimingForAutoDecline(this);
      }
    /// @}
    }
  }

  /**
   * M: Call back when camera error happened.
   */
  private void onCameraError(int errorCode) {
    LogUtil.e("DialerCall.onCameraError", "Camera session error: %d, Call: %s",
        errorCode, this.toString());
    /// M: ALPS04662404  camera error also should be set during upgrade voice call to video,not
    /// only for video call. So remove the code to check if video call is. @{
    /// M: [ALPS03900517] Ignore duplicate camera error event. @{
    if (getVideoFeatures().isCameraErrorHappened()) {
    /// @}
      return;
    }
    /// @}
    /// M: this will trigger call button ui don't show upgrade to video button
    getVideoFeatures().setCameraErrorHappened(true);
    if (getVideoFeatures().isImsCall()
        /// M: ALPS04662404  camera error also should be set during upgrade voice call to video,not
        /// only for video call. But downgrade is only for video.so add the code to check if video
        /// call is in here @{
        && VideoProfile.isVideo(getVideoState())) {
        /// @}
      getVideoTech().downgradeToAudio();
      //reset hide preview flag
      setHidePreview(false);
      //show message for downgrade
      InCallPresenter.getInstance().showMessage(com.android.incallui.
          R.string.video_call_downgrade_request);
    }
  }

  /**
   * M: [Video Debug info] notify data usage is change
   */
  public void onCallDataUsageChanged(long dataUsage) {
    InCallPresenter.getInstance().onCallDataUsageChanged(dataUsage);
  }

  /// M: [VoLTE Conference] Check details change to increase performance. @{
  private Call.Details oldDetails;

  /**
   * M: [VoLTE Conference] Get details of the call. Wrapper for telecomCall.getDetails().
   * @return details
   */
  public android.telecom.Call.Details getDetails() {
    if (telecomCall != null) {
      return telecomCall.getDetails();
    } else {
      LogUtil.d("DialerCall.getDetails", "telecomCall is null, need check!");
      return null;
    }
  }

  /**
   * M: [VoLTE Conference] @{
   * This function used to check whether certain info has been changed, if changed, handle them.
   * @param newDetails
   */
  private void handleDetailsChanged(android.telecom.Call.Details newDetails) {
    CallDetailChangeHandler.getInstance().onCallDetailChanged(this, oldDetails, newDetails);
    oldDetails = newDetails;
  }

  /**
   * M: [VoLTE Conference] Check whether the call is marked as Ecc by NW.
   * @return if it is marked ECC
   */
  public boolean isVolteMarkedEcc() {
    boolean isVolteEmerencyCall = false;
    isVolteEmerencyCall = InCallUIVolteUtils.isVolteMarkedEcc(getDetails());
    return isVolteEmerencyCall;
  }

  /**
   * M: [VoLTE Conference] Get pau field received from NW.
   * @return pau
   */
  public String getVoltePauField() {
    String voltePauField = "";
    voltePauField = InCallUIVolteUtils.getVoltePauField(getDetails());
    return voltePauField;
  }

  /**
   * M: [VoLTE Conference] Handle children change, notify member add or leave,
   * only for VoLTE conference call.
   * Note: call this function before update() in onChildrenChanged(),
   * for childCallIds used here will be changed in update()
   */
  private void handleChildrenChanged() {
    LogUtil.d("DialerCall.handleChildrenChanged", "start .... ");
    if (!InCallUIVolteUtils.isVolteSupport()
        || !hasProperty(mediatek.telecom.MtkCall.MtkDetails.MTK_PROPERTY_VOLTE)) {
      // below feature is only for VoLTE conference, so skip if not VoLTE conference.
      return;
    }

    List<String> newChildrenIds = new ArrayList<String>();
    for (int i = 0; i < telecomCall.getChildren().size(); i++) {
      newChildrenIds.add(
          CallList.getInstance().getCallByTelecomCall(
          telecomCall.getChildren().get(i)).getId());
    }
    ConferenceChildrenChangeHandler.getInstance()
        .handleChildrenChanged(childCallIds, newChildrenIds);
  }

  /**
   * M: [VoLTE Conference] This function translates call state to status string for conference
   * caller.
   * @param context The Context object for the call.
   * @return call status to show
   */
  public String getCallStatusFromState(Context context) {
    LogUtil.d("DialerCall.getCallStatusFromState", "state: " + state);
    String callStatus = "";
    switch (state) {
      case DialerCallState.ACTIVE:
        callStatus = context.getString(R.string.call_status_online);
        break;
      case DialerCallState.ONHOLD:
        callStatus = context.getString(R.string.call_status_onhold);
        break;
      case DialerCallState.DIALING:
      case DialerCallState.REDIALING:
        callStatus = context.getString(R.string.call_status_dialing);
        break;
      case DialerCallState.DISCONNECTING:
        callStatus = context.getString(R.string.call_status_disconnecting);
        break;
      case DialerCallState.DISCONNECTED:
        callStatus = context.getString(R.string.call_status_disconnected);
        break;
      default:
        LogUtil.v("DialerCall.getCallStatusFromState",
            "getCallStatusFromState() un-expected state: " + state);
        break;
    }
    return callStatus;
  }

  /**
   * M: [VoLTE Conference] Print parnet call and child call id mapping to telecom id.
   * @param call Parent call from telecom
   * @param children Children call from telecom
   */
  private void printChildrenChanged(Call call, List<Call> children) {
    String parentTelecomId = null;
    String parentDialerCallId = null;
    String parentIdMapping = null;
    String childTelecomId = null;
    String childDialerCallId = null;
    List<String> childrenIdsMapping = null;

    if (call != null) {
      parentTelecomId = call.getDetails().getTelecomCallId();
      DialerCall parentDialerCall = dialerCallDelegate.getDialerCallFromTelecomCall(call);
      if (parentDialerCall != null) {
        parentDialerCallId = parentDialerCall.getId();
      }
      parentIdMapping = parentDialerCallId + "->" + parentTelecomId;
    }

    if (children != null) {
      childrenIdsMapping = new ArrayList<String>();
      for (int i = 0; i < children.size(); i++) {
        Call child = children.get(i);
        childTelecomId = child.getDetails().getTelecomCallId();
        DialerCall childDialerCall = dialerCallDelegate.getDialerCallFromTelecomCall(child);
        if (childDialerCall != null) {
          childDialerCallId = childDialerCall.getId();
        }
        childrenIdsMapping.add(childDialerCallId + "->" + childTelecomId);
      }
    }

    String result = String.format(
        Locale.US,
        "[parent:%s, children:%s]",
        parentIdMapping,
        childrenIdsMapping);
    LogUtil.i("DialerCall.printChildrenChanged", result);
  }

  /**
   * M: [Modification for finishing Transparent InCall Screen if necessary]
   * such as:ALPS03477988,select sim press home key, show select again.
   * such as:ALPS02302461,occur JE when MT call arrive at some case. @{
   *
   * @{
   */
  public int getStateEx() {
    LogUtil.d(
        "DialerCall.getStateEx",
        "SELECT_PHONE_ACCOUNT, stateEx:" + stateEx);
    return stateEx;
  }
  /** @} */

  /**
   * M: [VoLTE Conference] Print parnet call and child call id mapping to telecom id.
   * @param call Child call from telecom
   * @param newParent Parent call from telecom
   */
  private void printParnetChanged(Call call, Call newParent) {
    String parentTelecomId = null;
    String parentDialerCallId = null;
    String parentIdMapping = null;
    String childTelecomId = null;
    String childDialerCallId = null;
    String childIdMapping = null;

    if (call != null) {
      childTelecomId = call.getDetails().getTelecomCallId();
      DialerCall childDialerCall = dialerCallDelegate.getDialerCallFromTelecomCall(call);
      if (childDialerCall != null) {
        childDialerCallId = childDialerCall.getId();
      }
      childIdMapping = childDialerCallId + "->" + childTelecomId;
    }

    if (newParent != null) {
      parentTelecomId = newParent.getDetails().getTelecomCallId();
      DialerCall parentDialerCall = dialerCallDelegate.getDialerCallFromTelecomCall(newParent);
      if (parentDialerCall != null) {
        parentDialerCallId = parentDialerCall.getId();
      }
      parentIdMapping = parentDialerCallId + "->" + parentTelecomId;
    }

    String result = String.format(
        Locale.US,
        "[child:%s, parent:%s]",
        childIdMapping,
        parentIdMapping);
    LogUtil.i("DialerCall.printParnetChanged", result);
  }

  /**
   * M: ALPS03881758, Check if the call is just in merge status.
   * @return if is in merge status.
   */
  public boolean getConferenceMergeStatus() {
    return duringConferenceMerge;
  }

  /**
   * M: ALPS03881758, Set merge status of the call.
   * @param status the merge status.
   */
  public void setConferenceMergeStatus(boolean duringMerge) {
    duringConferenceMerge = duringMerge;
  }

  /**
   * M: ALPS04156002, need check if having conference permission.
   * @return if have conference permission.
   */
  public boolean hasConferencePermission() {
    CtaManager ctaManager = CtaManagerFactory.getInstance().makeCtaManager();
    if (ctaManager.isCtaSupported()) {
      return PermissionsUtil.hasPermission(context, CTA_CONFERENCE_CALL);
    }
    return true;
  }

  /**
   * M: Check if is truly video call except playing video ringtone
   * @return if is truly video call.
   */
  public boolean isVideoCallExcludeVideoRingtone() {
    if (getVideoTech().isTransmitting() || (isVideoCall()
        && getState() != DialerCallState.DIALING)) {
      return true;
    } else {
      return false;
    }
  }

  /// M: [ALPS03837665] add for support local and peer swap video. @{
  public void setLargePreview(boolean isShowLargePreview) {
    this.isShowLargePreview = isShowLargePreview;
  }

  public boolean isLargePreview() {
    return isShowLargePreview;
  }
  /// @}

  /**
   * M: set the video pause state.
   * Used only if pause video without SIP message feature is enabled
   * @param videoPaused pause video state
   */
  public void setVideoPauseState(boolean videoPaused) {
    isVideoPaused = videoPaused;
  }
  ///@}

  /**
   * M: get the video pause state.
   * Used only if pause video without SIP message feature is enabled
   * @return boolean pause video state
   */
  public boolean getVideoPauseState() {
    return isVideoPaused;
  }
  ///@}

  /**
   * M: Get subId of this call.
   * @return subId int
   */
  public int getCallSubId() {
    LogUtil.d("DialerCall.getCallSubId", "subId: " + subId);
    return subId;
  }

  /**
   * M: get phone type from call using network type.
   * @param networkType The network type
   * @return phone type
   */
  public int getPhoneTypeByNetworkType(int networkType) {
    int phoneType = TelephonyManager.getPhoneType(networkType);
    LogUtil.d("DialerCall.getPhoneTypeByNetworkType", "phoneType: " + phoneType);
    return phoneType;
  }

  /**
   * M: get voice network type from call.
   * @return voice network type
   */
  public int getVoiceNetworkTypeForCall() {
    if (subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
      return TelephonyManager.NETWORK_TYPE_UNKNOWN;
    }

    int networkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
    if (ContextCompat.checkSelfPermission(context, permission.READ_PHONE_STATE)
        == PackageManager.PERMISSION_GRANTED) {
      TelephonyManager telephonyManager = context.getSystemService(TelephonyManager.class);
      networkType = telephonyManager.getVoiceNetworkType(subId);
    }
    LogUtil.d("DialerCall.getVoiceNetworkTypeForCall", "subId: " + subId
        + "networkType: " + networkType);
    return networkType;
  }

  /// M: [Performance] Check details change to increase performance. @{
  private void stateUpdate() {
    Trace.beginSection("DialerCall.stateUpdate");
    /// M: ALPS03606286, using real state to check if call state changed.
    /// Can not update child call state using getState() API, because getState()
    /// always return State.CONFERENCED in conference call. Aways skip update. @{
    int oldState = state;
    /// @}

    // Clear any cache here that could potentially change on update.
    videoTech = null;
    // We want to potentially register a video call callback here.
    updateFromTelecomCall();
    if (oldState != getState()) {
      oldDetails = telecomCall.getDetails();
      if (getState() == DialerCallState.DISCONNECTED) {
        for (DialerCallListener listener : listeners) {
          listener.onDialerCallDisconnect();
        }
        EnrichedCallComponent.get(context)
            .getEnrichedCallManager()
            .unregisterCapabilitiesListener(this);
        EnrichedCallComponent.get(context)
            .getEnrichedCallManager()
            .unregisterStateChangedListener(this);
      } else {
        for (DialerCallListener listener : listeners) {
          listener.onDialerCallUpdate();
        }
      }
    } else {
      LogUtil.d("DialerCall.stateUpdate", "Skip state update.");
    }
    Trace.endSection();
  }

  private boolean forceUpdateDetailByState() {
    int oldState = getState();
    final int translatedState = translateState(telecomCall.getState());
    if (oldState != translatedState) {
      LogUtil.d("DialerCall.forceUpdateDetailByState", "new state = " + translatedState);
      return true;
    }
    return false;
  }

  private boolean skipUpdateDetails(Call.Details o, Call.Details n) {
    if (o == null || n == null) {
      LogUtil.d("DialerCall.skipUpdateDetails", "force to update detais");
      return false;
    }

    return
        Objects.equals(o.getHandle(), n.getHandle()) &&
        Objects.equals(o.getHandlePresentation(), n.getHandlePresentation()) &&
        Objects.equals(o.getCallerDisplayName(), n.getCallerDisplayName()) &&
        Objects.equals(o.getCallerDisplayNamePresentation(),
            n.getCallerDisplayNamePresentation()) &&
        Objects.equals(o.getAccountHandle(), n.getAccountHandle()) &&
        Objects.equals(o.getCallCapabilities(), n.getCallCapabilities()) &&
        Objects.equals(o.getCallProperties(), n.getCallProperties()) &&
        Objects.equals(o.getDisconnectCause(), n.getDisconnectCause()) &&
        Objects.equals(o.getConnectTimeMillis(), n.getConnectTimeMillis()) &&
        Objects.equals(o.getGatewayInfo(), n.getGatewayInfo()) &&
        Objects.equals(o.getVideoState(), n.getVideoState()) &&
        Objects.equals(o.getStatusHints(), n.getStatusHints()) &&
        areBundlesEqual(o.getExtras(), n.getExtras()) &&
        areBundlesEqual(o.getIntentExtras(), n.getIntentExtras()) &&
        Objects.equals(o.getCreationTimeMillis(), n.getCreationTimeMillis());
  }

  private boolean areBundlesEqual(Bundle bundle, Bundle newBundle) {
    if (bundle == null || newBundle == null) {
      return bundle == newBundle;
    }

    for(String key : bundle.keySet()) {
      if (key != null) {
        if (filterUnUsedBundles(key)) {
          continue;
        }
        final Object value = bundle.get(key);
        final Object newValue = newBundle.get(key);
        if ((value != null && value instanceof Bundle)
            && (newValue != null && newValue instanceof Bundle)) {
          return areBundlesEqual((Bundle) value, (Bundle) newValue);
        } else if (!Objects.equals(value, newValue)) {
          return false;
        }
      }
    }
    return true;
  }

  private boolean filterUnUsedBundles(String keyVaule) {
    if (keyVaule != null && keyVaule.isEmpty()) {
      return true;
    }

    if (filterBundleList.contains(keyVaule)) {
      return true;
    }

    if (allUsedBundleList.contains(keyVaule)) {
      return false;
    }

    if (allUsedExtraBundleList.contains(keyVaule)) {
      return false;
    }

    return true;
  }

  /**
   * M: get account color.
   * @return phone account color
   */
  public int getAccountColor() {
    if (phoneAccountColor == PhoneAccount.NO_HIGHLIGHT_COLOR &&
        phoneAccountHandle != null) {
      PhoneAccount phoneAccount =
          context.getSystemService(TelecomManager.class).getPhoneAccount(phoneAccountHandle);
      if (phoneAccount != null) {
        phoneAccountColor = phoneAccount.getHighlightColor();
        LogUtil.d("DialerCall.getAccountColor", "phoneAccountColor = " + phoneAccountColor);
      }
    }
    return phoneAccountColor;
  }
  /// @}

  /// M: send rtt downgrade for RTT feature. @{
  public void sendRttDowngradeRequest() {
    getTelecomCall().stopRtt();
  }
  /// @}
}
