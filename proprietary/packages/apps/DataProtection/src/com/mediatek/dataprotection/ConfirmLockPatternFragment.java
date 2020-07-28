package com.mediatek.dataprotection;

import com.android.internal.widget.LinearLayoutWithDefaultTouchRecepient;
import com.android.internal.widget.LockPatternUtils;
import com.android.internal.widget.LockPatternView;
import com.android.internal.widget.LockPatternView.Cell;


import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.os.SystemClock;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.mediatek.dataprotection.utils.DataProtectionLockPatternUtils;
import com.mediatek.dataprotection.utils.Log;

import java.util.List;

public class ConfirmLockPatternFragment extends Fragment {

    // how long we wait to clear a wrong pattern
    private static final int WRONG_PATTERN_CLEAR_TIMEOUT_MS = 2000;

    private static final String KEY_NUM_WRONG_ATTEMPTS = "num_wrong_attempts";

    public static final String PACKAGE = "com.mediatek.dataprotection";
    public static final String HEADER_TEXT = PACKAGE
            + ".ConfirmLockPattern.header";
    public static final String FOOTER_TEXT = PACKAGE
            + ".ConfirmLockPattern.footer";
    public static final String HEADER_WRONG_TEXT = PACKAGE
            + ".ConfirmLockPattern.header_wrong";
    public static final String FOOTER_WRONG_TEXT = PACKAGE
            + ".ConfirmLockPattern.footer_wrong";

    private static final int FAILED_ATTEMPTS_BEFORE_TIMEOUT = 5;

    private static final String TAG = "ConfirmLockPatternFragment";

    private static final String EXTRA_HEADER_TEXT = "header_text";

    private LockPatternView mLockPatternView;
    private LockPatternUtils mLockPatternUtils;
    private DataProtectionLockPatternUtils mLocalPatternUtils;
    private int mNumWrongConfirmAttempts;
    private CountDownTimer mCountdownTimer;

    private TextView mHeaderTextView;
    private TextView mFooterTextView;

    // caller-supplied text for various prompts
    private CharSequence mHeaderText;
    private PatternEventListener mListener;

    /**
     * This method create one Fragment and show.
     *
     * @param fm FragmentManager to manager created Fragment
     * @param id created fragment label
     * @param listener listene user operation in fragment
     * @param tag fragment tag
     */
    public static void show(FragmentManager fm, int id,
            PatternEventListener listener, String tag) {
        final ConfirmLockPatternFragment fragment = new ConfirmLockPatternFragment(
                listener);

        final FragmentTransaction ft = fm.beginTransaction();
        ft.replace(id, fragment, tag);
        ft.commitAllowingStateLoss();
    }

    /**
     * This method create one Fragment and show.
     *
     * @param fm FragmentManager to manager created Fragment
     * @param id created fragment label
     * @param listener listene user operation in fragment
     * @param tag fragment tag
     * @param headerText used to alert user
     */
    public static void show(FragmentManager fm, int id,
            PatternEventListener listener, String tag, String headerText) {
        final Bundle args = new Bundle();
        if (headerText != null) {
            args.putString(EXTRA_HEADER_TEXT, headerText);
            Log.d(TAG, "headerText: " + headerText);
        }

        final ConfirmLockPatternFragment fragment = new ConfirmLockPatternFragment(
                listener);
        fragment.setArguments(args);

        final FragmentTransaction ft = fm.beginTransaction();
        ft.replace(id, fragment, tag);
        ft.commitAllowingStateLoss();
    }

    interface PatternEventListener {
        void onPatternVerifySuccess(String pattern);

        void onCancel();

        void onPatternNotSet();
    }

    // required constructor for fragments
    public ConfirmLockPatternFragment() {
    }

    public ConfirmLockPatternFragment(PatternEventListener listener) {
        mListener = listener;
    }

    static class Stage {
        static final int NeedToUnlock = 1;
        static final int NeedToUnlockWrong = 2;
        static final int LockedOut = 3;
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mLockPatternUtils = new LockPatternUtils(getActivity());
        mLocalPatternUtils = new DataProtectionLockPatternUtils(getActivity());
        Log.d(TAG, "onCreate...");
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        Log.d(TAG, "onCreateView...");
        View view = inflater.inflate(R.layout.confirm_lock_pattern, null);
        mHeaderTextView = (TextView) view.findViewById(R.id.headerText);
        mLockPatternView = (LockPatternView) view
                .findViewById(R.id.lockPattern);
        mLockPatternView
                .setOnPatternListener(mConfirmExistingLockPatternListener);

        mFooterTextView = (TextView) view.findViewById(R.id.footerText);

        // make it so unhandled touch events within the unlock screen go to
        // the lock pattern view.
        final LinearLayoutWithDefaultTouchRecepient topLayout = (LinearLayoutWithDefaultTouchRecepient) view
                .findViewById(R.id.topLayout);
        topLayout.setDefaultTouchRecepient(mLockPatternView);

        Bundle data = savedInstanceState;
        if (data == null) {
            data = getArguments();
        }
        mHeaderText = data != null ? data.getString(EXTRA_HEADER_TEXT) : null;
        Log.d(TAG, "headerText onCreateView " + mHeaderText);

        mLockPatternView.setTactileFeedbackEnabled(mLockPatternUtils
                .isTactileFeedbackEnabled());
        mLockPatternView
                .setOnPatternListener(mConfirmExistingLockPatternListener);
        updateStage(Stage.NeedToUnlock);

