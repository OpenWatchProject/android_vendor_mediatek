/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.android.camera;

import android.util.Log;

import com.mediatek.camera.v2.exif.ExifInterface;

import java.io.IOException;

public class Exif {
    private static final String TAG = "CameraExif";

    public static ExifInterface getExif(byte[] jpegData) {
        ExifInterface exif = new ExifInterface();
        try {
            exif.readExif(jpegData);
        } catch (IOException e) {
            Log.w(TAG, "Failed to read EXIF data", e);
        }
        return exif;
    }

    // Returns the degrees in clockwise. Values are 0, 90, 180, or 270.
    public static int getOrientation(ExifInterface exif) {
        Integer val = exif.getTagIntValue(ExifInterface.TAG_ORIENTATION);
        if (val == null) {
            return 0;
        } else {
            return ExifInterface.getRotationForOrientationValue(val.shortValue());
        }
    }

    public static int getOrientation(byte[] jpegData) {
        if (jpegData == null)
            return 0;
        ExifInterface exif = getExif(jpegData);
        return getOrientation(exif);
    }

    // group index
    public static int getGroupIndex(ExifInterface exif) {
        Integer val = exif.getTagIntValue(ExifInterface.TAG_GROUP_INDEX);
        if (val == null) {
            return 0;
        } else {
            Log.i(TAG, "group index = " + val.intValue());
            return val.intValue();
        }
    }

    public static int getGroupIndex(byte[] jpegData) {
        if (jpegData == null)
            return 0;
        ExifInterface exif = getExif(jpegData);
        return getGroupIndex(exif);
    }

    // group id
    public static long getGroupId(ExifInterface exif) {
        Long val = exif.getTagLongValue(ExifInterface.TAG_GROUP_ID);
        if (val == null) {
            return 0L;
        } else {
            Log.i(TAG, "group id = " + val.longValue());
            return val.longValue();
        }
    }

    public static long getGroupId(byte[] jpegData) {
        if (jpegData == null)
            return 0L;
        ExifInterface exif = getExif(jpegData);
        return getGroupId(exif);
    }

    // focus value low
    public static long getFocusValueLow(ExifInterface exif) {
        Long val = exif.getTagLongValue(ExifInterface.TAG_FOCUS_VALUE_HIGH);
        if (val == null) {
            return 0L;
        } else {
            Log.i(TAG, "focus value low = " + val.longValue());
            return val.longValue();
        }
    }

    public static long getFocusValueLow(byte[] jpegData) {
        if (jpegData == null)
            return 0L;
        ExifInterface exif = getExif(jpegData);
        return getFocusValueLow(exif);
    }

    // focus value high
    public static long getFocusValueHigh(ExifInterface exif) {
        Long val = exif.getTagLongValue(ExifInterface.TAG_FOCUS_VALUE_LOW);
        if (val == null) {
            return 0L;
        } else {
            Log.i(TAG, "focus value high = " + val.longValue());
            return val.longValue();
        }
    }

    public static long getFocusValueHigh(byte[] jpegData) {
        if (jpegData == null)
            return 0L;
        ExifInterface exif = getExif(jpegData);
        return getFocusValueHigh(exif);
    }
}
