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

import android.graphics.GraphicBuffer;
import android.view.Surface;
import com.mediatek.mmsdk.BinderHolder;
import com.mediatek.mmsdk.EffectHalVersion;
import com.mediatek.mmsdk.BaseParameters;
import com.mediatek.mmsdk.IEffectListener;
//@see IBatteryPropertiesListener.aidl
//@see ICameraServiceListener


/** @hide */
interface IEffectHalClient
{
    /**
     * Keep up-to-date with vendor/mediatek/proprietary/frameworks/av/include/mmsdk/IMMSdkService.h	
     */
    // rest of 'int' return values in this file are actually status_t

    int     init();
    int     uninit();
    int     configure();
    int     unconfigure();
    long    start();
    int     abort(in BaseParameters effectParameter);
    
    int     getNameVersion(out EffectHalVersion version);
    int     setEffectListener(IEffectListener listener);
    int     setParameter(in String key, in String paramValue); //@todo fix this
    int     setParameters(in BaseParameters parameter); 
    int     getCaptureRequirement(in BaseParameters effectParameter, out List<BaseParameters> requirement);
    int     prepare();
    int     release();

   // int     addInputFrame(in GraphicBuffer frame, in BaseParameters parameter);
   //int     addOutputFrame(in GraphicBuffer frame, in BaseParameters parameter);

    //int     getInputSurfaces(out List<BinderHolder> input);
    int     getInputSurfaces(out List<Surface> input);
    int     setOutputSurfaces(in List<Surface> output, in List<BaseParameters> parameters);
    int     addInputParameter(int index, in BaseParameters parameter, long timestamp, boolean repeat);
    int     addOutputParameter(int index, in BaseParameters parameter, long timestamp, boolean repeat);
    int     setInputsyncMode(int index, boolean sync);
    boolean getInputsyncMode(int index);
    int     setOutputsyncMode(int index, boolean sync);
    boolean getOutputsyncMode(int index);
    int     setBaseParameter(in BaseParameters parameters);
    int     dequeueAndQueueBuf(long timestamp);

}
