package com.mediatek.gba.element;

import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.UnsupportedEncodingException;
import java.util.Arrays;

/**
 * implementation for BsfId.
 *
 * @hide
 */
public class BsfId {
    private final static String TAG = "BsfId";
    private final static String UTF8 = "UTF-8";
    private final static int MAX_LENGTH = 5;

    private byte[] mBsfIdBin;
    private String mBsfFullDnsName;
    private byte[] mUaSecurityProtocolId = new byte[MAX_LENGTH];

    /**
     * @param bsfFullDnsName - full DNS name of BSF server
     * @param uaSecurityProtocolId - Ua security protocol id
     */
    public BsfId(final String bsfFullDnsName, final byte[] uaSecurityProtocolId) {
        if (bsfFullDnsName == null) {
            throw new IllegalArgumentException(
                "The input parameter bsfFullDnsName must be not null.");
        }

        if (uaSecurityProtocolId == null) {
            throw new IllegalArgumentException(
                "The input parameter uaSecurityProtocolId must be not null.");
        }

        mBsfFullDnsName = bsfFullDnsName;
        mUaSecurityProtocolId = uaSecurityProtocolId;

        final byte[] bsfIdBin = createBsfIdBin(bsfFullDnsName, uaSecurityProtocolId);

        mBsfIdBin = bsfIdBin;
    }

    /**
     * @param bsfIdBin - BSF_ID as a byte array
     */
    public BsfId(final byte[] bsfIdBin) {
        if (bsfIdBin == null) {
            throw new IllegalArgumentException("The input parameter must be not null.");
        }

        if (bsfIdBin.length <= MAX_LENGTH) {
            throw new IllegalArgumentException(
                "The length of input parameter must be greater then 5. cerLen="
                + bsfIdBin.length);
        }

        mBsfIdBin = bsfIdBin;
        mBsfFullDnsName = new String(bsfIdBin, 0, bsfIdBin.length - MAX_LENGTH);

        System.arraycopy(bsfIdBin, bsfIdBin.length - MAX_LENGTH, mUaSecurityProtocolId, 0,
                        MAX_LENGTH);
    }

    /**
     * @param bsfFullDnsName - full DNS name of BSF server
     * @param uaSecurityProtocolId - Ua security protocol id
     * @return BSF_ID as a byte array
     */
    private byte[] createBsfIdBin(final String bsfFullDnsName,
                                  final byte[] uaSecurityProtocolId) {

        byte[] bsfFullDnsNameBytes = null;

        try {
            bsfFullDnsNameBytes = bsfFullDnsName.getBytes(UTF8);
        } catch (UnsupportedEncodingException e) {
            Log.e(TAG, e.getMessage(), e);
            throw new IllegalStateException(e);
        }

        final ByteArrayOutputStream byteArrayWriter =
            new ByteArrayOutputStream(bsfFullDnsNameBytes.length + uaSecurityProtocolId.length);
        byteArrayWriter.write(bsfFullDnsNameBytes, 0, bsfFullDnsNameBytes.length);
        byteArrayWriter.write(uaSecurityProtocolId, bsfFullDnsNameBytes.length,
            uaSecurityProtocolId.length);

        final byte[] bsfIdBin = byteArrayWriter.toByteArray();
        return bsfIdBin;
    }

    /**
     * @return BSF_ID as a byte array
     */
    public byte[] getBsfIdBin() {
        return mBsfIdBin;
    }

    /**
     * @return full DNS name of BSF server
     */
    public String getBsfFullDnsName() {
        return mBsfFullDnsName;
    }

    /**
     * @return Ua security protocol id
     */
    public byte[] getUaSecurityProtocolId() {
        return mUaSecurityProtocolId;
    }

    @Override
    public String toString() {
        return "BsfId [bsfIdBin=" + Arrays.toString(mBsfIdBin)
               + ", bsfFullDnsName=" + mBsfFullDnsName + ", uaSecurityProtocolId="
               + Arrays.toString(mUaSecurityProtocolId) + "]";
    }

}
