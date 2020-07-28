package com.mediatek.op12.settings.wfc.ui;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.app.ProgressDialog;
import android.app.QueuedWork;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.TextView;
import android.widget.Toast;

import com.android.ims.ImsConfig;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;

import com.mediatek.op12.settings.R;
import com.mediatek.optin.OptInServerApi;
import com.mediatek.optin.ErrorCodes;
import com.mediatek.optin.parser.RequestResponse;
import com.mediatek.optin.parser.RequestResponse.Address;

import java.util.ArrayList;


/** LTE-RCS-UI-Guidelines-4.pdf:page#68:W72.
 * Activity to display/edit emergency address to user.
 */

public class E911AddressActivity extends Activity implements E911UiCallback {

    private static final String TAG = "OP12E911AddressActivity";

    public static final String INTENT_ACTIVATE_WFC = "com.mediatek.intent.action.ACTIVATE_WFC_REQ";
    public static final String INTENT_UPDATE_E911
            = "com.mediatek.intent.action.WFC_UPDATE_E911";
    public static final String INTENT_UPDATE_E911_REQ
            = "com.mediatek.intent.action.WFC_UPDATE_E911_REQ";
    public static final String INTENT_ACTIVATE_WFC_RESP
            = "com.mediatek.intent.action.ACTIVATE_WFC_RSP";
    public static final String INTENT_UPDATE_E911_RESP
            = "com.mediatek.intent.action.WFC_UPDATE_E911_RSP";
    public static final String PROVISIONED_EXTRA = "isProvisioned";
    public static final String PROVISION_FAIL_REASON_EXTRA = "fail_reason";

    private static final String PROPERTY_VOWIFI_MDN = "persist.vendor.sys.vowifiMdn";

    private static final String FOUND = "FOUND";
    private static final String NOTFOUND = "NOTFOUND";
    private static final String INVALID = "INVALID";
    private static final String SUCCESS = "SUCCESS";
    private static final String USER_DETAILS = "userDetails";
    private static final String ALT_ADDRESS_DETAILS = "altAddresses";
    private static final String SUCCESS_CODE = "00000";
    private static final String SUCCESS_TOKEN = "0";
    private static final String NOTFOUND_CODE = "13009";
    private static final String INVALID_TOKEN = "13013";

    private static final int SHOW_DIALOG = 0;
    private static final int DISMISS_DIALOG = 1;
    private static final int SHOW_FRAGMENT = 2;
    private static final int SHOW_TOAST = 3;

    private static final int PROGRESS_DIALOG = 0;
    private static final int NW_ERROR_DIALOG = 1;
    private static final int USER_ENTERED_E911_ADDRESS_DIALOG = 2;
    private static final int ALT_E911_ADDRESS_DIALOG = 3;
    private static final int SAVE_ANYWAY_E911_ADDRESS_DIALOG = 4;
    private static final int E911_ADDRESS_VALIDATION_FAILED_DIALOG = 5;

    private static final int E911_FRAGMENT = 0;
    private static final int TC_FRAGMENT = 1;

    private static final int WFC_ACTIVATED = 0;
    private static final int E911_ADDRESS_UPDATED = 1;
    private static final int E911_ADDRESS_NOT_UPDATED = 2;

    private RequestResponse.Address mUserAddress;
    private ArrayList<RequestResponse.Address> mAltAddress;
    private ProgressDialog mLoadingDialog;

    private Handler mHandler;
    private OptInServerApi mOptInServerApi;
    private NwInteractionTask mTask;

    /** Enum for states.
     */
    private enum E911States { STATE_E911_NO_STATE , STATE_E911_TC, STATE_E911_USER_INTERACTION,
            STATE_E911_USER_ENTERED_ADDRESS, STATE_E911_ALT_ADDRESS,
            STATE_E911_SAVE_ANYWAY, STATE_E911_ADDRESS_VALIDATION_FAILED };

    private E911States mCurrentState = E911States.STATE_E911_NO_STATE;

    private DialogInterface.OnCancelListener mDialogCancelListener
            = new DialogInterface.OnCancelListener() {
        @Override
        public void onCancel(final DialogInterface dialog) {
            Log.d(TAG, "onCancel:" + dialog);
            if (mLoadingDialog == dialog) {
                if (mTask != null) {
                    mTask.cancel(true);
                    mTask = null;
                }
                mOptInServerApi.cancelRequest();
            }
            //reset the state as dialog is dismissed and user is back on e911 fragment
            if (mCurrentState == E911States.STATE_E911_NO_STATE) {
                finish();
            } else {
                Log.d(TAG, "reset state to STATE_E911_USER_INTERACTION");
                mCurrentState = E911States.STATE_E911_USER_INTERACTION;
            }
        }
    };

