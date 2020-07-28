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
package com.mediatek.internal.telephony;

import com.android.internal.telephony.GsmCdmaCall;
import com.android.internal.telephony.GsmCdmaCallTracker;
import com.android.internal.telephony.GsmCdmaConnection;
import com.android.internal.telephony.Call.State;

/**
 * MTK add-on extension to GsmCdmaCall.
 */
public class MtkGsmCdmaCall extends GsmCdmaCall {

    public MtkGsmCdmaCall(GsmCdmaCallTracker owner) {
        super(owner);
    }

    @Override
    public boolean isMultiparty() {
        /// M: CC: Remove DISCONNECTED & DIALING conn from counting isMultiparty @{
        //return mConnections.size() > 1;
        int discConn = 0;
        boolean isMptyCall = false;

        for (int j = mConnections.size() - 1 ; j >= 0 ; j--) {
            GsmCdmaConnection cn = (GsmCdmaConnection) (mConnections.get(j));

            if (cn.getState() == GsmCdmaCall.State.DISCONNECTED) {
                discConn++;
            }
        }

        if (mConnections.size() <= 1) {
            isMptyCall = false;
        } else if (mConnections.size() > 1) {
            if ((mConnections.size() - discConn) <= 1) {
                isMptyCall = false;
            } else if (getState() == GsmCdmaCall.State.DIALING) {
                isMptyCall = false;
            } else {
                isMptyCall = true;
            }
        } else {
            isMptyCall = false;
        }

        return isMptyCall;
        /// @}
    }

    @Override
    public void onHangupLocal() {
        for (int i = 0, s = mConnections.size(); i < s; i++) {
            GsmCdmaConnection cn = (GsmCdmaConnection)mConnections.get(i);

            cn.onHangupLocal();
        }
        /// M: CC: Only set state to DISCONNECTING if the call is still alive @{
        //[ALPS01599687]
        //mState = State.DISCONNECTING;
        if ((mConnections.size() != 0) && getState().isAlive()) {
            mState = State.DISCONNECTING;
        }
        /// @}
    }
}
