package com.mediatek.rsu;

import android.content.Context;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.Signature;
import android.text.TextUtils;
import android.util.Log;

import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.util.Arrays;

/**
 * Utils functions
 */
public class RsuUtils {
    private static final String TAG = "RsuUtils";
    private static final char[] HEX_CHARS = {
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };

    public static int
    hexCharToInt(char c) {
        if (c >= '0' && c <= '9') return (c - '0');
        if (c >= 'A' && c <= 'F') return (c - 'A' + 10);
        if (c >= 'a' && c <= 'f') return (c - 'a' + 10);

        throw new RuntimeException ("invalid hex char '" + c + "'");
    }

    /**
     * Converts a hex String to a byte array.
     *
     * @param s A string of hexadecimal characters, must be an even number of
     *          chars long
     *
     * @return byte array representation
     *
     * @throws RuntimeException on invalid format
     */
    public static byte[] hexStringToBytes(String s) {
        byte[] ret;

        if (s == null || s.length() % 2 != 0) {
            loge("The length of hex string is not even numbers");
            return null;
        }

        int sz = s.length();

        ret = new byte[sz/2];

        for (int i=0 ; i <sz ; i+=2) {
            ret[i/2] = (byte) ((hexCharToInt(s.charAt(i)) << 4)
                    | hexCharToInt(s.charAt(i+1)));
        }

        return ret;
    }


    /**
     * Converts a byte array into a String of hexadecimal characters.
     *
     * @param bytes an array of bytes
     *
     * @return hex string representation of bytes array
     */
    public static String
    bytesToHexString(byte[] bytes) {
        if (bytes == null) return null;

        StringBuilder ret = new StringBuilder(2*bytes.length);

        for (int i = 0 ; i < bytes.length ; i++) {
            int b;

            b = 0x0f & (bytes[i] >> 4);

            ret.append(HEX_CHARS[b]);

            b = 0x0f & bytes[i];

            ret.append(HEX_CHARS[b]);
        }

        return ret.toString();
    }

    // Internal error category codes
    // No error, everything is OK
    public static final int NO_ERROR = 0;
    // SLE internal failure (should also have the SLE return code)
    public static final int SLE_FAILURE = 1;
    // Internal failure from the RSU components
    public static final int INTERNAL_FAILURE = 2;
    // HTTP failures
    public static final int SERVER_COMMUNICATION_FAILURE = 3;
    // Remote server error (should also have the server return code)
    public static final int REMOTE_SERVER_ERROR = 4;

    // Vendor codes
    // The vendor code is not available
    public static final int SLE_VENDOR_CODE_NA = -1;
    // Reserved value for the debug adapter
    public static final int SLE_VENDOR_CODE_MOCK = 0;
    // Using the Trustonic adapter
    public static final int SLE_VENDOR_CODE_TRUSTONIC = 1;
    // Using the Qualcomm adapter
    public static final int SLE_VENDOR_CODE_QUALCOMM = 2;
    // Using the Mediatek adapter
    public static final int SLE_VENDOR_CODE_MEDIATEK = 3;

    // Internal return error codes from SLE
    // Reserved APP value that indicates that there is no return code available. SLE vendors
    // should NOT return negative values for errors. In case it does, the adapter shall handle
    // this special return code.
    public static final int SLE_RETURN_CODE_NA = -1;
    // Indicates that everything went fine, no errors
    public static final int SLE_RETURN_CODE_SUCCESS = 0;
    // SLE general error
    public static final int SLE_RETURN_CODE_GENERAL_FAILURE = 1;

    // Internal error message hints

    // SLE_FAILURE
    // Generic SLE failure
    public static final String ERR_MSG_SLE_FAILURE = "ERR_MSG_SLE_FAILURE";
    // SLE time not available
    public static final String ERR_MSG_SLE_TIME_NOT_AVAILABLE = "ERR_MSG_SLE_TIME_NOT_AVAILABLE";

    // INTERNAL_FAILURE
    // Connection to the SLE related components failed
    public static final String RR_MSG_SLE_NOT_REACHABLE = "RR_MSG_SLE_NOT_REACHABLE";
    // Generic internal failure
    public static final String ERR_MSG_INTERNAL_FAILURE = "ERR_MSG_INTERNAL_FAILURE";
    // Malformed server blob
    public static final String ERR_MSG_INVALID_SERVER_BLOB = "ERR_MSG_INVALID_SERVER_BLOB";
    // Malformed SLE adapter blob
    public static final String ERR_MSG_INVALID_SLE_BLOB = "ERR_MSG_INVALID_SLE_BLOB";

    // SERVER_COMMUNICATION_FAILURE
    // Connection to the server failed
    public static final String ERR_MSG_SERVER_NOT_REACHABLE = "ERR_MSG_SERVER_NOT_REACHABLE";
    // There is not data received from the server
    public static final String ERR_MSG_SERVER_NO_CONTENT = "ERR_MSG_SERVER_NO_CONTENT";

    // REMOTE_SERVER_ERROR
    // The server returned an error
    public static final String ERR_MSG_SERVER_ERROR = "ERR_MSG_SERVER_ERROR";
    public static byte[] createGeneralErrorResponse() {
        return createErrorResponseBytes(SLE_VENDOR_CODE_MEDIATEK,
                SLE_FAILURE, SLE_RETURN_CODE_GENERAL_FAILURE, null);
    }

