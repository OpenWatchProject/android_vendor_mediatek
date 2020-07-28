/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.campostalgo;

import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

import java.util.HashMap;
import java.util.Map;

/**
 * Used to pass param between app and service.
 */
public class FeatureParam implements Parcelable {
    private static final String TAG = "FeatureParam";

    public static final Parcelable.Creator<FeatureParam> CREATOR =
            new Parcelable.Creator<FeatureParam>() {

                @Override
                public FeatureParam createFromParcel(Parcel source) {
                    try {
                        FeatureParam s = new FeatureParam();
                        s.readFromParcel(source);
                        return s;
                    } catch (Exception e) {
                        Log.e(TAG, "Exception creating FeatureParam from parcel", e);
                        return null;
                    }
                }

                @Override
                public FeatureParam[] newArray(int size) {
                    return new FeatureParam[size];
                }
            };


    private static final int VAL_NULL = -1;
    private static final int VAL_STRING = 0;
    private static final int VAL_INTEGER = 1;
    private static final int VAL_MAP = 2;
    private static final int VAL_BUNDLE = 3;
    private static final int VAL_PARCELABLE = 4;
    private static final int VAL_SHORT = 5;
    private static final int VAL_LONG = 6;
    private static final int VAL_FLOAT = 7;
    private static final int VAL_DOUBLE = 8;
    private static final int VAL_BOOLEAN = 9;
    private static final int VAL_CHARSEQUENCE = 10;
    private static final int VAL_LIST = 11;
    private static final int VAL_SPARSEARRAY = 12;
    private static final int VAL_BYTEARRAY = 13;
    private static final int VAL_STRINGARRAY = 14;
    private static final int VAL_IBINDER = 15;
    private static final int VAL_PARCELABLEARRAY = 16;
    private static final int VAL_OBJECTARRAY = 17;
    private static final int VAL_INTARRAY = 18;
    private static final int VAL_LONGARRAY = 19;
    private static final int VAL_BYTE = 20;
    private static final int VAL_SERIALIZABLE = 21;
    private static final int VAL_SPARSEBOOLEANARRAY = 22;
    private static final int VAL_BOOLEANARRAY = 23;
    private static final int VAL_CHARSEQUENCEARRAY = 24;
    private static final int VAL_PERSISTABLEBUNDLE = 25;
    private static final int VAL_SIZE = 26;
    private static final int VAL_SIZEF = 27;
    private static final int VAL_DOUBLEARRAY = 28;


    private final HashMap<String, Integer> mKeyToPosMap = new HashMap<String, Integer>();
    private static final int kInt32Size = 4;
    //  size + marker
    private static final int kMetaHeaderSize = 2 * kInt32Size;
    // size + id + type
    private static final int kRecordHeaderSize = 3 * kInt32Size;
    // 'M' 'E' 'T' 'A'
    private static final int kMetaMarker = 0x4d455441;
    private Parcel mParcelOutData;
    private int mOutBegin = 0;
    private Parcel mParcelInData;

    /**
     * The contructor.
     */
    public FeatureParam() {
        mParcelOutData = Parcel.obtain();
        mOutBegin = mParcelOutData.dataPosition();
        appendHeader();
    }

    /**
     * Add a integer value to param.
     *
     * @param key   The value Key.
     * @param value The value.
     */
    public void appendInt(String key, int value) {
        int begin = mParcelOutData.dataPosition();
        // 4 int32s: size, key, type, value.
        mParcelOutData.writeInt(-1);
        mParcelOutData.writeString(key);
        mParcelOutData.writeInt(VAL_INTEGER);
        //int beginData = mParcelData.dataPosition();
        mParcelOutData.writeInt(value);
        int end = mParcelOutData.dataPosition();

        //set data Size
        mParcelOutData.setDataPosition(begin);
        mParcelOutData.writeInt(end - begin);
        mParcelOutData.setDataPosition(end);
    }

