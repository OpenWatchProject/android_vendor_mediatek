/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.common.util;

import android.annotation.ProductApi;
import android.content.Context;
import android.text.TextUtils;
import android.util.Log;

import dalvik.system.PathClassLoader;

import java.io.File;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * OperatorCustomizationFactoryLoader. It's help to load operator customization factory from give
 * apk or jar file.
 *
 */
public class OperatorCustomizationFactoryLoader {

    private static final String TAG = "OperatorCustomizationFactoryLoader";

    private static final String USP_PACKAGE
                    = getSysProperty("ro.vendor.mtk_carrierexpress_pack", "no");
    private static final String SYSTEM_JAR_PATH = "/system/operator/libs/";
    private static final String CUSTOM_JAR_PATH = "/custom/operator/libs/";
    private static final String SYSTEM_APK_PATH = "/system/app/";
    private static final String CUSTOM_APK_PATH = "/custom/app/";
    private static final String PRODUCT_APK_PATH = "/product/app/";
    private static final String RSC_SYSTEM_APK_PATH
                    = getSysProperty("ro.sys.current_rsc_path", "");
    private static final String RSC_PRODUCT_APK_PATH
                    = getSysProperty("ro.product.current_rsc_path", "");
    private static final String  PROPERTY_OPERATOR_OPTR = "persist.vendor.operator.optr";
    private static final String  PROPERTY_OPERATOR_SEG = "persist.vendor.operator.seg";
    private static final String  PROPERTY_OPERATOR_SPEC = "persist.vendor.operator.spec";
    private static final boolean LOG_ENABLE = "eng".equals(getSysProperty("ro.build.type", "eng"))
                                               || Log.isLoggable(TAG, Log.DEBUG);

    private static final Map<OperatorFactoryInfo, Object> sFactoryMap =
                                new HashMap<OperatorFactoryInfo, Object>();

    private static class OperatorInfo {
        /** It's same as property of persist.vendor.operator.optr. */
        private String mOperator;

        /** It's same as property of persist.vendor.operator.spec. */
        private String mSpecification;

        /** It's same as property of persist.vendor.operator.seg. */
        private String mSegment;

        public OperatorInfo(String optr, String spec, String seg) {
            mOperator = optr;
            mSpecification = spec;
            mSegment = seg;
        }

        @Override
        public String toString() {
            return mOperator + "_" + mSpecification + "_" + mSegment;
        }
    }

    /**
     * OperatorFactoryInfo.
     *
     */
    public static class OperatorFactoryInfo {
        /** It's same as property of persist.vendor.operator.optr. */
        String mOperator;

        /** It's same as property of persist.vendor.operator.spec. */
        String mSpecification;

        /** It's same as property of persist.vendor.operator.seg. */
        String mSegment;

        /** apk or jar name. */
        String mLibName;

        /** factory class name. */
        String mFactoryName;

        /** apk's package name. it is null for jar. */
        String mPackageName;

        /**
         * OperatorFactoryInfo Construction.
         *
         * @param libName apk or jar name where factory in, it can not be null.
         * @param factoryName factory class name, it can not be null.
         * @param packageName apk's package name, set it as null if libName is a apk.
         * @param operator operator name. it's same as system property
         * "persist.vendor.operator.optr".
         * ex. OP01/OP02/OP03 ...
         */
        @ProductApi
        public OperatorFactoryInfo(String libName, String factoryName, String packageName,
                String operator) {
            this(libName, factoryName, packageName, operator, null, null);
        }

        /**
         * OperatorFactoryInfo Construction.
         *
         * @param libName apk or jar name where factory in, it can not be null.
         * @param factoryName factory class name, it can not be null.
         * @param packageName apk's package name, set it as null if libName is a apk.
         * @param operator operator name. it's same as system property
         * "persist.vendor.operator.optr".
         * ex. OP01/OP02/OP03 ...
         * @param segment operator segment. it's same as system property of
         * "persist.vendor.operator.seg";
         */
        @ProductApi
        public OperatorFactoryInfo(String libName, String factoryName, String packageName,
                String operator, String segment) {
            this(libName, factoryName, packageName, operator, segment, null);
        }

