package com.mediatek.gba.auth;

import android.os.Build;
import android.os.SystemProperties;
import android.text.TextUtils;
import android.util.Log;

import com.mediatek.gba.GbaConstant;
import com.mediatek.gba.element.BsfId;
import com.mediatek.gba.element.NafId;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.security.InvalidKeyException;
import java.security.NoSuchAlgorithmException;
import java.util.Arrays;

import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;

/**
 * implementation for KeyCalculator.
 *
 * @hide
 */
public class KeyCalculator {

    private final static String TAG = "KeyCalculator";
    // Sensitive log task
    private static final boolean SENLOG = GbaConstant.SENLOG;
    private static final boolean DBGLOG = GbaConstant.DBGLOG;

    private final static String ALGORITHM_HMAC_SHA_256 = "HmacSHA256";
    private final static String STR_GBA_ME = "gba-me";
    private final static String STR_GBA_U = "gba-u";
    private final static String STR_GBA_RES = "3gpp-gba-res";
    private final static String STR_GBA_KS = "3gpp-gba-ks";
    private final static String UTF8 = "UTF-8";
    private final static byte[] DEF_FC = new byte[] { 0x01};
    private final static int RES_LEN = 16;

    private static KeyCalculator sKeyCalculator = null;

    /**
     * Utility function to get the single instance of KeyCalculator.
     * @return the instance of KeyCalculator.
     *
     */
    public static KeyCalculator getInstance() {
        if (sKeyCalculator == null) {
            sKeyCalculator = new KeyCalculator();
        }

        return sKeyCalculator;
    }

    /**
     * calcuate NAS_KS_ext.
     * @param kc the value of kc.
     * @param rand the value of rand.
     * @param ksInput the value of ksInput.
     * @param sres the value of sres.
     * @return the ks value
     * @throws IOException if any IO error is occurred.
     *
     */
    public byte[] calculateKsBySres(byte[] kc, byte[] rand, byte[] ksInput, byte[] sres)
        throws IOException {

        byte[] result = new byte[RES_LEN];
        final byte[] ks = calculateResKs(kc, rand);
        final byte[] paramS = calculateKsS(DEF_FC, ksInput, STR_GBA_KS, sres);
        byte[] res = calculateHmacSha256(ks, paramS);

        if (res.length > RES_LEN) {
            Log.i(TAG, "truncated to 128 bits");
        }

        result = Arrays.copyOfRange(res, 0, RES_LEN);

        Log.i(TAG, "[done]calculateGbaKs");
        return res;
    }

    /**
     * calcuate GBA response with GBA_ME mode.
     * @param kc the value of kc.
     * @param rand the value of rand.
     * @param sres the value of sres.
     * @return the response value.
     * @throws IOException if any IO error is occurred.
     *
     */
    public byte[] calculateRes(byte[] kc, byte[] rand, byte[] sres) throws IOException {
        byte[] result = new byte[RES_LEN];
        final byte[] ks = calculateResKs(kc, rand);
        final byte[] paramS = calculateResS(DEF_FC, STR_GBA_RES, sres);
        byte[] res = calculateHmacSha256(ks, paramS);

        if (res.length > RES_LEN) {
            Log.i(TAG, "truncated to 128 bits");
        }

        result = Arrays.copyOfRange(res, 0, RES_LEN);

        Log.i(TAG, "[done]calculateRes");
        return res;
    }

    private byte[] calculateResKs(byte[] kc, byte[] rand) {
        final int ksLen = kc.length + kc.length + rand.length;

        final ByteArrayOutputStream byteArrayWriter = new ByteArrayOutputStream(ksLen);

        byteArrayWriter.write(kc, 0, kc.length);
        byteArrayWriter.write(kc, 0, kc.length);
        byteArrayWriter.write(rand, 0, rand.length);

        final byte[] ks = byteArrayWriter.toByteArray();
        return ks;
    }