    /**
     * Add a map value to param.
     *
     * @param key The value key.
     * @param map The value.
     */
    public void appendMap(String key, Map<String, Integer> map) {
        int begin = mParcelOutData.dataPosition();

        // 4 int32s: size, key, type, value.
        mParcelOutData.writeInt(-1);
        mParcelOutData.writeString(key);
        mParcelOutData.writeInt(VAL_MAP);
        // int beginData = mParcelData.dataPosition();
        mParcelOutData.writeMap(map);
        int end = mParcelOutData.dataPosition();

        //set data Size
        mParcelOutData.setDataPosition(begin);
        mParcelOutData.writeInt(end - begin);
        mParcelOutData.setDataPosition(end);
    }

    /**
     * Add a integer array to param.
     *
     * @param key   The value key.
     * @param array The value.
     */
    public void appendIntArray(String key, int[] array) {
        int begin = mParcelOutData.dataPosition();

        // 4 int32s: size, key, type, value.
        mParcelOutData.writeInt(-1);
        mParcelOutData.writeString(key);
        mParcelOutData.writeInt(VAL_INTARRAY);

        //TODO: should use writeIntArray instead of here.
        // for(int i = 0; i< array.length; i++) {
        //     mParcelData.writeInt(array[i]);
        // }
        mParcelOutData.writeIntArray(array);
        int end = mParcelOutData.dataPosition();

        //set data Size
        mParcelOutData.setDataPosition(begin);
        mParcelOutData.writeInt(end - begin);
        mParcelOutData.setDataPosition(end);
    }

    /**
     * Get a integer value.
     *
     * @param key The value key.
     * @return The value.
     */
    public int getInt(final String key) {
        checkType(key, VAL_INTEGER);
        return mParcelInData.readInt();
    }

    /**
     * Get the hash map value.
     * @param key The value key.
     * @return The value.
     */
    public HashMap<String, Integer> getHashMap(final String key) {
        checkType(key, VAL_MAP);
        return mParcelInData.readHashMap(ClassLoader.getSystemClassLoader());
    }

    /**
     * Get the integer array value.
     * @param key The key value.
     * @param value The value.
     */
    public void getIntArray(final String key, int[] value) {
        checkType(key, VAL_INTARRAY);
        mParcelInData.readIntArray(value);
    }


