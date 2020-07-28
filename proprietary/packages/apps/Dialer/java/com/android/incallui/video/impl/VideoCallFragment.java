/*
 * Copyright (C) 2016 The Android Open Source Project
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

package com.android.incallui.video.impl;

import android.Manifest.permission;
import android.content.Context;
import android.content.pm.PackageManager;
import android.content.res.Resources;
import android.graphics.Bitmap;
import android.graphics.Matrix;
import android.graphics.Outline;
import android.graphics.Point;
import android.graphics.drawable.Animatable;
import android.os.Bundle;
import android.os.SystemClock;
import android.renderscript.Allocation;
import android.renderscript.Element;
import android.renderscript.RenderScript;
import android.renderscript.ScriptIntrinsicBlur;
import android.support.annotation.ColorInt;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.support.annotation.VisibleForTesting;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentTransaction;
import android.support.v4.view.animation.FastOutLinearInInterpolator;
import android.support.v4.view.animation.LinearOutSlowInInterpolator;
import android.telecom.CallAudioState;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.Surface;
import android.view.TextureView;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.View.OnLayoutChangeListener;
import android.view.View.OnSystemUiVisibilityChangeListener;
import android.view.ViewGroup;
import android.view.ViewGroup.MarginLayoutParams;
import android.view.ViewOutlineProvider;
import android.view.accessibility.AccessibilityEvent;
import android.view.animation.AccelerateDecelerateInterpolator;
import android.view.animation.Interpolator;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.RelativeLayout;
import android.widget.TextView;
import com.android.dialer.common.Assert;
import com.android.dialer.common.FragmentUtils;
import com.android.dialer.common.LogUtil;
import com.android.dialer.util.PermissionsUtil;
import com.android.incallui.audioroute.AudioRouteSelectorDialogFragment;
import com.android.incallui.audioroute.AudioRouteSelectorDialogFragment.AudioRouteSelectorPresenter;
import com.android.incallui.contactgrid.ContactGridManager;
import com.android.incallui.hold.OnHoldFragment;
import com.android.incallui.incall.protocol.InCallButtonIds;
import com.android.incallui.incall.protocol.InCallButtonIdsExtension;
import com.android.incallui.incall.protocol.InCallButtonUi;
import com.android.incallui.incall.protocol.InCallButtonUiDelegate;
import com.android.incallui.incall.protocol.InCallButtonUiDelegateFactory;
import com.android.incallui.incall.protocol.InCallScreen;
import com.android.incallui.incall.protocol.InCallScreenDelegate;
import com.android.incallui.incall.protocol.InCallScreenDelegateFactory;
import com.android.incallui.incall.protocol.PrimaryCallState;
import com.android.incallui.incall.protocol.PrimaryInfo;
import com.android.incallui.incall.protocol.SecondaryInfo;
import com.android.incallui.video.impl.CheckableImageButton.OnCheckedChangeListener;
import com.android.incallui.video.protocol.VideoCallScreen;
import com.android.incallui.video.protocol.VideoCallScreenDelegate;
import com.android.incallui.video.protocol.VideoCallScreenDelegateFactory;
import com.android.incallui.videosurface.bindings.VideoSurfaceBindings;
import com.android.incallui.videosurface.protocol.VideoSurfaceTexture;
import com.android.incallui.videotech.utils.VideoUtils;
import com.mediatek.incallui.plugin.ExtensionManager;
import com.mediatek.incallui.video.VideoDebugInfo;

import java.util.Locale;
/** Contains UI elements for a video call. */