    /**
     * It calculates Ks_NAF.
     * @param ks the value of ks.
     * @param rand the value of rand.
     * @param impi the value of impi.
     * @param nafId the value of nafId.
     * @return the ksNaf value.
     * @throws IOException if any IO error is occurred.
     */
    public byte[] calculateKsNaf(final byte[] ks, final byte[] rand,
                                 final String impi, final NafId nafId) throws IOException {

        final byte[] nafIdBin = nafId.getNafIdBin();

        //3GPP TS 33.220
        //Annex B (normative):
        //Specification of the key derivation function KDF

        //The Key to be used in key derivation shall be:
        //Ks (i.e. CK || IK concatenated)

        //S = FC || P0 || L0 || P1 || L1 || P2 || L2 || P3 || L3 ||... || Pn || Ln
        //FC is single octet used to distinguish between different instances of the algorithm,
        //P0 ... Pn are the n+1 input parameter encodings, and
        //L0 ... Ln are the two-octet representations of
        //       the length of the corresponding input parameter encodings P0..Pn.
        //B.3 NAF specific key derivation in GBA and GBA_U

        final byte[] paramS = calculateS(DEF_FC, STR_GBA_ME, rand, impi, nafIdBin);

        Log.i(TAG, "nafId:" + nafId.getNafFqdn());

        //derived key = HMAC-SHA-256 ( Key , S )
        byte[] res = calculateHmacSha256(ks, paramS);

        return res;
    }

    /**
     * It calculates TMPI.
     * @param ks the value of ks.
     * @param rand the value of rand.
     * @param impi the value of impi.
     * @param bsfId the value of bsfI.d
     * @return the TMPI value.
     * @throws IOException if any IO error is occurred.
     */
    public byte[] calculateTmpi(final byte[] ks, final byte[] rand,
                                final String impi, final BsfId bsfId) throws IOException {

        final byte[] bsfIdBin = bsfId.getBsfIdBin();

        final byte[] paramS = calculateS(DEF_FC, STR_GBA_ME, rand, impi, bsfIdBin);
        byte[] res = calculateHmacSha256(ks, paramS);


        Log.i(TAG, "[done]calculateTmpi");
        return res;
    }

    private byte[] calculateKsS(final byte[] fc, byte[] inputKey, String gbaStr, byte[] sres) {
        byte[] gabByteArr = create(gbaStr);
        int sLen = 0;
        sLen += fc.length;
        sLen += inputKey.length;
        sLen += gabByteArr.length;
        sLen += sres.length;

        final ByteArrayOutputStream byteArrayWriter = new ByteArrayOutputStream(sLen);

        byteArrayWriter.write(fc, 0, fc.length);
        byteArrayWriter.write(inputKey, 0, inputKey.length);
        byteArrayWriter.write(gabByteArr, 0, gabByteArr.length);
        byteArrayWriter.write(sres, 0, sres.length);

        return byteArrayWriter.toByteArray();
    }

    private byte[] calculateResS(final byte[] fc, String gbaStr, byte[] sres) {
        byte[] gabByteArr = create(gbaStr);
        int sLen = 0;
        sLen += fc.length;
        sLen += gabByteArr.length;
        sLen += sres.length;

        final ByteArrayOutputStream byteArrayWriter = new ByteArrayOutputStream(sLen);

        byteArrayWriter.write(fc, 0, fc.length);
        byteArrayWriter.write(gabByteArr, 0, gabByteArr.length);
        byteArrayWriter.write(sres, 0, sres.length);

        return byteArrayWriter.toByteArray();
    }