    /**
    * Create error responses.
    *
    * @param sleVendor Vendor code
    * @param errorCode error code
    * @param sleVendorReturnCode the vendor return code
    * @param errorMessage error text message
    *
    * @return the SLE vendor adapter configuration data
    */
    public static byte[] createErrorResponseBytes(int sleVendor, int errorCode,
            int sleVendorReturnCode, String errorMessage) {
        loge("createErrorResponseBytes sleVendor:" + sleVendor + " errorCode:" + errorCode
                + " sleVendorReturnCode:" + sleVendorReturnCode
                + " errorMessage:" + errorMessage);

        /* Return error blob format (ERROR-BLOB)

        Name                Offset  Size(bytes) Description
        Message type        0       1           Constant value = -128(Byte.MIN_VALUE)
        Error code          1       4           The error code
        Vendor code         5       1           The vendor code
                                                1 = Trustonic
                                                2 = Qualcomm
        Vendor return code  6       4           The vendor return code (error code)
        Text message length 10      2           The length of text message in bytes (=i)
        Text message        12      i           The error text message that needs to be
                                                sent to the upper level app.
        */

        int length = 0;
        if (!TextUtils.isEmpty(errorMessage) && (errorMessage.length() % 2 == 0)) {
            length = errorMessage.length() / 2;
        }

        byte[] result = new byte[12 + length];

        // Message type
        result[0] = Byte.MIN_VALUE;
        // Error code
        intToBytes(errorCode, result, 1, 4, true);
        // Vendor code
        result[5] = (byte) sleVendor;
        // Vendor return code
        intToBytes(sleVendorReturnCode, result, 6, 4, true);
        // Text message length
        if (length == 0) {
            result[10] = 0;
            result[11] = 0;
        } else {
            intToBytes(length, result, 10, 2, false);
            byte[] message = hexStringToBytes(errorMessage);
            System.arraycopy(message, 0, result, 12, length);
        }
        return result;
    }

    private static int intToBytes(int value, byte[] dest, int offset, int size, boolean signed) {
        int l = byteNumForInt(value, signed);
        if (l > size) {
            l = size;
        }
        /*
        logd("intToBytes value:" + value + " offset:" + offset + " size:" + size + " signed:"
                + signed + " l:" + l + " dest.length:" + dest.length);
        */

        if (offset < 0 || offset + l > dest.length) {
            throw new IndexOutOfBoundsException("Not enough space to write. Required bytes: " + l);
        }
        for (int i = 0, v = value; i < l; i++, v >>>= 8) {
            byte b = (byte) (v & 0xFF);
            dest[offset + size - 1 - i] = b;
            //logd("intToBytes dest[" + (offset + 4 - 1 - i) + "]:" + dest[offset + 4 - 1 - i]);
        }
        return l;
    }

    private static int byteNumForInt(int value, boolean signed) {
        if (value < 0) {
            throw new IllegalArgumentException("value must be 0 or positive: " + value);
        }
        if (signed) {
            if (value <= 0x7F) {
                return 1;
            } else if (value <= 0x7FFF) {
                return 2;
            } else if (value <= 0x7FFFFF) {
                return 3;
            }
        } else {
            if (value <= 0xFF) {
                return 1;
            } else if (value <= 0xFFFF) {
                return 2;
            } else if (value <= 0xFFFFFF) {
                return 3;
            }
        }
        return 4;
    }

    public static byte[] getSignatureHash(Context context, String packageName) {
        if ((packageName == null) || (packageName.length() == 0)) {
            loge("getCallerSignature fail, packageName is null");
            return null;
        }
        PackageManager pm = context.getPackageManager();
        PackageInfo pi;
        try {
            pi = pm.getPackageInfo(packageName, PackageManager.GET_SIGNATURES);
            if (pi == null) {
                loge("getCallerSignature fail, package information is null for packageName "
                        + packageName);
                return null;
            }
        } catch (PackageManager.NameNotFoundException localNameNotFoundException) {
            loge("getCallerSignature fail, has no packageName " + packageName);
            return null;
        }
        logd("getCallerSignature packageName " + packageName + ", signatures " + pi.signatures);
        if (pi.signatures != null) {
            byte[] certHash256 = getCertHash(pi.signatures[0], "SHA-256");
            logd("Signature hash is: " + certHash256);
            return certHash256;
        }
        return null;
    }

    /**
     * Converts a Signature into a Certificate hash usable for comparison.
     */
    private static byte[] getCertHash(Signature signature, String algo) {
        try {
            MessageDigest md = MessageDigest.getInstance(algo);
            return md.digest(signature.toByteArray());
        } catch (NoSuchAlgorithmException ex) {
            loge("getCertHash NoSuchAlgorithmException " + ex);
        }
        return null;
    }

    private static void logd(String s) {
        Log.d(TAG, "[RSU-SIMLOCK] " + s);
    }

    private static void loge(String s) {
        Log.e(TAG, "[RSU-SIMLOCK] " + s);
    }
}