public class VideoCallFragment extends Fragment
    implements InCallScreen,
        InCallButtonUi,
        VideoCallScreen,
        OnClickListener,
        OnCheckedChangeListener,
        AudioRouteSelectorPresenter,
        OnSystemUiVisibilityChangeListener {

  @VisibleForTesting(otherwise = VisibleForTesting.PRIVATE)
  static final String ARG_CALL_ID = "call_id";

  private static final String TAG_VIDEO_CHARGES_ALERT = "tag_video_charges_alert";

  @VisibleForTesting static final float BLUR_PREVIEW_RADIUS = 16.0f;
  @VisibleForTesting static final float BLUR_PREVIEW_SCALE_FACTOR = 1.0f;
  private static final float BLUR_REMOTE_RADIUS = 25.0f;
  private static final float BLUR_REMOTE_SCALE_FACTOR = 0.25f;
  private static final float ASPECT_RATIO_MATCH_THRESHOLD = 0.2f;

  private static final int CAMERA_PERMISSION_REQUEST_CODE = 1;
  private static final long CAMERA_PERMISSION_DIALOG_DELAY_IN_MILLIS = 2000L;
  private static final long VIDEO_OFF_VIEW_FADE_OUT_DELAY_IN_MILLIS = 2000L;
  private static final long VIDEO_CHARGES_ALERT_DIALOG_DELAY_IN_MILLIS = 500L;

  private final ViewOutlineProvider circleOutlineProvider =
      new ViewOutlineProvider() {
        @Override
        public void getOutline(View view, Outline outline) {
          int x = view.getWidth() / 2;
          int y = view.getHeight() / 2;
          int radius = Math.min(x, y);
          outline.setOval(x - radius, y - radius, x + radius, y + radius);
        }
      };

  private InCallScreenDelegate inCallScreenDelegate;
  private VideoCallScreenDelegate videoCallScreenDelegate;
  private InCallButtonUiDelegate inCallButtonUiDelegate;
  private View endCallButton;
  private CheckableImageButton speakerButton;
  private SpeakerButtonController speakerButtonController;
  private CheckableImageButton muteButton;
  private CheckableImageButton cameraOffButton;
  private ImageButton swapCameraButton;
  private View switchOnHoldButton;
  private View onHoldContainer;
  private SwitchOnHoldCallController switchOnHoldCallController;
  private TextView remoteVideoOff;
  private ImageView remoteOffBlurredImageView;
  private View mutePreviewOverlay;
  private View previewOffOverlay;
  private ImageView previewOffBlurredImageView;
  private View controls;
  private View controlsContainer;
  private TextureView previewTextureView;
  private TextureView remoteTextureView;
  private View greenScreenBackgroundView;
  private View fullscreenBackgroundView;
  private boolean shouldShowRemote;
  private boolean shouldShowPreview;
  private boolean isInFullscreenMode;
  private boolean isInGreenScreenMode;
  private boolean hasInitializedScreenModes;
  private boolean isRemotelyHeld;
  private ContactGridManager contactGridManager;
  private SecondaryInfo savedSecondaryInfo;
  private final Runnable cameraPermissionDialogRunnable =
      new Runnable() {
        @Override
        public void run() {
          if (videoCallScreenDelegate.shouldShowCameraPermissionToast()) {
            LogUtil.i("VideoCallFragment.cameraPermissionDialogRunnable", "showing dialog");
            checkCameraPermission();
          }
        }
      };

  private final Runnable videoChargesAlertDialogRunnable =
      () -> {
        VideoChargesAlertDialogFragment existingVideoChargesAlertFragment =
            (VideoChargesAlertDialogFragment)
                getChildFragmentManager().findFragmentByTag(TAG_VIDEO_CHARGES_ALERT);
        if (existingVideoChargesAlertFragment != null) {
          LogUtil.i(
              "VideoCallFragment.videoChargesAlertDialogRunnable", "already shown for this call");
          return;
        }

        if (VideoChargesAlertDialogFragment.shouldShow(getContext(), getCallId())) {
          LogUtil.i("VideoCallFragment.videoChargesAlertDialogRunnable", "showing dialog");
          VideoChargesAlertDialogFragment.newInstance(getCallId())
              .show(getChildFragmentManager(), TAG_VIDEO_CHARGES_ALERT);
        }
      };

  /// M: ALPS03764823 the remoteVideoOff is set gone when the gap between the time of setting
  /// remoteVideoOff video on and off is shorter than 2 seconds. so it should remove set gone
  /// callback before set visible. @{
  private final Runnable setRemoteVideoOffGoneRunnable =
      new Runnable() {
        @Override
        public void run() {
          remoteVideoOff.setVisibility(View.GONE);
          LogUtil.d("VideoCallFragment.setRemoteVideoOffGoneRunnable", "set gone");
        }
      };
  /// @}

  public static VideoCallFragment newInstance(String callId) {
    Bundle bundle = new Bundle();
    bundle.putString(ARG_CALL_ID, Assert.isNotNull(callId));

    VideoCallFragment instance = new VideoCallFragment();
    instance.setArguments(bundle);
    return instance;
  }

  @Override
  public void onCreate(@Nullable Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    LogUtil.i("VideoCallFragment.onCreate", null);

    inCallButtonUiDelegate =
        FragmentUtils.getParent(this, InCallButtonUiDelegateFactory.class)
            .newInCallButtonUiDelegate();
    if (savedInstanceState != null) {
      inCallButtonUiDelegate.onRestoreInstanceState(savedInstanceState);
    }
  }

  @Override
  public void onRequestPermissionsResult(
      int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
    if (requestCode == CAMERA_PERMISSION_REQUEST_CODE) {
      if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
        LogUtil.i("VideoCallFragment.onRequestPermissionsResult", "Camera permission granted.");
        videoCallScreenDelegate.onCameraPermissionGranted();
      } else {
        LogUtil.i("VideoCallFragment.onRequestPermissionsResult", "Camera permission denied.");
      }
    }
    super.onRequestPermissionsResult(requestCode, permissions, grantResults);
  }

  @Nullable
  @Override
  public View onCreateView(
      LayoutInflater layoutInflater, @Nullable ViewGroup viewGroup, @Nullable Bundle bundle) {
    LogUtil.i("VideoCallFragment.onCreateView", null);

    View view =
        layoutInflater.inflate(
            isLandscape() ? R.layout.frag_videocall_land : R.layout.frag_videocall,
            viewGroup,
            false);
    contactGridManager =
        new ContactGridManager(view, null /* no avatar */, 0, false /* showAnonymousAvatar */);

    controls = view.findViewById(R.id.videocall_video_controls);
    controls.setVisibility(getActivity().isInMultiWindowMode() ? View.GONE : View.VISIBLE);
    controlsContainer = view.findViewById(R.id.videocall_video_controls_container);
    speakerButton = (CheckableImageButton) view.findViewById(R.id.videocall_speaker_button);
    muteButton = (CheckableImageButton) view.findViewById(R.id.videocall_mute_button);
    muteButton.setOnCheckedChangeListener(this);
    mutePreviewOverlay = view.findViewById(R.id.videocall_video_preview_mute_overlay);
    cameraOffButton = (CheckableImageButton) view.findViewById(R.id.videocall_mute_video);
    cameraOffButton.setOnCheckedChangeListener(this);
    previewOffOverlay = view.findViewById(R.id.videocall_video_preview_off_overlay);
    previewOffBlurredImageView =
        (ImageView) view.findViewById(R.id.videocall_preview_off_blurred_image_view);
    swapCameraButton = (ImageButton) view.findViewById(R.id.videocall_switch_video);
    swapCameraButton.setOnClickListener(this);
    view.findViewById(R.id.videocall_switch_controls)
        .setVisibility(getActivity().isInMultiWindowMode() ? View.GONE : View.VISIBLE);
    switchOnHoldButton = view.findViewById(R.id.videocall_switch_on_hold);
    onHoldContainer = view.findViewById(R.id.videocall_on_hold_banner);
    remoteVideoOff = (TextView) view.findViewById(R.id.videocall_remote_video_off);
    remoteVideoOff.setAccessibilityLiveRegion(View.ACCESSIBILITY_LIVE_REGION_POLITE);
    remoteOffBlurredImageView =
        (ImageView) view.findViewById(R.id.videocall_remote_off_blurred_image_view);
    endCallButton = view.findViewById(R.id.videocall_end_call);
    endCallButton.setOnClickListener(this);
    previewTextureView = (TextureView) view.findViewById(R.id.videocall_video_preview);
    previewTextureView.setClipToOutline(true);
    previewOffOverlay.setOnClickListener(
        new OnClickListener() {
          @Override
          public void onClick(View v) {
            checkCameraPermission();
          }
        });
    remoteTextureView = (TextureView) view.findViewById(R.id.videocall_video_remote);
    greenScreenBackgroundView = view.findViewById(R.id.videocall_green_screen_background);
    fullscreenBackgroundView = view.findViewById(R.id.videocall_fullscreen_background);

    remoteTextureView.addOnLayoutChangeListener(
        new OnLayoutChangeListener() {
          @Override
          public void onLayoutChange(
              View v,
              int left,
              int top,
              int right,
              int bottom,
              int oldLeft,
              int oldTop,
              int oldRight,
              int oldBottom) {
            LogUtil.i("VideoCallFragment.onLayoutChange", "remoteTextureView layout changed");
            updateRemoteVideoScaling();
            updateRemoteOffView();
          }
        });

    previewTextureView.addOnLayoutChangeListener(
        new OnLayoutChangeListener() {
          @Override
          public void onLayoutChange(
              View v,
              int left,
              int top,
              int right,
              int bottom,
              int oldLeft,
              int oldTop,
              int oldRight,
              int oldBottom) {
            LogUtil.i("VideoCallFragment.onLayoutChange", "previewTextureView layout changed");
            updatePreviewVideoScaling();
            updatePreviewOffView();

            /// M: ALPS03519689, preview does not update in non-full screen after rotation @{
            if (!hasInitializedScreenModes) {
              updateFullscreenAndGreenScreenMode(isInFullscreenMode, isInGreenScreenMode);
            }
            /// @}

            /// M: ALPS03502688 Scrollview's button will cover with local video, after you answer
            /// the vilte call request from notification.the rootcause is nevigation of height
            /// can't get before view start measure layout. So view can't set right padding
            /// to layout. when onGloalLayout callback, AP tringer view set padding
            /// by onApplyWindowInsets.@{
            if (getView() != null
                && getView().isAttachedToWindow()
                && !getActivity().isInMultiWindowMode()) {
              controlsContainer.onApplyWindowInsets(getView().getRootWindowInsets());
            }
            ///@}
          }
        });

    /// M: init MediaTek feature's view.
    initView(view);

    return view;
  }

  @Override
  public void onViewCreated(View view, @Nullable Bundle bundle) {
    super.onViewCreated(view, bundle);
    LogUtil.i("VideoCallFragment.onViewCreated", null);

    inCallScreenDelegate =
        FragmentUtils.getParentUnsafe(this, InCallScreenDelegateFactory.class)
            .newInCallScreenDelegate();
    videoCallScreenDelegate =
        FragmentUtils.getParentUnsafe(this, VideoCallScreenDelegateFactory.class)
            .newVideoCallScreenDelegate(this);

    speakerButtonController =
        new SpeakerButtonController(speakerButton, inCallButtonUiDelegate, videoCallScreenDelegate);
    switchOnHoldCallController =
        new SwitchOnHoldCallController(
            switchOnHoldButton, onHoldContainer, inCallScreenDelegate, videoCallScreenDelegate);

    videoCallScreenDelegate.initVideoCallScreenDelegate(getContext(), this);

    inCallScreenDelegate.onInCallScreenDelegateInit(this);
    inCallScreenDelegate.onInCallScreenReady();
    inCallButtonUiDelegate.onInCallButtonUiReady(this);

    view.setOnSystemUiVisibilityChangeListener(this);
    /// M: ALPS03556861 delete clear listener after videocallfragemnt onstop.Merge video conference
    /// success after video screen change to background, the incallui don't set surface to VTService
    /// in this case,the VTService will get stuck. @{
    videoCallScreenDelegate.onVideoCallScreenUiReady();
    /// @}
  }

  @Override
  public void onSaveInstanceState(Bundle outState) {
    super.onSaveInstanceState(outState);
    /// M: [ALPS03863711] ignore greenscreen or fullscreen update in background. @{
    isVisible = false;
    /// @}
    inCallButtonUiDelegate.onSaveInstanceState(outState);
  }

  @Override
  public void onDestroyView() {
    super.onDestroyView();
    LogUtil.i("VideoCallFragment.onDestroyView", null);
    inCallButtonUiDelegate.onInCallButtonUiUnready();
    inCallScreenDelegate.onInCallScreenUnready();
    /// M: ALPS03556861 delete clear listener after videocallfragemnt onstop.Merge video conference
    /// success after video screen change to background, the incallui don't set surface to VTService
    /// in this case,the VTService will get stuck. @{
    videoCallScreenDelegate.onVideoCallScreenUiUnready();
    /// @}
  }

  @Override
  public void onAttach(Context context) {
    super.onAttach(context);
    if (savedSecondaryInfo != null) {
      setSecondary(savedSecondaryInfo);
    }
  }

  @Override
  public void onStart() {
    super.onStart();
    LogUtil.d("VideoCallFragment.onStart", null);
    /// M: [ALPS03863711] ignore greenscreen or fullscreen update in background. @{
    isVisible = true;
    /// @}
    onVideoScreenStart();
  }

  @Override
  public void onVideoScreenStart() {
    /// M: [ALPS03855504] update greenscreen or fullscreen when screen resume @{
    videoCallScreenDelegate.updateFullscreenAndGreenScreenModeWhenUIStart();
    /// @}
    inCallButtonUiDelegate.refreshMuteState();
    /// M: ALPS03556861 delete clear listener after videocallfragemnt onstop.Merge video conference
    /// success after video screen change to background, the incallui don't set surface to VTService
    /// in this case,the VTService will get stuck. @{
    /// google original code:
    /// videoCallScreenDelegate.onVideoCallScreenUiReady();
    /// @}
    getView().postDelayed(cameraPermissionDialogRunnable, CAMERA_PERMISSION_DIALOG_DELAY_IN_MILLIS);
    getView()
        .postDelayed(videoChargesAlertDialogRunnable, VIDEO_CHARGES_ALERT_DIALOG_DELAY_IN_MILLIS);
  }

  @Override
  public void onResume() {
    super.onResume();
    LogUtil.d("VideoCallFragment.onResume", null);
    inCallScreenDelegate.onInCallScreenResumed();
    /// M: ALPS03387938 refresh mute state and align with voice call.
    inCallButtonUiDelegate.refreshMuteState();

    /** M: add VideoDebugInfo feature. @{ */
    if (VideoDebugInfo.isFeatureOn() && videoDebugInfo == null) {
      videoDebugInfo = new VideoDebugInfo();
      LogUtil.i("VideoCallFragment.onResume",
          "[checkPackageLoss]EngineerMode for package loss is on,set param");
      videoDebugInfo.setFragment(this);
      View parent = (View) previewTextureView.getParent();
      if (!(parent instanceof RelativeLayout)) {
        LogUtil.i("VideoCallFragment.onResume", "[checkPackageLoss]not a RelativeLayout:");
        return;
      }
      videoDebugInfo.setParent((RelativeLayout) parent);
    }
    /** @}*/
  }

  @Override
  public void onPause() {
    super.onPause();
    LogUtil.i("VideoCallFragment.onPause", null);
    inCallScreenDelegate.onInCallScreenPaused();
    /// M: ALPS03556861 delete clear listener after videocallfragemnt onstop.Merge video conference
    /// success after video screen change to background, the incallui don't set surface to VTService
    /// in this case,the VTService will get stuck. @{
    videoCallScreenDelegate.cancelAutoFullScreen();
    /// @}
  }

  @Override
  public void onStop() {
    super.onStop();
    LogUtil.i("VideoCallFragment.onStop", null);
    /// M: [ALPS03863711] ignore greenscreen or fullscreen update in background. @{
    isVisible = false;
    /// @}
    onVideoScreenStop();
  }

  @Override
  public void onVideoScreenStop() {
    getView().removeCallbacks(videoChargesAlertDialogRunnable);
    getView().removeCallbacks(cameraPermissionDialogRunnable);
    /// M: ALPS03556861 delete clear listener after videocallfragemnt onstop.Merge video conference
    /// success after video screen change to background, the incallui don't set surface to VTService
    /// in this case,the VTService will get stuck. @{
    /// google original code:
    /// videoCallScreenDelegate.onVideoCallScreenUiUnready();
    /// @}
  }

  private void exitFullscreenMode() {
    LogUtil.i("VideoCallFragment.exitFullscreenMode", null);

    if (!getView().isAttachedToWindow()) {
      /// M: ALPS03519689, preview does not update in non-full screen after rotation @{
      hasInitializedScreenModes = false;
      /// @}
      LogUtil.i("VideoCallFragment.exitFullscreenMode", "not attached");
      return;
    }

    showSystemUI();

    LinearOutSlowInInterpolator linearOutSlowInInterpolator = new LinearOutSlowInInterpolator();

    // Animate the controls to the shown state.
    controls
        .animate()
        .translationX(0)
        .translationY(0)
        .setInterpolator(linearOutSlowInInterpolator)
        .alpha(1)
        .start();

    // Animate onHold to the shown state.
    switchOnHoldButton
        .animate()
        .translationX(0)
        .translationY(0)
        .setInterpolator(linearOutSlowInInterpolator)
        /// M: ALPS04025061, should set correct alpha value when swap button is not enabled.
        /// And need set alpha value again in animation end action if the button is enabled during
        /// animation. @{
        /// Google code:
        // .alpha(1);
        .alpha(switchOnHoldButton.isEnabled() ? 1: BUTTOON_DISABLED_ALPHA_VALUE)
        .withEndAction(
            new Runnable() {
              @Override
              public void run() {
                float alpha = switchOnHoldButton.isEnabled() ? 1: BUTTOON_DISABLED_ALPHA_VALUE;
                LogUtil.d("VideoCallFragment.exitFullscreenMode", "set alpha as " + alpha);
                switchOnHoldButton.setAlpha(alpha);
              }
            })
        /// @}
        .withStartAction(
            new Runnable() {
              @Override
              public void run() {
                switchOnHoldCallController.setOnScreen();
              }
            });

    View contactGridView = contactGridManager.getContainerView();
    // Animate contact grid to the shown state.
    contactGridView
        .animate()
        .translationX(0)
        .translationY(0)
        .setInterpolator(linearOutSlowInInterpolator)
        .alpha(1)
        .withStartAction(
            new Runnable() {
              @Override
              public void run() {
                contactGridManager.show();
              }
            });

    endCallButton
        .animate()
        .translationX(0)
        .translationY(0)
        .setInterpolator(linearOutSlowInInterpolator)
        .alpha(1)
        .withStartAction(
            new Runnable() {
              @Override
              public void run() {
                endCallButton.setVisibility(View.VISIBLE);
              }
            })
        .start();

    // Animate all the preview controls up to make room for the navigation bar.
    // In green screen mode we don't need this because the preview takes up the whole screen and has
    // a fixed position.
    /// M: [ALPS03837665] add local and peer swap video fuction for support CMCC and CT do video
    /// call's quality test. @{
    /// the original code :
    /// if (!isInGreenScreenMode)  {
    if (!isInGreenScreenMode && !videoCallScreenDelegate.isPreviewLarge()) {
    /// @}
      Point previewOffsetStartShown = getPreviewOffsetStartShown();
      for (View view : getAllPreviewRelatedViews()) {
        // Animate up with the preview offset above the navigation bar.
        view.animate()
            .translationX(previewOffsetStartShown.x)
            .translationY(previewOffsetStartShown.y)
            .setInterpolator(new AccelerateDecelerateInterpolator())
            .start();
      }
    }

    updateOverlayBackground();
    /// M: show scroll view too.
    videoScrollView.setAlpha(1);
  }

  private void showSystemUI() {
    View view = getView();
    if (view != null) {
      // Code is more expressive with all flags present, even though some may be combined
      // noinspection PointlessBitwiseExpression
      view.setSystemUiVisibility(View.SYSTEM_UI_FLAG_VISIBLE | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
    }
  }

  /** Set view flags to hide the system UI. System UI will return on any touch event */
  private void hideSystemUI() {
    View view = getView();
    if (view != null) {
      view.setSystemUiVisibility(
          View.SYSTEM_UI_FLAG_FULLSCREEN
              | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
              | View.SYSTEM_UI_FLAG_LAYOUT_STABLE);
    }
  }

  private Point getControlsOffsetEndHidden(View controls) {
    if (isLandscape()) {
      return new Point(0, getOffsetBottom(controls));
    } else {
      return new Point(getOffsetStart(controls), 0);
    }
  }

  private Point getSwitchOnHoldOffsetEndHidden(View swapCallButton) {
    if (isLandscape()) {
      return new Point(0, getOffsetTop(swapCallButton));
    } else {
      return new Point(getOffsetEnd(swapCallButton), 0);
    }
  }

  private Point getContactGridOffsetEndHidden(View view) {
    return new Point(0, getOffsetTop(view));
  }

  private Point getEndCallOffsetEndHidden(View endCallButton) {
    if (isLandscape()) {
      return new Point(getOffsetEnd(endCallButton), 0);
    } else {
      return new Point(0, ((MarginLayoutParams) endCallButton.getLayoutParams()).bottomMargin);
    }
  }

  private Point getPreviewOffsetStartShown() {
    // No insets in multiwindow mode, and rootWindowInsets will get the display's insets.
    if (getActivity().isInMultiWindowMode()) {
      return new Point();
    }
    if (isLandscape()) {
      int stableInsetEnd =
          /// M: ALPS03908005, sometimes it's not right @{
          // Google orignal code
          // getView().getLayoutDirection() == View.LAYOUT_DIRECTION_RTL
          TextUtils.getLayoutDirectionFromLocale(Locale.getDefault()) == View.LAYOUT_DIRECTION_RTL
          /// @}
              ? getView().getRootWindowInsets().getStableInsetLeft()
              : -getView().getRootWindowInsets().getStableInsetRight();
      return new Point(stableInsetEnd, 0);
    } else {
      return new Point(0, -getView().getRootWindowInsets().getStableInsetBottom());
    }
  }

  private View[] getAllPreviewRelatedViews() {
    return new View[] {
      previewTextureView, previewOffOverlay, previewOffBlurredImageView, mutePreviewOverlay,
    };
  }

  private int getOffsetTop(View view) {
    return -(view.getHeight() + ((MarginLayoutParams) view.getLayoutParams()).topMargin);
  }

  private int getOffsetBottom(View view) {
    return view.getHeight() + ((MarginLayoutParams) view.getLayoutParams()).bottomMargin;
  }

  private int getOffsetStart(View view) {
    int offset = view.getWidth() + ((MarginLayoutParams) view.getLayoutParams()).getMarginStart();
    if (view.getLayoutDirection() == View.LAYOUT_DIRECTION_RTL) {
      offset = -offset;
    }
    return -offset;
  }

  private int getOffsetEnd(View view) {
    int offset = view.getWidth() + ((MarginLayoutParams) view.getLayoutParams()).getMarginEnd();
    if (view.getLayoutDirection() == View.LAYOUT_DIRECTION_RTL) {
      offset = -offset;
    }
    return offset;
  }

  private void enterFullscreenMode() {
    LogUtil.i("VideoCallFragment.enterFullscreenMode", null);

    hideSystemUI();

    Interpolator fastOutLinearInInterpolator = new FastOutLinearInInterpolator();

    // Animate controls to the hidden state.
    Point offset = getControlsOffsetEndHidden(controls);
    controls
        .animate()
        .translationX(offset.x)
        .translationY(offset.y)
        .setInterpolator(fastOutLinearInInterpolator)
        .alpha(0)
        .start();

    // Animate onHold to the hidden state.
    offset = getSwitchOnHoldOffsetEndHidden(switchOnHoldButton);
    switchOnHoldButton
        .animate()
        .translationX(offset.x)
        .translationY(offset.y)
        .setInterpolator(fastOutLinearInInterpolator)
        .alpha(0);

    View contactGridView = contactGridManager.getContainerView();
    // Animate contact grid to the hidden state.
    offset = getContactGridOffsetEndHidden(contactGridView);
    contactGridView
        .animate()
        .translationX(offset.x)
        .translationY(offset.y)
        .setInterpolator(fastOutLinearInInterpolator)
        .alpha(0);

    offset = getEndCallOffsetEndHidden(endCallButton);
    // Use a fast out interpolator to quickly fade out the button. This is important because the
    // button can't draw under the navigation bar which means that it'll look weird if it just
    // abruptly disappears when it reaches the edge of the naivgation bar.
    endCallButton
        .animate()
        .translationX(offset.x)
        .translationY(offset.y)
        .setInterpolator(fastOutLinearInInterpolator)
        .alpha(0)
        .withEndAction(
            new Runnable() {
              @Override
              public void run() {
                endCallButton.setVisibility(View.INVISIBLE);
              }
            })
        .setInterpolator(new FastOutLinearInInterpolator())
        .start();

    // Animate all the preview controls down now that the navigation bar is hidden.
    // In green screen mode we don't need this because the preview takes up the whole screen and has
    // a fixed position.
    /// M: [ALPS03837665] add local and peer swap video fuction for support CMCC and CT do video
    /// call's quality test. @{
    /// google original code:
    /// if (!isInGreenScreenMode) {
    if (!isInGreenScreenMode && !videoCallScreenDelegate.isPreviewLarge()) {
    /// @}
      for (View view : getAllPreviewRelatedViews()) {
        // Animate down with the navigation bar hidden.
        view.animate()
            .translationX(0)
            .translationY(0)
            .setInterpolator(new AccelerateDecelerateInterpolator())
            .start();
      }
    }
    updateOverlayBackground();
    /// M: hide scroll view too.
    videoScrollView.setAlpha(0);
  }

  @Override
  public void onClick(View v) {
    if (v == endCallButton) {
      LogUtil.i("VideoCallFragment.onClick", "end call button clicked");
      inCallButtonUiDelegate.onEndCallClicked();
      videoCallScreenDelegate.resetAutoFullscreenTimer();
    } else if (v == swapCameraButton) {
      if (swapCameraButton.getDrawable() instanceof Animatable) {
        ((Animatable) swapCameraButton.getDrawable()).start();
      }
      inCallButtonUiDelegate.toggleCameraClicked();
      videoCallScreenDelegate.resetAutoFullscreenTimer();
    /** M: Handle ext features @{*/
    } else {
      handleClick(v);
    }
    /** @}*/
  }

  @Override
  public void onCheckedChanged(CheckableImageButton button, boolean isChecked) {
    if (button == cameraOffButton) {
      if (!isChecked && !VideoUtils.hasCameraPermissionAndShownPrivacyToast(getContext())) {
        LogUtil.i("VideoCallFragment.onCheckedChanged", "show camera permission dialog");
        checkCameraPermission();
      } else {
        inCallButtonUiDelegate.pauseVideoClicked(isChecked);
        videoCallScreenDelegate.resetAutoFullscreenTimer();
      }
    } else if (button == muteButton) {
      inCallButtonUiDelegate.muteClicked(isChecked, true /* clickedByUser */);
      videoCallScreenDelegate.resetAutoFullscreenTimer();
    /** M: Handle ext features @{*/
    } else {
      handleCheckedChanged(button, isChecked);
    }
    /** @}*/
  }

  @Override
  public void showVideoViews(
      boolean shouldShowPreview, boolean shouldShowRemote, boolean isRemotelyHeld) {
    LogUtil.i(
        "VideoCallFragment.showVideoViews",
        "showPreview: %b, shouldShowRemote: %b",
        shouldShowPreview,
        shouldShowRemote);

    /// M: ALPS04808364, After back to background, can not get correct bitmap from textureView,
    /// and the bitmap have been cleared before.
    /// InCall will use cached bitmap to show blur image. @{
    if (videoCallScreenDelegate.getLocalVideoSurfaceTexture().getTextureView() != null) {
      Object obj = videoCallScreenDelegate.getLocalVideoSurfaceTexture().getTextureView().getTag();
      if (obj != null) {
        previewTextureView.setTag(obj);
      }
    }
    /// @}

    videoCallScreenDelegate.getLocalVideoSurfaceTexture().attachToTextureView(previewTextureView);
    videoCallScreenDelegate.getRemoteVideoSurfaceTexture().attachToTextureView(remoteTextureView);

    /// M: ALPS03515553, call onhold on remote screen @{
    /// Google code:
    /// this.isRemotelyHeld = isRemotelyHeld;
    /// @}
    if (this.shouldShowRemote != shouldShowRemote
        /// M: ALPS03515553, call onhold on remote screen @{
        || this.isRemotelyHeld != isRemotelyHeld) {
        /// @}
      this.shouldShowRemote = shouldShowRemote;
      this.isRemotelyHeld = isRemotelyHeld;
      updateRemoteOffView();
    }
    if (this.shouldShowPreview != shouldShowPreview) {
      this.shouldShowPreview = shouldShowPreview;
      updatePreviewOffView();
    }
  }

  @Override
  public void onLocalVideoDimensionsChanged() {
    LogUtil.d("VideoCallFragment.onLocalVideoDimensionsChanged", null);
    updatePreviewVideoScaling();
  }

  @Override
  public void onLocalVideoOrientationChanged() {
    LogUtil.d("VideoCallFragment.onLocalVideoOrientationChanged", null);
    updatePreviewVideoScaling();
  }

  /** Called when the remote video's dimensions change. */
  @Override
  public void onRemoteVideoDimensionsChanged() {
    LogUtil.i("VideoCallFragment.onRemoteVideoDimensionsChanged", null);
    updateRemoteVideoScaling();
  }

  @Override
  public void updateFullscreenAndGreenScreenMode(
      boolean shouldShowFullscreen, boolean shouldShowGreenScreen) {
    LogUtil.d(
        "VideoCallFragment.updateFullscreenAndGreenScreenMode",
        "shouldShowFullscreen: %b, shouldShowGreenScreen: %b",
        shouldShowFullscreen,
        shouldShowGreenScreen);

    if (getActivity() == null) {
      LogUtil.i("VideoCallFragment.updateFullscreenAndGreenScreenMode", "not attached to activity");
      return;
    }

    // Check if anything is actually going to change. The first time this function is called we
    // force a change by checking the hasInitializedScreenModes flag. We also force both fullscreen
    // and green screen modes to update even if only one has changed. That's because they both
    // depend on each other.
    if (hasInitializedScreenModes
        && shouldShowGreenScreen == isInGreenScreenMode
        && shouldShowFullscreen == isInFullscreenMode) {
      LogUtil.d(
          "VideoCallFragment.updateFullscreenAndGreenScreenMode", "no change to screen modes");
      return;
    }
    /// M: ALPS03855504 ignore update greenscreen or fullscreen in background. @{
    if (!isVisible) {
      LogUtil.d(
          "VideoCallFragment.updateFullscreenAndGreenScreenMode", "ignore update in background");
      return;
    }
    ///@}

    hasInitializedScreenModes = true;
    isInGreenScreenMode = shouldShowGreenScreen;
    isInFullscreenMode = shouldShowFullscreen;

    if (getView().isAttachedToWindow() && !getActivity().isInMultiWindowMode()) {
      controlsContainer.onApplyWindowInsets(getView().getRootWindowInsets());
    }
    if (shouldShowGreenScreen) {
      enterGreenScreenMode();
    /// M: [ALPS03837665] add local and peer swap video fuction for support CMCC and CT do video
    /// call's quality test. @{
    } else if (!videoCallScreenDelegate.isPreviewLarge()) {
      exitGreenScreenMode();
    }
    if (shouldShowFullscreen) {
      enterFullscreenMode();
    } else {
      exitFullscreenMode();
    }

    OnHoldFragment onHoldFragment =
        ((OnHoldFragment)
            getChildFragmentManager().findFragmentById(R.id.videocall_on_hold_banner));
    if (onHoldFragment != null) {
      onHoldFragment.setPadTopInset(!isInFullscreenMode);
    }
  }

  @Override
  public Fragment getVideoCallScreenFragment() {
    return this;
  }

  @Override
  @NonNull
  public String getCallId() {
    return Assert.isNotNull(getArguments().getString(ARG_CALL_ID));
  }

  @Override
  public void onHandoverFromWiFiToLte() {
    getView().post(videoChargesAlertDialogRunnable);
  }

  @Override
  public void showButton(@InCallButtonIds int buttonId, boolean show) {
    LogUtil.v(
        "VideoCallFragment.showButton",
        "buttonId: %s, show: %b",
        InCallButtonIdsExtension.toString(buttonId),
        show);
    if (buttonId == InCallButtonIds.BUTTON_AUDIO) {
      speakerButtonController.setEnabled(show);
    } else if (buttonId == InCallButtonIds.BUTTON_MUTE) {
      muteButton.setEnabled(show);
    } else if (buttonId == InCallButtonIds.BUTTON_PAUSE_VIDEO) {
      /// M : hide pause @{
      cameraOffButton.setVisibility(show ? View.VISIBLE : View.GONE);
      /// @}
      cameraOffButton.setEnabled(show);
    } else if (buttonId == InCallButtonIds.BUTTON_SWITCH_TO_SECONDARY) {
      switchOnHoldCallController.setVisible(show);
    } else if (buttonId == InCallButtonIds.BUTTON_SWITCH_CAMERA) {
      swapCameraButton.setEnabled(show);
    /** M: Handle ext features @{*/
    } else {
      showExtButtons(buttonId, show);
    }
    /** @}*/
  }

  @Override
  public void enableButton(@InCallButtonIds int buttonId, boolean enable) {
    LogUtil.v(
        "VideoCallFragment.setEnabled",
        "buttonId: %s, enable: %b",
        InCallButtonIdsExtension.toString(buttonId),
        enable);
    if (buttonId == InCallButtonIds.BUTTON_AUDIO) {
      speakerButtonController.setEnabled(enable);
    } else if (buttonId == InCallButtonIds.BUTTON_MUTE) {
      muteButton.setEnabled(enable);
    } else if (buttonId == InCallButtonIds.BUTTON_PAUSE_VIDEO) {
      cameraOffButton.setEnabled(enable);
    } else if (buttonId == InCallButtonIds.BUTTON_SWITCH_TO_SECONDARY) {
      switchOnHoldCallController.setEnabled(enable);
    /** M: Handle ext features @{*/
    } else {
      enableExtButton(buttonId, enable);
    }
    /** @}*/
  }

  @Override
  public void setEnabled(boolean enabled) {
    LogUtil.v("VideoCallFragment.setEnabled", "enabled: " + enabled);
    speakerButtonController.setEnabled(enabled);
    muteButton.setEnabled(enabled);
    cameraOffButton.setEnabled(enabled);
    switchOnHoldCallController.setEnabled(enabled);
    /** M: Handle ext features @{*/
    enableExtButtons(enabled);
    /** @}*/
  }

  @Override
  public void setHold(boolean value) {
    LogUtil.d("VideoCallFragment.setHold", "value: " + value);
    /// M: ALPS03729639. update the hold call button state based on call hold state. @{
    holdCallButton.setChecked(value);
    /// @}
  }

  @Override
  public void setCameraSwitched(boolean isBackFacingCamera) {
    LogUtil.i("VideoCallFragment.setCameraSwitched", "isBackFacingCamera: " + isBackFacingCamera);
  }

  @Override
  public void setVideoPaused(boolean isPaused) {
    LogUtil.d("VideoCallFragment.setVideoPaused", "isPaused: " + isPaused);
    cameraOffButton.setChecked(isPaused);
  }

  @Override
  public void setAudioState(CallAudioState audioState) {
    LogUtil.i("VideoCallFragment.setAudioState", "audioState: " + audioState);
    speakerButtonController.setAudioState(audioState);
    muteButton.setChecked(audioState.isMuted());
    updateMutePreviewOverlayVisibility();
  }

  @Override
  public void updateButtonStates() {
    LogUtil.d("VideoCallFragment.updateButtonState", null);
    speakerButtonController.updateButtonState();
    switchOnHoldCallController.updateButtonState();
  }

  @Override
  public void updateInCallButtonUiColors(@ColorInt int color) {}

  @Override
  public Fragment getInCallButtonUiFragment() {
    return this;
  }

  @Override
  public void showAudioRouteSelector() {
    LogUtil.i("VideoCallFragment.showAudioRouteSelector", null);
    AudioRouteSelectorDialogFragment.newInstance(inCallButtonUiDelegate.getCurrentAudioState())
        .show(getChildFragmentManager(), null);
  }

  @Override
  public void onAudioRouteSelected(int audioRoute) {
    LogUtil.i("VideoCallFragment.onAudioRouteSelected", "audioRoute: " + audioRoute);
    inCallButtonUiDelegate.setAudioRoute(audioRoute);
  }

  @Override
  public void onAudioRouteSelectorDismiss() {}

  @Override
  public void setPrimary(@NonNull PrimaryInfo primaryInfo) {
    LogUtil.d("VideoCallFragment.setPrimary", primaryInfo.toString());
    contactGridManager.setPrimary(primaryInfo);
  }

  @Override
  public void setSecondary(@NonNull SecondaryInfo secondaryInfo) {
    LogUtil.d("VideoCallFragment.setSecondary", secondaryInfo.toString());
    if (!isAdded()) {
      savedSecondaryInfo = secondaryInfo;
      return;
    }
    savedSecondaryInfo = null;
    switchOnHoldCallController.setSecondaryInfo(secondaryInfo);
    updateButtonStates();
    FragmentTransaction transaction = getChildFragmentManager().beginTransaction();
    Fragment oldBanner = getChildFragmentManager().findFragmentById(R.id.videocall_on_hold_banner);
    if (secondaryInfo.shouldShow()) {
      OnHoldFragment onHoldFragment = OnHoldFragment.newInstance(secondaryInfo);
      onHoldFragment.setPadTopInset(!isInFullscreenMode);
      transaction.replace(R.id.videocall_on_hold_banner, onHoldFragment);
    } else {
      if (oldBanner != null) {
        transaction.remove(oldBanner);
      }
    }
    transaction.setCustomAnimations(R.anim.abc_slide_in_top, R.anim.abc_slide_out_top);
    transaction.commitAllowingStateLoss();
  }

  @Override
  public void setCallState(@NonNull PrimaryCallState primaryCallState) {
    LogUtil.d("VideoCallFragment.setCallState", primaryCallState.toString());
    contactGridManager.setCallState(primaryCallState);
  }

  @Override
  public void setEndCallButtonEnabled(boolean enabled, boolean animate) {
    LogUtil.d("VideoCallFragment.setEndCallButtonEnabled", "enabled: " + enabled);
  }

  @Override
  public void showManageConferenceCallButton(boolean visible) {
    LogUtil.d("VideoCallFragment.showManageConferenceCallButton", "visible: " + visible);
    /// M: [Video Conference] @{
    videoConferenceButton.setVisibility(visible ? View.VISIBLE : View.GONE);
    videoConferenceButton.setEnabled(visible);
    /// @}
  }

  @Override
  public boolean isManageConferenceVisible() {
    LogUtil.d("VideoCallFragment.isManageConferenceVisible", null);
    /// M: [Video Conference] @{
    return videoConferenceButton.getVisibility() == View.VISIBLE;
    /// @}
  }

  @Override
  public void dispatchPopulateAccessibilityEvent(AccessibilityEvent event) {
    contactGridManager.dispatchPopulateAccessibilityEvent(event);
  }

  @Override
  public void showNoteSentToast() {
    LogUtil.i("VideoCallFragment.showNoteSentToast", null);
  }

  @Override
  public void updateInCallScreenColors() {
    LogUtil.d("VideoCallFragment.updateColors", null);
  }

  @Override
  public void onInCallScreenDialpadVisibilityChange(boolean isShowing) {
    LogUtil.i("VideoCallFragment.onInCallScreenDialpadVisibilityChange", null);
  }

  @Override
  public int getAnswerAndDialpadContainerResourceId() {
    return 0;
  }

  @Override
  public Fragment getInCallScreenFragment() {
    return this;
  }

  @Override
  public boolean isShowingLocationUi() {
    return false;
  }

  @Override
  public void showLocationUi(Fragment locationUi) {
    LogUtil.e("VideoCallFragment.showLocationUi", "Emergency video calling not supported");
    // Do nothing
  }

  private void updatePreviewVideoScaling() {
    if (previewTextureView.getWidth() == 0 || previewTextureView.getHeight() == 0) {
      LogUtil.i("VideoCallFragment.updatePreviewVideoScaling", "view layout hasn't finished yet");
      return;
    }
    VideoSurfaceTexture localVideoSurfaceTexture =
        videoCallScreenDelegate.getLocalVideoSurfaceTexture();
    Point cameraDimensions = localVideoSurfaceTexture.getSurfaceDimensions();
    if (cameraDimensions == null) {
      LogUtil.d(
          "VideoCallFragment.updatePreviewVideoScaling", "camera dimensions haven't been set");
      return;
    }
    if (isLandscape()) {
      /// M: [ALPS03837665] add local and peer swap video fuction for support CMCC and CT do video
      /// call's quality test. @{
      if (videoCallScreenDelegate.isPreviewLarge()) {
        VideoSurfaceBindings.scaleVideoMaintainingAspectRatioWithOrientation(
            previewTextureView,
            cameraDimensions.x,
            cameraDimensions.y,
            videoCallScreenDelegate.getDeviceOrientation());
      } else {
      /// @}
        VideoSurfaceBindings.scaleVideoAndFillView(
            previewTextureView,
            cameraDimensions.x,
            cameraDimensions.y,
            videoCallScreenDelegate.getDeviceOrientation());
      }
    } else {
      /// M: ALPS03646689, switch x y value to get correct scale while not in landscape
      /// but with 90 degree. @{
      /// Google code:
      /* VideoSurfaceBindings.scaleVideoAndFillView(
          previewTextureView,
          cameraDimensions.y,
          cameraDimensions.x,
          videoCallScreenDelegate.getDeviceOrientation());*/
      int rotationDegree = videoCallScreenDelegate.getDeviceOrientation();
      int yDimension = cameraDimensions.y;
      int xDimension = cameraDimensions.x;
      Context context = getContext();
      if (rotationDegree == 90 || rotationDegree == 270) {
        xDimension = cameraDimensions.y;
        yDimension = cameraDimensions.x;
        LogUtil.d("VideoCallFragment.updatePreviewVideoScaling", "switch camera dimensions");
      }
      /// M: [ALPS03837665] add local and peer swap video fuction for support CMCC and CT do video
      /// call's quality test. @{
      if (videoCallScreenDelegate.isPreviewLarge()) {
        VideoSurfaceBindings.scaleVideoMaintainingAspectRatioWithOrientation(
            previewTextureView,
            yDimension,
            xDimension,
            rotationDegree);
      } else {
      /// @}
        VideoSurfaceBindings.scaleVideoAndFillView(
            previewTextureView,
            yDimension,
            xDimension,
            rotationDegree);
      /// @}
      }
    }
  }

  private void updateRemoteVideoScaling() {
    VideoSurfaceTexture remoteVideoSurfaceTexture =
        videoCallScreenDelegate.getRemoteVideoSurfaceTexture();
    Point videoSize = remoteVideoSurfaceTexture.getSourceVideoDimensions();
    if (videoSize == null) {
      LogUtil.i("VideoCallFragment.updateRemoteVideoScaling", "video size is null");
      return;
    }
    if (remoteTextureView.getWidth() == 0 || remoteTextureView.getHeight() == 0) {
      LogUtil.i("VideoCallFragment.updateRemoteVideoScaling", "view layout hasn't finished yet");
      return;
    }

    // If the video and display aspect ratio's are close then scale video to fill display
    float videoAspectRatio = ((float) videoSize.x) / videoSize.y;
    float displayAspectRatio =
        ((float) remoteTextureView.getWidth()) / remoteTextureView.getHeight();
    float delta = Math.abs(videoAspectRatio - displayAspectRatio);
    float sum = videoAspectRatio + displayAspectRatio;
    if ((delta / sum < ASPECT_RATIO_MATCH_THRESHOLD
            /// M: cmcc requires peer video can't be cropped by plugin to implement. @{
            && ExtensionManager.getVilteAutoTestHelperExt().isAllowVideoCropped())
            /// @}
        /// M: [ALPS03837665] add local and peer swap video fuction for support CMCC and CT do
        /// video call's quality test. @{
        || videoCallScreenDelegate.isPreviewLarge()) {
        /// @}
      VideoSurfaceBindings.scaleVideoAndFillView(remoteTextureView, videoSize.x, videoSize.y, 0);
    } else {
      VideoSurfaceBindings.scaleVideoMaintainingAspectRatio(
          remoteTextureView, videoSize.x, videoSize.y);
    }
  }

  private boolean isLandscape() {
    /// M: ALPS03506919 Fixed no point JE issue @{
    if (getActivity() == null) {
      return false;
    }
    /// @}
    // Choose orientation based on display orientation, not window orientation
    int rotation = getActivity().getWindowManager().getDefaultDisplay().getRotation();
    return rotation == Surface.ROTATION_90 || rotation == Surface.ROTATION_270;
  }

  private void enterGreenScreenMode() {
    LogUtil.i("VideoCallFragment.enterGreenScreenMode", null);
    RelativeLayout.LayoutParams params =
        new RelativeLayout.LayoutParams(
            RelativeLayout.LayoutParams.MATCH_PARENT, RelativeLayout.LayoutParams.MATCH_PARENT);
    params.addRule(RelativeLayout.ALIGN_PARENT_START);
    params.addRule(RelativeLayout.ALIGN_PARENT_TOP);
    previewTextureView.setLayoutParams(params);
    previewTextureView.setOutlineProvider(null);
    updateOverlayBackground();
    contactGridManager.setIsMiddleRowVisible(true);
    updateMutePreviewOverlayVisibility();

    previewOffBlurredImageView.setLayoutParams(params);
    previewOffBlurredImageView.setOutlineProvider(null);
    previewOffBlurredImageView.setClipToOutline(false);

    /// M: ALPS03659894, update screen after request fail @{
    previewTextureView.setTranslationY(0);
    previewOffOverlay.setTranslationY(0);
    previewOffBlurredImageView.setTranslationY(0);
    /// @}
  }

  private void exitGreenScreenMode() {
    LogUtil.i("VideoCallFragment.exitGreenScreenMode", null);
    Resources resources = getResources();
    RelativeLayout.LayoutParams params =
        new RelativeLayout.LayoutParams(
            (int) resources.getDimension(R.dimen.videocall_preview_width),
            (int) resources.getDimension(R.dimen.videocall_preview_height));
    params.setMargins(
        0, 0, 0, (int) resources.getDimension(R.dimen.videocall_preview_margin_bottom));
    if (isLandscape()) {
      params.addRule(RelativeLayout.ALIGN_PARENT_END);
      params.setMarginEnd((int) resources.getDimension(R.dimen.videocall_preview_margin_end));
    } else {
      params.addRule(RelativeLayout.ALIGN_PARENT_START);
      params.setMarginStart((int) resources.getDimension(R.dimen.videocall_preview_margin_start));
    }
    params.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
    previewTextureView.setLayoutParams(params);
    previewTextureView.setOutlineProvider(circleOutlineProvider);
    updateOverlayBackground();
    contactGridManager.setIsMiddleRowVisible(false);
    updateMutePreviewOverlayVisibility();

    previewOffBlurredImageView.setLayoutParams(params);
    previewOffBlurredImageView.setOutlineProvider(circleOutlineProvider);
    previewOffBlurredImageView.setClipToOutline(true);
  }

  private void updatePreviewOffView() {
    LogUtil.enterBlock("VideoCallFragment.updatePreviewOffView");

    // Always hide the preview off and remote off views in green screen mode.
    /// M: ALPS03506468 Record hide preview and hide previewoffView @{
    /// boolean previewEnabled = isInGreenScreenMode || shouldShowPreview;
    boolean previewEnabled = isInGreenScreenMode || shouldShowPreview || hidePreview;
    /// @}
    previewOffOverlay.setVisibility(previewEnabled ? View.GONE : View.VISIBLE);
    updateBlurredImageView(
        previewTextureView,
        previewOffBlurredImageView,
        /// M: ALPS03506468 Record hide preview and hide previewoffView @{
        /// shouldShowPreview,
        (shouldShowPreview || hidePreview),
        /// @}
        BLUR_PREVIEW_RADIUS,
        BLUR_PREVIEW_SCALE_FACTOR);
  }

  private void updateRemoteOffView() {
    LogUtil.enterBlock("VideoCallFragment.updateRemoteOffView");
    boolean remoteEnabled = isInGreenScreenMode || shouldShowRemote;
    boolean isResumed = remoteEnabled && !isRemotelyHeld;
    if (isResumed) {
      boolean wasRemoteVideoOff =
          TextUtils.equals(
              remoteVideoOff.getText(),
              remoteVideoOff.getResources().getString(R.string.videocall_remote_video_off));
      // The text needs to be updated and hidden after enough delay in order to be announced by
      // talkback.
      remoteVideoOff.setText(
          wasRemoteVideoOff
              ? R.string.videocall_remote_video_on
              : R.string.videocall_remotely_resumed);
      /// M: ALPS03764823 the remoteVideoOff is set gone when the gap between the time of setting
      /// remoteVideoOff video on and off is shorter than 2 seconds. so it should remove set gone
      /// callback before set visible. @{
      /// google original code: @{
      /// remoteVideoOff.postDelayed(
      ///    new Runnable() {
      ///      @Override
      ///      public void run() {
      ///        remoteVideoOff.setVisibility(View.GONE);
      ///      }
      ///    },
      ///    VIDEO_OFF_VIEW_FADE_OUT_DELAY_IN_MILLIS);
      /// @}
      remoteVideoOff.postDelayed(setRemoteVideoOffGoneRunnable,
          VIDEO_OFF_VIEW_FADE_OUT_DELAY_IN_MILLIS);
      /// @}
    } else {
      /// M: ALPS03764823 the remoteVideoOff is set gone when the gap between the time of setting
      /// remoteVideoOff video on and off is shorter than 2 seconds. so it should remove set gone
      /// callback before set visible. @{
      remoteVideoOff.removeCallbacks(setRemoteVideoOffGoneRunnable);
      /// @}
      remoteVideoOff.setText(
          isRemotelyHeld ? R.string.videocall_remotely_held : R.string.videocall_remote_video_off);
      remoteVideoOff.setVisibility(View.VISIBLE);
    }
    updateBlurredImageView(
        remoteTextureView,
        remoteOffBlurredImageView,
        shouldShowRemote,
        BLUR_REMOTE_RADIUS,
        BLUR_REMOTE_SCALE_FACTOR);
  }

  @VisibleForTesting
  void updateBlurredImageView(
      TextureView textureView,
      ImageView blurredImageView,
      boolean isVideoEnabled,
      float blurRadius,
      float scaleFactor) {
    Context context = getContext();

    if (isVideoEnabled || context == null) {
      blurredImageView.setImageBitmap(null);
      blurredImageView.setVisibility(View.GONE);
      return;
    }

    long startTimeMillis = SystemClock.elapsedRealtime();
    int width = Math.round(textureView.getWidth() * scaleFactor);
    int height = Math.round(textureView.getHeight() * scaleFactor);

    LogUtil.d("VideoCallFragment.updateBlurredImageView", "width: %d, height: %d", width, height);

    // This call takes less than 10 milliseconds.
    Bitmap bitmap = textureView.getBitmap(width, height);

    if (bitmap == null) {
      blurredImageView.setImageBitmap(null);
      blurredImageView.setVisibility(View.GONE);
      return;
    }

    /// M: ALPS04808364, After back to background, can not get correct bitmap from textureView,
    /// and the bitmap have been cleared before.
    /// InCall will use cached bitmap to show blur image. @{
    if (previewTextureView == textureView) {
      float alpha = bitmap.getColor(width/2, height/2).alpha();
      LogUtil.d("VideoCallFragment.updateBlurredImageView", "bitmap alpha = " + alpha);
      if (alpha < 0.001) {
        if (blurredImageView.getDrawable() != null) {
          LogUtil.d("VideoCallFragment.updateBlurredImageView", "getDrawable is not null");
          return;
        } else if (videoCallScreenDelegate
            .getLocalVideoSurfaceTexture().getTextureView() != null) {
          bitmap = (Bitmap) videoCallScreenDelegate
            .getLocalVideoSurfaceTexture().getTextureView().getTag();
          if (bitmap == null
              || bitmap.getWidth() == 0) {
            blurredImageView.setImageBitmap(null);
            blurredImageView.setVisibility(View.GONE);
            LogUtil.d("VideoCallFragment.updateBlurredImageView", "cached Bitmap is invalid");
            return;
          }
          LogUtil.d("VideoCallFragment.updateBlurredImageView", "newWidth = " +
              width + ", width = " + bitmap.getWidth());
          if (width != bitmap.getWidth()) {
            float scale = ((float) width)/ bitmap.getWidth();
            Matrix matrix = new Matrix();
            matrix.postScale(scale, scale);
            bitmap =
              Bitmap.createBitmap(
                  bitmap,
                  0,
                  0,
                  bitmap.getWidth(),
                  bitmap.getHeight(),
                  matrix,
                  true);
          }
        }
      } else if (videoCallScreenDelegate.getLocalVideoSurfaceTexture().getTextureView() != null) {
        videoCallScreenDelegate.getLocalVideoSurfaceTexture().getTextureView().setTag(bitmap);
        LogUtil.d("VideoCallFragment.updateBlurredImageView", "save bitmap as cached");
      }
    }
    /// @}

    // TODO(mdooley): When the view is first displayed after a rotation the bitmap is empty
    // and thus this blur has no effect.
    // This call can take 100 milliseconds.
    blur(getContext(), bitmap, blurRadius);

    // TODO(mdooley): Figure out why only have to apply the transform in landscape mode
    if (width > height) {
      bitmap =
          Bitmap.createBitmap(
              bitmap,
              0,
              0,
              bitmap.getWidth(),
              bitmap.getHeight(),
              textureView.getTransform(null),
              true);
    }

    blurredImageView.setImageBitmap(bitmap);
    blurredImageView.setVisibility(View.VISIBLE);

    LogUtil.i(
        "VideoCallFragment.updateBlurredImageView",
        "took %d millis",
        (SystemClock.elapsedRealtime() - startTimeMillis));
  }

  private void updateOverlayBackground() {
    if (isInGreenScreenMode) {
      // We want to darken the preview view to make text and buttons readable. The fullscreen
      // background is below the preview view so use the green screen background instead.
      animateSetVisibility(greenScreenBackgroundView, View.VISIBLE);
      animateSetVisibility(fullscreenBackgroundView, View.GONE);
    } else if (!isInFullscreenMode) {
      // We want to darken the remote view to make text and buttons readable. The green screen
      // background is above the preview view so it would darken the preview too. Use the fullscreen
      // background instead.
      animateSetVisibility(greenScreenBackgroundView, View.GONE);
      animateSetVisibility(fullscreenBackgroundView, View.VISIBLE);
    } else {
      animateSetVisibility(greenScreenBackgroundView, View.GONE);
      animateSetVisibility(fullscreenBackgroundView, View.GONE);
    }
  }

  private void updateMutePreviewOverlayVisibility() {
    // Normally the mute overlay shows on the bottom right of the preview bubble. In green screen
    // mode the preview is fullscreen so there's no where to anchor it.
    mutePreviewOverlay.setVisibility(
        muteButton.isChecked() && !isInGreenScreenMode ? View.VISIBLE : View.GONE);
  }

  private void animateSetVisibility(final View view, final int visibility) {
    /// M: ALPS03460593, Sometimes when user click quick, will get incorrect result to
    /// get the visibility of the view. Because we use animate to modify the visibility
    /// of view. Should record latest visibility of fullscreen. @{
    LogUtil.d("VideoCallFragment.animateSetVisibility", "" + visibility);
    if ((view.getVisibility() == visibility && view != fullscreenBackgroundView)
       || (view == fullscreenBackgroundView && fullscreenLatestVisibility == visibility)) {
      return;
    }
    /// @}

    int startAlpha;
    int endAlpha;
    if (visibility == View.GONE) {
      startAlpha = 1;
      endAlpha = 0;
    } else if (visibility == View.VISIBLE) {
      startAlpha = 0;
      endAlpha = 1;
    } else {
      Assert.fail();
      return;
    }

    /// M: ALPS03460593, Sometimes when user click quick, will get incorrect result to
    /// get the visibility of the view. Because we use animate to modify the visibility
    /// of view. Should record latest visibility of fullscreen. @{
    if (view == fullscreenBackgroundView) {
      LogUtil.d("VideoCallFragment.animateSetVisibility", "" + visibility);
      fullscreenLatestVisibility = visibility;
    }
    /// @}

    view.setAlpha(startAlpha);
    view.setVisibility(View.VISIBLE);
    view.animate()
        .alpha(endAlpha)
        .withEndAction(
            new Runnable() {
              @Override
              public void run() {
                view.setVisibility(visibility);
              }
            })
        .start();
  }

  private static void blur(Context context, Bitmap image, float blurRadius) {
    RenderScript renderScript = RenderScript.create(context);
    ScriptIntrinsicBlur blurScript =
        ScriptIntrinsicBlur.create(renderScript, Element.U8_4(renderScript));
    Allocation allocationIn = Allocation.createFromBitmap(renderScript, image);
    Allocation allocationOut = Allocation.createFromBitmap(renderScript, image);
    blurScript.setRadius(blurRadius);
    blurScript.setInput(allocationIn);
    blurScript.forEach(allocationOut);
    allocationOut.copyTo(image);
    blurScript.destroy();
    allocationIn.destroy();
    allocationOut.destroy();
  }

  @Override
  public void onSystemUiVisibilityChange(int visibility) {
    /// M: ALPS03449886 fix don't enter full screen when click quickly.add check
    /// View.SYSTEM_UI_FLAG_FULLSCREEN to judge whether should set full screen false or not
    /// when callback of onSystemUiVisibilityChange. @{
    /// Google original code:
    /// boolean navBarVisible = (visibility & View.SYSTEM_UI_FLAG_HIDE_NAVIGATION) == 0;
    LogUtil.i("VideoCallFragment.onSystemUiVisibilityChange", "visible:" + visibility);
    int flag = View.SYSTEM_UI_FLAG_HIDE_NAVIGATION | View.SYSTEM_UI_FLAG_FULLSCREEN ;
    boolean navBarVisible = (visibility & flag) == 0;
    /// @}
    videoCallScreenDelegate.onSystemUiVisibilityChange(navBarVisible);
  }

  private void checkCameraPermission() {
    // Checks if user has consent of camera permission and the permission is granted.
    // If camera permission is revoked, shows system permission dialog.
    // If camera permission is granted but user doesn't have consent of camera permission
    // (which means it's first time making video call), shows custom dialog instead. This
    // will only be shown to user once.
    if (!VideoUtils.hasCameraPermissionAndShownPrivacyToast(getContext())) {
      videoCallScreenDelegate.onCameraPermissionDialogShown();
      if (!VideoUtils.hasCameraPermission(getContext())) {
        requestPermissions(new String[] {permission.CAMERA}, CAMERA_PERMISSION_REQUEST_CODE);
      } else {
        PermissionsUtil.showCameraPermissionToast(getContext());
        videoCallScreenDelegate.onCameraPermissionGranted();
      }
    }
  }

  /// M: -------------- Mediatek features-------------
  private ImageButton downgradeAudioButton = null;
  private View videoScrollView = null;
  private CheckableImageButton hidePrviewButton = null;
  private CheckableImageButton dialpadButton = null;
  /// M: ALPS03506468 Record hide preview and hide previewoffView @{
  private boolean hidePreview = false;
  private ImageButton ectButton = null;
  private ImageButton blindEctButton = null;
  /// @}
  /// [Video Conference] @{
  private ImageButton addCallButton = null;
  private ImageButton mergeCallButton = null;
  private ImageButton videoConferenceButton = null;
  /// @}
  private ImageButton cancelUpgradeButton = null;

  /// M: ALPS03454545, add hold button in videocallfragment.
  private CheckableImageButton holdCallButton = null;
  /// M:[Oneway Video] @{
  private ImageButton upgradeCallButton = null;
  /// @}
  /// M: ALPS03460593, record latest visibility of fullscreen.
  private int fullscreenLatestVisibility = View.INVISIBLE;

  /// M: ALPS04025061, define disabled alhpa value here, the value equals the defination
  /// in xml resource. @{
  private static final float BUTTOON_DISABLED_ALPHA_VALUE = 0.3f;
  /// @}

  private boolean isVisible = false;

  private void initView(View view) {
    downgradeAudioButton = (ImageButton) view.findViewById(R.id.videocall_downgrade_audio);
    downgradeAudioButton.setOnClickListener(this);

    videoScrollView = view.findViewById(R.id.videocall_video_controls_scrollview)!=null ?
        view.findViewById(R.id.videocall_video_controls_scrollview) :
            view.findViewById(R.id.videocall_video_controls_scrollview_land);
    videoScrollView.setVisibility(
      getActivity().isInMultiWindowMode() ? View.GONE : View.VISIBLE);
    hidePrviewButton = (CheckableImageButton) view.findViewById(R.id.hide_preview);
    hidePrviewButton.setOnCheckedChangeListener(this);

    /// [ECT/BlindECT] @{
    ectButton = (ImageButton) view.findViewById(R.id.transfer_call);
    ectButton.setOnClickListener(this);

    blindEctButton = (ImageButton) view.findViewById(R.id.blind_transfer_call);
    blindEctButton.setOnClickListener(this);
    /// @}

    /// [CMCC video ringtone] @{
    dialpadButton = (CheckableImageButton) view.findViewById(R.id.video_dialpad);
    dialpadButton.setOnCheckedChangeListener(this);

    /// [Video Conference] @{
    addCallButton = (ImageButton) view.findViewById(R.id.add_call);
    addCallButton.setOnClickListener(this);

    mergeCallButton = (ImageButton) view.findViewById(R.id.merge_call);
    mergeCallButton.setOnClickListener(this);

    videoConferenceButton = (ImageButton) view.findViewById(R.id.manage_video_conference);
    videoConferenceButton.setOnClickListener(this);
    /// @}

    /// [Cancel upgrade] @{
    cancelUpgradeButton = (ImageButton) view.findViewById(R.id.cancel_upgrade);
    cancelUpgradeButton.setOnClickListener(this);
    /// @}

    /// [Video hold] @{
    holdCallButton = (CheckableImageButton) view.findViewById(R.id.hold_call);
    holdCallButton.setOnCheckedChangeListener(this);
    /// @}

    /// [Oneway Video] @{
    upgradeCallButton = (ImageButton) view.findViewById(R.id.videocall_upgrade);
    upgradeCallButton.setOnClickListener(this);
    /// @}
  }

  private void handleCheckedChanged(CheckableImageButton button,
      boolean isChecked) {
    LogUtil.i("VideoCallFragment.handleCheckedChanged", "button: %s, checked: %b",
        button, isChecked);
    if (button == hidePrviewButton) {
      inCallButtonUiDelegate.hidePreviewClicked(isChecked);
      videoCallScreenDelegate.resetAutoFullscreenTimer();
      hidePrviewButton.setChecked(isChecked);
    /// [CMCC video ringtone] @{
    } else if (button == dialpadButton) {
      inCallButtonUiDelegate.showDialpadClicked(isChecked);
      videoCallScreenDelegate.resetAutoFullscreenTimer();
      dialpadButton.setChecked(isChecked);
    /// @}
    /// [Video hold] @{
    } else if (button == holdCallButton) {
      inCallButtonUiDelegate.holdClicked(isChecked);
      videoCallScreenDelegate.resetAutoFullscreenTimer();
      holdCallButton.setChecked(isChecked);
    /// @}
    }
  }

  private void handleClick(View v) {
    if (v == downgradeAudioButton) {
      LogUtil.i("VideoCallFragment.onClick", "downgrade audio button clicked");
      inCallButtonUiDelegate.changeToAudioClicked();
    } else if (v == ectButton) {
      LogUtil.i("VideoCallFragment.onClick", "ect button clicked");
      inCallButtonUiDelegate.onConsultativeEctClicked();
    } else if (v == blindEctButton) {
      LogUtil.i("VideoCallFragment.onClick", "blind ect button clicked");
      inCallButtonUiDelegate.onBlindOrAssuredEctClicked();
    /// [Video Conference] @{
    } else if (v == addCallButton) {
      LogUtil.i("VideoCallFragment.onClick", "add call button clicked");
      inCallButtonUiDelegate.addCallClicked();
    } else if (v == mergeCallButton) {
      LogUtil.i("VideoCallFragment.onClick", "merge call button clicked");
      inCallButtonUiDelegate.mergeClicked();
    } else if (v == videoConferenceButton) {
      LogUtil.i("VideoCallFragment.onClick", "manage conference button clicked");
      inCallScreenDelegate.onManageConferenceClicked();
    /// @}
    /// [Cancel upgrade] @{
    } else if (v == cancelUpgradeButton) {
      LogUtil.i("VideoCallFragment.onClick", "cancel ugrade button clicked");
      inCallButtonUiDelegate.cancelUpgradeClicked();
    /// @}
    /// [Oneway Video] @{
    } else if (v == upgradeCallButton) {
      LogUtil.i("VideoCallFragment.onClick", "upgrade call button clicked");
      inCallButtonUiDelegate.changeToVideoClicked();
    }
    /// @}
    videoCallScreenDelegate.resetAutoFullscreenTimer();
  }

  private void enableExtButton(@InCallButtonIds int buttonId, boolean enable) {
    if (buttonId == InCallButtonIds.BUTTON_DOWNGRADE_TO_AUDIO) {
      downgradeAudioButton.setEnabled(enable);
    } else if (buttonId == InCallButtonIds.BUTTON_HIDE_PREVIEW) {
      hidePrviewButton.setEnabled(enable);
    } else if (buttonId == InCallButtonIds.BUTTON_ECT) {
      ectButton.setEnabled(enable);
    } else if (buttonId == InCallButtonIds.BUTTON_BLIND_ECT) {
      blindEctButton.setEnabled(enable);
    /// [CMCC video ringtone] @{
    } else if (buttonId == InCallButtonIds.BUTTON_DIALPAD) {
      dialpadButton.setEnabled(enable);
    /// [Video Conference] @{
    } else if (buttonId == InCallButtonIds.BUTTON_ADD_CALL) {
      addCallButton.setEnabled(enable);
    } else if (buttonId == InCallButtonIds.BUTTON_MERGE) {
      mergeCallButton.setEnabled(enable);
    } else if (buttonId == InCallButtonIds.BUTTON_MANAGE_VIDEO_CONFERENCE) {
      videoConferenceButton.setEnabled(enable);
    /// @}
    /// [Cancel upgrade] @{
    } else if (buttonId == InCallButtonIds.BUTTON_CANCEL_UPGRADE) {
      cancelUpgradeButton.setEnabled(enable);
    /// @}
    /// [Video hold] @{
    } else if (buttonId == InCallButtonIds.BUTTON_HOLD) {
      holdCallButton.setEnabled(enable);
    /// @}
    /// [Oneway Video] @{
    } else if (buttonId == InCallButtonIds.BUTTON_UPGRADE_TO_VIDEO) {
      upgradeCallButton.setEnabled(enable);
    }
    /// @}
  }

  private void showExtButtons(@InCallButtonIds int buttonId, boolean show) {
    if (buttonId == InCallButtonIds.BUTTON_DOWNGRADE_TO_AUDIO) {
      downgradeAudioButton.setVisibility(show ? View.VISIBLE : View.GONE);
    } else if (buttonId == InCallButtonIds.BUTTON_HIDE_PREVIEW) {
      hidePrviewButton.setVisibility(show ? View.VISIBLE : View.GONE);
    } else if (buttonId == InCallButtonIds.BUTTON_ECT) {
      ectButton.setVisibility(show ? View.VISIBLE : View.GONE);
    } else if (buttonId == InCallButtonIds.BUTTON_BLIND_ECT) {
      blindEctButton.setVisibility(show ? View.VISIBLE : View.GONE);
    /// [CMCC video ringtone] @{
    } else if (buttonId == InCallButtonIds.BUTTON_DIALPAD) {
      dialpadButton.setVisibility(show ? View.VISIBLE : View.GONE);
    /// [Video Conference] @{
    } else if (buttonId == InCallButtonIds.BUTTON_ADD_CALL) {
      addCallButton.setVisibility(show ? View.VISIBLE : View.GONE);
    } else if (buttonId == InCallButtonIds.BUTTON_MERGE) {
      mergeCallButton.setVisibility(show ? View.VISIBLE : View.GONE);
    } else if (buttonId == InCallButtonIds.BUTTON_MANAGE_VIDEO_CONFERENCE) {
      videoConferenceButton.setVisibility(show ? View.VISIBLE : View.GONE);
    /// @}
    /// [Cancel upgrade] @{
    } else if (buttonId == InCallButtonIds.BUTTON_CANCEL_UPGRADE) {
      cancelUpgradeButton.setVisibility(show ? View.VISIBLE : View.GONE);
    /// @}
    /// [Video hold] @{
    } else if (buttonId == InCallButtonIds.BUTTON_HOLD) {
      holdCallButton.setVisibility(show ? View.VISIBLE : View.GONE);
    /// @}
    /// [Oneway Video] @{
    } else if (buttonId == InCallButtonIds.BUTTON_UPGRADE_TO_VIDEO) {
      upgradeCallButton.setVisibility(show ? View.VISIBLE : View.GONE);
    }
    /// @}
  }

  private void enableExtButtons(boolean enable) {
    downgradeAudioButton.setEnabled(enable);
    hidePrviewButton.setEnabled(enable);
    /// [ECT/BlindECT] @{
    ectButton.setEnabled(enable);
    blindEctButton.setEnabled(enable);
    /// @}
    /// [CMCC video ringtone] @{
    dialpadButton.setEnabled(enable);
    /// [Video Conference] @{
    addCallButton.setEnabled(enable);
    mergeCallButton.setEnabled(enable);
    videoConferenceButton.setEnabled(enable);
    /// @}
    /// [Cancel upgrade] @{
    cancelUpgradeButton.setEnabled(enable);
    /// @}
    /// [Video hold] @{
    holdCallButton.setEnabled(enable);
    /// @}
    /// [Oneway Video] @{
    upgradeCallButton.setEnabled(enable);
    /// @}
  }

  @Override
  public void hidePreview(boolean hide) {
    if (previewTextureView == null) {
      return;
    }
    hidePreview = hide;
    LogUtil.i("VideoCallFragment.hidePreview", "hide: " + hide);
    previewTextureView.setVisibility(hide ? View.GONE : View.VISIBLE);
    /// M: ALPS03453557 update state @{
    if (hide) {
      updatePreviewVideoScaling();
      updatePreviewOffView();
    }
    hidePrviewButton.setChecked(hide);
    /// @}
  }

  /// M: [Voice Record]
  @Override
  public void updateRecordStateUi(boolean isRecording) {
    // do nothing.
  }

  @Override
  public void updateDeclineTimer() {}

  private VideoDebugInfo videoDebugInfo = null;

  @Override
  public void updateVideoDebugInfo(long dataUsage) {
    if (videoDebugInfo != null && VideoDebugInfo.isFeatureOn()) {
      videoDebugInfo.onCallDataUsageChange(dataUsage);
    }
  }

  @Override
  public int getDialpadContainerResourceId() {
    return R.id.incall_dialpad_container;
  }

  @Override
  public void onVideoCallScreenDialpadVisibilityChange(boolean isShowing) {
    dialpadButton.setChecked(isShowing);
  }

  /// M:[ALPS03837665] add local and peer swap video fuction for support CMCC and CT do video
  /// call's quality test. @{
  @Override
  public void switchLocalAndPeer(boolean shouldShowLargePreview) {
    LogUtil.i("VideoCallFragment.switchLocalAndPeer",
        "shouldShowLargePreview: %b", shouldShowLargePreview);
    if (getActivity() == null) {
      LogUtil.i("VideoCallFragment.switchLocalAndPeer", "not attached to activity");
      return;
    }
    if (shouldShowLargePreview) {
      enterLargeLocalMode();
      enterSmalPeerMode();
    } else {
      enterLargePeerMode();
      exitGreenScreenMode();
    }
  }

  private void enterLargeLocalMode() {
    LogUtil.i("VideoCallFragment.enterLargeLocalMode", null);
    RelativeLayout.LayoutParams params =
        new RelativeLayout.LayoutParams(
            RelativeLayout.LayoutParams.MATCH_PARENT, RelativeLayout.LayoutParams.MATCH_PARENT);
    params.addRule(RelativeLayout.ALIGN_PARENT_START);
    params.addRule(RelativeLayout.ALIGN_PARENT_TOP);
    previewTextureView.setLayoutParams(params);
    previewTextureView.setTranslationX(0);
    previewTextureView.setTranslationY(0);
    previewTextureView.setOutlineProvider(null);

    previewOffBlurredImageView.setLayoutParams(params);
    previewOffBlurredImageView.setOutlineProvider(null);
    previewOffBlurredImageView.setClipToOutline(false);
  }

  private void enterLargePeerMode() {
    LogUtil.i("VideoCallFragment.enterLargePeerMode", null);
    RelativeLayout.LayoutParams params =
        new RelativeLayout.LayoutParams(
        RelativeLayout.LayoutParams.MATCH_PARENT, RelativeLayout.LayoutParams.MATCH_PARENT);
    params.addRule(RelativeLayout.ALIGN_PARENT_START);
    params.addRule(RelativeLayout.ALIGN_PARENT_TOP);
    remoteTextureView.setLayoutParams(params);
    remoteTextureView.setOutlineProvider(null);
  }

  private void enterSmalPeerMode() {
    LogUtil.i("VideoCallFragment.enterSmalPeerMode", null);
    Resources resources = getResources();
    RelativeLayout.LayoutParams params =
        new RelativeLayout.LayoutParams(
            (int) resources.getDimension(R.dimen.videocall_preview_width),
            (int) resources.getDimension(R.dimen.videocall_preview_height));
    params.setMargins(
        0, 0, 0, (int) resources.getDimension(R.dimen.videocall_preview_margin_bottom));
    if (isLandscape()) {
      params.addRule(RelativeLayout.ALIGN_PARENT_END);
      params.setMarginEnd((int) resources.getDimension(R.dimen.videocall_preview_margin_end));
    } else {
      params.addRule(RelativeLayout.ALIGN_PARENT_START);
      params.setMarginStart((int) resources.getDimension(R.dimen.videocall_preview_margin_start));
    }
    params.addRule(RelativeLayout.ALIGN_PARENT_BOTTOM);
    remoteTextureView.setLayoutParams(params);
    remoteTextureView.setOutlineProvider(circleOutlineProvider);
    remoteTextureView.setClipToOutline(true);
  }
  /// @}

}

