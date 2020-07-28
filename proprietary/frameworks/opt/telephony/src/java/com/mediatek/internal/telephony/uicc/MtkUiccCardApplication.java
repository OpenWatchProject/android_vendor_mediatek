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

package com.mediatek.internal.telephony.uicc;

import com.android.internal.telephony.uicc.UiccCard;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.telephony.uicc.IccCardApplicationStatus;
import com.android.internal.telephony.uicc.UiccCardApplication;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.uicc.IccCardApplicationStatus.AppState;
import com.android.internal.telephony.uicc.IccCardApplicationStatus.AppType;
import com.android.internal.telephony.uicc.IccCardApplicationStatus.PersoSubState;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.telephony.uicc.RuimRecords;
import com.android.internal.telephony.uicc.IsimUiccRecords;
import com.android.internal.telephony.uicc.SIMFileHandler;
import com.android.internal.telephony.uicc.RuimFileHandler;
import com.android.internal.telephony.uicc.CsimFileHandler;
import com.android.internal.telephony.uicc.UiccProfile;
import com.android.internal.telephony.uicc.UsimFileHandler;
import com.android.internal.telephony.uicc.IsimFileHandler;
import com.android.internal.telephony.RILConstants;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.CommandException;
import com.mediatek.internal.telephony.uicc.IccServiceInfo;
import com.mediatek.internal.telephony.MtkRIL;

import android.os.SystemProperties;
import android.telephony.Rlog;
import android.content.Context;
import android.os.Registrant;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.RegistrantList;

/**
 * {@hide}
 */
public class MtkUiccCardApplication extends UiccCardApplication {
    private static final String LOG_TAG_EX = "MtkUiccCardApp";
    private static final boolean DBG = true;

    // MTK-START: SIM ME LOCK
    public static final int CAT_NETWOEK                = 0;
    public static final int CAT_NETOWRK_SUBSET         = 1;
    public static final int CAT_SERVICE_PROVIDER       = 2;
    public static final int CAT_CORPORATE              = 3;
    public static final int CAT_SIM                    = 4;
    public static final int CAT_NS_SP                  = 5;
    public static final int CAT_SIM_C                  = 6;

    public static final int OP_UNLOCK                  = 0;
    public static final int OP_LOCK                    = 1;
    public static final int OP_ADD                     = 2;
    public static final int OP_REMOVE                  = 3;
    public static final int OP_PERMANENT_UNLOCK        = 4;
    private static final int EVENT_QUERY_NETWORK_LOCK_DONE = 101;
    private static final int EVENT_CHANGE_NETWORK_LOCK_DONE = 102;
    // MTK-END

    private static final int EVENT_RADIO_NOTAVAILABLE = 103;
    // [ALPS01827777]--- START ---
    private static final int EVENT_PUK1_CHANGE_PIN1_DONE = 104;
    private static final int EVENT_PUK2_CHANGE_PIN2_DONE = 105;
    // [ALPS01827777]--- END ---

    // [02772] start
    static final String[] UICCCARDAPPLICATION_PROPERTY_RIL_UICC_TYPE = {
        "vendor.gsm.ril.uicctype",
        "vendor.gsm.ril.uicctype.2",
        "vendor.gsm.ril.uicctype.3",
        "vendor.gsm.ril.uicctype.4",
    };
    protected String mIccType = null; /* Add for USIM detect */
    // [02772] end

    private static final String PROPERTY_PIN1_RETRY[] = {
        "vendor.gsm.sim.retry.pin1",
        "vendor.gsm.sim.retry.pin1.2",
        "vendor.gsm.sim.retry.pin1.3",
        "vendor.gsm.sim.retry.pin1.4",
    };

    private static final String PROPERTY_PIN2_RETRY[] = {
        "vendor.gsm.sim.retry.pin2",
        "vendor.gsm.sim.retry.pin2.2",
        "vendor.gsm.sim.retry.pin2.3",
        "vendor.gsm.sim.retry.pin2.4",
    };
    protected int mPhoneId;

