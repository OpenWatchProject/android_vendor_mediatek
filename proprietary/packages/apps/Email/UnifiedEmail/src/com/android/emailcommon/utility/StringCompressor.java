package com.android.emailcommon.utility;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.util.zip.DeflaterInputStream;
import java.util.zip.InflaterInputStream;

import android.text.TextUtils;

import com.android.mail.utils.LogUtils;
/**
 * M: A utility class used to compress and decompress strings
 */
public class StringCompressor {

    private static final String TAG = "StringCompressor";
    private static final int BYTES_OF_KB = 1024;

    /**
     * M: Decompressing from a compressed string to string
     * @param compressedString The compressed string
     * @return Decompressed string
     */
    public static String decompressFromString(String compressedString) {
        if (TextUtils.isEmpty(compressedString)) {
            return compressedString;
        }
        return decompressFromBytes(bytesFromString(compressedString));
    }

    /**
     * M: Decompressing from a compressed bytes to string
     * @param compressedBytes The compressed bytes
     * @return Decompressed string
     */
    public static String decompressFromBytes(byte[] compressedBytes) {
        if (null == compressedBytes) {
            return null;
        }
        long start = System.currentTimeMillis();
        int readCount = 0;
        int originSize = 0;
        byte[] decompressedBytes = null;
        byte[] readBuffer = new byte[BYTES_OF_KB];
        originSize = compressedBytes.length;

        InflaterInputStream compressedStr = new InflaterInputStream(
                new ByteArrayInputStream(compressedBytes));
        ByteArrayOutputStream outStream = new ByteArrayOutputStream();
        try {
            while ((readCount = compressedStr.read(readBuffer)) != -1) {
                outStream.write(readBuffer, 0, readCount);
            }
            decompressedBytes = outStream.toByteArray();

            outStream.close();
            compressedStr.close();
            LogUtils.d(TAG, "Decompressing: originSize = " + originSize
                    + ", decompressedBytes = " + decompressedBytes.length
                    + ", cost: " + (System.currentTimeMillis() - start));
        } catch (IOException e) {
            LogUtils.w(TAG, "Decompressing failed!");
            return null;
        }

        return new String(decompressedBytes);
    }

    /**
     * M: Compressing a string to a compressed string
     * @param originalString The original string
     * @return Compressed string
     */
    public static String compressToString(String originalString) {
        if (TextUtils.isEmpty(originalString)) {
            return originalString;
        }
        byte[] b = compressToBytes(originalString);
        return stringFromBytes(b);
    }

    /**
     * M: Compressing a string to a compressed bytes
     * @param originalString The original string
     * @return Compressed bytes
     */
    public static byte[] compressToBytes(String originalString) {
        if (null == originalString) {
            return null;
        }
        long start = System.currentTimeMillis();
        int readCount = 0;
        int originSize = 0;
        byte[] sourceBytes;
        byte[] compressedBytes;
        byte[] readBuffer = new byte[1024];
        sourceBytes = originalString.getBytes();
        originSize = sourceBytes.length;
        DeflaterInputStream compressedStr
                = new DeflaterInputStream(new ByteArrayInputStream(sourceBytes));
        ByteArrayOutputStream outStream = new ByteArrayOutputStream();
        try {
            while ((readCount = compressedStr.read(readBuffer)) != -1) {
                outStream.write(readBuffer, 0, readCount);
            }
            compressedBytes = outStream.toByteArray();
            outStream.close();
            compressedStr.close();
            LogUtils.d(TAG, "Compressing: originSize = " + originSize
                    + ", compressedBytes = " + compressedBytes.length
                    + ", cost: " + (System.currentTimeMillis() - start));
            return compressedBytes;
        } catch (IOException ioe) {
            LogUtils.w(TAG, "compressBodyData failed!");
        }
        return null;
    }

    /**
     * M: Convert string to bytes
     * @param string The string data
     * @return The byte data
     */
    public static byte[] bytesFromString(String string) {
        byte[] bs = new byte[string.length()];
            for (int i = 0; i < string.length(); i++) {
                bs[i] = (byte) string.charAt(i);
            }
        return bs;
    }

    /**
     * M: Convert bytes to string
     * @param bytes The byte data
     * @return The string data
     */
    public static String stringFromBytes(byte[] bytes) {
        if (bytes == null) {
            return null;
        }
        StringBuilder sb = new StringBuilder();
        for (byte b : bytes) {
            sb.append((char) b);
        }
        String s = sb.toString();
        return s;
    }
}