        if (savedInstanceState != null) {
            mNumWrongConfirmAttempts = savedInstanceState
                    .getInt(KEY_NUM_WRONG_ATTEMPTS);
        } else {
            // on first launch, if no lock pattern is set, then finish with
            // success (don't want user to get stuck confirming something
            if (!mLocalPatternUtils.isPatternSet()) {
                if (mListener != null) {
                    mListener.onPatternNotSet();
                    Log.d(TAG, "Pattern: has not set pattern...");
                }
            }
        }
        return view;
    }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        // deliberately not calling super since we are managing this in full
        outState.putInt(KEY_NUM_WRONG_ATTEMPTS, mNumWrongConfirmAttempts);
    }

    @Override
    public void onPause() {
        super.onPause();

        if (mCountdownTimer != null) {
            mCountdownTimer.cancel();
        }
    }

    @Override
    public void onResume() {
        super.onResume();

        // if the user is currently locked out, enforce it.
        long deadline = DataProtectionApplication.getLockoutAttemptDeadline(getActivity());
        if (deadline != 0) {
            handleAttemptLockout(deadline);
        } else if (!mLockPatternView.isEnabled()) {
            // The deadline has passed, but the timer was cancelled...
            // Need to clean up.
            mNumWrongConfirmAttempts = 0;
            updateStage(Stage.NeedToUnlock);
        }
    }

    private void updateStage(int stage) {
        switch (stage) {
        case Stage.NeedToUnlock:
            if (mHeaderText != null) {
                mHeaderTextView.setText(mHeaderText);
            } else {
                mHeaderTextView.setText(getString(R.string.unlock_pattern_header));
            }

            mFooterTextView
                    .setText(R.string.lockpattern_need_to_unlock_footer);
            mLockPatternView.setEnabled(true);
            mLockPatternView.enableInput();
            break;
        case Stage.NeedToUnlockWrong:
            mHeaderTextView
                    .setText(R.string.lockpattern_need_to_unlock_wrong);

            mFooterTextView
                    .setText(R.string.lockpattern_need_to_unlock_wrong_footer);

            mLockPatternView.setDisplayMode(LockPatternView.DisplayMode.Wrong);
            mLockPatternView.setEnabled(true);
            mLockPatternView.enableInput();
            break;
        case Stage.LockedOut:
            mLockPatternView.clearPattern();
            // enabled = false means: disable input, and have the
            // appearance of being disabled.
            mLockPatternView.setEnabled(false);
            break;
        }

        // Always announce the header for accessibility. This is a no-op
        // when accessibility is disabled.
        mHeaderTextView.announceForAccessibility(mHeaderTextView.getText());
    }

    private Runnable mClearPatternRunnable = new Runnable() {
        public void run() {
            mLockPatternView.clearPattern();
        }
    };

    // clear the wrong pattern unless they have started a new one
    private void postClearPatternRunnable() {
        mLockPatternView.removeCallbacks(mClearPatternRunnable);
        mLockPatternView.postDelayed(mClearPatternRunnable,
                WRONG_PATTERN_CLEAR_TIMEOUT_MS);
    }

    /**
     * The pattern listener that responds according to a user confirming an
     * existing lock pattern.
     */
    private LockPatternView.OnPatternListener mConfirmExistingLockPatternListener = new LockPatternView.OnPatternListener() {

        public void onPatternStart() {
            mLockPatternView.removeCallbacks(mClearPatternRunnable);
        }

        public void onPatternCleared() {
            mLockPatternView.removeCallbacks(mClearPatternRunnable);
        }

        public void onPatternCellAdded(List<Cell> pattern) {

        }

        public void onPatternDetected(List<LockPatternView.Cell> pattern) {
            if (mLocalPatternUtils.checkPattern(mLockPatternUtils
                    .patternToString(pattern))) {
                FragmentTransaction transaction = getFragmentManager()
                        .beginTransaction();
                transaction.remove(ConfirmLockPatternFragment.this);
                transaction.commitAllowingStateLoss();
                if (null != mListener) {
                    mListener.onPatternVerifySuccess(mLockPatternUtils
                            .patternToString(pattern));
                }
            } else {
                if (pattern.size() >= LockPatternUtils.MIN_PATTERN_REGISTER_FAIL
                        && ++mNumWrongConfirmAttempts >= FAILED_ATTEMPTS_BEFORE_TIMEOUT) {
                    long deadline = DataProtectionApplication.setLockoutAttemptDeadline(getActivity());
                    Log.d("TEST", "deadline " + deadline);
                    handleAttemptLockout(deadline);
                } else {
                    updateStage(Stage.NeedToUnlockWrong);
                    postClearPatternRunnable();
                }
            }
        }
    };

    private void handleAttemptLockout(long elapsedRealtimeDeadline) {
        updateStage(Stage.LockedOut);
        long elapsedRealtime = SystemClock.elapsedRealtime();
        mCountdownTimer = new CountDownTimer(elapsedRealtimeDeadline
                - elapsedRealtime,
                LockPatternUtils.FAILED_ATTEMPT_COUNTDOWN_INTERVAL_MS) {

            @Override
            public void onTick(long millisUntilFinished) {
                mHeaderTextView
                        .setText(getString(R.string.lockpattern_too_many_failed_confirmation_attempts_header));
                final int secondsCountdown = (int) (millisUntilFinished / 1000);
                mFooterTextView
                        .setText(String
                                .format(getString(R.string.lockpattern_too_many_failed_confirmation_attempts_footer),
                                        secondsCountdown));
            }

            @Override
            public void onFinish() {
                mNumWrongConfirmAttempts = 0;
                updateStage(Stage.NeedToUnlock);
            }
        } .start();
    }
}
