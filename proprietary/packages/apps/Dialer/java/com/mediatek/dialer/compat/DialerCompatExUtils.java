/*
 * Copyright (C) 2015 The Android Open Source Project
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
package com.mediatek.dialer.compat;

import android.text.TextUtils;
import android.util.Log;

import com.google.common.collect.Lists;

import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;

/**
 * [portable]the compatible utility class.
 */
public final class DialerCompatExUtils {

    private static final String TAG = DialerCompatExUtils.class.getSimpleName();
    private static ArrayList<String> sExistedMethods = Lists.newArrayList();
    private static ArrayList<String> sUnExistedMethods = Lists.newArrayList();

    /**
     * Determines if the given class's method is available to call. Can be used to check if system
     * apis exist at runtime.
     *
     * @param className the name of the class to look for
     * @param methodName the name of the method to look for
     * @param parameterTypes the needed parameter types for the method to look for
     * @return {@code true} if the given class is available, {@code false} otherwise or if className
     * or methodName are empty.
     */
    public static boolean isMethodAvailable(String className, String methodName,
            Class<?>... parameterTypes) {
        if (TextUtils.isEmpty(className) || TextUtils.isEmpty(methodName)) {
            return false;
        }
        String methodStr = argsToString(className, methodName, parameterTypes);
        if (sExistedMethods.contains(methodStr)) {
            return true;
        }
        if (sUnExistedMethods.contains(methodStr)) {
            return false;
        }
        try {
            Class.forName(className).getMethod(methodName, parameterTypes);
            sExistedMethods.add(methodStr);
            return true;
        } catch (ClassNotFoundException | NoSuchMethodException e) {
            Log.v(TAG, "Could not find method, add it to UnExisted list: " + methodStr);
            sUnExistedMethods.add(methodStr);
            return false;
        } catch (Throwable t) {
            Log.e(TAG, "Unexpected exception when checking if method: " + className + "#"
                    + methodName + " exists at runtime", t);
            return false;
        }
    }

    /**
     * Determines if the given class's field is available. Can be used to check if system
     * apis exist at runtime.
     *
     * @param className the name of the class to look for
     * @param fieldName the name of the method to look for
     * @return {@code true} if the given class field is available, {@code false} otherwise
     * or if className or fieldName are empty.
     */
    public static boolean isFieldAvailable(String className, String fieldName) {
        if (TextUtils.isEmpty(className) || TextUtils.isEmpty(fieldName)) {
            return false;
        }
        try {
            Class.forName(className).getField(fieldName);
            return true;
        } catch (ClassNotFoundException | NoSuchFieldException e) {
            Log.v(TAG, "Could not find Field: " + className + "#" + fieldName);
            return false;
        } catch (Throwable t) {
            Log.e(TAG, "Unexpected exception when checking if Field: " + className + "#"
                    + fieldName + " exists at runtime", t);
            return false;
        }
    }

    private static String argsToString(String className, String methodName,
            Class[] argTypes) {
        StringBuilder buf = new StringBuilder();
        buf.append(className).append("#").append(methodName).append("#");
        buf.append("(");
        if (argTypes != null) {
            for (int i = 0; i < argTypes.length; i++) {
                if (i > 0) {
                    buf.append(", ");
                }
                Class c = argTypes[i];
                buf.append((c == null) ? "null" : c.getName());
            }
        }
        buf.append(")");
        return buf.toString();
    }

    /**
     * Determines if the given class is defined
     *
     * @param className the name of the class to look for
     * @return {@code true} if the given class is exit;
     */
    public static boolean isClassExits(String className) {
        if (TextUtils.isEmpty(className)) {
            return false;
        }
        try {
            Class.forName(className);
            return true;
        } catch (ClassNotFoundException e) {
            Log.v(TAG, "Could not find class" );
            return false;
        } catch (Throwable t) {
            Log.e(TAG, "Unexpected exception when checking if class exist");
            return false;
        }
    }
}