    /* Activity can be invoked in 2 cases:
     * 1) To Activate WFC: via intent INTENT_ACTIVATE_WFC
     *      (i) On query: if address found, finish activity & if  not found,
     *            show fragment to interact with user.
     *      (ii) On Add: if added Successfully,Finish activity & if failed,show appropriate dialog
     * 2) Update E911 address after activation: via intent INTENT_UPDATE_E911
     *      (i) On query: If address found, show filled address
     *          fragment else empty fields in fragment.
     *      (ii) On Add: If added Successfully, show toast & finish activity else
     *          & if failed, show appropriate dialog.
     */
    private class ResponseListener implements OptInServerApi.OptInResponseCallback {
        @Override
        public void onQueryAddressResponse(RequestResponse.QueryAddressResponse rsp) {
            Log.d(TAG, "onQueryAddressResponse");
            Log.d(TAG, "mCurrentState: " + mCurrentState);
            Log.d(TAG, "errCode: " + rsp.errStatus.errCode);
            Log.d(TAG, "user address: " + rsp.userDetail.userAddress);
            Intent i = getIntent();
            Log.d(TAG, "intent is: " + i.getAction());
            if (SUCCESS_CODE.equalsIgnoreCase(rsp.errStatus.errCode)
                    || SUCCESS_TOKEN.equalsIgnoreCase(rsp.errStatus.errCode)) {
                Log.d(TAG, "query success");
                if (INTENT_ACTIVATE_WFC.equals(i.getAction())) {
                    //configureMdn(rsp.userDetail.mdn);
                    broadcastResponse(false, ErrorCodes.E911_ERR_CODE_SUCCESS);
                    mHandler.sendMessage(mHandler.obtainMessage(SHOW_TOAST, WFC_ACTIVATED, -1));
                    finish();
                } else if (INTENT_UPDATE_E911.equals(i.getAction())
                        || INTENT_UPDATE_E911_REQ.equals(i.getAction())) {
                    mHandler.sendMessage(mHandler.obtainMessage(SHOW_FRAGMENT, E911_FRAGMENT
                            , -1, rsp.userDetail.userAddress));
                }
            } else if (NOTFOUND_CODE.equalsIgnoreCase(rsp.errStatus.errCode)) {
                if (INTENT_ACTIVATE_WFC.equals(i.getAction())) {
                    mHandler.sendMessage(mHandler.obtainMessage(SHOW_FRAGMENT, TC_FRAGMENT
                            , -1, rsp.userDetail.userAddress));
                } else if (INTENT_UPDATE_E911.equals(i.getAction())
                        || INTENT_UPDATE_E911_REQ.equals(i.getAction())) {
                    // TODO: QUESTION: Can this happen: WFC activated, user enters in E911,
                    //Ue queries but no address found??
                    mHandler.sendMessage(mHandler.obtainMessage(SHOW_FRAGMENT, E911_FRAGMENT
                            , -1, rsp.userDetail.userAddress));
                }
            } else {
                manageOtherErrors(rsp.errStatus, rsp.userDetail.userAddress);
            }
        }

