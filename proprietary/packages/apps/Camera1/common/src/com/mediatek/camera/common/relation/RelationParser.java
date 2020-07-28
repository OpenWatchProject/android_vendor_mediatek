/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensor. Without
 * the prior written permission of MediaTek inc. and/or its licensor, any
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
 * NON-INFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
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
package com.mediatek.camera.common.relation;

import android.content.Context;
import android.util.AttributeSet;
import android.util.Log;
import android.util.Xml;
import android.view.InflateException;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.IOException;
import java.io.InputStream;

/**
 * This class is used to parse xml file which defines the relation
 * between features.
 */
public class RelationParser {
    private static final String TAG = RelationParser.class.getSimpleName();
    private static final String TAG_RELATION_GROUP = "RelationGroup";
    private static final String TAG_RELATION = "Relation";
    private static final String TAG_HEADER = "Header";
    private static final String TAG_BODY = "Body";
    private static final String ATTRIBUTE_HEADER_KEY = "headerKey";
    private static final String ATTRIBUTE_BODY_KEYS = "bodyKeys";
    private static final String ATTRIBUTE_KEY = "key";
    private static final String ATTRIBUTE_VALUE = "value";
    private static final String ATTRIBUTE_ENTRY_VALUES = "entryValues";

    /**
     * Parse XML resource.
     * @param context Activity context.
     * @param relationXml Resource id of XML.
     * @return The instance of {@link RelationGroup}, return {@code null} if
     *         it is parsed failed.
     */
    public RelationGroup parse(Context context, int relationXml) {
        XmlPullParser parser = context.getResources().getXml(relationXml);
        return inflate(parser);
    }

    /**
     * Parse XML stream.
     * @param inputStream The stream of XML.
     * @return The instance of {@link RelationGroup}, return {@code null} if
     *         it is parsed failed.
     */
    public RelationGroup parse(InputStream inputStream) {
        XmlPullParser parser;
        try {
            parser = XmlPullParserFactory.newInstance().newPullParser();
        } catch (XmlPullParserException e) {
            Log.e(TAG, e.getMessage());
            return null;
        }
        return inflate(parser);
    }

    private RelationGroup inflate(XmlPullParser parser) {
        AttributeSet attrs = Xml.asAttributeSet(parser);
        String nameSpace = parser.getAttributeNamespace(0);
        RelationGroup relationGroup = null;
        Relation.Builder relationBuilder = null;
        try {
            for (int type = parser.next(); type != XmlPullParser.END_DOCUMENT;
                    type = parser.next()) {
                String name = parser.getName();
                switch (name) {
                    case TAG_RELATION_GROUP:
                        if (type == XmlPullParser.START_TAG) {
                            String headerKey =
                                    attrs.getAttributeValue(nameSpace, ATTRIBUTE_HEADER_KEY);
                            String bodyKeys =
                                    attrs.getAttributeValue(nameSpace, ATTRIBUTE_BODY_KEYS);
                            relationGroup = new RelationGroup();
                            relationGroup.setHeaderKey(headerKey);
                            relationGroup.setBodyKeys(bodyKeys);
                        }
                        break;
                    case TAG_RELATION:
                        if (type == XmlPullParser.END_TAG) {
                            relationGroup.addRelation(relationBuilder.build());
                        }
                        break;
                    case TAG_HEADER:
                        if (type == XmlPullParser.START_TAG) {
                            String key =
                                    attrs.getAttributeValue(nameSpace, ATTRIBUTE_KEY);
                            String value =
                                    attrs.getAttributeValue(nameSpace, ATTRIBUTE_VALUE);
                            relationBuilder =
                                    new Relation.Builder(key, value);
                        }
                        break;
                    case TAG_BODY:
                        if (type == XmlPullParser.START_TAG) {
                            String key =
                                    attrs.getAttributeValue(nameSpace, ATTRIBUTE_KEY);
                            String value =
                                    attrs.getAttributeValue(nameSpace, ATTRIBUTE_VALUE);
                            String entryValues =
                                    attrs.getAttributeValue(nameSpace, ATTRIBUTE_ENTRY_VALUES);
                            relationBuilder.addBody(key, value, entryValues);
                        }
                        break;
                    default:
                        throw new InflateException(parser.getPositionDescription());
                }
            }
            return relationGroup;
        } catch (XmlPullParserException e) {
            throw new InflateException(e);
        } catch (IOException e) {
            throw new InflateException(parser.getPositionDescription(), e);
        }
    }
}
