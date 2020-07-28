package com.mediatek.email.outofoffice;

import java.util.Calendar;

import com.android.email.R;
import com.android.email.service.EmailServiceUtils;
import com.android.mail.utils.LogUtils;
import com.android.emailcommon.mail.ServerCommandInfo;
import com.android.emailcommon.provider.Account;
import com.android.emailcommon.service.EmailServiceProxy;
import com.android.emailcommon.service.OofParams;
import com.android.emailcommon.utility.EmailAsyncTask;
import com.android.emailcommon.utility.Utility;
import com.mediatek.mail.ui.utils.UiUtilities;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.DatePickerDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.ProgressDialog;
import android.app.TimePickerDialog;
import android.app.DatePickerDialog.OnDateSetListener;
import android.app.TimePickerDialog.OnTimeSetListener;
import android.content.Context;
import android.content.DialogInterface;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Parcelable;
import android.os.RemoteException;
import android.text.Editable;
import android.text.TextWatcher;
import android.text.format.DateUtils;
import android.text.format.Time;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.DatePicker;
import android.widget.TextView;
import android.widget.TimePicker;
import android.widget.Toast;

public class AccountSettingsOutOfOfficeFragment extends Fragment {
    private long mAccountId;
    private Context mContext;
    private Callback mCallback;
    private OofAsyncTask mOofAsyncTask;
    private WaitingFetchOofDialog mDialog;
    private OofParams mOofParams;
    private boolean mNeedToResume;
    private boolean mPaused;

    private TextView mTitleSumm;
    private TextView mFrom;
    private TextView mTo;
    private TextView mAutoReply;
    private TextView mReplySumm;
    private TextView mToServer;
    //view of the layout
    private View mTitleView;
    private View mReplyView;
    private View mExternalView;

    private Button mFromDateButton;
    private Button mFromTimeButton;
    private Button mToDateButton;
    private Button mToTimeButton;
    private Button mCancelButton;
    private Button mSaveButton;

    private CheckBox mOutOffice;
    private CheckBox mSeverCheck;

    //params for from time setting
    private int mFromYear;
    private int mFromDay;
    private int mFromWeekday;
    private int mFromMonth;
    private int mFromDayTime;
    private int mFromHour;
    private int mFromMinute;
    //params for to time setting
    private int mToYear;
    private int mToDay;
    private int mToWeekday;
    private int mToMonth;
    private int mToDayTime;
    private int mToHour;
    private int mToMinute;
    //params for time check
    private int mCurrentYear;
    private int mCurrentDay;
    private int mCurrentWeekday;
    private int mCurrentMonth;
    private int mCurrentHour;
    private int mCurrentMinute;
    private int mCurrentDayTime;
    //Constants for date or time pick dialog
    private static final int PICK_FROM_DATE = 0;
    private static final int PICK_TO_DATE = 1;
    private static final int PICK_FROM_TIME = 2;
    private static final int PICK_TO_TIME = 3;

    private static final String BUNDLE_KEY_ACTIVITY_TITLE
            = "AccountSettingsOutOfOfficeFragment.title";
    private static final String BUNDLE_KEY_NEED_RESUME
            = "ResumeSave";
    private static final String OOF_PARAMS = "Oof_params";
    private static final String BUNDLE_KEY_SAVE = "save";
    private static final String ACCOUNT_ID = "account_id";
    private static final String OOF_REPLY = "oof_reply";
    private static final String OOF_TO_EXTERNAL = "oof_external";
    private static final String OOF_KEY_FROM = "from";
    private static final String OOF_KEY_TO = "to";

    private static final String TO_YEAR = "to_year";
    private static final String TO_MONTH = "to_month";
    private static final String TO_DAY = "to_day";
    private static final String TO_HOUR = "to_hour";
    private static final String TO_MINUTE = "to_minute";
    private static final String TO_DAY_TIME = "to_daytime";

    /// M : TAG.
    private static final String TAG = "AccountSettingsOutOfOfficeFragment";
    public static final String CANCEL_ALERT_TAG = "CancelAlert";

    public interface Callback {
        /**
         * Called when the setting finished.
         */
        void onSettingFinished();
    }

    public String onStarted() {
        SharedPreferences pre = getActivity().getSharedPreferences(OOF_REPLY,
                android.content.Context.MODE_PRIVATE);
        return pre.getString(OOF_REPLY + mAccountId, "");
    }

