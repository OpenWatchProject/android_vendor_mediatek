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

package com.mediatek.opcommon.telephony;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.res.Resources;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.HwBinder;
import android.os.Message;
import android.os.Parcel;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.os.Registrant;
import android.os.RegistrantList;
import android.os.RemoteException;
import android.os.SystemProperties;
import android.os.WorkSource;
import android.telephony.Rlog;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.internal.telephony.CommandException;
import com.mediatek.internal.telephony.IMtkRilOp;
import static com.android.internal.telephony.RILConstants.*;
import static com.mediatek.internal.telephony.MtkRILConstants.*;
import static com.mediatek.opcommon.telephony.MtkRILConstantsOp.*;

import vendor.mediatek.hardware.radio_op.V2_0.IRadioOp;
import vendor.mediatek.hardware.radio_op.V2_0.IRadioResponseOp;
import vendor.mediatek.hardware.radio_op.V2_0.IRadioIndicationOp;
import vendor.mediatek.hardware.radio_op.V2_0.RsuRequestInfo;

import com.android.internal.telephony.RIL;
import com.android.internal.telephony.RILConstants;
import com.android.internal.telephony.RILRequest;
import com.mediatek.internal.telephony.MtkRILConstants;
import com.mediatek.opcommon.telephony.MtkRILConstantsOp;

import android.hardware.radio.V1_0.IRadio;
import android.hardware.radio.deprecated.V1_0.IOemHook;

import java.util.ArrayList;
import java.util.Random;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import java.util.concurrent.atomic.AtomicLong;

/**
 * Implement methods to support operator feature
 *
 * @return
 */
public class MtkRilOp extends RIL implements IMtkRilOp {
    static final String TAG = "MtkRilOp";
    static final boolean MTK_RILJOP_LOGD = true;
    static final String[] HIDL_SERVICE_NAME = {"slot1", "slot2", "slot3"};

    //***** Constants
    volatile IRadioOp mRadioProxyOp = null;
    IRadioResponseOp mRadioResponseOp = null;
    IRadioIndicationOp mRadioIndicationOp = null;

    protected final AtomicLong mRadioProxyCookie = new AtomicLong(0);
    protected final RadioOpProxyDeathRecipient mRadioOpProxyDeathRecipient;
    protected final RilHandlerOp mRilHandlerOp;

