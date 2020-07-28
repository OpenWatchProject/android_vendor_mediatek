package com.mediatek.gba.element;

import android.util.Log;

import java.io.UnsupportedEncodingException;
import java.util.Arrays;

/**
 * implementation for NafId.
 *
 * @hide
 */
public class NafId {
    private static final String TAG = "NafId";
    private String mNafFqdn = null;
    private byte[] mUaSecurityProtocolId = null;
    private byte[] mNafIdBin = null;
    private static final int UA_SECURITY_PROTOCOL_LENGTH = 5;

    private NafId(String nafFqdn, byte[] nafSecurProtocolId) {
        if (nafFqdn == null || nafFqdn.length() == 0) {
            throw new IllegalArgumentException("nafFqdn must be not null or empty:" + nafFqdn);
        }

        if (nafSecurProtocolId == null) {
            throw new IllegalArgumentException("nafSecurProtocolId must be not null.");
        }

        if (nafSecurProtocolId.length != UA_SECURITY_PROTOCOL_LENGTH) {
            throw new IllegalArgumentException("The length of nafSecurProtocolId is not 5.");
        }

        mNafFqdn = nafFqdn;
        mUaSecurityProtocolId = nafSecurProtocolId;
        mNafIdBin = createNafIdBin(nafFqdn, nafSecurProtocolId);
    }

    private NafId(byte[] nafIdBin) {
        if (nafIdBin == null) {
            throw new IllegalArgumentException("The nafIdBin must be not null.");
        }

        if (nafIdBin.length <= UA_SECURITY_PROTOCOL_LENGTH) {
            throw new IllegalArgumentException(
                "The length of nafIdBin must be greater then 5. cerLen=" + nafIdBin.length);
        }

        mNafIdBin = nafIdBin;
        mNafFqdn = new String(nafIdBin, 0, nafIdBin.length - UA_SECURITY_PROTOCOL_LENGTH);
        byte[] buffer = new byte[UA_SECURITY_PROTOCOL_LENGTH];
        System.arraycopy(nafIdBin, nafIdBin.length - UA_SECURITY_PROTOCOL_LENGTH, buffer,
                        0, UA_SECURITY_PROTOCOL_LENGTH);
        mUaSecurityProtocolId = buffer;
    }

    private byte[] createNafIdBin(String nafFqdn, byte[] nafSecurProtocolId) {
        byte[] nafFqdnBytes = null;

        try {
            nafFqdnBytes = nafFqdn.getBytes("UTF-8");
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        }

        byte[] nafIdBin = new byte[nafFqdnBytes.length + UA_SECURITY_PROTOCOL_LENGTH];

        System.arraycopy(nafFqdnBytes, 0, nafIdBin, 0, nafFqdnBytes.length);
        System.arraycopy(nafSecurProtocolId, 0, nafIdBin, nafFqdnBytes.length,
                        UA_SECURITY_PROTOCOL_LENGTH);

        Log.i(TAG, "Naf Id:" + Arrays.toString(nafIdBin));
        return nafIdBin;
    }

    public String getNafFqdn() {
        return mNafFqdn;
    }

    public byte[] getUaSecurityProtocolId() {
        return mUaSecurityProtocolId;
    }

    public byte[] getNafIdBin() {
        return mNafIdBin;
    }

    /**
     *  The string info of this class.
     *
     * @return the string resprentation of this class.
     *
     */
    public String toString() {
        return "NafId [nafIdBin=" + Arrays.toString(mNafIdBin) + ", nafFqdn=" + mNafFqdn
                + ", uaSecurityProtocolId=" + mUaSecurityProtocolId + "]";
    }

    /**
     *  Convert byte array to NafId class.
     *
     * @param nafIdBytes the byte array of NAF ID.
     * @return NafId object.
     *
     */
    public static NafId createFromBytes(byte[] nafIdBytes) {
        NafId nafId = new NafId(nafIdBytes);
        return nafId;
    }

    /**
     *  Create a NafId class by NAF's FQDN and Security Protocol Id.
     *
     * @param nafFqdn the FQDN of NAF server.
     * @param nafSecurProtocolId the security protocol id of NAF server.
     * @return NafId object
     *
     */
    public static NafId createFromNafFqdnAndProtocolId(String nafFqdn,
        byte[] nafSecurProtocolId) {
        NafId nafId = new NafId(nafFqdn, nafSecurProtocolId);
        return nafId;
    }

    @Override
    public int hashCode() {
        return Arrays.hashCode(mNafIdBin);
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj) {
            return true;
        }

        if (obj == null) {
            return false;
        }

        if (getClass() != obj.getClass()) {
            return false;
        }

        NafId other = (NafId) obj;

        if (!Arrays.equals(mNafIdBin, other.getNafIdBin())) {
            return false;
        }

        return true;
    }
}