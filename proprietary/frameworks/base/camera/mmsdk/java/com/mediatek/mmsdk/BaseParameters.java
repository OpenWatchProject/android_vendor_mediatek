/*
 * Copyright (C) 2013 The Android Open Source Project
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

package com.mediatek.mmsdk;

import java.util.HashMap;
import java.util.List;

import android.os.Parcel;
import android.os.Parcelable;

import android.graphics.Rect;
import android.util.Log;
import android.text.TextUtils;


/**
 * Information about a camera
 *
 * @hide
 */

/*
public class BaseParameter implements Parcelable {
    
    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(left);
        out.writeInt(top);
        out.writeInt(right);
        out.writeInt(bottom);
    }

    public void readFromParcel(Parcel in) {
        left = in.readInt();
        top = in.readInt();
        right = in.readInt();
        bottom = in.readInt();
    }

    public static final Parcelable.Creator<Rect> CREATOR =
            new Parcelable.Creator<Rect>() {
        @Override
        public Rect createFromParcel(Parcel in) {
            return new Rect(in);
        }

        @Override
        public Rect[] newArray(int size) {
            return new Rect[size];
        }
    };

    private Rect(Parcel in) {
        left = in.readInt();
        top = in.readInt();
        right = in.readInt();
        bottom = in.readInt();
    }
};
*/


public class BaseParameters implements Parcelable {
    private static final String TAG = "BaseParameters";

    public static final String CAMERA_MM_SERVICE_BINDER_NAME = "media.mmsdk";
    public static final String KEY_EFFECT_NAME_CFB = "capture_face_beauty";
    public static final String KEY_EFFECT_NAME_HDR = "hdr";
    public static final String KEY_PICTURE_SIZE = "picture-size";
    public static final String KEY_PICTURE_WIDTH = "picture-width";
    public static final String KEY_PICTURE_HEIGHT = "picture-height";

    public static final String KEY_IMAGE_FORMAT = "picture-format";
    
    public static final String KEY_FACE_BEAUTY_SHAPE = "fb-sharp";
    public static final String KEY_FACE_BEAUTY_SHAPE_MAX = "fb-sharp-max";
    public static final String KEY_FACE_BEAUTY_SHAPE_MIN = "fb-sharp-min";
    
    public static final String KEY_FACE_BEAUTY_SKIN_COLOR = "fb-skin-color";
    public static final String KEY_FACE_BEAUTY_SKIN_COLOR_MAX = "fb-skin-color-max";
    public static final String KEY_FACE_BEAUTY_SKIN_COLOR_MIN = "fb-skin-color-min";
    
    public static final String KEY_FACE_BEAUTY_SMOOTH = "fb-smooth-level";
    public static final String KEY_FACE_BEAUTY_SMOOTH_MAX = "fb-smooth-level-max";
    public static final String KEY_FACE_BEAUTY_SMOOTH_MIN = "fb-smooth-level-min";
    
    public static final String KEY_FACE_BEAUTY_SLIM_FACE = "fb-slim-face";
    public static final String KEY_FACE_BEAUTY_SLIM_FACE_MAX = "fb-slim-face-max";
    public static final String KEY_FACE_BEAUTY_SLIM_FACE_MIN = "fb-slim-face-min";
    
    public static final String KEY_OUT_PUT_CAPTURE_NUMBER   = "picture-number";
    
    public static final String KEY_PICTURE_ROTATION         = "rotation";

    //for callback jpeg
    public static final String KEY_CALLBACK_MSG_TYPE    = "callback-msg-type";