    protected class RilHandlerOp extends Handler {
        //***** Handler implementation
        @Override public void
        handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_RADIO_PROXY_DEAD:
                    log("handleMessage: EVENT_RADIO_PROXY_DEAD cookie = " + msg.obj +
                            " mRadioProxyCookie = " + mRadioProxyCookie.get());
                    if ((long) msg.obj == mRadioProxyCookie.get()) {
                        resetProxyAndRequestList();

                        // todo: rild should be back up since message was sent with a delay. this is
                        // a hack.
                        getRadioOpProxy(null);
                    }
                    break;
                default:
                    break;
            }
        }
    }

    final class RadioOpProxyDeathRecipient implements HwBinder.DeathRecipient {
        @Override
        public void serviceDied(long cookie) {
            // Deal with service going away
            log("serviceDied");
            // todo: temp hack to send delayed message so that rild is back up by then
            //mRilHandlerOp.sendMessage(mRilHandlerOp.obtainMessage(EVENT_RADIO_PROXY_DEAD, cookie));
            mRilHandlerOp.sendMessageDelayed(
                    mRilHandlerOp.obtainMessage(EVENT_RADIO_PROXY_DEAD, cookie),
                    IRADIO_GET_SERVICE_DELAY_MILLIS);
        }
    }

    public MtkRilOp(Context context, int preferredNetworkType,
            int cdmaSubscription, Integer instanceId) {
        super(context, preferredNetworkType, cdmaSubscription, instanceId);
        log("MtkRilOp constructor ");
        mRadioResponseOp = new MtkRadioResponseOp(this);
        mRadioIndicationOp = new MtkRadioIndicationOp(this);
        mRilHandlerOp = new RilHandlerOp();
        mRadioOpProxyDeathRecipient = new RadioOpProxyDeathRecipient();
        getRadioOpProxy(null);
    }

    public IRadio getRadioProxy(Message result) {
        log("MtkRilOp getRadioProxy");
        return null;
    }

    public IOemHook getOemHookProxy(Message result) {
        log("MtkRilOp getOemHookProxy");
        return null;
    }

    protected IRadioOp getRadioOpProxy(Message result) {
        if (mRadioProxyOp != null) {
            return mRadioProxyOp;
        }

        try {
            mRadioProxyOp = IRadioOp.getService(HIDL_SERVICE_NAME[mPhoneId == null ? 0 : mPhoneId]);
            if (mRadioProxyOp != null) {
                mRadioProxyOp.linkToDeath(mRadioOpProxyDeathRecipient,
                        mRadioProxyCookie.incrementAndGet());
                mRadioProxyOp.setResponseFunctions(mRadioResponseOp, mRadioIndicationOp);
            } else {
                log("getRadioOpProxy: mRadioProxy == null");
            }
        } catch (RemoteException | RuntimeException e) {
            mRadioProxyOp = null;
            log("RadioProxy getService/setResponseFunctions: " + e);
        }

        if (mRadioProxyOp == null) {
            if (result != null) {
                AsyncResult.forMessage(result, null,
                        CommandException.fromRilErrno(RADIO_NOT_AVAILABLE));
                result.sendToTarget();
            }

            // if service is not up, treat it like death notification to try to get service again
            mRilHandlerOp.sendMessageDelayed(
                    mRilHandlerOp.obtainMessage(EVENT_RADIO_PROXY_DEAD, mRadioProxyCookie.get()),
                    IRADIO_GET_SERVICE_DELAY_MILLIS);
        }

        return mRadioProxyOp;
    }

    protected void handleRadioOpProxyExceptionForRR(RILRequest rr, String caller, Exception e) {
        log(caller + ": " + e);
        resetProxyAndRequestList();

        // service most likely died, handle exception like death notification to try to get service
        // again
        mRilHandlerOp.sendMessageDelayed(
                mRilHandlerOp.obtainMessage(EVENT_RADIO_PROXY_DEAD,
                        mRadioProxyCookie.incrementAndGet()),
                IRADIO_GET_SERVICE_DELAY_MILLIS);
    }

    protected void resetProxyAndRequestList() {
        super.resetProxyAndRequestList();
        mRadioProxyOp = null;
    }

    public void log(String text) {
        Rlog.d(TAG, text);
    }

    // MTK-START: SIM TMO RSU
    protected RegistrantList mMelockRegistrants = new RegistrantList();

    public void registerForMelockChanged(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mMelockRegistrants.add(r);
    }

    public void unregisterForMelockChanged(Handler h) {
        mMelockRegistrants.remove(h);
    }
    // MTK-END

    /**
     * Set CS incoming call virtual line
     * RIL_REQUEST_SET_INCOMING_VIRTUAL_LINE
     *
     * @param toMsisdn
     */
    public void setIncomingVirtualLine(String fromMsisdn, String toMsisdn, Message response) {
        IRadioOp radioProxy = getRadioOpProxy(response);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_SET_INCOMING_VIRTUAL_LINE, response,
                    mRILDefaultWorkSource);

            if (MTK_RILJOP_LOGD) {
                riljLog(rr.serialString()
                        + ">  " + requestToString(rr.mRequest) + " fromMsisdn = " + fromMsisdn
                        + " toMsisdn = " + toMsisdn);
            }

            try {
                radioProxy.setIncomingVirtualLine(rr.mSerial, fromMsisdn, toMsisdn);
            } catch (RemoteException | RuntimeException e) {
                handleRadioOpProxyExceptionForRR(rr, "setIncomingVirtualLineResponse", e);
            }
        }
    }


    // MTK_TC1_FEATURE for Antenna Testing start
    public void setRxTestConfig (int AntType, Message result) {
        log("setRxTestConfig");
        IRadioOp radioProxy = getRadioOpProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_VSS_ANTENNA_CONF, result,
                    mRILDefaultWorkSource);

            if (MTK_RILJOP_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
            }

            try {
                radioProxy.setRxTestConfig(rr.mSerial, AntType);
            } catch (RemoteException | RuntimeException e) {
                handleRadioOpProxyExceptionForRR(rr, "setRxTestConfig", e);
            }
        }
    }

    public void getRxTestResult(Message result) {
        log("getRxTestResult");
        IRadioOp radioProxy = getRadioOpProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(RIL_REQUEST_VSS_ANTENNA_INFO, result,
                    mRILDefaultWorkSource);

            if (MTK_RILJOP_LOGD) {
                riljLog(rr.serialString() + "> " + requestToString(rr.mRequest));
            }

            try {
                radioProxy.getRxTestResult(rr.mSerial, 0);
            } catch (RemoteException | RuntimeException e) {
                handleRadioOpProxyExceptionForRR(rr, "getRxTestResult", e);
            }
        }
    }

    protected RegistrantList mModulationRegistrants = new RegistrantList();

    public void registerForModulation(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mModulationRegistrants.add(r);
    }

    public void unregisterForModulation(Handler h) {
        mModulationRegistrants.remove(h);
    }

    public void setDisable2G(boolean mode, Message result)
    {
        log("setDisable2G " + mode);
        IRadioOp radioProxy = getRadioOpProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(MtkRILConstantsOp.RIL_REQUEST_SET_DISABLE_2G, result,
                    mRILDefaultWorkSource);

            if (MTK_RILJOP_LOGD) {
                riljLog(rr.serialString() + "> " + "RIL_REQUEST_SET_DISABLE_2G");
            }

            try {
                radioProxy.setDisable2G(rr.mSerial, mode);
            } catch (RemoteException | RuntimeException e) {
                handleRadioOpProxyExceptionForRR(rr, "setDisable2G", e);
            }
        }
    }

    public void getDisable2G(Message result)
    {
        log("getDisable2G");
        IRadioOp radioProxy = getRadioOpProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(MtkRILConstantsOp.RIL_REQUEST_GET_DISABLE_2G, result,
                    mRILDefaultWorkSource);

            if (MTK_RILJOP_LOGD) {
                riljLog(rr.serialString() + "> " + "RIL_REQUEST_GET_DISABLE_2G");
            }

            try {
                radioProxy.getDisable2G(rr.mSerial);
            } catch (RemoteException | RuntimeException e) {
                handleRadioOpProxyExceptionForRR(rr, "getDisable2G", e);
            }
        }
    }

    public void exitSCBM(Message result)
    {
        log("exitSCBM");
        IRadioOp radioProxy = getRadioOpProxy(result);
        if (radioProxy != null) {
            RILRequest rr = obtainRequest(MtkRILConstantsOp.RIL_REQUEST_EXIT_SCBM, result,
                    mRILDefaultWorkSource);

            if (MTK_RILJOP_LOGD) {
                riljLog(rr.serialString() + "> " + "RIL_REQUEST_EXIT_SCBM");
            }

            try {
                vendor.mediatek.hardware.radio_op.V2_0.IRadioOp radioProxy20 =
                vendor.mediatek.hardware.radio_op.V2_0.IRadioOp.castFrom(radioProxy);
                if (radioProxy20 != null) {
                    radioProxy20.exitSCBM(rr.mSerial);
                }
            } catch (RemoteException | RuntimeException e) {
                handleRadioOpProxyExceptionForRR(rr, "exitSCBM", e);
            }
        }
    }

    public void sendRsuRequest(RsuRequestInfo rri, Message result) {
        IRadioOp radioProxy = getRadioOpProxy(result);
        if (radioProxy != null && rri != null) {
            RILRequest rr = obtainRequest(MtkRILConstantsOp.RIL_REQUEST_SEND_RSU_REQUEST, result,
                    mRILDefaultWorkSource);

            if (MTK_RILJOP_LOGD) {
                riljLog(rr.serialString() + "> " + "RIL_REQUEST_SEND_RSU_REQUEST opId = " + rri.opId
                        + " requestId = " + rri.requestId);
            }

            try {
                radioProxy.sendRsuRequest(rr.mSerial, rri);
            } catch (RemoteException | RuntimeException e) {
                handleRadioOpProxyExceptionForRR(rr, "sendRsuRequest", e);
            }
        }
    }

    public RegistrantList mExitSCBMRegistrants = new RegistrantList();

    public RegistrantList mRsuEventRegistrants = new RegistrantList();

    public void registerForExitSCBM(Handler h, int what, Object obj) {
        Registrant r = new Registrant (h, what, obj);
        mExitSCBMRegistrants.add(r);
    }

    public void registerForRsuEvent(Handler h, int what, Object obj) {
        Registrant r = new Registrant (h, what, obj);
        mRsuEventRegistrants.add(r);
    }

    public void unregisterForExitSCBM(Handler h) {
        mExitSCBMRegistrants.remove(h);
    }

    public void unregisterForRsuEvent(Handler h) {
        mRsuEventRegistrants.remove(h);
    }

    public Registrant mEnterSCBMRegistrant;

    public void setSCBM(Handler h, int what, Object obj) {
        mEnterSCBMRegistrant = new Registrant (h, what, obj);
    }
}

