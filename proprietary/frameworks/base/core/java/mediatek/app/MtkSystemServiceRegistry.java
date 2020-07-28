/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2006 The Android Open Source Project
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
package mediatek.app;

import android.app.ContextImpl;
import android.app.SystemServiceRegistry;
import android.app.SystemServiceRegistry.ServiceFetcher;
import android.app.SystemServiceRegistry.CachedServiceFetcher;

import android.os.IBinder;
import android.os.ServiceManager;
import android.content.Context;
import android.os.Looper;
import android.os.ServiceManager.ServiceNotFoundException;

import android.net.ConnectivityThread;
import android.util.Log;
import com.mediatek.search.SearchEngineManager;
//only use for string define
//import com.mediatek.common.omadm.IOmadmManager;
//import com.mediatek.common.omadm.OmadmManager;

import java.lang.reflect.Method;
import java.lang.reflect.Constructor;
import java.lang.Object;
import android.util.ArrayMap;
import java.util.Optional;

public final class MtkSystemServiceRegistry {
    private static final String TAG = "MtkSystemServiceRegistry";

    // Service registry information.
    // This information is never changed once static initialization has completed.
    private static ArrayMap<Class<?>, String> sSystemServiceNames;
    private static ArrayMap<String, ServiceFetcher<?>> sSystemServiceFetchers;

    private MtkSystemServiceRegistry() { }

    ///Register service to here.
    public static void registerAllService () {
        Log.i(TAG, "registerAllService start");
        registerService("search_engine_service", SearchEngineManager.class,
           new CachedServiceFetcher<SearchEngineManager>() {
            @Override
            public SearchEngineManager createService(ContextImpl ctx) {
                return new SearchEngineManager(ctx);
            }});
        /// Register FmRadioService
        registerFmService();

        /// Register OmadmService
        registerOmadmService();

    }

    public static void setMtkSystemServiceName(ArrayMap<Class<?>, String> names,
            ArrayMap<String, ServiceFetcher<?>> fetchers) {
        Log.i(TAG, "setMtkSystemServiceName start names" + names + ",fetchers" + fetchers);
        sSystemServiceNames = names;
        sSystemServiceFetchers = fetchers;
    }

    /**
     * Statically registers a system service with the context.
     * This method must be called during static initialization only.
     */
    private static <T> void registerService(String serviceName, Class<T> serviceClass,
            ServiceFetcher<T> serviceFetcher) {
        sSystemServiceNames.put(serviceClass, serviceName);
        sSystemServiceFetchers.put(serviceName, serviceFetcher);
    }

    public static void registerFmService(){
        String className = "com.mediatek.fmradio.FmRadioPackageManager";
        Class<?> clazz = null;
        try {
            clazz = Class.forName(className);
            if (clazz != null) {
                Method method = clazz.getMethod("getPackageName", null);
                Object object = method.invoke(null);
                String clazzName = (String)object;
                clazz = Class.forName(clazzName);
                if(clazz != null) {
                    Constructor constructor =
                      clazz.getConstructor(new Class[]{Context.class, Looper.class});
                if (constructor != null) {
                    registerService("fm_radio_service", Optional.class,
                    new CachedServiceFetcher<Optional>() {
                        @Override
                        public Optional createService(ContextImpl ctx)
                          throws ServiceNotFoundException {
                           Optional optObj = Optional.empty();
                           Object obj = null;
                           try {
                               obj = constructor.newInstance(new Object[]{ctx,
                                 ConnectivityThread.getInstanceLooper()});
                               optObj = Optional.of(obj);
                           } catch (Exception e) {
                               Log.e(TAG, "Exception while creating FmRadioManager object");
                           }
                           return optObj;
                       }
                    });
               }
           }
        }
        }
        catch(Exception e) {
           Log.e(TAG, "Exception while getting FmRadioPackageManager class");
        }
    }

    public static void registerOmadmService(){
        String className = "com.mediatek.common.omadm.OmadmManager";
        Class<?> clazz = null;
        try {
            clazz = Class.forName(className);
            if(clazz != null) {
                Constructor constructor = clazz.getConstructor(new Class[]{Context.class});
                if (constructor != null) {
                    registerService("omadm_service", Optional.class,
                    new CachedServiceFetcher<Optional>() {
                        @Override
                        public Optional createService(ContextImpl ctx)
                          throws ServiceNotFoundException {
                           Optional optObj = Optional.empty();
                           Object obj = null;
                           try {
                               obj = constructor.newInstance(new Object[]{ctx});
                               optObj = Optional.of(obj);
                           } catch (Exception e) {
                               Log.e(TAG, "Exception while creating OmadmManager object");
                           }
                           return optObj;
                       }
                   });
                }
            }
        }
        catch(Exception e) {
           Log.e(TAG, "Exception while getting OmadmManager class");
        }
    }
}