        /**
         * OperatorFactoryInfo Construction.
         *
         * @param libName apk or jar name where factory in, it can not be null.
         * @param factoryName factory class name, it can not be null.
         * @param packageName apk's package name, set it as null if libName is a apk.
         * @param operator operator name. it's same as system property
         * "persist.vendor.operator.optr".
         * ex. OP01/OP02/OP03 ...
         * @param segment operator segment. it's same as system property of
         * "persist.vendor.operator.seg";
         * @param specification operator specification.it's same as system property of
         * "persist.vendor.operator.spec";
         */
        @ProductApi
        public OperatorFactoryInfo(String libName, String factoryName, String packageName,
                String operator, String segment, String specification) {
            mLibName = libName;
            mFactoryName = factoryName;
            mPackageName = packageName;
            mOperator = operator;
            mSegment = segment;
            mSpecification = specification;
        }

        @Override
        public String toString() {
            return "OperatorFactoryInfo(" + mOperator + "_" + mSpecification + "_" + mSegment
                    + ":" + mLibName + ":" + mFactoryName + ":" + mPackageName + ")";
        }
    }

    /**
     * Get active operator info.
     *
     * @return the Operator Info.
     *
     */
    private static OperatorInfo getActiveOperatorInfo() {
        OperatorInfo info =  new OperatorInfo(getSysProperty(PROPERTY_OPERATOR_OPTR, ""),
                getSysProperty(PROPERTY_OPERATOR_SPEC, ""),
                getSysProperty(PROPERTY_OPERATOR_SEG, ""));
        return info;
    }

    /**
     * Get active operator info.
     *
     * @return the Operator Info.
     *
     */
    private static OperatorInfo getActiveOperatorInfo(int slot) {
        OperatorInfo info = null;
        if (slot != -1 && !"no".equals(USP_PACKAGE)) {
            String optrProperty = getSysProperty("persist.vendor.mtk_usp_optr_slot_" + slot, "");
            logD("usp optr property is " + optrProperty);
            if (!TextUtils.isEmpty(optrProperty)) {
                String[] items = optrProperty.split("_");
                if (items != null) {
                    if (items.length == 1) {
                        info = new OperatorInfo(items[0], "", "");
                    } else if (items.length == 3) {
                        info = new OperatorInfo(items[0], items[1], items[2]);
                    } else {
                        logE("usp optr property no content or wrong");
                    }
                }
            }
        } else {
            info = getActiveOperatorInfo();
        }
        logD("Slot " + slot + "'s OperatorInfo is" + info);
        return info;
    }

   /**
     * Load Operator customization factory by {@link OperatorFactoryInfo} list. It will get active
     * operator information first, then find out the matched {@link OperatorFactoryInfo} from the
     * list.
     *
     * @param clazzLoader ClassLoader. Will use it as Prarent ClassLoader to create a
     * PathLoaderLoader with apk/lib name
     * @param list OperatorFactoryInfo list. Caller should pass correct {@link OperatorFactoryInfo}
     * list
     * @return Factory instance if found, else return null
     */
    @ProductApi
    public static Object loadFactory(ClassLoader clazzLoader, List<OperatorFactoryInfo> list) {
        return loadFactory(clazzLoader, list, -1);
    }

    /**
     * Load Operator customization factory by {@link OperatorFactoryInfo} list. It will get active
     * operator information first, then find out the matched {@link OperatorFactoryInfo} from the
     * list. To support Carrier Express 2.0.
     *
     * @param clazzLoader ClassLoader. Will use it as Prarent ClassLoader to create a
     * PathLoaderLoader with apk/lib name
     * @param list OperatorFactoryInfo list. Caller should pass correct {@link OperatorFactoryInfo}
     * list
     * @param slot slot index
     * @return Factory instance if found, else return null
     */
    @ProductApi
    public static Object loadFactory(ClassLoader clazzLoader, List<OperatorFactoryInfo> list,
            int slot) {
        return loadFactory(clazzLoader, null, list, slot);
    }

