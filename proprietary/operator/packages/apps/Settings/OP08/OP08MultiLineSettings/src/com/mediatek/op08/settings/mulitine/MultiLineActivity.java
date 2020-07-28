package com.mediatek.op08.settings.mulitine;

import android.app.Activity;
import android.app.ProgressDialog;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import com.mediatek.digits.DigitsConst;
import com.mediatek.digits.DigitsManager;
import com.mediatek.digits.DigitsProfile;

/**
 * Activity to provide option to user for login and signup into TMO Digits.
 */
public class MultiLineActivity extends Activity implements View.OnClickListener {

    public static final String TAG = "MultiLineActivity";
    public static final String EXTRA_MESSAGE = "com.multiline.mtk33170.url";
    private DigitsManager mManager;
    private DigitsProfile mProfile;
    private int mState;
    private Context mContext;
    private boolean mConnected = false;
    private boolean mSubscribed = false;
    private String mE911ServerUrl = null;
    private String mE911ServerData = null;
    private WebsheetFragment mWebsheetFragment = null;
    private SignUpWebsheetFragment mSignUpWebsheetFragment = null;
    private E911WebsheetFragment mE911WebsheetFragment = null;
    private ProgressDialog mPd = null;
    private boolean mOpenSettingsByWeb = false;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_multi_line);
        mContext = this;
        mManager = new DigitsManager(this);
        mManager.registerEventListener(mListener);
        mProfile = mManager.getDigitsProfile();
        mState = mManager.getServiceState();
        Button loginButton = (Button) findViewById(R.id.buttonSignIn);
        loginButton.setOnClickListener(this);
        Button buttonCreateAccount = (Button) findViewById(R.id.buttonCreateAccount);
        buttonCreateAccount.setOnClickListener(this);
        /*Button buttonRequestUse = (Button) findViewById(R.id.buttonRequestUse);
        buttonRequestUse.setOnClickListener(this);*/
        ((TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE)).listen(
                mPhoneStateListener, PhoneStateListener.LISTEN_SERVICE_STATE);
        showProgressDialog();
    }

    @Override
    protected void onResume() {
         super.onResume();
         TelephonyManager tm =
                (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        int state = tm.getSimState();
        if (state == TelephonyManager.SIM_STATE_ABSENT ||
            state == TelephonyManager.SIM_STATE_NOT_READY) {
            Toast.makeText(getApplicationContext(), "No SIM Available", Toast.LENGTH_LONG).show();
            finish();
        }
    }
    private PhoneStateListener mPhoneStateListener = new PhoneStateListener() {

        @Override
        public void onServiceStateChanged(ServiceState serviceState) {
            final TelephonyManager tm =
                (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
            int state = tm.getSimState();
            if (state == TelephonyManager.SIM_STATE_ABSENT ||
                state == TelephonyManager.SIM_STATE_NOT_READY) {
                Toast.makeText(getApplicationContext(), "No SIM Available",
                        Toast.LENGTH_LONG).show();
                finish();
            }
        }
    };

    private DigitsManager.EventListener mListener = new DigitsManager.EventListener() {
        @Override
        public void onEvent(int event, int result, Bundle extras) {

            int state = mManager.getServiceState();
            Log.i(TAG, "mListener onEvent(), event:" + DigitsConst.eventToString(event) +
                    ", result:" + result + ", extras:" + extras + ", Current State: " + state);
            switch (event) {
                case DigitsConst.EVENT_SERVICE_CONNECTION_RESULT:
                    if (result == DigitsConst.RESULT_SUCCEED) {
                        mConnected = true;
                        if ((mPd != null) && mPd.isShowing()) {
                            mPd.dismiss();
                        }
                        if (state == DigitsConst.STATE_SUBSCRIBED && !mOpenSettingsByWeb) {
                            mSubscribed = true;
                            goToMultiLineSettingsActivity();
                        }

                    }
                    break;
                case DigitsConst.EVENT_SERVICE_STATE_CHANGED:
                    mState = mManager.getServiceState();
                    if (mState == DigitsConst.STATE_SUBSCRIBED &&
                            result == DigitsConst.RESULT_SUCCEED && !mOpenSettingsByWeb) {
                        mSubscribed = true;
                        goToMultiLineSettingsActivity();
                    }
                    break;
                case DigitsConst.EVENT_NO_E911_ADDRESS_ON_FILE:
                    mE911ServerUrl = extras.getString(DigitsConst.EXTRA_STRING_SERVER_URL);
                    mE911ServerData = extras.getString(DigitsConst.EXTRA_STRING_SERVER_DATA);
                    launchE911WebSheetFragment();
                    break;
                default:
                    return;
            }
        }
    };

    private void showProgressDialog() {
        // TODO Auto-generated method stub
        Log.d(TAG, "showProgressDialog");
        mPd = new ProgressDialog(this);
        mPd.setMessage(mContext.getText(R.string.wait_message));
        mPd.setCancelable(true);
        mPd.setCanceledOnTouchOutside(false);
        mPd.setOnDismissListener(new DialogInterface.OnDismissListener() {
            @Override
            public void onDismiss(DialogInterface dialog) {
                if (mPd != null) {
                    mPd = null;
                }
            }
        });
        mPd.setIndeterminate(true);
        mPd.show();
    }

    /**
     * Provide Login url to current Web sheet.
     * @return provide the url to open for Login
     */
    public String getLoginUrl() {
        return mManager.getTmoidLoginURL();
    }

    /**
     * Open the Settings App and close the current Web sheet.
     * @return provide the url to open for Sign Up
     */
    public String getSignUpUrl() {
        return mManager.getTmoidSignupURL();
    }

    /**
     * Open the Settings App and close the current Web sheet.
     * @return provide the url to open for Entitlement
     */
    public String getEntitlementUrl() {
        return mE911ServerUrl;
    }

    /**
     * Open the Settings App and close the current Web sheet.
     * @return provide the data to open for Entitlement
     */
    public String getEntitlementPostData() {
        return mE911ServerData;
    }
    @Override
    public void onClick(View view) {
        String message = "https://www.google.com";
        Intent intent = new Intent();
        if (mManager != null && mConnected) {
            int state = mManager.getServiceState();
            if (state == DigitsConst.STATE_IDLE ||
                    state == DigitsConst.STATE_UNKNOWN) {
                Toast.makeText(this, "Service not available from Network", Toast.LENGTH_LONG).
                        show();
                return;
            }
        }
        switch (view.getId()) {
            case R.id.buttonSignIn:
                if (mManager != null && mConnected) {
                    if (mSubscribed && mProfile != null && mProfile.getEmail().length() != 0) {
                        goToMultiLineSettingsActivity();
                    } else {
                        message = mManager.getTmoidLoginURL();
                        /*if (message.length() > 0) {
                            Intent login = new Intent(this, LoginActivity.class);
                            login.putExtra(EXTRA_MESSAGE, message);
                            startActivity(login);
                            intent.setClass(mContext, MultiLineLoginService.class);
                            mContext.startService(intent);
                        }*/
                        FragmentTransaction ft = getFragmentManager().beginTransaction();
                        if (mWebsheetFragment == null) {
                            mWebsheetFragment = new WebsheetFragment();
                        }
                        mOpenSettingsByWeb = true;
                        ft.replace(android.R.id.content, mWebsheetFragment);
                        ft.addToBackStack(null);
                        ft.commit();
                    }
                }
                break;
            case R.id.buttonCreateAccount:
                if (mManager != null  && mConnected == true) {
                    message = mManager.getTmoidSignupURL();
                    if (message.length() > 0) {
                        FragmentTransaction ft = getFragmentManager().beginTransaction();
                        if (mSignUpWebsheetFragment == null) {
                            mSignUpWebsheetFragment = new SignUpWebsheetFragment();
                        }
                        mOpenSettingsByWeb = true;
                        ft.replace(android.R.id.content, mSignUpWebsheetFragment);
                        ft.addToBackStack(null);
                        ft.commit();
                    }
                }
                break;
            case R.id.buttonRequestUse:
                /* Currently TMO's document said user can activate a new line without login.
                   But TMO server isn't ready, so we only add public API, but not implement yet

                   void initTmoidCreation(String msisdn)
                   void initMsisdnAuthAndUpdateTmoId(String msisdn, String email, String password)*/
                break;
            default:
                Toast.makeText(this, "No Activity available", Toast.LENGTH_LONG).show();
                return;
        }
    }

    /**
     * Launch E911 fragment to add user address.
     */
    private void launchE911WebSheetFragment() {
        finishFragment();
        FragmentTransaction ft = getFragmentManager().beginTransaction();
        if (mE911WebsheetFragment == null) {
            mE911WebsheetFragment = new E911WebsheetFragment();
        }
        ft.replace(android.R.id.content, mE911WebsheetFragment);
        ft.addToBackStack(null);
        ft.commit();
    }

    @Override
    protected void onDestroy() {
        mManager.unregisterEventListener(mListener);
        mManager.shutdown();
        ((TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE)).listen(
                mPhoneStateListener, PhoneStateListener.LISTEN_NONE);
        if ((mPd != null) && mPd.isShowing()) {
            mPd.dismiss();
            mPd = null;
        }
        super.onDestroy();
    }

    /**
     * Method of WebsheetFragment.WebsheetFragmentListener interface.
     * @return
     */
    public void finishFragment() {
        getFragmentManager().popBackStackImmediate();
    }

    /**
    * Method of WebsheetFragment.WebsheetFragmentListener interface.
    * @param isComplete Abandoned/Completed
    */
    public void webSheetStateChange(boolean isComplete) {
        getFragmentManager().popBackStackImmediate();
       /* if (!isComplete) {
            cancelEntitlementCheck();
        }*/
    }

    /**
     * Start Multi Line Settings Activity.
     */
    public void goToMultiLineSettingsActivity() {
        Intent requestUse = new Intent(this, MultiLineSettingsActivity.class);
        startActivity(requestUse);
    }

    /**
     * Close the current Web sheet fragment.
     * @param ft Fragment activity to be close
     */
    public void closeFragment(Fragment ft) {
        getFragmentManager().beginTransaction().remove(ft).commit();
        mOpenSettingsByWeb = false;
    }

    /**
     * Open the Settings App and close the current Web sheet fragment.
     * @param ft Fragment activity to be close
     */
    public void closeFragmentHostAndOpenSettings(Fragment ft) {
        getFragmentManager().beginTransaction().remove(ft).commit();
        goToMultiLineSettingsActivity();
    }

    public Context getMultilineContext() {
        return mContext;
    }
   /**
     * Provide DigitManager's instance tp the current Web sheet fragment.
     * @return DigitManager's instance
     */
    public DigitsManager getDigitsManagerInstance() {
        return mManager;
    }

    /**
     * Get the user's profile if available .
     * @return email id of current user
     */
    public String getProfileEmail() {
        if (mSubscribed && mProfile != null && mProfile.getEmail().length() != 0) {
            return mProfile.getEmail();
        } else {
            return null;
        }
    }
}