    @Override
    public void writeToParcel(Parcel dest, int flags) {
        int length = updateLength();
        dest.appendFrom(mParcelOutData, 0, length);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    /**
     * Read the param from parcel.
     * @param in The parcel.
     */
    public void readFromParcel(Parcel in) {
        mParcelInData = Parcel.obtain();
        int offset = in.dataPosition();
        int length = in.dataAvail();

        parse(in);
        mParcelInData.appendFrom(in, offset, length);
    }

    private void appendHeader() {
        // Placeholder for the length of the metadata
        mParcelOutData.writeInt(-1);
        mParcelOutData.writeInt(kMetaMarker);


    }

    private int updateLength() {
        int end = mParcelOutData.dataPosition();
        int length = end - mOutBegin;
        mParcelOutData.setDataPosition(mOutBegin);
        mParcelOutData.writeInt(length);
        mParcelOutData.setDataPosition(end);
        return length;
    }

    /**
     * Go over all the records, collecting metadata keys and records'
     * type field offset in the Parcel. These are stored in
     * mKeyToPosMap for latter retrieval.
     * Format of a metadata record:
     * <pre>
     * 1                   2                   3
     * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |                     record size                               |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |                     metadata key                              |  // TITLE
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |                     metadata type                             |  // STRING_VAL
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |                                                               |
     * |                .... metadata payload ....                     |
     * |                                                               |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * </pre>
     *
     * @param parcel    With the serialized records.
     * @param bytesLeft How many bytes in the parcel should be processed.
     * @return false if an error occurred during parsing.
     */
    private boolean scanAllRecords(Parcel parcel, int offset, int bytesLeft) {
        int recCount = 0;
        boolean error = false;
        mKeyToPosMap.clear();
        while (bytesLeft > kRecordHeaderSize) {
            final int start = parcel.dataPosition();
            // Check the size.
            final int size = parcel.readInt();
            if (size <= kRecordHeaderSize) {  // at least 1 byte should be present.
                Log.e(TAG, "Record is too short");
                error = true;
                break;
            }

            // Check the metadata key.
            final String metadataId = parcel.readString();
            // Store the record offset which points to the type
            // field so we can later on read/unmarshall the record
            // payload.
            if (mKeyToPosMap.containsKey(metadataId)) {
                Log.e(TAG, "Duplicate metadata ID found");
                error = true;
                break;
            }

            mKeyToPosMap.put(metadataId, parcel.dataPosition() - offset);

            // Check the metadata type.
            final int metadataType = parcel.readInt();

            // Skip to the next one.
            try {
                parcel.setDataPosition(addOrThrow(start, size));
            } catch (IllegalArgumentException e) {
                Log.e(TAG, "Invalid size: " + e.getMessage());
                error = true;
                break;
            }


            bytesLeft -= size;
            ++recCount;
        }

        if (0 != bytesLeft || error) {
            Log.e(TAG, "Ran out of data or error on record " + recCount);
            mKeyToPosMap.clear();
            return false;
        } else {
            return true;
        }
    }

    /**
     * Check a parcel containing metadata is well formed. The header
     * is checked as well as the individual records format. However, the
     * data inside the record is not checked because we do lazy access
     * (we check/unmarshall only data the user asks for.)
     * <p>
     * Format of a metadata parcel:
     * <pre>
     * 1                   2                   3
     * 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |                     metadata total size                       |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |     'M'       |     'E'       |     'T'       |     'A'       |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * |                                                               |
     * |                .... metadata records ....                     |
     * |                                                               |
     * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     * </pre>
     *
     * @param parcel With the serialized data. Metadata keeps a
     *               reference on it to access it later on. The caller
     *               should not modify the parcel after this call (and
     *               not call recycle on it.)
     * @return false if an error occurred.
     * {@hide}
     */
    private boolean parse(Parcel parcel) {
        if (parcel.dataAvail() < kMetaHeaderSize) {
            Log.e(TAG, "Not enough data " + parcel.dataAvail());
            return false;
        }
        int offset = parcel.dataPosition();
        final int pin = parcel.dataPosition();  // to roll back in case of errors.
        final int size = parcel.readInt();

        // The extra kInt32Size below is to account for the int32 'size' just read.
        if (parcel.dataAvail() + kInt32Size < size || size < kMetaHeaderSize) {
            Log.e(TAG, "Bad size " + size + " avail " + parcel.dataAvail() + " position " + pin);
            parcel.setDataPosition(pin);
            return false;
        }

        // Checks if the 'M' 'E' 'T' 'A' marker is present.
        final int kShouldBeMetaMarker = parcel.readInt();
        if (kShouldBeMetaMarker != kMetaMarker) {
            Log.e(TAG, "Marker missing " + Integer.toHexString(kShouldBeMetaMarker));
            parcel.setDataPosition(pin);
            return false;
        }

        // Scan the records to collect metadata ids and offsets.
        if (!scanAllRecords(parcel, offset, size - kMetaHeaderSize)) {
            parcel.setDataPosition(pin);
            return false;
        }
        return true;
    }

    /**
     * Check the type of the data match what is expected.
     */
    private void checkType(final String key, final int expectedType) {
        final int pos = mKeyToPosMap.get(key);
        mParcelInData.setDataPosition(pos);

        final int type = mParcelInData.readInt();
        if (type != expectedType) {
            throw new IllegalStateException("Wrong type " + expectedType + " but got " + type);
        }
    }

    /**
     * Returns the sum of the two parameters, or throws an exception if the resulting sum would
     * cause an overflow or underflow.
     *
     * @throws IllegalArgumentException when overflow or underflow would occur.
     */
    private static int addOrThrow(int a, int b) throws IllegalArgumentException {
        if (b == 0) {
            return a;
        }

        if (b > 0 && a <= (Integer.MAX_VALUE - b)) {
            return a + b;
        }

        if (b < 0 && a >= (Integer.MIN_VALUE - b)) {
            return a + b;
        }
        throw new IllegalArgumentException("Addition overflow: " + a + " + " + b);
    }


}
