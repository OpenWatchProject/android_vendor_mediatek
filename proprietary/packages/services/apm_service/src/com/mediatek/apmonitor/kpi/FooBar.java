package com.mediatek.apmonitor.kpi;

import java.util.ArrayList;

// Extends abstract class KpiObjBaseRoot for the first node of data structure,
// and specifiy version in constructor, and we will prepend 4 bytes version
// when build() is called.
// For the inner KPI data structure referenced by root, please extends
// abstract class KpiObjBase instead of KpiObjBaseRoot.
// Need to implement serialize() interface called in build() flow to
// generate byte[] payload data.
public class FooBar extends KpiObjBaseRoot {
    // Version code is important, receiver implement payload parser and
    // parse based on corresponding version code.
    // Increase version code if the structure of KPI changed, otherwise,
    // you have to make sure your structure is backward compatible across
    // system and vendor partition.
    private static final int VERSION = 1;

    // Fields
    // We need to have length(L) field for the value(V) so that
    // receiver side can parse it if it is dynamic length of memory block.
    public byte mByte;
    public Integer mBytesLen; // L
    public byte[] mBytes;
    public boolean mBoolean;
    public Short mShort;
    public Integer mInteger;
    public Float mFloat;
    public Long mLong;
    public Double mDouble;
    public Integer mStringLen; // L
    public String mString;
    private Integer mInnerListSize = 0; // L
    ArrayList<FooBarInner> mInnerList;

    public FooBar() {
        // Must call super constructor and assgin version.
        super(VERSION);
        mInnerList = new ArrayList<FooBarInner>();
    }

    // There may be some inner structure
    public void AddInner(Integer foo, Integer bar) {
        FooBarInner inner = new FooBarInner();
        inner.mFoo = foo;
        inner.mBar = bar;
        mInnerList.add(inner);
        mInnerListSize++; // Shall be alinged to mInnerList.size()
    }

    // Implement to flat KPI fields to byte stream.
    @Override
    public void serialize() {
        writeByte(mByte); // 1
        writeInteger(mBytesLen); // 4
        writeBoolean(mBoolean); // 1
        writeShort(mShort); // 2
        writeInteger(mInteger); // 4
        writeFloat(mFloat); // 4
        writeLong(mLong); // 8
        writeDouble(mDouble); // 8
        // Add 1 byte for the end byte.
        writeIntegerForStringLen(mString); // 4
        writeInteger(mInnerList.size());

        // Put the dynamic memory block in the tail
        writeByteArray(mBytes.length, mBytes); // x
        // writeString() will append 0x00 in the tail.
        writeString(mString); // x

        for (int i = 0; i < mInnerList.size(); i++) {
            FooBarInner inner = mInnerList.get(i);
            byte[] bytes = inner.build();
            writeByteArray(bytes.length, bytes);
        }
    }

    // No need to implement this currently.
    @Override
    public void deserialize() {
        // Not implemented
    }

    // Inner data class need to extends class KpiObjBase
    public class FooBarInner extends KpiObjBase {
        public Integer mFoo;
        public Integer mBar;

        @Override
        public void serialize() {
            writeInteger(mFoo);
            writeInteger(mBar);
        }

        @Override
        public void deserialize() {
            // Not implemented
        }
    }
}
