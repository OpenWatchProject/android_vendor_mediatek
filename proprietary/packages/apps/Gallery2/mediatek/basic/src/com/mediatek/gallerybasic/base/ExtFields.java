package com.mediatek.gallerybasic.base;

import android.content.Context;
import android.database.Cursor;

import com.mediatek.gallerybasic.util.Log;
import com.mediatek.gallerybasic.util.MediaUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map.Entry;

/**
 * Save the extend fields info and value for MediaData.
 */
public class ExtFields {
    private static final String TAG = "MtkGallery2/ExtFields";

    private static HashMap<String, Position> sImageFields = new HashMap<String, Position>();
    private static HashMap<String, Position> sVideoFields = new HashMap<String, Position>();

    private static List<String> sImageColumns = new ArrayList<String>();
    private static List<String> sVideoColumns = new ArrayList<String>();

    private Object[] mImageFieldValues;
    private Object[] mVideoFieldValues;

    /**
     * Default constructor, when there is no cursor and other info,
     * construct a empty ExtFields to avoid check null in many scenarios.
     */
    public ExtFields() {
    }

    /**
     * Constructor, get the field values from cursor.
     *
     * @param cursor  The Cursor that has extended field info
     * @param isImage If the cursor refers a image record
     */
    public ExtFields(Cursor cursor, boolean isImage) {
        if (isImage && !sImageFields.isEmpty()) {
            mImageFieldValues = new Object[sImageFields.size()];
            setFieldsValue(cursor, sImageFields, mImageFieldValues);
        } else if (!isImage && !sVideoFields.isEmpty()) {
            mVideoFieldValues = new Object[sVideoFields.size()];
            setFieldsValue(cursor, sVideoFields, mVideoFieldValues);
        }
    }

    /**
     * Add field definitions.
     *
     * @param definitions The definition list
     */
    public static void registerFieldDefinition(IFieldDefinition[] definitions) {
        for (IFieldDefinition define : definitions) {
            define.onFieldDefine();
        }
    }

    /**
     * Add field in image view.
     *
     * @param column The column in image view
     */
    public static void addImageFiled(String column) {
        if (sImageColumns != null && sImageColumns.contains(column)) {
            Log.d(TAG, "<addImageFiled> support [" + column + "] on current platform");
            addField(sImageFields, column);
        } else {
            Log.d(TAG, "<addImageFiled> not support [" + column + "] on current platform");
        }
    }

    /**
     * Add field in video view.
     *
     * @param column The column in video view
     */
    public static void addVideoFiled(String column) {
        if (sVideoColumns != null && sVideoColumns.contains(column)) {
            Log.d(TAG, "<addVideoFiled> support [" + column + "] on current platform");
            addField(sVideoFields, column);
        } else {
            Log.d(TAG, "<addVideoFiled> not support [" + column + "] on current platform");
        }
    }

    /**
     * Base on orginal image projection, add new columns.
     *
     * @param projection Original image projection
     * @return The extended image projection
     */
    public static String[] getImageProjection(String[] projection) {
        return getProjection(sImageFields, projection);
    }

    /**
     * Base on orginal video projection, add new columns.
     *
     * @param projection Original video projection
     * @return The extended video projection
     */
    public static String[] getVideoProjection(String[] projection) {
        return getProjection(sVideoFields, projection);
    }

    /**
     * Initialize all image and video columns before add field.
     *
     * @param context The current application context
     */
    public static void initColumns(Context context) {
        sImageColumns = MediaUtils.getImageColumns(context);
        sVideoColumns = MediaUtils.getVideoColumns(context);
    }

    private static void addField(HashMap<String, Position> fields, String column) {
        if (fields.containsKey(column)) {
            Log.d(TAG, "<addField> already has column[" + column + "], return");
            return;
        }

        Position field = new Position();
        fields.put(column, field);
        Log.d(TAG, "<addField> add column[" + column + "], return");
    }

    private static String[] getProjection(HashMap<String, Position> fields, String[] projection) {
        if (fields.isEmpty()) {
            return projection;
        }

        String[] newProjection = new String[projection.length + fields.size()];
        int projectionIndex = 0;
        int valueIndex = 0;
        // copy old to new
        for (String str : projection) {
            newProjection[projectionIndex] = projection[projectionIndex];
            projectionIndex++;
        }
        // set ext fields to new
        Iterator<Entry<String, Position>> itr = fields.entrySet().iterator();
        Entry<String, Position> entry;
        while (itr.hasNext()) {
            entry = itr.next();
            newProjection[projectionIndex] = entry.getKey();
            entry.getValue().projectionIndex = projectionIndex;
            entry.getValue().valueIndex = valueIndex;
            projectionIndex++;
            valueIndex++;
        }
        return newProjection;
    }

    private static void setFieldsValue(Cursor cursor, HashMap<String, Position> fields,
                                       Object[] values) {
        Iterator<Entry<String, Position>> itr = fields.entrySet().iterator();
        Entry<String, Position> entry;
        int valueIndex = 0;
        while (itr.hasNext()) {
            entry = itr.next();
            switch (cursor.getType(entry.getValue().projectionIndex)) {
                case Cursor.FIELD_TYPE_BLOB:
                    values[valueIndex] = cursor.getBlob(entry.getValue().projectionIndex);
                    break;
                case Cursor.FIELD_TYPE_FLOAT:
                    values[valueIndex] = cursor.getFloat(entry.getValue().projectionIndex);
                    break;
                case Cursor.FIELD_TYPE_INTEGER:
                    values[valueIndex] = cursor.getInt(entry.getValue().projectionIndex);
                    break;
                case Cursor.FIELD_TYPE_STRING:
                    values[valueIndex] = cursor.getString(entry.getValue().projectionIndex);
                    break;
                case Cursor.FIELD_TYPE_NULL:
                    values[valueIndex] = null;
                    break;
                default:
                    values[valueIndex] = null;
                    break;
            }
            valueIndex++;
        }
    }

    /**
     * Get the field in image view.
     *
     * @param name The column name
     * @return The field value
     */
    public Object getImageField(String name) {
        if (mImageFieldValues == null) {
            return null;
        }
        if (sImageFields.containsKey(name)) {
            return mImageFieldValues[sImageFields.get(name).valueIndex];
        } else {
            return null;
        }
    }

    /**
     * Get the field in video view.
     *
     * @param name The column name
     * @return The field value
     */
    public Object getVideoField(String name) {
        if (mVideoFieldValues == null) {
            return null;
        }
        if (sVideoFields.containsKey(name)) {
            return mVideoFieldValues[sVideoFields.get(name).valueIndex];
        } else {
            return null;
        }
    }

    /**
     * The position info of field.
     */
    private static class Position {
        public int projectionIndex;
        public int valueIndex;
    }
}