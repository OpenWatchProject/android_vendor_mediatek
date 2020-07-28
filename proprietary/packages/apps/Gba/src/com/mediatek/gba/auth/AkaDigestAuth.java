package com.mediatek.gba.auth;

import android.os.Build;
import android.os.SystemProperties;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.util.HexDump;

import com.mediatek.gba.GbaConstant;
import com.mediatek.gba.header.WwwAuthHeader;
import com.mediatek.internal.telephony.MtkSuppServHelper;

import java.io.UnsupportedEncodingException;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Date;

 /**
  * implementation for AkaDigestAuth.
  *
  * @hide
  */
public class AkaDigestAuth {
    private static final String TAG = "AkaDigestAuth";

    // Sensitive log task
    private static final boolean SENLOG = GbaConstant.SENLOG;
    private static final boolean DBGLOG = GbaConstant.DBGLOG;

    private static final char[] HEXADECIMAL = {
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd',
        'e', 'f'
    };

    private final static String QOP_AUTH_NAME = "auth";
    private final static String QOP_AUTH_INT_NAME = "auth-int";
    private final static String QOP_AUTH_BOTH = "auth, auth-int";
    private final static String QOP_UNSPECIFIED = "";

    private final static String SEPARATOR = ":";
    private final static String QUOTE = "\"";

    private final static String AKA_VERSION = "AKAv";
    private final static String AKA_SPLITTER = "-";
    private final static String ALGORITHM_NAME_UNSPECIFIED = "";
    private final static String ALGORITHM_NAME_MD5 = "MD5";
    private final static String ALGORITHM_NAME_MD5_SESS = "MD5-sess";

    private final static String HEADER_VALUE_FORMAT_WITH_RESPONSE = ""
            + "Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", qop=%s, "
            + "nc=%s, cnonce=\"%s\", response=\"%s\", opaque=\"%s\"";

    private final static String HEADER_VALUE_FORMAT_WITH_RESPONSE2 = ""
            + "Digest username=\"%s\", realm=\"%s\", nonce=\"%s\", uri=\"%s\", qop=%s, "
            + "nc=%s, cnonce=\"%s\", response=\"%s\"";

    private String mQop;
    private String mAlgorithm;
    private String mCharSet;
    private String mUsername;
    private String mRealm;
    private String mAuts;
    private String mPassword;
    private String mNonce;
    private String mCnonce;
    private String mMethod;
    private String mUri;
    private String mEntityBody;
    private String mNc;
    private String mResponse;
    private String mOpaque;
    MessageDigest mMd5Helper;

    /**
     * Construction function of AkaDigestAuth.
     */
    private AkaDigestAuth(String userName, String auts, String passwd, String uri, String nc,
        String qop, String algorithm, String realm, String nonce, String opaque, String cnonce,
        String method) {

        initMd5();

        mQop = qop;
        mAlgorithm = algorithm;
        mUsername = userName;
        mRealm = realm;
        mPassword = passwd;
        mAuts = auts;
        mNonce = nonce;
        mOpaque = opaque;
        mUri = uri;
        mNc = nc;
        mMethod = method;
        mCnonce = cnonce;
        mEntityBody = "";
        mResponse = "";

        mCharSet = GbaConstant.ISO_8859_1;
    }

    /**
     * Construction function of AkaDigestAuth.
     * @param header the header value.
     * @param userName user name.
     * @param auts the value of auts.
     * @param passwd password.
     * @param uri the uri value.
     * @param nc the value of nc.
     */
    public AkaDigestAuth(WwwAuthHeader header, String userName, String auts, String passwd,
        String uri, String nc) {

        initMd5();

        mQop = header.getQop();

        if (mQop.indexOf(QOP_AUTH_NAME) != -1 && mQop.indexOf(QOP_AUTH_INT_NAME) != -1) {
            mQop = QOP_AUTH_INT_NAME; //only support AUTH INT
        }

        if (header.getAlgorithm() != null) {
            mAlgorithm = header.getAlgorithm();
        } else {
            mAlgorithm = ALGORITHM_NAME_UNSPECIFIED;
        }

        mUsername = userName;
        mRealm = header.getRealm();
        mPassword = passwd;
        mAuts = auts;
        mNonce = header.getNonce();
        mOpaque = header.getOpaque();
        mUri = uri;
        mNc = nc;
        mMethod = "GET";
        mCnonce = createCNonce();
        mEntityBody = "";
        mResponse = "";
        mCharSet = GbaConstant.ISO_8859_1;

    }

