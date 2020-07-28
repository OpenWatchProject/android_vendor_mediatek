package com.mediatek.digits.utils;

import android.util.Base64;
import android.util.Log;

import javax.crypto.SecretKey;
import javax.crypto.spec.SecretKeySpec;
import javax.crypto.Cipher;

import java.io.BufferedReader;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.InputStreamReader;
import java.io.IOException;
import java.io.UnsupportedEncodingException;

import java.lang.Character;
import java.lang.StringBuilder;
import java.lang.StringBuffer;
import java.lang.IllegalArgumentException;

import java.nio.ByteBuffer;

import java.security.GeneralSecurityException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

import java.text.DateFormat;
import java.text.SimpleDateFormat;

import java.util.Date;
import java.util.Formatter;
import java.util.TimeZone;
import java.util.zip.GZIPInputStream;
import java.util.zip.GZIPOutputStream;

public class Utils {

    static final String TAG = "Digits-Utils";

    public static String bytesToHexString(byte[] bytes) {
        char[] hexArray = "0123456789abcdef".toCharArray();
        char[] hexChars = new char[bytes.length * 2];
        for (int j = 0; j < bytes.length; j++) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = hexArray[v >>> 4];
            hexChars[j * 2 + 1] = hexArray[v & 0x0F];
        }
        return new String(hexChars);
    }

    public static byte[] hexStringToByteArray(String inputString) {
        if (inputString == null) {
            return null;
        }
        int len = inputString.length();
        Log.d(TAG, "hexToBytes: inputLen = " + len);
        byte[] result = new byte[len / 2];
        int[] temp = new int[2];
        for (int i = 0; i < len / 2; i++) {
            temp[0] = inputString.charAt(i * 2);
            temp[1] = inputString.charAt(i * 2 + 1);
            for (int j = 0; j < 2; j++) {
                if (temp[j] >= 'A' && temp[j] <= 'F') {
                    temp[j] = temp[j] - 'A' + 10;
                } else if (temp[j] >= 'a' && temp[j] <= 'f') {
                    temp[j] = temp[j] - 'a' + 10;
                } else if (temp[j] >= '0' && temp[j] <= '9') {
                    temp[j] = temp[j] - '0';
                } else {
                    return null;
                }
            }
            result[i] = (byte) (temp[0] << 4);
            result[i] |= temp[1];
        }
        return result;
    }

    public static String base64Encode(byte[] input) {
        return Base64.encodeToString(input, Base64.NO_WRAP);
    }

    public static String base64Encode(String input) {
        try {
            final byte[] textByte = input.getBytes("UTF-8");
            return Base64.encodeToString(textByte, Base64.NO_WRAP);
        } catch (UnsupportedEncodingException e) {
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
        } catch (UnsupportedEncodingException e) {
            System.out.println("Unsupported character set");
        } catch (IllegalArgumentException e) {
            System.out.println("IllegalArgumentException");
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


    public static  byte[] blowfishEncrypt(String key, String plainText) throws GeneralSecurityException {

        SecretKey secret_key = new SecretKeySpec(key.getBytes(), "Blowfish");

        Cipher cipher = Cipher.getInstance("Blowfish");
        cipher.init(Cipher.ENCRYPT_MODE, secret_key);

        return cipher.doFinal(plainText.getBytes());
    }

    public static  String blowfishDecrypt(String key, byte[] encryptedText) throws GeneralSecurityException {

        SecretKey secret_key = new SecretKeySpec(key.getBytes(), "Blowfish");

        Cipher cipher = Cipher.getInstance("Blowfish");
        cipher.init(Cipher.DECRYPT_MODE, secret_key);

        byte[] decrypted = cipher.doFinal(encryptedText);

        return new String(decrypted);
    }

    public static String md5(String s) {
        try {
            // Create MD5 Hash
            MessageDigest digest = java.security.MessageDigest.getInstance("MD5");
            digest.update(s.getBytes());
            byte messageDigest[] = digest.digest();

            // Create Hex String
            StringBuffer hexString = new StringBuffer();
            for (int i=0; i<messageDigest.length; i++)
                hexString.append(Integer.toHexString(0xFF & messageDigest[i]));
            return hexString.toString();

        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        }
        return "";
    }

    public static String getTimestamp() {
        TimeZone tz = TimeZone.getTimeZone("UTC");
        DateFormat df = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm'Z'"); // Quoted "Z" to indicate UTC, no timezone offset
        df.setTimeZone(tz);
        String nowAsISO = df.format(new Date());
        return nowAsISO;
    }
}
