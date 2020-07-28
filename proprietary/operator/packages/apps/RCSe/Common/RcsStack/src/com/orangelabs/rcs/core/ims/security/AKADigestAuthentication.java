package com.orangelabs.rcs.core.ims.security;

import java.security.MessageDigest;

import android.content.Context;
import android.os.RemoteException;
import android.util.Base64;
import android.util.Log;
import android.telephony.TelephonyManager;
import android.telephony.SubscriptionManager;
import android.os.ServiceManager;

import com.orangelabs.rcs.core.ims.ImsModule;
import com.orangelabs.rcs.platform.AndroidFactory;
import com.orangelabs.rcs.utils.logger.Logger;
import com.android.internal.telephony.PhoneConstants;

/**
 * AKA Digest authentication MD5 and SHA (see RFC3310)
 *
 * @author mtk33054
 */
public class AKADigestAuthentication {
    /**
     */
    public static final String NC_PARAM = "nc";

    /**
     * Hex chars
     */
    private static final char[] HEX = { '0', '1', '2', '3', '4', '5', '6', '7',
            '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

    /**
     * Domain name
     */
    private String realm = null;

    /**
     * Opaque parameter
     */
    private String opaque = "";

    /**
     * Nonce
     */
    private String nonce = "";

    /**
     * Next nonce
     */
    private String nextnonce = null;

    /**
     * Qop
     */
    private String qop = "";

    /**
     * Cnonce
     */
    private String cnonce = "" + System.currentTimeMillis();

    // FIX IT
    private String algorithm = "AKAv1-MD5";// TODO , just to make sure first
                                            // request has MD5 as per vendor .

    /**
     * Cnonce counter
     */
    private int nc = 0;

    /**
     * MD5 algorithm
     */
    private MD5Digest md5Digest = new MD5Digest();

    /**
     * The logger
     */
    private Logger logger = Logger.getLogger(this.getClass().getName());

    /**
     * Constructor
     */
    public AKADigestAuthentication() {
        // for first request server wants some initial values

        realm = ImsModule.IMS_USER_PROFILE.getRealm();

    }

    /**
     * Returns realm parameter
     *
     * @return Realm
     */
    public String getRealm() {
        return realm;
    }

    /**
     * Set the realm parameter
     *
     * @param realm
     *            Realm
     */
    public void setRealm(String realm) {
        this.realm = realm;
    }

    /**
     * Returns opaque parameter
     *
     * @return Opaque
     */
    public String getOpaque() {
        return opaque;
    }

    /**
     * Set the opaque parameter
     *
     * @param opaque
     *            Opaque
     */
    public void setOpaque(String opaque) {
        this.opaque = opaque;
    }

    /**
     * Get the client nonce parameter
     *
     * @return Client nonce
     */
    public String getCnonce() {
        return cnonce;
    }

    /**
     * Get the nonce parameter
     *
     * @return Nonce
     */
    public String getNonce() {
        return nonce;
    }

    /**
     * Set the nonce parameter
     *
     * @param nonce
     *            Nonce
     */
    public void setNonce(String nonce) {
        this.nonce = nonce;
    }

    /**
     * Returns the next nonce parameter
     *
     * @return Next nonce
     */
    public String getNextnonce() {
        return nextnonce;
    }

    /**
     * Set the next nonce parameter
     *
     * @param nextnonce
     *            Next nonce
     */
    public void setNextnonce(String nextnonce) {
        this.nextnonce = nextnonce;
    }

    /**
     * Returns the Qop parameter
     *
     * @return Qop
     */
    public String getQop() {
        return qop;
    }

    /**
     * Set the Qop parameter
     *
     * @param qop
     *            Qop parameter
     */
    public void setQop(String qop) {
        if (qop != null) {
            qop = qop.split(",")[0];
        }
        this.qop = qop;
    }

    /**
     * Update the nonce parameters
     */
    public void updateNonceParameters() {
        // Update nonce and nc
        if (nextnonce.equals(nonce)) {
            // Next nonce == nonce
            nc++;
        } else {
            // Next nonce != nonce
            nc = 1;
            nonce = nextnonce;
        }
    }

    /**
     * Build the cnonce counter
     *
     * @return String (ie. "00000001")
     */
    public String buildNonceCounter() {
        String result = Integer.toHexString(nc);
        while (result.length() != 8) {
            result = "0" + result;
        }
        return result;
    }

    /**
     * Convert to hexa string
     *
     * @param value
     *            Value to convert
     * @return String
     */
    private String toHexString(byte[] value) {
        int pos = 0;
        char[] c = new char[value.length * 2];
        for (int i = 0; i < value.length; i++) {
            c[pos++] = HEX[value[i] >> 4 & 0xf];
            c[pos++] = HEX[value[i] & 0xf];
        }
        return new String(c);
    }

    /*
     * Using the shared secret K and the sequence number SQN, the client
     * verifies the AUTN with the ISIM. If the verification is successful, the
     * network has been authenticated. The client then produces an
     * authentication response RES, using the shared secret K and the random
     * challenge RAND.
     */

    public byte[] calculateAkaAuthAndRes(byte[] rand, byte[] autn, int subId) {
        TelephonyManager tm = (TelephonyManager)AndroidFactory.getApplicationContext()
                .getSystemService(Context.TELEPHONY_SERVICE);

        String res = null;

        try {
            byte[] contactData = new byte[rand.length+autn.length+2];
            int len = 0;

            /* Data format:
             *  1: RAND length(L1)
             *  2 ~ L1+1: RAND
             *  L1+2: AUTN length(L2)
             *  L1+3 ~ (L1+L2+2): AUTN
             *  Currently length of both RAND and AUTN are fixed 16 bytes
             */
            contactData[len++] = 0x10;
            System.arraycopy(rand, 0, contactData, len, rand.length);
            len += rand.length;
            contactData[len++] = 0x10;
            System.arraycopy(autn, 0, contactData, len, autn.length);
            byte[] encodeData = Base64.encode(contactData, Base64.NO_WRAP);

            String cardType = "USIM";//iTelEx.getIccCardType(subId);

            if ("USIM".equalsIgnoreCase(cardType))
                res = tm.getIccAuthentication(subId, PhoneConstants.APPTYPE_USIM,
                        PhoneConstants.AUTH_CONTEXT_EAP_AKA, new String(encodeData));
            else if ("CSIM".equalsIgnoreCase(cardType))
                res = tm.getIccAuthentication(subId, PhoneConstants.APPTYPE_CSIM,
                        PhoneConstants.AUTH_CONTEXT_EAP_AKA, new String(encodeData));
            else if ("ISIM".equalsIgnoreCase(cardType))
                res = tm.getIccAuthentication(subId, PhoneConstants.APPTYPE_ISIM,
                        PhoneConstants.AUTH_CONTEXT_EAP_AKA, new String(encodeData));

            byte[] decodeData = Base64.decode(res, Base64.DEFAULT);

            if (decodeData[0] == (byte)0xDB) {
                return decodeData;
            }
        } catch (Exception ex) {
            ex.printStackTrace();
        }

        return null;
    }

    /**
     * Calculate AKA Digest nonce response
     *
     * @param user
     *            User
     * @param password
     *            Password
     * @param method
     *            Method
     * @param uri
     *            Request URI
     * @param nc
     *            Nonce counter
     * @param body
     *            Entity body
     * @throws Exception
     */
    public String calculateResponse(String user, byte[] password,
            String method, String uri, String nc, String body) throws Exception {
        if (user == null || realm == null || uri == null || nonce == null) {
            throw new Exception("Invalid Authorization header" + user + "/"
                    + realm + "/" + uri + "/" + nonce);
        }

        byte[] tmpData = (user + ":" + realm + ":").getBytes();
        byte[] a1 = new byte[tmpData.length + password.length];
        int i = 0;
        for (; i < tmpData.length; i++)
            a1[i] = tmpData[i];
        for (int j = 0; j < password.length; j++)
            a1[i++] = password[j];
        String a2 = method + ":" + uri;

        if (qop != null) {
            if (!qop.startsWith("auth")) {
                throw new Exception("Invalid qop: " + qop);
            }

            if (nc == null || cnonce == null) {
                throw new Exception("Invalid Authorization header: " + nc + "/"
                        + cnonce);
            }

            if (qop.equals("auth-int")) {
                a2 = a2 + ":" + H(body);
            }
            if (logger.isActivated()) {
                logger.error("------------AKA Authentication log start----------");
                logger.error("user: " + user);
                logger.error("realm: " + realm);
                logger.error("passwd: " + password);
                logger.error("a1: " + a1);
                logger.error("nonce: " + nonce);
                logger.error("nc: " + nc);
                logger.error("cnonce: " + cnonce);
                logger.error("qop: " + qop);
                logger.error("a2: " + a2);
                logger.error("H(a1): " + H(a1));
                logger.error("H(a2): " + H(a2));
                logger.error("------------AKA Authentication log end----------");
            }
            return H(H(a1) + ":" + nonce + ":" + nc + ":" + cnonce + ":" + qop
                    + ":" + H(a2));
        } else {
            return H(H(a1) + ":" + nonce + ":" + H(a2));
        }
    }

    /**
     * AKA Digest algo
     *
     * @param data
     *            Input data
     * @return Hash key
     */
    private String H(String data) {
        try {
            if (data == null) {
                data = "";
            }

            if (algorithm.contains("MD5")) {

                md5Digest.update(data.getBytes(), 0, data.getBytes().length);
                byte returnValue[] = new byte[md5Digest.getDigestSize()];
                md5Digest.doFinal(returnValue, 0);
                return toHexString(returnValue);
            } else if (algorithm.contains("SHA")) {

                MessageDigest md = MessageDigest.getInstance("SHA-256");
                md.update(data.getBytes("UTF-8"));
                byte[] digest = md.digest();
                return toHexString(digest);
            }
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("AKA digest  algo has failed", e);
            }
            return null;
        }
        return null;
    }

    /**
     * AKA Digest algo
     *
     * @param data
     *            Input data
     * @return Hash key
     */
    private String H(byte[] data) {
        try {
            if (data == null) {
                return null;
            }

            if (algorithm.contains("MD5")) {
                md5Digest.update(data, 0, data.length);
                byte returnValue[] = new byte[md5Digest.getDigestSize()];
                md5Digest.doFinal(returnValue, 0);
                return toHexString(returnValue);
            } else if (algorithm.contains("SHA")) {
                MessageDigest md = MessageDigest.getInstance("SHA-256");
                md.update(data);
                byte[] digest = md.digest();
                return toHexString(digest);
            }
        } catch (Exception e) {
            if (logger.isActivated()) {
                logger.error("AKA digest  algo has failed", e);
            }
            return null;
        }
        return null;
    }

    public void setAlgorithm(String algorithm) {
        this.algorithm = algorithm;
    }

    public String getAlgorithm() {
        // TODO Auto-generated method stub
        return algorithm;
    }
}
