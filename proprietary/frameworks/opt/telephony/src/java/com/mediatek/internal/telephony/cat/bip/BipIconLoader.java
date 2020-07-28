/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.internal.telephony.cat;

import com.android.internal.telephony.cat.ImageDescriptor;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.util.HexDump;

import android.graphics.Bitmap;
import android.graphics.Color;
import android.telephony.TelephonyManager;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import java.util.HashMap;

/**
 * Class for loading icons from the SIM card. Has two states: single, for loading
 * one icon. Multi, for loading icons list.
 *
 */
class BipIconLoader extends Handler {
    private static final String TAG = "Stk-BipIL";
    // members
    private int mState = STATE_SINGLE_ICON;
    private ImageDescriptor mId = null;
    private Bitmap mCurrentIcon = null;
    private int mRecordNumber;
    private IccFileHandler mSimFH = null;
    private Message mEndMsg = null;
    private byte[] mIconData = null;
    // multi icons state members
    private int[] mRecordNumbers = null;
    private int mCurrentRecordIndex = 0;
    private Bitmap[] mIcons = null;
    private HashMap<Integer, Bitmap> mIconsCache = null;
    private int mSlotId;

    private static BipIconLoader[] sLoader = null;
    private static HandlerThread[] sThread = null;
    private static int sSimCount = 0;

    // Loader state values.
    private static final int STATE_SINGLE_ICON = 1;
    private static final int STATE_MULTI_ICONS = 2;

    // Finished loading single record from a linear-fixed EF-IMG.
    private static final int EVENT_READ_EF_IMG_RECOED_DONE  = 1;
    // Finished loading single icon from a Transparent DF-Graphics.
    private static final int EVENT_READ_ICON_DONE           = 2;
    // Finished loading single colour icon lookup table.
    private static final int EVENT_READ_CLUT_DONE           = 3;

    // Color lookup table offset inside the EF.
    private static final int CLUT_LOCATION_OFFSET = 4;
    // CLUT entry size, {Red, Green, Black}
    private static final int CLUT_ENTRY_SIZE = 3;

    private BipIconLoader(Looper looper , IccFileHandler fh, int slotId) {
        super(looper);
        mSimFH = fh;
        mSlotId = slotId;
        mIconsCache = new HashMap<Integer, Bitmap>(50);
    }

    static BipIconLoader getInstance(Handler caller, IccFileHandler fh, int slotId) {
        if (sLoader != null && sLoader[slotId] !=null) {
            return sLoader[slotId];
        }
        if (null == sThread) {
            sSimCount = TelephonyManager.getDefault().getSimCount();
            sThread = new HandlerThread[sSimCount];
            for (int i = 0; i < sSimCount; i++) {
                sThread[i] = null;
            }
        }
        if (null == sLoader) {
            sSimCount = TelephonyManager.getDefault().getSimCount();
            sLoader = new BipIconLoader[sSimCount];
            for (int i = 0; i < sSimCount; i++) {
                sLoader[i] = null;
            }
        }
        if (fh != null) {
            if (null == sThread[slotId]) {
                sThread[slotId] = new HandlerThread("BIP Icon Loader");
                sThread[slotId].start();
            }
            if (null == sLoader[slotId]) {
                if (null != sThread[slotId].getLooper()) {
                    sLoader[slotId] = new BipIconLoader(sThread[slotId].getLooper(), fh, slotId);
                    return sLoader[slotId];
                } else {
                    return null;
                }
            }
        }
        return null;
    }

    void loadIcons(int[] recordNumbers, Message msg) {
        if (recordNumbers == null || recordNumbers.length == 0 || msg == null) {
            return;
        }
        mEndMsg = msg;
        // initialize multi icons load variables.
        mIcons = new Bitmap[recordNumbers.length];
        mRecordNumbers = recordNumbers;
        mCurrentRecordIndex = 0;
        mState = STATE_MULTI_ICONS;
        startLoadingIcon(recordNumbers[0]);
    }

    void loadIcon(int recordNumber, Message msg) {
        if (msg == null) {
            return;
        }
        mEndMsg = msg;
        mState = STATE_SINGLE_ICON;
        startLoadingIcon(recordNumber);
    }

    private void startLoadingIcon(int recordNumber) {
        MtkCatLog.d(TAG, "call startLoadingIcon");
        // Reset the load variables.
        mId = null;
        mIconData = null;
        mCurrentIcon = null;
        mRecordNumber = recordNumber;

        // make sure the icon was not already loaded and saved in the local cache.
        if (mIconsCache.containsKey(recordNumber)) {
            MtkCatLog.d(TAG, "mIconsCache contains record " + recordNumber);
            mCurrentIcon = mIconsCache.get(recordNumber);
            postIcon();
            return;
        }

        // start the first phase ==> loading Image Descriptor.
        MtkCatLog.d(TAG, "to load icon from EFimg");
        readId();
    }

