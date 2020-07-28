package com.mediatek.digits;

import android.os.Bundle;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;
import android.util.Log;

import java.util.ArrayList;


public final class DigitsManager {

    private static final String TAG = "DigitsManager";

    /** The client context (e.g. activity). */
    private final Context mContext;

    /** The backend system. */
    private volatile IDigitsService mDigitsService;

    private DigitsEventListener mDigitsEventListener;

    /**
     * Class for interacting with the main interface of the backend.
     */
    private ServiceConnection mConnection;

    private ArrayList<EventListener> mEventListeners = new ArrayList<EventListener>();

    /** Interface for callback invocation on an application action */
    public abstract class ActionListener extends IDigitsActionListener.Stub {

        /** The operation succeeded */
        abstract public void onSuccess(Bundle extras);

        /**
         * The operation failed
         * @param reason The reason for failure
         */
        abstract public void onFailure(int reason, Bundle extras);
    }

    /** Interface for callback invocation on an application action */
    public interface EventListener {

        public void onEvent(int event, int result, Bundle extras);
    }

    public class DigitsEventListener extends IDigitsEventListener.Stub {
        @Override
        synchronized public void onDigitsEvent(int event, int result, Bundle extras) {
            Log.v(TAG, "onDigitsEvent(), event:" + DigitsConst.eventToString(event) +
                  ", result: " + result);

            switch (event) {
                case DigitsConst.EVENT_SERVICE_STATE_CHANGED:
                case DigitsConst.EVENT_CALL_ANSWERED:
                case DigitsConst.EVENT_REGISTERED_DEVICES_CHANGED:
                case DigitsConst.EVENT_REGISTERED_MSISDN_CHANGED:
                case DigitsConst.EVENT_DEVICE_NAME_CHANGED:
                case DigitsConst.EVENT_LINE_NAME_CHANGED:
                case DigitsConst.EVENT_NO_E911_ADDRESS_ON_FILE:

                    for (EventListener l : mEventListeners) {
                        l.onEvent(event, result, extras);
                    }
                    break;
                default:
                    Log.e(TAG, "Error: No handle the unknown event! Check why ?");
            }
        }
    }

    public DigitsManager(Context context) {
        if (context == null) {
            throw new NullPointerException("context must not be null");
        }

        mContext = context;

        mConnection = new ServiceConnection() {
            public synchronized void onServiceConnected(
                ComponentName className, IBinder service) {
                mDigitsService = IDigitsService.Stub.asInterface(service);
                Log.v(TAG, "Service onServiceConnection");
                mDigitsEventListener = new DigitsEventListener();
                try {
                    mDigitsService.registerListener(mDigitsEventListener);
                } catch (RemoteException ex) {
                    Log.v(TAG, "registerListener failed");
                }

                onServiceConnectionResult(true);

            }
            public void onServiceDisconnected(ComponentName className) {
                mDigitsService = null;
                Log.v(TAG, "Service onServiceDisconnected");

                onServiceConnectionResult(false);
            }
        };

        Intent intent = new Intent("com.mediatek.digits.BIND_SERVICE");
        //intent.setPackage("com.mediatek.digits");
        intent.setClassName("com.mediatek.digits", "com.mediatek.digits.DigitsService");
        boolean bindingSuccessful = mContext.bindService(intent, mConnection,
                                    Context.BIND_AUTO_CREATE);
        Log.v(TAG, "bindingSuccessful: " + bindingSuccessful);
    }

    public void shutdown() {

        if (mDigitsService != null && mDigitsEventListener != null) {
            try {
                mDigitsService.unregisterListener(mDigitsEventListener);
            } catch (RemoteException e) {

            }
        }

        mContext.unbindService(mConnection);

        mDigitsService = null;
        mConnection = null;
    }

    synchronized public void registerEventListener(EventListener listener) {

        if (!mEventListeners.contains(listener)) {
            mEventListeners.add(listener);
        }
    }

    synchronized public void unregisterEventListener(EventListener listener) {

        if (mEventListeners.contains(listener)) {
            mEventListeners.remove(listener);
        }
    }

    /**
       * Tells whether or not the service is connected.
       *
       * @return <code>true</code> if the service is connected.
       */
    public boolean isConnected() {
        return mDigitsService != null;
    }


