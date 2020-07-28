package com.mediatek.lbs.em2.utils;

import java.io.BufferedOutputStream;
import java.io.DataInputStream;
import java.io.IOException;

public class DataCoder2 {
    
    public static class DataCoderBuffer {
        public byte[] mBuff;
        public int mOffset;
        public DataCoderBuffer(int buffSize) {
            mBuff = new byte[buffSize];
            mOffset = 0;
        }
        public void write(byte[] data) {
            for(int i = 0; i < data.length; i++) {
                mBuff[mOffset] = data[i];
                mOffset++;
            }
        }
        public void clear() {
            mOffset = 0;
        }
        
        // for write operation
        public void flush() {
            //TODO need to send the message to the remote side

            //reset
            mOffset = 0;
        }
        
        // for read operation
        public void readFully(byte[] b, int off, int len) {
            for(int i = 0; i < len; i++) {
                b[i] = mBuff[mOffset];
                mOffset++;
            }
        }
    }

    public static void putBoolean(DataCoderBuffer out, boolean data)
            throws IOException {
        putByte(out, (data == true) ? (byte) 1 : (byte) 0);
    }

    public static void putByte(DataCoderBuffer out, byte data)
            throws IOException {
        out.mBuff[out.mOffset] = data;
        out.mOffset++;
    }

    public static void putShort(DataCoderBuffer out, short data)
            throws IOException {
        putByte(out, (byte) (data & 0xff));
        putByte(out, (byte) ((data >> 8) & 0xff));
    }

    public static void putInt(DataCoderBuffer out, int data)
            throws IOException {
        putShort(out, (short) (data & 0xffff));
        putShort(out, (short) ((data >> 16) & 0xffff));
    }

    public static void putLong(DataCoderBuffer out, long data)
            throws IOException {
        putInt(out, (int) (data & 0xffffffff));
        putInt(out, (int) ((data >> 32) & 0xffffffff));
    }

    public static void putFloat(DataCoderBuffer out, float data)
            throws IOException {
        putInt(out, Float.floatToIntBits(data));
    }

    public static void putDouble(DataCoderBuffer out, double data)
            throws IOException {
        putLong(out, Double.doubleToLongBits(data));
    }

    public static void putString(DataCoderBuffer out, String data)
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

    //TODO
    public static void putBinary(DataCoderBuffer out, byte data[])
            throws IOException {
        putInt(out, data.length);
        out.write(data);
    }

    public static boolean getBoolean(DataCoderBuffer in) throws IOException {
        return (getByte(in) == 0 ? false : true);
    }

    public static byte getByte(DataCoderBuffer in) throws IOException {
        byte ret = in.mBuff[in.mOffset];
        in.mOffset++;
        return ret;
    }

    public static short getShort(DataCoderBuffer in) throws IOException {
        short ret = 0;
        ret |= getByte(in) & 0xff;
        ret |= (getByte(in) << 8);
        return ret;
    }

    public static int getInt(DataCoderBuffer in) throws IOException {
        int ret = 0;
        ret |= getShort(in) & 0xffff;
        ret |= (getShort(in) << 16);
        return ret;
    }

    public static long getLong(DataCoderBuffer in) throws IOException {
        long ret = 0;
        ret |= getInt(in) & 0xffffffffL;
        ret |= ((long) getInt(in) << 32);
        return ret;
    }

    public static float getFloat(DataCoderBuffer in) throws IOException {
        int ret = getInt(in);
        return Float.intBitsToFloat(ret);
    }

    public static double getDouble(DataCoderBuffer in) throws IOException {
        long ret = getLong(in);
        return Double.longBitsToDouble(ret);
    }

    public static String getString(DataCoderBuffer in) throws IOException {
        if (getByte(in) == 0) {
            return null;
        } else {
            int len = getInt(in);
            byte buff[] = new byte[len];
            in.readFully(buff, 0, len);
            return new String(buff).trim();
        }
    }

    public static byte[] getBinary(DataCoderBuffer in) throws IOException {
        int len = getInt(in);
        byte buff[] = new byte[len];
        in.readFully(buff, 0, len);
        return buff;
    }
}
