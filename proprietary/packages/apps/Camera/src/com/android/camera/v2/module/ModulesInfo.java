/*
 * Copyright (C) 2013 The Android Open Source Project
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

package com.android.camera.v2.module;

import com.android.camera.v2.app.AppController;
import com.android.camera.v2.app.ModuleManager;
import com.android.camera.v2.bridge.ModuleControllerAdapter;

import android.content.Context;

/**
 * A class holding the module information and registers them to
 * {@link com.android.camera.v2.app.ModuleManager}.
 */
public class ModulesInfo {

    public static void setupModules(Context context, ModuleManager moduleManager) {
        registerCameraModule(moduleManager, ModuleControllerAdapter.CAMERA_MODULE_INDEX);
        registerDualCameraModule(moduleManager, ModuleControllerAdapter.DUAL_CAMERA_MODULE_INDEX);
    }

    private static void registerCameraModule(ModuleManager moduleManager, final int moduleId) {
        moduleManager.registerModule(new ModuleManager.ModuleAgent() {
            @Override
            public int getModuleId() {
                return moduleId;
            }

            @Override
            public boolean requestAppForCamera() {
                return false;
            }

            @Override
            public ModuleController createModule(AppController app) {
                return new ModuleControllerAdapter(app, moduleId);
            }
        });
    }

    private static void registerDualCameraModule(ModuleManager moduleManager, final int moduleId) {
        moduleManager.registerModule(new ModuleManager.ModuleAgent() {
            @Override
            public int getModuleId() {
                return moduleId;
            }

            @Override
            public boolean requestAppForCamera() {
                return false;
            }

            @Override
            public ModuleController createModule(AppController app) {
                return new ModuleControllerAdapter(app, moduleId);
            }
        });
    }
}
