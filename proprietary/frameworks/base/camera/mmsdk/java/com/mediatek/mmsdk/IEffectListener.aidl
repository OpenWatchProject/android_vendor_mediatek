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
import com.mediatek.mmsdk.BaseParameters;
import com.mediatek.mmsdk.IEffectHalClient;


/** @hide */
oneway interface IEffectListener  //TTT3
//interface IEffectListener
{
    /**
     * Keep up-to-date with vendor/mediatek/proprietary/frameworks/av/include/mmsdk/IMMSdkService.h	
     */
    // rest of 'int' return values in this file are actually status_t
    void    onPrepared(IEffectHalClient effect, in BaseParameters result);
    void    onInputFrameProcessed(IEffectHalClient effect, in BaseParameters parameter, in BaseParameters partialResult);
    void    onOutputFrameProcessed(IEffectHalClient effect, in BaseParameters parameter, in BaseParameters partialResult);
    void    onCompleted(IEffectHalClient effect, in BaseParameters partialResult, long uid);
    void    onAborted(IEffectHalClient effect, in BaseParameters result);
    void    onFailed(IEffectHalClient effect, in BaseParameters result);
}
