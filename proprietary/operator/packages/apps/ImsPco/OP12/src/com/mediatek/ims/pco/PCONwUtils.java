package com.mediatek.ims.pco;

import android.content.Context;
import android.content.Intent;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.net.Uri;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.provider.Settings;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.widget.Toast;

import com.mediatek.internal.telephony.IMtkTelephonyEx;
import com.mediatek.internal.telephony.selfactivation.ISelfActivation;

import static com.mediatek.ims.pco.PCOConfig.DEBUG;

final public class PCONwUtils {

    private static final String TAG = "PCONwUtils";

    private static Context sContext;

    public static void setAppContext(Context mContext) {
        sContext = mContext;
    }

    private static Context getAppContext() {
        return sContext;
    }

    /**
     *  To turn ON the radio signal.
     */
    public static void turnOnCellularRadio() {
        Log.d(TAG, "turnOnCellularRadio");
        IMtkTelephonyEx iMtkTelephonyEx = IMtkTelephonyEx.Stub.asInterface(
                ServiceManager.getService("phoneEx"));
        if (iMtkTelephonyEx == null) {
            Log.d(TAG, "Can not access iMtkTelephonyEx");
            return;
        }
        try {
            int subId = SubscriptionManager.getDefaultDataSubscriptionId();
            int selfActivateState = iMtkTelephonyEx.getSelfActivateState(subId);
            if (selfActivateState != ISelfActivation.STATE_ACTIVATED) {
                int duration = Toast.LENGTH_SHORT;
                Toast toast = Toast.makeText(getAppContext(), "Turning On Radio", duration);
                toast.show();
                Bundle extra = new Bundle();
                int operationType = ISelfActivation.ADD_DATA_AGREE;
                extra.putInt(ISelfActivation.EXTRA_KEY_ADD_DATA_OP, operationType);

                iMtkTelephonyEx.selfActivationAction(
                        ISelfActivation.ACTION_ADD_DATA_SERVICE, extra, subId);
                }

        } catch (RemoteException e) {
            log("iMtkTelephonyEx RemoteException");
        }

    }

    /**
     * To turn OFF the radio signal.
     */
    public static void turnOffCellularRadio() {

        log("turnOffCellularRadio");
        IMtkTelephonyEx iMtkTelephonyEx = IMtkTelephonyEx.Stub.asInterface(ServiceManager
                .getService("phoneEx"));
        if (iMtkTelephonyEx == null) {
            log("Can not access iMtkTelephonyEx");
            return;
        }
        try {
            int subId = SubscriptionManager.getDefaultDataSubscriptionId();
            int selfActivateState =  iMtkTelephonyEx.getSelfActivateState(subId);
//            if (selfActivateState == ISelfActivation.STATE_ACTIVATED) {
            int duration = Toast.LENGTH_SHORT;
            Toast toast = Toast.makeText(getAppContext(), "Turning Off Radio", duration);
            toast.show();
            Bundle extra = new Bundle();
            int operationType = ISelfActivation.ADD_DATA_DECLINE;
            extra.putInt(ISelfActivation.EXTRA_KEY_ADD_DATA_OP, operationType);
            iMtkTelephonyEx.selfActivationAction(ISelfActivation.ACTION_ADD_DATA_SERVICE,
                    extra, subId);
//            }
        } catch (RemoteException e) {
            log("iMtkTelephonyEx RemoteException");
        }

    }

    /**
     * To Turn off the BackGround data. FW API is not available.
     */
    public static void turnOFFBackGroundData() {

        log("turnOffBackGroundData");
        IMtkTelephonyEx iMtkTelephonyEx = IMtkTelephonyEx.Stub.asInterface(ServiceManager
                .getService("phoneEx"));
        if (iMtkTelephonyEx == null) {
            log("Can not access iMtkTelephonyEx");
            return;
        }
        int duration = Toast.LENGTH_SHORT;
        Toast toast = Toast.makeText(getAppContext(), "Turning Off Radio", duration);
        toast.show();
        try {
            int subId = SubscriptionManager.getDefaultDataSubscriptionId();
            int selfActivateState =  iMtkTelephonyEx.getSelfActivateState(subId);
            if (selfActivateState != 0) {
                Bundle extra = new Bundle();
                int operationType = ISelfActivation.ADD_DATA_DECLINE;
                extra.putInt(ISelfActivation.EXTRA_KEY_ADD_DATA_OP, operationType);
                iMtkTelephonyEx.selfActivationAction(ISelfActivation.ACTION_ADD_DATA_SERVICE,
                        extra, subId);
            }
        } catch (RemoteException e) {
            log("iMtkTelephonyEx RemoteException");
        }
    }

    /**
     * To convert Radio state.
     * @param stateInt : Radio signal state.
     * @return boolean : true if radio is ON or false if Radio is OFF.
     */
    public static boolean getRadioStateFromInt(int stateInt) {
       int state = -1;
       boolean ret = false;
        switch (stateInt) {
            case android.hardware.radio.V1_0.RadioState.OFF:
                state = TelephonyManager.RADIO_POWER_OFF;
                break;
            case android.hardware.radio.V1_0.RadioState.UNAVAILABLE:
                state = TelephonyManager.RADIO_POWER_UNAVAILABLE;
                break;
            case android.hardware.radio.V1_0.RadioState.ON:
                state = TelephonyManager.RADIO_POWER_ON;
                ret = true;
                break;
            default:
                throw new RuntimeException("Unrecognized RadioState: " + stateInt);
        }
        return ret;
    }

