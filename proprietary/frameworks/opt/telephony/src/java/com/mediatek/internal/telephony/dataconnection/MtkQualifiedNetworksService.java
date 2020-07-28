/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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


package com.mediatek.internal.telephony.dataconnection;

import android.os.AsyncResult;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.telephony.AccessNetworkConstants.AccessNetworkType;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.telephony.data.QualifiedNetworksService;
import android.telephony.data.IQualifiedNetworksServiceCallback;
import android.telephony.data.ApnSetting.ApnType;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.mediatek.internal.telephony.MtkRIL;

import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;

public class MtkQualifiedNetworksService extends QualifiedNetworksService {
    private static final String TAG = MtkQualifiedNetworksService.class.getSimpleName();

    private static final int UPDATE_QUALIFIED_NETWORKS = 1;

    /**
     * The abstract class of the network availability updater implementation. The vendor qualified
     * network service must extend this class to report the available networks for data
     * connection setup. Note that each instance of network availability updater is associated with
     * one physical SIM slot.
     */
    public class MtkNetworkAvailabilityProvider
            extends QualifiedNetworksService.NetworkAvailabilityProvider {
        private static final boolean DBG = true;

        private final Phone mPhone;
        private final Looper mLooper;
        private final Handler mHandler;
        private final HandlerThread mHandlerThread;

        private static final int MD_NW_TYPE_UNKNOWN  = 0;
        private static final int MD_NW_TYPE_CELLULAR = 1;
        private static final int MD_NW_TYPE_IWLAN    = 2;

        /**
         * Constructor
         */
        public MtkNetworkAvailabilityProvider(int slotIndex) {
           super(slotIndex);

            mPhone = PhoneFactory.getPhone(getSlotIndex());

            mHandlerThread = new HandlerThread(
                    MtkQualifiedNetworksService.class.getSimpleName());
            mHandlerThread.start();
            mLooper = mHandlerThread.getLooper();
            mHandler = new Handler(mLooper) {
                @Override
                public void handleMessage(Message message) {

                    AsyncResult ar = (AsyncResult) message.obj;
                    switch (message.what) {
                        case UPDATE_QUALIFIED_NETWORKS:
                            // availabilityUpdate
                            // int[0]   is the mode
                            // int[1]   is apnTypes
                            // int[2~3] is qualifiedNetworkTypes

                            int idx = 2;
                            int[] availabilityUpdate = (int[])ar.result;
                            int length = availabilityUpdate.length;
                            List<Integer> qualifiedNetworkTypes = new ArrayList<>();
                            int mode = availabilityUpdate[0];
                            int apnTypes = availabilityUpdate[1];

                            log("UPDATE_QUALIFIED_NETWORKS mode=" + mode + " apnTypes=" +
                                    apnTypes);
                            for(int i = idx; i < length; i++) {
                                qualifiedNetworkTypes.add(
                                        converNetworkType(availabilityUpdate[i]));
                                log("availabilityUpdate[" + i + "]=" +
                                        converNetworkType(availabilityUpdate[i]));
                            }

                            updateQualifiedNetworkTypes(apnTypes, qualifiedNetworkTypes);
                            break;
                        default:
                            loge("Unexpected event: " + message.what);
                            return;
                    }
                }
            };

            if (DBG) log("Register for qyalified networks changed.");
            ((MtkRIL)mPhone.mCi).registerForQualifiedNetworkTypesChanged(
                    mHandler, UPDATE_QUALIFIED_NETWORKS, null);
        }

        /**
         * Called when the qualified networks updater is removed. The extended class should
         * implement this method to perform cleanup works.
         */
        @Override
        public void close() {
            ((MtkRIL)mPhone.mCi).unregisterForQualifiedNetworkTypesChanged(mHandler);
            mHandlerThread.quit();
        }

        public int converNetworkType(int mdReportType){
            int type = AccessNetworkType.UNKNOWN;
            switch (mdReportType) {
                case MD_NW_TYPE_CELLULAR:
                    type = AccessNetworkType.EUTRAN;
                    break;
                case MD_NW_TYPE_IWLAN:
                    type = AccessNetworkType.IWLAN;
                    break;
                default:
                    type = AccessNetworkType.UNKNOWN;
                    break;
            }
            return type;
        }
    }

    @Override
    public NetworkAvailabilityProvider onCreateNetworkAvailabilityProvider(int slotIndex) {
        log("MtkQNS create MtkNetworkAvailabilityProvider for slot " + slotIndex);
        if (!SubscriptionManager.isValidSlotIndex(slotIndex)) {
            loge("Tried to cellular data service with invalid slotId " + slotIndex);
            return null;
        }
        return new MtkNetworkAvailabilityProvider(slotIndex);
    }

    private void log(String s) {
        Rlog.d(TAG, s);
    }

    private void loge(String s) {
        Rlog.e(TAG, s);
    }
}