    public MtkUiccCardApplication(UiccProfile uiccProfile,
                        IccCardApplicationStatus as,
                        Context c,
                        CommandsInterface ci) {
        super(uiccProfile, as, c, ci);

        // static method getAuthContext can not be overrided.
        // MTK-START
        mAuthContext = getAuthContextEx(mAppType);
        mPhoneId = getPhoneId();
        // MTK-END
    }

    @Override
    public void update (IccCardApplicationStatus as, Context c, CommandsInterface ci) {
        synchronized (mLock) {
            if (mDestroyed) {
                mtkLoge("Application updated after destroyed! Fix me!");
                return;
            }

            if (DBG) mtkLog(mAppType + " update. New " + as);
            mContext = c;
            mCi = ci;
            AppType oldAppType = mAppType;
            AppState oldAppState = mAppState;
            PersoSubState oldPersoSubState = mPersoSubState;
            mAppType = as.app_type;
            // MTK-START
            mAuthContext = getAuthContextEx(mAppType);
            // MTK-END
            mAppState = as.app_state;
            mPersoSubState = as.perso_substate;
            mAid = as.aid;
            mAppLabel = as.app_label;
            mPin1Replaced = (as.pin1_replaced != 0);
            mPin1State = as.pin1;
            mPin2State = as.pin2;

            if (mAppType != oldAppType) {
                if (mIccFh != null) { mIccFh.dispose();}
                if (mIccRecords != null) { mIccRecords.dispose();}
                mIccFh = createIccFileHandler(as.app_type);
                mIccRecords = createIccRecords(as.app_type, c, ci);
            }

            // MTK-START: SIM ME LOCK
            //MTK platform will set network locked for all of subState
            if (DBG) mtkLog("mPersoSubState: " + mPersoSubState +
                    " oldPersoSubState: " + oldPersoSubState);
            if (mPersoSubState != oldPersoSubState /*&&
                    mPersoSubState == PersoSubState.PERSOSUBSTATE_SIM_NETWORK*/) {
            // MTK-END
                notifyNetworkLockedRegistrantsIfNeeded(null);
            }

            // MTK-START
            if (DBG) mtkLog("update,  mAppState=" + mAppState + "  oldAppState=" + oldAppState);
            // MTK-END
            if (mAppState != oldAppState) {
                if (DBG) mtkLog(oldAppType + " changed state: " + oldAppState + " -> " + mAppState);
                // If the app state turns to APPSTATE_READY, then query FDN status,
                //as it might have failed in earlier attempt.
                // MTK-START
                //if (mAppState == AppState.APPSTATE_READY) {
                if (mAppState == AppState.APPSTATE_READY && mAppType != AppType.APPTYPE_ISIM) {
                // MTK-END
                    queryFdn();
                    queryPin1State();
                }
                notifyPinLockedRegistrantsIfNeeded(null);
                notifyReadyRegistrantsIfNeeded(null);
            // MTK-START
            // For CDMA 3G dual mode card, card type could be switched into SIM or RUIM. UiccCard
            // and UiccCardApplication will not be disposed. SIM and RUIM have same index and keep
            // in ready state. It should udpate FDN and Pin1 state after cardtype is switched.
            } else if (mAppState == AppState.APPSTATE_READY) {
                if (((mAppType == AppType.APPTYPE_SIM) && (oldAppType == AppType.APPTYPE_RUIM)) ||
                        ((mAppType == AppType.APPTYPE_RUIM) &&
                        (oldAppType == AppType.APPTYPE_SIM))) {
                    queryFdn();
                    queryPin1State();
                }
            }
            // MTK-END
        }
    }

    protected void notifyNetworkLockedRegistrantsIfNeeded(Registrant r) {
        if (mDestroyed) {
            return;
        }

        // MTK-START: SIM ME LOCK
        if (mAppState == AppState.APPSTATE_SUBSCRIPTION_PERSO /*&&
                mPersoSubState == PersoSubState.PERSOSUBSTATE_SIM_NETWORK*/) {
        // MTK-END
            if (r == null) {
                if (DBG) mtkLog("Notifying registrants: NETWORK_LOCKED");
                mNetworkLockedRegistrants.notifyRegistrants();
            } else {
                if (DBG) mtkLog("Notifying 1 registrant: NETWORK_LOCED");
                r.notifyRegistrant(new AsyncResult(null, null, null));
            }
        }
    }

