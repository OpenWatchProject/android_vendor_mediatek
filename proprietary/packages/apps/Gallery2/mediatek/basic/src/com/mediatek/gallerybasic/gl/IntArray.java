package com.mediatek.gallerybasic.gl;

public class IntArray {
    private static final int INIT_CAPACITY = 8;

    private int mData[] = new int[INIT_CAPACITY];
    private int mSize = 0;

    public void add(int value) {
        if (mData.length == mSize) {
            int temp[] = new int[mSize + mSize];
            System.arraycopy(mData, 0, temp, 0, mSize);
            mData = temp;
        }
        mData[mSize++] = value;
    }

    public int removeLast() {
        mSize--;
        return mData[mSize];
    }

    public int size() {
        return mSize;
    }

    // For testing only
    public int[] toArray(int[] result) {
        if (result == null || result.length < mSize) {
            result = new int[mSize];
        }
        System.arraycopy(mData, 0, result, 0, mSize);
        return result;
    }

    public int[] getInternalArray() {
        return mData;
    }

    public void clear() {
        mSize = 0;
        if (mData.length != INIT_CAPACITY) mData = new int[INIT_CAPACITY];
    }
}
