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
 * limitations under the License.
 */
package com.android.keyguard;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.admin.DevicePolicyManager;
import android.content.Context;
import android.content.res.ColorStateList;
import android.graphics.Rect;
import android.metrics.LogMaker;
import android.os.UserHandle;
import android.util.AttributeSet;
import android.util.Log;
import android.util.Slog;
import android.util.StatsLog;
import android.util.TypedValue;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.VelocityTracker;
import android.view.View;
import android.view.ViewConfiguration;
import android.view.WindowManager;
import android.widget.FrameLayout;

import androidx.annotation.VisibleForTesting;
import androidx.dynamicanimation.animation.DynamicAnimation;
import androidx.dynamicanimation.animation.SpringAnimation;

import com.android.internal.logging.MetricsLogger;
import com.android.internal.logging.nano.MetricsProto.MetricsEvent;
import com.android.internal.widget.LockPatternUtils;
import com.android.keyguard.KeyguardSecurityModel.SecurityMode;
import com.android.systemui.Dependency;
import com.android.systemui.SystemUIFactory;
import com.android.systemui.util.InjectionInflationController;

import com.mediatek.keyguard.AntiTheft.AntiTheftManager;
import com.mediatek.keyguard.Telephony.KeyguardSimPinPukMeView;
import com.mediatek.keyguard.VoiceWakeup.VoiceWakeupManagerProxy;

public class KeyguardSecurityContainer extends FrameLayout implements KeyguardSecurityView {
    private static final boolean DEBUG = KeyguardConstants.DEBUG;
    private static final String TAG = "KeyguardSecurityView";

    private static final int USER_TYPE_PRIMARY = 1;
    private static final int USER_TYPE_WORK_PROFILE = 2;
    private static final int USER_TYPE_SECONDARY_USER = 3;

    // Bouncer is dismissed due to no security.
    private static final int BOUNCER_DISMISS_NONE_SECURITY = 0;
    // Bouncer is dismissed due to pin, password or pattern entered.
    private static final int BOUNCER_DISMISS_PASSWORD = 1;
    // Bouncer is dismissed due to biometric (face, fingerprint or iris) authenticated.
    private static final int BOUNCER_DISMISS_BIOMETRIC = 2;
    // Bouncer is dismissed due to extended access granted.
    private static final int BOUNCER_DISMISS_EXTENDED_ACCESS = 3;
    // Bouncer is dismissed due to sim card unlock code entered.
    private static final int BOUNCER_DISMISS_SIM = 4;

    // Make the view move slower than the finger, as if the spring were applying force.
    private static final float TOUCH_Y_MULTIPLIER = 0.25f;
    // How much you need to drag the bouncer to trigger an auth retry (in dps.)
    private static final float MIN_DRAG_SIZE = 10;
    // How much to scale the default slop by, to avoid accidental drags.
    private static final float SLOP_SCALE = 2f;

    private KeyguardSecurityModel mSecurityModel;
    private LockPatternUtils mLockPatternUtils;

    private KeyguardSecurityViewFlipper mSecurityViewFlipper;
    private boolean mIsVerifyUnlockOnly;
    private SecurityMode mCurrentSecuritySelection = SecurityMode.Invalid;
    private KeyguardSecurityView mCurrentSecurityView;
    private SecurityCallback mSecurityCallback;
    private AlertDialog mAlertDialog;
    private InjectionInflationController mInjectionInflationController;
    private boolean mSwipeUpToRetry;

    private final ViewConfiguration mViewConfiguration;
    private final SpringAnimation mSpringAnimation;
    private final VelocityTracker mVelocityTracker = VelocityTracker.obtain();
    private final KeyguardUpdateMonitor mUpdateMonitor;

    private final MetricsLogger mMetricsLogger = Dependency.get(MetricsLogger.class);
    private float mLastTouchY = -1;
    private int mActivePointerId = -1;
    private boolean mIsDragging;
    private float mStartTouchY = -1;

    // Used to notify the container when something interesting happens.
    public interface SecurityCallback {
        public boolean dismiss(boolean authenticated, int targetUserId);
        public void userActivity();
        public void onSecurityModeChanged(SecurityMode securityMode, boolean needsInput);

