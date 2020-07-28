package com.mediatek.lbs.em2.utils;

import java.io.BufferedOutputStream;
import java.io.DataInputStream;
import java.io.IOException;

public class DataCoder {

    public static void putBoolean(BufferedOutputStream out, boolean data)
            throws IOException {
        putByte(out, (data == true) ? (byte) 1 : (byte) 0);
    }

    public static void putByte(BufferedOutputStream out, byte data)
            throws IOException {
        out.write(data);
    }

    public static void putShort(BufferedOutputStream out, short data)
            throws IOException {
        putByte(out, (byte) (data & 0xff));
        putByte(out, (byte) ((data >> 8) & 0xff));
    }

    public static void putInt(BufferedOutputStream out, int data)
            throws IOException {
        putShort(out, (short) (data & 0xffff));
        putShort(out, (short) ((data >> 16) & 0xffff));
    }

    public static void putLong(BufferedOutputStream out, long data)
            throws IOException {
        putInt(out, (int) (data & 0xffffffff));
        putInt(out, (int) ((data >> 32) & 0xffffffff));
    }

    public static void putFloat(BufferedOutputStream out, float data)
            throws IOException {
        putInt(out, Float.floatToIntBits(data));
    }

    public static void putDouble(BufferedOutputStream out, double data)
            throws IOException {
        putLong(out, Double.doubleToLongBits(data));
    }

    public static void putString(BufferedOutputStream out, String data)
            throws IOException {
        if (data == null) {
            putByte(out, (byte) 0);
        } else {
            putByte(out, (byte) 1);
            byte output[] = data.getBytes();
            putInt(out, output.length + 1);
            out.write(output);
            putByte(out, (byte) 0); // End of string
        }
    }

    public static void putBinary(BufferedOutputStream out, byte data[])
            throws IOException {
        putInt(out, data.length);
        out.write(data);
    }

    public static boolean getBoolean(DataInputStream in) throws IOException {
        return (in.readByte() == 0 ? false : true);
    }

    public static byte getByte(DataInputStream in) throws IOException {
        return in.readByte();
    }

    public static short getShort(DataInputStream in) throws IOException {
        short ret = 0;
        ret |= getByte(in) & 0xff;
        ret |= (getByte(in) << 8);
        return ret;
    }

    public static int getInt(DataInputStream in) throws IOException {
        int ret = 0;
        ret |= getShort(in) & 0xffff;
        ret |= (getShort(in) << 16);
        return ret;
    }

    public static long getLong(DataInputStream in) throws IOException {
        long ret = 0;
        ret |= getInt(in) & 0xffffffffL;
        ret |= ((long) getInt(in) << 32);
        return ret;
    }

    public static float getFloat(DataInputStream in) throws IOException {
        int ret = getInt(in);
        return Float.intBitsToFloat(ret);
    }

    public static double getDouble(DataInputStream in) throws IOException {
        long ret = getLong(in);
        return Double.longBitsToDouble(ret);
    }

    public static String getString(DataInputStream in) throws IOException {
        if (getByte(in) == 0) {
            return null;
        } else {
            int len = getInt(in);
            byte buff[] = new byte[len];
            in.readFully(buff, 0, len);
            return new String(buff).trim();
        }
    }

    public static byte[] getBinary(DataInputStream in) throws IOException {
        int len = getInt(in);
        byte buff[] = new byte[len];
        in.readFully(buff, 0, len);
        return buff;
    }
}