    @Override
    public void handleMessage(Message msg) {
        AsyncResult ar;
        byte[] rawData = null;

        try {
            switch (msg.what) {
            case EVENT_READ_EF_IMG_RECOED_DONE:
                MtkCatLog.d(TAG, "load EFimg done");
                if (msg.obj == null) {
                    MtkCatLog.e(TAG, "msg.obj is null.");
                    break;
                }
                MtkCatLog.d(TAG, "msg.obj is " + msg.obj.getClass().getName());
                ar = (AsyncResult) msg.obj;
                rawData = ((byte[]) ar.result);
                MtkCatLog.d(TAG, "EFimg raw data: " + HexDump.toHexString(rawData));
                if (handleImageDescriptor((byte[]) ar.result)) {
                    readIconData();
                } else {
                    throw new Exception("Unable to parse image descriptor");
                }
                break;
            case EVENT_READ_ICON_DONE:
                MtkCatLog.d(TAG, "load icon done");
                ar = (AsyncResult) msg.obj;
                rawData = ((byte[]) ar.result);
                MtkCatLog.d(TAG, "icon raw data: " + HexDump.toHexString(rawData));
                MtkCatLog.d(TAG, "load icon CODING_SCHEME = " + mId.mCodingScheme);
                if (mId.mCodingScheme == ImageDescriptor.CODING_SCHEME_BASIC) {
                    mCurrentIcon = parseToBnW(rawData, rawData.length);
                    mIconsCache.put(mRecordNumber, mCurrentIcon);
                    postIcon();
                } else if (mId.mCodingScheme == ImageDescriptor.CODING_SCHEME_COLOUR) {
                    mIconData = rawData;
                    readClut();
                } else {
                    MtkCatLog.d(TAG, "else  /postIcon ");
                    postIcon();
                }
                break;
            case EVENT_READ_CLUT_DONE:
                    MtkCatLog.d(TAG, "load clut done");
                ar = (AsyncResult) msg.obj;
                byte [] clut = ((byte[]) ar.result);
                mCurrentIcon = parseToRGB(mIconData, mIconData.length,
                        false, clut);
                mIconsCache.put(mRecordNumber, mCurrentIcon);
                postIcon();
                break;
            }
        } catch (Exception e) {
            MtkCatLog.d(this, "Icon load failed");
            e.printStackTrace();
            // post null icon back to the caller.
            postIcon();
        }
    }

    /**
     * Handles Image descriptor parsing and required processing. This is the
     * first step required to handle retrieving icons from the SIM.
     *
     * @param rawData byte [] containing Image Instance descriptor as defined in
     * TS 51.011.
     */
    private boolean handleImageDescriptor(byte[] rawData) {
        MtkCatLog.d(TAG, "call handleImageDescriptor");
        mId = ImageDescriptor.parse(rawData, 1);
        if (mId == null) {
            MtkCatLog.d(TAG, "fail to parse image raw data");
            return false;
        }
        MtkCatLog.d(TAG, "success to parse image raw data");
        return true;
    }

    // Start reading color lookup table from SIM card.
    private void readClut() {
        int length = mIconData[3] * CLUT_ENTRY_SIZE;
        Message msg = obtainMessage(EVENT_READ_CLUT_DONE);
        mSimFH.loadEFImgTransparent(mId.mImageId,
                mIconData[CLUT_LOCATION_OFFSET],
                mIconData[CLUT_LOCATION_OFFSET + 1], length, msg);
    }

    // Start reading Image Descriptor from SIM card.
    private void readId() {
        MtkCatLog.d(TAG, "call readId");
        if (mRecordNumber < 0) {
            mCurrentIcon = null;
            postIcon();
            return;
        }
        Message msg = obtainMessage(EVENT_READ_EF_IMG_RECOED_DONE);
        mSimFH.loadEFImgLinearFixed(mRecordNumber, msg);
    }

    // Start reading icon bytes array from SIM card.
    private void readIconData() {
        MtkCatLog.d(TAG, "call readIconData");
        Message msg = obtainMessage(EVENT_READ_ICON_DONE);
        mSimFH.loadEFImgTransparent(mId.mImageId, 0, 0, mId.mLength ,msg);
    }

    // When all is done pass icon back to caller.
    private void postIcon() {
        if (mState == STATE_SINGLE_ICON) {
            mEndMsg.obj = mCurrentIcon;
            mEndMsg.sendToTarget();
        } else if (mState == STATE_MULTI_ICONS) {
            mIcons[mCurrentRecordIndex++] = mCurrentIcon;
            // If not all icons were loaded, start loading the next one.
            if (mCurrentRecordIndex < mRecordNumbers.length) {
                startLoadingIcon(mRecordNumbers[mCurrentRecordIndex]);
            } else {
                mEndMsg.obj = mIcons;
                mEndMsg.sendToTarget();
            }
        }
    }

