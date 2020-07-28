package com.mediatek.camera.tests.v3.util;

import android.accessibilityservice.AccessibilityService;
import android.content.Context;
import android.content.Intent;
import android.graphics.Point;
import android.graphics.Rect;
import android.hardware.input.InputManager;
import android.media.ExifInterface;
import android.os.Build;
import android.os.Environment;
import android.os.StatFs;
import android.os.SystemClock;
import android.support.test.InstrumentationRegistry;
import android.support.test.uiautomator.By;
import android.support.test.uiautomator.BySelector;
import android.support.test.uiautomator.StaleObjectException;
import android.support.test.uiautomator.UiDevice;
import android.support.test.uiautomator.UiObject2;
import android.view.InputDevice;
import android.view.InputEvent;
import android.view.MotionEvent;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.portability.SystemProperties;
import com.mediatek.camera.portability.storage.StorageManagerExt;
import com.mediatek.camera.tests.v3.arch.TestContext;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.lang.reflect.Method;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.regex.Pattern;

public class Utils {
    public static int STABILITY_REPEAT_TIMES = Build.VERSION.SDK_INT >= 28 ?
            SystemProperties.getInt("vendor.mtk.camera.app.stability.repeat", 100) :
            SystemProperties.getInt("mtk.camera.app.stability.repeat", 100);
    // 100 times = 2 min = 120 s
    public static int STABILITY_CONTINUE_SECONDS = STABILITY_REPEAT_TIMES * 100 / 120;

    public static String[] WIFI_NAMES = new String[]{"WSD_OSS6_AT", "AP10", "AP15"};
    public static String[] WIFI_PASSWORDS = new String[]{"1234567890", "oss6_ap10", "woshimima"};

    public static int TIME_OUT_RIGHT_NOW = 10; // 10ms
    public static int TIME_OUT_SHORT_SHORT = 1000; // 1s
    public static int TIME_OUT_SHORT = 5000; // 5s
    public static int TIME_OUT_NORMAL = 10000; // 10s
    public static int TIME_OUT_LONG = 30000; // 30s
    public static int TIME_OUT_LONG_LONG = 180000; // 3min

    public static int TIME_DURATION_FIVE_MIN = 300;
    public static int SCROLL_TIMES_LESS = 3;
    public static int SCROLL_TIMES_NORMAL = 10;
    public static int SCROLL_TIMES_MORE = 20;

    private static final int WAIT_POWER_OFF_IN_MS = 30000;
    private static final int BUFFER_SIZE_OF_COPY = 1024 * 1024;

    private static final LogUtil.Tag TAG = getTestTag(Utils.class.getSimpleName());
    private static UiDevice mUiDevice;
    private static Context mContext;
    private static Context mTargetContext;
    private static InputManager mInputManager;
    private static Method mInjectInputMethod;
    private static FeatureSpecParser mFeatureSpecParser;

    static {
        Method getInstanceMethod = ReflectUtils.getMethod(InputManager.class, "getInstance");
        if (getInstanceMethod == null) {
            mInputManager = null;
        }
        mInputManager = (InputManager) ReflectUtils.callMethodOnObject(null, getInstanceMethod);

        mInjectInputMethod = ReflectUtils.getMethod(InputManager.class, "injectInputEvent",
                InputEvent.class, int.class);
    }

    public static LogUtil.Tag getTestTag(String tag) {
        return new LogUtil.Tag("AT_" + tag);
    }

    public static BySelector getShutterSelector() {
        return By.res(Pattern.compile("com.mediatek.camera:id/shutter_.*"))
                .clazz("android.widget.ImageView").longClickable(true);
    }

    public static BySelector getSystemSettingListSelector() {
        if (Build.VERSION.CODENAME.equals("Q") || Build.VERSION.SDK_INT >= 29) {
            return By.res("com.android.settings:id/recycler_view");
        } else {
            return By.res("com.android.settings:id/dashboard_container");
        }
    }

    public static void setTimeoutForSanity() {
        TIME_OUT_NORMAL = 60000;
    }

    public static UiDevice getUiDevice() {
        if (mUiDevice == null) {
            mUiDevice = UiDevice.getInstance(InstrumentationRegistry.getInstrumentation());
            mUiDevice.registerWatcher(UnExceptedPopupUiWatcher.class.getSimpleName(),
                    new UnExceptedPopupUiWatcher());
//            mUiDevice.registerWatcher(ConnectCameraWatcher.class.getSimpleName(),
//                    new ConnectCameraWatcher());
            mUiDevice.registerWatcher(FocusOnSystemUiWatcher.class.getSimpleName(),
                    new FocusOnSystemUiWatcher());
            mUiDevice.runWatchers();
        }
        try {
            mUiDevice.setCompressedLayoutHeirarchy(false);
        } catch (NullPointerException e) {
            // null exception may pop up when setCompressedLayoutHeirarchy, ignore
        }
        return mUiDevice;
    }

