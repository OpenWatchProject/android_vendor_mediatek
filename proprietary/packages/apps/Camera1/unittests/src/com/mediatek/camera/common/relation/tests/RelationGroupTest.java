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
package com.mediatek.camera.common.relation.tests;

import com.mediatek.camera.common.relation.Relation;
import com.mediatek.camera.common.relation.RelationGroup;

import junit.framework.Assert;

import org.junit.Test;

import java.util.List;

/**
 * Test relation group.
 */
public class RelationGroupTest {
    private static final String HEAD_KEY = "key_test";

    /**
     * Test body keys can be normally parsed and the relation's body keys are
     * equals to the input body keys.
     */
    @Test
    public void testBodyKeys() {
        String bodyKeys = "key_test1,key_test2,key_test3,key_test4";
        Relation relation = new Relation.Builder(HEAD_KEY, "on")
                .addBody("key_test1", "value_test1", "value_test1, value1_test1")
                .addBody("key_test2", "value_test2", "value_test2, value1_test2")
                .build();

        RelationGroup group = new RelationGroup();
        group.setBodyKeys(bodyKeys);
        group.addRelation(relation);
        Relation queryRelation = group.getRelation("on", true);
        List<String> queryBodyKeys = queryRelation.getBodyKeys();
        String queryBodyKeysInStr = queryBodyKeys.get(0);
        for (int i = 1; i < queryBodyKeys.size(); i++) {
            queryBodyKeysInStr = queryBodyKeysInStr + "," + queryBodyKeys.get(i);
        }
        Assert.assertEquals(bodyKeys, queryBodyKeysInStr);
    }

    /**
     * Test body keys which has space can be normally parsed and the relation's body
     * keys are equals to the input body keys.
     */
    @Test
    public void testBodyKeysHasSpace() {
        String bodyKeys = "key_test1,  key_test2, key_test3,    key_test4";
        Relation relation = new Relation.Builder(HEAD_KEY, "on")
                .addBody("key_test1", "value_test1", "value_test1, value1_test1")
                .addBody("key_test2", "value_test2", "value_test2, value1_test2")
                .build();

        RelationGroup group = new RelationGroup();
        group.setBodyKeys(bodyKeys);
        group.addRelation(relation);
        Relation queryRelation = group.getRelation("on", true);
        List<String> queryBodyKeys = queryRelation.getBodyKeys();
        String queryBodyKeysInStr = queryBodyKeys.get(0);
        for (int i = 1; i < queryBodyKeys.size(); i++) {
            queryBodyKeysInStr = queryBodyKeysInStr + "," + queryBodyKeys.get(i);
        }
        Assert.assertEquals(bodyKeys.replaceAll(" ", ""), queryBodyKeysInStr);
    }

    /**
     * Test the relation info is meeting to the input info.
     */
    @Test
    public void testGetRelation() {
        String bodyKeys = "key_test1,key_test2,key_test3,key_test4";
        Relation relation = new Relation.Builder(HEAD_KEY, "on")
                .addBody("key_test1", "value_test1", "value_test1, value1_test1")
                .addBody("key_test2", "value_test2", "value_test2, value1_test2")
                .build();

        RelationGroup group = new RelationGroup();
        group.setBodyKeys(bodyKeys);
        group.addRelation(relation);
        Relation queryRelation = group.getRelation("on", true);

        String headerKey = queryRelation.getHeaderKey();
        Assert.assertEquals(headerKey, HEAD_KEY);

        List<String> queryBodyKeys = queryRelation.getBodyKeys();
        String queryBodyKeysInStr = queryBodyKeys.get(0);
        for (int i = 1; i < queryBodyKeys.size(); i++) {
            queryBodyKeysInStr = queryBodyKeysInStr + "," + queryBodyKeys.get(i);
        }
        Assert.assertEquals(bodyKeys, queryBodyKeysInStr);

        String test1Value = queryRelation.getBodyValue("key_test1");
        String test1EntryValues = queryRelation.getBodyEntryValues("key_test1");
        Assert.assertEquals("value_test1", test1Value);
        Assert.assertEquals("value_test1, value1_test1", test1EntryValues);

        String test2Value = queryRelation.getBodyValue("key_test2");
        String test2EntryValues = queryRelation.getBodyEntryValues("key_test2");
        Assert.assertEquals("value_test2", test2Value);
        Assert.assertEquals("value_test2, value1_test2", test2EntryValues);
    }

    /**
     * Test get an empty relation.
     */
    @Test
    public void testGetEmptyRelation() {
        String bodyKeys = "key_test1,key_test2,key_test3,key_test4";
        Relation relation = new Relation.Builder(HEAD_KEY, "on")
                .addBody("key_test1", "value_test1", "value_test1, value1_test1")
                .addBody("key_test2", "value_test2", "value_test2, value1_test2")
                .build();

        RelationGroup group = new RelationGroup();
        group.setBodyKeys(bodyKeys);
        group.addRelation(relation);
        Relation queryRelation = group.getRelation("off", true);

        String test1Value = queryRelation.getBodyValue("key_test1");
        String test1EntryValues = queryRelation.getBodyEntryValues("key_test1");
        Assert.assertNull(test1Value);
        Assert.assertNull(test1EntryValues);

        String test2Value = queryRelation.getBodyValue("key_test2");
        String test2EntryValues = queryRelation.getBodyEntryValues("key_test2");
        Assert.assertNull(test2Value);
        Assert.assertNull(test2EntryValues);

        String test3Value = queryRelation.getBodyValue("key_test3");
        String test3EntryValues = queryRelation.getBodyEntryValues("key_test3");
        Assert.assertNull(test3Value);
        Assert.assertNull(test3EntryValues);

        String test4Value = queryRelation.getBodyValue("key_test4");
        String test4EntryValues = queryRelation.getBodyEntryValues("key_test4");
        Assert.assertNull(test4Value);
        Assert.assertNull(test4EntryValues);
    }

    /**
     * Test get a null relation.
     */
    @Test
    public void testGetNullRelation() {
        String bodyKeys = "key_test1,key_test2,key_test3,key_test4";
        Relation relation = new Relation.Builder(HEAD_KEY, "on")
                .addBody("key_test1", "value_test1", "value_test1, value1_test1")
                .addBody("key_test2", "value_test2", "value_test2, value1_test2")
                .build();

        RelationGroup group = new RelationGroup();
        group.setBodyKeys(bodyKeys);
        group.addRelation(relation);
        Relation queryRelation = group.getRelation("off", false);
        Assert.assertNull(queryRelation);
    }
}