    @Override
    protected IccRecords createIccRecords(AppType type, Context c, CommandsInterface ci) {
        if (DBG) mtkLog("UiccCardAppEx createIccRecords, AppType = " + type);

        if (type == AppType.APPTYPE_USIM || type == AppType.APPTYPE_SIM) {
            return new MtkSIMRecords(this, c, ci);
        } else if (type == AppType.APPTYPE_RUIM || type == AppType.APPTYPE_CSIM){
            return new MtkRuimRecords(this, c, ci);
        } else if (type == AppType.APPTYPE_ISIM) {
            // MTK-START: ISIM
            return new MtkIsimUiccRecords(this, c, ci);
            // MTK-END
        } else {
            // Unknown app type (maybe detection is still in progress)
            return null;
        }
    }

    @Override
    protected IccFileHandler createIccFileHandler(AppType type) {
        switch (type) {
            case APPTYPE_SIM:
                return new MtkSIMFileHandler(this, mAid, mCi);
            case APPTYPE_RUIM:
                return new MtkRuimFileHandler(this, mAid, mCi);
            case APPTYPE_USIM:
                return new MtkUsimFileHandler(this, mAid, mCi);
            case APPTYPE_CSIM:
                return new MtkCsimFileHandler(this, mAid, mCi);
            case APPTYPE_ISIM:
                return new MtkIsimFileHandler(this, mAid, mCi);
            default:
                return null;
        }
    }

    @Override
    protected void onChangeFdnDone(AsyncResult ar) {
        super.onChangeFdnDone(ar);
        if (ar.exception == null) {
            mtkLog("notifyFdnChangedRegistrants");
            notifyFdnChangedRegistrants();
        }
    }

