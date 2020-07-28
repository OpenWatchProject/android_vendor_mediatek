package com.mediatek.digits;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Binder;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.os.Process;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.os.UserManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.Toast;

import com.mediatek.digits.DigitsController;
import com.mediatek.digits.DigitsDevice;
import com.mediatek.digits.DigitsLine;

public class DigitsServiceImpl extends IDigitsService.Stub {
    static protected final String TAG = "DigitsServiceImpl";

    // Declare action listener array index
    static final private int ACTION_LISTENER_INDEX_SUBSCRIBE = 0;
    static final private int ACTION_LISTENER_INDEX_ACTIVATE_LINE = 1;
    static final private int ACTION_LISTENER_INDEX_DEACTIVATE_LINE = 2;
    static final private int ACTION_LISTENER_INDEX_ADD_NEW_LINE = 3;
    static final private int ACTION_LISTENER_INDEX_SET_DEVICE_NAME = 4;
    static final private int ACTION_LISTENER_INDEX_SET_LINE_NAME = 5;
    static final private int ACTION_LISTENER_INDEX_SET_LINE_COLOR = 6;
    static final private int ACTION_LISTENER_INDEX_NOTIFY_LOGOUT = 7;
    static final private int ACTION_LISTENER_INDEX_UPDATE_LOCATION_AND_TC = 8;
    static final private int ACTION_LISTENER_INDEX_GET_DEVICE_CONFIG = 9;
    static final private int ACTION_LISTENER_INDEX_MAX = 10;

    // Listener callback
    protected static final int CMD_SERVICE_STATE_CHANGED = 1;
    protected static final int CMD_CALL_ANSWERED = 2;
    protected static final int CMD_REGISTERED_DEVICES_CHANGED = 3;
    protected static final int CMD_REGISTERED_MSISDN_CHANGED = 4;
    protected static final int CMD_DEVICE_NAME_CHANGED = 5;
    protected static final int CMD_LINE_NAME_CHANGED = 6;
    protected static final int CMD_NO_E911_ADDRESS_ON_FILE = 7;

    // Internal callback
    /**
     * @see EXTRA_OBJECT_ARRAY_DIGIT_DEVICE
     * @see EXTRA_OBJECT_ARRAY_DIGIT_LINE
     * @see EXTRA_OBJECT_DIGIT_PROFILE
     */
    protected static final int CMD_SUBSCRIBE_RESULT = 101;

    protected static final int CMD_LOGOUT_RESULT = 102;

    /**
     * @see #EXTRA_STRING_DEVICE_NAME
     */
    protected static final int CMD_SET_DEVICE_NAME_RESULT = 104;

    /**
     * @see #EXTRA_STRING_MSISDN
     * @see #EXTRA_STRING_LINE_NAME
     */
    protected static final int CMD_SET_LINE_NAME_RESULT = 105;

    /**
     * @see #EXTRA_STRING_MSISDN
     * @see #EXTRA_INT_LINE_COLOR
     */
    protected static final int CMD_SET_LINE_COLOR_RESULT = 106;

    /**
     * @see #EXTRA_OBJECT_ARRAY_DIGITS_LINE
     */
    protected static final int CMD_ACTIVATE_LINE_RESULT = 107;

    /**
     * @see #EXTRA_OBJECT_ARRAY_DIGITS_LINE
     */
    protected static final int CMD_DEACTIVATE_LINE_RESULT = 108;

    /**
     * @see #EXTRA_STRING_MSISDN
     */
    protected static final int CMD_ADD_NEW_LINE_RESULT = 109;

    /**
     * @see #EXTRA_STRING_SERVER_URL
     * @see #EXTRA_STRING_SERVER_DATA
     */
    protected static final int CMD_UPDATE_LOCATION_AND_TC_RESULT = 110;

    protected static final int CMD_GET_DEVICE_CONFIG_RESULT = 111;

    private Context mContext;

    private DigitsController mDigitsController;

