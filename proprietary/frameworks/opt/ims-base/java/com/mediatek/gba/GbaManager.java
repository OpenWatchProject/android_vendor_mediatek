package com.mediatek.gba;

import android.content.Context;
import android.net.Network;
import android.os.Build;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;
import android.util.Log;

import com.mediatek.ims.internal.IMtkImsService;

/**
 * A class provides the GBA service APIs.
 *
 * @hide
 */
public final class GbaManager {
    private static final String TAG = "GbaManager";

    private static final String PROP_FORCE_DEBUG_KEY = "persist.vendor.log.tel_dbg";
    public static final boolean SENLOG = TextUtils.equals(Build.TYPE, "user");
    public static final boolean DBGLOG =
            !SENLOG || (SystemProperties.getInt(PROP_FORCE_DEBUG_KEY, 0) == 1);

    private final Context mContext;
    private static IGbaService mService;
    private static GbaManager mGbaManager = null;
    private static int mNetId;

    public static final int IMS_GBA_NONE     = 0;
    public static final int IMS_GBA_ME       = 1;
    public static final int IMS_GBA_U        = 2;

    public static final String IMS_GBA_KS_NAF       = "Ks_NAF";
    public static final String IMS_GBA_KS_EXT_NAF   = "Ks_ext_NAF";
    public static final String IMS_SERVICE = "ims";
    public static final String MTK_IMS_SERVICE = "mtkIms";

    public static final byte[] DEFAULT_UA_SECURITY_PROTOCOL_ID0 = new byte[]
                    {(byte) 0x01, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x00};
    public static final byte[] DEFAULT_UA_SECURITY_PROTOCOL_ID1 = new byte[]
                    {(byte) 0x01, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x01}; //MBMS
    public static final byte[] DEFAULT_UA_SECURITY_PROTOCOL_ID2 = new byte[]
                    {(byte) 0x01, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x02};
    public static final byte[] DEFAULT_UA_SECURITY_PROTOCOL_ID3 = new byte[]
                    {(byte) 0x01, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x03}; //MBMS

    private static final byte[] DEFAULT_UA_SECURITY_PROTOCOL_ID_HTTP =
        new byte[] {(byte) 0x01, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x02};
    private static final byte[] DEFAULT_UA_SECURITY_PROTOCOL_ID_TLS =
        new byte[] {(byte) 0x01, (byte) 0x00, (byte) 0x01, (byte) 0x00, (byte) 0x2F};

    /**
     * Helpers to get the default GbaManager.
     */
    public static GbaManager getDefaultGbaManager(Context context) {
        if (context == null) {
            throw new IllegalArgumentException("context cannot be null");
        }

        synchronized (GbaManager.class) {
            if (mGbaManager == null) {
                if (!supportMdAutoSetupIms()) {
                    IBinder b = ServiceManager.getService("GbaService");

                    if (b == null) {
                        Log.i("debug", "The binder is null");
                        return null;
                    }

                    mService = IGbaService.Stub.asInterface(b);
                }

                mGbaManager = new GbaManager(context);
            }

            return mGbaManager;
        }
    }

    GbaManager(Context context) {
        mContext = context;
    }

    /**
     * Check GBA is supported and support type or not.
     *
     * @return GBA Support Type
     */
    public int getGbaSupported() {
        try {
            return mService.getGbaSupported();
        } catch (RemoteException e) {
            return 0;
        }
    }

    /**
     * Check GBA is supported and support type or not for a particular subscription.
     *
     * @param subId subscription whose subscriber id is returned
     *
     * @return GBA Support Type
     */
    public int getGbaSupported(int subId) {
        try {
            return mService.getGbaSupported();
        } catch (RemoteException e) {
            return 0;
        }
    }

    /**
     * Check GBA NAFSession key is expired or not.
     *
     * @param nafFqdn The FQDN address of NAF server
     * @param nafSecurProtocolId The security protocol id of NAF server
     *
     * @return indicate key is expired or not
     */
    public boolean isGbaKeyExpired(String nafFqdn, byte[] nafSecurProtocolId) {
        try {
            return mService.isGbaKeyExpired(nafFqdn, nafSecurProtocolId);
        } catch (RemoteException e) {
            return true;
        }
    }

    /**
     * Check GBA NAFSession key is expired or not for a particular subscription.
     *
     * @param nafFqdn The FQDN address of NAF server
     * @param nafSecurProtocolId The security protocol id of NAF server
     * @param subId subscription whose subscriber id is returned
     *
     * @return indicate key is expired or not
     */
    public boolean isGbaKeyExpired(String nafFqdn, byte[] nafSecurProtocolId, int subId) {
        try {
            return mService.isGbaKeyExpiredForSubscriber(nafFqdn, nafSecurProtocolId, subId);
        } catch (RemoteException e) {
            return true;
        }
    }

    /**
     * Perform GBA bootstrap authentication.
     *
     * @param nafFqdn The FQDN address of NAF server
     * @param nafSecureProtocolId The security protocol id of NAF server
     * @param forceRun Indicate to force run GBA bootstrap procedure without
     *                 get NAS Session key from GBA cache
     *
     * @return GBA NAS Session Key
     */
    public NafSessionKey runGbaAuthentication(String nafFqdn, byte[] nafSecureProtocolId,
            boolean forceRun) {
        try {
            if (supportMdAutoSetupIms()) {
                int subId = SubscriptionManager.getDefaultSubscriptionId();
                int phoneId = SubscriptionManager.getPhoneId(subId);

                return runNativeGba(nafFqdn, nafSecureProtocolId, forceRun, mNetId, phoneId);
            } else {
                return mService.runGbaAuthentication(nafFqdn, nafSecureProtocolId, forceRun);
            }
        } catch (RemoteException e) {
            return null;
        }
    }

