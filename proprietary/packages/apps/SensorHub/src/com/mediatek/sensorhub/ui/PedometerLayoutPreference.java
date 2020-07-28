/*
 * Copyright (C) 2015 The Android Open Source Project
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

package com.mediatek.sensorhub.ui;

import android.content.Context;
import android.preference.Preference;
import android.util.AttributeSet;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

public class PedometerLayoutPreference extends Preference {

    private View mRootView;

    public PedometerLayoutPreference(Context context, AttributeSet attrs) {
        super(context, attrs);
        setSelectable(false);
        // Need to create view now so that findViewById can be called
        // immediately.
        final View view = LayoutInflater.from(getContext()).inflate(
                R.layout.sensor_pedometer_view, null, false);
        mRootView = view;
        setShouldDisableView(false);
    }

    @Override
    protected View onCreateView(ViewGroup parent) {
        return mRootView;
    }

    @Override
    protected void onBindView(View view) {
        // Do nothing.
    }

    public View findViewById(int id) {
        return mRootView.findViewById(id);
    }

}