    public static Context getContext() {
        if (mContext == null) {
            mContext = InstrumentationRegistry.getContext();
        }
        return mContext;
    }

    public static Context getTargetContext() {
        if (mTargetContext == null) {
            mTargetContext = InstrumentationRegistry.getTargetContext();
        }
        return mTargetContext;
    }

    public static void assertObject(BySelector selector, int timeout) {
        Condition condition = new Condition() {
            @Override
            public boolean isSatisfied() {
                return mUiDevice.findObject(selector) != null;
            }
        };
        condition.assertMe(timeout);
    }


    public static void assertObject(BySelector selector) {
        assertObject(selector, TIME_OUT_NORMAL);
    }

    public static void assertNoObject(BySelector selector, int timeout) {
        Condition condition = new Condition() {
            @Override
            public boolean isSatisfied() {
                return mUiDevice.findObject(selector) == null;
            }
        };
        condition.assertMe(timeout);
    }

    public static void assertNoObject(BySelector selector) {
        assertNoObject(selector, TIME_OUT_NORMAL);
    }


    public static boolean waitObject(BySelector selector, int timeout) {
        Condition condition = new Condition() {
            @Override
            public boolean isSatisfied() {
                return mUiDevice.findObject(selector) != null;
            }
        };
        return condition.waitMe(timeout);
    }

    public static boolean waitNoObject(BySelector selector, int timeout) {
        Condition condition = new Condition() {
            @Override
            public boolean isSatisfied() {
                return mUiDevice.findObject(selector) == null;
            }
        };
        return condition.waitMe(timeout);
    }

    public static boolean waitObject(BySelector selector) {
        return waitObject(selector, TIME_OUT_NORMAL);
    }

    public static boolean waitNoObject(BySelector selector) {
        return waitNoObject(selector, TIME_OUT_NORMAL);
    }

    public static UiObject2 findObject(BySelector selector, int timeout) {
        boolean exist = waitObject(selector, timeout);
        if (exist) {
            return mUiDevice.findObject(selector);
        } else {
            return null;
        }
    }

    public static UiObject2 findNoObject(BySelector selector, int timeout) {
        boolean notExist = waitNoObject(selector, timeout);
        if (!notExist) {
            return mUiDevice.findObject(selector);
        } else {
            return null;
        }
    }

    public static UiObject2 findObject(BySelector selector) {
        return findObject(selector, TIME_OUT_NORMAL);
    }

    public static UiObject2 findNoObject(BySelector selector) {
        return findNoObject(selector, TIME_OUT_NORMAL);
    }

    public static UiObject2 pressBackUtilFindObject(BySelector selector, int maxPressBackTimes) {
        int times = 0;
        UiObject2 object;
        while (true) {
            object = findObject(selector, Utils.TIME_OUT_SHORT_SHORT);
            if (object == null && times < maxPressBackTimes) {
                getUiDevice().pressBack();
                times++;
                continue;
            } else if (object != null) {
                return object;
            } else if (times >= maxPressBackTimes) {
                return null;
            }
        }
    }

    public static UiObject2 pressBackUtilFindNoObject(BySelector selector, int maxPressBackTimes) {
        int times = 1;
        LogHelper.d(TAG, "[pressBackUtilFindNoObject] press back key");
        getUiDevice().pressBack();
        UiObject2 object;
        while (true) {
            object = findNoObject(selector, Utils.TIME_OUT_SHORT_SHORT);
            if (object != null && times < maxPressBackTimes) {
                LogHelper.d(TAG, "[pressBackUtilFindNoObject] press back key");
                getUiDevice().pressBack();
                times++;
                continue;
            } else if (object == null) {
                return object;
            } else if (times >= maxPressBackTimes) {
                return null;
            }
        }
    }

    public static UiObject2 scrollOnScreenToFind(BySelector selector) {
        return scrollOnScreenToFind(selector, SCROLL_TIMES_NORMAL);
    }

    public static UiObject2 scrollOnScreenToFind(BySelector selector, int scrollTimes) {
        int STEPS = 20;
        int startX = Utils.getUiDevice().getDisplayWidth() / 2;
        int endX = Utils.getUiDevice().getDisplayWidth() / 2;
        int startY = Utils.getUiDevice().getDisplayHeight() / 4 * 3;
        int endY = Utils.getUiDevice().getDisplayHeight() / 4;

        UiObject2 res = null;

        // scroll down
        for (int i = 0; i < scrollTimes; i++) {
            getUiDevice().waitForIdle();
            res = findObject(selector, TIME_OUT_SHORT_SHORT);
            if (res != null) {
                return res;
            } else {
                getUiDevice().swipe(startX, startY, endX, endY, STEPS);
            }
        }

        // scroll up
        for (int i = 0; i < scrollTimes; i++) {
            res = findObject(selector, TIME_OUT_SHORT_SHORT);
            if (res != null) {
                return res;
            } else {
                getUiDevice().swipe(startX, endY, endX, startY, STEPS);
            }
        }

        return res;
    }