    /**
     * Load Operator customization factory by {@link OperatorFactoryInfo} list. It will get active
     * operator information first, then find out the matched {@link OperatorFactoryInfo} from the
     * list.
     *
     * @param context application context
     * @param list OperatorFactoryInfo list. Caller should pass correct {@link OperatorFactoryInfo}
     * list
     * @return Factory instance if found, else return null
     */
    @ProductApi
    public static Object loadFactory(Context context, List<OperatorFactoryInfo> list) {
        return loadFactory(context, list, -1);
    }

    /**
     * Load Operator customization factory by {@link OperatorFactoryInfo} list. It will get active
     * operator information first, then find out the matched {@link OperatorFactoryInfo} from the
     * list. To support Carrier Express 2.0.
     *
     * @param context application context
     * @param list OperatorFactoryInfo list. Caller should pass correct {@link OperatorFactoryInfo}
     * list
     * @param slot slot index
     * @return Factory instance if found, else return null
     */
    @ProductApi
    public static synchronized Object loadFactory(Context context, List<OperatorFactoryInfo> list,
            int slot) {
        return loadFactory(null, context, list, slot);
    }

    private static synchronized Object loadFactory(ClassLoader clazzLoader, Context context,
            List<OperatorFactoryInfo> list, int slot) {
        if (list == null) {
            logE("loadFactory failed, because param list is null");
            return null;
        }
        OperatorFactoryInfo factoryInfo = findOpertorFactoryInfo(list, slot);
        if (factoryInfo == null) {
            StringBuilder sb = new StringBuilder();
            for (int index = 0; index < list.size(); index ++) {
                sb.append(index + ": ").append(list.get(index)).append("\n");
            }
            logD("can not find operatorFactoryInfo by slot id " + slot + " from \n"
                        + sb.toString());
            return null;
        }

        //get factory from cache first.
        Object factory = sFactoryMap.get(factoryInfo);
        if (factory != null) {
            logD("return " + factory + " from cache by " + factoryInfo);
            return factory;
        }
        String path = searchTargetPath(factoryInfo.mLibName);
        if (TextUtils.isEmpty(path)) {
//            logE("can not find target " + factoryInfo.mLibName);
            return null;
        }

        factory = loadFactoryInternal(clazzLoader, context, path, factoryInfo.mFactoryName,
                                        factoryInfo.mPackageName);
        if (factory != null) {
            //save factory to cache.
            sFactoryMap.put(factoryInfo, factory);
        }
        return factory;
    }

    private static Object loadFactoryInternal(ClassLoader clazzLoader, Context context,
            String target, String factoryClassName, String packageName) {
        logD("load factory " + factoryClassName + " from " + target +
                " whose packageName is " + packageName + ", context is " + context);
        try {
            ClassLoader classLoader;
            if (clazzLoader != null) {
                classLoader = new PathClassLoader(target, clazzLoader);
            } else if (context != null) {
                classLoader = new PathClassLoader(target, context.getClassLoader());
            } else {
                classLoader = new PathClassLoader(target,
                                        ClassLoader.getSystemClassLoader().getParent());
            }

            Class<?> clazz = classLoader.loadClass(factoryClassName);
            logD("Load class : " +  factoryClassName
                        + " successfully with classLoader:" + classLoader);

            if (!TextUtils.isEmpty(packageName) && context != null) {
                try {
                    Constructor<?> constructor = clazz.getConstructor(Context.class);
                    Context opContext = context.createPackageContext(packageName,
                            Context.CONTEXT_INCLUDE_CODE | Context.CONTEXT_IGNORE_SECURITY);
                    return constructor.newInstance(opContext);
                } catch (NoSuchMethodException e) {
                    // Use default constructor
                    logD("Exception occurs when using constructor with Context");
                } catch (InvocationTargetException e) {
                    // Use default constructor
                    Log.e(TAG, "Exception occurs when execute constructor with Context", e);
                }
            }
            return clazz.newInstance();
        } catch (Exception ex) {
            Log.e(TAG, "Exception when initial instance", ex);
        }
        return null;
    }

