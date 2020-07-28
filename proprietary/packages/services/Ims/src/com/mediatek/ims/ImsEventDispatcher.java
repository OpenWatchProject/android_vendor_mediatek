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

package com.mediatek.ims;

import android.content.Context;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

//import com.mediatek.ims.ImsAdapter;
import com.mediatek.ims.ImsAdapter.VaEvent;
import com.mediatek.ims.ImsAdapter.VaSocketIO;

import com.mediatek.ims.internal.CallControlDispatcher;
import com.mediatek.ims.internal.WfcDispatcher;
import com.mediatek.ims.internal.ImsDataTracker;
import com.mediatek.ims.internal.ImsSimservsDispatcher;

import static com.mediatek.ims.VaConstants.*;

import java.util.ArrayList;

public class ImsEventDispatcher extends Handler {

    private Context mContext;
    private VaSocketIO mSocket;
    private ArrayList<VaEventDispatcher> mVaEventDispatcher = new ArrayList<VaEventDispatcher>();
    private static final String TAG = "[ImsEventDispatcher]";

    public ImsEventDispatcher(Context context, VaSocketIO IO) {
        mContext = context;
        mSocket = IO;

        createDispatcher();
    }

    public interface VaEventDispatcher {
        void vaEventCallback(VaEvent event);
        void enableRequest(int phoneId);
        void disableRequest(int phoneId);
    }

    void enableRequest(int phoneId) {
        for (VaEventDispatcher dispatcher : mVaEventDispatcher) {
            dispatcher.enableRequest(phoneId);
        }
    }

    void disableRequest(int phoneId) {
        for (VaEventDispatcher dispatcher : mVaEventDispatcher) {
            dispatcher.disableRequest(phoneId);
        }
    }

    /* modify the following for domain owners */

    /* Event Dispatcher */
    private CallControlDispatcher mCallControlDispatcher;
    private ImsDataTracker mImsDataTracker;
    private ImsSimservsDispatcher mSimservsDispatcher;
    private WfcDispatcher mWfcDispatcher;

    private void createDispatcher() {

        mCallControlDispatcher = new CallControlDispatcher(mContext, mSocket);
        mVaEventDispatcher.add(mCallControlDispatcher);

        mImsDataTracker = new ImsDataTracker(mContext, mSocket);
        mVaEventDispatcher.add(mImsDataTracker);

        mSimservsDispatcher = new ImsSimservsDispatcher(mContext, mSocket);
        mVaEventDispatcher.add(mSimservsDispatcher);

        mWfcDispatcher = new WfcDispatcher(mContext, mSocket);
        mVaEventDispatcher.add(mWfcDispatcher);
    }

    @Override
    public void handleMessage(Message msg) {
        dispatchCallback((VaEvent) msg.obj);
    }

    /* dispatch Callback */
    void dispatchCallback(VaEvent event) {
        Log.d(TAG, "dispatchCallback: request ID:" + ImsAdapter.requestIdToString(event.getRequestID()));

        switch (event.getRequestID()) {
            case MSG_ID_NOTIFY_XUI_IND:
                mSimservsDispatcher.vaEventCallback(event);
                break;

            case MSG_ID_NOTIFY_SS_PROGRESS_INDICATION :
                mCallControlDispatcher.vaEventCallback(event);
                break;

            case MSG_ID_REQUEST_PCSCF_DISCOVERY:
            case MSG_ID_WRAP_IMSM_IMSPA_PDN_ACT_REQ:
            case MSG_ID_WRAP_IMSM_IMSPA_INFORMATION_REQ:
            case MSG_ID_WRAP_IMSM_IMSPA_PDN_DEACT_REQ:
                mImsDataTracker.vaEventCallback(event);
                break;

            case MSG_ID_IMS_ENABLE_IND:
                enableRequest(event.getPhoneId());
                break;

            case MSG_ID_IMS_DISABLE_IND:
                disableRequest(event.getPhoneId());
                break;

            case MSG_ID_REQUEST_VOWIFI_RELATED_INFO:
                mWfcDispatcher.vaEventCallback(event);
                break;

            default:
                Log.d(TAG, "Receive unsupported Request ID");
                break;
        }
    }
}