    public static boolean scrollOnScreenToMapCondition(Condition condition) {
        return scrollOnScreenToMapCondition(condition, SCROLL_TIMES_NORMAL);
    }

    public static boolean scrollOnScreenToMapCondition(Condition condition, int scrollTimes) {
        int STEPS = 20;
        int startX = Utils.getUiDevice().getDisplayWidth() / 2;
        int endX = Utils.getUiDevice().getDisplayWidth() / 2;
        int startY = Utils.getUiDevice().getDisplayHeight() / 4 * 3;
        int endY = Utils.getUiDevice().getDisplayHeight() / 4;

        boolean res = false;
        int scrollDownTimes = 0;
        int scrollUpTimes = 0;

        while (scrollDownTimes < scrollTimes || scrollUpTimes < scrollTimes) {
            res = waitCondition(condition, TIME_OUT_SHORT_SHORT);
            if (res == true) {
                break;
            }
            if (scrollDownTimes < scrollTimes) {
                getUiDevice().swipe(startX, startY, endX, endY, STEPS);
                scrollDownTimes++;
            } else if (scrollUpTimes < scrollTimes) {
                getUiDevice().swipe(startX, endY, endX, startY, STEPS);
                scrollUpTimes++;
            }
        }
        return res;
    }

    public static UiObject2 scrollUpOnScreenToFind(BySelector selector) {
        return scrollUpOnScreenToFind(selector, SCROLL_TIMES_NORMAL);
    }

    public static UiObject2 scrollUpOnScreenToFind(BySelector selector, int scrollTimes) {
        int STEPS = 20;
        int startX = Utils.getUiDevice().getDisplayWidth() / 2;
        int endX = Utils.getUiDevice().getDisplayWidth() / 2;
        int startY = Utils.getUiDevice().getDisplayHeight() / 4 * 3;
        int endY = Utils.getUiDevice().getDisplayHeight() / 4;

        UiObject2 res = null;

        // scroll up
        for (int i = 0; i < scrollTimes; i++) {
            res = findObject(selector, TIME_OUT_RIGHT_NOW);
            if (res != null) {
                return res;
            } else {
                getUiDevice().swipe(startX, endY, endX, startY, STEPS);
            }
        }

        return res;
    }

    public static UiObject2 scrollLeftOnScreenToFind(BySelector selector) {
        return scrollLeftOnScreenToFind(selector, SCROLL_TIMES_NORMAL);
    }

    public static UiObject2 scrollLeftOnScreenToFind(BySelector selector, int scrollTimes) {
        int STEPS = 20;
        int startX = Utils.getUiDevice().getDisplayWidth() / 4;
        int endX = Utils.getUiDevice().getDisplayWidth() / 4 * 3;
        int startY = Utils.getUiDevice().getDisplayHeight() / 2;
        int endY = Utils.getUiDevice().getDisplayHeight() / 2;

        UiObject2 res = null;

        // scroll left
        for (int i = 0; i < scrollTimes; i++) {
            res = findObject(selector, TIME_OUT_RIGHT_NOW);
            if (res != null) {
                return res;
            } else {
                getUiDevice().swipe(startX, endY, endX, startY, STEPS);
            }
        }

        return res;
    }

    public static UiObject2 scrollOnObjectToFind(BySelector selector, UiObject2 inObject) {
        return scrollOnObjectToFind(selector, inObject, SCROLL_TIMES_NORMAL);
    }

    public static UiObject2 scrollOnObjectToFind(BySelector selector, UiObject2 inObject,
                                                 int scrollTimes) {
        int STEPS = 20;
        Rect rect = inObject.getVisibleBounds();
        int startX = rect.left + rect.width() / 2;
        int endX = rect.left + rect.width() / 2;
        int startY = rect.top + rect.height() / 4 * 3;
        int endY = rect.top + rect.height() / 4;

        UiObject2 res = null;
        // scroll down
        for (int i = 0; i < scrollTimes; i++) {
            res = findObject(selector, TIME_OUT_SHORT_SHORT);
            if (res != null) {
                return res;
            } else {
                getUiDevice().swipe(startX, startY, endX, endY, STEPS);
            }
        }

        // scroll up
        for (int i = 0; i < scrollTimes; i++) {
            res = findObject(selector, TIME_OUT_SHORT_SHORT);
            if (res != null) {
                return res;
            } else {
                getUiDevice().swipe(startX, endY, endX, startY, STEPS);
            }
        }
        return res;
    }

