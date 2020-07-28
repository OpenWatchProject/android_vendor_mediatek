package com.mediatek.apmonitor.kpi;

import android.util.Log;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public abstract class KpiObjBase implements Serializable {
    private static final String TAG = "KpiObjBase";
    protected ByteArrayOutputStream mBytes = new ByteArrayOutputStream();

    public KpiObjBase() {};

    public byte[] build() {
        // Call serialize() implementation
        serialize();
        return mBytes.toByteArray();
    }

    public int currentWriteBytes() {
        return mBytes.size();
    }

    public void close() {
        try {
            mBytes.close();
        } catch (IOException ex) {
            // Close failed!
            Log.e(TAG, "close " + this.getClass().getName() + " failed!");
        }
    }

    // 1 byte
    protected KpiObjBase writeByte(byte x) {
        ByteBuffer buffer = ByteBuffer.allocate(Byte.BYTES);
        buffer.put(x);
        mBytes.write(buffer.array(), 0, Byte.BYTES);
        return this;
    }

    // bytes of len
    protected KpiObjBase writeByteArray(int len, byte[] x) {
        ByteBuffer buffer = ByteBuffer.wrap(x);
        mBytes.write(buffer.array(), 0, len);
        return this;
    }

    // 1 byte
    protected KpiObjBase writeBoolean(boolean x) {
        ByteBuffer buffer = ByteBuffer.allocate(Byte.BYTES);
        if (x) {
           buffer.put((byte)1);
        } else {
           buffer.put((byte)0);
        }
        mBytes.write(buffer.array(), 0, Byte.BYTES);
        return this;
    }

    // 2 byte
    protected KpiObjBase writeShort(short x) {
        ByteBuffer buffer = ByteBuffer.allocate(Short.BYTES);
        buffer.order(ByteOrder.LITTLE_ENDIAN);
        buffer.putShort(x);
        mBytes.write(buffer.array(), 0, Short.BYTES);
        return this;
    }

    // 4 bytes
    protected KpiObjBase writeInteger(int x) {
        ByteBuffer buffer = ByteBuffer.allocate(Integer.BYTES);
        buffer.order(ByteOrder.LITTLE_ENDIAN);
        buffer.putInt(x);
        mBytes.write(buffer.array(), 0, Integer.BYTES);
        return this;
    }

    // 4 bytes
    protected KpiObjBase writeFloat(float x) {
        ByteBuffer buffer = ByteBuffer.allocate(Float.BYTES);
        buffer.order(ByteOrder.LITTLE_ENDIAN);
        buffer.putFloat(x);
        mBytes.write(buffer.array(), 0, Float.BYTES);
        return this;
    }

    // 8 bytes
    protected KpiObjBase writeLong(Long x) {
        if (x == null) return this;

        ByteBuffer buffer = ByteBuffer.allocate(Long.BYTES);
        buffer.order(ByteOrder.LITTLE_ENDIAN);
        buffer.putLong(x);
        mBytes.write(buffer.array(), 0, Long.BYTES);
        return this;
    }

    // 8 bytes
    protected KpiObjBase writeDouble(Double x) {
        if (x == null) return this;

        ByteBuffer buffer = ByteBuffer.allocate(Double.BYTES);
        buffer.order(ByteOrder.LITTLE_ENDIAN);
        buffer.putDouble(x);
        mBytes.write(buffer.array(), 0, Double.BYTES);
        return this;
    }

    // bytes of len
    protected KpiObjBase writeString(String x) {
        if (x == null) return this;

        byte[] str = x.getBytes();
        if (str.length != 0) {
            writeByteArray(str.length, str);
            writeByte((byte)0x00);
        }
        return this;
    }

    protected KpiObjBase writeIntegerForStringLen(String x) {
        // Null String
        if (x == null) {
            writeInteger(0);
            return this;
        }
        // Empty String
        byte[] str = x.getBytes();
        if (str.length != 0) {
            // Plus 1 for end char 0x00
            writeInteger(str.length + 1);
        } else {
            writeInteger(0);
        }
        return this;
    }
}