package com.mediatek.cta;

//import com.android.okhttp.OkHttpClient;
//import com.android.okhttp.internalandroidapi.OkHttpClient.PermissionCheckListener;
//import com.android.okhttp.internalandroidapi.OkHttpClient.ServiceType;

import android.util.Log;

//import java.net.Socket;
//import java.net.Socket.SocketPortCheckListener;

/**
 * CTA implementation for send MMS and send Email permission check.
 * @hide
 */
final public class CtaHttp {
    private static final String TAG = "Cta_CtaHttp";
    private static CtaHttp sInstance = null;

    /**
     * Return a CtaHttp instance
     */
/*    public static CtaHttp getInstance() {
       if (sInstance == null) {
           sInstance = new CtaHttp();
        }
        return sInstance;
    }

    public void init() {
        OkHttpClient.registerPermissionCheckListener(mMmsListener, ServiceType.MMS);
        OkHttpClient.registerPermissionCheckListener(mEmailListener, ServiceType.EMAIL);
        Socket.registerSocketPortCheckListener(mSocketListener);
    }

    // OkHTTP
    PermissionCheckListener mMmsListener = new PermissionCheckListener() {
        @Override
        public boolean isPermissionGrant() {
            if (!CtaUtils.enforceCheckPermission("com.mediatek.permission.CTA_SEND_MMS",
                    "Send MMS")) {
                Log.d(TAG, "Fail to send due to user permission");
                return false;
            }
            return true;
        }
    };

    PermissionCheckListener mEmailListener = new PermissionCheckListener() {
        @Override
        public boolean isPermissionGrant() {
            if (!CtaUtils.enforceCheckPermission("com.mediatek.permission.CTA_SEND_EMAIL",
                    "Send emails")) {
                Log.d(TAG, "Fail to send due to user permission");
                return false;
            }
            return true;
        }
    };

    // Socket
    SocketPortCheckListener mSocketListener = new SocketPortCheckListener() {
        @Override
        public boolean isServerPortDenied(int port) {
            if (port == 25 || port == 465 || port == 587) {
                Log.i(TAG, "port:" + port);
                if (!CtaUtils.enforceCheckPermission("com.mediatek.permission.CTA_SEND_EMAIL",
                        "Send emails")) {
                    Log.d(TAG, "Fail to send due to user permission");
                    return true;
                }
            }
            return false;
        }
    };*/

}