    private HandlerThread mSrvHandlerThread;
    private Handler mHandler;

    private RemoteCallbackList<IDigitsEventListener> mClientListeners = new
    RemoteCallbackList<IDigitsEventListener>();

    private IDigitsActionListener[] mActionListeners = new IDigitsActionListener[ACTION_LISTENER_INDEX_MAX];

    // Constructor
    public DigitsServiceImpl(Context context) {
        Log.d(TAG, "DigitsServiceImpl Constructor");

        mContext = context;

        // Start handler thread for DigitsServiceImpl handler
        mSrvHandlerThread = new HandlerThread("DigitsServiceImpl");
        mSrvHandlerThread.start();
        initHandler();

        initController();

        registerForBroadcast();
    }

    // ////////////////////////////
    // exposed APIs
    // ////////////////////////////

    @Override
    synchronized public void registerListener(IDigitsEventListener listener) {
        Log.d(TAG, "registerListener(), pid: " + Binder.getCallingPid());

        enforceDigitsPermission();
        mClientListeners.register(listener);
    }

    @Override
    synchronized public void unregisterListener(IDigitsEventListener listener) {
        Log.d(TAG, "unregisterListener(), pid: " + Binder.getCallingPid());

        enforceDigitsPermission();
        mClientListeners.unregister(listener);
    }


    @Override
    synchronized public String getTmoidLoginURL() {
        enforceDigitsPermission();
        return (mDigitsController != null) ? mDigitsController.getTmoidLoginURL() : null;
    }

    @Override
    synchronized public String getMsisdnAuthURL() {
        enforceDigitsPermission();
        return (mDigitsController != null) ? mDigitsController.getMsisdnAuthURL() : null;
    }

    @Override
    synchronized public String getTmoidSignupURL() {
        enforceDigitsPermission();
        return (mDigitsController != null) ? mDigitsController.getTmoidSignupURL() : null;
    }

    @Override
    synchronized public void getDeviceConfig(IDigitsActionListener listener) {
        enforceDigitsPermission();

        if (mDigitsController == null ||
            mActionListeners[ACTION_LISTENER_INDEX_GET_DEVICE_CONFIG] != null) {

            reply(listener, DigitsConst.RESULT_FAIL_BUSY);
            return;
        }
        mActionListeners[ACTION_LISTENER_INDEX_GET_DEVICE_CONFIG] = listener;
        mDigitsController.getDeviceConfig();
    }

    @Override
    synchronized public void updateLocationAndTc(IDigitsActionListener listener) {
        enforceDigitsPermission();

        if (mDigitsController == null ||
            mActionListeners[ACTION_LISTENER_INDEX_UPDATE_LOCATION_AND_TC] != null) {

            reply(listener, DigitsConst.RESULT_FAIL_BUSY);
            return;
        }
        mActionListeners[ACTION_LISTENER_INDEX_UPDATE_LOCATION_AND_TC] = listener;
        mDigitsController.updateLocationAndTc();
    }

    @Override
    synchronized public void notifyAddressOnFile() {
        enforceDigitsPermission();

        if (mDigitsController != null) {
            mDigitsController.notifyAddressOnFile();
        }
    }

    @Override
    synchronized public void subscribe(String authCode, IDigitsActionListener listener) {
        enforceDigitsPermission();
        if (mDigitsController == null ||
            mActionListeners[ACTION_LISTENER_INDEX_SUBSCRIBE] != null) {

            reply(listener, DigitsConst.RESULT_FAIL_BUSY);
            return;
        } else if (authCode == null || authCode.length() == 0) {
            mActionListeners[ACTION_LISTENER_INDEX_SUBSCRIBE] = listener;

            // Notify caller
            mHandler.sendMessage(mHandler.obtainMessage(
                CMD_SUBSCRIBE_RESULT, DigitsConst.RESULT_FAIL_UNKNOWN, 0, null));
            return;
        }
        mActionListeners[ACTION_LISTENER_INDEX_SUBSCRIBE] = listener;
        mDigitsController.subscribe(authCode);
    }

