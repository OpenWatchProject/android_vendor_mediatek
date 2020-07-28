/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2007 The Android Open Source Project
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

package com.android.calendar;

import android.app.Application;

import com.mediatek.calendar.InjectedServices;

public class CalendarApplication extends Application {

    private static InjectedServices sInjectedServices;

    @Override
    public void onCreate() {
        super.onCreate();

        /*
         * Ensure the default values are set for any receiver, activity,
         * service, etc. of Calendar
         */
        GeneralPreferences.setDefaultValues(this);

        // Save the version number, for upcoming 'What's new' screen.  This will be later be
        // moved to that implementation.
        Utils.setSharedPreference(this, GeneralPreferences.KEY_VERSION,
                Utils.getVersionCode(this));
        // Initialize the registry mapping some custom behavior.
        ExtensionsFactory.init(getAssets());
    }

    /**
     * M: Overrides the system services with mocks for testing. Use
     * CalendarApplication.injectServices(services) to inject this mock system
     * service, use CalendarApplication.injectServices(null) to release the mock
     * service. It will auto set to null when testing over.
     */
    public static void injectServices(InjectedServices services) {
        sInjectedServices = services;
    }

    /**
     * M: When want a system account while testing, should use injectServices()
     * to inject one mock system service, and must change source code from
     * AccountManager.get(this) or AccountManager.get(context) to
     * AccountManager.get(getApplicationContext()), because just use
     * getApplicationContext(), the AccountManager.get() will call back to
     * CalendarApplication.class file.
     *
     * many using details look {@link TestCaseUtils}
     */
    @Override
    public Object getSystemService(String name) {
        if (sInjectedServices != null) {
            Object service = sInjectedServices.getSystemService(name);
            if (service != null) {
                return service;
            }
        }
        return super.getSystemService(name);
    }
}