    public static AccountSettingsOutOfOfficeFragment newInstance(Long accountID, Parcelable oofParams) {
        AccountSettingsOutOfOfficeFragment f = new AccountSettingsOutOfOfficeFragment();
        Bundle bundle = new Bundle();
        bundle.putLong(ACCOUNT_ID, accountID);
        bundle.putParcelable(OOF_PARAMS, oofParams);
        f.setArguments(bundle);
        return f;
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Bundle args = getArguments();
        mAccountId = args.getLong("account_id");
        Account account = Account.restoreAccountWithId(getActivity(), mAccountId);
        if (account == null) {
            getActivity().finish();
        }

        if (savedInstanceState != null) {
            mOofParams = savedInstanceState.getParcelable(OOF_PARAMS);
            mToYear = savedInstanceState.getInt(TO_YEAR);
            mToMonth = savedInstanceState.getInt(TO_MONTH);
            mToDay = savedInstanceState.getInt(TO_DAY);
            mToHour = savedInstanceState.getInt(TO_HOUR);
            mToMinute = savedInstanceState.getInt(TO_MINUTE);
            mToDayTime = savedInstanceState.getInt(TO_DAY_TIME);
        } else {
            mOofParams = args.getParcelable(OOF_PARAMS);
        }
        mCallback = (Callback) getActivity();
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        LogUtils.d(TAG, "AccountSettingsOutOfOfficeFragment onCreateView");
        int layoutId = R.layout.account_settings_out_of_office_fragment;
        View view = inflater.inflate(layoutId, container, false);
        mContext = getActivity();
        loadSettings(view, savedInstanceState);
        if (savedInstanceState != null) {
            mSaveButton = (Button) view.findViewById(R.id.save);
            mSaveButton.setEnabled(savedInstanceState.getBoolean(BUNDLE_KEY_SAVE));
        }
        return view;
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        // StartPreferencePanel launches this fragment with the right title initially, but
        // if the device is rotate we must set the title ourselves
        if (savedInstanceState != null) {
            getActivity().setTitle(savedInstanceState.getString(BUNDLE_KEY_ACTIVITY_TITLE));
            mNeedToResume = savedInstanceState.getBoolean(BUNDLE_KEY_NEED_RESUME, false);
        }
        // Set the title of the activity which contains this fragment dynamically, to make the
        // title change according to the language and locale setting change, as this fragment is
        // created by setPreferencePanel(), the title will not change as the locale changes by use
        // of setBreadCrumbTitle()
        this.getActivity().setTitle(R.string.account_settings_oof_label);
    }

    @Override
    public void onPause() {
        mPaused = true;
        if (mDialog != null) {
            mDialog.dismissAllowingStateLoss();
        }
        if (mOofAsyncTask != null) {
            mNeedToResume = true;
            onCheckingDialogCancel();
        }
        super.onPause();
    }