        @Override
        public void onAddUpdateAddressResponse(RequestResponse.AddUpdateAddressResponse rsp) {
            Log.d(TAG, "onAddUpdateAddressResponse");
            Log.d(TAG, "mCurrentState: " + mCurrentState);
            Log.d(TAG, "errCode: " + rsp.errStatus.errCode);

            if (SUCCESS_CODE.equalsIgnoreCase(rsp.errStatus.errCode)
                    || SUCCESS_TOKEN.equalsIgnoreCase(rsp.errStatus.errCode)) {
         // TODO: QUESTION, in which case will mdn will be reset??: sim change & factory reset
                if (INTENT_ACTIVATE_WFC.equals(getIntent().getAction())) {
                    configureMdn(rsp.userDetail.mdn);
                    broadcastResponse(true, ErrorCodes.E911_ERR_CODE_SUCCESS);
                    mHandler.sendMessage(mHandler.obtainMessage(SHOW_TOAST, WFC_ACTIVATED, -1));
                } else if (INTENT_UPDATE_E911.equals(getIntent().getAction())
                        || INTENT_UPDATE_E911_REQ.equals(getIntent().getAction())) {
                    mHandler.sendMessage(mHandler
                            .obtainMessage(SHOW_TOAST, E911_ADDRESS_UPDATED, -1));
                }
                finish();
            } else if (INVALID.equalsIgnoreCase(rsp.errStatus.errCode)) {
                /* Network interaction happens on only two states:STATE_E911_USER_INTERACTION
                 * STATE_E911_SAVE_ANYWAY. So handling only these 2 states. */
                mUserAddress = rsp.userDetail.userAddress;
                if (mCurrentState == E911States.STATE_E911_USER_INTERACTION) {
                    // check if alt address found, if yes, show alt address dialog
                    // else only user address dialog.
                    if (rsp.altAddress == null || rsp.altAddress.size() == 0) {
                        mHandler.sendMessage(mHandler.obtainMessage(SHOW_DIALOG
                                , USER_ENTERED_E911_ADDRESS_DIALOG, -1
                                , rsp.userDetail.userAddress));
                    } else {
                        mAltAddress = rsp.altAddress;
                        mHandler.sendMessage(mHandler.obtainMessage(SHOW_DIALOG
                            , ALT_E911_ADDRESS_DIALOG, -1, rsp));
                    }
                } else if (mCurrentState == E911States.STATE_E911_SAVE_ANYWAY) {
                    mHandler.sendMessage(mHandler.obtainMessage(SHOW_DIALOG
                            , E911_ADDRESS_VALIDATION_FAILED_DIALOG
                            , -1, rsp.userDetail.userAddress));
                }
            } else {
                manageOtherErrors(rsp.errStatus, rsp.userDetail.userAddress);
            }
        }

        @Override
        public void onNetworkError(int cause) {
            Log.d(TAG, "onNetworkError: " + cause);
            handleNetworkError(cause);
        }
    }
    private ResponseListener mRespListener;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.e911_activity_layout);

        mHandler = new UiHandler();

        mOptInServerApi = OptInServerApi.getInstance(this);
        mRespListener = new ResponseListener();

        mTask = new NwInteractionTask(RequestResponse.QUERY_REQUEST, null);
        mTask.execute();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        mCurrentState = E911States.STATE_E911_NO_STATE;
        if (mTask != null) {
            mTask.cancel(true);
            mTask = null;
        }
        mOptInServerApi.cancelRequest();
    }

/****************************** Fragment & Dialogs Callbacks *************************************/
    /**
         * Method to intimate user choice of saving E911 address.
         * @param userDetails userDetails entered
         * @return
        */
    public void saveE911Adress(RequestResponse.Address userDetails) {
        Log.d(TAG, "saveE911Adress, mTask:" + mTask);
        if (mTask == null) {
            mTask = new NwInteractionTask(RequestResponse.ADD_REQUEST, userDetails);
            mTask.execute();
        }
    }

    /**
     * Method to intimate activity to perform partial validation of address.
     * @param userDetails userDetails entered
     * @return
     */
    public void doPartialValidationE911Add(RequestResponse.Address userDetails) {
        Log.d(TAG, "doPartialValidationE911Add, mTask:" + mTask);
        if (mTask == null) {
            mTask = new NwInteractionTask(RequestResponse.PARTIAL_VALIDATION_REQUEST, userDetails);
            mTask.execute();
        }
    }

    /**
     * Method to intimate Activity to show SaveAnyway dialog.
     * @param userDetails userDetails entered
     * @return
     */
    public void showSaveAnywayE911Dialog(RequestResponse.Address userDetails) {
        mHandler.sendMessage(mHandler.obtainMessage(SHOW_DIALOG, SAVE_ANYWAY_E911_ADDRESS_DIALOG
                                    , -1, userDetails));
    }

    /**
     * Method to intimate Activity to show E911 user interaction fragment.
     * @param userDetails userDetails entered
     * @return
     */
    public void showE911AddressFragment(RequestResponse.Address userDetails) {
        mHandler.sendMessage(mHandler.obtainMessage(SHOW_FRAGMENT, E911_FRAGMENT
                                    , -1, userDetails));
    }

    /**
     * Finish upper most fragment.
     * @return
     */
    public void finishFragment() {
        getFragmentManager().popBackStackImmediate();
    }

    /**
     * Finish upper most fragment.
     * @return
     */
    public void finishActivity() {
        finish();
    }

    /**
     * Method to intimate Activity about dismissing of dialog interacting with user.
     * @return
     */
    public void onDialogDismiss() {
        mCurrentState = E911States.STATE_E911_USER_INTERACTION;
    }

    /**
     * Method to intimate Activity about user's cancel selection.
     * @return
     */
    public void handleUserCancel() {
        broadcastResponse(false, ErrorCodes.E911_ERR_USER_CANCELED);
        finish();
    }

