package com.mediatek.camera.tests.v3.util;

import com.mediatek.camera.common.debug.LogUtil;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

public class ReflectUtils {
    private static final LogUtil.Tag TAG = Utils.getTestTag(ReflectUtils.class.getSimpleName());

    /**
     * Call the specified method on object via reflect.
     *
     * @param receiver   The object on which to call the method
     * @param methodName The method want to call
     * @param args       The arguments to the method
     * @return the result
     */
    public static Object
    callMethodOnObject(Object receiver, String methodName, Object... args) {
        Method method = getMethod(receiver.getClass(), methodName, getTypeForParameters(args));
        return callMethodOnObject(receiver, method, args);
    }

    public static Object callMethodOnObject(Object receiver, Method method, Object... args) {
        try {
            return method.invoke(receiver, args);
        } catch (InvocationTargetException e2) {
            LogHelper.e(TAG, "[callMethodOnObject]", e2);
        } catch (IllegalAccessException e3) {
            LogHelper.e(TAG, "[callMethodOnObject]", e3);
        }
        return null;
    }

    /**
     * Create instance by the plugin ClassLoader.
     *
     * @param classPath   The class path.
     * @param classLoader Create instance by the classLoader.
     * @param args        Args for instance.
     * @return The instance.
     */
    public static Object createInstance(String classPath, ClassLoader classLoader,
                                        Object... args) {
        try {
            Class<?> clazz = classLoader.loadClass(classPath);
            Constructor<?> localConstructor =
                    getConstructor(clazz, getTypeForParameters(args));
            return createInstance(localConstructor, args);
        } catch (ClassNotFoundException e) {
            LogHelper.e(TAG, "[createInstance]", e);
        } catch (IllegalArgumentException e) {
            LogHelper.e(TAG, "[createInstance]", e);
        }
        return null;
    }

    public static Object createInstance(String classPath, Object... args) {
        Constructor<?> cons = getConstructor(classPath, getTypeForParameters(args));
        return createInstance(cons, args);
    }

    public static Object createInstance(Constructor<?> cons, Object... args) {
        try {
            return cons.newInstance(args);
        } catch (IllegalAccessException e1) {
            LogHelper.e(TAG, "[createInstance]", e1);
        } catch (IllegalArgumentException e2) {
            LogHelper.e(TAG, "[createInstance]", e2);
        } catch (InvocationTargetException e3) {
            LogHelper.e(TAG, "[createInstance]", e3);
        } catch (InstantiationException e4) {
            LogHelper.e(TAG, "[createInstance]", e4);
        }
        return null;
    }

    public static Constructor<?> getConstructor(String classPath, Class<?>... paramTypes) {
        try {
            Class<?> clazz = Class.forName(classPath);
            Constructor<?> cons = clazz.getDeclaredConstructor(paramTypes);
            cons.setAccessible(true);
            return cons;
        } catch (ClassNotFoundException e1) {
            LogHelper.e(TAG, "[getConstructor]", e1);
        } catch (NoSuchMethodException e2) {
            LogHelper.e(TAG, "[getConstructor]", e2);
        }
        return null;
    }

    /**
     * Get a private/protected method from a declared class.
     *
     * @param clazz          The class where you need to get private/protected method
     * @param methodName     The name of this method
     * @param parameterTypes parameter types
     * @return The field method
     */
    public static Method getMethod(Class<?> clazz, String methodName,
                                   Class<?>... parameterTypes) {
        try {
            Method method = clazz.getDeclaredMethod(methodName, parameterTypes);
            method.setAccessible(true);
            return method;
        } catch (NoSuchMethodException e) {
            LogHelper.e(TAG, "[getMethod]", e);
        }
        return null;
    }

    public static Field getField(Class<?> clazz, String fieldName) {
        try {
            Field field = clazz.getDeclaredField(fieldName);
            field.setAccessible(true);
            return field;
        } catch (NoSuchFieldException e) {
            LogHelper.e(TAG, "[getField]", e);
        }
        return null;
    }

    public static Object getFieldOnObject(Class<?> clazz, String fieldName, Object object) {
        Field field = getField(clazz, fieldName);
        if (field != null) {
            try {
                return field.get(object);
            } catch (IllegalAccessException e) {
                LogHelper.e(TAG, "[getFieldOnObject]", e);
            }
        }
        return null;
    }

    /**
     * Get a constructor from a declared class.
     *
     * @param clazz          The class where you need to get private/protected constructor
     * @param parameterTypes parameter array
     * @return The constructor
     */
    public static Constructor<?> getConstructor(Class<?> clazz, Class<?>... parameterTypes) {
        try {
            Constructor<?> constructor = clazz.getDeclaredConstructor(parameterTypes);
            constructor.setAccessible(true);
            return constructor;
        } catch (NoSuchMethodException e) {
            LogHelper.e(TAG, "[getConstructor]", e);
        }
        return null;
    }

    private static Class<?>[] getTypeForParameters(Object... args) {
        if (args.length == 0) {
            return new Class<?>[]{};
        }
        Class<?>[] res = new Class[args.length];
        int index = 0;
        for (Object o : args) {
            res[index++] = o.getClass();
        }
        return res;
    }
}
