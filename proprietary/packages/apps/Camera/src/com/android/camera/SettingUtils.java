package com.android.camera;

import android.content.Context;
import android.graphics.Point;
import android.hardware.Camera.Size;
import android.view.View;


import java.text.DecimalFormat;
import java.text.DecimalFormatSymbols;
import java.util.ArrayList;
import java.util.List;
import java.util.Locale;


public class SettingUtils {
    private static final String TAG = "SettingUtils";

    private static final String ENABLE_LIST_HEAD = "[L];";
    private static final String ENABLE_LIST_SEPERATOR = ";";
    public static final String RESET_STATE_VALUE_DISABLE = "disable-value";
    private static final int UNKNOWN = -1;

    private static final float ALPHA_ENABLE = 1.0F;
    private static final float ALPHA_DISABLE = 0.3F;

    private SettingUtils() {
    }

    public static void setEnabledState(View view, boolean enabled) {
        if (view != null) {
            float alpha = enabled ? ALPHA_ENABLE : ALPHA_DISABLE;
            view.setAlpha(alpha);
        }
    }

    public static boolean isDisableValue(String value) {
        boolean reset = false;
        if (RESET_STATE_VALUE_DISABLE.equals(value)) {
            reset = true;
        }
//        Log.d(TAG, "isResetValue(" + value + ") return " + reset);
        return reset;
    }

    public static String buildEnableList(String[] list, String current) {
        String listStr = null;
        if (list != null) {
            listStr = ENABLE_LIST_HEAD + current + ENABLE_LIST_SEPERATOR;
            List<String> uniqueList = new ArrayList<String>();
            for (int i = 0, len = list.length; i < len; i++) {
                if (uniqueList.contains(list[i])) {
                    continue;
                }
                uniqueList.add(list[i]);
                if (i == (len - 1)) {
                    listStr += list[i];
                } else {
                    listStr += (list[i] + ENABLE_LIST_SEPERATOR);
                }
            }
        }
        return listStr;
    }

    public static boolean isBuiltList(String listString) {
        boolean isList = false;
        if (listString != null && listString.startsWith(ENABLE_LIST_HEAD)) {
            isList = true;
        }
        return isList;
    }

    // public static String[] getEnabledArray(String listString) {
    // String[] list = null;
    // if (isBuiltList(listString)) {
    // String[] temp = listString.split(ENABLE_LIST_SEPERATOR);
    // list = new String[temp.length - 2];
    // for (int i = 2, len = temp.length; i < len; i++) {
    // list[i - 2] = temp[i];
    // }
    // }
    // Log.d(TAG, "getEnabledArray(" + listString + ") return " + list);
    // return list;
    // }

    public static List<String> getEnabledList(String listString) {
        ArrayList<String> list = new ArrayList<String>();
        if (isBuiltList(listString)) {
            String[] temp = listString.split(ENABLE_LIST_SEPERATOR);
            for (int i = 2, len = temp.length; i < len; i++) {
                if (!list.contains(temp[i])) {
                    list.add(temp[i]);
                }
            }
        }
        return list;
    }

    public static String getDefaultValue(String listString) {
        String value = null;
        if (isBuiltList(listString)) {
            String[] temp = listString.split(ENABLE_LIST_SEPERATOR);
            if (temp != null && temp.length > 1) {
                value = temp[1];
            }
        }
//        Log.i(TAG, "getDefaultValue(" + listString + ") return " + value);
        return value;
    }

    public static boolean contains(int[] list, int value) {
        boolean find = false;
        if (list != null) {
            for (int i = 0, len = list.length; i < len; i++) {
                if (list[i] == value) {
                    find = true;
                    break;
                }
            }
        }
//        Log.d(TAG, "contains(" + list + ", " + value + ") return " + find);
        return find;
    }

    public static boolean contains(String[] list, String value) {
        boolean find = false;
        if (list != null && value != null) {
            for (int i = 0, len = list.length; i < len; i++) {
                if (value.equals(list[i])) {
                    find = true;
                    break;
                }
            }
        }
//        Log.d(TAG, "contains(" + list + ", " + value + ") return " + find);
        return find;
    }

    public static boolean contains(CharSequence[] list, String value) {
        boolean find = false;
        if (list != null && value != null) {
            for (int i = 0, len = list.length; i < len; i++) {
                if (value.equals(list[i])) {
                    find = true;
                    break;
                }
            }
        }
//        Log.d(TAG, "contains(" + list + ", " + value + ") return " + find);
        return find;
    }

    public static int index(String[] list, String value) {
        int findIndex = UNKNOWN;
        if (list != null && value != null) {
            for (int i = 0, len = list.length; i < len; i++) {
                if (value.equals(list[i])) {
                    findIndex = i;
                    break;
                }
            }
        }
//        Log.v(TAG, "index(" + list + ", " + value + ") return " + findIndex);
        return findIndex;
    }

    public static int index(CharSequence[] list, String value) {
        int findIndex = UNKNOWN;
        if (list != null && value != null) {
            for (int i = 0, len = list.length; i < len; i++) {
                if (value.equals(list[i])) {
                    findIndex = i;
                    break;
                }
            }
        }
//        Log.d(TAG, "index(" + list + ", " + value + ") return " + findIndex);
        return findIndex;
    }

    private static final DecimalFormat DECIMAL_FORMATOR = new DecimalFormat("######.####",
            new DecimalFormatSymbols(Locale.ENGLISH));

    public static String getRatioString(double ratio) {
        return DECIMAL_FORMATOR.format(ratio);
    }

    public static Point getSize(String sizeString) {
        Point size = null;
        int index = sizeString.indexOf('x');
        if (index != UNKNOWN) {
            int width = Integer.parseInt(sizeString.substring(0, index));
            int height = Integer.parseInt(sizeString.substring(index + 1));
            size = new Point(width, height);
        }
        return size;
    }

    public static String buildSize(Size size) {
        if (size != null) {
            return "" + size.width + "x" + size.height;
        } else {
            return "null";
        }
    }

    public static String buildSize(int width, int height) {
        return "" + width + "x" + height;
    }

    public static int getMainColor(Context context) {
        // M: For 4.4 migration because resource remove interface
        // int themeColor = 0;
        int finalColor = 0;
        // if (FeatureSwitcher.isThemeEnabled()) {
        // Resources res = context.getResources();
        // themeColor = res.getThemeMainColor();
        // }
        // if (themeColor == 0) {
        finalColor = context.getResources().getColor(R.color.setting_item_text_color_highlight);
        // } else {
        // finalColor = themeColor;
        // }
        return finalColor;
    }

    public static boolean isSupported(Object value, List<?> supported) {
        return supported == null ? false : supported.indexOf(value) >= 0;
    }
}