    public static void scrollDownOnObject(UiObject2 object) {
        int STEPS = 20;
        Rect rect = object.getVisibleBounds();
        int startX = rect.left + rect.width() / 2;
        int endX = rect.left + rect.width() / 2;
        int startY = rect.top + rect.height() / 4 * 3;
        int endY = rect.top + rect.height() / 4;

        getUiDevice().swipe(startX, startY, endX, endY, STEPS);
    }

    public static void scrollUpOnObject(UiObject2 object) {
        int STEPS = 20;
        Rect rect = object.getVisibleBounds();
        int startX = rect.left + rect.width() / 2;
        int endX = rect.left + rect.width() / 2;
        int startY = rect.top + rect.height() / 4;
        int endY = rect.top + rect.height() / 4 * 3;

        getUiDevice().swipe(startX, startY, endX, endY, STEPS);
    }

    public static UiObject2 scrollFromTopOnScreenToFind(BySelector selector) {
        int SCROLL_TIMES = 10;
        int STEPS = 20;
        int startX = Utils.getUiDevice().getDisplayWidth() / 2;
        int endX = Utils.getUiDevice().getDisplayWidth() / 2;
        int startY = 0;
        int endY = Utils.getUiDevice().getDisplayHeight() / 2;

        UiObject2 res = null;
        for (int i = 0; i < SCROLL_TIMES; i++) {
            res = findObject(selector, TIME_OUT_SHORT_SHORT);
            if (res != null) {
                return res;
            } else {
                getUiDevice().swipe(startX, startY, endX, endY, STEPS);
            }
        }
        return res;
    }

    public static UiObject2 scrollOnScreenToFindNo(BySelector selector) {
        return scrollOnScreenToFindNo(selector, SCROLL_TIMES_NORMAL);
    }

    public static UiObject2 scrollOnScreenToFindNo(BySelector selector, int scrollTimes) {
        int STEPS = 20;
        int startX = Utils.getUiDevice().getDisplayWidth() / 2;
        int endX = Utils.getUiDevice().getDisplayWidth() / 2;
        int startY = Utils.getUiDevice().getDisplayHeight() / 4 * 3;
        int endY = Utils.getUiDevice().getDisplayHeight() / 4;

        UiObject2 res = null;
        // scroll down
        for (int i = 0; i < scrollTimes; i++) {
            res = findNoObject(selector, TIME_OUT_SHORT_SHORT);
            if (res != null) {
                return res;
            } else {
                getUiDevice().swipe(startX, startY, endX, endY, STEPS);
            }
        }

        // scroll up
        for (int i = 0; i < scrollTimes; i++) {
            res = findNoObject(selector, TIME_OUT_SHORT_SHORT);
            if (res != null) {
                return res;
            } else {
                getUiDevice().swipe(startX, endY, endX, startY, STEPS);
            }
        }
        return res;
    }

    public static void assertObjectRightNow(final BySelector selector) {
        Condition condition = new Condition() {
            @Override
            public boolean isSatisfied() {
                return getUiDevice().findObject(selector) != null;
            }
        };
        condition.assertMe(TIME_OUT_RIGHT_NOW);
    }

    public static void assertRightNow(boolean assertValue) {
        Condition condition = new Condition() {
            @Override
            public boolean isSatisfied() {
                return assertValue;
            }
        };
        condition.assertMe(TIME_OUT_RIGHT_NOW);
    }

    public static void assertRightNow(boolean assertValue, String failMessage) {
        Condition condition = new Condition() {
            @Override
            public boolean isSatisfied() {
                return assertValue;
            }
        };
        condition.assertMe(TIME_OUT_RIGHT_NOW, failMessage);
    }

    public static boolean waitCondition(Condition condition) {
        return condition.waitMe(TIME_OUT_NORMAL);
    }

    public static boolean waitCondition(Condition condition, int timeout) {
        return condition.waitMe(timeout);
    }

    public static void assertCondition(Condition condition) {
        condition.assertMe(TIME_OUT_NORMAL);
    }

    public static void assertCondition(Condition condition, int timeout) {
        condition.assertMe(timeout);
    }

    public static void waitSafely(int ms) {
        try {
            synchronized (getUiDevice()) {
                getUiDevice().wait(ms);
            }
        } catch (InterruptedException e) {

        }
    }

    public static String getStringInExif(String filePath, String tag, String defaultValue) {
        try {
            ExifInterface exifInterface = new ExifInterface(filePath);
            return exifInterface.getAttribute(tag);
        } catch (IOException e) {
            return defaultValue;
        }
    }

    public static int getIntInExif(String filePath, String tag, int defaultValue) {
        try {
            ExifInterface exifInterface = new ExifInterface(filePath);
            return exifInterface.getAttributeInt(tag, defaultValue);
        } catch (IOException e) {
            return defaultValue;
        }
    }

    public static double getDoubleInExif(String filePath, String tag, double defaultValue) {
        try {
            ExifInterface exifInterface = new ExifInterface(filePath);
            return exifInterface.getAttributeDouble(tag, defaultValue);
        } catch (IOException e) {
            return defaultValue;
        }
    }