    /**
     * Order matters: Keys that are {@link #set(String, String) set} later
     * will take precedence over keys that are set earlier (if the two keys
     * conflict with each other).
     *
     * <p>One example is {@link #setPreviewFpsRange(int, int)} , since it
     * conflicts with {@link #setPreviewFrameRate(int)} whichever key is set later
     * is the one that will take precedence.
     * </p>
     */
    private HashMap<String, String> mMap;

//-----------------------------------------------------------------------------	
//parcelable
//-----------------------------------------------------------------------------	
    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel out, int flags) {
    	//@todo implement this
    	Log.i(TAG, "writeToParcel");
		out.writeString(flatten());
    }

    public void readFromParcel(Parcel in) {
    	//@todo implement this
    	Log.i(TAG, "readFromParcel");
        mMap = new HashMap<String, String>(128);

		int dataSize = in.dataSize();
		int dataPosition = in.dataPosition();
    	Log.i(TAG, "readFromParcel - in.dataSize " + dataSize);
    	Log.i(TAG, "readFromParcel - in.dataPosition " + dataPosition);
		byte[] marshell = in.createByteArray();
		for(int i=0; i<marshell.length; i++)  {
			char c = (char)marshell[i];
			Log.i(TAG, i + " - " + marshell[i] + ", " + c);
		}

		in.setDataPosition(dataPosition);
    	Log.i(TAG, "readFromParcel - in.dataPosition2 " + in.dataPosition());
		int totalSize = in.readInt();
		Log.i(TAG, "totalSize=" + totalSize);
		String string = in.readString();
		if(string != null) {
			Log.i(TAG, "readFromParcel - string=" + string);
			unflatten(string);
		} else {
	    	Log.e(TAG, "can't read string from parcel");
		}
    }

	public static final Parcelable.Creator<BaseParameters> CREATOR =
            new Parcelable.Creator<BaseParameters>() {
        @Override
        public BaseParameters createFromParcel(Parcel in) {
	    	Log.i(TAG, "createFromParcel");
            return new BaseParameters(in);
        }

        @Override
        public BaseParameters[] newArray(int size) {
	    	Log.i(TAG, "newArray");
            return new BaseParameters[size];
        }
    };

    private BaseParameters(Parcel in) {
    	Log.i(TAG, "BaseParameters(Parcel in)");
		readFromParcel(in);
    }

