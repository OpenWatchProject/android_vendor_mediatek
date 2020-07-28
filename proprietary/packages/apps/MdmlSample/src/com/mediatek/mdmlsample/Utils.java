package com.mediatek.mdmlsample;

public class Utils {
    public static long GetIntFromByte(byte [] data, int startpos, int len) {
        long ret = 0;

        if (startpos + len > data.length) {
            return -1;
        }
        for (int i = len - 1; i >= 0; --i) {
            ret += data[startpos + i] & 0xFF;
            if (i != 0 ) {
                ret <<= 8;
            }
        }
        return ret;
    }
}