    @Override
    synchronized public void logout(IDigitsActionListener listener) {
        enforceDigitsPermission();
        if (mDigitsController == null ||
            mActionListeners[ACTION_LISTENER_INDEX_NOTIFY_LOGOUT] != null) {

            reply(listener, DigitsConst.RESULT_FAIL_BUSY);
            return;
        }
        mActionListeners[ACTION_LISTENER_INDEX_NOTIFY_LOGOUT] = listener;
        mDigitsController.logout();
    }

    @Override
    synchronized public void setDeviceName(String name, IDigitsActionListener listener) {
        enforceDigitsPermission();
        if (mDigitsController == null ||
            mActionListeners[ACTION_LISTENER_INDEX_SET_DEVICE_NAME] != null) {

            reply(listener, DigitsConst.RESULT_FAIL_BUSY);
            return;
        } else if (name == null || name.length() == 0) {
            mActionListeners[ACTION_LISTENER_INDEX_SET_DEVICE_NAME] = listener;
            Bundle output = new Bundle();
            output.putString(DigitsConst.EXTRA_STRING_DEVICE_NAME, name);

            // Notify caller
            mHandler.sendMessage(mHandler.obtainMessage(
                CMD_SET_DEVICE_NAME_RESULT, DigitsConst.RESULT_FAIL_UNKNOWN, 0, output));
            return;
        }
        mActionListeners[ACTION_LISTENER_INDEX_SET_DEVICE_NAME] = listener;
        mDigitsController.setDeviceName(name);
    }

    @Override
    synchronized public void setLineName(String msisdn, String name, IDigitsActionListener listener) {
        enforceDigitsPermission();
        if (mDigitsController == null ||
            mActionListeners[ACTION_LISTENER_INDEX_SET_LINE_NAME] != null) {

            reply(listener, DigitsConst.RESULT_FAIL_BUSY);
            return;
        } else if (msisdn == null || msisdn.length() == 0 || name == null || name.length() == 0) {
            mActionListeners[ACTION_LISTENER_INDEX_SET_LINE_NAME] = listener;
            Bundle output = new Bundle();
            output.putString(DigitsConst.EXTRA_STRING_MSISDN, msisdn);
            output.putString(DigitsConst.EXTRA_STRING_LINE_NAME, name);

            // Notify caller
            mHandler.sendMessage(mHandler.obtainMessage(
                CMD_SET_LINE_NAME_RESULT, DigitsConst.RESULT_FAIL_UNKNOWN, 0, output));
            return;
        }
        mActionListeners[ACTION_LISTENER_INDEX_SET_LINE_NAME] = listener;

        mDigitsController.setLineName(msisdn, name);
    }

    @Override
    synchronized public void setLineColor(String msisdn, int color, IDigitsActionListener listener) {
        enforceDigitsPermission();

        /*
        if (mDigitsController == null ||
            mActionListeners[ACTION_LISTENER_INDEX_SET_LINE_COLOR] != null) {

            reply(listener, DigitsConst.RESULT_FAIL_BUSY);
            return;
        } else if (msisdn == null || msisdn.length() == 0 || color >= DigitsLine.LINE_COLOR_NUM) {
        */
            mActionListeners[ACTION_LISTENER_INDEX_SET_LINE_COLOR] = listener;
            Bundle output = new Bundle();
            output.putString(DigitsConst.EXTRA_STRING_MSISDN, msisdn);
            output.putInt(DigitsConst.EXTRA_INT_LINE_COLOR, color);

            // Notify caller we don't support the API now
            mHandler.sendMessage(mHandler.obtainMessage(
                CMD_SET_LINE_COLOR_RESULT, DigitsConst.RESULT_FAIL_UNKNOWN, 0, output));
            return;
        /*
        }

        mActionListeners[ACTION_LISTENER_INDEX_SET_LINE_COLOR] = listener;

        mDigitsController.setLineColor(msisdn, color);
        */
    }