//-----------------------------------------------------------------------------	
//BaseParameters
//-----------------------------------------------------------------------------	
    //private BaseParameters() {
    public BaseParameters() {
        mMap = new HashMap<String, String>(128);
    }

    /**
     * Clone parameter from current settings.
     * @hide
     * @return the clone parameter
     */
    public BaseParameters copy() {
        BaseParameters para = new BaseParameters();
        para.mMap = new HashMap<String, String>(mMap);
        return para;
    }
     /** 
     * Overwrite existing BaseParameters with a copy of the ones from {@code other}.
     *
     * <b>For use by the legacy shim only.</b>
     *
     * @hide
     */
    public void copyFrom(BaseParameters other) {
        if (other == null) {
            throw new NullPointerException("other must not be null");
        }

        mMap.putAll(other.mMap);
    }

    /**
     * Value equality check.
     *
     * @hide
     */
    public boolean same(BaseParameters other) {
        if (this == other) {
            return true;
        }
        return other != null && BaseParameters.this.mMap.equals(other.mMap);
    }

    /**
     * Writes the current BaseParameters to the log.
     * @hide
     * @deprecated
     */
    @Deprecated
    public void dump() {
        Log.e(TAG, "dump: size=" + mMap.size());
        for (String k : mMap.keySet()) {
            Log.e(TAG, "dump: " + k + "=" + mMap.get(k));
        }
    }

    /**
     * Creates a single string with all the BaseParameters set in
     * this BaseParameters object.
     * <p>The {@link #unflatten(String)} method does the reverse.</p>
     *
     * @return a String with all values from this BaseParameters object, in
     *         semi-colon delimited key-value pairs
     */
    public String flatten() {
        StringBuilder flattened = new StringBuilder(128);
        for (String k : mMap.keySet()) {
            flattened.append(k);
            flattened.append("=");
            flattened.append(mMap.get(k));
            flattened.append(";");
        }
        // chop off the extra semicolon at the end
		if(flattened.length() > 0) {
	        flattened.deleteCharAt(flattened.length()-1);
		}
        return flattened.toString();
    }

    /**
     * Takes a flattened string of BaseParameters and adds each one to
     * this BaseParameters object.
     * <p>The {@link #flatten()} method does the reverse.</p>
     *
     * @param flattened a String of BaseParameters (key-value paired) that
     *                  are semi-colon delimited
     */
    public void unflatten(String flattened) {
        mMap.clear();

        TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(';');
        splitter.setString(flattened);
        for (String kv : splitter) {
            int pos = kv.indexOf('=');
            if (pos == -1) {
                continue;
            }
            String k = kv.substring(0, pos);
            String v = kv.substring(pos + 1);
            mMap.put(k, v);
        }
    }

    public void remove(String key) {
        mMap.remove(key);
    }

    /**
     * Sets a String parameter.
     *
     * @param key   the key name for the parameter
     * @param value the String value of the parameter
     */
    public void set(String key, String value) {
        if (key.indexOf('=') != -1 || key.indexOf(';') != -1 || key.indexOf(0) != -1) {
            Log.e(TAG, "Key \"" + key + "\" contains invalid character (= or ; or \\0)");
            return;
        }
        if (value.indexOf('=') != -1 || value.indexOf(';') != -1 || value.indexOf(0) != -1) {
            Log.e(TAG, "Value \"" + value + "\" contains invalid character (= or ; or \\0)");
            return;
        }

        put(key, value);
    }

    /**
     * Sets an integer parameter.
     *
     * @param key   the key name for the parameter
     * @param value the int value of the parameter
     */
    public void set(String key, int value) {
        put(key, Integer.toString(value));
    }

    private void put(String key, String value) {
        /*
         * Remove the key if it already exists.
         *
         * This way setting a new value for an already existing key will always move
         * that key to be ordered the latest in the map.
         */
        mMap.remove(key);
        mMap.put(key, value);
    }

	/*
    private void set(String key, List<Area> areas) {
        if (areas == null) {
            set(key, "(0,0,0,0,0)");
        } else {
            StringBuilder buffer = new StringBuilder();
            for (int i = 0; i < areas.size(); i++) {
                Area area = areas.get(i);
                Rect rect = area.rect;
                buffer.append('(');
                buffer.append(rect.left);
                buffer.append(',');
                buffer.append(rect.top);
                buffer.append(',');
                buffer.append(rect.right);
                buffer.append(',');
                buffer.append(rect.bottom);
                buffer.append(',');
                buffer.append(area.weight);
                buffer.append(')');
                if (i != areas.size() - 1) buffer.append(',');
            }
            set(key, buffer.toString());
        }
    }
    */

    /**
     * Returns the value of a String parameter.
     *
     * @param key the key name for the parameter
     * @return the String value of the parameter
     */
    public String get(String key) {
        return mMap.get(key);
    }

    /**
     * Returns the value of an integer parameter.
     *
     * @param key the key name for the parameter
     * @return the int value of the parameter
     */
    public int getInt(String key) {
        return Integer.parseInt(mMap.get(key));
    }