/****************************************************************************/

    private void handleNetworkError(int cause) {
        //broadcastResponse(false, cause);
        mHandler.sendMessage(mHandler.obtainMessage(SHOW_DIALOG, NW_ERROR_DIALOG, -1));
    }

    private void manageOtherErrors(RequestResponse.ErrorCode error, Address address) {
        if(error == null) {
            Log.d(TAG, "manageOtherErrors: null");
            return;
        }
        int errorCode = -1;
        int errorLevel = -1;
        Log.d(TAG, "manageOtherErrors errorCode:" + error.errCode + "errorlevel:" + error.errLevel);
        if(error.errCode !=null) {
            Log.d(TAG, "manageOtherErrors errorCode length:" + error.errCode.length());
        }
        if(!TextUtils.isEmpty(error.errCode) && TextUtils.isDigitsOnly(error.errCode)) {
            errorCode = Integer.parseInt(error.errCode);
        }
        if(!TextUtils.isEmpty(error.errLevel) && TextUtils.isDigitsOnly(error.errLevel)) {
            errorLevel = Integer.parseInt(error.errLevel);
        }
        //broadcastResponse(false, errorCode);
        switch (errorCode) {
            case ErrorCodes.E911_ERR_CODE_INVALID_TOKEN:
                // TODO: refresh token, really needed, can query everytime via spc interface
                mHandler.sendMessage(mHandler.obtainMessage(SHOW_DIALOG, NW_ERROR_DIALOG, -1));
                break;
            case ErrorCodes.E911_ERR_CODE_INTERNAL_SERVER_ERROR:
                mHandler.sendMessage(mHandler.obtainMessage(SHOW_DIALOG, NW_ERROR_DIALOG, -1));
            //case ErrorCodes.E911_ERR_CODE_TOKEN_NOT_PROVIDED:
                break;
            case ErrorCodes.E911_ERR_CODE_TIMEOUT:
                mHandler.sendMessage(mHandler.obtainMessage(SHOW_DIALOG, NW_ERROR_DIALOG, -1));
                break;
            case ErrorCodes.E911_ERR_CODE_NO_RECORD_FOUND:
                if (INTENT_ACTIVATE_WFC.equals(getIntent().getAction())) {
                    mHandler.sendMessage(mHandler.obtainMessage(SHOW_FRAGMENT, TC_FRAGMENT
                            , -1, address));
                } else if (INTENT_UPDATE_E911.equals(getIntent().getAction())
                        || INTENT_UPDATE_E911_REQ.equals(getIntent().getAction())) {
                    // TODO: QUESTION: Can this happen: WFC activated, user enters in E911,
                    //Ue queries but no address found??
                    mHandler.sendMessage(mHandler.obtainMessage(SHOW_FRAGMENT, E911_FRAGMENT
                            , -1, address));
                }
                return;
            default:
                Log.d(TAG, "invalid error cause:" + errorCode);
                break;
        }
    }

    private void showProgressDialog() {
        Log.d(TAG, "showProgressDialog");
        mLoadingDialog = new ProgressDialog(this);
        mLoadingDialog.setMessage(getResources().getString(R.string.loading_message));
        mLoadingDialog.setCancelable(true);
        mLoadingDialog.setCanceledOnTouchOutside(false);
        mLoadingDialog.setInverseBackgroundForced(false);
        mLoadingDialog.setOnCancelListener(mDialogCancelListener);
        mLoadingDialog.show();
    }

    private void dismissProgressDialog() {
        if (mLoadingDialog != null && mLoadingDialog.isShowing()) {
            mLoadingDialog.dismiss();
        }
    }

    private void showNetworkErrorDialog() {
        Log.d(TAG, "showNetworkErrorDialog entry");
        Intent i = new Intent();
        if (INTENT_ACTIVATE_WFC.equals(getIntent().getAction())) {
            Log.d(TAG, "showNetworkErrorDialog for wfc activation");
        } else if (INTENT_UPDATE_E911.equals(getIntent().getAction())
                || INTENT_UPDATE_E911_REQ.equals(getIntent().getAction())) {
            Log.d(TAG, "showNetworkErrorDialog for address updation");
            showNetworkErrorDialogForUpdate();
            return;
        }        
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage(R.string.unable_to_activate_wfc)
                .setPositiveButton(android.R.string.ok,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                finish();
                            }
                        });
        AlertDialog dialog = builder.create();
        dialog.setCancelable(false);
        dialog.setCanceledOnTouchOutside(false);
        dialog.show();
    }

    private void showNetworkErrorDialogForUpdate() {
        Log.d(TAG, "showNetworkErrorDialogForUpdate entry");
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setMessage(R.string.unable_to_update_wfc)
                .setPositiveButton(android.R.string.ok,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                finish();
                            }
                        });
        AlertDialog dialog = builder.create();
        dialog.setCancelable(false);
        dialog.setCanceledOnTouchOutside(false);
        dialog.show();
    }

    private void showE911Dialog(int id) {
        FragmentTransaction ft = getFragmentManager().beginTransaction();
        Fragment prev = getFragmentManager().findFragmentByTag("E911Dialog");
        Log.d(TAG, "prev:" + prev);
        if (prev != null) {
            ft.remove(prev);
    }
        //ft.addToBackStack(null);

        // Create and show the dialog.
        DialogFragment newFragment = new E911DialogFragment(id);
        newFragment.show(getFragmentManager(), "E911Dialog");
    }

    private void showTcFragment(RequestResponse.Address rsp) {
        Fragment fragment = new TCFragment(rsp);
        FragmentTransaction ft = getFragmentManager().beginTransaction();

        ft.replace(R.id.fragment_container, fragment);
        //ft.addToBackStack(null);

        ft.commitAllowingStateLoss();
    }

    private void showE911Fragment(RequestResponse.Address rsp) {
        Fragment fragment = new E911AddressFragment(rsp);
        FragmentTransaction ft = getFragmentManager().beginTransaction();

        ft.replace(R.id.fragment_container, fragment);
        //ft.addToBackStack(null);
        ft.commitAllowingStateLoss();
    }

    private void broadcastResponse(boolean wfcProvisioned, int extra) {
        Intent i = new Intent();
        if (INTENT_ACTIVATE_WFC.equals(getIntent().getAction())) {
            i.setAction(INTENT_ACTIVATE_WFC_RESP);
        } else if (INTENT_UPDATE_E911.equals(getIntent().getAction())
                || INTENT_UPDATE_E911_REQ.equals(getIntent().getAction())) {
            i.setAction(INTENT_UPDATE_E911_RESP);
        }
        i.putExtra(PROVISIONED_EXTRA, wfcProvisioned);
        i.putExtra(PROVISION_FAIL_REASON_EXTRA, extra);
        Log.d(TAG, "broadcastResponse: " + i);
        sendBroadcast(i);
    }

    private void configureMdn(String mdn) {
        // TODO: can this happen after SUCCESS response that sim mdn & response mdn do not match??
        //String simMdn = TelephonyManager.getCdmaMdn(SubscriptionManager.getDefaultSubId());
        //SystemProperties.set(PROPERTY_VOWIFI_MDN, mdn);
        Log.d(TAG, "mdn: " + mdn);
        final ImsConfig imsConfig = getImsConfig(SubscriptionManager.getDefaultVoicePhoneId());
        if (imsConfig != null) {
            final boolean newValue = mdn.equals("0") ? false : true;
            Log.d(TAG, "newValue: " + newValue);
            QueuedWork.queue(new Runnable() {
                public void run() {
                    try {
                        //imsConfig.setProvisionedValue(
                            //ImsConfig.ConfigConstants.VOICE_OVER_WIFI_SETTING_ENABLED,
                            //newValue ? 1 : 0);
                        Log.d(TAG, "Set wfc provision success: ");
                        imsConfig.setProvisionedValue(
                                ImsConfig.ConfigConstants.VOICE_OVER_WIFI_SETTING_ENABLED, 1);
                    } catch (ImsException e) {
                        Log.e(TAG, " VOICE_OVER_WIFI_SETTING_ENABLED provision exception:");
                        e.printStackTrace();
                    }
                }
            }, false);
        }
    }

    private ImsConfig getImsConfig(int phoneId) {
        ImsConfig imsConfig = null;
        ImsManager imsManager = ImsManager.getInstance(this, phoneId);
        if (imsManager != null) {
            try {
                imsConfig = imsManager.getConfigInterface();
            } catch (ImsException e) {
                e.printStackTrace();
            }
        } else {
           Log.e(TAG, "ImsManager null");
        }
        return imsConfig;
    }

    /**
     * Handler to handle UI changes.
     */
    private class UiHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case SHOW_DIALOG: {
                    int whichDialog = msg.arg1;
                    Log.d(TAG, "Show dialog:" + whichDialog);
                    switch (whichDialog) {
                        case PROGRESS_DIALOG:
                            showProgressDialog();
                            break;
                        case NW_ERROR_DIALOG:
                            showNetworkErrorDialog();
                            break;
                        case USER_ENTERED_E911_ADDRESS_DIALOG:
                            mCurrentState = E911States.STATE_E911_USER_ENTERED_ADDRESS;
                            showE911Dialog(USER_ENTERED_E911_ADDRESS_DIALOG);
                            break;
                        case ALT_E911_ADDRESS_DIALOG:
                            mCurrentState = E911States.STATE_E911_ALT_ADDRESS;
                            showE911Dialog(ALT_E911_ADDRESS_DIALOG);
                            break;
                        case SAVE_ANYWAY_E911_ADDRESS_DIALOG:
                            mCurrentState = E911States.STATE_E911_SAVE_ANYWAY;
                            showE911Dialog(SAVE_ANYWAY_E911_ADDRESS_DIALOG);
                            break;
                        case E911_ADDRESS_VALIDATION_FAILED_DIALOG:
                            mCurrentState = E911States.STATE_E911_ADDRESS_VALIDATION_FAILED;
                            showE911Dialog(E911_ADDRESS_VALIDATION_FAILED_DIALOG);
                            break;
                        default:
                            Log.d(TAG, "Invalid Dialog");
                            break;
                    }
                    }
                    break;

                case DISMISS_DIALOG:
                    int whichDialog = msg.arg1;
                    Log.d(TAG, "Dismiss Dialog:" + whichDialog);
                    switch (whichDialog) {
                        case PROGRESS_DIALOG:
                            dismissProgressDialog();
                            break;
                        case NW_ERROR_DIALOG://Nw error dialog can be finished only by user's click
                        default:
                            Log.d(TAG, "invalid Dialog");
                            break;
                    }
                    break;

                case SHOW_FRAGMENT:
                    int whichFragment = msg.arg1;
                    Log.d(TAG, "Show fragment:" + whichFragment);
                    switch (whichFragment) {
                        case E911_FRAGMENT:
                            mCurrentState = E911States.STATE_E911_USER_INTERACTION;
                            showE911Fragment((RequestResponse.Address) msg.obj);
                            break;
                        case TC_FRAGMENT:
                            mCurrentState = E911States.STATE_E911_TC;
                            showTcFragment((RequestResponse.Address) msg.obj);
                            break;
                        default:
                            Log.d(TAG, "invalid fragment");
                            break;
                    }
                    break;

                case SHOW_TOAST:
                    int whichToast = msg.arg1;
                    Log.d(TAG, "show toast:" + whichToast);
                    switch (whichToast) {
                        case WFC_ACTIVATED:
                            Toast.makeText(E911AddressActivity.this, R.string.wfc_turned_on,
                                    Toast.LENGTH_LONG).show();
                            break;
                        case E911_ADDRESS_UPDATED:
                            Toast.makeText(E911AddressActivity.this, R.string.e911_address_updated,
                                Toast.LENGTH_LONG).show();
                            break;
                        case E911_ADDRESS_NOT_UPDATED:
                            Toast.makeText(E911AddressActivity.this, R.string.unable_to_update_wfc,
                                Toast.LENGTH_LONG).show();
                            break;
                        default:
                            Log.d(TAG, "Invalid Toast");
                            break;
                    }
                    break;

                default:
                    Log.d(TAG, "Invalid action");
                    break;
            }
        }
    }

    /**
      * Class to create all Dialogs.
      */
    private class E911DialogFragment extends DialogFragment {
        private int mDialogId;
        private Dialog mDialog = null;

        private View.OnClickListener mClickListener = new View.OnClickListener() {
            public void onClick(View v) {
                Log.d(TAG, "onClick, v:" + v);
                Log.d(TAG, "findViewById(R.id.save):" + findViewById(R.id.save));
                if (v == mDialog.findViewById(R.id.save)) {
                    if (mDialogId == SAVE_ANYWAY_E911_ADDRESS_DIALOG) {
                        //saveanyway Dialog
                        doPartialValidationE911Add(mUserAddress);
                    } else {
                        // userenteredAddress & altaddress Dialogs
                        showSaveAnywayE911Dialog(mUserAddress);
                    }
                } else if (v == mDialog.findViewById(R.id.edit)) {
                    // userenteredAddress, altaddress, failedValidation Dialogs
                    showE911AddressFragment(mUserAddress);
                } else if (v == mDialog.findViewById(R.id.cancel)) {
                    //userenteredAddress, altaddress, saveanyway, failedValidation Dialogs
                    onDialogDismiss();
                }
                dismiss();
            }
        };

        public E911DialogFragment(int id) {
            mDialogId = id;
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            Log.d(TAG, "onCreateDialog, Dialog id:" + mDialogId);
            mDialog = null;
            switch (mDialogId) {
                case USER_ENTERED_E911_ADDRESS_DIALOG:
                    createUserEneteredE911AddressDialog();
                break;
                case ALT_E911_ADDRESS_DIALOG:
                    createAltE911AddressDialog();
                break;
                case SAVE_ANYWAY_E911_ADDRESS_DIALOG:
                    createSaveAnywayDialog();
                    break;
                case E911_ADDRESS_VALIDATION_FAILED_DIALOG:
                    createValidationFailedDialog();
                break;
                default:
                    Log.d(TAG, "invalid dialog");
                break;
            }
            return mDialog;
        }

        @Override
        public void onCancel(DialogInterface dialog) {
            super.onCancel(dialog);
            Log.d(TAG, "onCancel, reset state to STATE_E911_USER_INTERACTION");
            mCurrentState = E911States.STATE_E911_USER_INTERACTION;
        }

        private void createUserEneteredE911AddressDialog() {
            mDialog = new Dialog(getActivity());
            mDialog.setContentView(R.layout.user_address_only);
            mDialog.setCanceledOnTouchOutside(false);

            TextView userAddress = (TextView) mDialog.findViewById(R.id.user_address);
            fillUserAddress(userAddress);

            Button saveButton = (Button) mDialog.findViewById(R.id.save);
            Button editButton = (Button) mDialog.findViewById(R.id.edit);
            Button cancelButton = (Button) mDialog.findViewById(R.id.cancel);

            saveButton.setOnClickListener(mClickListener);
            editButton.setOnClickListener(mClickListener);
            cancelButton.setOnClickListener(mClickListener);
        }

        private void createAltE911AddressDialog() {
            mDialog = new Dialog(getActivity());
            mDialog.setContentView(R.layout.alternate_address);
            mDialog.setCanceledOnTouchOutside(false);

            RadioButton userAddress = (RadioButton) mDialog.findViewById(R.id.user_address_button);
            userAddress.setId(0);
            RadioGroup altAddressGroup = (RadioGroup) mDialog.findViewById(R.id.altAddressgroup);
            Button saveButton = (Button) mDialog.findViewById(R.id.save);
            Button okButton = (Button) mDialog.findViewById(R.id.ok);
            Button editButton = (Button) mDialog.findViewById(R.id.edit);
            Button cancelButton = (Button) mDialog.findViewById(R.id.cancel);

            altAddressGroup.setOnCheckedChangeListener(new RadioGroup.OnCheckedChangeListener() {
                @Override
                public void onCheckedChanged(RadioGroup group, int checkedId) {
                    Log.d(TAG, "mRadioGroupCheckedListener:checkedId:" + checkedId);
                    boolean enableOkButton = altAddressGroup.getCheckedRadioButtonId() > 0;
                    saveButton.setVisibility(checkedId > 0 ? View.GONE : View.VISIBLE);
                    okButton.setVisibility(checkedId > 0 ? View.VISIBLE : View.GONE);
                }
            });
            saveButton.setOnClickListener(mClickListener);
            okButton.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    // Radio button ids are in continuation but alt address is stored in array
                    // seperate from user address, so decrement by 1 to get alt address
                    RequestResponse.Address selectedDetails
                            = mAltAddress.get(altAddressGroup.getCheckedRadioButtonId() - 1);
                    saveE911Adress(selectedDetails);
                    dismiss();
                }
            });
            editButton.setOnClickListener(mClickListener);
            cancelButton.setOnClickListener(mClickListener);

            okButton.setVisibility(View.GONE);
            userAddress.setChecked(true);
            fillUserAddress(userAddress);
            fillAltAddresses(altAddressGroup);
        }

        private void createSaveAnywayDialog() {
            mDialog = new Dialog(getActivity());
            mDialog.setContentView(R.layout.confirmation_save_invalid_address);
            mDialog.setCanceledOnTouchOutside(false);

            TextView userAddress = (TextView) mDialog.findViewById(R.id.user_address);
            Button saveButton = (Button) mDialog.findViewById(R.id.save);
            Button cancelButton = (Button) mDialog.findViewById(R.id.cancel);

            saveButton.setOnClickListener(mClickListener);
            cancelButton.setOnClickListener(mClickListener);

            fillUserAddress(userAddress);
        }

        private void createValidationFailedDialog() {
            mDialog = new Dialog(getActivity());
            mDialog.setContentView(R.layout.address_partial_validation_fail);
            mDialog.setCanceledOnTouchOutside(false);

            TextView userAddress = (TextView) mDialog.findViewById(R.id.user_address);
            Button editButton = (Button) mDialog.findViewById(R.id.edit);
            Button cancelButton = (Button) mDialog.findViewById(R.id.cancel);

            editButton.setOnClickListener(mClickListener);
            cancelButton.setOnClickListener(mClickListener);

            fillUserAddress(userAddress);
        }

        private void fillUserAddress(TextView userAddress) {
            Log.d(TAG, "mUserAddress:" + mUserAddress);
            String address = mUserAddress.houseNumber + " " + mUserAddress.road
                    + "\n" + mUserAddress.city + ", " + mUserAddress.state;
            userAddress.setText(address);
        }

        private void fillUserAddress(RadioButton userAddress) {
            Log.d(TAG, "mUserAddress:" + mUserAddress);
            String address = mUserAddress.houseNumber + " " + mUserAddress.road
                    + "\n" + mUserAddress.city + ", " + mUserAddress.state;
            userAddress.setText(address);
        }

        private void fillAltAddresses(RadioGroup altAddressGroup) {
            int id = 1;
            Log.d(TAG, "address array:" + mAltAddress);
            for (RequestResponse.Address address: mAltAddress) {
                String addressText = address.houseNumber + " " + address.road
                        + "\n" + address.city + ", " + address.state;
                Log.d(TAG, "id:" + id + ", address:" + addressText);
                RadioButton b = new RadioButton(getActivity());
                b.setText(addressText);
                b.setId(id++);
                altAddressGroup.addView(b);
                Log.d(TAG, "child number:" + altAddressGroup.getChildCount());
            }
        }
    }

    /** AyncTask Class interacting with network.
     */
    private class NwInteractionTask extends AsyncTask<Void, Void, Void> {
        int mReqType;
        RequestResponse.Address mUserDetails;

        public NwInteractionTask(int reqType, RequestResponse.Address userDetails) {
            mReqType = reqType;
            mUserDetails = userDetails;
        }

        @Override
        protected void onPreExecute() {
            Log.d(TAG, "In preExecute");
            // Check whether a task is already executing
            if (mLoadingDialog != null && mLoadingDialog.isShowing()) {
                // Should never happen
                Log.d(TAG, "Another request in execution. drop this request.");
                cancel(true);
                return;
            }
            mHandler.sendMessage(mHandler.obtainMessage(SHOW_DIALOG, PROGRESS_DIALOG, -1));
        }

        @Override
        protected Void doInBackground(Void... params) {
            Log.d(TAG, "doInBackground, req type:" + mReqType);
            if (isCancelled()) {
                return null;
            }
            switch (mReqType) {
                case RequestResponse.QUERY_REQUEST:
                    mOptInServerApi.queryE911Address(mRespListener);
                    break;

                case RequestResponse.PARTIAL_VALIDATION_REQUEST:
                    mOptInServerApi.performPartialValidationAddress(mUserDetails, mRespListener);
                    break;

                case RequestResponse.ADD_REQUEST:
                    mOptInServerApi.addAddress(mUserDetails, mRespListener);
                    break;

                default:
                    Log.e(TAG, "Invalid request:" + mReqType);
                    break;
            }
            return null;
        }


        @Override
        protected void onPostExecute(Void param) {
            Log.d(TAG, "in onPostExecute");
            mHandler.sendMessage(mHandler.obtainMessage(DISMISS_DIALOG, PROGRESS_DIALOG, -1));
            mTask = null;
        }
    }
}
