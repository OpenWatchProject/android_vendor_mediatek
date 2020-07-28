package com.mediatek.gba.header;

import java.util.HashMap;
import java.util.Map;

/**
 * implementation for AuthInfoHeader.
 *
 * @hide
 */
public class AuthInfoHeader {

    public static final String HEADER_NAME = "Authentication-Info";

    protected static final String RSPAUTH  = "rspauth";
    protected static final String NONCE    = "nonce";
    protected static final String CNONCE   = "cnonce";
    protected static final String QOP      = "qop";
    protected static final String NC       = "nc";
    protected static final String OPAQUE   = "opaque";

    private String mQop;
    private String mRspauth;
    private String mCnonce;
    private String mNonceCount;
    private String mOpaque;
    private String mNonce;

    /**
     * Constructs an AuthInfoHeader.
     *
     */
    protected AuthInfoHeader(String qop, String rspauth, String cnonce, String nonceCount,
                          String opaque, String nonce) {
        mQop = qop;
        mRspauth = rspauth;
        mCnonce = cnonce;
        mNonceCount = nonceCount;
        mOpaque = opaque;
        mNonce = nonce;
    }

    public String getQop() {
        return mQop;
    }

    public void setQop(String qop) {
        mQop = qop;
    }

    public String getRspauth() {
        return mRspauth;
    }

    public void setRspauth(String rspauth) {
        mRspauth = rspauth;
    }

    public String getCnonce() {
        return mCnonce;
    }

    public void setCnonce(String cnonce) {
        mCnonce = cnonce;
    }

    public String getNonceCount() {
        return mNonceCount;
    }

    public void setNonceCount(String nc) {
        mNonceCount = nc;
    }

    public String getOpaque() {
        return mOpaque;
    }

    public void setOpaque(String opaque) {
        mOpaque = opaque;
    }

    public String getNonce() {
        return mNonce;
    }

    public void setNonce(String nonce) {
        mNonce = nonce;
    }

    @Override
    public String toString() {
        return "AuthenticationInfoHeader [qop=" + mQop + ", rspauth="
               + mRspauth + ", cnonce=" + mCnonce + ", nonceCount="
               + mNonceCount + ", opaque=" + mOpaque + ", nonce="
               + mNonce + "]";
    }

    /**
     * Utility function to parse HTTP Authentication-Info header.
     * To construct WwwAuthHeader class if parsed done.
     *
     * @param headerValue the value of HTTP WWW-Authenticate header.
     * @return the object of WwwAuthHeader class.
     *
     */
    public static AuthInfoHeader parse(String headerValue) {
        AuthInfoHeader obj = null;

        String value = headerValue;
        int pos = 0;
        while (pos < value.length()) {
            int tokenStart = pos;
            pos = HeaderParser.skipUntil(value, pos, " ");

            String scheme = value.substring(tokenStart, pos).trim();
            pos = HeaderParser.skipWhitespace(value, pos);

            String rest = value.substring(pos); // rest of challenge except scheme
            pos += rest.length();

            String rspauth = null;
            String cnonce = null;
            String qop = null;
            String nc = null;
            String nonce = null;
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

                if ("rspauth".equals(key)) {
                    rspauth = kv;
                } else if ("cnonce".equals(key)) {
                    cnonce = kv;
                } else if ("qop".equals(key)) {
                    qop = kv;
                } else if ("nc".equals(key)) {
                    nc = kv;
                }
                i++;
            }
            obj = new AuthInfoHeader(qop, rspauth, cnonce, nc, opaque, nonce);
        }
        return obj;
    }
}