    /**
     * Get feature supported status of specified tag
     *
     * @param tag
     * @param defaultValue
     * @return Return true, the support value is true in spec
     * Return false, the support value is false in spec
     * Return default value when not define support status in spec
     */
    public static Boolean isFeatureSupported(String tag, Boolean defaultValue) {
        if (mFeatureSpecParser == null) {
            mFeatureSpecParser = new FeatureSpecParser(getContext());
        }
        Boolean originValue = mFeatureSpecParser.isFeatureSupported(tag);
        return originValue == null ? defaultValue : originValue;
    }

    /**
     * Get feature supported status of specified tag
     *
     * @param tag
     * @return Return true, the support value is true in spec
     * Return false, the support value is false in spec or there is not define support status in
     * spec
     */
    public static boolean isFeatureSupported(String tag) {
        return isFeatureSupported(tag, false).booleanValue();
    }

    public static void delete(File file) {
        if (file.isFile()) {
            file.delete();
        } else {
            File[] files = file.listFiles();
            if (files != null) {
                for (File temp : files) {
                    delete(temp);
                }
            }
        }
    }

    public static void longPress(UiObject2 object2) {
        int defaultDurationMills = 500;
        longPress(object2, defaultDurationMills);
    }

    public static void longPress(UiObject2 object2, int durationMillSeconds) {
        int x = object2.getVisibleBounds().centerX();
        int y = object2.getVisibleBounds().centerY();
        getUiDevice().swipe(x, y, x, y, getSwipeStepsByDuration(durationMillSeconds));
    }

    public static void longPressUntil(UiObject2 object2, Condition condition,
                                      int timeoutInMillSeconds) {
        if (mInputManager == null || mInjectInputMethod == null) {
            LogHelper.d(TAG, "[longPressUntil] mInputManager == null || mInjectInputMethod == " +
                    "null, return false");
            return;
        }

        long downTime = SystemClock.uptimeMillis();
        int event_time_gap = 30;

        Point start = object2.getVisibleCenter();
        Point end = object2.getVisibleCenter();

        MotionEvent.PointerProperties[] pointerProperties = new MotionEvent.PointerProperties[1];
        MotionEvent.PointerProperties pointerProperties1 = new MotionEvent.PointerProperties();
        pointerProperties1.id = 0;
        pointerProperties1.toolType = MotionEvent.TOOL_TYPE_FINGER;
        pointerProperties[0] = pointerProperties1;

        MotionEvent.PointerCoords[] pointerCoords = new MotionEvent.PointerCoords[1];
        MotionEvent.PointerCoords pointerCoords1 = new MotionEvent.PointerCoords();
        pointerCoords1.x = start.x;
        pointerCoords1.y = start.y;
        pointerCoords1.pressure = 1;
        pointerCoords1.size = 1;
        pointerCoords[0] = pointerCoords1;

        // click down
        MotionEvent event = MotionEvent.obtain(downTime, SystemClock.uptimeMillis(),
                MotionEvent.ACTION_DOWN, 1,
                pointerProperties,
                pointerCoords,
                0, 0, 1, 1, 0, 0, 0, 0);
        event.setSource(InputDevice.SOURCE_TOUCHSCREEN);
        injectInputEvent(event);

        long timeout = System.currentTimeMillis() + timeoutInMillSeconds;
        // continuous down
        while (true) {
            Point point = lerp(start, end, 1, 1);
            pointerCoords[0].x = point.x;
            pointerCoords[0].y = point.y;

            event = MotionEvent.obtain(downTime, SystemClock.uptimeMillis(),
                    MotionEvent.ACTION_MOVE, 1, pointerProperties,
                    pointerCoords, 0, 0, 1, 1, 0, 0, 0, 0);
            event.setSource(InputDevice.SOURCE_TOUCHSCREEN);
            injectInputEvent(event);

            waitSafely(event_time_gap);

            if (System.currentTimeMillis() > timeout) {
                LogHelper.d(TAG, "[longPressUntil] time out, click up");
                break;
            }

            if (condition.isSatisfied()) {
                LogHelper.d(TAG, "[longPressUntil] condition is satisfied, click up");
                break;
            }
        }

        // click up
        pointerCoords[0].x = end.x;
        pointerCoords[0].y = end.y;
        event = MotionEvent.obtain(downTime, SystemClock.uptimeMillis(),
                MotionEvent.ACTION_UP, 1,
                pointerProperties,
                pointerCoords,
                0, 0, 1, 1, 0, 0, 0, 0);
        event.setSource(InputDevice.SOURCE_TOUCHSCREEN);
        injectInputEvent(event);
    }

