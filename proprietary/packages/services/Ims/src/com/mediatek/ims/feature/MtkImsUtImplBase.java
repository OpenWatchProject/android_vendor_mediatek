/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.ims.feature;

import android.annotation.SystemApi;
import android.os.Bundle;
import android.os.RemoteException;
import android.os.Messenger;
import android.os.Message;
import android.telephony.ims.ImsUtListener;

import com.mediatek.ims.internal.IMtkImsUt;
import com.mediatek.ims.internal.IMtkImsUtListener;
import com.mediatek.ims.feature.MtkImsUtListener;

/**
 * Base implementation of IMS UT interface, which implements stubs. Override these methods to
 * implement functionality.
 *
 * @hide
 */
// DO NOT remove or change the existing APIs, only add new ones to this Base implementation or you
// will break other implementations of ImsUt maintained by other ImsServices.
@SystemApi
public class MtkImsUtImplBase {

    private IMtkImsUt.Stub mServiceImpl = new IMtkImsUt.Stub() {
        @Override
        public void setListener(IMtkImsUtListener listener) throws RemoteException {
            MtkImsUtImplBase.this.setListener(new MtkImsUtListener(listener));
        }

        @Override
        public String getUtIMPUFromNetwork() {
            return MtkImsUtImplBase.this.getUtIMPUFromNetwork();
        }

        @Override
        public void setupXcapUserAgentString(String userAgent) {
            MtkImsUtImplBase.this.setupXcapUserAgentString(userAgent);
        }

        /// For OP01 UT @{
        @Override
        public int queryCallForwardInTimeSlot(int condition) {

            return MtkImsUtImplBase.this.queryCallForwardInTimeSlot(condition);
        }

        @Override
        public int updateCallForwardInTimeSlot(int action, int condition,
                String number, int timeSeconds, long[] timeSlot) {
            int requestId;

            return MtkImsUtImplBase.this.updateCallForwardInTimeSlot(
                    action, condition, number, timeSeconds, timeSlot);
        }
        /// @}

        @Override
        public int updateCallBarringForServiceClass(String password, int cbType, int enable,
                String[] barrList, int serviceClass) {
            return MtkImsUtImplBase.this.updateCallBarringForServiceClass(password, cbType,
                    enable, barrList, serviceClass);
        }

        @Override
        public void processECT(Message result, Messenger target) {

            MtkImsUtImplBase.this.explicitCallTransfer(result, target);
        }

        @Override
        public boolean isSupportCFT() {
            return MtkImsUtImplBase.this.isSupportCFT();
        }

        @Override
        public String getXcapConflictErrorMessage() {
            return MtkImsUtImplBase.this.getXcapConflictErrorMessage();
        }

        @Override
        public int queryCFForServiceClass(int condition, String number, int serviceClass) {
            return MtkImsUtImplBase.this.queryCFForServiceClass(condition, number, serviceClass);
        }
    };


    /**
     * Updates or retrieves the supplementary service configuration.
     */
    public int transact(Bundle ssInfo) {
        return -1;
    }

    /// For OP01 UT @{
    /**
     * Retrieves the configuration of the call forward in a time slot.
     */
    public int queryCallForwardInTimeSlot(int condition) {
        return -1;
    }

    /**
     * Updates the configuration of the call forward in a time slot.
     */
    public int updateCallForwardInTimeSlot(int action, int condition,
            String number, int timeSeconds, long[] timeSlot) {
        return -1;
    }
    /// @}

    public int updateCallBarringForServiceClass(String password, int cbType, int enable,
            String[] barrList, int serviceClass) {
        return -1;
    }

    /**
     * Do the explicit Call Transfer between FG call and BG call.
     */
    public void explicitCallTransfer(Message result, Messenger target) {

    }
    /// @}

    public boolean isSupportCFT() {
        return false;
    }

    public String getXcapConflictErrorMessage() {
        return "";
    }

    /**
     * Sets the listener.
     */
    public void setListener(MtkImsUtListener listener) {
    }

    /**
     * Get IMPU info from netowrk
     */
    public String getUtIMPUFromNetwork() {
        return "";
    }

    public void setupXcapUserAgentString(String userAgent) {
    }

    /**
     * Retrieves the configuration of the call forward.
     * The return value of ((AsyncResult)result.obj) is an array of {@link ImsCallForwardInfo}.
     */
    public int queryCFForServiceClass(int condition, String number, int serviceClass) {
        return -1;
    }

    /**
     * @hide
     */
    public IMtkImsUt getInterface() {
        return mServiceImpl;
    }
}
