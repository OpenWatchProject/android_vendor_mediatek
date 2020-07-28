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
package com.mediatek.camera.common.utils.tests;

import android.view.KeyEvent;

import com.mediatek.camera.common.app.IApp.KeyEventListener;
import com.mediatek.camera.common.utils.PriorityConcurrentSkipListMap;

import junit.framework.Assert;

import org.junit.Test;

/**
 * Unit test for {@link PriorityConcurrentSkipListMap}.
 */
public class PriorityConcurrentSkipListMapTest {

    /**
     * Test put with different priority, it will sort automatically.
     */
    @Test
    public void testPutWithDifferentPriority() {
        PriorityConcurrentSkipListMap<String, Object> map = new PriorityConcurrentSkipListMap(true);
        Object o1 = new Object();
        Object o3 = new Object();
        Object o5 = new Object();
        map.put(map.getPriorityKey(5, o5), o5);
        map.put(map.getPriorityKey(1, o1), o1);
        map.put(map.getPriorityKey(3, o3), o3);
        Assert.assertEquals(o1, map.firstEntry().getValue());
        Assert.assertEquals(o5, map.lastEntry().getValue());
    }

    /**
     * Test put with same priority, same priority can be put successfully.
     */
    @Test
    public void testPutWithSamePriority() {
        PriorityConcurrentSkipListMap<String, Object> map = new PriorityConcurrentSkipListMap(true);
        Object o1 = new Object();
        Object o3 = new Object();
        Object o33 = new Object();
        Object o5 = new Object();
        map.put(map.getPriorityKey(5, o5), o5);
        map.put(map.getPriorityKey(1, o1), o1);
        map.put(map.getPriorityKey(3, o3), o3);
        map.put(map.getPriorityKey(3, o33), o33);
        Assert.assertEquals(o1, map.firstEntry().getValue());
        Assert.assertEquals(o5, map.lastEntry().getValue());
        Assert.assertEquals(4, map.size());
    }

    /**
     * Test put with same priority, same priority can be put successfully.
     * Specially, we create an instance of {@link KeyEventListener} as value.
     */
    @Test
    public void testPutWithSamePriorityByKeyEventListener() {
        PriorityConcurrentSkipListMap<String, KeyEventListener> map =
                new PriorityConcurrentSkipListMap(true);
        KeyEventListener o1 = new KeyEventListenerObject();
        KeyEventListener o3 = new KeyEventListenerObject();
        KeyEventListener o33 = new KeyEventListenerObject();
        KeyEventListener o5 = new KeyEventListenerObject();
        map.put(map.getPriorityKey(5, o5), o5);
        map.put(map.getPriorityKey(1, o1), o1);
        map.put(map.getPriorityKey(3, o3), o3);
        map.put(map.getPriorityKey(3, o33), o33);
        Assert.assertEquals(o1, map.firstEntry().getValue());
        Assert.assertEquals(o5, map.lastEntry().getValue());
        Assert.assertEquals(4, map.size());
    }

    /**
     * Test with same key, same key will be replaced.
     */
    @Test
    public void testPutWithSameKey() {
        PriorityConcurrentSkipListMap<String, Object> map = new PriorityConcurrentSkipListMap(true);
        Object o1 = new Object();
        Object o3 = new Object();
        Object o33 = new Object();
        Object o5 = new Object();
        map.put(map.getPriorityKey(5, o5), o5);
        map.put(map.getPriorityKey(1, o1), o1);
        map.put(map.getPriorityKey(3, o3), o3);
        map.put(map.getPriorityKey(3, o3), o33);
        Assert.assertEquals(o1, map.firstEntry().getValue());
        Assert.assertEquals(o5, map.lastEntry().getValue());
        Assert.assertEquals(3, map.size());
    }

    /**
     * Test remove by value, after remove the map is also sorted.
     */
    @Test
    public void testRemove() {
        PriorityConcurrentSkipListMap<String, Object> map = new PriorityConcurrentSkipListMap(true);
        Object o1 = new Object();
        Object o3 = new Object();
        Object o5 = new Object();
        map.put(map.getPriorityKey(5, o5), o5);
        map.put(map.getPriorityKey(1, o1), o1);
        map.put(map.getPriorityKey(3, o3), o3);
        map.remove(map.findKey(o1));
        Assert.assertEquals(o3, map.firstEntry().getValue());
        Assert.assertEquals(o5, map.lastEntry().getValue());
        Assert.assertEquals(2, map.size());
    }

    /**
     * An implementation of KeyEventListener used to create test value.
     */
    class KeyEventListenerObject implements KeyEventListener {
        @Override
        public boolean onKeyDown(int keyCode, KeyEvent event) {
            return false;
        }
        @Override
        public boolean onKeyUp(int keyCode, KeyEvent event) {
            return false;
        }
    }
}