    /**
     * Launches browser with Sign-up or operator/registration portal.
     * @param url: Sign UP portal.
     */
    public static void launchBrowserURL(String url, String mIMEI, String mICCID) {
        Context mContext = getAppContext();
        int duration = Toast.LENGTH_SHORT;
        Toast toast = Toast.makeText(mContext, "Opening Portal", duration);
        toast.show();
        log("url=" + url + ", imei=" + mIMEI + "ICCID=" + mICCID);
        String mfinalURL = url + "HTTPS POST parameters: iccid=" + mICCID + "&IMEI=" +
                mIMEI;
        Intent browserIntent = new Intent(Intent.ACTION_VIEW,
                       Uri.parse(mfinalURL));
        log("mfinalURL=" + mfinalURL);
        browserIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        mContext.startActivity(browserIntent);

    }

    /**
     * To get Internet status.
     * @return boolean: true if WIfi/PS data is ON  or false.
     */
    public static boolean isInternetConnected() {

    Context mContext = getAppContext();
    ConnectivityManager connectivityMgr =
            (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
    NetworkInfo wifi = connectivityMgr.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
    NetworkInfo mobile = connectivityMgr.getNetworkInfo(ConnectivityManager.TYPE_MOBILE);
    // Check if wifi or mobile network is available or not. If any of them is
    // available or connected then it will return true, otherwise false;

    if (wifi != null) {
        if (wifi.isConnected()) {
            log("isInternetConnected: Wifi is connected");
            return true;
        }
    }
    if (mobile != null) {
        if (mobile.isConnected()) {
            log("isInternetConnected: Mobile data is connected");
            return true;
        }
    }
    log("isInternetConnected: Internet not Connected");
    return false;
  }

    /**
     * To get Internet status.
     * @return boolean: true if WIfi/PS data is ON  or false.
     */
    public static boolean isOnline() {
        Context mContext = getAppContext();
        ConnectivityManager connMgr =
                (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo networkInfo = connMgr.getActiveNetworkInfo();
        return (networkInfo != null && networkInfo.isConnected());
    }

    /**
     * To get Mobile data status.
     * @return boolean: true if PS data is ON  or false.
     */
    public static boolean isMobileDataConnected() {
        Context mContext = getAppContext();
        boolean onlineStatus = false;
        ConnectivityManager connMgr =
                (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo mobile = connMgr.getNetworkInfo(ConnectivityManager.TYPE_MOBILE);
        onlineStatus = (mobile != null && mobile.isConnected());
        log("isMobileDataConnected():" +  onlineStatus);
        return onlineStatus;
    }

    /**
     *  To get WIFI status.
      * @return  boolean: true if WIFI is ON  or false.
     */
    public static boolean isWIFIConnected() {
        Context mContext = getAppContext();
        boolean onlineStatus = false;
        ConnectivityManager connectivityMgr =
                (ConnectivityManager) mContext.getSystemService(Context.CONNECTIVITY_SERVICE);
        NetworkInfo wifi = connectivityMgr.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
        onlineStatus = (wifi != null && wifi.isConnected());
        log("isWIFIConnected():" +  onlineStatus);
        return onlineStatus;
    }

    /**
     * To get Airplane status.
     * @return boolean: returns turn if Airplane mode is ON else false.
     */
    public static boolean isAirplaneModeOn() {
        Context mContext = getAppContext();
        return Settings.System.getInt(mContext.getContentResolver(),
                Settings.System.AIRPLANE_MODE_ON, 0) != 0;
    }

    /**
     * Calls dialer app with 911 number.
     */
    public static void call911() {
        Context mContext = getAppContext();
        int duration = Toast.LENGTH_SHORT;
        Toast toast = Toast.makeText(mContext, "Calling 911", duration);
        toast.show();

        Uri callUri = Uri.parse("tel://911");
        Intent callIntent = new Intent(Intent.ACTION_CALL, callUri);
        callIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_NO_USER_ACTION);
        mContext.startActivity(callIntent);
    }

    /**
     * Calls Wireless connectivity settings.
     */
    public static void enableWirelessConnectivity() {
        Context mContext = getAppContext();
        Intent intent = new Intent(Settings.ACTION_WIRELESS_SETTINGS);
        intent.setFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP 
                | Intent.FLAG_ACTIVITY_NEW_TASK 
                | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        mContext.startActivity(intent);
       // new PCOWaitForWifiTask(mContext).execute();
    }

    /**
     * Calls WI-IF connectivity settings.
     */
    public static void enableWifi()  {
        Context mContext = getAppContext();
        Intent intent = new Intent(Settings.ACTION_WIFI_SETTINGS);        
        intent.setFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP 
                | Intent.FLAG_ACTIVITY_NEW_TASK 
                | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        mContext.startActivity(intent);
        //new PCOWaitForWifiTask(context).execute();
    }

    private static void log(String s) {
        if (DEBUG) {
            Log.d(TAG, s);
        }
    }

}
