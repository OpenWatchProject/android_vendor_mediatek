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

import com.google.common.base.Splitter;
import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;

import java.util.concurrent.CopyOnWriteArrayList;

/**
 * Relation group is used to contain all the relations which are defined
 * between the header that indicated by the "headerKey" attribute and the
 * multi-body that indicated by the "bodyKeys"
 *
 * "HeaderKey" indicator the setting which will going to be actively changed
 * by user click. "bodyKeys" indicator the settings which will be affected when
 * the value of setting indicated by "HeaderKey" changed.
 *
 * If one setting' value changed will affect other settings, this setting will has
 * a relation group to hold the restriction info with other settings.
 */

public class RelationGroup {
    private static final Tag TAG = new Tag(RelationGroup.class.getSimpleName());
    private String mHeaderKey;
    private final CopyOnWriteArrayList<String> mBodyKeys = new CopyOnWriteArrayList<>();
    private final CopyOnWriteArrayList<Relation> mRelationList = new CopyOnWriteArrayList<>();

    /**
     * RelationGroup constructor.
     */
    public RelationGroup() {
    }

    /**
     * Set the key of setting which will going to be actively changed by user click
     * as the header key of relation group. This is also the key of header part in
     * relation.
     * @param headerKey The strings to indicate the header.
     */
    public void setHeaderKey(String headerKey) {
        mHeaderKey = headerKey;
    }

    /**
     * Set the keys of settings which will be affected by the setting indicated by
     * "HeaderKey" as the body keys of relation group.
     *
     * @param bodyKeys The strings to indicate the keys of affected settings, one key
     *                 is corresponding to a setting. every key must be separated by
     *                 comma. Like "key1,key2,key3".
     */
    public void setBodyKeys(String bodyKeys) {
        if (bodyKeys == null) {
            LogHelper.e(TAG, "[setBodyKeys] with Null bodyKeys!!!!!!");
            return;
        }
        Iterable<String> bodyIterable =
                Splitter.on(Relation.BODY_SPLITTER)
                        .trimResults()
                        .omitEmptyStrings()
                        .split(bodyKeys);
        for (String bodyKey: bodyIterable) {
            mBodyKeys.add(bodyKey);
        }
    }

    /**
     * Add a relation into this relation group.
     *
     * @param relation The special relation.
     */
     public void addRelation(Relation relation) {
         mRelationList.add(relation);
     }

    /**
     * Get a relation from the relation group according to the input header value.
     *
     * @param headerValue The value of setting used to find setting's restriction when
     *                    setting is on the input header value.
     * @param defaultEmpty If a relation can be find in the relation group according to
     *                     the input head value, this parameter has no effect, otherwise,
     *                     this method will return an empty relation object when it is
     *                     true, and this method will return null object when it is false.
     * @return A relation object or null object.
     */
    public Relation getRelation(String headerValue, boolean defaultEmpty) {
        Relation find = null;
        for (Relation relation : mRelationList) {
            if (relation.getHeaderValue().equals(headerValue)) {
                find = buildNewRelation(relation);
                break;
            }
        }

        if (find == null && defaultEmpty) {
            find = buildEmptyRelation(mHeaderKey, headerValue);
        }
        return find;
    }

    private Relation buildNewRelation(Relation relation) {
        Relation.Builder builder = new Relation.Builder(relation.getHeaderKey(),
                                                    relation.getHeaderValue());
        for (String bodyKey : mBodyKeys) {
            builder.addBody(bodyKey, relation.getBodyValue(bodyKey),
                    relation.getBodyEntryValues(bodyKey));
        }
        return builder.build();
    }

    private Relation buildEmptyRelation(String headerKey, String headerValue) {
        Relation.Builder builder = new Relation.Builder(headerKey, headerValue);
        for (String bodyKey : mBodyKeys) {
            builder.addBody(bodyKey, null, null);
        }
        return builder.build();
    }
}