    private byte[] calculateS(final byte[] fc, String gbaStr, byte[] rand, String impi,
        byte[] nafIdBin) {

        //FC = 0x01,
        //P0 = "gba-me" (i.e. 0x67 0x62 0x61 0x2d 0x6d 0x65),
        //L0 = length of P0 is 6 octets (i.e., 0x00 0x06).
        //P0 = "gba-u" (i.e. 0x67 0x62 0x61 0x2d 0x75), and
        //L0 = length of P0 is 5 octets (i.e., 0x00 0x05).
        //P1 = RAND,
        //L1 = length of RAND is 16 octets (i.e. 0x00 0x10),
        //P2 = IMPI encoded to an octet string using UTF-8 encoding (see clause B.2.1),
        //L2 = length of IMPI is variable (not greater that 65535),
        //P3 = NAF_ID with the FQDN part of the NAF_ID encoded to
        //     an octet string using UTF-8 encoding
        //L3 = length of NAF_ID is variable (not greater that 65535).

        if (fc == null) {
            throw new NullPointerException("The fc must be not null.");
        }

        if (gbaStr == null || gbaStr.length() == 0) {
            throw new IllegalArgumentException("The p0 must be not null.");
        }

        if (rand == null || rand.length == 0) {
            throw new IllegalArgumentException("The p1(rand) must be not null.");
        }

        if (impi == null || impi.length() == 0) {
            throw new IllegalArgumentException("The p2(impi) must be not null.");
        }

        if (nafIdBin == null || nafIdBin.length == 0) {
            throw new IllegalArgumentException("The p3(NAF_ID) must be not null.");
        }

        GbaConstant.PiiLog(TAG, "gbaStr:" + gbaStr);
        GbaConstant.PiiLog(TAG, "impi:" + impi);

        byte[] gabByteArr = create(gbaStr);
        byte[] impiByteArr = create(impi);

        int sLen = 0;
        sLen += fc.length;
        sLen += gabByteArr.length;
        sLen += rand.length;
        sLen += impiByteArr.length;
        sLen += nafIdBin.length;

        final ByteArrayOutputStream byteArrayWriter = new ByteArrayOutputStream(sLen);

        byteArrayWriter.write(fc, 0, fc.length);
        byteArrayWriter.write(gabByteArr, 0, gabByteArr.length);
        byteArrayWriter.write(getLenByte(gabByteArr.length), 0, 2);
        byteArrayWriter.write(rand, 0, rand.length);
        byteArrayWriter.write(getLenByte(rand.length), 0, 2);
        byteArrayWriter.write(impiByteArr, 0, impiByteArr.length);
        byteArrayWriter.write(getLenByte(impiByteArr.length), 0, 2);
        byteArrayWriter.write(nafIdBin, 0, nafIdBin.length);
        byteArrayWriter.write(getLenByte(nafIdBin.length), 0, 2);

        return byteArrayWriter.toByteArray();
    }

    protected static byte[] create(final String param) {
        if (param == null) {
            throw new IllegalArgumentException("The param must be not null.");
        }

        byte[] content = null;

        try {
            content = param.getBytes(UTF8);
        } catch (UnsupportedEncodingException e) {
            Log.e(TAG, e.getMessage(), e);
            throw new IllegalStateException(e);
        }

        return content;
    }

    /**
     * Calculate Ks with GBA_ME.
     * @param ck the value of cipher key.
     * @param ik the value of integrated key.
     * @return the Ks value.
     *
     */
    public byte[] calculateKsByGbaMe(final byte[] ck, final byte[] ik) {

        final int ksLen = ck.length + ik.length;

        final ByteArrayOutputStream byteArrayWriter = new ByteArrayOutputStream(ksLen);

        byteArrayWriter.write(ck, 0, ck.length);
        byteArrayWriter.write(ik, 0, ik.length);

        final byte[] ks = byteArrayWriter.toByteArray();
        return ks;
    }

    private byte[] calculateHmacSha256(final byte[] paramKey, final byte[] paramS)
        throws IOException {

        if (paramKey == null || paramKey.length == 0) {
            throw new IllegalStateException("paramKey must be initialized");
        }

        if (paramS == null || paramS.length == 0) {
            throw new IllegalStateException("paramS must be initialized");
        }

        Mac mac = null;

        try {
            mac = Mac.getInstance(ALGORITHM_HMAC_SHA_256);
        } catch (NoSuchAlgorithmException e) {
            Log.e(TAG, e.getMessage(), e);
            throw new IOException(e);
        }

        try {
            mac.init(new SecretKeySpec(paramKey, mac.getAlgorithm()));
        } catch (InvalidKeyException e) {
            Log.e(TAG, e.getMessage(), e);
            throw new IOException(e);
        }

        mac.update(paramS);

        final byte[] res = mac.doFinal();
        return res;
    }

    private byte[] getLenByte(int value) {
        byte[] data = new byte[2];
        data[1] = (byte) (value % 256);
        data[0] = (byte) (value / 256);
        return data;
    }

}
