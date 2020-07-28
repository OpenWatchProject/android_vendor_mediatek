/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2008 Esmertec AG.
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

package com.android.mms.model;

import java.util.concurrent.CopyOnWriteArrayList;

public class Model {
    protected CopyOnWriteArrayList<IModelChangedObserver> mModelChangedObservers =
            new CopyOnWriteArrayList<IModelChangedObserver>();

    public void registerModelChangedObserver(IModelChangedObserver observer) {
        if (!mModelChangedObservers.contains(observer)) {
            mModelChangedObservers.add(observer);
            registerModelChangedObserverInDescendants(observer);
        }
    }

    public void unregisterModelChangedObserver(IModelChangedObserver observer) {
        mModelChangedObservers.remove(observer);
        unregisterModelChangedObserverInDescendants(observer);
    }

    public void unregisterAllModelChangedObservers() {
        unregisterAllModelChangedObserversInDescendants();
        mModelChangedObservers.clear();
    }

    protected void notifyModelChanged(boolean dataChanged) {
        for (IModelChangedObserver observer : mModelChangedObservers) {
            observer.onModelChanged(this, dataChanged);
        }
    }

    protected void registerModelChangedObserverInDescendants(
            IModelChangedObserver observer) {
        // Dummy method.
    }

    protected void unregisterModelChangedObserverInDescendants(
            IModelChangedObserver observer) {
        // Dummy method.
    }

    protected void unregisterAllModelChangedObserversInDescendants() {
        // Dummy method.
    }

    /*
     * M: Whether the model should be update or not.
     * If one model is not modified, it's no need to save in db.
     */
    protected boolean mNeedUpdate = false;

    /// M: add page index for each model. @{
    private int mCurrentPage;
    public int getCurrentPage() {
        return mCurrentPage;
    }
    public void setCurrentPage(int mCurrentPage) {
        this.mCurrentPage = mCurrentPage;
    }
    /// @}

    public boolean needUpdate() {
        return mNeedUpdate;
    }

    public void setNeedUpdate(boolean needUpdate) {
        this.mNeedUpdate = needUpdate;
    }
}