    public static boolean zoom(UiObject2 object, float percentStart, float percentEnd, long
            timeMills) {
        Rect rect = object.getVisibleBounds();
        Point startLeftBottom = getPointAtPercent(rect, (1 - percentStart) / 2);
        Point endLeftBottom = getPointAtPercent(rect, (1 - percentEnd) / 2);
        Point startRightTop = getPointAtPercent(rect, (1 - percentStart) / 2 + percentStart);
        Point endRightTop = getPointAtPercent(rect, (1 - percentEnd) / 2 + percentEnd);

        return zoom(startLeftBottom, endLeftBottom, startRightTop, endRightTop, timeMills);
    }

    private static Point getPointAtPercent(Rect rect, float percent) {
        Point start = new Point(rect.left, rect.bottom);
        Point end = new Point(rect.right, rect.top);

        return lerp(start, end, percent);
    }

    public static boolean zoom(Point start1, Point end1, Point start2, Point end2, long
            timeMills) {
        if (mInputManager == null || mInjectInputMethod == null) {
            LogHelper.d(TAG, "[zoom] mInputManager == null || mInjectInputMethod == " +
                    "null, return false");
            return false;
        }

        long downTime = SystemClock.uptimeMillis();
        int event_time_gap = 30;
        int totalStep = (int) (timeMills / event_time_gap);

        MotionEvent.PointerProperties[] pointerProperties = new MotionEvent.PointerProperties[2];
        MotionEvent.PointerProperties pointerProperties1 = new MotionEvent.PointerProperties();
        MotionEvent.PointerProperties pointerProperties2 = new MotionEvent.PointerProperties();
        pointerProperties1.id = 0;
        pointerProperties1.toolType = MotionEvent.TOOL_TYPE_FINGER;
        pointerProperties2.id = 1;
        pointerProperties2.toolType = MotionEvent.TOOL_TYPE_FINGER;
        pointerProperties[0] = pointerProperties1;
        pointerProperties[1] = pointerProperties2;

        MotionEvent.PointerCoords[] pointerCoords = new MotionEvent.PointerCoords[2];
        MotionEvent.PointerCoords pointerCoords1 = new MotionEvent.PointerCoords();
        MotionEvent.PointerCoords pointerCoords2 = new MotionEvent.PointerCoords();
        pointerCoords1.x = start1.x;
        pointerCoords1.y = start1.y;
        pointerCoords1.pressure = 1;
        pointerCoords1.size = 1;
        pointerCoords2.x = start2.x;
        pointerCoords2.y = start2.y;
        pointerCoords2.pressure = 1;
        pointerCoords2.size = 1;
        pointerCoords[0] = pointerCoords1;
        pointerCoords[1] = pointerCoords2;

        // click down point 1
        MotionEvent event = MotionEvent.obtain(downTime, SystemClock.uptimeMillis(),
                MotionEvent.ACTION_DOWN, 1,
                pointerProperties,
                pointerCoords,
                0, 0, 1, 1, 0, 0, 0, 0);
        event.setSource(InputDevice.SOURCE_TOUCHSCREEN);
        injectInputEvent(event);

        // click down point 2
        event = MotionEvent.obtain(downTime, SystemClock.uptimeMillis(),
                MotionEvent.ACTION_POINTER_DOWN +
                        (pointerProperties2.id << MotionEvent.ACTION_POINTER_INDEX_SHIFT),
                2,
                pointerProperties,
                pointerCoords,
                0, 0, 1, 1, 0, 0, 0, 0);
        event.setSource(InputDevice.SOURCE_TOUCHSCREEN);
        injectInputEvent(event);

        // action move
        for (int step = 1; step < totalStep; step++) {
            Point point1 = lerp(start1, end1, step, totalStep);
            Point point2 = lerp(start2, end2, step, totalStep);
            pointerCoords[0].x = point1.x;
            pointerCoords[0].y = point1.y;
            pointerCoords[1].x = point2.x;
            pointerCoords[1].y = point2.y;

            event = MotionEvent.obtain(downTime, SystemClock.uptimeMillis(),
                    MotionEvent.ACTION_MOVE, 2, pointerProperties,
                    pointerCoords, 0, 0, 1, 1, 0, 0, 0, 0);
            event.setSource(InputDevice.SOURCE_TOUCHSCREEN);
            injectInputEvent(event);

            waitSafely(event_time_gap);
        }

        // click up point 1
        pointerCoords[0].x = end1.x;
        pointerCoords[0].y = end1.y;
        pointerCoords[1].x = end2.x;
        pointerCoords[1].y = end2.y;
        event = MotionEvent.obtain(downTime, SystemClock.uptimeMillis(),
                MotionEvent.ACTION_POINTER_UP +
                        (pointerProperties2.id << MotionEvent.ACTION_POINTER_INDEX_SHIFT),
                1,
                pointerProperties,
                pointerCoords,
                0, 0, 1, 1, 0, 0, 0, 0);
        event.setSource(InputDevice.SOURCE_TOUCHSCREEN);
        injectInputEvent(event);

        // click up point 2
        pointerCoords[0].x = end1.x;
        pointerCoords[0].y = end1.y;
        pointerCoords[1].x = end2.x;
        pointerCoords[1].y = end2.y;
        event = MotionEvent.obtain(downTime, SystemClock.uptimeMillis(),
                MotionEvent.ACTION_UP, 2,
                pointerProperties,
                pointerCoords,
                0, 0, 1, 1, 0, 0, 0, 0);
        event.setSource(InputDevice.SOURCE_TOUCHSCREEN);
        injectInputEvent(event);

        return true;
    }