    private static String getSysProperty(String prop, String def) {
        String propValue = "";
        try {
            Class<?> systemPropertiesClass = Class.forName("android.os.SystemProperties");
            Method getPropMethod =
                    systemPropertiesClass.getMethod("get", String.class, String.class);
            propValue = (String) getPropMethod.invoke(null, prop, def);
//            logD("SystemProperties " + prop + " is" + propValue);
        } catch (ClassNotFoundException e) {
            logE("Get system properties failed! " + e);
        } catch (NoSuchMethodException e) {
            logE("Get system properties failed! " + e);
        } catch (IllegalAccessException e) {
            logE("Get system properties failed! " + e);
        } catch (InvocationTargetException e) {
            logE("Get system properties failed! " + e);
        }
        return propValue;
    }

    private static String searchTargetPath(String target) {
        if (TextUtils.isEmpty(target)) {
            logE("target is null");
            return null;
        }
        final String[] searchFolders;
        String search = target;
        if (target.endsWith(".apk")) {
            search = target.substring(0, target.length() - 4) + '/' + target;
//            logD("ro.sys.current_rsc_path value is " + RSC_SYSTEM_APK_PATH);
            if (!TextUtils.isEmpty(RSC_SYSTEM_APK_PATH)) {
                // support RSC. RSC_SYSTEM_APK_PATH and RSC_PRODUCT_APK_PATH are
                // both empty or not on the same project.
                searchFolders = new String[] {RSC_SYSTEM_APK_PATH + "/app/",
                                              RSC_PRODUCT_APK_PATH + "/app/",
                                              SYSTEM_APK_PATH,
                                              PRODUCT_APK_PATH,
                                              CUSTOM_APK_PATH};
            } else {
                searchFolders = new String[] {SYSTEM_APK_PATH,
                                              PRODUCT_APK_PATH,
                                              CUSTOM_APK_PATH};
            }
        } else {
            searchFolders = new String[] {SYSTEM_JAR_PATH, CUSTOM_JAR_PATH};
        }
        for (String folder : searchFolders) {
            File file = new File(folder + search);
            if (file.exists()) {
                return folder + search;
            }
        }
        logD("can not find target " + target + " in " + Arrays.toString(searchFolders));
        return null;
    }

    private static OperatorFactoryInfo findOpertorFactoryInfo(List<OperatorFactoryInfo> list,
            int slot) {
        OperatorFactoryInfo ret = null;
        OperatorInfo optrInfo = getActiveOperatorInfo(slot);
        if (optrInfo == null || TextUtils.isEmpty(optrInfo.mOperator)) {
            //it's OM load
            logD("It's OM load or parse failed, because operator is null");
            return ret;
        }

        List<OperatorFactoryInfo> unSignedOperatorIdFactoryInfos =
                new ArrayList<OperatorCustomizationFactoryLoader.OperatorFactoryInfo>();
        for (OperatorFactoryInfo factoryInfo : list) {
            if (optrInfo.mOperator.equals(factoryInfo.mOperator)) {
                if (factoryInfo.mSegment != null) {
                    if (factoryInfo.mSegment.equals(optrInfo.mSegment)) {
                        if (factoryInfo.mSpecification == null ||
                                factoryInfo.mSpecification.equals(optrInfo.mSpecification)) {
                            ret = factoryInfo;
                            break;
                        }
                    }
                } else if (factoryInfo.mSpecification == null ||
                        factoryInfo.mSpecification.equals(optrInfo.mSpecification)) {
                    ret = factoryInfo;
                    break;
                }
            } else if (TextUtils.isEmpty(factoryInfo.mOperator)) {
                unSignedOperatorIdFactoryInfos.add(factoryInfo);
            }
        }
        if (ret == null) {
            // if not found, research from unsigned operator id list
            for (OperatorFactoryInfo factoryInfo : unSignedOperatorIdFactoryInfos) {
                String target = searchTargetPath(factoryInfo.mLibName);
                if (!TextUtils.isEmpty(target)) {
                    ret = factoryInfo;
                    break;
                }
            }
        }
        return ret;
    }

    private static void logD(String log) {
        if (LOG_ENABLE) {
            Log.d(TAG, log);
        }
    }

    private static void logE(String log) {
        if (LOG_ENABLE) {
            Log.e(TAG, log);
        }
    }
}