        /**
         * @param strongAuth wheher the user has authenticated with strong authentication like
         *                   pattern, password or PIN but not by trust agents or fingerprint
         * @param targetUserId a user that needs to be the foreground user at the finish completion.
         */
        public void finish(boolean strongAuth, int targetUserId);
        public void reset();
        public void onCancelClicked();
        ///M: add more callbacks
        public void updateNavbarStatus() ;
    }

    public KeyguardSecurityContainer(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }

    public KeyguardSecurityContainer(Context context) {
        this(context, null, 0);
    }

    public KeyguardSecurityContainer(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        mSecurityModel = new KeyguardSecurityModel(context);
        mLockPatternUtils = new LockPatternUtils(context);
        mUpdateMonitor = KeyguardUpdateMonitor.getInstance(mContext);
        mSpringAnimation = new SpringAnimation(this, DynamicAnimation.Y);
        mInjectionInflationController =  new InjectionInflationController(
            SystemUIFactory.getInstance().getRootComponent());
        mViewConfiguration = ViewConfiguration.get(context);
    }

    public void setSecurityCallback(SecurityCallback callback) {
        mSecurityCallback = callback;
    }

    @Override
    public void onResume(int reason) {
        if (mCurrentSecuritySelection != SecurityMode.None) {
            getSecurityView(mCurrentSecuritySelection).onResume(reason);
        }
        updateBiometricRetry();
    }

    @Override
    public void onPause() {
        if (mAlertDialog != null) {
            mAlertDialog.dismiss();
            mAlertDialog = null;
        }
        if (mCurrentSecuritySelection != SecurityMode.None) {
            getSecurityView(mCurrentSecuritySelection).onPause();
        }
    }

    @Override
    public boolean shouldDelayChildPressedState() {
        return true;
    }

    @Override
    public boolean onInterceptTouchEvent(MotionEvent event) {
        switch (event.getActionMasked()) {
            case MotionEvent.ACTION_DOWN:
                int pointerIndex = event.getActionIndex();
                mStartTouchY = event.getY(pointerIndex);
                mActivePointerId = event.getPointerId(pointerIndex);
                mVelocityTracker.clear();
                break;
            case MotionEvent.ACTION_MOVE:
                if (mIsDragging) {
                    return true;
                }
                if (!mSwipeUpToRetry) {
                    return false;
                }
                // Avoid dragging the pattern view
                if (mCurrentSecurityView.disallowInterceptTouch(event)) {
                    return false;
                }
                int index = event.findPointerIndex(mActivePointerId);
                float touchSlop = mViewConfiguration.getScaledTouchSlop() * SLOP_SCALE;
                if (mCurrentSecurityView != null && index != -1
                        && mStartTouchY - event.getY(index) > touchSlop) {
                    mIsDragging = true;
                    return true;
                }
                break;
            case MotionEvent.ACTION_CANCEL:
            case MotionEvent.ACTION_UP:
                mIsDragging = false;
                break;
        }
        return false;
    }

