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

package com.mediatek.ims;

import android.content.Context;

import android.os.Bundle;
import android.os.Message;
import android.os.RemoteException;
import android.telephony.ims.ImsCallForwardInfo;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.Rlog;

import com.android.ims.ImsUt;
import com.android.ims.ImsException;

import com.android.ims.internal.IImsUt;
import com.android.ims.internal.IImsUtListener;

import com.mediatek.ims.MtkImsCallForwardInfo;

import com.mediatek.ims.internal.IMtkImsUt;
import com.mediatek.ims.internal.IMtkImsUtListener;

import java.util.Arrays;

public class MtkImsUt extends ImsUt {
    private static final String TAG = "MtkImsUt";

    private final IMtkImsUt miMtkUt;

    public MtkImsUt(IImsUt iUt, IMtkImsUt iMtkUt) {
        super(iUt);

        miMtkUt = iMtkUt;

        if (miMtkUt != null) {
            try {
                miMtkUt.setListener(new IMtkImsUtListenerProxy());
            } catch (RemoteException e) {
            }
        }
    }

    public void close() {
        super.close();
    }
    /**
     * A listener type for the result of the supplementary service configuration.
     */
    private class IMtkImsUtListenerProxy extends IMtkImsUtListener.Stub {
        /**
         * Notifies the status of the call forwarding in a time slot supplementary service.
         */
        @Override
        public void utConfigurationCallForwardInTimeSlotQueried(IMtkImsUt iMtkUt,
                int id, MtkImsCallForwardInfo[] cfInfo) {
            Integer key = Integer.valueOf(id);

            synchronized (mLockObj) {
                sendSuccessReport(mPendingCmds.get(key), cfInfo);
                mPendingCmds.remove(key);
            }
        }

        /**
         * Notifies the status of the call forwarding supplementary service.
         */
        @Override
        public void utConfigurationCallForwardQueried(IMtkImsUt iMtkUt,
                int id, ImsCallForwardInfo[] cfInfo) {
            Integer key = Integer.valueOf(id);

            synchronized(mLockObj) {
                sendSuccessReport(mPendingCmds.get(key), cfInfo);
                mPendingCmds.remove(key);
            }
        }
    }

    public String getUtIMPUFromNetwork() throws ImsException {
        if (DBG) {
            log("getUtIMPUFromNetwork :: Ut = " + miMtkUt);
        }

        synchronized (mLockObj) {
            try {
                return miMtkUt.getUtIMPUFromNetwork();
            } catch (RemoteException e) {
                throw new ImsException("getUtIMPUFromNetwork()", e,
                        ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE);
            }
        }
    }

    public void setupXcapUserAgentString(String userAgent) throws ImsException {
        if (DBG) {
            log("setupXcapUserAgentString :: Ut = " + miMtkUt);
        }

        synchronized (mLockObj) {
            try {
                miMtkUt.setupXcapUserAgentString(userAgent);
            } catch (RemoteException e) {
                throw new ImsException("setupXcapUserAgentString()", e,
                        ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE);
            }
        }
    }

    public void queryCallForwardInTimeSlot(int condition, Message result) {
        if (DBG) {
            log("queryCallForwardInTimeSlot :: Ut = " + miMtkUt + ", condition = " + condition);
        }

        synchronized (mLockObj) {
            try {
                int id = miMtkUt.queryCallForwardInTimeSlot(condition);

                if (id < 0) {
                    sendFailureReport(result,
                            new ImsReasonInfo(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE, 0));
                    return;
                }

                mPendingCmds.put(Integer.valueOf(id), result);
            } catch (RemoteException e) {
                sendFailureReport(result,
                        new ImsReasonInfo(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE, 0));
            }
        }
    }

    public void updateCallForwardInTimeSlot(int action, int condition, String number,
            int timeSeconds, long[] timeSlot, Message result) {
        if (DBG) {
            log("updateCallForwardInTimeSlot :: Ut = " + miMtkUt + ", action = " + action
                    + ", condition = " + condition + ", number = " + number
                    + ", timeSeconds = " + timeSeconds
                    + ", timeSlot = " + Arrays.toString(timeSlot));
        }

        synchronized (mLockObj) {
            try {
                int id = miMtkUt.updateCallForwardInTimeSlot(action,
                        condition, number, timeSeconds, timeSlot);

                if (id < 0) {
                    sendFailureReport(result,
                            new ImsReasonInfo(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE, 0));
                    return;
                }

                mPendingCmds.put(Integer.valueOf(id), result);
            } catch (RemoteException e) {
                sendFailureReport(result,
                        new ImsReasonInfo(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE, 0));
            }
        }
    }

    /**
     * Modifies the configuration of the call barring for specified service class.
     */
    public void updateCallBarring(String password, int cbType, int action, Message result,
            String[] barrList, int serviceClass) {
        if (DBG) {
            if (barrList != null) {
                String bList = new String();
                for (int i = 0; i < barrList.length; i++) {
                    bList.concat(barrList[i] + " ");
                }
                log("updateCallBarring :: Ut=" + miMtkUt + ", cbType=" + cbType
                        + ", action=" + action + ", serviceClass=" + serviceClass
                        + ", barrList=" + bList);
            }
            else {
                log("updateCallBarring :: Ut=" + miMtkUt + ", cbType=" + cbType
                        + ", action=" + action + ", serviceClass=" + serviceClass);
            }
        }

        synchronized(mLockObj) {
            try {
                int id = miMtkUt.updateCallBarringForServiceClass(password, cbType, action,
                        barrList, serviceClass);

                if (id < 0) {
                    sendFailureReport(result,
                            new ImsReasonInfo(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE, 0));
                    return;
                }

                mPendingCmds.put(Integer.valueOf(id), result);
            } catch (RemoteException e) {
                sendFailureReport(result,
                        new ImsReasonInfo(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE, 0));
            }
        }
    }

    public String getXcapConflictErrorMessage() throws ImsException {
        if (DBG) {
            log("getXcapConflictErrorMessage :: Ut = " + miMtkUt);
        }

        synchronized (mLockObj) {
            try {
                return miMtkUt.getXcapConflictErrorMessage();
            } catch (RemoteException e) {
                throw new ImsException("getXcapConflictErrorMessage()", e,
                        ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE);
            }
        }
    }

    /**
     * Retrieves the configuration of the call forward.
     * The return value of ((AsyncResult)result.obj) is an array of {@link ImsCallForwardInfo}.
     */
    public void queryCFForServiceClass(int condition, String number, int serviceClass,
                                       Message result) {
        if (DBG) {
            log("queryCFForServiceClass :: condition=" + condition
                    + ", number=" + Rlog.pii(TAG, number) + ", serviceClass = " + serviceClass);
        }

        synchronized(mLockObj) {
            try {
                int id = miMtkUt.queryCFForServiceClass(condition, number, serviceClass);

                if (id < 0) {
                    sendFailureReport(result,
                            new ImsReasonInfo(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE, 0));
                    return;
                }

                mPendingCmds.put(Integer.valueOf(id), result);
            } catch (RemoteException e) {
                sendFailureReport(result,
                        new ImsReasonInfo(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE, 0));
            }
        }
    }

    @Override
    protected void log(String s) {
        Rlog.d(TAG, s);
    }
}