    public String getTmoidLoginURL() {
        try {
            if (mDigitsService == null) {
                Log.v(TAG, "getTmoidLoginURL(), service isn't connected");
                return null;
            }
            return mDigitsService.getTmoidLoginURL();
        } catch (RemoteException ex) {
            return null;
        }
    }

    public String getMsisdnAuthURL() {
        try {
            if (mDigitsService == null) {
                Log.v(TAG, "getMsisdnAuthURL(), service isn't connected");
                return null;
            }
            return mDigitsService.getMsisdnAuthURL();
        } catch (RemoteException ex) {
            return null;
        }
    }

    public String getTmoidSignupURL() {
        try {
            if (mDigitsService == null) {
                Log.v(TAG, "getTmoidSignupURL(), service isn't connected");
                return null;
            }
            return mDigitsService.getTmoidSignupURL();
        } catch (RemoteException ex) {
            return null;
        }
    }

    public void notifyAddressOnFile() {
        try {
            if (mDigitsService == null) {
                Log.v(TAG, "notifyAddressOnFile(), service isn't connected");
                return;
            }
            mDigitsService.notifyAddressOnFile();
        } catch (RemoteException ex) {
        }
    }

    synchronized public void getDeviceConfig(ActionListener listener) {
        try {
            if (mDigitsService == null) {
                listener.onFailure(DigitsConst.RESULT_FAIL_SERVICE_NOT_READY, null);
            } else {
                mDigitsService.getDeviceConfig(listener);
            }
        } catch (RemoteException ex) {
            listener.onFailure(DigitsConst.RESULT_FAIL_UNKNOWN, null);
        }
    }

    synchronized public void updateLocationAndTc(ActionListener listener) {
        try {
            if (mDigitsService == null) {
                listener.onFailure(DigitsConst.RESULT_FAIL_SERVICE_NOT_READY, null);
            } else {
                mDigitsService.updateLocationAndTc(listener);
            }
        } catch (RemoteException ex) {
            listener.onFailure(DigitsConst.RESULT_FAIL_UNKNOWN, null);
        }
    }

    synchronized public void subscribe(String authCode, ActionListener listener) {
        try {
            if (mDigitsService == null) {
                listener.onFailure(DigitsConst.RESULT_FAIL_SERVICE_NOT_READY, null);
            } else {
                mDigitsService.subscribe(authCode, listener);
            }
        } catch (RemoteException ex) {
            listener.onFailure(DigitsConst.RESULT_FAIL_UNKNOWN, null);
        }
    }

    synchronized public boolean logout(ActionListener listener) {
        try {
            if (mDigitsService == null) {
                listener.onFailure(DigitsConst.RESULT_FAIL_SERVICE_NOT_READY, null);
            } else {
                mDigitsService.logout(listener);
            }
        } catch (RemoteException ex) {
            return false;
        }
        return true;
    }

    synchronized public void setDeviceName(String name, ActionListener listener) {
        try {
            if (mDigitsService == null) {
                listener.onFailure(DigitsConst.RESULT_FAIL_SERVICE_NOT_READY, null);
            } else {
                mDigitsService.setDeviceName(name, listener);
            }
        } catch (RemoteException ex) {
            listener.onFailure(DigitsConst.RESULT_FAIL_UNKNOWN, null);
        }
    }

    synchronized public void setLineName(String msisdn, String name, ActionListener listener) {
        try {
            if (mDigitsService == null) {
                listener.onFailure(DigitsConst.RESULT_FAIL_SERVICE_NOT_READY, null);
            } else {
                mDigitsService.setLineName(msisdn, name, listener);
            }
        } catch (RemoteException ex) {
            listener.onFailure(DigitsConst.RESULT_FAIL_UNKNOWN, null);
        }
    }

    synchronized public void setLineColor(String msisdn, int color, ActionListener listener) {
        try {
            if (mDigitsService == null) {
                listener.onFailure(DigitsConst.RESULT_FAIL_SERVICE_NOT_READY, null);
            } else {
                mDigitsService.setLineColor(msisdn, color, listener);
            }
        } catch (RemoteException ex) {
            listener.onFailure(DigitsConst.RESULT_FAIL_UNKNOWN, null);
        }
    }

    public DigitsLine[] getRegisteredLine() {
        try {
            if (mDigitsService == null) {
                Log.v(TAG, "getRegisteredLine(), service isn't connected");
                return null;
            }
            return mDigitsService.getRegisteredLine();
        } catch (RemoteException ex) {
            return null;
        }
    }

