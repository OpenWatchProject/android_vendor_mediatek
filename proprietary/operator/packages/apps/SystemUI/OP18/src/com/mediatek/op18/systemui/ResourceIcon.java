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

package com.mediatek.op18.systemui;

import android.content.Context;
import android.graphics.drawable.Animatable;
import android.graphics.drawable.Drawable;
import android.util.Log;
import android.util.SparseArray;

//import com.android.systemui.qs.QSTile.Icon;
import com.android.systemui.plugins.qs.QSTile.Icon;

/** Implementation of the Resource Icon. **/
public class ResourceIcon extends Icon {
    private static final String TAG = "OP18ResourceIcon";
    private static final SparseArray<Icon> ICONS = new SparseArray<Icon>();

    protected final int mResId;
    protected Context mContext;

    private ResourceIcon(int resId, Context context) {
        mResId = resId;
        mContext = context;
    }

    /**
         * To get Icon instance.
         * @param resId resId
         * @param context context
         * @return Icon Icon
         */
    public static Icon get(int resId, Context context) {
        Icon icon = ICONS.get(resId);
        if (icon == null) {
            Log.d(TAG, "making new icon:" + resId + "context:" + context);
            icon = new ResourceIcon(resId, context);
            ICONS.put(resId, icon);
        }
        Log.d(TAG, "icon:" + icon);
        return icon;
    }

    /**
         * To get Icon drawable.
         * @param context context
         * @return Drawable Drawable
         */

    @Override
    public Drawable getDrawable(Context context) {
        // context passed is host app context, so will  not use it
        // because we will use plugin context to inflate plugin resources
        Drawable d = mContext.getDrawable(mResId);
        Log.d(TAG, "drawable:" + d);
        if (d instanceof Animatable) {
            ((Animatable) d).start();
        }
        return d;
    }

    /**
         * Convert resId to string.
         * @return String 
         */
    @Override
        public String toString() {
        return String.format("ResourceIcon[resId=0x%08x]", mResId);
    }
}

