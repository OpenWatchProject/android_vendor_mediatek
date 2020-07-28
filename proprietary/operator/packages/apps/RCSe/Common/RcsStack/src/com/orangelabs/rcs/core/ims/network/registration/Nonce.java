package com.orangelabs.rcs.core.ims.network.registration;

import android.util.Base64;
import android.util.Log;

import java.lang.IllegalArgumentException;
import java.util.Arrays;


public class Nonce {
    private static final String TAG = "Nonce";
    
    private final static int RAND_LEN = 16;
    private final static int AUTN_LEN = 16;

    private byte[] mRand;
    private byte[] mAutn;
    private byte[] mServerSpecificData;
    private String mRawData;

    public Nonce(byte[] rand, byte[] autn, byte[] serverSpecificData,String rawData) {        
        if (rand == null) {
            throw new IllegalArgumentException("rand must be not null.");
        }
        if (autn == null) {
            throw new IllegalArgumentException("autn must be not null.");
        }
        mRand = rand;
        mAutn = autn;
        mServerSpecificData = serverSpecificData;
        mRawData = rawData;
    }

    public byte[] getRand() {
        return mRand;
    }

    public byte[] getAutn() {
        return mAutn;
    }

    public byte[] getServerSpecificData() {
        return mServerSpecificData;
    }

    public String getValueStr() {
        return mRawData;
    }

    @Override
    public int hashCode() {        
        int result = 1;
        result = 31 * result + Arrays.hashCode(mAutn);
        result = 37 * result + Arrays.hashCode(mRand);
        result = 43 * result + Arrays.hashCode(mServerSpecificData);
        result = 47 * result + ((mRawData == null) ? 0 : mRawData.hashCode());
        return result;
    }

    @Override
    public boolean equals(Object obj) {
        if (this == obj)
            return true;
        if (obj == null)
            return false;
        if (getClass() != obj.getClass())
            return false;
        Nonce other = (Nonce) obj;
        if (!Arrays.equals(mAutn, other.mAutn))
            return false;
        if (!Arrays.equals(mRand, other.mRand))
            return false;
        if (!Arrays.equals(mServerSpecificData, other.mServerSpecificData))
            return false;
        if (mRawData == null) {
            if (other.mRawData != null)
                return false;
        } else if (!mRawData.equals(other.mRawData))
            return false;
        return true;
    }

    public static String byteArrayToHexString(byte[] b) {
        if (b == null)
            return "";
        StringBuffer sb = new StringBuffer(b.length * 2);
        for (int i = 0; i < b.length; i++) {
            int v = b[i] & 0xff;
            if (v < 16) {
                sb.append('0');
            }
            sb.append(Integer.toHexString(v));
        }
        return sb.toString().toUpperCase();
    }

    @Override
    public String toString() {
        return "Nonce [rand=" + byteArrayToHexString(mRand) + ", autn=" + byteArrayToHexString(mAutn)
                + ", serverSpecificData=" + byteArrayToHexString(mServerSpecificData)
                + ", valueStr=" + mRawData + "]";
    }

    public static Nonce decodeNonce(String nonceStr) {
        if (nonceStr == null || nonceStr.length() == 0) {
            throw new IllegalArgumentException("nonceStr must be not empty.");
        }
        
        byte[] decodedData = Base64.decode(nonceStr, Base64.DEFAULT);        
        if (decodedData.length < RAND_LEN + AUTN_LEN) {
            throw new IllegalStateException("The length of decoded content is less then required.");
        }
        
        final byte[] rand = new byte[RAND_LEN];
        final byte[] autn = new byte[AUTN_LEN];
        System.arraycopy(decodedData, 0, rand, 0, RAND_LEN);
        System.arraycopy(decodedData, RAND_LEN, autn, 0, AUTN_LEN);

        byte[] serverSpecificData = null;
        int serverSpecificDataLen = decodedData.length - (RAND_LEN + AUTN_LEN);

        if (serverSpecificDataLen > 0) {
            serverSpecificData = new byte[serverSpecificDataLen];
            System.arraycopy(decodedData, RAND_LEN + AUTN_LEN, serverSpecificData, 0,
                    serverSpecificDataLen);
        }

        Nonce nonce = new Nonce(rand, autn, serverSpecificData, nonceStr);

        return nonce;
    }
    
    /*
    public static String encodeNonce(Nonce nonce) {
        if (nonce == null) {
            throw new IllegalArgumentException("nonceStr must be not empty.");
        }

        byte[] rand = nonce.getRand();
        byte[] autn = nonce.getAutn();
        byte[] serverSpecificData = nonce.getServerSpecificData();

        return nonce;
    }
    */
    
}
