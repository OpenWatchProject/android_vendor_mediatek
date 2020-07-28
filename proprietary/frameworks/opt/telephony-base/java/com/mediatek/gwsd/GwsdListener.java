/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/
/* MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.gwsd;

import android.telephony.Rlog;

import java.lang.ref.WeakReference;

public class GwsdListener {

     /**
     * Callback function to be invoked to inform the client
     * @param status: 0 is SUCCESS, -1 is FAIL
     * @param reason: response of the request placed.
     */
    public void onAddListenered(int status, String reason) {
        // default implementation empty
    }

    /**
     * Callback function to be invoked to inform the client when user selection mode configure finish
     * @param status: 0 is SUCCESS, -1 is FAIL
     * @param reason: response of the request placed.
     */
    public void onUserSelectionModeChanged(int status, String reason) {
        // default implementation empty
    }

    /**
     * Callback function to be invoked to inform the client when auto reject mode configure finish
     * @param status: 0 is SUCCESS, -1 is FAIL
     * @param reason: response of the request placed.
     */
    public void onAutoRejectModeChanged(int status, String reason) {
         // default implementation empty
    }

    /**
     * Callback function to be invoked to inform the client when call valid timer be changed
     * @param status: 0 is SUCCESS, -1 is FAIL
     * @param reason: response of the request placed.
     */
    public void onCallValidTimerChanged(int status, String reason) {
        // default implementation empty
    }

    /**
     * Callback function to be invoked to inform the client when ignore same call timer be changed
     * @param status: 0 is SUCCESS, -1 is FAIL
     * @param reason: response of the request placed.
     */
    public void onIgnoreSameNumberIntervalChanged(int status, String reason) {
        // default implementation empty
    }

    /**
     * Callback function to be invoked to inform the client when sync info to service finish
     * @param status: 0 is SUCCESS, -1 is FAIL
     * @param reason: response of the request placed.
     */
    public void onSyncGwsdInfoFinished(int status, String reason) {
         // default implementation empty
    }

    /**
     * Callback function to be invoked to inform the client when system state changed
     * @param state: state of system
     * SYSTEM_STATE_MODEM_RESET
     * SYSTEM_STATE_DEFAULT_DATA_SWITCHED
     */
    public void onSystemStateChanged(int state) {
        // default implementation empty
    }

    private static class IGwsdListenerStub extends IGwsdListener.Stub {
        private String TAG = "IGwsdListenerStub";
        private WeakReference<GwsdListener> mGwsdListenerWeakRef;

        public IGwsdListenerStub(GwsdListener gwsdListener) {
            mGwsdListenerWeakRef = new WeakReference<GwsdListener>(gwsdListener);
        }

        public void onAddListenered(int status, String reason) {
            Rlog.d(TAG, "onAddListenered, status: " + status + " reason: " + reason);
            GwsdListener listener = mGwsdListenerWeakRef.get();
            if (listener != null) {
                listener.onAddListenered(status, reason);
            }
        }

        public void onUserSelectionModeChanged(int status, String reason) {
            Rlog.d(TAG, "onUserSelectionModeChanged, status: " + status + " reason: " + reason);
            GwsdListener listener = mGwsdListenerWeakRef.get();
            if (listener != null) {
                listener.onUserSelectionModeChanged(status, reason);
            }
        }

        public void onAutoRejectModeChanged(int status, String reason) {
            Rlog.d(TAG, "onAutoRejectModeChanged, status: " + status + " reason: " + reason);
            GwsdListener listener = mGwsdListenerWeakRef.get();
            if (listener != null) {
                listener.onAutoRejectModeChanged(status, reason);
            }
        }

        public void onSyncGwsdInfoFinished(int status, String reason) {
            Rlog.d(TAG, "onSyncGwsdInfoFinished, status: " + status + " reason: " + reason);
            GwsdListener listener = mGwsdListenerWeakRef.get();
            if (listener != null) {
                listener.onSyncGwsdInfoFinished(status, reason);
            }
        }

        public void onSystemStateChanged(int state) {
            Rlog.d(TAG, "onSystemStateChanged, state: " + state);
            GwsdListener listener = mGwsdListenerWeakRef.get();
            if (listener != null) {
                listener.onSystemStateChanged(state);
            }
        }

        public void onCallValidTimerChanged(int status, String reason) {
            Rlog.d(TAG, "onCallValidTimerChanged, status: " + status + " reason: " + reason);
            GwsdListener listener = mGwsdListenerWeakRef.get();
            if (listener != null) {
                listener.onCallValidTimerChanged(status, reason);
            }
        }

        public void onIgnoreSameNumberIntervalChanged(int status, String reason) {
            Rlog.d(TAG, "onIgnoreSameNumberIntervalChanged, status: " + status + " reason: " + reason);
            GwsdListener listener = mGwsdListenerWeakRef.get();
            if (listener != null) {
                listener.onIgnoreSameNumberIntervalChanged(status, reason);
            }
        }
    }

    public IGwsdListener callback = new IGwsdListenerStub(this);
}