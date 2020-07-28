package com.mediatek.dataprotection;

import com.google.android.collect.Lists;
import com.android.internal.widget.LinearLayoutWithDefaultTouchRecepient;
import com.android.internal.widget.LockPatternUtils;
import com.android.internal.widget.LockPatternView;
import com.android.internal.widget.LockPatternView.Cell;

import static com.android.internal.widget.LockPatternView.DisplayMode;
import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.Intent;
import android.drm.DrmManagerClient;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import com.mediatek.dataprotection.ChooseLockPatternFragment.LeftButtonMode;
import com.mediatek.dataprotection.ChooseLockPatternFragment.RightButtonMode;
import com.mediatek.dataprotection.ChooseLockPatternFragment.Stage;
import com.mediatek.dataprotection.utils.CtaDrmUtils;
import com.mediatek.dataprotection.utils.DataProtectionLockPatternUtils;
import com.mediatek.dataprotection.utils.Log;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class ChooseLockPatternFragment extends Fragment implements
        View.OnClickListener {

    public static final int TYPE_SET_PATTERN = 1;
    public static final int TYPE_CHANGE_PATTERN = 2;
    public static final int CONFIRM_EXISTING_REQUEST = 55;
    static final int RESULT_FINISHED = Activity.RESULT_FIRST_USER;

    // how long after a confirmation message is shown before moving on
    static final int INFORMATION_MSG_TIMEOUT_MS = 3000;

    // how long we wait to clear a wrong pattern
    private static final int WRONG_PATTERN_CLEAR_TIMEOUT_MS = 2000;

    private static final int ID_EMPTY_MESSAGE = -1;

    protected TextView mHeaderText;
    protected LockPatternView mLockPatternView;
    protected TextView mFooterText;
    private TextView mFooterLeftButton;
    private TextView mFooterRightButton;
    protected List<LockPatternView.Cell> mChosenPattern = null;
    private PatternLockChooseEventListener mListener = null;
    private int mRequestType = -1;
    private Bundle mCurrentState = null;

    /**
     * The patten used during the help screen to show how to draw a pattern.
     */
    private final List<LockPatternView.Cell> mAnimatePattern = Collections
            .unmodifiableList(Lists.newArrayList(LockPatternView.Cell.of(0, 0),
                    LockPatternView.Cell.of(0, 1),
                    LockPatternView.Cell.of(1, 1),
                    LockPatternView.Cell.of(2, 1)));

    public static void show(FragmentManager manager,
            PatternLockChooseEventListener listener, int id, int type,
            String data, String tag, Bundle cachedState) {
        FragmentTransaction transaction = manager.beginTransaction();
        final ChooseLockPatternFragment fragment = new ChooseLockPatternFragment(
                listener, cachedState);
        Bundle bundle = new Bundle();
        bundle.putInt("TYPE", type);
        if (data != null) {
            bundle.putString("DATA", data);
        }
        fragment.setArguments(bundle);
        transaction.replace(id, fragment, tag);
        transaction.commitAllowingStateLoss();

    }

    // required constructor for fragments
    public ChooseLockPatternFragment() {
    }

    public ChooseLockPatternFragment(PatternLockChooseEventListener listener, Bundle oldState) {
        mListener = listener;
        mCurrentState = oldState;
    }

    interface PatternLockChooseEventListener {
        void onPatternSetSuccess(String pattern);

        void onCancel();

        void onPatternChangeSuccess(String oldPattern, String newPattern);
    }

    /**
     * This method query the cached pattern.
     *
     * @return cached pattern
     */
    public String getOldPattern() {
        String data = null;
        Bundle bundle = getArguments();
        if (bundle != null) {
            data = bundle.getString("DATA");
        }
        return data;
    }

    public Bundle getCurrentState() {
        if (mCurrentState == null) {
            mCurrentState = new Bundle();
        }

        mCurrentState.putInt(KEY_UI_STAGE, mUiStage.ordinal());
        if (mChosenPattern != null) {
            mCurrentState.putString(KEY_PATTERN_CHOICE,
                    LockPatternUtils.patternToString(mChosenPattern));
        }
        return mCurrentState;
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        switch (requestCode) {
        case CONFIRM_EXISTING_REQUEST:
            if (resultCode != Activity.RESULT_OK) {
                getActivity().setResult(RESULT_FINISHED);
                getActivity().finish();
            }
            updateStage(Stage.Introduction);
            break;
        }
    }

    /**
     * The pattern listener that responds according to a user choosing a new
     * lock pattern.
     */
    protected LockPatternView.OnPatternListener mChooseNewLockPatternListener = new LockPatternView.OnPatternListener() {

        public void onPatternStart() {
            mLockPatternView.removeCallbacks(mClearPatternRunnable);
            patternInProgress();
        }

        public void onPatternCleared() {
            mLockPatternView.removeCallbacks(mClearPatternRunnable);
        }

        public void onPatternDetected(List<LockPatternView.Cell> pattern) {
            if (mUiStage == Stage.NeedToConfirm
                    || mUiStage == Stage.ConfirmWrong) {
                if (mChosenPattern == null)
                    throw new IllegalStateException(
                            "null chosen pattern in stage 'need to confirm");
                if (mChosenPattern.equals(pattern)) {
                    updateStage(Stage.ChoiceConfirmed);
                } else {
                    updateStage(Stage.ConfirmWrong);
                }
            } else if (mUiStage == Stage.Introduction
                    || mUiStage == Stage.ChoiceTooShort) {
                if (pattern.size() < LockPatternUtils.MIN_LOCK_PATTERN_SIZE) {
                    updateStage(Stage.ChoiceTooShort);
                } else {
                    mChosenPattern = new ArrayList<LockPatternView.Cell>(
                            pattern);
                    updateStage(Stage.FirstChoiceValid);
                }
            } else {
                throw new IllegalStateException("Unexpected stage " + mUiStage
                        + " when " + "entering the pattern.");
            }
        }

        public void onPatternCellAdded(List<Cell> pattern) {

        }

        private void patternInProgress() {
            mHeaderText.setText(R.string.lockpattern_recording_inprogress);
            mFooterText.setText("");
            mFooterLeftButton.setEnabled(false);
            mFooterRightButton.setEnabled(false);
        }
    };

    /**
     * The states of the left footer button.
     */
    enum LeftButtonMode {
        Cancel(R.string.lockpattern_cancel, true),
        CancelDisabled(R.string.lockpattern_cancel, false),
        Retry(R.string.lockpattern_retry_button_text, true),
        RetryDisabled(R.string.lockpattern_retry_button_text, false),
        Gone(ID_EMPTY_MESSAGE, false);

        /**
         * @param text
         *            The displayed text for this mode.
         * @param enabled
         *            Whether the button should be enabled.
         */
        LeftButtonMode(int text, boolean enabled) {
            this.text = text;
            this.enabled = enabled;
        }

        final int text;
        final boolean enabled;
    }

    /**
     * The states of the right button.
     */
    enum RightButtonMode {
        Continue(R.string.lockpattern_continue_button_text, true), ContinueDisabled(
                R.string.lockpattern_continue_button_text, false), Confirm(
                R.string.lockpattern_confirm_button_text, true), ConfirmDisabled(
                R.string.lockpattern_confirm_button_text, false), Ok(
                android.R.string.ok, true);

        /**
         * @param text
         *            The displayed text for this mode.
         * @param enabled
         *            Whether the button should be enabled.
         */
        RightButtonMode(int text, boolean enabled) {
            this.text = text;
            this.enabled = enabled;
        }

        final int text;
        final boolean enabled;
    }

    /**
     * Keep track internally of where the user is in choosing a pattern.
     */
    protected enum Stage {

        Introduction(R.string.first_input_pattern_message,
                LeftButtonMode.Cancel, RightButtonMode.ContinueDisabled,
                ID_EMPTY_MESSAGE, true), HelpScreen(
                R.string.lockpattern_settings_help_how_to_record,
                LeftButtonMode.Gone, RightButtonMode.Ok, ID_EMPTY_MESSAGE,
                false), ChoiceTooShort(
                R.string.lockpattern_recording_incorrect_too_short,
                LeftButtonMode.Retry, RightButtonMode.ContinueDisabled,
                ID_EMPTY_MESSAGE, true), FirstChoiceValid(
                R.string.lockpattern_pattern_entered_header,
                LeftButtonMode.Retry, RightButtonMode.Continue,
                ID_EMPTY_MESSAGE, false), NeedToConfirm(
                R.string.double_check_pattern_message, LeftButtonMode.Cancel,
                RightButtonMode.ConfirmDisabled, ID_EMPTY_MESSAGE, true), ConfirmWrong(
                R.string.lockpattern_need_to_unlock_wrong,
                LeftButtonMode.Cancel, RightButtonMode.ConfirmDisabled,
                ID_EMPTY_MESSAGE, true), ChoiceConfirmed(
                R.string.lockpattern_pattern_confirmed_header,
                LeftButtonMode.Cancel, RightButtonMode.Confirm,
                ID_EMPTY_MESSAGE, false);

        /**
         * @param headerMessage
         *            The message displayed at the top.
         * @param leftMode
         *            The mode of the left button.
         * @param rightMode
         *            The mode of the right button.
         * @param footerMessage
         *            The footer message.
         * @param patternEnabled
         *            Whether the pattern widget is enabled.
         */
        Stage(int headerMessage, LeftButtonMode leftMode,
                RightButtonMode rightMode, int footerMessage,
                boolean patternEnabled) {
            this.headerMessage = headerMessage;
            this.leftMode = leftMode;
            this.rightMode = rightMode;
            this.footerMessage = footerMessage;
            this.patternEnabled = patternEnabled;
        }

        final int headerMessage;
        final LeftButtonMode leftMode;
        final RightButtonMode rightMode;
        final int footerMessage;
        final boolean patternEnabled;
    }

    private Stage mUiStage = Stage.Introduction;

    private Runnable mClearPatternRunnable = new Runnable() {
        public void run() {
            mLockPatternView.clearPattern();
        }
    };

    // private ChooseLockSettingsHelper mChooseLockSettingsHelper;

    private static final String KEY_UI_STAGE = "uiStage";
    private static final String KEY_PATTERN_CHOICE = "chosenPattern";
    private static final String TAG = "ChooseLockPatternFragment";

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Bundle bundle = getArguments();
        if (bundle != null) {
            mRequestType = bundle.getInt("TYPE");
            Log.d(TAG, "request type: " + mRequestType);
        }
        /*
         * mChooseLockSettingsHelper = new
         * ChooseLockSettingsHelper(getActivity()); if (!(getActivity()
         * instanceof ChooseLockPattern)) { throw new
         * SecurityException("Fragment contained in wrong activity"); }
         */
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.choose_lock_pattern, container, false);
        return view;
    }

    @Override
    public void onViewCreated(View view, Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        Log.d("EYE", "onViewCreated");
        mHeaderText = (TextView) view.findViewById(R.id.headerText);
        mLockPatternView = (LockPatternView) view
                .findViewById(R.id.lockPattern);
        mLockPatternView.setOnPatternListener(mChooseNewLockPatternListener);
        /*
         * mLockPatternView.setTactileFeedbackEnabled(mChooseLockSettingsHelper
         * .utils().isTactileFeedbackEnabled());
         */

        mFooterText = (TextView) view.findViewById(R.id.footerText);

        mFooterLeftButton = (TextView) view.findViewById(R.id.footerLeftButton);
        mFooterRightButton = (TextView) view
                .findViewById(R.id.footerRightButton);

        mFooterLeftButton.setOnClickListener(this);
        mFooterRightButton.setOnClickListener(this);

        // make it so unhandled touch events within the unlock screen go to the
        // lock pattern view.
        final LinearLayoutWithDefaultTouchRecepient topLayout = (LinearLayoutWithDefaultTouchRecepient) view
                .findViewById(R.id.topLayout);
        topLayout.setDefaultTouchRecepient(mLockPatternView);

        final boolean confirmCredentials = getActivity().getIntent()
                .getBooleanExtra("confirm_credentials", true);

        if (savedInstanceState == null && mCurrentState == null) {
            updateStage(Stage.Introduction);
            /*
             * if (confirmCredentials) { // first launch. As a security measure,
             * we're in NeedToConfirm // mode until we // know there isn't an
             * existing password or the user confirms // their password.
             * updateStage(Stage.NeedToConfirm); boolean
             * launchedConfirmationActivity = mChooseLockSettingsHelper
             * .launchConfirmationActivity(CONFIRM_EXISTING_REQUEST, null,
             * null); if (!launchedConfirmationActivity) {
             * updateStage(Stage.Introduction); } } else {
             * updateStage(Stage.Introduction); }
             */
        } else if (savedInstanceState != null) {
            // restore from previous state
            final String patternString = savedInstanceState
                    .getString(KEY_PATTERN_CHOICE);
            if (patternString != null) {
                mChosenPattern = LockPatternUtils
                        .stringToPattern(patternString);
            }
            updateStage(Stage.values()[savedInstanceState.getInt(KEY_UI_STAGE)]);
        } else if (mCurrentState != null) {
            final String patternString = mCurrentState
                    .getString(KEY_PATTERN_CHOICE);
            if (patternString != null) {
                mChosenPattern = LockPatternUtils
                        .stringToPattern(patternString);
            }
            updateStage(Stage.values()[mCurrentState.getInt(KEY_UI_STAGE)]);
        }
    }

    public void onClick(View v) {
        if (v == mFooterLeftButton) {
            if (mUiStage.leftMode == LeftButtonMode.Retry) {
                mChosenPattern = null;
                mLockPatternView.clearPattern();
                updateStage(Stage.Introduction);
            } else if (mUiStage.leftMode == LeftButtonMode.Cancel) {
                // cancel set password.
                FragmentManager mgr = getFragmentManager();
                FragmentTransaction transaction = mgr.beginTransaction();
                transaction.remove(this);
                transaction.commitAllowingStateLoss();
                Bundle bundle = getArguments();
                if (bundle != null && bundle.getInt("TYPE") == TYPE_SET_PATTERN) {
                    getActivity().setResult(RESULT_FINISHED);
                    getActivity().finish();
                } else if (bundle == null) {
/*                    getActivity().setResult(RESULT_FINISHED);
                    getActivity().finish();*/
                } else {
                    if (mListener != null) {
                        mListener.onCancel();
                    }
                }

            } else {
                throw new IllegalStateException(
                        "left footer button pressed, but stage of " + mUiStage
                                + " doesn't make sense");
            }
        } else if (v == mFooterRightButton) {

            if (mUiStage.rightMode == RightButtonMode.Continue) {
                if (mUiStage != Stage.FirstChoiceValid) {
                    throw new IllegalStateException("expected ui stage "
                            + Stage.FirstChoiceValid + " when button is "
                            + RightButtonMode.Continue);
                }
                updateStage(Stage.NeedToConfirm);
            } else if (mUiStage.rightMode == RightButtonMode.Confirm) {
                if (mUiStage != Stage.ChoiceConfirmed) {
                    throw new IllegalStateException("expected ui stage "
                            + Stage.ChoiceConfirmed + " when button is "
                            + RightButtonMode.Confirm);
                }
                saveChosenPatternAndFinish();
            } else if (mUiStage.rightMode == RightButtonMode.Ok) {
                if (mUiStage != Stage.HelpScreen) {
                    throw new IllegalStateException(
                            "Help screen is only mode with ok button, but "
                                    + "stage is " + mUiStage);
                }
                mLockPatternView.clearPattern();
                mLockPatternView.setDisplayMode(DisplayMode.Correct);
                updateStage(Stage.Introduction);
            }
        }
    }

    public boolean onKeyDown(int keyCode, KeyEvent event) {
        if (keyCode == KeyEvent.KEYCODE_BACK && event.getRepeatCount() == 0) {
            if (mUiStage == Stage.HelpScreen) {
                updateStage(Stage.Introduction);
                return true;
            }
        }
        if (keyCode == KeyEvent.KEYCODE_MENU && mUiStage == Stage.Introduction) {
            updateStage(Stage.HelpScreen);
            return true;
        }
        return false;
    }

    public void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putInt(KEY_UI_STAGE, mUiStage.ordinal());
        if (mChosenPattern != null) {
            outState.putString(KEY_PATTERN_CHOICE,
                    LockPatternUtils.patternToString(mChosenPattern));
        }
    }

    /**
     * Updates the messages and buttons appropriate to what stage the user is at
     * in choosing a view. This doesn't handle clearing out the pattern; the
     * pattern is expected to be in the right state.
     *
     * @param stage
     */
    protected void updateStage(Stage stage) {
        final Stage previousStage = mUiStage;

        mUiStage = stage;

        // header text, footer text, visibility and
        // enabled state all known from the stage
        if (stage == Stage.ChoiceTooShort) {
            String text = getString(R.string.lockpattern_recording_incorrect_too_short);
            mHeaderText.setText(text);
        } else {
            mHeaderText.setText(stage.headerMessage);
        }
        if (stage.footerMessage == ID_EMPTY_MESSAGE) {
            mFooterText.setText("");
        } else {
            mFooterText.setText(stage.footerMessage);
        }

        if (stage.leftMode == LeftButtonMode.Gone) {
            mFooterLeftButton.setVisibility(View.GONE);
        } else {
            mFooterLeftButton.setVisibility(View.VISIBLE);
            mFooterLeftButton.setText(stage.leftMode.text);
            mFooterLeftButton.setEnabled(stage.leftMode.enabled);
        }

        Log.d("LockPatternActivity", "right button text: "
                + getString(stage.rightMode.text) + " mode: "
                + stage.rightMode.enabled);
        // mFooterRightButton.setText(stage.rightMode.text);
        mFooterRightButton.setText(getString(stage.rightMode.text));
        mFooterRightButton.setEnabled(stage.rightMode.enabled);

        // same for whether the patten is enabled
        if (stage.patternEnabled) {
            mLockPatternView.enableInput();
        } else {
            mLockPatternView.disableInput();
        }

        // the rest of the stuff varies enough that it is easier just to handle
        // on a case by case basis.
        mLockPatternView.setDisplayMode(DisplayMode.Correct);

        switch (mUiStage) {
        case Introduction:
            mLockPatternView.clearPattern();
            break;
        case HelpScreen:
            mLockPatternView.setPattern(DisplayMode.Animate, mAnimatePattern);
            break;
        case ChoiceTooShort:
            mLockPatternView.setDisplayMode(DisplayMode.Wrong);
            postClearPatternRunnable();
            break;
        case FirstChoiceValid:
            mLockPatternView.setPattern(DisplayMode.Correct, mChosenPattern);
            break;
        case NeedToConfirm:
            mLockPatternView.clearPattern();
            break;
        case ConfirmWrong:
            mLockPatternView.setDisplayMode(DisplayMode.Wrong);
            postClearPatternRunnable();
            break;
        case ChoiceConfirmed:
            mLockPatternView.setPattern(DisplayMode.Correct, mChosenPattern);
            break;
        }

        // If the stage changed, announce the header for accessibility. This
        // is a no-op when accessibility is disabled.
        if (previousStage != stage) {
            mHeaderText.announceForAccessibility(mHeaderText.getText());
        }
    }

    // clear the wrong pattern unless they have started a new one
    // already
    private void postClearPatternRunnable() {
        mLockPatternView.removeCallbacks(mClearPatternRunnable);
        mLockPatternView.postDelayed(mClearPatternRunnable,
                WRONG_PATTERN_CLEAR_TIMEOUT_MS);
    }

    private void saveChosenPatternAndFinish() {
        LockPatternUtils utils = new LockPatternUtils(getActivity());
        DataProtectionLockPatternUtils localPatternUtils = new DataProtectionLockPatternUtils(getActivity());
        localPatternUtils.saveLockPattern(null);
        localPatternUtils.saveLockPattern(utils.patternToString(mChosenPattern));

        if (null != mListener) {
            FragmentTransaction transaction = getFragmentManager()
                    .beginTransaction();
            transaction.remove(this);
            transaction.commitAllowingStateLoss();
            if (mRequestType == TYPE_SET_PATTERN) {
                mListener.onPatternSetSuccess(utils
                        .patternToString(mChosenPattern));

                DrmManagerClient client = new DrmManagerClient(getActivity());
                CtaDrmUtils.setKey(client, utils.patternToString(mChosenPattern).getBytes());

                client.release();
                Log.d("LockPatternActivity", "pattern size: "
                        + utils.patternToString(mChosenPattern).getBytes().length);
            } else if (mRequestType == TYPE_CHANGE_PATTERN) {
                String data = null;
                Bundle bundle = getArguments();
                if (bundle != null) {
                    data = bundle.getString("DATA");
                }
                if (data != null) {
                    mListener.onPatternChangeSuccess(data,
                            utils.patternToString(mChosenPattern));
                } else {
                    Log.d(TAG, " save pattern... data is null");
                }
            }
        }
        /*
         * if (commandKey != null) {
         * Settings.System.putString(getActivity().getContentResolver(),
         * commandKey, commandValue); }
         */
        // M @}
        // utils.setLockPatternEnabled(true);

        /*
         * if (lockVirgin) { utils.setVisiblePatternEnabled(true); }
         */

        /*
         * Intent intent = getActivity().getIntent();
         * intent.putExtra(LockPatternActivity.KEY_INVOKE, true);
         * getActivity().setResult(Activity.RESULT_OK, intent);
         * getActivity().finish();
         */
    }
}