    @Override
    synchronized public DigitsLine[] getRegisteredLine() {
        enforceDigitsPermission();
        return (mDigitsController != null) ?
            mDigitsController.getRegisteredLine() : new DigitsLine[0];

    }

    @Override
    synchronized public DigitsDevice[] getRegisteredDevice() {
        enforceDigitsPermission();
        return (mDigitsController != null) ?
            mDigitsController.getRegisteredDevice() : new DigitsDevice[0];
    }

    @Override
    synchronized public DigitsProfile getDigitsProfile() {
        enforceDigitsPermission();
        return (mDigitsController != null) ? mDigitsController.getConsumerProfile() : null;
    }

    @Override
    synchronized public void activateLine(String msisdn, IDigitsActionListener listener) {
        enforceDigitsPermission();
        if (mDigitsController == null ||
            mActionListeners[ACTION_LISTENER_INDEX_ACTIVATE_LINE] != null) {

            reply(listener, DigitsConst.RESULT_FAIL_BUSY);
            return;
        } else if (callIsActive()) {
            reply(listener, DigitsConst.RESULT_FAIL_ACTIVE_CALL);
            return;
        }
        mActionListeners[ACTION_LISTENER_INDEX_ACTIVATE_LINE] = listener;
        String[] msisdns = {msisdn};
        mDigitsController.activateLine(msisdns);
    }

    @Override
    synchronized public void deactivateLine(String msisdn, IDigitsActionListener listener) {
        enforceDigitsPermission();
        if (mDigitsController == null ||
            mActionListeners[ACTION_LISTENER_INDEX_DEACTIVATE_LINE] != null) {

            reply(listener, DigitsConst.RESULT_FAIL_BUSY);
            return;
        } else if (callIsActive()) {
            reply(listener, DigitsConst.RESULT_FAIL_ACTIVE_CALL);
            return;
        }
        mActionListeners[ACTION_LISTENER_INDEX_DEACTIVATE_LINE] = listener;
        String[] msisdns = {msisdn};
        mDigitsController.deactivateLine(msisdns);
    }

    @Override
    synchronized public void activateMultiLines(String[] msisdns, IDigitsActionListener listener) {
        enforceDigitsPermission();
        if (mDigitsController == null ||
            mActionListeners[ACTION_LISTENER_INDEX_ACTIVATE_LINE] != null) {

            reply(listener, DigitsConst.RESULT_FAIL_BUSY);
            return;
        } else if (callIsActive()) {
            reply(listener, DigitsConst.RESULT_FAIL_ACTIVE_CALL);
            return;
        }
        mActionListeners[ACTION_LISTENER_INDEX_ACTIVATE_LINE] = listener;
        mDigitsController.activateLine(msisdns);
    }

    @Override
    synchronized public void deactivateMultiLines(String[] msisdns, IDigitsActionListener listener) {
        enforceDigitsPermission();
        if (mDigitsController == null ||
            mActionListeners[ACTION_LISTENER_INDEX_DEACTIVATE_LINE] != null) {

            reply(listener, DigitsConst.RESULT_FAIL_BUSY);
            return;
        } else if (callIsActive()) {
            reply(listener, DigitsConst.RESULT_FAIL_ACTIVE_CALL);
            return;
        }

        mActionListeners[ACTION_LISTENER_INDEX_DEACTIVATE_LINE] = listener;
        mDigitsController.deactivateLine(msisdns);
    }

    @Override
    synchronized public void addNewLine(String msisdn, IDigitsActionListener listener) {
        enforceDigitsPermission();
        if (mDigitsController == null ||
            mActionListeners[ACTION_LISTENER_INDEX_ADD_NEW_LINE] != null) {

            reply(listener, DigitsConst.RESULT_FAIL_BUSY);
            return;
        } else if (callIsActive()) {
            reply(listener, DigitsConst.RESULT_FAIL_ACTIVE_CALL);
            return;
        }

        mActionListeners[ACTION_LISTENER_INDEX_ADD_NEW_LINE] = listener;
        mDigitsController.initMsisdnAuth(msisdn);
    }

