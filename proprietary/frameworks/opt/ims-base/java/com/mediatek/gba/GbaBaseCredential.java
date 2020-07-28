package com.mediatek.gba;

import android.content.Context;
import android.net.Network;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.telephony.SubscriptionManager;
import android.util.Log;

/**
 * HTTP Authenticator for GBA procedure.
 * It is based class.
 *
 * @hide
 */
public abstract class GbaBaseCredential {
    private final static String TAG = "GbaBaseCredential";

    static final String NAFFQDN_PREFER = "original.naf.prefer";
    static final byte[] DEFAULT_UA_SECURITY_PROTOCOL_ID_HTTP =
        new byte[] {(byte) 0x01, (byte) 0x00, (byte) 0x00, (byte) 0x00, (byte) 0x02};
    static final byte[] DEFAULT_UA_SECURITY_PROTOCOL_ID_TLS =
        new byte[] {(byte) 0x01, (byte) 0x00, (byte) 0x01, (byte) 0x00, (byte) 0x2F};
    final protected static char[] HEX_ARRAY = "0123456789abcdef".toCharArray();

    protected Network mNetwork;
    protected int mSubId;
    protected boolean mIsTlsEnabled;
    protected String mNafAddress;

    protected static IGbaService sService;

    /**
      * Empty construciton function.
      *
      */
    GbaBaseCredential() {

    }

    /**
      * Construciton function with initalization parameters.
      *
      */
    GbaBaseCredential(Context context, String nafAddress, int subId) {
        super();
        mSubId = subId;

        if (nafAddress.charAt(nafAddress.length() - 1) == '/') {
            nafAddress = nafAddress.substring(0, nafAddress.length() - 1);
        }

        mIsTlsEnabled = true;
        mNafAddress = nafAddress.toLowerCase();

        if (mNafAddress.indexOf("http://") != -1) {
            mNafAddress = nafAddress.substring(7);
            mIsTlsEnabled = false;
        } else if (mNafAddress.indexOf("https://") != -1) {
            mNafAddress = nafAddress.substring(8);
            mIsTlsEnabled = true;
        }

        Log.d(TAG, "nafAddress:" + mNafAddress);
    }

    /**
      * Tell GbaCredential the connection is TLS or not.
      *
      * @param tlsEnabled indicate the connection is over TLS or not.
      *
      */
    public void setTlsEnabled(boolean tlsEnabled) {
        mIsTlsEnabled = tlsEnabled;
    }

    /**
      * Configure which subscription to use in GBA procedure.
      *
      * @param subId indicate the subscription id.
      *
      */
    public void setSubId(int subId) {
        mSubId = subId;
    }

    /**
      * Configure dedicated network.
      *
      * @param network network that will be used to establish socket connection.
      *
      */
    public void setNetwork(Network network) {
        if (network != null) {
            Log.i(TAG, "GBA dedicated network netid:" + network);
            mNetwork = network;
        }
    }

    /**
      * Get session key for NAF server by GBA procedure.
      *
      @return NafSessionKey: the session key of NAF server.
      */
    public NafSessionKey getNafSessionKey() {
        NafSessionKey nafSessionKey = null;

        try {
            IBinder b = ServiceManager.getService("GbaService");

            if (b == null) {
                Log.i("debug", "The binder is null");
                return null;
            }

            sService = IGbaService.Stub.asInterface(b);
        } catch (NullPointerException e) {
            e.printStackTrace();
        }

        try {
            byte[] uaId = DEFAULT_UA_SECURITY_PROTOCOL_ID_TLS;

            if (mIsTlsEnabled) {
                String gbaStr = System.getProperty("gba.ciper.suite", "");

                if (gbaStr.length() > 0) {
                    GbaCipherSuite cipherSuite = GbaCipherSuite.getByName(gbaStr);

                    if (cipherSuite != null) {
                        byte[] cipherSuiteCode = cipherSuite.getCode();
                        uaId[3] = cipherSuiteCode[0];
                        uaId[4] = cipherSuiteCode[1];
                    }
                }
            } else {
                uaId = DEFAULT_UA_SECURITY_PROTOCOL_ID_HTTP;
            }

            if (mNetwork != null) {
                sService.setNetwork(mNetwork);
            }

            String realm = System.getProperty("digest.realm", "");
            String originalNafPrefer = System.getProperty(NAFFQDN_PREFER, "");
            Log.i(TAG, "realm:" + realm);
            Log.i(TAG, "NAFFQDN_PREFER:" + originalNafPrefer);
            if (realm.length() > 0) {
                if (originalNafPrefer.length() == 0) {
                    String[] segments = realm.split(";");
                    mNafAddress = segments[0].substring(segments[0].indexOf("@") + 1);
                }
                Log.i(TAG, "NAF FQDN:" + mNafAddress);
            } else {
                return null;
            }

            boolean forceRun = false;
            Log.d(TAG, "gba.auth: " + System.getProperty("gba.auth"));
            if ("401".equals(System.getProperty("gba.auth"))) {
                forceRun = true;
                System.setProperty("gba.auth", "");
            }
            Log.d(TAG, "forceRun: " + forceRun);
            if (SubscriptionManager.INVALID_SUBSCRIPTION_ID == mSubId) {
                nafSessionKey = sService.runGbaAuthentication(mNafAddress,
                                uaId, forceRun);
            } else {
                nafSessionKey = sService.runGbaAuthenticationForSubscriber(mNafAddress,
                                uaId, forceRun, mSubId);
            }

            if (nafSessionKey != null && (nafSessionKey.getException() != null) &&
                    (nafSessionKey.getException() instanceof IllegalStateException)) {
                String msg = ((IllegalStateException) nafSessionKey.getException())
                        .getMessage();

                if ("HTTP 403 Forbidden".equals(msg)) {
                    Log.i(TAG, "GBA hit 403");
                    System.setProperty("gba.auth", "403");
                } else if ("HTTP 400 Bad Request".equals(msg)) {
                    Log.i(TAG, "GBA hit 400");
                }
            }
        } catch (RemoteException re) {
            re.printStackTrace();
        }

        return nafSessionKey;
    }
}