    @Override
    public void onResume() {
        super.onResume();
        mPaused = false;
        if (mNeedToResume) {
            mNeedToResume = false;
            mOofAsyncTask = (OofAsyncTask) new OofAsyncTask().executeParallel();
            mDialog = WaitingFetchOofDialog.newInstance(
                    AccountSettingsOutOfOfficeFragment.this);
            mDialog.show(getFragmentManager(), WaitingFetchOofDialog.TAG);
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    private void onCheckingDialogCancel() {
        // 1. kill the checker
        Utility.cancelTaskInterrupt(mOofAsyncTask);
        mOofAsyncTask = null;
   }

    @Override
    public void onSaveInstanceState(Bundle outState) {
        outState.putString(BUNDLE_KEY_ACTIVITY_TITLE, (String) getActivity().getTitle());
        outState.putBoolean(BUNDLE_KEY_SAVE, mSaveButton.isEnabled());
        outState.putBoolean(BUNDLE_KEY_NEED_RESUME, mNeedToResume);
        mOofParams.setOofState(mOutOffice.isChecked() ? 1 : 0);
        mOofParams.setIsExternal(mSeverCheck.isChecked() ? 1 : 0);
        Calendar calendar = Calendar.getInstance();
        calendar.set(mFromYear, mFromMonth, mFromDay,
                mFromDayTime == 0 ? mFromHour : ((mFromHour == 12
                        ? 0 : mFromHour) + 12), mFromMinute);
        mOofParams.setStartTimeInMillis(calendar.getTimeInMillis());
        calendar.set(mToYear, mToMonth, mToDay,
                mToDayTime == 0 ? mToHour : ((mToHour == 12 ? 0 : mToHour) + 12), mToMinute);
        mOofParams.setEndTimeInMillis(calendar.getTimeInMillis());
        mOofParams.setReplyMessage(onStarted());
        outState.putParcelable(OOF_PARAMS, mOofParams);
        outState.putInt(TO_YEAR, mToYear);
        outState.putInt(TO_MONTH, mToMonth);
        outState.putInt(TO_DAY, mToDay);
        outState.putInt(TO_HOUR, mToHour);
        outState.putInt(TO_MINUTE, mToMinute);
        outState.putInt(TO_DAY_TIME, mToDayTime);
        super.onSaveInstanceState(outState);
    }

    public void loadSettings(View view, Bundle bundle) {
        mTitleSumm = (TextView) view.findViewById(R.id.summ);
        mTitleView = view.findViewById(R.id.title);
        mReplyView = view.findViewById(R.id.reply);
        mExternalView = view.findViewById(R.id.to_external);
        mFrom = (TextView) view.findViewById(R.id.from_label);
        mTo = (TextView) view.findViewById(R.id.to_label);
        mAutoReply = (TextView) view.findViewById(R.id.auto_reply);
        mReplySumm = (TextView) view.findViewById(R.id.reply_summ);
        mToServer = (TextView) view.findViewById(R.id.to_server);
        mFromDateButton = (Button) view.findViewById(R.id.start_date);
        mFromTimeButton = (Button) view.findViewById(R.id.start_time);
        mToDateButton = (Button) view.findViewById(R.id.end_date);
        mToTimeButton = (Button) view.findViewById(R.id.end_time);
        mCancelButton = (Button) view.findViewById(R.id.cancel);
        mSaveButton = (Button) view.findViewById(R.id.save);

        mTitleView.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mOutOffice.isChecked()) {
                    mOutOffice.setChecked(false);
                    disableOutOfficeOption();
                    return;
                }
                mOutOffice.setChecked(true);
                enableOutOfficeOption();
            }
        });

        mExternalView.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mSeverCheck.isChecked()) {
                    mSeverCheck.setChecked(false);
                    return;
                }
                mSeverCheck.setChecked(true);
            }
        });

        mFromDateButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                DateTimePickerDialogFragment
                        .showDialog(AccountSettingsOutOfOfficeFragment.this,
                                PICK_FROM_DATE);
            }
        });

        mToDateButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                DateTimePickerDialogFragment.showDialog(
                        AccountSettingsOutOfOfficeFragment.this, PICK_TO_DATE);
            }
        });

        mFromTimeButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                DateTimePickerDialogFragment
                        .showDialog(AccountSettingsOutOfOfficeFragment.this,
                                PICK_FROM_TIME);
            }
        });

        mToTimeButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                DateTimePickerDialogFragment.showDialog(
                        AccountSettingsOutOfOfficeFragment.this, PICK_TO_TIME);
            }
        });

        mReplyView.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                mSaveButton.setEnabled(true);
                FragmentManager fm = getFragmentManager();
                AutoReplyDialogFragment fragment = (AutoReplyDialogFragment) fm
                        .findFragmentByTag(AutoReplyDialogFragment.TAG);
                if (fragment == null) {
                    DialogFragment newFragment = AutoReplyDialogFragment
                    .newInstance(mAccountId);
                    newFragment.show(getFragmentManager(), AutoReplyDialogFragment.TAG);
                }
            }
        });
        mOutOffice = (CheckBox) view.findViewById(R.id.check1);
        mSeverCheck = (CheckBox) view.findViewById(R.id.check2);
        setDisplay(bundle);
        addChangeListener();

        mSaveButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (timeCheck()) {
                    return;
                }
                SharedPreferences.Editor editor = getActivity().getSharedPreferences(OOF_REPLY,
                        android.content.Context.MODE_PRIVATE).edit();
                editor.putBoolean(OOF_TO_EXTERNAL + mAccountId, mSeverCheck.isChecked());
                editor.commit();
                if (mOofAsyncTask == null) {
                    mOofAsyncTask = (OofAsyncTask) new OofAsyncTask().executeParallel();
                    mDialog = WaitingFetchOofDialog.newInstance(
                            AccountSettingsOutOfOfficeFragment.this);
                    mDialog.show(getFragmentManager(), WaitingFetchOofDialog.TAG);
                }
            }
        });

        mSaveButton.setEnabled(false);
        mCancelButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (mSaveButton.isEnabled()) {
                    FragmentManager fm = getFragmentManager();
                    AlertDialogFragment adf = (AlertDialogFragment) fm
                            .findFragmentByTag(CANCEL_ALERT_TAG);
                    if (adf == null) {
                        adf = AlertDialogFragment.newInstance(
                                R.string.account_settings_oof_cancel,
                                R.string.account_settings_oof_cancel_summary, mCallback);
                        adf.show(getFragmentManager(), CANCEL_ALERT_TAG);
                    }
                } else {
                    mCallback.onSettingFinished();
                }
            }
        });
    }

    /**
     * Get SaveButton Status.
     */
    public boolean getSaveButtonStatus() {
        return mSaveButton.isEnabled();
    }

    /**
     * Set SaveButton Status.
     */
    public void setSaveButtonStatus(boolean enable) {
        mSaveButton.setEnabled(enable);
    }

    /**
     * Check the To time is later than the current time or not.
     */
    protected boolean timeCheck() {
        if (mOutOffice.isChecked()) {
            getCurrentTime();
            if (getSumMinute(mCurrentYear, mCurrentMonth, mCurrentDay,
                    mCurrentHour, mCurrentMinute, mCurrentDayTime)
                    > getSumMinute(mToYear, mToMonth, mToDay, mToHour,
                            mToMinute, mToDayTime)) {
                AlertDialogFragment adf = AlertDialogFragment.newInstance(
                        R.string.account_settings_oof_save_fail,
                        R.string.account_settings_oof_save_fail_summary);
                adf.show(getFragmentManager(), "AlertDialogFragment");
                return true;
            }
        }
        return false;
    }

    /**
     * Get the minutes by the given time params.
     * @param year
     * @param month
     * @param day
     * @param hour
     * @param minute
     * @param dayTime
     * @return the sum of minutes
     */
    private long getSumMinute(int year, int month, int day, int hour, int minute, int dayTime) {
        int hourOf24;
        long sumOfDay = getDayNums(year, month, day);
        if (hour != 12) {
            hourOf24 = (dayTime == 0 ? hour : hour + 12);
        } else {
            hourOf24 = hour;
        }
        return getMinuteNums(sumOfDay, hourOf24, minute);
    }

    private void reportProgress(int code) {
        AlertDialogFragment ad;
        mOofAsyncTask = null;
        if (mDialog != null) {
            mDialog.dismissAllowingStateLoss();
        }
        if (code != ServerCommandInfo.OofInfo.SET_OR_SAVE_SUCCESS) {
            if (code == ServerCommandInfo.OofInfo.NETWORK_SHUT_DOWN) {
                if (UiUtilities.isWifiOnly(getActivity())) { //For wifi only case,ALPS00458924.
                    ad = AlertDialogFragment.newInstance(
                            R.string.unable_to_connect,
                            R.string.need_wifi_connection_prompt);
                } else {
                     ad = AlertDialogFragment.newInstance(
                            R.string.unable_to_connect,
                            R.string.need_connection_prompt);
                }
                ad.show(getFragmentManager(), "AlertDialogFragment");
                return;
            }
            AlertDialogFragment adf = AlertDialogFragment.newInstance(
                    R.string.account_settings_oof_save_fail,
                    R.string.account_settings_oof_save_fail_summary);
            adf.show(getFragmentManager(), "AlertDialogFragment");
            return;
        }
        mSaveButton.setEnabled(false);
        mCallback.onSettingFinished();
    }

    public class OofAsyncTask extends EmailAsyncTask<Void, Void, Integer> {

        public OofAsyncTask() {
            super(null);
        }

        @Override
        protected Integer doInBackground(Void... params) {
            Time fromTime = new Time();
            fromTime.set(0, mFromMinute, mFromDayTime == 0 ? mFromHour
                    : ((mFromHour == 12 ? 0 : mFromHour) + 12), mFromDay,
                    mFromMonth, mFromYear);
            fromTime.normalize(true);
            fromTime.switchTimezone("GMT");

            Time toTime = new Time();
            toTime.set(0, mToMinute, mToDayTime == 0 ? mToHour
                    : ((mToHour == 12 ? 0 : mToHour) + 12), mToDay, mToMonth,
                    mToYear);
            toTime.normalize(true);
            toTime.switchTimezone("GMT");

            OofParams oofParams = new OofParams(0, mOutOffice.isChecked() ? 1 : 0,
                    fromTime.toMillis(false), toTime.toMillis(false), mSeverCheck.isChecked() ? 1
                            : 0, onStarted());
            final EmailServiceProxy proxy = EmailServiceUtils
                    .getServiceForAccount(mContext, mAccountId);
            try {
                oofParams = proxy.syncOof(mAccountId, oofParams, false);
            } catch (RemoteException e) {
                LogUtils.d(TAG, "OofAsyncTask syncOof is Error.");
            }
            if (oofParams == null) {
                LogUtils.d(TAG, "OofAsyncTask oofParams is null.");
                return ServerCommandInfo.OofInfo.SERVER_NOT_SUPPORT_OOF;
            }

            return Integer.valueOf(oofParams.getmStatus());
        }

        @Override
        protected void onSuccess(Integer result) {
            if (!mPaused) {
                reportProgress(result);
            }
        }
    }

    public static class AlertDialogFragment extends DialogFragment {
        public static final String TAG = "AlertDialogFragment";

        private static final String TITLE_ID = "titleId";
        private static final String MSG_ID = "messageId";
        private static final String IS_ALRET = "mIsSaveAlert";
        private int mTitleId;
        private int mMessageId;
        private boolean mIsSaveAlert;
        private Callback mCallback;

        /**
         * This public constructor is required so that DialogFragment
         * state can be automatically restored by the framework.
         */
        public AlertDialogFragment() {
        }

        public AlertDialogFragment(int titleId, int messageId) {
            super();
            this.mTitleId = titleId;
            this.mMessageId = messageId;
            this.mIsSaveAlert = false;
        }

        public AlertDialogFragment(int titleId, int messageId, Callback callback) {
            super();
            this.mTitleId = titleId;
            this.mMessageId = messageId;
            this.mIsSaveAlert = true;
            this.mCallback = callback;
        }

        public static AlertDialogFragment newInstance(int titleId, int messageId) {
            final AlertDialogFragment dialog = new AlertDialogFragment(titleId, messageId);
            return dialog;
        }

        public static AlertDialogFragment newInstance(int titleId,
                int messageId, Callback callback) {
            final AlertDialogFragment dialog = new AlertDialogFragment(
                    titleId, messageId, callback);
            return dialog;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final Context context = getActivity();
            final AlertDialog.Builder dialogBuilder = new AlertDialog.Builder(context);
            if (savedInstanceState != null) {
                mTitleId = savedInstanceState.getInt(TITLE_ID);
                mMessageId = savedInstanceState.getInt(MSG_ID);
                mIsSaveAlert = savedInstanceState.getBoolean(IS_ALRET);
                mCallback = (Callback) context;
            }
            if (mIsSaveAlert) {
                dialogBuilder.setTitle(mTitleId)
                .setIconAttribute(android.R.attr.alertDialogIcon)
                .setMessage(mMessageId)
                .setPositiveButton(android.R.string.ok,
                        new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        AccountSettingsOutOfOfficeFragment fragment = (AccountSettingsOutOfOfficeFragment)
                                getFragmentManager().findFragmentByTag(AccountSettingsOutOfOfficeFragment.TAG);
                        fragment.setSaveButtonStatus(false);
                        mCallback.onSettingFinished();
                    }
                })
                .setNegativeButton(android.R.string.cancel,
                        new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                    }
                });
            } else {
                dialogBuilder.setTitle(mTitleId).setIconAttribute(
                        android.R.attr.alertDialogIcon).setMessage(mMessageId)
                        .setPositiveButton(android.R.string.ok,
                                new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog,
                                    int which) {
                            }
                        });
            }
            AlertDialog alertDialog = dialogBuilder.create();
            alertDialog.setCanceledOnTouchOutside(false);
            return alertDialog;
        }

        @Override
        public void onSaveInstanceState(Bundle outState) {
            outState.putInt(TITLE_ID, mTitleId);
            outState.putInt(MSG_ID, mMessageId);
            outState.putBoolean(IS_ALRET, mIsSaveAlert);
            super.onSaveInstanceState(outState);
        }
    }

    public static class WaitingFetchOofDialog extends DialogFragment {
        public static final String TAG = "WaitingFetchOofDialog";

        public static WaitingFetchOofDialog newInstance(
                AccountSettingsOutOfOfficeFragment fragment) {
            final WaitingFetchOofDialog dialog = new WaitingFetchOofDialog();
            dialog.setTargetFragment(fragment, 0);
            return dialog;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            final Context context = getActivity();
            final ProgressDialog progressDialog = new ProgressDialog(context);
            progressDialog.setIndeterminate(true);
            progressDialog.setCanceledOnTouchOutside(false);
            progressDialog.setMessage(context
                    .getString(R.string.account_settings_oof_save_sets)
                    + "...");
            return progressDialog;
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            AccountSettingsOutOfOfficeFragment fragment =
                    (AccountSettingsOutOfOfficeFragment) getTargetFragment();
            fragment.onCheckingDialogCancel();
            super.onCancel(dialog);
        }

    }

    public void getCurrentTime() {
        Calendar c = Calendar.getInstance();
        mCurrentYear = c.get(Calendar.YEAR);
        mCurrentMonth = c.get(Calendar.MONTH);
        mCurrentDay = c.get(Calendar.DAY_OF_MONTH);
        mCurrentWeekday = c.get(Calendar.DAY_OF_WEEK);
        mCurrentHour = c.get(Calendar.HOUR);
        mCurrentMinute = c.get(Calendar.MINUTE);
        mCurrentDayTime = c.get(Calendar.AM_PM);
    }

    public void setDisplay(Bundle bundle) {
        boolean isOpen = mOofParams.getOofState() != 0;
        initTimeDisplay(isOpen, bundle);
        mOutOffice.setChecked(isOpen);
        if (isOpen) {
            mSeverCheck.setChecked(mOofParams.getIsExternal() != 0);
            String s = mOofParams.getReplyMessage();
            if (null != s && !s.isEmpty()) {
                SharedPreferences.Editor editor = getActivity().getSharedPreferences(OOF_REPLY,
                        android.content.Context.MODE_PRIVATE).edit();
                editor.putString(OOF_REPLY + mAccountId, s);
                editor.commit();
            }
        } else {
            SharedPreferences pre = getActivity().getSharedPreferences(OOF_REPLY,
                    android.content.Context.MODE_PRIVATE);
            boolean isExternal = pre.getBoolean(OOF_TO_EXTERNAL + mAccountId, false);
            mSeverCheck.setChecked(isExternal);
        }
    }//next  save to protect the temp input and isExternal

    public void initTimeDisplay(boolean isOpen, Bundle bundle) {
        if (isOpen || bundle != null) {
            if (isOpen) {
                enableOutOfficeOption();
            } else {
                disableOutOfficeOption();
            }
            long time = mOofParams.getStartTimeInMillis();
            Calendar calendar = Calendar.getInstance();
            calendar.setTimeInMillis(time);
            setTimeByCalendar(calendar, OOF_KEY_FROM);
            time = mOofParams.getEndTimeInMillis();
            calendar.setTimeInMillis(time);
            setTimeByCalendar(calendar, OOF_KEY_TO);
        } else {
            init();
        }
    }

    public void setTimeByCalendar(Calendar calendar, String tag) {
        String date;
        String time;
        if (tag.equalsIgnoreCase(OOF_KEY_FROM)) {
            mFromYear = calendar.get(Calendar.YEAR);
            mFromMonth = calendar.get(Calendar.MONTH);
            mFromDay = calendar.get(Calendar.DAY_OF_MONTH);
            mFromWeekday = calendar.get(Calendar.DAY_OF_WEEK);
            mFromHour = calendar.get(Calendar.HOUR);
            mFromMinute = calendar.get(Calendar.MINUTE);
            mFromDayTime = calendar.get(Calendar.AM_PM);
            date = DateUtils.getDayOfWeekString(mFromWeekday, 20) + ", "
                    + DateUtils.getMonthString(mFromMonth, 30)
                    + " " + mFromDay + ", " + mFromYear;
            time = (mFromHour == 0 ? 12 : mFromHour) + ":"
                    + (mFromMinute < 10 ? ("0" + mFromMinute) : mFromMinute)
                    + " " + DateUtils.getAMPMString(mFromDayTime);
            mFromDateButton.setText(date);
            mFromTimeButton.setText(time);
            return;
        }
        mToYear = calendar.get(Calendar.YEAR);
        mToMonth = calendar.get(Calendar.MONTH);
        mToDay = calendar.get(Calendar.DAY_OF_MONTH);
        mToWeekday = calendar.get(Calendar.DAY_OF_WEEK);
        mToHour = calendar.get(Calendar.HOUR);
        mToMinute = calendar.get(Calendar.MINUTE);
        mToDayTime = calendar.get(Calendar.AM_PM);
        date = DateUtils.getDayOfWeekString(mToWeekday, 20) + ", "
                + DateUtils.getMonthString(mToMonth, 30)
                + " " + mToDay + ", " + mToYear;
        time = (mToHour == 0 ? 12 : mToHour) + ":"
                + (mToMinute < 10 ? ("0" + mToMinute) : mToMinute)
                + " " + DateUtils.getAMPMString(mToDayTime);
        mToDateButton.setText(date);
        mToTimeButton.setText(time);
    }

    public void init() {
        getCurrentTime();
        String date = DateUtils.getDayOfWeekString(mCurrentWeekday, 20) + ", "
                + DateUtils.getMonthString(mCurrentMonth, 30)
                + " " + mCurrentDay + ", " + mCurrentYear;
        String time;
        time = (mCurrentHour == 0 ? 12 : mCurrentHour) + ":00" + " " + DateUtils.getAMPMString(mCurrentDayTime);
        mFromYear = mCurrentYear;
        mFromMonth = mCurrentMonth;
        mFromWeekday = mCurrentWeekday;
        mFromDay = mCurrentDay;
        mFromDayTime = mCurrentDayTime;
        mFromHour = mCurrentHour;
        mFromMinute = 0;
        Calendar c = Calendar.getInstance();
        c.set(mCurrentYear, mCurrentMonth, mCurrentDay);
        long timeMillis = c.getTimeInMillis();
        timeMillis += 86400000;
        c.setTimeInMillis(timeMillis);
        mToYear = c.get(Calendar.YEAR);
        mToMonth = c.get(Calendar.MONTH);
        mToDay = c.get(Calendar.DAY_OF_MONTH);
        mToWeekday = c.get(Calendar.DAY_OF_WEEK);
        mToHour = mCurrentHour;
        mToMinute = 0;
        mToDayTime = mCurrentDayTime;

        mFromDateButton.setText(date);
        mToTimeButton.setText(time);
        date = DateUtils.getDayOfWeekString(mToWeekday, 20) + ", "
                + DateUtils.getMonthString(mToMonth, 30)
                + " " + mToDay + ", " + mToYear;
        mToDateButton.setText(date);
        mFromTimeButton.setText(time);
        if (!mOutOffice.isChecked()) {
            disableOutOfficeOption();
            return;
        }
        enableOutOfficeOption();
    }

    private class TextChangeWatcher implements TextWatcher {
        @Override
        public void afterTextChanged(Editable s) {
            mSaveButton.setEnabled(true);
        }

        @Override
        public void beforeTextChanged(CharSequence s, int start, int count,
                int after) {
        }

        @Override
        public void onTextChanged(CharSequence s, int start, int before,
                int count) {
        }
    }

    protected Dialog showDialog(int id) {
        switch (id) {
            case PICK_FROM_DATE:
                return new DatePickerDialog(mContext, callBack1, mFromYear, mFromMonth, mFromDay);
            case PICK_TO_DATE:
                return new DatePickerDialog(mContext, callBack2, mToYear, mToMonth, mToDay);
            case PICK_FROM_TIME:
                return new TimePickerDialog(mContext, callBack3, mFromDayTime == 0 ? mFromHour
                        : mFromHour + 12, mFromMinute, false);
            case PICK_TO_TIME:
                return new TimePickerDialog(mContext, callBack4, mToDayTime == 0 ? mToHour
                        : mToHour + 12, mToMinute, false);
            default:
                return null;
        }
    }

    private OnDateSetListener callBack1 = new OnDateSetListener() {
        @Override
        public void onDateSet(DatePicker view, int year, int monthOfYear,
                int dayOfMonth) {
            getCurrentTime();
            int hour = 0;
            if (mFromHour != 12) {
                hour = mFromDayTime == 0 ? mFromHour : mFromHour + 12;
            } else {
                hour = mFromHour;
            }
            Long sumFromMinute = 0L;
            Long sumFromDay = 0L;
            sumFromDay += getDayNums(year, monthOfYear, dayOfMonth);
            sumFromMinute += getMinuteNums(sumFromDay, hour, mFromMinute);

            Calendar c = Calendar.getInstance();
            c.set(Calendar.YEAR, year);
            c.set(Calendar.MONTH, monthOfYear);
            c.set(Calendar.DATE, dayOfMonth);
            mFromWeekday = c.get(Calendar.DAY_OF_WEEK);
            mFromYear = year;
            mFromMonth = monthOfYear;
            mFromDay = dayOfMonth;
            mFromDateButton.setText(DateUtils.getDayOfWeekString(mFromWeekday, 20) + ", "
                    + DateUtils.getMonthString(mFromMonth, 30)
                    + " " + mFromDay + ", " + mFromYear);

            if (mToHour != 12) {
                hour = mToDayTime == 0 ? mToHour : mToHour + 12;
            } else {
                hour = mToHour;
            }
            Long sumToMinute = 0L;
            Long sumToDay = 0L;
            sumToDay += getDayNums(mToYear, mToMonth, mToDay);
            sumToMinute += getMinuteNums(sumToDay, hour, mToMinute);
            if (sumToMinute < sumFromMinute) {
                mToYear = mFromYear;
                mToMonth = mFromMonth;
                mToDay = mFromDay;
                mToHour = mFromHour;
                mToMinute = mFromMinute;
                mToDayTime = mFromDayTime;
                mToWeekday = mFromWeekday;
                mToDateButton.setText(DateUtils.getDayOfWeekString(mToWeekday, 20) + ", "
                        + DateUtils.getMonthString(mToMonth, 30)
                        + " " + mToDay + ", " + mToYear);
                mToTimeButton.setText((mToHour == 0 ? 12 : mToHour) + ":"
                        + (mToMinute < 10 ? ("0" + mToMinute) : mToMinute)
                        + " " + DateUtils.getAMPMString(mToDayTime));
            }
        }
    };

    private OnDateSetListener callBack2 = new OnDateSetListener() {
        @Override
        public void onDateSet(DatePicker view, int year, int monthOfYear,
                int dayOfMonth) {
            int hour = 0;
            getCurrentTime();
            if (mCurrentHour != 12) {
                hour = mCurrentDayTime == 0 ? mCurrentHour : mCurrentHour + 12;
            } else {
                hour = mCurrentHour;
            }
            Long sumCurrentMinute = 0L;
            Long sumCurrentDay = 0L;
            sumCurrentDay += getDayNums(mCurrentYear, mCurrentMonth, mCurrentDay);
            sumCurrentMinute += getMinuteNums(sumCurrentDay, hour, mCurrentMinute);

            if (mToHour != 12) {
                hour = mToDayTime == 0 ? mToHour : mToHour + 12;
            } else {
                hour = mToHour;
            }
            Long sumToMinute = 0L;
            Long sumToDay = 0L;
            sumToDay += getDayNums(year, monthOfYear, dayOfMonth);
            sumToMinute += getMinuteNums(sumToDay, hour, mToMinute);
            if (sumToMinute < sumCurrentMinute) {
                if (sumToDay.equals(sumCurrentDay)) {
                    mToHour = mCurrentHour;
                    mToMinute = mCurrentMinute;
                    mToDayTime = mCurrentDayTime;
                    mToTimeButton.setText((mToHour == 0 ? 12 : mToHour) + ":"
                            + (mToMinute < 10 ? ("0" + mToMinute) : mToMinute)
                            + " " + DateUtils.getAMPMString(mToDayTime));
                    sumToMinute = sumCurrentMinute;
                } else {
                    Toast.makeText(mContext, getString(R.string.account_settings_oof_time_alert),
                            Toast.LENGTH_SHORT).show();
                    return;
                }
            }
            Calendar c = Calendar.getInstance();
            c.set(Calendar.YEAR, year);
            c.set(Calendar.MONTH, monthOfYear);
            c.set(Calendar.DATE, dayOfMonth);
            mToWeekday = c.get(Calendar.DAY_OF_WEEK);
            mToYear = year;
            mToMonth = monthOfYear;
            mToDay = dayOfMonth;
            mToDateButton.setText(DateUtils.getDayOfWeekString(mToWeekday, 20) + ", "
                    + DateUtils.getMonthString(mToMonth, 30)
                    + " " + mToDay + ", " + mToYear);

            if (mFromHour != 12) {
                hour = mFromDayTime == 0 ? mFromHour : mFromHour + 12;
            } else {
                hour = mFromHour;
            }
            Long sumFromMinute = 0L;
            Long sumFromDay = 0L;
            sumFromDay += getDayNums(mFromYear, mFromMonth, mFromDay);
            sumFromMinute += getMinuteNums(sumFromDay, hour, mFromMinute);
            if (sumToMinute < sumFromMinute) {
                mFromYear = mToYear;
                mFromMonth = mToMonth;
                mFromDay = mToDay;
                mFromHour = mToHour;
                mFromMinute = mToMinute;
                mFromDayTime = mToDayTime;
                mFromWeekday = mToWeekday;
                mFromDateButton.setText(DateUtils.getDayOfWeekString(mFromWeekday, 20) + ", "
                        + DateUtils.getMonthString(mFromMonth, 30)
                        + " " + mFromDay + ", " + mFromYear);
                mFromTimeButton.setText((mFromHour == 0 ? 12 : mFromHour) + ":"
                        + (mFromMinute < 10 ? ("0" + mFromMinute) : mFromMinute)
                        + " " + DateUtils.getAMPMString(mFromDayTime));
            }
        }
    };

    private OnTimeSetListener callBack3 = new OnTimeSetListener() {
        @Override
        public void onTimeSet(TimePicker view, int hourOfDay, int minute) {
            int hour;
            getCurrentTime();
            if (hourOfDay < 12) {
                mFromDayTime = 0;
            } else {
                mFromDayTime = 1;
            }
            Long sumFromMinute = 0L;
            Long sumFromDay = 0L;
            sumFromDay += getDayNums(mFromYear, mFromMonth, mFromDay);
            sumFromMinute += getMinuteNums(sumFromDay, hourOfDay, minute);

            mFromMinute = minute;
            mFromHour = hourOfDay;
            if (mFromHour > 12) {
                mFromHour -= 12;
            }
            mFromTimeButton.setText((mFromHour == 0 ? 12 : mFromHour) + ":"
                    + (mFromMinute < 10 ? ("0" + mFromMinute) : mFromMinute)
                    + " " + DateUtils.getAMPMString(mFromDayTime));

            if (mToHour != 12) {
                hour = mToDayTime == 0 ? mToHour : mToHour + 12;
            } else {
                hour = mToHour;
            }
            Long sumToMinute = 0L;
            Long sumToDay = 0L;
            sumToDay += getDayNums(mToYear, mToMonth, mToDay);
            sumToMinute += getMinuteNums(sumToDay, hour, mToMinute);
            if (sumToMinute < sumFromMinute) {
                mToYear = mFromYear;
                mToMonth = mFromMonth;
                mToDay = mFromDay;
                mToHour = mFromHour;
                mToMinute = mFromMinute;
                mToDayTime = mFromDayTime;
                mToWeekday = mFromWeekday;
                mToDateButton.setText(DateUtils.getDayOfWeekString(mToWeekday, 20) + ", "
                        + DateUtils.getMonthString(mToMonth, 30)
                        + " " + mToDay + ", " + mToYear);
                mToTimeButton.setText((mToHour == 0 ? 12 : mToHour) + ":"
                        + (mToMinute < 10 ? ("0" + mToMinute) : mToMinute)
                        + " " + DateUtils.getAMPMString(mToDayTime));
            }
        }
    };

    private OnTimeSetListener callBack4 = new OnTimeSetListener() {
        @Override
        public void onTimeSet(TimePicker view, int hourOfDay, int minute) {
            int hour;
            getCurrentTime();
            Long sumToMinute = 0L;
            Long sumToDay = 0L;
            sumToDay += getDayNums(mToYear, mToMonth, mToDay);
            sumToMinute += getMinuteNums(sumToDay, hourOfDay, minute);
            if (mCurrentHour != 12) {
                hour = mCurrentDayTime == 0 ? mCurrentHour : mCurrentHour + 12;
            } else {
                hour = mCurrentHour;
            }
            Long sumCurrentMinute = 0L;
            Long sumCurrentDay = 0L;
            sumCurrentDay += getDayNums(mCurrentYear, mCurrentMonth, mCurrentDay);
            sumCurrentMinute += getMinuteNums(sumCurrentDay, hour, mCurrentMinute);
            if (sumCurrentMinute > sumToMinute) {
                Toast.makeText(mContext, getString(R.string.account_settings_oof_time_alert),
                        Toast.LENGTH_SHORT).show();
                return;
            }

            /** M: The time of To should be set only if the set time is vaild @{*/
            if (hourOfDay < 12) {
                mToDayTime = 0;
            } else {
                mToDayTime = 1;
            }
            /** @} */
            mToHour = hourOfDay;
            mToMinute = minute;
            if (mToHour > 12) {
                mToHour -= 12;
            }

            mToTimeButton.setText((mToHour == 0 ? 12 : mToHour) + ":"
                    + (mToMinute < 10 ? ("0" + mToMinute) : mToMinute)
                    + " " + DateUtils.getAMPMString(mToDayTime));

            if (mFromHour != 12) {
                hour = mFromDayTime == 0 ? mFromHour : mFromHour + 12;
            } else {
                hour = mFromHour;
            }
            Long sumFromMinute = 0L;
            Long sumFromDay = 0L;
            sumFromDay += getDayNums(mFromYear, mFromMonth, mFromDay);
            sumFromMinute += getMinuteNums(sumFromDay, hour, mFromMinute);
            if (sumToMinute < sumFromMinute) {
                mFromYear = mToYear;
                mFromMonth = mToMonth;
                mFromDay = mToDay;
                mFromHour = mToHour;
                mFromMinute = mToMinute;
                mFromDayTime = mToDayTime;
                mFromWeekday = mToWeekday;
                mFromDateButton.setText(DateUtils.getDayOfWeekString(mFromWeekday, 20) + ", "
                        + DateUtils.getMonthString(mFromMonth, 30)
                        + " " + mFromDay + ", " + mFromYear);
                mFromTimeButton.setText((mFromHour == 0 ? 12 : mFromHour) + ":"
                        + (mFromMinute < 10 ? ("0" + mFromMinute) : mFromMinute)
                        + " " + DateUtils.getAMPMString(mFromDayTime));
            }
        }
    };

    private Long getDayNums(int year, int month, int dayOfMonth) {
        long newYear = (long) (year - 1899);
        long newMonth = (long) month;
        long newDayOfMonth = (long) dayOfMonth;
        long result = newYear * 372 + newMonth * 31 + newDayOfMonth;
        return result;
    }

    private Long getMinuteNums(Long days, int hour, int minute) {
        long newHour = (long) hour;
        long newMinute = (long) minute;
        return days * 43200 + newHour * 60 + newMinute;
    }

    public void enableOutOfficeOption() {
        mFrom.setEnabled(true);
        mTo.setEnabled(true);
        mAutoReply.setEnabled(true);
        mReplySumm.setEnabled(true);
        mToServer.setEnabled(true);
        mFromDateButton.setEnabled(true);
        mFromTimeButton.setEnabled(true);
        mToDateButton.setEnabled(true);
        mToTimeButton.setEnabled(true);
        mSeverCheck.setEnabled(true);
        mExternalView.setEnabled(true);
        mReplyView.setEnabled(true);
        mTitleSumm.setText(R.string.account_settings_oof_checked_summary);
    }

    public void disableOutOfficeOption() {
        mFrom.setEnabled(false);
        mTo.setEnabled(false);
        mAutoReply.setEnabled(false);
        mReplySumm.setEnabled(false);
        mToServer.setEnabled(false);
        mFromDateButton.setEnabled(false);
        mFromTimeButton.setEnabled(false);
        mToDateButton.setEnabled(false);
        mToTimeButton.setEnabled(false);
        mSeverCheck.setEnabled(false);
        mExternalView.setEnabled(false);
        mReplyView.setEnabled(false);
        mTitleSumm.setText(R.string.account_settings_oof_unchecked_summary);
    }

    public void addChangeListener() {
        mTitleSumm.addTextChangedListener(new TextChangeWatcher());
        mFromDateButton.addTextChangedListener(new TextChangeWatcher());
        mFromTimeButton.addTextChangedListener(new TextChangeWatcher());
        mToDateButton.addTextChangedListener(new TextChangeWatcher());
        mToTimeButton.addTextChangedListener(new TextChangeWatcher());
        mSeverCheck.setOnCheckedChangeListener(new OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                mSaveButton.setEnabled(true);
            }
        });
        mOutOffice.setOnCheckedChangeListener(new OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                mSaveButton.setEnabled(true);
            }
        });
    }

    public static class DateTimePickerDialogFragment extends DialogFragment {
        public static final String TAG = "DateTimePickerDialogFragment";

        public DateTimePickerDialogFragment() {
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            AccountSettingsOutOfOfficeFragment f =
                    (AccountSettingsOutOfOfficeFragment) getTargetFragment();
            return f.showDialog(getTargetRequestCode());
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
        }

        public static void showDialog(AccountSettingsOutOfOfficeFragment f,
                int dialogId) {
            DateTimePickerDialogFragment dialogFragment =
                    new DateTimePickerDialogFragment();
            dialogFragment.setTargetFragment(f, dialogId);

            dialogFragment.show(f.getFragmentManager(), TAG + dialogId);
        }
    }
}