/*
    // Splits a comma delimited string to an ArrayList of String.
    // Return null if the passing string is null or the size is 0.
    private ArrayList<String> split(String str) {
        if (str == null) return null;

        TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
        splitter.setString(str);
        ArrayList<String> substrings = new ArrayList<String>();
        for (String s : splitter) {
            substrings.add(s);
        }
        return substrings;
    }

    // Splits a comma delimited string to an ArrayList of Integer.
    // Return null if the passing string is null or the size is 0.
    private ArrayList<Integer> splitInt(String str) {
        if (str == null) return null;

        TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
        splitter.setString(str);
        ArrayList<Integer> substrings = new ArrayList<Integer>();
        for (String s : splitter) {
            substrings.add(Integer.parseInt(s));
        }
        if (substrings.size() == 0) return null;
        return substrings;
    }

    private void splitInt(String str, int[] output) {
        if (str == null) return;

        TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
        splitter.setString(str);
        int index = 0;
        for (String s : splitter) {
            output[index++] = Integer.parseInt(s);
        }
    }

    // Splits a comma delimited string to an ArrayList of Float.
    private void splitFloat(String str, float[] output) {
        if (str == null) return;

        TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
        splitter.setString(str);
        int index = 0;
        for (String s : splitter) {
            output[index++] = Float.parseFloat(s);
        }
    }

    // Returns the value of a float parameter.
    private float getFloat(String key, float defaultValue) {
        try {
            return Float.parseFloat(mMap.get(key));
        } catch (NumberFormatException ex) {
            return defaultValue;
        }
    }

    // Returns the value of a integer parameter.
    private int getInt(String key, int defaultValue) {
        try {
            return Integer.parseInt(mMap.get(key));
        } catch (NumberFormatException ex) {
            return defaultValue;
        }
    }

    // Splits a comma delimited string to an ArrayList of Size.
    // Return null if the passing string is null or the size is 0.
    private ArrayList<Size> splitSize(String str) {
        if (str == null) return null;

        TextUtils.StringSplitter splitter = new TextUtils.SimpleStringSplitter(',');
        splitter.setString(str);
        ArrayList<Size> sizeList = new ArrayList<Size>();
        for (String s : splitter) {
            Size size = strToSize(s);
            if (size != null) sizeList.add(size);
        }
        if (sizeList.size() == 0) return null;
        return sizeList;
    }

    // Parses a string (ex: "480x320") to Size object.
    // Return null if the passing string is null.
    private Size strToSize(String str) {
        if (str == null) return null;

        int pos = str.indexOf('x');
        if (pos != -1) {
            String width = str.substring(0, pos);
            String height = str.substring(pos + 1);
            return new Size(Integer.parseInt(width),
                            Integer.parseInt(height));
        }
        Log.e(TAG, "Invalid size parameter string=" + str);
        return null;
    }

    // Splits a comma delimited string to an ArrayList of int array.
    // Example string: "(10000,26623),(10000,30000)". Return null if the
    // passing string is null or the size is 0.
    private ArrayList<int[]> splitRange(String str) {
        if (str == null || str.charAt(0) != '('
                || str.charAt(str.length() - 1) != ')') {
            Log.e(TAG, "Invalid range list string=" + str);
            return null;
        }

        ArrayList<int[]> rangeList = new ArrayList<int[]>();
        int endIndex, fromIndex = 1;
        do {
            int[] range = new int[2];
            endIndex = str.indexOf("),(", fromIndex);
            if (endIndex == -1) endIndex = str.length() - 1;
            splitInt(str.substring(fromIndex, endIndex), range);
            rangeList.add(range);
            fromIndex = endIndex + 3;
        } while (endIndex != str.length() - 1);

        if (rangeList.size() == 0) return null;
        return rangeList;
    }

    // Splits a comma delimited string to an ArrayList of Area objects.
    // Example string: "(-10,-10,0,0,300),(0,0,10,10,700)". Return null if
    // the passing string is null or the size is 0 or (0,0,0,0,0).
    private ArrayList<Area> splitArea(String str) {
        if (str == null || str.charAt(0) != '('
                || str.charAt(str.length() - 1) != ')') {
            Log.e(TAG, "Invalid area string=" + str);
            return null;
        }

        ArrayList<Area> result = new ArrayList<Area>();
        int endIndex, fromIndex = 1;
        int[] array = new int[5];
        do {
            endIndex = str.indexOf("),(", fromIndex);
            if (endIndex == -1) endIndex = str.length() - 1;
            splitInt(str.substring(fromIndex, endIndex), array);
            Rect rect = new Rect(array[0], array[1], array[2], array[3]);
            result.add(new Area(rect, array[4]));
            fromIndex = endIndex + 3;
        } while (endIndex != str.length() - 1);

        if (result.size() == 0) return null;

        if (result.size() == 1) {
            Area area = result.get(0);
            Rect rect = area.rect;
            if (rect.left == 0 && rect.top == 0 && rect.right == 0
                    && rect.bottom == 0 && area.weight == 0) {
                return null;
            }
        }

        return result;
    }

    private boolean same(String s1, String s2) {
        if (s1 == null && s2 == null) return true;
        if (s1 != null && s1.equals(s2)) return true;
        return false;
    }
*/

};

