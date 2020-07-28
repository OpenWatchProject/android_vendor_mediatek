/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
/**
 * ****************************************************************************
 *
 * <p>The original Work has been changed by ST Microelectronics S.A.
 *
 * <p>Copyright (C) 2017 ST Microelectronics S.A.
 *
 * <p>Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of the License at
 *
 * <p>http://www.apache.org/licenses/LICENSE-2.0
 *
 * <p>Unless required by applicable law or agreed to in writing, software distributed under the
 * License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * <p>****************************************************************************
 */
package com.mediatek.nfcsettingsadapter;

import android.content.ComponentName;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.res.Resources;
import android.content.res.Resources.NotFoundException;
import android.graphics.drawable.Drawable;
import android.os.Parcel;
import android.os.Parcelable;
import android.util.Log;

/**
 * These entries only represent the card emulation services declared by applications with at least
 * one AID group that belongs to the category CardEmulation.CATEGORY_OTHER. Only those specific
 * groups can be enabled or disabled. CATEGORY_PAYMENT groups are handled differently in Android
 * (Tap&Pay).
 *
 * <p>The content of this class must be considered as read-only from Settings application.
 */
public final class ServiceEntry implements Parcelable {
    /** Name of the component that registered this service */
    String tag;

    /** Name of the AID group: either the service name or the specific AID group. */
    String title;

    /** Resource id of the banner of the service */
    Integer banner;

    /** Is this service currently enabled in the committed table? */
    Boolean wasEnabled;

    /** If user wants to change the state */
    Boolean wantEnabled;

    // For internal use
    ComponentName component;

    private static String TAG = "ServiceEntry";

    public ServiceEntry(
            ComponentName component,
            String tag,
            String title,
            Integer banner,
            Boolean wasEnabled,
            Boolean wantEnabled) {
        this.component = component;
        this.tag = tag;
        this.title = title;
        this.banner = banner;
        this.wasEnabled = wasEnabled;
        this.wantEnabled = wantEnabled;
    }

    public Drawable getIcon(PackageManager pm) {
        try {
            Drawable icon = pm.getApplicationIcon(component.getPackageName());
            return icon;
        } catch (NameNotFoundException e) {
            Log.e(TAG, "Could not load icon.");
            return null;
        }
    }

    public Drawable getBanner(PackageManager pm) {
        Resources res;
        try {
            res = pm.getResourcesForApplication(component.getPackageName());
            Drawable b = res.getDrawable(banner);
            return b;
        } catch (NotFoundException e) {
            Log.e(TAG, "Could not load banner.");
            return null;
        } catch (NameNotFoundException e) {
            Log.e(TAG, "Could not load banner.");
            return null;
        }
    }

    public Integer getBannerId() {
        return banner;
    }

    public String getTag() {
        return tag;
    }

    public String getTitle() {
        return title;
    }

    public Boolean getWasEnabled() {
        return wasEnabled;
    }

    public Boolean getWantEnabled() {
        return wantEnabled;
    }

    public void setWantEnabled(Boolean enable) {
        wantEnabled = enable;
    }

    public ComponentName getComponent() {
        return component;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(tag);
        dest.writeString(title);
        dest.writeInt(banner);
        if (wasEnabled.booleanValue()) {
            dest.writeInt(1);
        } else {
            dest.writeInt(0);
        }
        if (wantEnabled.booleanValue()) {
            dest.writeInt(1);
        } else {
            dest.writeInt(0);
        }
        dest.writeParcelable(component, flags);
    }

    public static final Parcelable.Creator<ServiceEntry> CREATOR =
            new Parcelable.Creator<ServiceEntry>() {
                @Override
                public ServiceEntry createFromParcel(Parcel source) {
                    String tag = source.readString();
                    String title = source.readString();
                    Integer banner = source.readInt();
                    Boolean wasEnabled = new Boolean(source.readInt() != 0);
                    Boolean wantEnabled = new Boolean(source.readInt() != 0);
                    ComponentName component = null;
                    if (getClass().getClassLoader() != null) {
                        component =
                                (ComponentName) source.readParcelable(getClass().getClassLoader());
                    }

                    return new ServiceEntry(component, tag, title, banner, wasEnabled, wantEnabled);
                }

                @Override
                public ServiceEntry[] newArray(int size) {
                    return new ServiceEntry[size];
                }
            };
}
