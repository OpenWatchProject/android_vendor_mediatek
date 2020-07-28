/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2008 The Android Open Source Project
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

package com.android.browser;

import android.webkit.ValueCallback;

/**
 * @hide
 */
public class SavePageClient {

    /**
     * ask the app for download path.
     * @hide
     * @param callback the method to set the save dir
     * @param canSaveAsComplete can the page save as complete or not
     */
    public void getSaveDir(ValueCallback<String> callback, boolean canSaveAsComplete) {
    }

    /**
     * save page start callback.
     * @hide
     * @param id the id of the save page
     * @param path the path of the save page which store
     */
    public void onSavePageStart(int id, String path) {
    }

    /**
     * save progress change callback.
     * @hide
     * @param progress the progress of the save process
     * @param id the id of the save page
     */
    public void onSaveProgressChange(int progress, int id) {
    }

    /**
     * save finish callback.
     * @hide
     * @param flag the flag of the state of save page
     * @param id the id of the save page
     */
    public void onSaveFinish(int flag, int id) {
    }
}