    @Override
    synchronized public int getServiceState() {
        enforceDigitsPermission();
        return (mDigitsController != null) ? mDigitsController.getState() : DigitsConst.STATE_IDLE;
    }

    @Override
    synchronized public void setRefreshToken(String refreshToken) {
        enforceDigitsPermission();

        if (mDigitsController != null && refreshToken != null && refreshToken.length() != 0) {
            mDigitsController.setRefreshToken(refreshToken);
        }
    }

    // ////////////////////////////
    // Private APIs
    // ////////////////////////////

    private void registerForBroadcast() {
        Log.d(TAG, "registerForBroadcast()");

        IntentFilter filter = new IntentFilter();

        filter.addAction(Intent.ACTION_USER_UNLOCKED);

        mContext.registerReceiver(mReceiver, filter);
    }

    private void initController() {

        UserManager userManager = (UserManager) mContext.getSystemService(Context.USER_SERVICE);
        boolean unlocked = userManager.isUserUnlocked();

        Log.d(TAG, "initController(), unlock:" + unlocked +
            ", mDigitsController: " + mDigitsController);

        if (mDigitsController == null && unlocked) {

            mDigitsController = new DigitsController(mContext, mHandler);
        }
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {

            if (intent.getAction().equals(Intent.ACTION_USER_UNLOCKED)) {

                Log.d(TAG, "onReceive() ACTION_USER_UNLOCKED");

                // SharedPreference only can be accessed after USER UNLOCKED
                // so postpone to init controller
                initController();
            }
        }
    };

