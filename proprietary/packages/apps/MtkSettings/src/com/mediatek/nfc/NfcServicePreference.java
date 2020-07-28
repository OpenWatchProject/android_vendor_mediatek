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

package com.mediatek.nfc;

import android.content.ComponentName;
import android.content.Context;
import androidx.preference.Preference;
import androidx.preference.PreferenceViewHolder;
import android.view.View;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.TextView;

import com.android.settings.R;
import com.mediatek.nfcsettingsadapter.ServiceEntry;

public class NfcServicePreference extends Preference {
    private CheckBox mCheckBox;
    private boolean mShowCheckBox;
    boolean mSelected;
    ComponentName mComponent;

    public NfcServicePreference(Context context, ServiceEntry serviceEntry) {
        super(context);
        setLayoutResource(R.layout.nfc_service);
        setIcon(serviceEntry.getIcon(context.getPackageManager()));
        setTitle(serviceEntry.getTitle());
        mSelected = serviceEntry.getWantEnabled().booleanValue();
        mComponent = serviceEntry.getComponent();
    }

    public void setShowCheckbox(boolean showCheckbox) {
        mShowCheckBox = showCheckbox;
    }

    public boolean isChecked() {
        return mCheckBox.isChecked();
    }

    public void setChecked(boolean checked) {
        mCheckBox.setChecked(checked);
    }

    @Override
    public void onBindViewHolder(PreferenceViewHolder viewHolder) {
        super.onBindViewHolder(viewHolder);
        mCheckBox = (CheckBox) viewHolder.findViewById(R.id.checkbox);
        mCheckBox.setChecked(mSelected);
        mCheckBox.setVisibility(mShowCheckBox ? View.VISIBLE : View.INVISIBLE);
    }
}