    private Handler mHandlerEx = new Handler() {
        @Override
        public void handleMessage(Message msg){
            AsyncResult ar;

            if (mDestroyed) {
                // MTK-START
                if (EVENT_PIN1_PUK1_DONE == msg.what ||
                        EVENT_QUERY_NETWORK_LOCK_DONE == msg.what) {
                    ar = (AsyncResult) msg.obj;
                    Message response = (Message) ar.userObj;
                    AsyncResult.forMessage(response).exception = CommandException.fromRilErrno(
                            RILConstants.RADIO_NOT_AVAILABLE);
                    mtkLoge("Received message " + msg + "[" + msg.what
                            + "] while being destroyed. return exception.");
                    response.arg1 = -1;
                    response.sendToTarget();
                } else {
                // MTK-END
                    mtkLoge("Received message " + msg + "[" + msg.what
                            + "] while being destroyed. Ignoring.");
                // MTK-START
                }
                // MTK-END
                return;
            }

            switch (msg.what) {
                case EVENT_PIN1_PUK1_DONE:
                    // a PIN/PUK/PIN2/PUK2 complete
                    // request has completed. ar.userObj is the response Message
                    int attemptsRemaining = -1;
                    ar = (AsyncResult)msg.obj;
                    if ((ar.exception != null) && (ar.result != null)) {
                        attemptsRemaining = parsePinPukErrorResult(ar);
                    }
                    Message response = (Message)ar.userObj;
                    AsyncResult.forMessage(response).exception = ar.exception;
                    response.arg1 = attemptsRemaining;
                    response.sendToTarget();
                    break;
                // MTK-START
                // [ALPS01827777]--- START ---
                // Need to query lock setting since it might be changed when
                // entering PUK to change PIN.
                case EVENT_PUK1_CHANGE_PIN1_DONE:
                    mtkLog("EVENT_PUK1_CHANGE_PIN1_DONE");
                    int attemptsRemainingPuk = -1;
                    ar = (AsyncResult) msg.obj;
                    if ((ar.exception != null) && (ar.result != null)) {
                        attemptsRemainingPuk = parsePinPukErrorResult(ar);
                    }
                    Message responsePuk = (Message) ar.userObj;
                    AsyncResult.forMessage(responsePuk).exception = ar.exception;
                    responsePuk.arg1 = attemptsRemainingPuk;
                    responsePuk.sendToTarget();
                    queryPin1State();
                    break;
                case EVENT_PUK2_CHANGE_PIN2_DONE:
                    int attemptsRemainingPuk2 = -1;
                    ar = (AsyncResult) msg.obj;
                    if ((ar.exception != null) && (ar.result != null)) {
                        attemptsRemainingPuk2 = parsePinPukErrorResult(ar);
                    }
                    Message responsePuk2 = (Message) ar.userObj;
                    AsyncResult.forMessage(responsePuk2).exception = ar.exception;
                    responsePuk2.arg1 = attemptsRemainingPuk2;
                    responsePuk2.sendToTarget();
                    queryFdn();
                    break;
                // [ALPS01827777]--- END ---
                // MTK-END
                // MTK-START: SIM ME LOCK
                case EVENT_QUERY_NETWORK_LOCK_DONE:
                    if (DBG) mtkLog("handleMessage (EVENT_QUERY_NETWORK_LOCK)");
                    ar = (AsyncResult) msg.obj;

                    if (ar.exception != null) {
                        Rlog.e(LOG_TAG_EX, "Error query network lock with exception "
                            + ar.exception);
                    }
                    AsyncResult.forMessage((Message) ar.userObj, ar.result, ar.exception);
                    ((Message) ar.userObj).sendToTarget();
                    break;
                case EVENT_CHANGE_NETWORK_LOCK_DONE:
                    if (DBG) mtkLog("handleMessage (EVENT_CHANGE_NETWORK_LOCK)");
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception != null) {
                        Rlog.e(LOG_TAG_EX, "Error change network lock with exception "
                            + ar.exception);
                    }
                    AsyncResult.forMessage(((Message) ar.userObj)).exception
                                                        = ar.exception;
                    ((Message) ar.userObj).sendToTarget();
                    break;
                // MTK-END
                default:
                    mtkLoge("Unknown Event " + msg.what);
            }
        }
    };

    /**
     * Returns the authContext based on the type of UiccCard.
     *
     * @param appType the app type
     * @return authContext corresponding to the type or AUTH_CONTEXT_UNDEFINED if appType not
     * supported
     */
    private static int getAuthContextEx(AppType appType) {
        int authContext;

        switch (appType) {
            case APPTYPE_SIM:
                authContext = AUTH_CONTEXT_EAP_SIM;
                break;

            case APPTYPE_USIM:
            // MTK-START
            case APPTYPE_ISIM:
            // MTK-END
                authContext = AUTH_CONTEXT_EAP_AKA;
                break;

            default:
                authContext = AUTH_CONTEXT_UNDEFINED;
                break;
        }

        return authContext;
    }

    /**
     * Supply the ICC PIN to the ICC
     *
     * When the operation is complete, onComplete will be sent to its
     * Handler.
     *
     * onComplete.obj will be an AsyncResult
     * onComplete.arg1 = remaining attempts before puk locked or -1 if unknown
     *
     * ((AsyncResult)onComplete.obj).exception == null on success
     * ((AsyncResult)onComplete.obj).exception != null on fail
     *
     * If the supplied PIN is incorrect:
     * ((AsyncResult)onComplete.obj).exception != null
     * && ((AsyncResult)onComplete.obj).exception
     *       instanceof com.android.internal.telephony.gsm.CommandException)
     * && ((CommandException)(((AsyncResult)onComplete.obj).exception))
     *          .getCommandError() == CommandException.Error.PASSWORD_INCORRECT
     */
    @Override
    public void supplyPin (String pin, Message onComplete) {
        synchronized (mLock) {
            mCi.supplyIccPinForApp(pin, mAid, mHandlerEx.obtainMessage(EVENT_PIN1_PUK1_DONE,
                    onComplete));
        }
    }

    /**
     * Supply the ICC PUK to the ICC
     *
     * When the operation is complete, onComplete will be sent to its
     * Handler.
     *
     * onComplete.obj will be an AsyncResult
     * onComplete.arg1 = remaining attempts before Icc will be permanently unusable
     * or -1 if unknown
     *
     * ((AsyncResult)onComplete.obj).exception == null on success
     * ((AsyncResult)onComplete.obj).exception != null on fail
     *
     * If the supplied PIN is incorrect:
     * ((AsyncResult)onComplete.obj).exception != null
     * && ((AsyncResult)onComplete.obj).exception
     *       instanceof com.android.internal.telephony.gsm.CommandException)
     * && ((CommandException)(((AsyncResult)onComplete.obj).exception))
     *          .getCommandError() == CommandException.Error.PASSWORD_INCORRECT
     *
     *
     */
    @Override
    public void supplyPuk (String puk, String newPin, Message onComplete) {
        synchronized (mLock) {
        // MTK-START
        // [ALPS01827777]--- START ---
        if (DBG) mtkLog("supplyPuk");
        mCi.supplyIccPukForApp(puk, newPin, mAid,
                mHandlerEx.obtainMessage(EVENT_PUK1_CHANGE_PIN1_DONE, onComplete));
        // [ALPS01827777]--- END ---
        // MTK-END
        }
    }

    @Override
    public void supplyPuk2 (String puk2, String newPin2, Message onComplete) {
        synchronized (mLock) {
            // MTK-START
            // [ALPS01827777]--- START ---
            mCi.supplyIccPuk2ForApp(puk2, newPin2, mAid,
                    mHandlerEx.obtainMessage(EVENT_PUK2_CHANGE_PIN2_DONE, onComplete));
            // [ALPS01827777]--- END ---
            // MTK-END
        }
    }

     //MTK-START: SIM ME LOCK [mtk80601][111215][ALPS00093395]
     /**
      * Check whether ICC network lock is enabled
      * This is an async call which returns lock state to applications directly
      */
     public void queryIccNetworkLock(int category, Message onComplete) {
         if (DBG) mtkLog("queryIccNetworkLock(): category =  " + category);

         switch(category) {
             case CAT_NETWOEK:
             case CAT_NETOWRK_SUBSET:
             case CAT_CORPORATE:
             case CAT_SERVICE_PROVIDER:
             case CAT_SIM:
             case CAT_NS_SP:
             case CAT_SIM_C:
                 ((MtkRIL)mCi).queryNetworkLock(category,
                         mHandlerEx.obtainMessage(EVENT_QUERY_NETWORK_LOCK_DONE, onComplete));
                 break;
             default:
                 Rlog.e(LOG_TAG_EX, "queryIccNetworkLock unknown category = " + category);
                 break;
         }
    }

     /**
      * Set the ICC network lock enabled or disabled
      * When the operation is complete, onComplete will be sent to its handler
      */
     public void setIccNetworkLockEnabled(int category, int lockop, String password,
             String data_imsi, String gid1, String gid2, Message onComplete) {
         if (DBG) mtkLog("SetIccNetworkEnabled(): category = " + category
             + " lockop = " + lockop + " password = " + password
             + " data_imsi = " + data_imsi + " gid1 = " + gid1 + " gid2 = " + gid2);

         switch(lockop) {
             case OP_REMOVE:
             case OP_ADD:
             case OP_LOCK:
             case OP_PERMANENT_UNLOCK:
             case OP_UNLOCK:
                 ((MtkRIL)mCi).setNetworkLock(category, lockop, password, data_imsi, gid1, gid2,
                         mHandlerEx.obtainMessage(EVENT_CHANGE_NETWORK_LOCK_DONE, onComplete));
                 break;
             default:
                 Rlog.e(LOG_TAG_EX, "SetIccNetworkEnabled unknown operation" + lockop);
                 break;
         }
     }
     // MTK-END

     // MTK-START
     private RegistrantList mFdnChangedRegistrants = new RegistrantList();

     public void registerForFdnChanged(Handler h, int what, Object obj) {
         synchronized (mLock) {
             Registrant r = new Registrant(h, what, obj);
             mFdnChangedRegistrants.add(r);
         }
     }

     public void unregisterForFdnChanged(Handler h) {
         synchronized (mLock) {
             mFdnChangedRegistrants.remove(h);
         }
     }

     private void notifyFdnChangedRegistrants() {
         if (mDestroyed) {
             return;
         }

         mFdnChangedRegistrants.notifyRegistrants();
     }
     // MTK-END
     //public int getSlotId() {
     //    return mUiccCard.getPhoneId();
     //}

     public String getIccCardType() {
          if (mIccType == null || mIccType.equals("")) {
             mIccType = SystemProperties.get(UICCCARDAPPLICATION_PROPERTY_RIL_UICC_TYPE[mPhoneId]);
          }

         mtkLog("getIccCardType(): mIccType = " + mIccType);
         return mIccType;
     }


     /** Assumes mLock is held. */
     @Override
     public void queryFdn() {
         //This shouldn't change run-time. So needs to be called only once.
         int serviceClassX;
         // MTK BSP+ START
         if (getType() == AppType.APPTYPE_ISIM) {
             mtkLog("queryFdn(): do nothing for ISIM.");
             return;
         }
         // MTK BSP+ END
         serviceClassX = CommandsInterface.SERVICE_CLASS_VOICE +
                         CommandsInterface.SERVICE_CLASS_DATA +
                         CommandsInterface.SERVICE_CLASS_FAX;
         mCi.queryFacilityLockForApp (
                 CommandsInterface.CB_FACILITY_BA_FD, "", serviceClassX,
                 mAid, mHandler.obtainMessage(EVENT_QUERY_FACILITY_FDN_DONE));
     }

     @Override
     protected void queryPin1State() {
         int serviceClassX = CommandsInterface.SERVICE_CLASS_VOICE +
                 CommandsInterface.SERVICE_CLASS_DATA +
                 CommandsInterface.SERVICE_CLASS_FAX;
         // MTK BSP+ START
         if (getType() == AppType.APPTYPE_ISIM) {
             mtkLog("queryPin1State(): do nothing for ISIM.");
             return;
         }
         // MTK BSP+ END
         mCi.queryFacilityLockForApp (
             CommandsInterface.CB_FACILITY_BA_SIM, "", serviceClassX,
             mAid, mHandler.obtainMessage(EVENT_QUERY_FACILITY_LOCK_DONE));
     }
     /** REMOVE when mIccLockEnabled is not needed*/

     @Override
     public boolean getIccFdnAvailable() {
         // MTK-START
         if (mIccRecords == null) {
             if (DBG) mtkLoge("isFdnExist mIccRecords == null");
             return false;
         }

         IccServiceInfo.IccServiceStatus iccSerStatus = IccServiceInfo.IccServiceStatus
                 .NOT_EXIST_IN_USIM;
         boolean isPhbReady = false;
         if (mIccRecords instanceof MtkSIMRecords) {
             iccSerStatus = ((MtkSIMRecords)mIccRecords).getSIMServiceStatus(
                 IccServiceInfo.IccService.FDN);
             isPhbReady = ((MtkSIMRecords) mIccRecords).isPhbReady();
         } else if (mIccRecords instanceof RuimRecords) {
             iccSerStatus = ((MtkRuimRecords)mIccRecords).getSIMServiceStatus(
                 IccServiceInfo.IccService.FDN);
             isPhbReady = ((MtkRuimRecords) mIccRecords).isPhbReady();
         } else {
             iccSerStatus = IccServiceInfo.IccServiceStatus.NOT_EXIST_IN_USIM;
         }
         if (DBG) {
             log("getIccFdnAvailable status iccSerStatus:" + iccSerStatus);
         }
         // FDN toggle operation should be done after phb ready, otherwise it will
         // impact the FDN list build process
         return ((iccSerStatus == IccServiceInfo.IccServiceStatus.ACTIVATED) && isPhbReady);

         // MTK-END
     }

     @Override
     protected void log(String msg) {
         Rlog.d(LOG_TAG, msg + " (slot " + mPhoneId + ")");
     }
     @Override
     protected void loge(String msg) {
         Rlog.e(LOG_TAG, msg + " (slot " + mPhoneId + ")");
     }

     protected void mtkLog(String msg) {
         Rlog.d(LOG_TAG_EX, msg + " (slot " + mPhoneId + ")");
     }
     protected void mtkLoge(String msg) {
         Rlog.e(LOG_TAG_EX, msg + " (slot " + mPhoneId + ")");
     }
}
