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

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil.Tag;

import java.util.List;
import java.util.concurrent.CopyOnWriteArrayList;

import javax.annotation.Nonnull;
import javax.annotation.Nullable;

/**
 * This class is used to describe the relation between features.
 *
 * <p>This class has inner class {@link Header} and {@link Body}. Head class is used
 * to describe the feature's key and value, assume the feature's key is "A". The Body
 * class is used to describe what the value and entry values of features affected by A
 * feature should be set when A feature value is set as the value in header.</p>
 */
public class Relation {
    private static final Tag TAG = new Tag(Relation.class.getSimpleName());
    public static final String BODY_SPLITTER = ",";
    private Header mHeader;
    private final CopyOnWriteArrayList<Body> mBodyList = new CopyOnWriteArrayList<>();

    /**
     * This class is used to build a relation object.
     */
    public static class Builder {
        private final Relation mRelation;

        /**
         * Builder constructor.
         *
         * @param headerKey The key used to indicator relation.
         * @param headerValue the value of header.
         */
        public Builder(@Nonnull String headerKey, @Nonnull String headerValue) {
            mRelation = new Relation();
            mRelation.createHeader(headerKey, headerValue);
        }

        /**
         * Add a body of relation.
         *
         * @param key The key of body.
         * @param value The value of body.
         * @param entryValues The entry values of body.
         * @return The relation builder object.
         */
        public Builder addBody(String key, String value, String entryValues) {
            mRelation.addBody(key, value, entryValues);
            return this;
        }

        /**
         * Build a relation object.
         *
         * @return The relation object.
         */
        public Relation build() {
            return mRelation.copy();
        }
    }

    /**
     * Head class is used to describe the feature's key and value.
     */
    private class Header {
        public final String key;
        public final String value;

        /**
         * Construct a new header instance.
         *
         * @param key The key of header.
         * @param value The value of header.
         */
        public Header(String key, String value) {
            this.key = key;
            this.value = value;
        }

        /**
         * Copy a new header object.
         *
         * @return A new header object.
         */
        public Header copy() {
            return new Header(key, value);
        }
    }

    /**
     * The Body class is used to describe what the value and entry values of features
     * affected should be set.
     */
    private class Body {
        public final String key;
        public String value;
        public String entryValues;

        /**
         * Construct a new body instance.
         *
         * @param key The key used to indicate body.
         * @param value The value of the body.
         * @param entryValues The entry values of the body.
         */
        public Body(String key, String value, String entryValues) {
            this.key = key;
            this.value = value;
            this.entryValues = entryValues;
        }

        /**
         * Copy a new body object.
         *
         * @return A new body object.
         */
        public Body copy() {
            return new Body(key, value, entryValues);
        }
    }

    private Relation() {
    }

    /**
     * Get the key of header in this relation.
     *
     * @return The key of header in this relation.
     */
    public String getHeaderKey() {
        return mHeader.key;
    }

    /**
     * Get the value of header in this relation.
     *
     * @return The value of header in this relation.
     */
    public String getHeaderValue() {
        return mHeader.value;
    }

    /**
     * Get the list of body keys.
     *
     * @return an list of body keys.
     */
    public List<String> getBodyKeys() {
        List<String> bodyKeys = new CopyOnWriteArrayList<>();
        for (Body body: mBodyList) {
            bodyKeys.add(body.key);
        }
        return bodyKeys;
    }

    /**
     * Get the value of the body indicated by input key.
     *
     * @param bodyKey The key used to indicate body.
     * @return The value of the indicated body.
     */
    public String getBodyValue(String bodyKey) {
        Body body = findBody(bodyKey);
        if (body != null) {
            return body.value;
        }
        return null;
    }

    /**
     * Get the entry values of the body indicated by input key.
     *
     * @param bodyKey The key used to indicate body.
     * @return The entry values of the indicated body.
     */
    @Nullable
    public String getBodyEntryValues(String bodyKey) {
        Body body = findBody(bodyKey);
        if (body != null) {
            return body.entryValues;
        }
        return null;
    }

    /**
     * Print info of header in this relation, for debug.
     *
     * @return The string of header info.
     */
    public String printHeader() {
        return "Header:key=" + mHeader.key + ", value=" + mHeader.value;
    }

    /**
     * Print info of body list in this relation.
     *
     * @return The strings of body list info, for debug.
     */
    public String printBody() {
        StringBuilder builder = new StringBuilder();
        for (int i = 0; i < mBodyList.size(); i++) {
            Body body = mBodyList.get(i);
            builder.append("Body: key=").append(body.key).append(", value=").append(body.value)
                .append(", entryValues=").append(body.entryValues).append("; ");
        }
        return builder.toString();
    }


    /**
     * Copy a new relation object.
     *
     * @return A new relation object.
     */
    public Relation copy() {
        Relation relation = new Relation();
        relation.setHeader(mHeader.copy());
        for (Body body : mBodyList) {
            relation.addBody(body.copy());
        }
        return relation;
    }

    /**
     * Add one body to this relation. If the body already has this body, then update
     * its value and entry values as the input value and entry values.
     *
     * @param key The key of body.
     * @param value The value of body.
     * @param entryValues The entry values of body.
     */
    public void addBody(String key, String value, String entryValues) {
        Body body = findBody(key);
        if (body != null) {
            body.value = value;
            body.entryValues = entryValues;
        } else {
            mBodyList.add(new Body(key, value, entryValues));
        }
    }

    /**
     * Remove the body indicate by the input key from body list.
     *
     * @param key The key of body.
     */
    public void removeBody(String key) {
        Body body = findBody(key);
        if (body != null) {
            mBodyList.remove(body);
        }
    }

    private void createHeader(String key, String value) {
        mHeader =  new Header(key, value);
    }

    private void setHeader(Header header) {
        mHeader = header;
    }

    private Body findBody(String key) {
        for (int i = 0; i < mBodyList.size(); i++) {
            Body body = mBodyList.get(i);
            if (body.key.equals(key)) {
                return body;
            }
        }
        return null;
    }

    private void addBody(Body body) {
        mBodyList.add(body);
    }
}