    private void initHandler() {
        /** Handler to handle internal message command, run on service handler thread */
        mHandler = new Handler(mSrvHandlerThread.getLooper()) {

            @Override
            public synchronized void handleMessage(Message msg) {

                Log.d(TAG, "handleMessage(): " + messageToString(msg) +
                      " result:" + msg.arg1 + ", bundle:" + msg.obj);

                switch (msg.what) {
                    case CMD_SERVICE_STATE_CHANGED:
                    case CMD_CALL_ANSWERED:
                    case CMD_REGISTERED_DEVICES_CHANGED:
                    case CMD_REGISTERED_MSISDN_CHANGED:
                    case CMD_DEVICE_NAME_CHANGED:
                    case CMD_LINE_NAME_CHANGED:
                    case CMD_NO_E911_ADDRESS_ON_FILE:

                        int i = mClientListeners.beginBroadcast();

                        while (i > 0) {
                            i--;
                            try {
                                mClientListeners.getBroadcastItem(i).onDigitsEvent(
                                    getEvent(msg.what), msg.arg1, (Bundle)msg.obj);
                            } catch (RemoteException e) {
                                Log.e(TAG, "onInitResult() RemoteException: " + e);
                            }
                        }
                        mClientListeners.finishBroadcast();

                        // Demo purpose only - Start (Customer should remove the session)
                        if (msg.what == CMD_CALL_ANSWERED) {
                            showCallAnsweredToast((Bundle)msg.obj);
                        }
                        // Demo purpose only - End

                        break;

                    case CMD_SUBSCRIBE_RESULT:
                    case CMD_LOGOUT_RESULT:
                    case CMD_SET_DEVICE_NAME_RESULT:
                    case CMD_SET_LINE_NAME_RESULT:
                    case CMD_SET_LINE_COLOR_RESULT:
                    case CMD_ACTIVATE_LINE_RESULT:
                    case CMD_DEACTIVATE_LINE_RESULT:
                    case CMD_ADD_NEW_LINE_RESULT:
                    case CMD_UPDATE_LOCATION_AND_TC_RESULT:
                    case CMD_GET_DEVICE_CONFIG_RESULT:
                        int index = getListenerIndex(msg.what);

                        if (mActionListeners[index] == null) {
                            Log.v(TAG, "mActionListeners is null");
                            break;
                        }
                        switch (msg.arg1) {
                            case DigitsConst.RESULT_SUCCEED:
                                try {
                                    mActionListeners[index].onSuccess((Bundle)msg.obj);
                                } catch (RemoteException e) {
                                    Log.e(TAG, "onSuccess() RemoteException: " + e);
                                }
                                break;
                            default:
                                try {
                                    mActionListeners[index].onFailure(msg.arg1, (Bundle)msg.obj);
                                } catch (RemoteException e) {
                                    Log.e(TAG, "onFailure() RemoteException: " + e);
                                }
                                break;
                        }
                        mActionListeners[index] = null;
                        break;
                    default:
                        Log.e(TAG, "Error: No handle the unknown event! Check why ?");
                }
            }

            private String messageToString(Message msg) {
                switch (msg.what) {

                    // Listener callback
                    case CMD_SERVICE_STATE_CHANGED:
                        return "CMD_SERVICE_STATE_CHANGED";
                    case CMD_CALL_ANSWERED:
                        return "CMD_CALL_ANSWERED";
                    case CMD_REGISTERED_DEVICES_CHANGED:
                        return "CMD_REGISTERED_DEVICES_CHANGED";
                    case CMD_REGISTERED_MSISDN_CHANGED:
                        return "CMD_REGISTERED_MSISDN_CHANGED";
                    case CMD_DEVICE_NAME_CHANGED:
                        return "CMD_DEVICE_NAME_CHANGED";
                    case CMD_LINE_NAME_CHANGED:
                        return "CMD_LINE_NAME_CHANGED";
                    case CMD_NO_E911_ADDRESS_ON_FILE:
                        return "CMD_NO_E911_ADDRESS_ON_FILE";

                    // Internal callback
                    case CMD_SUBSCRIBE_RESULT:
                        return "CMD_SUBSCRIBE_RESULT";
                    case CMD_LOGOUT_RESULT:
                        return "CMD_LOGOUT_RESULT";
                    case CMD_SET_DEVICE_NAME_RESULT:
                        return "CMD_SET_DEVICE_NAME_RESULT";
                    case CMD_SET_LINE_NAME_RESULT:
                        return "CMD_SET_LINE_NAME_RESULT";
                    case CMD_SET_LINE_COLOR_RESULT:
                        return "CMD_SET_LINE_COLOR_RESULT";
                    case CMD_ACTIVATE_LINE_RESULT:
                        return "CMD_ACTIVATE_LINE_RESULT";
                    case CMD_DEACTIVATE_LINE_RESULT:
                        return "CMD_DEACTIVATE_LINE_RESULT";
                    case CMD_ADD_NEW_LINE_RESULT:
                        return "CMD_ADD_NEW_LINE_RESULT";
                    case CMD_UPDATE_LOCATION_AND_TC_RESULT:
                        return "CMD_UPDATE_LOCATION_AND_TC_RESULT";
                    default:
                        return Integer.toString(msg.what);
                }
            }

        };
    }

    private void reply(IDigitsActionListener listener, int result) {
        try {
            listener.onFailure(result, null);
        } catch (RemoteException e) {
            Log.e(TAG, "onFailure() RemoteException: " + e);
        }
    }

    private void enforceDigitsPermission() {

        if (Binder.getCallingUid() == Process.SYSTEM_UID ||
            Binder.getCallingUid() == Process.PHONE_UID) {
            return;
        }

        try {
            mContext.enforceCallingPermission(
                "android.permission.DIGITS", "App doesn't have DIGITS permission");

        } catch (SecurityException e) {
            Log.d(TAG, "Security exception: No android.permission.DIGITS permission");

            throw e;
        }
    }

    private boolean callIsActive() {

        TelephonyManager tm = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);

        int callState = tm.getCallState();

