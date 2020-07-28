package com.mediatek.gba.header;

import android.util.Log;

import java.util.HashMap;
import java.util.Map;

/**
 * implementation for WwwAuthHeader.
 *
 * @hide
 */
public class WwwAuthHeader {
    private static final String TAG = "WwwAuthenticateHeader";

    private static final String DIGEST_SCHEME = "Digest";
    private static final String REALM = "realm";
    private static final String NONCE = "nonce";
    private static final String ALGO = "algorithm";
    private static final String QOP   = "qop";
    private static final String QOP_AUTH = "auth";
    private static final String QOP_AUTH_INT = "auth-int";
    private static final String OPAQUE = "opaque";

    private String mSchemeName;
    private String mRealm;
    private String mNonce;
    private String mAlgorithm;
    private String mQop;
    private String mOpaque;

    /**
     * Constructs an WwwAuthHeader.
     *
     */
    protected WwwAuthHeader(String schemeName, String realm, String nonce, String algorithm,
                         String qop, String opaque) {
        mSchemeName = schemeName;
        mRealm = realm;
        mNonce = nonce;
        mAlgorithm = algorithm;
        mQop = qop;
        mOpaque = opaque;
    }

    public String getRealm() {
        return mRealm;
    }

    public void setRealm(String realm) {
        mRealm = realm;
    }

    public String getNonce() {
        return mNonce;
    }

    public void setNonce(String nonce) {
        mNonce = nonce;
    }

    public String getAlgorithm() {
        return mAlgorithm;
    }

    public void setAlgorithm(String algorithm) {
        mAlgorithm = algorithm;
    }

    public String getQop() {
        return mQop;
    }

    public void setQop(String qop) {
        mQop = qop;
    }

    public String getOpaque() {
        return mOpaque;
    }

    public void setOpaque(String opaque) {
        mOpaque = opaque;
    }

    public String getSchemeName() {
        return mSchemeName;
    }

    public void setSchemeName(String schemeName) {
        mSchemeName = schemeName;
    }

    @Override
    public String toString() {
        return "WwwAuthHeader [realm=" + mRealm + ", schema name="
               + mSchemeName + ", nonce=" + mNonce + ", algorithm="
               + mAlgorithm + ", qop=" + mQop + ", opaque=" + mOpaque + "]";
    }

    /**
     * Utility function to parse HTTP WWW-Authenticate header.
     * Generate WwwAuthHeader to use.
     *
     * @param headerValue the value of HTTP WWW-Authenticate header.
     * @return the object of WwwAuthHeader class.
     * @throws MalformedChallengeException if the foramt of header is wrong.
     *
     */
    public static WwwAuthHeader parse(String headerValue) {
        WwwAuthHeader obj = null;

        String value = headerValue;
        int pos = 0;
        while (pos < value.length()) {
            int tokenStart = pos;
            pos = HeaderParser.skipUntil(value, pos, " ");

            String scheme = value.substring(tokenStart, pos).trim();
            pos = HeaderParser.skipWhitespace(value, pos);

            String rest = value.substring(pos); // rest of challenge except scheme
            pos += rest.length();

            String realm = null;
            String domain = null;
            String nonce = null;
            String algo = null;
            String stale = null;
            String qop = null;
            String opaque = null;

            String[] fields = rest.split(",");
            int i = 0;
            for (String field : fields) {
                System.out.println("field[" + i + "]: " + field);
                String[] keyValue = field.trim().split("=");
                if (keyValue.length < 2) {
                   System.out.println("No support:" + field);
                   i++;
                   continue;
                }

                String key = keyValue[0];
                String kv;
                if (keyValue.length > 2) {
                    kv = field.trim().substring(key.length() + 1);
                } else {
                    kv = keyValue[1];
                }

                if (kv.indexOf("\"") >= 0) {
                    kv = HeaderParser.getQuoteString(kv, key, 0);
                }

                if ("realm".equals(key)) {
                    realm = kv;
                } else if ("uri".equals(key)) {
                } else if ("algorithm".equals(key)) {
                    algo = kv;
                } else if ("domain".equals(key)) {
                    domain = kv;
                } else if ("nonce".equals(key)) {
                    nonce = kv;
                } else if ("stale".equals(key)) {
                    stale = kv;
                } else if ("qop".equals(key)) {
                    qop = kv;
                } else if ("opaque".equals(key)) {
                    opaque = kv;
        }
                i++;
        }
            obj = new WwwAuthHeader(scheme, realm, nonce, algo, qop, opaque);
        }

        Log.d(TAG, "Dump:" + obj);

        return obj;
    }
}
