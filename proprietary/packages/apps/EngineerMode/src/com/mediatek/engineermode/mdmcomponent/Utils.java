package com.mediatek.engineermode.mdmcomponent;

public class Utils {
    private static final String TAG = "EmInfo/MDMLUtils";
    public static long getIntFromByte(byte[] data, int startpos, int len) {
        long ret = 0;

        if (startpos + len > data.length) {
            return -1;
        }
        for (int i = len - 1; i >= 0; --i) {
            ret += data[startpos + i] & 0xFF;
            if (i != 0) {
                ret <<= 8;
            }
        }
        return ret;
    }

    public static long getNegLongFromByte(byte[] data, int startpos, int len) {
        long ret = 0;
        long sign = 1;
        long temp = 0;
        long value = 0;
        long a = 0;

        if (startpos + len > data.length) {
            return -1;
        }

        if ((data[startpos + len - 1] & 0x80) != 0) {
            sign = -1;
        }
        //Elog.d(TAG, "sign in getNegLongFromByte:" + sign);

        for (int i = len - 1; i >= 0; --i) {
            ret += data[startpos + i] & 0xFF;
            if (i != 0) {
                ret <<= 8;
            }
        }
        if (sign != 1) {
            for (int i = 8 - len; i >= 0; --i) {
                temp += 0xFF;
                if (i != 0) {
                    temp <<= 8;
                }
            }
            for (int i = len - 1; i >= 0; --i) {
                temp <<= 8;
            }
        }
            ret |= temp;
        //Elog.d(TAG, "ret in getNegLongFromByte:" + ret);
        return ret;
    }

    public static long getIntFromByte(byte[] data) {
        long result = 0;
        if ((data == null) || (data.length <= 0)) {
            return 0;
        }
        result = getIntFromByte(data, 0, data.length);
        return result;
    }
    public static long getIntFromByte(byte[] data, boolean sign) {
        long result = 0;
        //Elog.d(TAG, "getIntFromByte:2");
        if ((data == null) || (data.length <= 0)) {
            return 0;
        }
        //Elog.d(TAG, "data = " + Arrays.toString(data));
        result = getNegLongFromByte(data, 0, data.length);
        return result;
    }
    public static boolean getBoolFromByte(byte[] data) {
        boolean result = false;
        result = getIntFromByte(data, 0, data.length) == 1;
        return result;
    }
}
