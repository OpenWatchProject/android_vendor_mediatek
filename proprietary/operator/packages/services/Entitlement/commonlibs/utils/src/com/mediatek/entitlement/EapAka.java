package com.mediatek.entitlement;

import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;
import com.mediatek.entitlement.Utils;


import javax.crypto.Mac;
import javax.crypto.spec.SecretKeySpec;
import java.security.InvalidKeyException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class EapAka {
    private static final String TAG = "Entitlement-EapAka";
    private static final boolean DEBUG = true;

    static {
        System.loadLibrary("eap-aka");
    }

    public static native byte[] getEapAkaAuthKey(byte[] masterkey);

    public static String getImsiEap(TelephonyManager telephonyManager) {
        return getImsiEapWithSlotId(telephonyManager, 0);
    }

    // Support for Dual SIM
    public static String getImsiEapWithSlotId(TelephonyManager telephonyManager, int slotId) {

        String imsi, mccMnc;

        if (isGeminiSupport(telephonyManager)) {

            int subId = getSubId(slotId);

            imsi = telephonyManager.getSubscriberId(subId);
            mccMnc = telephonyManager.getSimOperator(subId);
        } else {

            imsi = telephonyManager.getSubscriberId();
            mccMnc = telephonyManager.getSimOperator();
        }

        if (imsi == null || imsi.isEmpty()) {
            Log.e(TAG, "[" + slotId + "] " + "getImsiEap: cannot get imsi, return directly");
            return null;
        }

        if (mccMnc == null || mccMnc.length() < 5) {
            Log.e(TAG, "[" + slotId + "] " + "getImsiEap: cannot get mccMnc, return directly");
            return null;
        }

        int len = mccMnc.length();
        String mcc = mccMnc.substring(0, 3);
        String mnc = mccMnc.substring(3, len);
        if (len == 5) {
            mnc = "0" + mnc;
        }
        return "0" + imsi + "@nai.epc.mnc" + mnc + ".mcc" + mcc + ".3gppnetwork.org";
    }

    public static String doEapAkaAuthentication(
        TelephonyManager telephonyManager, String rfc4187AkaChallenge) {

        return doEapAkaAuthenticationWithSlotId(telephonyManager, rfc4187AkaChallenge, 0);
    }


    // Support for Dual SIM
    public static String doEapAkaAuthenticationWithSlotId(
        TelephonyManager telephonyManager, String rfc4187AkaChallenge, int slotId) {

        String akaChallenge_forSIM = transformRfc4187ToTs31102(rfc4187AkaChallenge, slotId);

        log("[" + slotId + "]" + "received aka-challenge from SES server: " + rfc4187AkaChallenge);
        log("[" + slotId + "]" + "try EAP-AKA with SIM: " + akaChallenge_forSIM);

        boolean dualSim = isGeminiSupport(telephonyManager);
        int subId = -1;

        String akaResponseFromSIM = getIccAuthentication(
            telephonyManager, TelephonyManager.APPTYPE_USIM, akaChallenge_forSIM, slotId);

        if (akaResponseFromSIM == null) {
            log("[" + slotId + "]" + "retry with ISIM!!");

            akaResponseFromSIM = getIccAuthentication(
                telephonyManager, TelephonyManager.APPTYPE_ISIM, akaChallenge_forSIM, slotId);
        }

        if (akaResponseFromSIM == null) {
            log("[" + slotId + "]" + "retry with SIM!!");

            akaResponseFromSIM = getIccAuthentication(
                telephonyManager, TelephonyManager.APPTYPE_SIM, akaChallenge_forSIM, slotId);
        }

        String akaResponse = calculateRfc4187AkaResponse(
            getImsiEapWithSlotId(telephonyManager, slotId), akaResponseFromSIM, slotId);

        log("[" + slotId + "]" + "received aka-response from SIM: " + akaResponseFromSIM);
        log("[" + slotId + "]" + "response aka-response to SES server: " + akaResponse);

        return akaResponse;
    }

    private static String getIccAuthentication(
        TelephonyManager telephonyManager, int appType, String akaChallenge_forSIM, int slotId) {

        if (isGeminiSupport(telephonyManager)) {

            int subId = getSubId(slotId);
            if (subId == SubscriptionManager.INVALID_SUBSCRIPTION_ID) {

                Log.e(TAG, "[" + slotId + "]" + "getIccAuthentication(), invalid subId !");
                return null;
            }

            return telephonyManager.getIccAuthentication(
                    subId,
                    appType,
                    TelephonyManager.AUTHTYPE_EAP_AKA,
                    akaChallenge_forSIM);

        } else {

            return telephonyManager.getIccAuthentication(
                    appType,
                    TelephonyManager.AUTHTYPE_EAP_AKA,
                    akaChallenge_forSIM);
        }
    }

    /**
      * @get SimConfig by TelephonyManager.getDefault().getMultiSimConfiguration().
      * @return true if the device has 2 or more slots
      */
    private static boolean isGeminiSupport(TelephonyManager telephonyManager) {
        TelephonyManager.MultiSimVariants simConfig = telephonyManager.getMultiSimConfiguration();

        //log("isGeminiSupport(), simConfig:" + simConfig);

        if (simConfig == TelephonyManager.MultiSimVariants.DSDS ||
            simConfig == TelephonyManager.MultiSimVariants.DSDA) {
            return true;
        }
        return false;
    }

    private static int getSubId(int slotId) {
        int[] subIds = SubscriptionManager.getSubId(slotId);
        if (subIds != null) {
            return subIds[0];
        } else {
            return SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        }
    }

    private static String transformRfc4187ToTs31102(String akaChallenge, int slotId) {
        log("[" + slotId + "]" + "transformRfc4187ToTs31102: input: " + akaChallenge);

        byte decodedOutput[] = Utils.base64Decode(akaChallenge);
        if (decodedOutput == null) {
            return null;
        }
        log("[" + slotId + "]" + "transformRfc4187ToTs31102: decoded: " + Utils.bytesToHexString(decodedOutput));

        /*
                Rfc4187: First 8 bytes for EAP message:

                01: Code 1: EAP-Request
                02: Identifier: 2
                0080: Length: 128 bytes, 126 bytes effective Data fields.

                17: EAP Type = AKA
                01: SubType: AKA-Challenge
                0000: Reserved
        */
        if (decodedOutput[0] != 1 && decodedOutput[1] != 2 &&
                decodedOutput[4] != 23 && decodedOutput[5] != 1) {
            Log.e(TAG, "[" + slotId + "]" + "transformRfc4187ToTs31102: invalid input");
            return null;
        }

        /*
                TS 31102:

                L1 (1 byte): length of RAND
                RAND (L1 byte)
                L2 (1 byte): length of AUTN
                AUTN (L2 byte)
         */
        byte[] outputBytedata = new byte[34];    // 1+16+1+16
        outputBytedata[0] = 16; // L1
        outputBytedata[17] = 16; // L2

        // Now we start to find AT_RAND/AT_AUTN from decoded AKA-Challenge data.

        int index = 8;  // start from the 8th byte.
        while (index < decodedOutput.length) {
            int attributeType = decodedOutput[index];
            int attributeLength = decodedOutput[index + 1];

            if (attributeType == 1) {
                /*
                    Byte#0 = 01: Attribute Type: AT_RAND
                    Byte#1 = size. (should be 5)
                    Byte#2~Byte#3: reserved.
                    Byte#4~Byte#19: data.
                */
                if (attributeLength != 5) {
                    Log.e(TAG, "[" + slotId + "]" + "transformRfc4187ToTs31102: invalid AT_RAND length");
                    return null;
                }

                System.arraycopy(decodedOutput, index + 4, outputBytedata, 1, 16);
            } else if (attributeType == 2) { // 02: Attribute Type: AT_AUTN
                /*
                    Byte#0 = 01: Attribute Type: AT_AUTN
                    Byte#1 = size. (should be 5)
                    Byte#2~Byte#3: reserved.
                    Byte#4~Byte#19: data.
                */
                if (attributeLength != 5) {
                    Log.e(TAG, "[" + slotId + "]" + "transformRfc4187ToTs31102: invalid AT_AUTN length");
                    return null;
                }

                System.arraycopy(decodedOutput, index + 4, outputBytedata, 18, 16);
            }

            index += (attributeLength * 4);
        }

        log("[" + slotId + "]" + "transformRfc4187ToTs31102: output(TS-31102): " + Utils.bytesToHexString(outputBytedata));

        return Utils.base64Encode(outputBytedata);
    }

    private static String calculateRfc4187AkaResponse(String identity, String ts31102AkaResponse, int slotId) {
        log("[" + slotId + "]" + "calculateAkaResponse: input: " + ts31102AkaResponse);
        if (ts31102AkaResponse == null) {
            return null;
        }

        byte decodedOutput[] = Utils.base64Decode(ts31102AkaResponse);
        if (decodedOutput == null) {
            return null;
        }
        log("[" + slotId + "]" + "calculateAkaResponse: decoded: " + Utils.bytesToHexString(decodedOutput));

        /*
         TS 31102:
         tag: 'DB' means successful.
         L3 (1 byte): length of RES
         RES (L3 byte)
         L4 (1 byte): length of CK
         CK (L4 byte)
         L5 (1 byte): length of IK
         IK (L5 byte)
        */

        int tag = decodedOutput[0];

        if (tag != (byte) 0xdb) {
            Log.e(TAG, "[" + slotId + "]" + "calculateAkaResponse: EAP-AKA is failed, tag=" + tag);
            return null;
        }

        int resLength = decodedOutput[1];
        int ckLength = decodedOutput[resLength + 2];
        int ikLength = decodedOutput[resLength + ckLength + 3];

        byte[] res = new byte[resLength];
        byte[] ck = new byte[ckLength];
        byte[] ik = new byte[ikLength];

        System.arraycopy(decodedOutput, 2, res, 0, resLength);
        System.arraycopy(decodedOutput, resLength + 3, ck, 0, ckLength);
        System.arraycopy(decodedOutput, resLength + ckLength + 4, ik, 0, ikLength);

        log("[" + slotId + "]" + "res:" + Utils.bytesToHexString(res));
        log("[" + slotId + "]" + "ck:" + Utils.bytesToHexString(ck));
        log("[" + slotId + "]" + "ik:" + Utils.bytesToHexString(ik));

        /*
         Rfc4187: First 8 bytes for EAP message:

         01: Code 1: EAP-Response (=2)
         02: Identifier: 2
         00xx: Length.

         17: EAP Type = AKA
         01: SubType: AKA-Challenge
         0000: Reserved

         AT_RES code: 3
         length of RES in word (1byte)
         length of RES in bits (2byte)
         RES data

         AT_MAC code: 0B
         05: length always to be 5
         0000: Reserved
         MAC data

         So total bytes are 8 + (RES_BYTES + 4) + 5*4
        */

        int resAttributeSize = resLength + 4;
        int macAttributeSize = 20;  // Spec specific.
        int outputSize = 8 + resAttributeSize + macAttributeSize;
        //int outputSize = 8 + resAttributeSize;
        byte[] outputBytes = new byte[outputSize];
        outputBytes[0] = outputBytes[1] = 2;
        outputBytes[2] = (byte) (outputSize >> 8 & 0xFF);
        outputBytes[3] = (byte) (outputSize & 0xFF);

        outputBytes[4] = 23;
        outputBytes[5] = 1;
        // outputBytes[6] = outputBytes[7] = 0; //reserved.

        // AT_RES
        outputBytes[8] = 3;
        outputBytes[9] = (byte) (resLength / 4 + 1 + (resLength % 4 == 0 ? 0 : 1));

        int resLengthInBits = resLength * 8;
        outputBytes[10] = (byte) (resLengthInBits >> 8 & 0xFF);
        outputBytes[11] = (byte) (resLengthInBits & 0xFF);
        System.arraycopy(res, 0, outputBytes, 12, resLength);

        // AT_MAC
        int macIndex = 12 + resLength;
        outputBytes[macIndex] = 11;
        outputBytes[macIndex + 1] = 5;

        byte[] k_aut = getEapAkaAuthKey(getEapAkaMasterKey(identity.getBytes(), ck, ik, slotId));

        log("[" + slotId + "]" + "k_aut is :" + Utils.bytesToHexString(k_aut));

        byte[] mac = calculateRFC2104HMAC(outputBytes, k_aut, slotId);
        log("[" + slotId + "]" + "mac is :" + Utils.bytesToHexString(mac));

        System.arraycopy(mac, 0, outputBytes, macIndex + 4, 16);

        log("[" + slotId + "]" + "calculateAkaResponse: output(RFC4187): " + Utils.bytesToHexString(outputBytes));
        return Utils.base64Encode(outputBytes);
    }

    private static byte[] getEapAkaMasterKey(byte[] identity, byte[] ck, byte[] ik, int slotId) {
        byte[] mk = null;

        byte[] rawData = new byte[identity.length + ck.length + ik.length];
        // concatenation of Identity|IK|CK.
        System.arraycopy(identity, 0, rawData, 0, identity.length);
        System.arraycopy(ik, 0, rawData, identity.length, ik.length);
        System.arraycopy(ck, 0, rawData, identity.length + ik.length, ck.length);
        log("[" + slotId + "]" + "getEapAkaMasterKey: input is " + Utils.bytesToHexString(rawData));

        try {
            MessageDigest md;
            md = MessageDigest.getInstance("SHA-1");
            md.update(rawData);
            mk = md.digest();

            log("[" + slotId + "]" + "MasterKey: " + Utils.bytesToHexString(mk));
        } catch (NoSuchAlgorithmException e) {
            Log.e(TAG, "[" + slotId + "]" + "getEapAkaMasterKey: exception-", e);
        }
        return mk;
    }

    private static byte[] calculateRFC2104HMAC(byte[] data, byte[] key, int slotId) {
        try {
            SecretKeySpec signingKey = new SecretKeySpec(key, "HmacSHA1");
            Mac mac = Mac.getInstance("HmacSHA1");
            mac.init(signingKey);
            return mac.doFinal(data);
        } catch (NoSuchAlgorithmException e) {
            Log.e(TAG, "[" + slotId + "]" + "calculateRFC2104HMAC: NoSuchAlgorithmException- ", e);
        } catch (InvalidKeyException e) {
            Log.e(TAG, "[" + slotId + "]" + "calculateRFC2104HMAC: InvalidKeyException- ", e);
        }
        return null;
    }

    private static void log(String s) {
        if (DEBUG) Log.d(TAG, s);
    }
}
