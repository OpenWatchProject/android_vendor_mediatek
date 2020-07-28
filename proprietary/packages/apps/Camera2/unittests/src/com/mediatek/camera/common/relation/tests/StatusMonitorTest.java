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


import com.mediatek.camera.common.relation.StatusMonitor;
import com.mediatek.camera.common.relation.StatusMonitor.StatusChangeListener;
import com.mediatek.camera.common.relation.StatusMonitor.StatusResponder;

import junit.framework.Assert;

import org.junit.Test;

import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.never;
import static org.mockito.Mockito.reset;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

/**
 * Unit test for {@link com.mediatek.camera.common.relation.StatusMonitor}.
 */

public class StatusMonitorTest {

    /**
     * Test register status changed listener.
     */
    @Test
    public void testRegisterStatusChangeListener() {
        StatusMonitor monitor = new StatusMonitor();
        StatusChangeListener listener = new MyStatusChangeListener();
        monitor.registerValueChangedListener("key_test", listener);

        StatusResponder responder = monitor.getStatusResponder("key_test");
        Assert.assertNotNull(responder);
        Assert.assertEquals(responder.getResponderName(), "key_test");
    }

    /**
     * Test unregister status changed listener.
     */
    @Test
    public void testUnRegisterStatusChangeListener() {
        StatusMonitor monitor = new StatusMonitor();
        StatusChangeListener listener = new MyStatusChangeListener();
        monitor.registerValueChangedListener("key_test", listener);
        monitor.unregisterValueChangedListener("key_test", listener);

        StatusResponder responder = monitor.getStatusResponder("key_test");
        Assert.assertNotNull(responder);
    }

    /**
     * Test one listener to listen status changed.
     */
    @Test
    public void testStatusChangeForOneListener() {
        StatusMonitor monitor = new StatusMonitor();
        StatusChangeListener listener = mock(MyStatusChangeListener.class);
        monitor.registerValueChangedListener("key_test", listener);

        StatusResponder responder = monitor.getStatusResponder("key_test");
        responder.statusChanged("key_test", "test_value");
        verify(listener, times(1)).onStatusChanged("key_test", "test_value");

        reset(listener);
        monitor.unregisterValueChangedListener("key_test", listener);
        responder.statusChanged("key_test", "test_value");
        verify(listener, never()).onStatusChanged("key_test", "test_value");
    }

    /**
     * Test multi listeners to listen status changed.
     */
    @Test
    public void testStatusChangeForMultiListeners() {
        StatusMonitor monitor = new StatusMonitor();
        StatusChangeListener listener1 = mock(MyStatusChangeListener.class);
        monitor.registerValueChangedListener("key_test", listener1);
        StatusChangeListener listener2 = mock(MyStatusChangeListener.class);
        monitor.registerValueChangedListener("key_test", listener2);

        StatusResponder responder = monitor.getStatusResponder("key_test");
        responder.statusChanged("key_test", "test_value");
        verify(listener1, times(1)).onStatusChanged("key_test", "test_value");
        verify(listener2, times(1)).onStatusChanged("key_test", "test_value");

        reset(listener1);
        reset(listener2);
        monitor.unregisterValueChangedListener("key_test", listener2);
        responder.statusChanged("key_test", "test_value2");
        verify(listener1, times(1)).onStatusChanged("key_test", "test_value2");
        verify(listener2, never()).onStatusChanged("key_test", "test_value2");
    }

    /**
     * Status change listener implement.
     */
    public class MyStatusChangeListener implements StatusChangeListener {
        @Override
        public void onStatusChanged(String key, String value) {

        }

    }
}
