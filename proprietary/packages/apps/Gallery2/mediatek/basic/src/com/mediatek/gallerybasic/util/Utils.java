package com.mediatek.gallerybasic.util;

import android.content.Context;
import android.net.Uri;
import android.os.Build;
import android.os.ParcelFileDescriptor;
import android.util.DisplayMetrics;
import android.view.WindowManager;

import java.io.BufferedReader;
import java.io.Closeable;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.URLDecoder;
import java.net.URLEncoder;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class Utils {
    private static final String TAG = "MtkGallery2/Utils";
    /**
     * The special characters of file uri.
     */
    public static final String SPECIAL_CHARACTERS = "[#&$*^@!%]";
    /**
     * The length of "file://".
     */
    public static final int SIZE_SCHEME_FILE = 7;
    private static float sPixelDensity = -1f;

    public static final boolean HAS_GLES20_REQUIRED =
        Build.VERSION.SDK_INT >= VERSION_CODES.HONEYCOMB;

    public static void initialize(Context context) {
        DisplayMetrics metrics = new DisplayMetrics();
        WindowManager wm = (WindowManager) context
                .getSystemService(Context.WINDOW_SERVICE);
        wm.getDefaultDisplay().getMetrics(metrics);
        sPixelDensity = metrics.density;
    }

    public static interface VERSION_CODES {
        // These value are copied from Build.VERSION_CODES
        public static final int GINGERBREAD_MR1 = 10;
        public static final int HONEYCOMB = 11;
        public static final int HONEYCOMB_MR1 = 12;
        public static final int HONEYCOMB_MR2 = 13;
        public static final int ICE_CREAM_SANDWICH = 14;
        public static final int ICE_CREAM_SANDWICH_MR1 = 15;
        public static final int JELLY_BEAN = 16;
        public static final int JELLY_BEAN_MR1 = 17;
        public static final int JELLY_BEAN_MR2 = 18;
    }

    public static void waitWithoutInterrupt(Object object) {
        try {
            object.wait();
        } catch (InterruptedException e) {
            Log.w(TAG, "<waitWithoutInterrupt> unexpected interrupt: " + object);
        }
    }

    public static void assertTrue(boolean cond) {
        if (!cond) {
            throw new AssertionError();
        }
    }

    public static <T> T checkNotNull(T object) {
        if (object == null) {
            throw new NullPointerException();
        }
        return object;
    }

    public static int nextPowerOf2(int n) {
        if (n <= 0 || n > (1 << 30)) {
            throw new IllegalArgumentException("n is invalid: " + n);
        }
        n -= 1;
        n |= n >> 16;
        n |= n >> 8;
        n |= n >> 4;
        n |= n >> 2;
        n |= n >> 1;
        return n + 1;
    }

    // Returns the previous power of two.
    // Returns the input if it is already power of 2.
    // Throws IllegalArgumentException if the input is <= 0
    public static int prevPowerOf2(int n) {
        if (n <= 0) {
            throw new IllegalArgumentException();
        }
        return Integer.highestOneBit(n);
    }

    public static void closeSilently(ParcelFileDescriptor fd) {
        try {
            if (fd != null) {
                fd.close();
            }
        } catch (IOException e) {
            Log.w(TAG, "<closeSilently> fail to close ParcelFileDescriptor", e);
        }
    }

    public static void closeSilently(Closeable c) {
        if (c == null) {
            return;
        }
        try {
            c.close();
        } catch (IOException t) {
            Log.w(TAG, "<closeSilently> fail to close Closeable", t);
        }
    }

    public static float dpToPixel(float dp) {
        return sPixelDensity * dp;
    }

    public static int dpToPixel(int dp) {
        return Math.round(dpToPixel((float) dp));
    }

    public static int meterToPixel(float meter) {
        // 1 meter = 39.37 inches, 1 inch = 160 dp.
        return Math.round(dpToPixel(meter * 39.37f * 160));
    }

    public static String decodePath(String filePath) {
        try {
            return URLDecoder.decode(filePath, "utf-8");
        } catch (UnsupportedEncodingException e) {
            Log.d(TAG, "<decodePath> fail");
            e.printStackTrace();
        }
        return null;
    }

    public static String encodePath(String filePath) {
        try {
            return URLEncoder.encode(filePath, "utf-8");
        } catch (UnsupportedEncodingException e) {
            Log.w(TAG, "<encodePath> fail");
            e.printStackTrace();
        }
        return null;
    }

    private static long sDeviceRam = -1;

    public static long getDeviceRam() {
        if (sDeviceRam != -1) {
            return sDeviceRam;
        }

        String path = "/proc/meminfo";
        String content = null;
        BufferedReader br = null;
        try {
            br = new BufferedReader(new FileReader(path), 8);
            String line;
            if ((line = br.readLine()) != null) {
                content = line;
            }
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        } finally {
            if (br != null) {
                try {
                    br.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        if (content != null) {
            int begin = content.indexOf(':');
            int end = content.indexOf('k');
            content = content.substring(begin + 1, end).trim();
            sDeviceRam = Integer.parseInt(content);
            Log.d(TAG, "<getDeviceRam> " + sDeviceRam + "KB");
        }
        return sDeviceRam;
    }

    /**
     * Check whether has special characters in this uri.
     * @param uri the file uri.
     * @return  whether has special characters, or not.
     */
    public static boolean hasSpecialCharaters(final Uri uri) {
        if (uri == null) {
            return false;
        }
        Pattern patten = Pattern.compile(SPECIAL_CHARACTERS);
        Matcher matcher = patten.matcher(uri.toString());
        return matcher.find();
    }
}