    /**
     * Convert a TS 131.102 image instance of code scheme '11' into Bitmap
     * @param data The raw data
     * @param length The length of image body
     * @return The bitmap
     */
    public static Bitmap parseToBnW(byte[] data, int length){
        int valueIndex = 0;
        int width = data[valueIndex++] & 0xFF;
        int height = data[valueIndex++] & 0xFF;
        int numOfPixels = width*height;

        int[] pixels = new int[numOfPixels];

        int pixelIndex = 0;
        int bitIndex = 7;
        byte currentByte = 0x00;
        while (pixelIndex < numOfPixels) {
            // reassign data and index for every byte (8 bits).
            if (pixelIndex % 8 == 0) {
                currentByte = data[valueIndex++];
                bitIndex = 7;
            }
            pixels[pixelIndex++] = bitToBnW((currentByte >> bitIndex-- ) & 0x01);
        }

        if (pixelIndex != numOfPixels) {
            MtkCatLog.d("BipIconLoader", "parseToBnW; size error");
        }
        return Bitmap.createBitmap(pixels, width, height, Bitmap.Config.ARGB_8888);
    }

    /**
     * Decode one bit to a black and white color:
     * 0 is black
     * 1 is white
     * @param bit to decode
     * @return RGB color
     */
    private static int bitToBnW(int bit){
        if(bit == 1){
            return Color.WHITE;
        } else {
            return Color.BLACK;
        }
    }

    /**
     * a TS 131.102 image instance of code scheme '11' into color Bitmap
     *
     * @param data The raw data
     * @param length the length of image body
     * @param transparency with or without transparency
     * @param clut coulor lookup table
     * @return The color bitmap
     */
    public static Bitmap parseToRGB(byte[] data, int length,
            boolean transparency, byte[] clut) {
        int valueIndex = 0;
        int width = data[valueIndex++] & 0xFF;
        int height = data[valueIndex++] & 0xFF;
        int bitsPerImg = data[valueIndex++] & 0xFF;
        int numOfClutEntries = data[valueIndex++] & 0xFF;

        if (true == transparency) {
            clut[numOfClutEntries - 1] = Color.TRANSPARENT;
        }

        int numOfPixels = width * height;
        int[] pixels = new int[numOfPixels];

        valueIndex = 6;
        int pixelIndex = 0;
        int bitsStartOffset = 8 - bitsPerImg;
        int bitIndex = bitsStartOffset;
        byte currentByte = data[valueIndex++];
        int mask = getMask(bitsPerImg);
        boolean bitsOverlaps = (8 % bitsPerImg == 0);
        while (pixelIndex < numOfPixels) {
            // reassign data and index for every byte (8 bits).
            if (bitIndex < 0) {
                currentByte = data[valueIndex++];
                bitIndex = bitsOverlaps ? (bitsStartOffset) : (bitIndex * -1);
            }
            int clutEntry = ((currentByte >> bitIndex) & mask);
            int clutIndex = clutEntry * CLUT_ENTRY_SIZE;
            pixels[pixelIndex++] = Color.rgb(clut[clutIndex],
                    clut[clutIndex + 1], clut[clutIndex + 2]);
            bitIndex -= bitsPerImg;
        }

        return Bitmap.createBitmap(pixels, width, height,
                Bitmap.Config.ARGB_8888);
    }

    /**
     * Calculate bit mask for a given number of bits. The mask should enable to
     * make a bitwise and to the given number of bits.
     * @param numOfBits number of bits to calculate mask for.
     * @return bit mask
     */
    private static int getMask(int numOfBits) {
        int mask = 0x00;

        switch (numOfBits) {
        case 1:
            mask = 0x01;
            break;
        case 2:
            mask = 0x03;
            break;
        case 3:
            mask = 0x07;
            break;
        case 4:
            mask = 0x0F;
            break;
        case 5:
            mask = 0x1F;
            break;
        case 6:
            mask = 0x3F;
            break;
        case 7:
            mask = 0x7F;
            break;
        case 8:
            mask = 0xFF;
            break;
        }
        return mask;
    }
    public void dispose() {
        int i;
        mSimFH = null;

        if (sThread != null && sThread[mSlotId] != null) {
            sThread[mSlotId].quit();
            sThread[mSlotId] = null;
        }

        if (sLoader != null && sLoader[mSlotId] != null) {
            sLoader[mSlotId] = null;
        }
        // Check if all sThread[] is null
        for (i = 0 ; i < sSimCount ; i++) {
            if (null != sThread[i]) {
                break;
            }
        }
        // All sThread[] has been null, set sThread as null
        if (i == sSimCount) {
            sThread = null;
            sLoader = null;
        }
        mIconsCache = null;
    }
}
