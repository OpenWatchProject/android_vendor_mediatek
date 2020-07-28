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

package com.mediatek.mmsdk;

import java.util.List;
import com.mediatek.mmsdk.BinderHolder;
import com.mediatek.mmsdk.EffectHalVersion;

/** @hide */
interface IEffectFactory
{
    /**
     * Keep up-to-date with vendor/mediatek/proprietary/frameworks/av/include/mmsdk/IMMSdkService.h
     */
    // rest of 'int' return values in this file are actually status_t
    //@see    void setFindAccessibilityNodeInfosResult(in List<AccessibilityNodeInfo> infos,
    int createCallbackClient(in EffectHalVersion nameVersion, out BinderHolder callbackClient);
    int createEffectHal(in EffectHalVersion nameVersion, out BinderHolder effectHal);
    int createEffectHalClient(in EffectHalVersion nameVersion, out BinderHolder effectHalClient);
    int getSupportedVersion(String effectName, out List<EffectHalVersion> versions);
    //@todo implement this - wait chengtian
    //int getSupportedVersion(in List<String> effectNames, out List<List<EffectHalVersion>> versions);  //@todo
    int     getAllSupportedEffectHal(out List<String> version);
}
