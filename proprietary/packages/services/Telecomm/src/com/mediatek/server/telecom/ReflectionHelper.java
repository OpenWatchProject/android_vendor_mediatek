/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.server.telecom;

import android.telecom.Log;

import dalvik.system.PathClassLoader;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

/**
 * Reflection help class.
 */
public class ReflectionHelper {
    private static final String TAG = ReflectionHelper.class.getSimpleName();

    private static Result sFailure = new Result(false, null);

    /**
     * Replace static field.
     *
     * @param className Class name.
     * @param fieldName Static field name.
     * @param newInstance Class instance.
     */
    public static void replaceStaticField(
            String className, String fieldName, Object newInstance) {
        boolean success = false;
        Throwable t = null;
        try {
            Class clazz = Class.forName(className);
            Field field = clazz.getDeclaredField(fieldName);
            field.setAccessible(true);
            field.set(null, newInstance);
            success = true;
        } catch (ClassNotFoundException e) {
            t = e;
        } catch (NoSuchFieldException e) {
            t = e;
        } catch (IllegalAccessException e) {
            t = e;
        }
        if (!success) {
            Log.e(TAG, t, "Failed to replace static field for %s.%s = %s",
                    className, fieldName, newInstance);
        }
    }

    /**
     * Invoke method
     *
     * @param classPackage Class package.
     * @param className Class name.
     * @param methodName Method name.
     * @param params The params of method.
     * @return The invoke result.
     */
    public static Result callMethod(
            String classPackage, String className, String methodName,
            Object... params) throws InvocationTargetException {
        Throwable t = null;
        try {
            Class clazz;
            if (classPackage != null) {
                PathClassLoader loader = new PathClassLoader(classPackage,
                        ReflectionHelper.class.getClassLoader());
                clazz = Class.forName(className, false, loader);
            } else {
                clazz = Class.forName(className);
            }
            Class[] paramTypes = new Class[params.length];
            for (int i = 0; i < params.length; i++) {
                paramTypes[i] = params[i].getClass();
            }
            Method method = clazz.getDeclaredMethod(methodName, paramTypes);
            method.setAccessible(true);
            Object ret = method.invoke(null, params);
            return new Result(true, ret);
        } catch (ClassNotFoundException e) {
            t = e;
        } catch (InvocationTargetException e) {
            Log.e(TAG, e, "[callMethod]Exception occurred to the invoke of" +
                    " %s.%s, throw it", className, methodName);
            throw e;
        } catch (NoSuchMethodException e) {
            t = e;
        } catch (IllegalAccessException e) {
            t = e;
        }
        Log.e(TAG, t, "[callMethod]Failed to call %s.%s", className, methodName);
        return sFailure;
    }

    /**
     * Invoke static method.
     *
     * @param classPackage Class package.
     * @param className Class name.
     * @param methodName Method name.
     * @param params The params of method.
     * @return The invoke result.
     */
    public static Result callStaticMethod(
            String classPackage, String className, String methodName,
            Object... params) throws InvocationTargetException {
        return callMethod(classPackage, className, methodName, params);
    }

    /**
     * The Result of execute method.
     */
    public static class Result {
        public boolean mSuccess;
        public Object mReturn;
        public Result(boolean success, Object returnValue) {
            mSuccess = success;
            mReturn = returnValue;
        }
    }
}