        if (callState == TelephonyManager.CALL_STATE_OFFHOOK ||
            callState == TelephonyManager.CALL_STATE_RINGING) {

            Log.d(TAG, "callIsActive(), callState:" + callState);
            return true;
        }
        return false;
    }

    private int getListenerIndex(int event) {
        switch (event) {
            case CMD_SUBSCRIBE_RESULT:
                return ACTION_LISTENER_INDEX_SUBSCRIBE;

            case CMD_ACTIVATE_LINE_RESULT:
                return ACTION_LISTENER_INDEX_ACTIVATE_LINE;

            case CMD_DEACTIVATE_LINE_RESULT:
                return ACTION_LISTENER_INDEX_DEACTIVATE_LINE;

            case CMD_ADD_NEW_LINE_RESULT:
                return ACTION_LISTENER_INDEX_ADD_NEW_LINE;

            case CMD_SET_DEVICE_NAME_RESULT:
                return ACTION_LISTENER_INDEX_SET_DEVICE_NAME;

            case CMD_SET_LINE_NAME_RESULT:
                return ACTION_LISTENER_INDEX_SET_LINE_NAME;

            case CMD_SET_LINE_COLOR_RESULT:
                return ACTION_LISTENER_INDEX_SET_LINE_COLOR;

            case CMD_LOGOUT_RESULT:
                return ACTION_LISTENER_INDEX_NOTIFY_LOGOUT;

            case CMD_UPDATE_LOCATION_AND_TC_RESULT:
                return ACTION_LISTENER_INDEX_UPDATE_LOCATION_AND_TC;

            case CMD_GET_DEVICE_CONFIG_RESULT:
                return ACTION_LISTENER_INDEX_GET_DEVICE_CONFIG;
        }

        Log.e(TAG, "getListenerIndex(), unknown event:" + event + ", check what happens!");
        return -1;
    }

    private int getEvent(int cmd) {

        switch (cmd) {

            case CMD_SERVICE_STATE_CHANGED:
                return DigitsConst.EVENT_SERVICE_STATE_CHANGED;

            case CMD_CALL_ANSWERED:
                return DigitsConst.EVENT_CALL_ANSWERED;

            case CMD_REGISTERED_DEVICES_CHANGED:
                return DigitsConst.EVENT_REGISTERED_DEVICES_CHANGED;

            case CMD_REGISTERED_MSISDN_CHANGED:
                return DigitsConst.EVENT_REGISTERED_MSISDN_CHANGED;

            case CMD_DEVICE_NAME_CHANGED:
                return DigitsConst.EVENT_DEVICE_NAME_CHANGED;

            case CMD_LINE_NAME_CHANGED:
                return DigitsConst.EVENT_LINE_NAME_CHANGED;

            case CMD_NO_E911_ADDRESS_ON_FILE:
                return DigitsConst.EVENT_NO_E911_ADDRESS_ON_FILE;
        }

        Log.v(TAG, "getEvent(), unknown cmd:" + cmd + ", check what happens!");
        return -1;
    }

    private void showCallAnsweredToast(Bundle bundle) {

        String who = bundle.getString(DigitsConst.EXTRA_STRING_DEVICE_ID);

        DigitsUtil util = DigitsUtil.getInstance(mContext);

        // It's me picks up the phone
        if (util.sameDeviceId(util.getDeviceId(), who)) {

            Log.v(TAG, "showCallAnsweredToast(), it's me pick up the phone");

            return;
        }

        if (mDigitsController != null) {
            DigitsDevice[] devices = mDigitsController.getRegisteredDevice();

            if (devices != null) {

                for (DigitsDevice d : devices) {

                    String dId = d.getDeviceId();

                    // Replace who as device name if found
                    if (dId != null && util.sameDeviceId(dId, who)) {

                        who = d.getDeviceName();
                        break;
                    }
                }
            }
        }
        Log.v(TAG, "showCallAnsweredToast(), who:" + who);

        Toast.makeText(mContext, "1 answered call answered by " + who,
            Toast.LENGTH_LONG).show();

    }
}