    private void initMd5() {
        try {
            mMd5Helper = MessageDigest.getInstance(ALGORITHM_NAME_MD5);
            mMd5Helper.reset();
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        }
    }

    /**
     * Create Authorization header.
     *
     * @return the value of Authorization header.
     *
     */
    public String createAuthorHeaderValue() {
        StringBuilder headerValue = null;

        if (mOpaque != null) {
            headerValue = new StringBuilder(String.format(HEADER_VALUE_FORMAT_WITH_RESPONSE,
                mUsername, mRealm, mNonce, mUri, mQop, mNc, mCnonce, mResponse, mOpaque,
                mAlgorithm));
        } else {
            headerValue = new StringBuilder(String.format(HEADER_VALUE_FORMAT_WITH_RESPONSE2,
                mUsername, mRealm, mNonce, mUri, mQop, mNc, mCnonce, mResponse, mAlgorithm));
        }

        if (mAlgorithm != null && mAlgorithm.length() > 0) {
            headerValue.append(", algorithm=" + mAlgorithm);
        }

        if (mAuts != null && mAuts.length() > 0) {
            headerValue.append(", auts=\"" + mAuts.trim() + "\"");
        }

        return headerValue.toString();
    }

    private String createCNonce() {
        String cnonce = "";

        final String dataStr = "" + new Date().getTime();
        Log.i(TAG, "dataStr:" + dataStr);
        try {
            cnonce = md5(dataStr.getBytes(GbaConstant.US_ASCII));
        } catch (UnsupportedEncodingException ue) {
            Log.e(TAG, "ue:" + ue);
        }

        return cnonce;
    }

    private String encode(byte[] binaryData) {
        if (binaryData.length != 16) {
            return null;
        }

        char[] buffer = new char[32];

        for (int i = 0; i < 16; i++) {
            int low = (binaryData[i] & 0x0f);
            int high = ((binaryData[i] & 0xf0) >> 4);
            buffer[i * 2] = HEXADECIMAL[high];
            buffer[(i * 2) + 1] = HEXADECIMAL[low];
        }

        return new String(buffer);
    }


    /**
     * calcuate HTTP digest response.
     *
     * @return the response of HTTP digest.
     *
     */
    public String calculateRequestDigest() {
        String res = null;


        Log.i(TAG, "mQop:" + mQop);

        if (mQop.equals(QOP_AUTH_NAME) || mQop.equals(QOP_AUTH_INT_NAME)) {
            //if the qop directive is 'auth' or 'auth-int'
            //response = md5(ha1 : nonce : nonceCount : clientNonce : qop : ha2)
            String ha1 = calculateHA1(mAlgorithm, mUsername, mRealm, mPassword, mNonce, mCnonce);
            String ha2 = calculateHA2(mQop, mMethod, mUri, mEntityBody);
            String rawRes = ha1 + SEPARATOR + mNonce + SEPARATOR + mNc + SEPARATOR + mCnonce
                            + SEPARATOR + mQop + SEPARATOR + ha2;
            Log.i(TAG, "rawRes:" + rawRes);
            res = md5(getBytes(rawRes));
            Log.i(TAG, "response:" + res);
        } else if (mQop.equals(QOP_UNSPECIFIED)) {
            String ha1 = calculateHA1(mAlgorithm, mUsername, mRealm, mPassword, mNonce, mCnonce);
            String ha2 = calculateHA2(mQop, mMethod, mUri, mEntityBody);
            String rawRes = ha1 + SEPARATOR + mNonce + SEPARATOR + ha2;
            Log.i(TAG, "rawRes:" + rawRes);
            res = md5(getBytes(rawRes));
            Log.i(TAG, "response:" + res);
        } else {
            Log.e(TAG, "Unsupported qop value, qop=" + mQop);
        }

        mResponse = res;
        return res;
    }

