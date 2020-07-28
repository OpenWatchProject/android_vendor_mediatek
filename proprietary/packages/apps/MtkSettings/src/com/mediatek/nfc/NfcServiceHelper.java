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
import android.content.pm.PackageManager;
import android.graphics.drawable.Drawable;
import android.nfc.cardemulation.CardEmulation;
import android.os.Bundle;
import android.os.UserHandle;
import android.util.Log;

import com.android.settings.R;
import com.mediatek.nfcsettingsadapter.NfcSettingsAdapter;
import com.mediatek.nfcsettingsadapter.ServiceEntry;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class NfcServiceHelper {
    private static final String KEY_CHECKED_SERVICES = "checked_services";
    private static final String TAG = "NfcServiceHelper";
    private ArrayList<String> mCheckedServices;
    private Context mContext;
    private boolean mEditMode;
    private PackageManager mPm;
    private NfcSettingsAdapter mNfcSettingsAdapter;
    private List<ServiceEntry> mServiceEntryList;

    public NfcServiceHelper (Context context) {
        mContext = context;
        mNfcSettingsAdapter = NfcSettingsAdapter.getDefaultAdapter(mContext);
    }

    private final static Comparator<ServiceEntry> sServiceCompare
            = new Comparator<ServiceEntry>() {
        @Override
        public int compare(ServiceEntry ls, ServiceEntry rs) {
            if (ls.getWasEnabled().equals(rs.getWasEnabled())) {
                return ls.getTitle().compareTo(rs.getTitle());
            } else if (ls.getWasEnabled().booleanValue()) {
                return -1;
            } else if (rs.getWasEnabled().booleanValue()){
                return 1;
            }
            return 0;
        }
    };

    void initServiceList() {
        if (mNfcSettingsAdapter != null) {
            mServiceEntryList = mNfcSettingsAdapter.getServiceEntryList(UserHandle.myUserId());
            if (mServiceEntryList == null) {
                Log.e(TAG, "Cannot get ServiceEntry list");
            }
        }
    }

    void sortList() {
        if (mServiceEntryList != null) {
            Collections.sort(mServiceEntryList, sServiceCompare);
        }
    }

    void saveChange() {
        if (mNfcSettingsAdapter != null && mServiceEntryList != null) {
            mNfcSettingsAdapter.commitServiceEntryList(mServiceEntryList);
        }
    }

    void saveState(Bundle outInstanceState) {
        if (outInstanceState != null && mEditMode && mServiceEntryList != null) {
            mCheckedServices = new ArrayList<String>();
            for (ServiceEntry serviceEntry : mServiceEntryList) {
                if (serviceEntry.getWantEnabled().booleanValue()) {
                    mCheckedServices.add(serviceEntry.getComponent().flattenToString());
                }
            }
            outInstanceState.putStringArrayList(KEY_CHECKED_SERVICES, mCheckedServices);
        }
    }

    void restoreState(Bundle savedInstanceState) {
        Log.d(TAG, "restoreState mEditMode = " + mEditMode);
        if (savedInstanceState != null && mEditMode) {
            mCheckedServices = savedInstanceState.getStringArrayList(KEY_CHECKED_SERVICES);
        }
    }

    void restoreCheckedState() {
        if (mCheckedServices == null || mServiceEntryList == null) {
            return;
        }
        if(mEditMode) {
            for (ServiceEntry serviceEntry : mServiceEntryList) {
                serviceEntry.setWantEnabled(new Boolean(
                        mCheckedServices.contains(serviceEntry.getComponent().flattenToString())));
            }
        }
    }

    void setEditMode(boolean editMode) {
        mEditMode = editMode;
    }

    List<ServiceEntry> getServiceList() {
        return mServiceEntryList;
    }

    int getAllServiceCount() {
        int allCount = 0;
        if (mServiceEntryList != null) {
            allCount = mServiceEntryList.size();
        }
        return allCount;
    }

    int getSelectServiceCount() {
        int count=0;
        if (mServiceEntryList != null) {
            for (ServiceEntry service : mServiceEntryList) {
                if (service.getWasEnabled().booleanValue()) {
                    count++;
                }
            }
        }
        return count;
    }

    boolean setEnabled(NfcServicePreference pref, boolean enabled) {
        for (ServiceEntry serviceEntry : mServiceEntryList) {
            if (pref.mComponent.equals(serviceEntry.getComponent())) {
                Boolean previous = serviceEntry.getWantEnabled();
                serviceEntry.setWantEnabled(new Boolean(enabled));
                if (canDoAction()) {
                    return true;
                } else {
                    serviceEntry.setWantEnabled(previous);
                    return false;
                }
            }
        }
        return false;
    }

    // When user set one service as checked, we need to know whether it will cause nfc overflow.
    // If it will cause overflow, we don't allow this action.
    private boolean canDoAction() {
        boolean result = false;
        if (mNfcSettingsAdapter != null && mServiceEntryList != null) {
            result = mNfcSettingsAdapter.testServiceEntryList(mServiceEntryList);
        }
        Log.d(TAG, "Can do this action ? " + result);
        return result;
    }
}
