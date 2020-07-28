package com.mediatek.socket.base;

import java.io.UnsupportedEncodingException;
import java.util.Arrays;

public class SocketUtils {

    public interface Codable {
        public void encode(BaseBuffer _buff);

        public Codable decode(BaseBuffer _buff);

        public Codable[] getArray(Codable[] data);
    }

    public interface UdpServerInterface {
        public boolean read();

        public BaseBuffer getBuff();

        public boolean setSoTimeout(int timeout);
    }

    public interface ProtocolHandler {
        public abstract int getProtocolType();

        public abstract boolean decode(UdpServerInterface server);
    }

    public static class BaseBuffer {
        private int mOffset;
        private byte[] mBuff;

        public BaseBuffer(int buffSize) {
            mOffset = 0;
            mBuff = new byte[buffSize];
        }

        public String toString() {
            StringBuilder o = new StringBuilder();
            o.append("BaseBuffer ");
            o.append("offset=[" + mOffset + "] ");
            o.append("maxSize=[" + mBuff.length + "] ");
            return o.toString();
        }

        public void setOffset(int position) {
            mOffset = position;
        }

        public int getOffset() {
            return mOffset;
        }

        public byte[] getBuff() {
            return mBuff;
        }

        public int size() {
            return mBuff.length;
        }

        public void clear() {
            mOffset = 0;
            Arrays.fill(mBuff, (byte) 0);
        }

        public void putBool(boolean data) {
            putByte(data ? (byte) 1 : (byte) 0);
        }

        public void putByte(byte data) {
            mBuff[mOffset] = data;
            mOffset++;
        }

        public void putShort(short data) {
            putByte((byte) (data & 0xff));
            putByte((byte) ((data >> 8) & 0xff));
        }

        public void putInt(int data) {
            putShort((short) (data & 0xffff));
            putShort((short) ((data >> 16) & 0xffff));
        }

        public void putLong(long data) {
            putInt((int) (data & 0xffffffff));
            putInt((int) ((data >> 32) & 0xffffffff));
        }

        public void putFloat(float data) {
            putInt(Float.floatToIntBits(data));
        }

        public void putDouble(double data) {
            putLong(Double.doubleToLongBits(data));
        }

        public void putString(String data) {
            assertNotNull(data);
            try {
                byte output[] = data.getBytes("utf8");
                putInt(output.length);
                System.arraycopy(output, 0, mBuff, mOffset, output.length);
                mOffset += output.length;
            } catch (UnsupportedEncodingException e) {
                e.printStackTrace();
            }
        }

        public void putCodable(Codable data) {
            assertNotNull(data);
            data.encode(this);
        }

        public void putArrayBool(boolean[] data) {
            assertNotNull(data);
            putInt(data.length);
            for (int i = 0; i < data.length; i++) {
                putBool(data[i]);
            }
        }

        public void putArrayByte(byte[] data) {
            assertNotNull(data);
            putInt(data.length);
            System.arraycopy(data, 0, mBuff, mOffset, data.length);
            mOffset += data.length;
        }

        public void putArrayShort(short[] data) {
            assertNotNull(data);
            putInt(data.length);
            for (int i = 0; i < data.length; i++) {
                putShort(data[i]);
            }
        }

        public void putArrayInt(int[] data) {
            assertNotNull(data);
            putInt(data.length);
            for (int i = 0; i < data.length; i++) {
                putInt(data[i]);
            }
        }

        public void putArrayLong(long[] data) {
            assertNotNull(data);
            putInt(data.length);
            for (int i = 0; i < data.length; i++) {
                putLong(data[i]);
            }
        }

        public void putArrayFloat(float[] data) {
            assertNotNull(data);
            putInt(data.length);
            for (int i = 0; i < data.length; i++) {
                putFloat(data[i]);
            }
        }

        public void putArrayDouble(double[] data) {
            assertNotNull(data);
            putInt(data.length);
            for (int i = 0; i < data.length; i++) {
                putDouble(data[i]);
            }
        }