    @Override
    public boolean onTouchEvent(MotionEvent event) {
        final int action = event.getActionMasked();
        switch (action) {
            case MotionEvent.ACTION_MOVE:
                mVelocityTracker.addMovement(event);
                int pointerIndex = event.findPointerIndex(mActivePointerId);
                float y = event.getY(pointerIndex);
                if (mLastTouchY != -1) {
                    float dy = y - mLastTouchY;
                    setTranslationY(getTranslationY() + dy * TOUCH_Y_MULTIPLIER);
                }
                mLastTouchY = y;
                break;
            case MotionEvent.ACTION_UP:
            case MotionEvent.ACTION_CANCEL:
                mActivePointerId = -1;
                mLastTouchY = -1;
                mIsDragging = false;
                startSpringAnimation(mVelocityTracker.getYVelocity());
                break;
            case MotionEvent.ACTION_POINTER_UP:
                int index = event.getActionIndex();
                int pointerId = event.getPointerId(index);
                if (pointerId == mActivePointerId) {
                    // This was our active pointer going up. Choose a new
                    // active pointer and adjust accordingly.
                    final int newPointerIndex = index == 0 ? 1 : 0;
                    mLastTouchY = event.getY(newPointerIndex);
                    mActivePointerId = event.getPointerId(newPointerIndex);
                }
                break;
        }
        if (action == MotionEvent.ACTION_UP) {
            if (-getTranslationY() > TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP,
                    MIN_DRAG_SIZE, getResources().getDisplayMetrics())) {
                mUpdateMonitor.requestFaceAuth();
            }
        }
        return true;
    }

    private void startSpringAnimation(float startVelocity) {
        mSpringAnimation
            .setStartVelocity(startVelocity)
            .animateToFinalPosition(0);
    }

    public void startAppearAnimation() {
        if (mCurrentSecuritySelection != SecurityMode.None) {
            getSecurityView(mCurrentSecuritySelection).startAppearAnimation();
        }
    }

    public boolean startDisappearAnimation(Runnable onFinishRunnable) {
        if (mCurrentSecuritySelection != SecurityMode.None) {
            return getSecurityView(mCurrentSecuritySelection).startDisappearAnimation(
                    onFinishRunnable);
        }
        return false;
    }

    /**
     * Enables/disables swipe up to retry on the bouncer.
     */
    private void updateBiometricRetry() {
        SecurityMode securityMode = getSecurityMode();
        int userId = KeyguardUpdateMonitor.getCurrentUser();
        mSwipeUpToRetry = mUpdateMonitor.isUnlockWithFacePossible(userId)
                && securityMode != SecurityMode.SimPinPukMe1
                && securityMode != SecurityMode.SimPinPukMe2
                && securityMode != SecurityMode.SimPinPukMe3
                && securityMode != SecurityMode.SimPinPukMe4
                && securityMode != SecurityMode.None;
    }

    public CharSequence getTitle() {
        return mSecurityViewFlipper.getTitle();
    }

    private KeyguardSecurityView getSecurityView(SecurityMode securityMode) {
        final int securityViewIdForMode = getSecurityViewIdForMode(securityMode);
        KeyguardSecurityView view = null;
        final int children = mSecurityViewFlipper.getChildCount();
        for (int child = 0; child < children; child++) {
            if (mSecurityViewFlipper.getChildAt(child).getId() == securityViewIdForMode) {
                view = ((KeyguardSecurityView)mSecurityViewFlipper.getChildAt(child));
                break;
            }
        }
        int layoutId = getLayoutIdFor(securityMode);
        if (view == null && layoutId != 0) {
            final LayoutInflater inflater = LayoutInflater.from(mContext);
            if (DEBUG) Log.v(TAG, "inflating id = " + layoutId);
            View v = mInjectionInflationController.injectable(inflater)
                    .inflate(layoutId, mSecurityViewFlipper, false);
            view = (KeyguardSecurityView)v;
            /// M: Use KeygaurdSimPinPukView for pin/puk, so set sim Id for it
            if (view instanceof KeyguardSimPinPukMeView) {
                KeyguardSimPinPukMeView pinPukView = (KeyguardSimPinPukMeView) view;
                final int phoneId = mSecurityModel.getPhoneIdUsingSecurityMode(securityMode);
                pinPukView.setPhoneId(phoneId);
            }
            mSecurityViewFlipper.addView(v);
            updateSecurityView(v);
            view.reset();
        } else if (view != null && (view instanceof KeyguardSimPinPukMeView)
                  && (securityMode != mCurrentSecuritySelection)) {
            ///M: mediatek add only for PIN1 and PIN2, in this case,
            //we needn't recreate from the same layout file
             Log.i(TAG, "getSecurityView, here, we will refresh the layout");
             KeyguardSimPinPukMeView pinPukView = (KeyguardSimPinPukMeView) view;
             final int phoneId = mSecurityModel.getPhoneIdUsingSecurityMode(securityMode);
             pinPukView.setPhoneId(phoneId);
        }

        return view;
    }

    private void updateSecurityView(View view) {
        if (view instanceof KeyguardSecurityView) {
            KeyguardSecurityView ksv = (KeyguardSecurityView) view;
            ksv.setKeyguardCallback(mCallback);
            ksv.setLockPatternUtils(mLockPatternUtils);
        } else {
            Log.w(TAG, "View " + view + " is not a KeyguardSecurityView");
        }
    }

    protected void onFinishInflate() {
        mSecurityViewFlipper = findViewById(R.id.view_flipper);
        mSecurityViewFlipper.setLockPatternUtils(mLockPatternUtils);
    }

    public void setLockPatternUtils(LockPatternUtils utils) {
        mLockPatternUtils = utils;
        mSecurityModel.setLockPatternUtils(utils);
        mSecurityViewFlipper.setLockPatternUtils(mLockPatternUtils);
    }

    @Override
    protected boolean fitSystemWindows(Rect insets) {
        // Consume bottom insets because we're setting the padding locally (for IME and navbar.)
        setPadding(getPaddingLeft(), getPaddingTop(), getPaddingRight(), insets.bottom);
        insets.bottom = 0;
        return false;
    }

    private void showDialog(String title, String message) {
        if (mAlertDialog != null) {
            mAlertDialog.dismiss();
        }

        mAlertDialog = new AlertDialog.Builder(mContext)
            .setTitle(title)
            .setMessage(message)
            .setCancelable(false)
            .setNeutralButton(R.string.ok, null)
            .create();
        if (!(mContext instanceof Activity)) {
            mAlertDialog.getWindow().setType(WindowManager.LayoutParams.TYPE_KEYGUARD_DIALOG);
        }
        mAlertDialog.show();
    }

    private void showTimeoutDialog(int userId, int timeoutMs) {
        int timeoutInSeconds = (int) timeoutMs / 1000;
        int messageId = 0;

        switch (mSecurityModel.getSecurityMode(userId)) {
            case Pattern:
                messageId = R.string.kg_too_many_failed_pattern_attempts_dialog_message;
                break;
            case PIN:
                messageId = R.string.kg_too_many_failed_pin_attempts_dialog_message;
                break;
            case Password:
                messageId = R.string.kg_too_many_failed_password_attempts_dialog_message;
                break;
            // These don't have timeout dialogs.
            case Invalid:
            case None:
            // case SimPin:
            // case SimPuk:
            ///M:
            case SimPinPukMe1:
            case SimPinPukMe2:
            case SimPinPukMe3:
            case SimPinPukMe4:
                break;
        }

        if (messageId != 0) {
            final String message = mContext.getString(messageId,
                    mLockPatternUtils.getCurrentFailedPasswordAttempts(userId),
                    timeoutInSeconds);
            showDialog(null, message);
        }
    }

    private void showAlmostAtWipeDialog(int attempts, int remaining, int userType) {
        String message = null;
        switch (userType) {
            case USER_TYPE_PRIMARY:
                message = mContext.getString(R.string.kg_failed_attempts_almost_at_wipe,
                        attempts, remaining);
                break;
            case USER_TYPE_SECONDARY_USER:
                message = mContext.getString(R.string.kg_failed_attempts_almost_at_erase_user,
                        attempts, remaining);
                break;
            case USER_TYPE_WORK_PROFILE:
                message = mContext.getString(R.string.kg_failed_attempts_almost_at_erase_profile,
                        attempts, remaining);
                break;
        }
        showDialog(null, message);
    }

    private void showWipeDialog(int attempts, int userType) {
        String message = null;
        switch (userType) {
            case USER_TYPE_PRIMARY:
                message = mContext.getString(R.string.kg_failed_attempts_now_wiping,
                        attempts);
                break;
            case USER_TYPE_SECONDARY_USER:
                message = mContext.getString(R.string.kg_failed_attempts_now_erasing_user,
                        attempts);
                break;
            case USER_TYPE_WORK_PROFILE:
                message = mContext.getString(R.string.kg_failed_attempts_now_erasing_profile,
                        attempts);
                break;
        }
        showDialog(null, message);
    }

    private void reportFailedUnlockAttempt(int userId, int timeoutMs) {
        // +1 for this time
        final int failedAttempts = mLockPatternUtils.getCurrentFailedPasswordAttempts(userId) + 1;

        if (DEBUG) Log.d(TAG, "reportFailedPatternAttempt: #" + failedAttempts);

        final DevicePolicyManager dpm = mLockPatternUtils.getDevicePolicyManager();
        final int failedAttemptsBeforeWipe =
                dpm.getMaximumFailedPasswordsForWipe(null, userId);

        final int remainingBeforeWipe = failedAttemptsBeforeWipe > 0 ?
                (failedAttemptsBeforeWipe - failedAttempts)
                : Integer.MAX_VALUE; // because DPM returns 0 if no restriction
        if (remainingBeforeWipe < LockPatternUtils.FAILED_ATTEMPTS_BEFORE_WIPE_GRACE) {
            // The user has installed a DevicePolicyManager that requests a user/profile to be wiped
            // N attempts. Once we get below the grace period, we post this dialog every time as a
            // clear warning until the deletion fires.
            // Check which profile has the strictest policy for failed password attempts
            final int expiringUser = dpm.getProfileWithMinimumFailedPasswordsForWipe(userId);
            int userType = USER_TYPE_PRIMARY;
            if (expiringUser == userId) {
                // TODO: http://b/23522538
                if (expiringUser != UserHandle.USER_SYSTEM) {
                    userType = USER_TYPE_SECONDARY_USER;
                }
            } else if (expiringUser != UserHandle.USER_NULL) {
                userType = USER_TYPE_WORK_PROFILE;
            } // If USER_NULL, which shouldn't happen, leave it as USER_TYPE_PRIMARY
            if (remainingBeforeWipe > 0) {
                showAlmostAtWipeDialog(failedAttempts, remainingBeforeWipe, userType);
            } else {
                // Too many attempts. The device will be wiped shortly.
                Slog.i(TAG, "Too many unlock attempts; user " + expiringUser + " will be wiped!");
                showWipeDialog(failedAttempts, userType);
            }
        }
        mLockPatternUtils.reportFailedPasswordAttempt(userId);
        if (timeoutMs > 0) {
            mLockPatternUtils.reportPasswordLockout(timeoutMs, userId);
            /// M: design change, don't show Time out dialog.
            //showTimeoutDialog(userId, timeoutMs);
        }
    }

    /**
     * Shows the primary security screen for the user. This will be either the multi-selector
     * or the user's security method.
     * @param turningOff true if the device is being turned off
     */
    void showPrimarySecurityScreen(boolean turningOff) {
        SecurityMode securityMode = mSecurityModel.getSecurityMode(
                KeyguardUpdateMonitor.getCurrentUser());
        if (DEBUG) Log.v(TAG, "showPrimarySecurityScreen(turningOff=" + turningOff + ")");
        Log.v(TAG, "showPrimarySecurityScreen(securityMode=" + securityMode + ")");

        ///M: fix ALPS01856335, we should always update phone id
        ///   when current SimPinPukMe mode is not secured.
        if (mSecurityModel.isSimPinPukSecurityMode(mCurrentSecuritySelection)) {
            Log.d(TAG, "showPrimarySecurityScreen() - current is " +
                       mCurrentSecuritySelection) ;

            int phoneId = mSecurityModel.getPhoneIdUsingSecurityMode(
                mCurrentSecuritySelection) ;
            Log.d(TAG, "showPrimarySecurityScreen() - phoneId of currentView is " +
                       phoneId) ;

            boolean isCurrentModeSimPinSecure = mUpdateMonitor.isSimPinSecure(phoneId) ;
            Log.d(TAG, "showPrimarySecurityScreen() - isCurrentModeSimPinSecure = " +
                       isCurrentModeSimPinSecure) ;

            if (isCurrentModeSimPinSecure) {
                Log.d(TAG, "Skip show security because it already shows SimPinPukMeView");
                return;
            } else {
                Log.d(TAG, "showPrimarySecurityScreen() - since current simpinview not secured," +
                           " we should call showSecurityScreen() to set correct " +
                           "PhoneId for next view.") ;
            }
        }

        showSecurityScreen(securityMode);
    }

    /**
     * Shows the next security screen if there is one.
     * @param authenticated true if the user entered the correct authentication
     * @param targetUserId a user that needs to be the foreground user at the finish (if called)
     *     completion.
     * @return true if keyguard is done
     */
    boolean showNextSecurityScreenOrFinish(boolean authenticated, int targetUserId) {
        if (DEBUG) Log.d(TAG, "showNextSecurityScreenOrFinish(" + authenticated + ")");
        Log.d(TAG, "showNext.. mCurrentSecuritySelection = " + mCurrentSecuritySelection);
        boolean finish = false;
        boolean strongAuth = false;
        int eventSubtype = -1;
        if (mUpdateMonitor.getUserHasTrust(targetUserId)) {
            finish = true;
            eventSubtype = BOUNCER_DISMISS_EXTENDED_ACCESS;
        } else if (mUpdateMonitor.getUserUnlockedWithBiometric(targetUserId)) {
            finish = true;
            eventSubtype = BOUNCER_DISMISS_BIOMETRIC;
        } else if (SecurityMode.None == mCurrentSecuritySelection) {
            SecurityMode securityMode = mSecurityModel.getSecurityMode(targetUserId);
            if (SecurityMode.None == securityMode) {
                if (DEBUG) {
                    Log.d(TAG, "showNextSecurityScreenOrFinish() " +
                            "- securityMode is None, just finish.");
                }
                finish = true; // no security required
                eventSubtype = BOUNCER_DISMISS_NONE_SECURITY;
            } else {
                if (DEBUG) {
                    Log.d(TAG, "showNextSecurityScreenOrFinish()" +
                        "- switch to the alternate security view for None mode.");
                }
                showSecurityScreen(securityMode); // switch to the alternate security view
            }
        } else if (authenticated) {
            if (DEBUG) {
                Log.d(TAG, "showNextSecurityScreenOrFinish() - authenticated is True," +
                   " and mCurrentSecuritySelection = " + mCurrentSecuritySelection);
            }
            // Shortcut for SIM PIN/PUK to go to directly to user's security screen or home
            SecurityMode securityMode = mSecurityModel.getSecurityMode(
                    KeyguardUpdateMonitor.getCurrentUser());
            if (DEBUG) {
                Log.v(TAG, "securityMode = " + securityMode);
            }
            Log.d(TAG, "mCurrentSecuritySelection: " + mCurrentSecuritySelection);
            switch (mCurrentSecuritySelection) {
                case Pattern:
                case Password:
                case PIN:
                    strongAuth = true;
                    finish = true;
                    eventSubtype = BOUNCER_DISMISS_PASSWORD;
                    break;

                // case SimPin:
                // case SimPuk:
                case SimPinPukMe1:
                case SimPinPukMe2:
                case SimPinPukMe3:
                case SimPinPukMe4:
                    // Shortcut for SIM PIN/PUK to go to directly to user's security screen or home
                    //SecurityMode securityMode = mSecurityModel.getSecurityMode(targetUserId);
                    if (securityMode == SecurityMode.None) {
                        finish = true;
                        eventSubtype = BOUNCER_DISMISS_SIM;
                    } else {
                        showSecurityScreen(securityMode);
                    }
                    break;
                ///M: ALPS01772213 for handling antitheft mode.
                case AntiTheft:
                    SecurityMode nextMode = mSecurityModel.getSecurityMode(
                            KeyguardUpdateMonitor.getCurrentUser());
                    if (DEBUG) {
                        Log.v(TAG, "now is Antitheft, next securityMode = " + nextMode);
                    }
                    if (nextMode != SecurityMode.None) {
                        showSecurityScreen(nextMode);
                    } else {
                        finish = true;
                    }
                    break;
                default:
                    Log.v(TAG, "Bad security screen " + mCurrentSecuritySelection + ", fail safe");
                    showPrimarySecurityScreen(false);
                    break;
            }
        }
        if (eventSubtype != -1) {
            mMetricsLogger.write(new LogMaker(MetricsEvent.BOUNCER)
                    .setType(MetricsEvent.TYPE_DISMISS).setSubtype(eventSubtype));
        }

        mSecurityCallback.updateNavbarStatus();

        if (finish) {
            mSecurityCallback.finish(strongAuth, targetUserId);
            Log.d(TAG, "finish ");
        }

        if (DEBUG) Log.d(TAG, "showNextSecurityScreenOrFinish() - return finish = " + finish) ;
        return finish;
    }

    /**
     * Switches to the given security view unless it's already being shown, in which case
     * this is a no-op.
     *
     * @param securityMode
     */
    private void showSecurityScreen(SecurityMode securityMode) {
        if (DEBUG) Log.d(TAG, "showSecurityScreen(" + securityMode + ")");

        ///M: Though we only have one "AntiTheft" mode, it in fact covers multiple
        ///   different scenarios. Ex: DM Lock, PPL Lock, etc. They may appear consecutively.
        ///   Ex: After dismissing DM Lock, the next security mode is PPL Lock.
        ///   They share same lock view file and same SecurityMode.AntiTheft,
        ///   but 2nd antitheft view cannot show except the 1st view since we will just end
        ///   showing if  "securityMode == mCurrentSecuritySelection".
        ///   Add a new condition for AntiTheft mode to avoid the above mentioned case.
        if ((securityMode == mCurrentSecuritySelection)
                && (securityMode != SecurityMode.AntiTheft)) {
            return;
        }

        VoiceWakeupManagerProxy.getInstance().notifySecurityModeChange(
                mCurrentSecuritySelection, securityMode);

        Log.d(TAG, "showSecurityScreen() - get oldview for" + mCurrentSecuritySelection
                + ", get newview for" + securityMode);
        KeyguardSecurityView oldView = getSecurityView(mCurrentSecuritySelection);
        KeyguardSecurityView newView = getSecurityView(securityMode);

        // Emulate Activity life cycle
        if (oldView != null) {
            oldView.onPause();
            oldView.setKeyguardCallback(mNullCallback); // ignore requests from old view
        }
        if (securityMode != SecurityMode.None) {
            /// M: fix ALPS01832185,
            ///    KeyguardAntiTheftView needs the latest mCallback before onResume.
            newView.setKeyguardCallback(mCallback);
            Log.d(TAG, "showSecurityScreen() - newview.setKeyguardCallback(mCallback)");
            newView.onResume(KeyguardSecurityView.VIEW_REVEALED);
        }

        // Find and show this child.
        final int childCount = mSecurityViewFlipper.getChildCount();

        final int securityViewIdForMode = getSecurityViewIdForMode(securityMode);
        for (int i = 0; i < childCount; i++) {
            if (mSecurityViewFlipper.getChildAt(i).getId() == securityViewIdForMode) {
                mSecurityViewFlipper.setDisplayedChild(i);
                break;
            }
        }

        mCurrentSecuritySelection = securityMode;
        mCurrentSecurityView = newView;
        Log.d(TAG, "Before update, mCurrentSecuritySelection = " + securityMode
                + "After update, mCurrentSecuritySelection = " + mCurrentSecuritySelection);
        mSecurityCallback.onSecurityModeChanged(securityMode,
                securityMode != SecurityMode.None && newView.needsInput());
    }

    private KeyguardSecurityViewFlipper getFlipper() {
        for (int i = 0; i < getChildCount(); i++) {
            View child = getChildAt(i);
            if (child instanceof KeyguardSecurityViewFlipper) {
                return (KeyguardSecurityViewFlipper) child;
            }
        }
        return null;
    }

    private KeyguardSecurityCallback mCallback = new KeyguardSecurityCallback() {
        public void userActivity() {
            if (mSecurityCallback != null) {
                mSecurityCallback.userActivity();
            }
        }

        public void dismiss(boolean authenticated, int targetId) {
            mSecurityCallback.dismiss(authenticated, targetId);
        }

        public boolean isVerifyUnlockOnly() {
            return mIsVerifyUnlockOnly;
        }

        public void reportUnlockAttempt(int userId, boolean success, int timeoutMs) {
            if (success) {
                StatsLog.write(StatsLog.KEYGUARD_BOUNCER_PASSWORD_ENTERED,
                    StatsLog.KEYGUARD_BOUNCER_PASSWORD_ENTERED__RESULT__SUCCESS);
                mLockPatternUtils.reportSuccessfulPasswordAttempt(userId);
            } else {
                StatsLog.write(StatsLog.KEYGUARD_BOUNCER_PASSWORD_ENTERED,
                    StatsLog.KEYGUARD_BOUNCER_PASSWORD_ENTERED__RESULT__FAILURE);
                KeyguardSecurityContainer.this.reportFailedUnlockAttempt(userId, timeoutMs);
            }
            mMetricsLogger.write(new LogMaker(MetricsEvent.BOUNCER)
                    .setType(success ? MetricsEvent.TYPE_SUCCESS : MetricsEvent.TYPE_FAILURE));
        }

        public void reset() {
            mSecurityCallback.reset();
        }

        public void onCancelClicked() {
            mSecurityCallback.onCancelClicked();
        }
    };

    // The following is used to ignore callbacks from SecurityViews that are no longer current
    // (e.g. face unlock). This avoids unwanted asynchronous events from messing with the
    // state for the current security method.
    private KeyguardSecurityCallback mNullCallback = new KeyguardSecurityCallback() {
        @Override
        public void userActivity() { }
        @Override
        public void reportUnlockAttempt(int userId, boolean success, int timeoutMs) { }
        @Override
        public boolean isVerifyUnlockOnly() { return false; }
        @Override
        public void dismiss(boolean securityVerified, int targetUserId) { }
        @Override
        public void reset() {}
    };

    private int getSecurityViewIdForMode(SecurityMode securityMode) {
        switch (securityMode) {
            case Pattern: return R.id.keyguard_pattern_view;
            case PIN: return R.id.keyguard_pin_view;
            case Password: return R.id.keyguard_password_view;
            // case SimPin: return R.id.keyguard_sim_pin_view;
            // case SimPuk: return R.id.keyguard_sim_puk_view;
            ///M:
            case SimPinPukMe1:
            case SimPinPukMe2:
            case SimPinPukMe3:
            case SimPinPukMe4:
                return R.id.keyguard_sim_pin_puk_me_view;
            ///M: add anti-theft view id
            case AntiTheft:
                return AntiTheftManager.getAntiTheftViewId();
        }
        return 0;
    }

    @VisibleForTesting
    public int getLayoutIdFor(SecurityMode securityMode) {
        switch (securityMode) {
            case Pattern: return R.layout.keyguard_pattern_view;
            case PIN: return R.layout.keyguard_pin_view;
            case Password: return R.layout.keyguard_password_view;
            // case SimPin: return R.layout.keyguard_sim_pin_view;
            // case SimPuk: return R.layout.keyguard_sim_puk_view;
            ///M:
            case SimPinPukMe1:
            case SimPinPukMe2:
            case SimPinPukMe3:
            case SimPinPukMe4:
                return R.layout.mtk_keyguard_sim_pin_puk_me_view;
            ///M: add dmlock view layout
            case AntiTheft:
                return AntiTheftManager.getAntiTheftLayoutId();
            default:
                return 0;
        }
    }

    public SecurityMode getSecurityMode() {
        return mSecurityModel.getSecurityMode(KeyguardUpdateMonitor.getCurrentUser());
    }

    public SecurityMode getCurrentSecurityMode() {
        return mCurrentSecuritySelection;
    }

    public KeyguardSecurityView getCurrentSecurityView() {
        return mCurrentSecurityView;
    }

    public void verifyUnlock() {
        mIsVerifyUnlockOnly = true;
        showSecurityScreen(getSecurityMode());
    }

    public SecurityMode getCurrentSecuritySelection() {
        return mCurrentSecuritySelection;
    }

    public void dismiss(boolean authenticated, int targetUserId) {
        mCallback.dismiss(authenticated, targetUserId);
    }

    public boolean needsInput() {
        return mSecurityViewFlipper.needsInput();
    }

    @Override
    public void setKeyguardCallback(KeyguardSecurityCallback callback) {
        mSecurityViewFlipper.setKeyguardCallback(callback);
    }

    @Override
    public void reset() {
        mSecurityViewFlipper.reset();
    }

    @Override
    public KeyguardSecurityCallback getCallback() {
        return mSecurityViewFlipper.getCallback();
    }

    @Override
    public void showPromptReason(int reason) {
        if (mCurrentSecuritySelection != SecurityMode.None) {
            if (reason != PROMPT_REASON_NONE) {
                Log.i(TAG, "Strong auth required, reason: " + reason);
            }
            getSecurityView(mCurrentSecuritySelection).showPromptReason(reason);
        }
    }

    public void showMessage(CharSequence message, ColorStateList colorState) {
        if (mCurrentSecuritySelection != SecurityMode.None) {
            getSecurityView(mCurrentSecuritySelection).showMessage(message, colorState);
        }
    }

    @Override
    public void showUsabilityHint() {
        mSecurityViewFlipper.showUsabilityHint();
    }

}

