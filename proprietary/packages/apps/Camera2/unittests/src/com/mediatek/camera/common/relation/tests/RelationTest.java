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

import junit.framework.Assert;

import org.junit.Test;

import java.util.List;

/**
 * Test relation.
 */
public class RelationTest {
    private static final String HEAD_KEY = "key_test";

    /**
     * Test relation builder can create a correct relation object.
     */
    @Test
    public void testRelationBuilder() {
        String bodyKeys = "key_test1,key_test2,key_test3,key_test4";
        Relation relation = new Relation.Builder(HEAD_KEY, "on")
                .addBody("key_test1", "value_test1", "value_test1, value1_test1")
                .addBody("key_test2", "value_test2", "value_test2, value1_test2")
                .build();

        String queryHeaderKey = relation.getHeaderKey();
        Assert.assertEquals(queryHeaderKey, HEAD_KEY);
        String queryHeaderValue = relation.getHeaderValue();
        Assert.assertEquals(queryHeaderValue, "on");

        List<String> queryBodyKeys = relation.getBodyKeys();
        String queryBodyKeysInStr = queryBodyKeys.get(0);
        for (int i = 1; i < queryBodyKeys.size(); i++) {
            queryBodyKeysInStr = queryBodyKeysInStr + "," + queryBodyKeys.get(i);
        }
        Assert.assertEquals(queryBodyKeysInStr, "key_test1,key_test2");

        String test1Value = relation.getBodyValue("key_test1");
        String test1EntryValues = relation.getBodyEntryValues("key_test1");
        Assert.assertEquals("value_test1", test1Value);
        Assert.assertEquals("value_test1, value1_test1", test1EntryValues);

        String test2Value = relation.getBodyValue("key_test2");
        String test2EntryValues = relation.getBodyEntryValues("key_test2");
        Assert.assertEquals("value_test2", test2Value);
        Assert.assertEquals("value_test2, value1_test2", test2EntryValues);
    }

    /**
     * Test relation copy method.
     */
    @Test
    public void testCopyRelation() {
        Relation relation = new Relation.Builder(HEAD_KEY, "on")
                .addBody("key_test1", "value_test1", "value_test1, value1_test1")
                .addBody("key_test2", "value_test2", "value_test2, value1_test2")
                .build();

        Relation copyRelation = relation.copy();
        Assert.assertEquals(relation.getHeaderKey(), copyRelation.getHeaderKey());
        Assert.assertEquals(relation.getHeaderValue(), copyRelation.getHeaderValue());
        Assert.assertEquals(relation.getBodyKeys(), copyRelation.getBodyKeys());

        Assert.assertEquals(relation.getBodyValue("key_test1"),
                copyRelation.getBodyValue("key_test1"));
        Assert.assertEquals(relation.getBodyEntryValues("key_test1"),
                copyRelation.getBodyEntryValues("key_test1"));

        Assert.assertEquals(relation.getBodyValue("key_test2"),
                copyRelation.getBodyValue("key_test2"));
        Assert.assertEquals(relation.getBodyEntryValues("key_test2"),
                copyRelation.getBodyEntryValues("key_test2"));
    }
}