    public static boolean injectInputEvent(InputEvent event) {
        if (mInputManager == null || mInjectInputMethod == null) {
            LogHelper.d(TAG, "[injectInputEvent] mInputManager == null || mInjectInputMethod == " +
                    "null, return false");
            return false;
        }
        ReflectUtils.callMethodOnObject(mInputManager, mInjectInputMethod, event, 0);
        return true;
    }

    public static void copyFile(String fromPath, String toPath) {
        int readByteCount = 0;
        try {
            FileInputStream fsFrom = new FileInputStream(fromPath);
            FileOutputStream fsTo = new FileOutputStream(toPath);
            byte[] buffer = new byte[BUFFER_SIZE_OF_COPY];
            LogHelper.d(TAG, "[copyFile] start copy buffer");
            while ((readByteCount = fsFrom.read(buffer)) != -1) {
                fsTo.write(buffer, 0, readByteCount);
            }
            LogHelper.d(TAG, "[copyFile] end copy buffer");
            fsFrom.close();
            fsTo.close();
        } catch (FileNotFoundException e) {
        } catch (IOException e) {
        }
    }

    private static Point lerp(Point start, Point end, int currentStep, int totalStep) {
        Point res = new Point();
        float alpha = (float) currentStep / (float) totalStep;
        res.x = (int) ((float) (end.x - start.x) * alpha + start.x);
        res.y = (int) ((float) (end.y - start.y) * alpha + start.y);
        return res;
    }

    private static Point lerp(Point start, Point end, float percent) {
        Point res = new Point();
        res.x = (int) ((float) (end.x - start.x) * percent + start.x);
        res.y = (int) ((float) (end.y - start.y) * percent + start.y);
        return res;
    }

    public static int getSwipeStepsByDuration(int durationMillSeconds) {
        int millsPreStep = 5; // 5ms
        return durationMillSeconds / millsPreStep;
    }

    public static long getReserveSpaceInMB() {
        return getReserveSpaceInByte() / 1024 / 1024;
    }

    public static long getReserveSpaceInByte() {
        StatFs statFs = new StatFs(getDefaultStoragePath());
        long b = statFs.getBlockSizeLong() * statFs.getAvailableBlocksLong();
        return b;
    }

    public static String getDefaultStoragePath() {
        return StorageManagerExt.getDefaultPath();
    }

    public static String getFileName(String filePath) {
        if (filePath == null) {
            return null;
        }

        return filePath.substring(filePath.lastIndexOf('/') + 1);
    }

    public static void rebootDevice() {
        getContext().sendBroadcast(new Intent("com.mediatek.reboot"));
        if (Utils.waitObject(By.textContains("Shutting down"))) {
            // wait shutting down
            Utils.waitSafely(WAIT_POWER_OFF_IN_MS);
        }
    }