    /**
     * Perform GBA bootstrap authentication for a particular subscription.
     *
     * @param nafFqdn The FQDN address of NAF server
     * @param nafSecureProtocolId The security protocol id of NAF server
     * @param forceRun Indicate to force run GBA bootstrap procedure without
     *                 get NAS Session key from GBA cache
     * @param subId subscription whose subscriber id is returned
     *
     * @return GBA NAS Session Key
     */
    public NafSessionKey runGbaAuthentication(String nafFqdn, byte[] nafSecureProtocolId,
            boolean forceRun, int subId) {
        try {
            if (supportMdAutoSetupIms()) {
                int phoneId = SubscriptionManager.getPhoneId(subId);

                return runNativeGba(nafFqdn, nafSecureProtocolId, forceRun, mNetId, phoneId);
            } else {
                return mService.runGbaAuthenticationForSubscriber(nafFqdn,
                        nafSecureProtocolId, forceRun, subId);
            }
        } catch (RemoteException e) {
            return null;
        }
    }

    private NafSessionKey runNativeGba(String nafFqdn, byte[] nafSecureProtocolId,
            boolean forceRun, int netId, int phoneId) {
        NafSessionKey nafkey = null;

        // We only has one ImsService
        IBinder b = ServiceManager.getService(MTK_IMS_SERVICE);
        if (b == null) {
            Log.e(TAG, "Service is unavailable binder is null");
            return null;
        }

        IMtkImsService mMtkImsService = IMtkImsService.Stub.asInterface(b);
        if (mMtkImsService == null) {
            Log.e(TAG, "Service is unavailable mImsService is null");
            return null;
        }

        try {
             nafkey = mMtkImsService.runGbaAuthentication(nafFqdn, nafSecureProtocolId,
                     forceRun, netId, phoneId);
        } catch (RemoteException e) {
            Log.e(TAG, "RemotaException mImsService.runGbaAuthentication()");
            return null;
        }

        return nafkey;
    }

    /**
     * Get the NAF security protocol id.
     * If NAF client uses TLS connection, this API should be called after the TLS connection
     * is established. By this case, the cipher suite has been decided.
     * @param isTls Indicate to connect with server with TLS connection or not.
     *
     * @return return the NAF security protocol id.
     */

   public byte[] getNafSecureProtocolId(boolean isTls, String cipher) {
        if (DBGLOG) Log.d(TAG, "getNafSecureProtocolId isTls = " + isTls + ", cipher = " + cipher);
        byte[] uaId = DEFAULT_UA_SECURITY_PROTOCOL_ID_TLS;

        if (isTls) {
            if (cipher != null && cipher.length() > 0) {
                GbaCipherSuite cipherSuite = GbaCipherSuite.getByName(cipher);
                if (cipherSuite != null) {
                    if (DBGLOG) Log.d(TAG, "cipherSuite name = " + cipherSuite.getName());
                    byte[] cipherSuiteCode = cipherSuite.getCode();
                    if (DBGLOG) {
                        Log.d(TAG, "uaId = "+ String.format("0x%2x", cipherSuiteCode[0])
                                     + ", " + String.format("0x%2x", cipherSuiteCode[1]));
                    }
                    uaId[3] = cipherSuiteCode[0];
                    uaId[4] = cipherSuiteCode[1];
                }
            }
        } else {
            if (DBGLOG) Log.e(TAG, "Default cipherSuite");
            uaId = DEFAULT_UA_SECURITY_PROTOCOL_ID_HTTP;
        }

        return uaId;
   }

   /**
     * Configure dedicated network.
     *
     * @param network network that will be used to establish socket connection.
     *
     */
   public void setNetwork(Network network) {
       try {
           mService.setNetwork(network);
           mNetId = network.netId;
       } catch (RemoteException e) {
           Log.e(TAG, "remote expcetion for setNetwork");
       }
   }

   /**
     * Get cached GBA bootstrap authentication for a particular subscription.
     *
     * @param nafFqdn The FQDN address of NAF server
     * @param nafSecureProtocolId The security protocol id of NAF server
     * @param subId subscription whose subscriber id is returned
     *
     * @return GBA NAS Session Key
     */
   public NafSessionKey getCachedKey(String nafFqdn, byte[] nafSecureProtocolId, int subId) {
       try {
           return mService.getCachedKey(nafFqdn, nafSecureProtocolId, subId);
       } catch (RemoteException e) {
           Log.e(TAG, "remote expcetion for getCachedKey");
           return null;
       }
   }

    /**
      * Update cached GBA bootstrap authentication for a particular subscription.
      *
      * @param nafFqdn The FQDN address of NAF server
      * @param nafSecureProtocolId The security protocol id of NAF server
      * @param subId subscription whose subscriber id is returned
      * @param NafSessionKey
      *
      * @return GBA NAS Session Key
      */
    public void updateCachedKey(String nafFqdn, byte[] nafSecureProtocolId, int subId,
            NafSessionKey nafSessionKey) {
        try {
            mService.updateCachedKey(nafFqdn, nafSecureProtocolId, subId, nafSessionKey);
        } catch (RemoteException e) {
            Log.e(TAG, "remote expcetion for updateCachedKey");
        }
    }

    private static boolean supportMdAutoSetupIms() {
        boolean r = false;
        if (SystemProperties.get("ro.vendor.md_auto_setup_ims").equals("1")) {
            r = true;
        }
        return r;
    }
}
