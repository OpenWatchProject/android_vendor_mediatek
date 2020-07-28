package com.mediatek.entitlement;

import android.util.Base64;
import android.util.Log;

import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStreamReader;
import java.io.IOException;
import java.io.UnsupportedEncodingException;

import java.lang.Character;
import java.lang.StringBuilder;

import java.nio.ByteBuffer;

import java.util.Formatter;
import java.util.zip.GZIPInputStream;
import java.util.zip.GZIPOutputStream;

public class Utils {

    static final String TAG = "Entitlement-Utils";

    public static String bytesToHexString(byte[] bytes) {
        StringBuilder sb = new StringBuilder(bytes.length * 2);

        Formatter formatter = new Formatter(sb);
        for (byte b : bytes) {
            formatter.format("%02x", b);
        }

        return sb.toString();
    }

    public static byte[] hexStringToByteArray(String s) {
        int len = s.length();
        byte[] data = new byte[len / 2];
        for (int i = 0; i < len; i += 2) {
            data[i / 2] = (byte) ((Character.digit(s.charAt(i), 16) << 4)
                    + Character.digit(s.charAt(i+1), 16));
        }
        return data;
    }

    public static String base64Encode(byte[] input) {
        return Base64.encodeToString(input, Base64.NO_WRAP);
    }

    public static String base64Encode(String input) {
        try {
            final byte[] textByte = input.getBytes("UTF-8");
            return Base64.encodeToString(textByte, Base64.NO_WRAP);
        } catch (UnsupportedEncodingException e){
            Log.d(TAG, "base64Encode: Unsupported character set");
        }
        return null;
    }

    public static String base64Encode(int input) {
        Log.d(TAG, "base64Encode: input:" + input);
        ByteBuffer newBuf = ByteBuffer.allocate(8);
        newBuf.putInt(input);
        byte[] bytes = newBuf.array();
        return Base64.encodeToString(bytes, Base64.DEFAULT);
    }

    public static byte[] base64Decode(String input) {
        try {
            final byte[] textByte = input.getBytes("UTF-8");
            return Base64.decode(textByte, Base64.DEFAULT);
        } catch (UnsupportedEncodingException e){
            System.out.println("Unsupported character set");
        }
        return null;
    }

    public static byte[] gzipCompress(final String str) {
        if ((str == null) || (str.length() == 0)) {
            return null;
        }

        try {
            ByteArrayOutputStream obj = new ByteArrayOutputStream();
            GZIPOutputStream gzip = new GZIPOutputStream(obj);
            gzip.write(str.getBytes("UTF-8"));
            gzip.close();
            return obj.toByteArray();
        } catch (IOException e) {
            Log.d(TAG, "gzipCompress: exception:" + e);
            return null;
        }
    }

    public static String gzipDecompress(final byte[] compressed) {
        String outStr = "";
        if ((compressed == null) || (compressed.length == 0)) {
            return "";
        }

        if (isCompressed(compressed)) {
            try {
                GZIPInputStream gis = new GZIPInputStream(new ByteArrayInputStream(compressed));
                BufferedReader bufferedReader = new BufferedReader(new InputStreamReader(gis, "UTF-8"));

                String line;
                while ((line = bufferedReader.readLine()) != null) {
                    outStr += line;
                }
                gis.close();
            } catch (IOException e) {
                Log.d(TAG, "gzipCompress: exception:" + e);
                return null;
            }
        } else {
            outStr = new String(compressed);
        }
        return outStr;
    }

    public static boolean isCompressed(final byte[] compressed) {
        return (compressed[0] == (byte) (GZIPInputStream.GZIP_MAGIC)) && (compressed[1] == (byte) (GZIPInputStream.GZIP_MAGIC >> 8));
    }
}