    public static String dump() {
        String dirPath = Environment.getExternalStorageDirectory() + "/mtklog/mobilelog/";
        File dirFile = new File(dirPath);
        File latestLogFolder = null;
        if (!dirFile.exists()) {
            dirPath = Environment.getExternalStorageDirectory() + "/debuglogger/mobilelog/";
            dirFile = new File(dirPath);
        }
        if (dirFile.exists() && dirFile.isDirectory()) {
            File[] logFolders = dirFile.listFiles();
            if (logFolders != null) {
                long lastModifiedTime = -1;
                for (File logFolder : logFolders) {
                    if (logFolder.isDirectory() && lastModifiedTime <= logFolder.lastModified()) {
                        lastModifiedTime = logFolder.lastModified();
                        latestLogFolder = logFolder;
                    }
                }
            }
        }
        if (latestLogFolder == null) {
            dirPath = Environment.getExternalStorageDirectory() + "/CameraAssertFail/";
        } else {
            dirPath = latestLogFolder.getAbsolutePath() + "/CameraAssertFail/";
        }
        File file = new File(dirPath);
        file.exists();
        file.mkdirs();

        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd_hh_mm_ss");
        Date date = new Date();
        String time = sdf.format(date);

        // dump latest photos/videos
        if (TestContext.mLatestPhotoPath != null) {
            LogHelper.d(TAG, "[dump] dump " + TestContext.mLatestPhotoPath + " to " + dirPath);
            Utils.copyFile(TestContext.mLatestPhotoPath,
                    dirPath + Utils.getFileName(TestContext.mLatestPhotoPath));
        }
        if (TestContext.mLatestVideoPath != null) {
            LogHelper.d(TAG, "[dump] dump " + TestContext.mLatestVideoPath + " to " + dirPath);
            Utils.copyFile(TestContext.mLatestVideoPath,
                    dirPath + Utils.getFileName(TestContext.mLatestVideoPath));
        }
        if (TestContext.mLatestDngPath != null) {
            LogHelper.d(TAG, "[dump] dump " + TestContext.mLatestDngPath + " to " + dirPath);
            Utils.copyFile(TestContext.mLatestDngPath,
                    dirPath + Utils.getFileName(TestContext.mLatestDngPath));
        }
        if (TestContext.mLatestCsPhotoPath != null) {
            for (String photo : TestContext.mLatestCsPhotoPath) {
                LogHelper.d(TAG, "[dump] dump " + photo + " to " + dirPath);
                Utils.copyFile(photo,
                        dirPath + Utils.getFileName(photo));
            }
        }

        // dump window hierarchy
        String targetPath = dirPath + time + ".uix";
        File hierarchyDumpFile = new File(targetPath);
        try {
            mUiDevice.dumpWindowHierarchy(hierarchyDumpFile);
            LogHelper.d(TAG, "[dump] dump window hierarchy to " + targetPath);
        } catch (IOException e) {
            LogHelper.d(TAG, "[dump] Fail to dump window hierarchy", e);
        }

        // dump screen shot
        targetPath = dirPath + time + ".png";
        LogHelper.d(TAG, "[dump] dump screenshot to " + targetPath);
        boolean successToDumpScreenShot = takeScreenShot(targetPath);

        if (successToDumpScreenShot) {
            return "<Success to save screenshot at " + targetPath + ">";
        } else {
            return "<Fail to save screenshot at " + targetPath + ">";
        }
    }

    private static boolean takeScreenShot(String targetPath) {
        return getUiDevice().takeScreenshot(new File(targetPath));
    }

    public static void writeTestCaseCostTime(String testcase, int timeInSeconds) {
        writeKeyValueTo("camera_test_case_cost_time.csv", testcase, String.valueOf(timeInSeconds));
    }

    public static void writeComponentCostTime(String component, long timeInMillSeconds) {
        writeKeyValueTo("camera_component_in_test_case_cost_time.csv", component,
                String.valueOf(timeInMillSeconds));
    }

    private static void writeKeyValueTo(String fileName, String key, String value) {
        String filePath = Environment.getExternalStorageDirectory()
                + "/" + fileName;
        FileWriter fileWriter = null;
        try {
            fileWriter = new FileWriter(filePath, true);
            fileWriter.write(key + ", " + value + "\n");
            fileWriter.close();
        } catch (IOException e) {
            LogHelper.e(TAG, "[writeKeyValueTo] IOException", e);
        } finally {
            try {
                if (fileWriter != null) {
                    fileWriter.close();
                }
            } catch (IOException e) {
                LogHelper.e(TAG, "[writeKeyValueTo] IOException", e);
            }
        }
    }

    public static void runWithoutStaleObjectException(Runnable runnable) {
        int times = 0;
        while (times < 3) {
            times++;
            try {
                runnable.run();
                return;
            } catch (StaleObjectException e) {
                LogHelper.d(TAG, "[runRunnableWithoutStaleObjectException]", e);
                continue;
            }
        }
    }

    /**
     * Change to multi-window mode.
     */
    public static void activeMultiWindowMode() {
        LogHelper.d(TAG, "<activeMultiWindowMode> in & out multiWindow mode");
        waitSafely(2 * TIME_OUT_SHORT_SHORT);
        InstrumentationRegistry.getInstrumentation().getUiAutomation().performGlobalAction(
                AccessibilityService.GLOBAL_ACTION_TOGGLE_SPLIT_SCREEN);
        waitSafely(2 * TIME_OUT_SHORT_SHORT);
    }

    public static void tryToDoUntilCondition(
            Runnable runnable, Condition condition, int tryTimes, String assertMsg) {
        int currTimes = 0;
        while (currTimes < tryTimes) {
            LogHelper.d(TAG, "<tryToDoUntilCondition> currTimes = " + currTimes
                    + ", tryTimes = " + tryTimes + ", run runnable");
            runnable.run();
            if (condition.waitMe(TIME_OUT_NORMAL)) {
                LogHelper.d(TAG, "<tryToDoUntilCondition> condition satisfied, return");
                return;
            }
            currTimes++;
        }
        LogHelper.d(TAG, "<tryToDoUntilCondition> condition not satisfied until max tryTimes" +
                ", assert");
        Utils.assertRightNow(false, assertMsg);
    }
}