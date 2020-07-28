package com.mediatek.gba.element;

import android.util.Log;

import com.mediatek.gba.GbaConstant;

import java.util.Arrays;

/**
 * implementation for AkaResponse.
 *
 * @hide
 */
public class AkaResponse {
    private static final String TAG = "AkaResponse";
    private byte[] mCk;
    private byte[] mIk;
    private byte[] mAuts;
    private byte[] mRes;

    private static final byte OK_AKA_RESPONSE = (byte) 0xDB;
    private static final byte FAILED_AKA_RESPONSE = (byte) 0xDC;

    /**
     * Constructs an AkaResponse class.
     *
     * @param response the AKA response from UICC
     * @param gbaType the type of GBA procedure
     *
     */
    public AkaResponse(byte[] response, int gbaType) {
        mCk = null;
        mIk = null;
        mRes = null;
        mAuts = null;

        //"Successful 3G authentication" tag = 'DB' from TS 31.103 or 31.102
        //AT+ESIMAUTH=1,1,"dd109565c13a21635978949d7a3d76eeb36710629a8003876800005b6d276c58f259f8"
        //+ESIMAUTH: 144, 0, "DB08BA951159C06BDDB8"
        if (response[0] == OK_AKA_RESPONSE) {
            int resLen = (int) response[1];
            mRes = new byte[resLen];
            mRes = Arrays.copyOfRange(response, 2, 2 + resLen);

            int ckLen = 0;
            int ikLen = 0;

            if (gbaType == GbaConstant.GBA_ME) {
                ckLen = (int) response[2 + resLen];
                mCk = new byte[ckLen];
                mCk = Arrays.copyOfRange(response, 3 + resLen, 3 + resLen + ckLen);
                ikLen = (int) response[3 + resLen + ckLen];
                mIk = new byte[ikLen];
                mIk = Arrays.copyOfRange(response, 4 + resLen + ckLen, 4 + resLen + ckLen + ikLen);
            }

            Log.i(TAG, "Decode done:" + resLen + ":" + ckLen + ":" + ikLen);

        } else if (response[0] == FAILED_AKA_RESPONSE) {
            int autsLen = (int) response[1];
            mAuts = new byte[autsLen];
            mAuts = Arrays.copyOfRange(response, 2, 2 + autsLen);
            Log.i(TAG, "Decode done:" + autsLen);
            Log.i(TAG, "AUTS:"  + Arrays.toString(mAuts));
        }
    }


    public byte[] getRes() {
        return mRes;
    }

    public byte[] getAuts() {
        return mAuts;
    }

    public byte[] getCk() {
        return mCk;
    }

    public byte[] getIk() {
        return mIk;
    }

    @Override
    public String toString() {
        return "AkaResponse [ck=" + Arrays.toString(mCk) + ", ik=" + Arrays.toString(mIk)
               + ", auts=" + Arrays.toString(mAuts) + ", res=" + Arrays.toString(mRes) + "]";
    }

}