    public DigitsDevice[] getRegisteredDevice() {
        try {
            if (mDigitsService == null) {
                Log.v(TAG, "getRegisteredDevice(), service isn't connected");
                return null;
            }
            return mDigitsService.getRegisteredDevice();
        } catch (RemoteException ex) {
            return null;
        }
    }

    public DigitsProfile getDigitsProfile() {
        try {
            if (mDigitsService == null) {
                Log.v(TAG, "getDigitsProfile(), service isn't connected");
                return null;
            }
            return mDigitsService.getDigitsProfile();
        } catch (RemoteException ex) {
            return null;
        }
    }

    synchronized public void activateLine(String msisdn, ActionListener listener) {
        try {
            if (mDigitsService == null) {
                listener.onFailure(DigitsConst.RESULT_FAIL_SERVICE_NOT_READY, null);
            } else {
                mDigitsService.activateLine(msisdn, listener);
            }
        } catch (RemoteException ex) {
            listener.onFailure(DigitsConst.RESULT_FAIL_UNKNOWN, null);
        }
    }

    synchronized public void deactivateLine(String msisdn, ActionListener listener) {
        try {
            if (mDigitsService == null) {
                listener.onFailure(DigitsConst.RESULT_FAIL_SERVICE_NOT_READY, null);
            } else {
                mDigitsService.deactivateLine(msisdn, listener);
            }
        } catch (RemoteException ex) {
            listener.onFailure(DigitsConst.RESULT_FAIL_UNKNOWN, null);

        }
    }

    synchronized public void activateMultiLines(String[] msisdns, ActionListener listener) {
        try {
            if (mDigitsService == null) {
                listener.onFailure(DigitsConst.RESULT_FAIL_SERVICE_NOT_READY, null);
            } else {
                mDigitsService.activateMultiLines(msisdns, listener);
            }
        } catch (RemoteException ex) {
            listener.onFailure(DigitsConst.RESULT_FAIL_UNKNOWN, null);
        }
    }

    synchronized public void deactivateMultiLines(String[] msisdns, ActionListener listener) {
        try {
            if (mDigitsService == null) {
                listener.onFailure(DigitsConst.RESULT_FAIL_SERVICE_NOT_READY, null);
            } else {
                mDigitsService.deactivateMultiLines(msisdns, listener);
            }
        } catch (RemoteException ex) {
            listener.onFailure(DigitsConst.RESULT_FAIL_UNKNOWN, null);

        }
    }

    synchronized public void addNewLineAndLogin(String msisdn, ActionListener listener) {
    }

    synchronized public void addNewLine(String msisdn, ActionListener listener) {
        try {
            if (mDigitsService == null) {
                listener.onFailure(DigitsConst.RESULT_FAIL_SERVICE_NOT_READY, null);
            } else {
                mDigitsService.addNewLine(msisdn, listener);
            }
        } catch (RemoteException ex) {
            listener.onFailure(DigitsConst.RESULT_FAIL_UNKNOWN, null);
        }
    }

    synchronized public void addNewLineAndUpdateTmoId (String msisdn, String email, String password,
            ActionListener listener) {
    }

    synchronized public void cancelPendingLine (String msisdn, ActionListener listener) {
    }

    synchronized public int getServiceState() {
        try {
            if (mDigitsService != null) {
                return mDigitsService.getServiceState();
            }
        } catch (RemoteException ex) {

        }
        return DigitsConst.STATE_UNKNOWN;
    }

    /*
     * The API is for AT (Auto Test)
     */
    synchronized public void setRefreshToken(String refreshToken) {
        try {
            if (mDigitsService == null) {
                Log.v(TAG, "setRefreshToken(), service isn't connected");
                return;
            }
            mDigitsService.setRefreshToken(refreshToken);
        } catch (RemoteException ex) {
        }
    }

    private void onServiceConnectionResult(boolean connected) {
        int result = connected ? DigitsConst.RESULT_SUCCEED : DigitsConst.RESULT_FAIL_SERVICE_NOT_READY;

        for (EventListener l : mEventListeners) {
            l.onEvent(DigitsConst.EVENT_SERVICE_CONNECTION_RESULT, result, null);
        }
    }
}
