/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2014. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.ct.deviceregister.dm.utils;

import android.content.Context;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.util.Log;

import com.ct.deviceregister.dm.Const;
import com.mediatek.internal.telephony.IMtkTelephonyEx;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Locale;

public class EsnWrapper {

    private static final String TAG = Const.TAG_PREFIX + "EsnWrapper";

    private static final String MEID_TO_PESN_HASH_NAME = "SHA-1";
    private static final String PREFIX_PESN = "80";
    private static final int LENGTH_ESN = 8;
    private static final int LENGTH_MEID = Const.LENGTH_MEID;

    public static String getEsnOrMeidFromUim(int slotId) {
        int readCommand = 176;
        int length = 8;
        byte[] response = doCmdOnSlotId(slotId, readCommand, length, null);

        return processResponsefromUim(response);
    }

    private static String processResponsefromUim(byte[] rawBytes) {
        // Need to ignore suffix 90 00
        String esnOrMeid = processResponseWithSuffix(rawBytes, 2);

        Log.i(TAG, "processEsnfromUim " + PlatformManager.encryptMessage(esnOrMeid));
        return esnOrMeid;
    }

    public static String processResponseFromFW(String rawEsnOrMeid) {
        byte[] rawBytes = Utils.hexStringToBytes(rawEsnOrMeid);
        String esnOrMeid = processResponseWithSuffix(rawBytes, 0);

        Log.i(TAG, "processEsnFromFW " + PlatformManager.encryptMessage(esnOrMeid));
        return esnOrMeid;
    }

    /**
     * Extract pEsn/MEID from rawBytes
     * @param response - the raw byte data of response
     * @param suffix - the length of suffix
     * @return if info is pESN, will return value like 04abcdefgh000000
     *         if info is MEID, will return value like 07abcdefghijklmn,
     *         if error happen, return null.
     */
    private static String processResponseWithSuffix(byte[] response, int suffix) {
        String result = "";
        if (response != null && response.length > 2) {
            Log.i(TAG, "Process response " + encryptByteMessage(response));

            int realLength = 0;

            if (response[0] == 0x04) {
                realLength = response.length - 3 - suffix;

            } else {
                realLength = response.length - suffix;
            }

            byte[] realResult = new byte[realLength];
            System.arraycopy(response, 0, realResult, 0, realLength);
            result = Utils.bytesToHexString(extractInfo(realResult));
            if (result != null) {
                result = result.toUpperCase(Locale.ENGLISH);
            }
        } else {
            Log.e(TAG, "Response is " + response);
        }

        return result;
    }

    /*
     * extract info from "length + reversed bytes"
     */
    private static byte[] extractInfo(byte[] byteSrc) {
        byte[] resultByte = new byte[byteSrc.length - 1];

        for (int i = 0, j = byteSrc.length - 1; j > 0; j--, i++) {
            resultByte[i] = byteSrc[j];
        }

        return resultByte;
    }

    /*
     * length + reversed bytes
     */
    private static byte[] reverseAndAddHeader(byte[] byteSrc) {
        byte[] resultByte = new byte[byteSrc.length + 1];
        int i = 0;
        resultByte[i] = (byte) byteSrc.length;

        for (int j = byteSrc.length - 1; j >= 0; j--) {
            i++;
            resultByte[i] = byteSrc[j];
        }

        return resultByte;
    }

    public static void setEsnToUim(int slotId, String esnOrMeid) {
        String pEsn = convertToEsn(esnOrMeid);
        Log.i(TAG, "write pESN " + PlatformManager.encryptMessage(pEsn) + " to uim.");

        int writeCommand = 222;

        byte[] pEsnByte = Utils.hexStringToBytes(pEsn);
        byte[] pEsnByteReverse = reverseAndAddHeader(pEsnByte);
        int reverseLength = pEsnByteReverse.length;

        String pEsnReverseHex = Utils.bytesToHexString(pEsnByteReverse);

        // print return value, framework may need it when debug
        byte[] writeResult = doCmdOnSlotId(slotId, writeCommand, reverseLength, pEsnReverseHex);
        Log.i(TAG, "Write pEsn result " + encryptByteMessage(writeResult));
    }

    private static byte[] doCmdOnSlotId(int slotId, int command, int length, String value) {
        byte[] response = null;

        int fileId = 0x6F38;
        String path = "3F007F25";

        IMtkTelephonyEx iTel = IMtkTelephonyEx.Stub.asInterface(
                ServiceManager.getService(Context.TELEPHONY_SERVICE));
        int[] subId = PlatformManager.getSubId(slotId);

        if (PlatformManager.isSubIdsValid(subId)) {
            try {
                response = iTel.iccExchangeSimIOEx(subId[0], fileId,
                        command, 0, 0, length, path, value, null);

            } catch (RemoteException e) {
                Log.e(TAG, "RemoteException " + e.getMessage());
                e.printStackTrace();
            }
        }
        return response;
    }

    public static String convertToEsn(String esnOrMeid) {
        // ESN or pESN
        if (esnOrMeid.length() == LENGTH_ESN) {
            return esnOrMeid;

        } else {
            // MEID, convert to pESN
            return convertMeidToEsn(esnOrMeid);
        }
    }

    private static String convertMeidToEsn(String meid) {
        if (!PlatformManager.isInfoValid(meid, LENGTH_MEID)) {
            return "";
        }

        String pEsn = "";
        try {

            byte[] meidByte = Utils.hexStringToBytes(meid);
            MessageDigest md = MessageDigest.getInstance(MEID_TO_PESN_HASH_NAME);
            md.update(meidByte);
            String result = Utils.bytesToHexString(md.digest());
            int length = result.length();

            if (length > 6) {
                pEsn = PREFIX_PESN + result.substring(length - 6, length);
            } else {
                Log.e(TAG, "digest result length < 6, it is not valid:" + result);
            }

        } catch (NoSuchAlgorithmException e) {
            Log.e(TAG, "No such algorithm:" + MEID_TO_PESN_HASH_NAME);
            e.printStackTrace();
        }

        if (pEsn != null) {
            pEsn = pEsn.toUpperCase(Locale.ENGLISH);
        }
        return pEsn;
    }

    private static String encryptByteMessage(byte[] data) {
        return PlatformManager.encryptMessage(Utils.bytesToHexString(data));
    }
}