    private String calculateHA1(String algorithm, String username, String realm, String password,
                        String nonce, String cnonce) {
        String a1Res = null;

        if (!SENLOG) {
            Log.i(TAG, "run calculateHA1:" + algorithm + "/" +  username + "/" + realm + "/"
                    + password + "/" + nonce + "/" + cnonce);
        } else {
            Log.i(TAG, "run calculateHA1:" + algorithm + "/[hidden]/" + realm + "/"
                    + password + "/" + nonce + "/" + cnonce);
        }

        //# If the algorithm directive in the WWW-Authenticate header is 'MD5' or unspecified
        //ha1 = md5(username : realm : password)
        //# Else, if the algorithm directive is 'MD5-Sess', the nonce and client nonce becomes
        //  part of ha1
        //ha1 = md5(md5(username : realm : password) : nonce : cnonce)

        if (algorithm.endsWith(ALGORITHM_NAME_UNSPECIFIED) ||
            algorithm.endsWith(ALGORITHM_NAME_MD5)) {
            Log.i(TAG, "A = username: relam : password");
            String tmpStr = username + SEPARATOR + realm + SEPARATOR;
            a1Res = HexDump.toHexString(getBytes(tmpStr)) + password;
            Log.i(TAG, "a1Res:" + a1Res);
        } else {
            String tempStr = username + SEPARATOR + realm + SEPARATOR + password;
            a1Res = md5(getBytes(tempStr))
                        + SEPARATOR + nonce + SEPARATOR + cnonce;
        }

        Log.i(TAG, "a1:" + a1Res);
        String ha1Result = md5(HexDump.hexStringToByteArray(a1Res));
        Log.i(TAG, "ha1Result:" + ha1Result);

        return ha1Result;
    }

    private String calculateHA2(String qop, String method, String uri, String entityBody) {
        String a2Res = null;

        //For ha2, if the qop directive is 'auth' or unspecified
        //ha2 = md5(method : digestURI)
        //else if the qop directive is 'auth-int
        //ha2 = md5(method : digestURI : md5(entityBody))

        Log.d(TAG, "run calculateHA2:" + qop + "/" +  method +
                    "/" + uri + "/Body : " + MtkSuppServHelper.encryptString(entityBody));

        if (mQop.equals(QOP_UNSPECIFIED) || mQop.equals(QOP_AUTH_NAME)) {
            a2Res = method + SEPARATOR + uri;
        } else if (mQop.equals(QOP_AUTH_INT_NAME)) {
            a2Res = method + SEPARATOR + uri + SEPARATOR
                + md5(getBytes(entityBody));
            Log.i(TAG, "a2Res:" + a2Res);
        }

        Log.i(TAG, "a2:" + MtkSuppServHelper.encryptString(a2Res));
        String ha2Result = md5(getBytes(a2Res));
        Log.i(TAG, "ha2Result:" + ha2Result);

        return ha2Result;
    }

    private String md5(byte[] input) {
        String hRes = null;

        final byte[] digestRes = mMd5Helper.digest(input);
        hRes = encode(digestRes);
        Log.i(TAG, "encode:" + hRes);

        return hRes;
    }
    private byte[] getBytes(String data) {
        byte[] rawData = null;
        try {
            Log.i(TAG, "getBytes:" + MtkSuppServHelper.encryptString(data) + ", Charset : " + mCharSet);
            rawData = data.getBytes(mCharSet);
        } catch (UnsupportedEncodingException ue) {
            ue.printStackTrace();
        }
        return rawData;
    }

}
