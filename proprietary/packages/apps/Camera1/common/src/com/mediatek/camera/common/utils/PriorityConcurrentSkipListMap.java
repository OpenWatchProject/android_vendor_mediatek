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
package com.mediatek.camera.common.utils;

import com.google.common.base.Splitter;

import java.util.Comparator;
import java.util.concurrent.ConcurrentSkipListMap;

import javax.annotation.Nonnull;
import javax.annotation.Nullable;

/**
 * This class is extended from ConcurrentSkipListMap, with priority incremented.
 *
 * @param <K> the type of keys maintained by this map, must be String.
 * @param <V> the type of mapped values.
 */
public class PriorityConcurrentSkipListMap<K, V> extends ConcurrentSkipListMap<String, V> {
    private static final String PRIORITY_SPLITTER = "-";

    /**
     * Constructs a new with self-defined {@link Comparator} to compare priority.
     * priority is sorted incremented.
     *
     * @param increasing whether sort increasing.
     */
    public PriorityConcurrentSkipListMap(final boolean increasing) {
        super(new Comparator<String>() {
            @Override
            public int compare(String s1, String s2) {
                int priority1 = Integer.parseInt(
                        Splitter.on(PRIORITY_SPLITTER).trimResults().splitToList(s1).get(0));
                int hashCode1 = Integer.parseInt(
                        Splitter.on(PRIORITY_SPLITTER).trimResults().splitToList(s1).get(1));
                int priority2 = Integer.parseInt(
                        Splitter.on(PRIORITY_SPLITTER).trimResults().splitToList(s2).get(0));
                int hashCode2 = Integer.parseInt(
                        Splitter.on(PRIORITY_SPLITTER).trimResults().splitToList(s2).get(1));
                if (priority1 == priority2) {
                    return hashCode1 - hashCode2;
                }
                if (increasing) {
                    return priority1 > priority2 ? 1 : -1;
                }
                return priority1 > priority2 ? -1 : 1;
            }
        });
    }

    /**
     * Find the key by value.
     *
     * @param value the value want to find key.
     * @return key of this value, or NULL if not contains this value.
     */
    @Nullable
    public String findKey(V value) {
        if (containsValue(value)) {
            for (Entry<String, V> entry : entrySet()) {
                if (value.hashCode() == entry.getValue().hashCode()) {
                    return entry.getKey();
                }
            }
        }
        return null;
    }

    /**
     * Compose a new string key with priority and object's hashcode.
     *
     * @param priority the priority key.
     * @param object the object value.
     * @return an String key which compose priority and object's hashcode.
     */
    @Nonnull
    public static String getPriorityKey(int priority, @Nonnull Object object) {
        return priority + PRIORITY_SPLITTER + object.hashCode();
    }

    /**
     *Get the priority value by the given key string.
     * @param key The key string.
     * @return Priority value.
     */
    @Nonnull
    public static int getPriorityByKey(String key) {
        return Integer.parseInt(
                Splitter.on(PRIORITY_SPLITTER).trimResults().splitToList(key).get(0));
    }
}