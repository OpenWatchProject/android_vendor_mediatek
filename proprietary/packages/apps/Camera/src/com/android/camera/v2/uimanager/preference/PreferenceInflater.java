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
 * MediaTek Inc. (C) 2014. All rights reserved.
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
package com.android.camera.v2.uimanager.preference;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Xml;
import android.view.InflateException;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.lang.reflect.Constructor;
import java.util.ArrayList;
import java.util.HashMap;

/**
 * Inflate <code>CameraPreference</code> from XML resource.
 */
public class PreferenceInflater {
    private static final String PACKAGE_NAME = PreferenceInflater.class.getPackage().getName();

    private static final Class<?>[] CTOR_SIGNATURE =
            new Class[] { Context.class,
            AttributeSet.class};
    private static final HashMap<String, Constructor<?>> sConstructorMap =
            new HashMap<String, Constructor<?>>();

    private Context mContext;
    private AttributeSet mAttrs;

    public PreferenceInflater(Context context) {
        mContext = context;
    }

    public CameraPreference inflate(int resId) {
        return inflate(mContext.getResources().getXml(resId));
    }

    private CameraPreference newPreference(String tagName, Object[] args) {
        String name = PACKAGE_NAME + "." + tagName;

        if (name.equals(PreferenceGroup.class.getName())) {
            return (CameraPreference) (new PreferenceGroup(mContext, mAttrs));
        } else if (name.equals(IconListPreference.class.getName())) {
            return (CameraPreference) (new IconListPreference(mContext, mAttrs));
        } else if (name.equals(ListPreference.class.getName())) {
            return (CameraPreference) (new ListPreference(mContext, mAttrs));
        } else if (name.equals(RecordLocationPreference.class.getName())) {
            return (CameraPreference) (new RecordLocationPreference(mContext, mAttrs));
        } else {
            return null;
        }
    }

    private CameraPreference inflate(XmlPullParser parser) {

        AttributeSet attrs = Xml.asAttributeSet(parser);
        mAttrs = attrs;
        ArrayList<CameraPreference> list = new ArrayList<CameraPreference>();
        Object args[] = new Object[] { mContext, attrs};

        try {
            for (int type = parser.next(); type != XmlPullParser.END_DOCUMENT; type
                    = parser.next()) {
                if (type != XmlPullParser.START_TAG)
                    continue;
                CameraPreference pref = newPreference(parser.getName(), args);

                int depth = parser.getDepth();
                if (depth > list.size()) {
                    list.add(pref);
                } else {
                    list.set(depth - 1, pref);
                }
                if (depth > 1) {
                    ((PreferenceGroup) list.get(depth - 2)).addChild(pref);
                }
            }

            if (list.size() == 0) {
                throw new InflateException("No root element found");
            }
            return list.get(0);
        } catch (XmlPullParserException e) {
            throw new InflateException(e);
        } catch (IOException e) {
            throw new InflateException(parser.getPositionDescription(), e);
        }
    }
}