        public void putArrayString(String[] data) {
            assertNotNull(data);
            putInt(data.length);
            for (int i = 0; i < data.length; i++) {
                putString(data[i]);
            }
        }

        public void putArrayCodable(Codable[] data) {
            assertNotNull(data);
            putInt(data.length);
            for (int i = 0; i < data.length; i++) {
                putCodable(data[i]);
            }
        }

        public boolean getBool() {
            return (getByte() == 0) ? false : true;
        }

        public byte getByte() {
            byte ret = mBuff[mOffset];
            mOffset++;
            return ret;
        }

        public short getShort() {
            short ret = 0;
            ret |= getByte() & 0xff;
            ret |= (getByte() << 8);
            return ret;
        }

        public int getInt() {
            int ret = 0;
            ret |= getShort() & 0xffff;
            ret |= (getShort() << 16);
            return ret;
        }

        public long getLong() {
            long ret = 0;
            ret |= getInt() & 0xffffffffL;
            ret |= ((long) getInt() << 32);
            return ret;
        }

        public float getFloat() {
            int ret = getInt();
            return Float.intBitsToFloat(ret);
        }

        public double getDouble() {
            long ret = getLong();
            return Double.longBitsToDouble(ret);
        }

        public String getString() {
            int len = getInt();
            byte buff[] = new byte[len];
            System.arraycopy(mBuff, mOffset, buff, 0, len);
            mOffset += len;
            try {
                return new String(buff, "utf8").trim();
            } catch (UnsupportedEncodingException e) {
                e.printStackTrace();
            }
            return null;
        }

        public Codable getCodable(Codable instance) {
            return instance.decode(this);
        }

        public boolean[] getArrayBool() {
            int len = getInt();
            boolean[] out = new boolean[len];
            for (int i = 0; i < len; i++) {
                out[i] = getBool();
            }
            return out;
        }

        public byte[] getArrayByte() {
            int len = getInt();
            byte buff[] = new byte[len];
            System.arraycopy(mBuff, mOffset, buff, 0, len);
            mOffset += len;
            return buff;
        }

        public short[] getArrayShort() {
            int len = getInt();
            short[] out = new short[len];
            for (int i = 0; i < len; i++) {
                out[i] = getShort();
            }
            return out;
        }

        public int[] getArrayInt() {
            int len = getInt();
            int[] out = new int[len];
            for (int i = 0; i < len; i++) {
                out[i] = getInt();
            }
            return out;
        }

        public long[] getArrayLong() {
            int len = getInt();
            long[] out = new long[len];
            for (int i = 0; i < len; i++) {
                out[i] = getLong();
            }
            return out;
        }

        public float[] getArrayFloat() {
            int len = getInt();
            float[] out = new float[len];
            for (int i = 0; i < len; i++) {
                out[i] = getFloat();
            }
            return out;
        }

        public double[] getArrayDouble() {
            int len = getInt();
            double[] out = new double[len];
            for (int i = 0; i < len; i++) {
                out[i] = getDouble();
            }
            return out;
        }

        public String[] getArrayString() {
            int len = getInt();
            String[] out = new String[len];
            for (int i = 0; i < len; i++) {
                out[i] = getString();
            }
            return out;
        }

        public Codable[] getArrayCodable(Codable instance) {
            int len = getInt();
            Codable[] out = new Codable[len];
            for (int i = 0; i < len; i++) {
                out[i] = getCodable(instance);
            }
            return out;
        }

        private void assertNotNull(Object object) {
            if (object == null) {
                throw new RuntimeException("assertNotNull() failed");
            }
        }
    }

