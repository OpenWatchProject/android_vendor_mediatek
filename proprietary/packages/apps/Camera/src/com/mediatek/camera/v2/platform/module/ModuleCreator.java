/*
 * Copyright (C) 2014 The Android Open Source Project
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

package com.mediatek.camera.v2.platform.module;

import com.mediatek.camera.v2.module.CameraModule;
import com.mediatek.camera.v2.module.DualCameraModule;
import com.mediatek.camera.v2.platform.app.AppController;

public class ModuleCreator {
    /**
     * According current module type create the relative module controller.
     * @param app
     *            the app controller.
     * @param isDualCamera
     *            whether is dual camera module or not, such as pip.
     * @return the module controller.
     */
    public static ModuleController create(AppController app, boolean isDualCamera) {
        ModuleController controller = null;
        if (isDualCamera) {
            controller = new DualCameraModule(app);
        } else {
            controller = new CameraModule(app);
        }
        return controller;
    }
}
