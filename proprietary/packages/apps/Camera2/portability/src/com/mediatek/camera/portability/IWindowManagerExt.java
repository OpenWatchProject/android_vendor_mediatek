package com.mediatek.camera.portability;

import java.lang.reflect.Method;

/**
 * Window manager extension.
 */
public class IWindowManagerExt {
    private static final String TAG = "WindowManagerExt";

    private static Class<?> sWindowManagerGlobalClass = ReflectUtil.getClass(
            "android.view.WindowManagerGlobal");
    private static Method sGetWindowManagerServiceMethod =
            ReflectUtil.getMethod(sWindowManagerGlobalClass, "getWindowManagerService");

    private static Class<?> sIWindowManagerClass = ReflectUtil.getClass(
            "android.view.IWindowManager");
    private static Method sGetAnimationScaleMethod =
            ReflectUtil.getMethod(sIWindowManagerClass, "getAnimationScale", int.class);
    private static Method sSetAnimationScaleMethod =
            ReflectUtil.getMethod(sIWindowManagerClass,
                    "setAnimationScale", int.class, float.class);

    private Object mIWinowManager;

    private IWindowManagerExt(Object wm) {
        mIWinowManager = wm;
    }

    /**
     * Get the window manager extension instance.
     * @return extension instance.
     */
    public static IWindowManagerExt getWindowManagerService() {
        Object wm = ReflectUtil.callMethodOnObject(sWindowManagerGlobalClass,
                sGetWindowManagerServiceMethod);
        return new IWindowManagerExt(wm);
    }

    /**
     * Get the animation scale.
     * @param arg0 The param of scale.
     * @return the scale value.
     */
    public float getAnimationScale(int arg0) {
        return (float) ReflectUtil.callMethodOnObject(mIWinowManager,
                sGetAnimationScaleMethod, arg0);
    }

    public void setAnimationScale(int arg0, float arg1) {
        ReflectUtil.callMethodOnObject(mIWinowManager,
                sSetAnimationScaleMethod, arg0, arg1);
    }

}