    public static void assertSize(Object data, int maxSize, int maxSize2) {
        int size = 0;
        if (data instanceof boolean[]) {
            size = ((boolean[]) data).length;
        } else if (data instanceof byte[]) {
            size = ((byte[]) data).length;
        } else if (data instanceof short[]) {
            size = ((short[]) data).length;
        } else if (data instanceof int[]) {
            size = ((int[]) data).length;
        } else if (data instanceof long[]) {
            size = ((long[]) data).length;
        } else if (data instanceof float[]) {
            size = ((float[]) data).length;
        } else if (data instanceof double[]) {
            size = ((double[]) data).length;
        } else if (data instanceof String[]) {
            size = ((String[]) data).length;
            for (String s : (String[]) data) {
                if (s.length() > maxSize2) {
                    throw new RuntimeException("your string.length="
                            + s.length() + " is more than maxSize2=" + maxSize2);
                }
            }
        } else if (data instanceof Object[]) {
            size = ((Object[]) data).length;
        } else if (data instanceof String) {
            size = ((String) data).length();
        }

        if (size > maxSize) {
            throw new RuntimeException("your size=" + size
                    + " is more than maxSize=" + maxSize);
        }
    }

    public static void assertEqual(Object o1, Object o2) {
        if (o1 instanceof boolean[]) {
            boolean[] t1 = (boolean[]) o1;
            boolean[] t2 = (boolean[]) o2;
            if (!Arrays.equals(t1, t2)) {
                throw new RuntimeException("t1=[" + Arrays.toString(t1)
                        + "] is not equal to t2=[" + Arrays.toString(t2) + "]");
            }
        } else if (o1 instanceof byte[]) {
            byte[] t1 = (byte[]) o1;
            byte[] t2 = (byte[]) o2;
            if (!Arrays.equals(t1, t2)) {
                throw new RuntimeException("t1=[" + Arrays.toString(t1)
                        + "] is not equal to t2=[" + Arrays.toString(t2) + "]");
            }
        } else if (o1 instanceof short[]) {
            short[] t1 = (short[]) o1;
            short[] t2 = (short[]) o2;
            if (!Arrays.equals(t1, t2)) {
                throw new RuntimeException("t1=[" + Arrays.toString(t1)
                        + "] is not equal to t2=[" + Arrays.toString(t2) + "]");
            }
        } else if (o1 instanceof int[]) {
            int[] t1 = (int[]) o1;
            int[] t2 = (int[]) o2;
            if (!Arrays.equals(t1, t2)) {
                throw new RuntimeException("t1=[" + Arrays.toString(t1)
                        + "] is not equal to t2=[" + Arrays.toString(t2) + "]");
            }
        } else if (o1 instanceof long[]) {
            long[] t1 = (long[]) o1;
            long[] t2 = (long[]) o2;
            if (!Arrays.equals(t1, t2)) {
                throw new RuntimeException("t1=[" + Arrays.toString(t1)
                        + "] is not equal to t2=[" + Arrays.toString(t2) + "]");
            }
        } else if (o1 instanceof float[]) {
            float[] t1 = (float[]) o1;
            float[] t2 = (float[]) o2;
            if (!Arrays.equals(t1, t2)) {
                throw new RuntimeException("t1=[" + Arrays.toString(t1)
                        + "] is not equal to t2=[" + Arrays.toString(t2) + "]");
            }
        } else if (o1 instanceof double[]) {
            double[] t1 = (double[]) o1;
            double[] t2 = (double[]) o2;
            if (!Arrays.equals(t1, t2)) {
                throw new RuntimeException("t1=[" + Arrays.toString(t1)
                        + "] is not equal to t2=[" + Arrays.toString(t2) + "]");
            }
        } else if (o1 instanceof String[]) {
            String[] t1 = (String[]) o1;
            String[] t2 = (String[]) o2;
            if (!Arrays.equals(t1, t2)) {
                throw new RuntimeException("t1=[" + Arrays.toString(t1)
                        + "] is not equal to t2=[" + Arrays.toString(t2) + "]");
            }
        } else if (o1 instanceof Object[]) {
            Object[] t1 = (Object[]) o1;
            Object[] t2 = (Object[]) o2;
            if (!Arrays.equals(t1, t2)) {
                throw new RuntimeException("t1=[" + Arrays.toString(t1)
                        + "] is not equal to t2=[" + Arrays.toString(t2) + "]");
            }
        } else {
            if (!o1.equals(o2)) {
                throw new RuntimeException("o1=[" + o1
                        + "] is not equal to o2=[" + o2 + "]");
            }
        }
    }
}
