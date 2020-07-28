/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2014. All rights reserved.
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

package com.mediatek.ims.ril;


import android.content.Context;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Registrant;
import android.os.RegistrantList;
import android.telephony.Rlog;
import android.telephony.TelephonyManager;

/**
 * {@hide}
 */
public abstract class ImsBaseCommands implements ImsCommandsInterface {

    //***** Instance Variables

    // Context
    protected Context mContext;

    // Radio State of Current Phone
    protected int mState = TelephonyManager.RADIO_POWER_UNAVAILABLE;

    // Current Phone Id
    protected int mPhoneId;

    // Lock Token
    protected Object mStateMonitor = new Object();

    protected RegistrantList mRadioStateChangedRegistrants = new RegistrantList();
    protected RegistrantList mOnRegistrants = new RegistrantList();
    protected RegistrantList mOffRegistrants = new RegistrantList();
    protected RegistrantList mAvailRegistrants = new RegistrantList();
    protected RegistrantList mOffOrNotAvailRegistrants = new RegistrantList();
    protected RegistrantList mNotAvailRegistrants = new RegistrantList();

    // Call Control
    protected RegistrantList mIncomingCallIndicationRegistrants = new RegistrantList();
    protected RegistrantList mCallAdditionalInfoRegistrants = new RegistrantList();
    protected RegistrantList mCallRatIndicationRegistrants = new RegistrantList();

    // VoLTE
    protected RegistrantList mImsEnableStartRegistrants = new RegistrantList();
    protected RegistrantList mImsDisableStartRegistrants = new RegistrantList();
    protected RegistrantList mImsEnableDoneRegistrants = new RegistrantList();
    protected RegistrantList mImsDisableDoneRegistrants = new RegistrantList();

    protected RegistrantList mImsRegistrationInfoRegistrants = new RegistrantList();

    /// IMS Stack Relative

    /* Register for updating conference call merged/added result. */
    protected RegistrantList mEconfResultRegistrants = new RegistrantList();
    /* Register for updating call mode and pau. */
    protected RegistrantList mCallInfoRegistrants = new RegistrantList();
    /* Register for updating explicit call transfer result. */
    protected RegistrantList mEctResultRegistrants = new RegistrantList();

    protected RegistrantList mCallProgressIndicatorRegistrants = new RegistrantList();

    // ViLTE feature, call mode changed event.
    protected RegistrantList mCallModeChangeIndicatorRegistrants = new RegistrantList();
    protected RegistrantList mVideoCapabilityIndicatorRegistrants = new RegistrantList();

    // For incoming USSI event
    protected RegistrantList mUSSIRegistrants = new RegistrantList();

    // ECBM
    protected RegistrantList mEnterECBMRegistrants = new RegistrantList();
    protected RegistrantList mExitECBMRegistrants = new RegistrantList();
    protected RegistrantList mNoECBMRegistrants = new RegistrantList();

    // IMS Provisioning
    protected RegistrantList mImsGetProvisionDoneRegistrants = new RegistrantList();
    // Register for updating RTP packets info
    protected RegistrantList mRTPInfoRegistrants = new RegistrantList();

    // Sync VoLTE setting value
    protected RegistrantList mVolteSettingRegistrants = new RegistrantList();
    protected Object mVolteSettingValue = null;

    // Register for IMS Bearer update
    protected RegistrantList mBearerStateRegistrants = new RegistrantList();
    protected RegistrantList mBearerInitRegistrants = new RegistrantList();

    // Register for IMS Data info notification
    protected RegistrantList mImsDataInfoNotifyRegistrants = new RegistrantList();

    // Register for IMS Xui information update
    protected RegistrantList mXuiRegistrants = new RegistrantList();

    // Register for VoLTE subscription update
    protected RegistrantList mVolteSubscriptionRegistrants = new RegistrantList();

    // Register for Supplementary service notification
    protected RegistrantList mSuppServiceNotificationRegistrants = new RegistrantList();

    // Register for IMS Event Package
    protected RegistrantList mImsEvtPkgRegistrants = new RegistrantList();

    // IMS Deregister Done
    protected RegistrantList mImsDeregistrationDoneRegistrants = new RegistrantList();

    // MD support multiple IMS count
    protected RegistrantList mMultiImsCountRegistrants = new RegistrantList();

    // Register for IMS Conference participants info
    protected RegistrantList mImsConfInfoRegistrants = new RegistrantList();
    // Register for LTE Message Waiting Information
    protected RegistrantList mLteMsgWaitingRegistrants = new RegistrantList();

    // Ims Cfg dynamic ims switch complete
    protected RegistrantList mImsCfgDynamicImsSwitchCompleteRegistrants = new RegistrantList();

    // Ims Cfg feature changed
    protected RegistrantList mImsCfgFeatureChangedRegistrants = new RegistrantList();

    // Ims Cfg config changed
    protected RegistrantList mImsCfgConfigChangedRegistrants = new RegistrantList();

    // Ims Cfg Config Loaded
    protected RegistrantList mImsCfgConfigLoadedRegistrants = new RegistrantList();

    // RTT Capability Indication RegistrantList
    protected RegistrantList mRttCapabilityIndicatorRegistrants = new RegistrantList();

    // RTT Modify Response RegistrantList
    protected RegistrantList mRttModifyResponseRegistrants = new RegistrantList();

    // RTT Text Receive RegistrantList
    protected RegistrantList mRttTextReceiveRegistrants = new RegistrantList();

    // RTT Modify Request Receive RegistrantList
    protected RegistrantList mRttModifyRequestReceiveRegistrants = new RegistrantList();

    // RTT Modify Request Receive RegistrantList
    protected RegistrantList mRttAudioIndicatorRegistrants = new RegistrantList();

    // VoPS indication
    protected RegistrantList mVopsStatusIndRegistrants = new RegistrantList();

    // Report the indicator +EIMSREGURI and +EIMSREGRESP
    protected RegistrantList mImsRegStatusIndRistrants = new RegistrantList();

    // Report the SIP headers
    protected RegistrantList mImsSipHeaderRegistrants = new RegistrantList();

    // Report the detail IMS registration information
    protected RegistrantList mEiregIndRegistrants = new RegistrantList();

    // Report the SSAC information
    protected RegistrantList mSsacIndRegistrants = new RegistrantList();


    /**
     * Constructor
     * @param context Android Context
     * @param phoneid PhoneId
     */
    public ImsBaseCommands(Context context, int phoneid) {
        mContext = context;
        mPhoneId = phoneid;
    }

    @Override
    public void setOnIncomingCallIndication(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mIncomingCallIndicationRegistrants.add(r);
    }

    @Override
    public void unsetOnIncomingCallIndication(Handler h) {
        mIncomingCallIndicationRegistrants.remove(h);
    }

    @Override
    public void registerForCallAdditionalInfo(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mCallAdditionalInfoRegistrants.add(r);
    }

    @Override
    public void unregisterForCallAdditionalInfo(Handler h) {
        mCallAdditionalInfoRegistrants.remove(h);
    }

    @Override
    public void registerForCallRatIndication(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mCallRatIndicationRegistrants.add(r);
    }

    @Override
    public void unregisterForCallRatIndication(Handler h) {
        mCallRatIndicationRegistrants.remove(h);
    }

    @Override
    public void registerForEconfResult(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mEconfResultRegistrants.add(r);
    }

    @Override
    public void unregisterForEconfResult(Handler h) {
        mEconfResultRegistrants.remove(h);
    }

    @Override
    public void registerForCallInfo(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mCallInfoRegistrants.add(r);
    }

    @Override
    public void unregisterForCallInfo(Handler h) {
        mCallInfoRegistrants.remove(h);
    }

    @Override
    public void registerForImsEnableStart(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mImsEnableStartRegistrants.add(r);
    }

    @Override
    public void unregisterForImsEnableStart(Handler h) {
        mImsEnableStartRegistrants.remove(h);
    }

    @Override
    public void registerForImsDisableStart(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mImsDisableStartRegistrants.add(r);
    }

    @Override
    public void unregisterForImsDisableStart(Handler h) {
        mImsDisableStartRegistrants.remove(h);
    }

    @Override
    public void registerForImsEnableComplete(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mImsEnableDoneRegistrants.add(r);
    }

    @Override
    public void unregisterForImsEnableComplete(Handler h) {
        mImsEnableDoneRegistrants.remove(h);
    }

    @Override
    public void registerForImsDisableComplete(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mImsDisableDoneRegistrants.add(r);
    }

    @Override
    public void unregisterForImsDisableComplete(Handler h) {
        mImsDisableDoneRegistrants.remove(h);
    }

    @Override
    public void registerForImsRegistrationInfo(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mImsRegistrationInfoRegistrants.add(r);
    }

    @Override
    public void unregisterForImsRegistrationInfo(Handler h) {
        mImsRegistrationInfoRegistrants.remove(h);
    }

    @Override
    public void registerForCallProgressIndicator(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);

        mCallProgressIndicatorRegistrants.add(r);
    }

    @Override
    public void unregisterForCallProgressIndicator(Handler h) {
        mCallProgressIndicatorRegistrants.remove(h);
    }

    @Override
    public void registerForOnEnterECBM(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mEnterECBMRegistrants.add(r);
    }

    @Override
    public void unregisterForOnEnterECBM(Handler h) {
        mEnterECBMRegistrants.remove(h);
    }

    @Override
    public void registerForOnExitECBM(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mExitECBMRegistrants.add(r);
    }

    @Override
    public void unregisterForOnExitECBM(Handler h) {
        mExitECBMRegistrants.remove(h);
    }

    @Override
    public void registerForOnNoECBM(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mNoECBMRegistrants.add(r);
    }

    @Override
    public void unregisterForOnNoECBM(Handler h) {
        mNoECBMRegistrants.remove(h);
    }

    @Override
    public void registerForGetProvisionComplete(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mImsGetProvisionDoneRegistrants.add(r);
    }

    @Override
    public void unregisterForGetProvisionComplete(Handler h) {
        mImsGetProvisionDoneRegistrants.remove(h);
    }

    @Override
    public void registerForEctResult(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mEctResultRegistrants.add(r);
    }

    @Override
    public void unregisterForEctResult(Handler h) {
        mEctResultRegistrants.remove(h);
    }

    @Override
    public void registerForCallModeChangeIndicator(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);

        mCallModeChangeIndicatorRegistrants.add(r);
    }

    @Override
    public void unregisterForCallModeChangeIndicator(Handler h) {
        mCallModeChangeIndicatorRegistrants.remove(h);
    }

    @Override
    public void registerForVideoCapabilityIndicator(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mVideoCapabilityIndicatorRegistrants.add(r);
    }

    @Override
    public void unregisterForVideoCapabilityIndicator(Handler h) {
        mVideoCapabilityIndicatorRegistrants.remove(h);
    }

    @Override
    public void registerForImsRTPInfo(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mRTPInfoRegistrants.add(r);
    }

    @Override
    public void unregisterForImsRTPInfo(Handler h) {
        mRTPInfoRegistrants.remove(h);
    }

    @Override
    public void registerForVolteSettingChanged(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mVolteSettingRegistrants.add(r);
        if (mVolteSettingValue != null) {
            r.notifyRegistrant(new AsyncResult(null, mVolteSettingValue, null));
        }
    }

    @Override
    public void unregisterForVolteSettingChanged(Handler h) {
        mVolteSettingRegistrants.remove(h);
    }

    @Override
    public void registerForBearerState(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mBearerStateRegistrants.add(r);
    }

    @Override
    public void unregisterForBearerState(Handler h) {
        mBearerStateRegistrants.remove(h);
    }

    @Override
    public void registerForImsDataInfoNotify(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mImsDataInfoNotifyRegistrants.add(r);
    }

    @Override
    public void unregisterForImsDataInfoNotify(Handler h) {
        mImsDataInfoNotifyRegistrants.remove(h);
    }

    @Override
    public void registerForXuiInfo(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mXuiRegistrants.add(r);
    }

    @Override
    public void unregisterForXuiInfo(Handler h) {
        mXuiRegistrants.remove(h);
    }

    @Override
    public void registerForVolteSubscription(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mVolteSubscriptionRegistrants.add(r);
    }

    @Override
    public void unregisterForVolteSubscription(Handler h) {
        mVolteSubscriptionRegistrants.remove(h);
    }

    @Override
    public void setOnSuppServiceNotification(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mSuppServiceNotificationRegistrants.add(r);
    }

    @Override
    public void unSetOnSuppServiceNotification(Handler h) {
        mSuppServiceNotificationRegistrants.remove(h);
    }

    @Override
    public void registerForImsEventPackage(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mImsEvtPkgRegistrants.add(r);
    }

    @Override
    public void setOnUSSI(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mUSSIRegistrants.add(r);
    }

    @Override
    public void unSetOnUSSI(Handler h) {
        mUSSIRegistrants.remove(h);
    }

    /**
     * Un-register for ims event package.
     *
     * @param h handler
     * @hide
     */
    @Override
    public void unregisterForImsEventPackage(Handler h) {
        mImsEvtPkgRegistrants.remove(h);
    }

    /** Register for IMS bearer initialize event.
     *
     * @param h handler
     * @param what message
     * @param obj object
     * @hide
     */
    @Override
    public void registerForBearerInit(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mBearerInitRegistrants.add(r);
    }

    /**
     * Un-register for IMS bearer initialize event.
     *
     * @param h handler
     * @hide
     */
    @Override
    public void unregisterForBearerInit(Handler h) {
        mBearerInitRegistrants.remove(h);
    }


    /**
     * Register for Not available
     * @param h
     * @param what
     * @param obj
     */
    @Override
    public void registerForNotAvailable(Handler h, int what, Object obj) {
        Registrant r = new Registrant (h, what, obj);

        synchronized (mStateMonitor) {
            mNotAvailRegistrants.add(r);

            if (mState == TelephonyManager.RADIO_POWER_UNAVAILABLE) {
                r.notifyRegistrant(new AsyncResult(null, null, null));
            }
        }
    }

    /**
     * Unregister for not available
     * @param h
     */
    @Override
    public void unregisterForNotAvailable(Handler h) {
        synchronized (mStateMonitor) {
            mNotAvailRegistrants.remove(h);
        }
    }

    /**
     * Register for OFF
     * @param h
     * @param what
     * @param obj
     */
    @Override
    public void registerForOff(Handler h, int what, Object obj) {
        Registrant r = new Registrant (h, what, obj);

        synchronized (mStateMonitor) {
            mOffRegistrants.add(r);

            if (mState == TelephonyManager.RADIO_POWER_OFF) {
                r.notifyRegistrant(new AsyncResult(null, null, null));
            }
        }
    }

    /**
     * Unregister for OFF
     * @param h
     * @param what
     * @param obj
     */
    @Override
    public void unregisterForOff(Handler h) {
        synchronized(mStateMonitor) {
            mOffRegistrants.remove(h);
        }
    }

    /**
     * Register for ON
     * @param h
     * @param what
     * @param obj
     */
    @Override
    public void registerForOn(Handler h, int what, Object obj) {
        Registrant r = new Registrant (h, what, obj);

        synchronized (mStateMonitor) {
            mOnRegistrants.add(r);

            if (mState == TelephonyManager.RADIO_POWER_ON) {
                r.notifyRegistrant(new AsyncResult(null, null, null));
            }
        }
    }

    /**
     * Unregister for ON
     * @param h
     * @param what
     * @param obj
     */
    @Override
    public void unregisterForOn(Handler h) {
        synchronized (mStateMonitor) {
            mOnRegistrants.remove(h);
        }
    }

    /**
     * Register for IMS Deregister Complete
     * @param h
     * @param what
     * @param obj
     */
    @Override
    public void registerForImsDeregisterComplete(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mImsDeregistrationDoneRegistrants.add(r);
    }

    /**
     * Unregister for IMS Deregister Complete
     * @param h
     */
    @Override
    public void unregisterForImsDeregisterComplete(Handler h) {
        mImsDeregistrationDoneRegistrants.remove(h);
    }

    /**
     * Register for MD support IMS count indication
     * @param h
     * @param what
     * @param obj
     */
    @Override
    public void registerForMultiImsCount(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mMultiImsCountRegistrants.add(r);
    }

    /**
     * Unregister for MD support IMS count indication
     * @param h
     */
    @Override
    public void unregisterForMultiImsCount(Handler h) {
        mMultiImsCountRegistrants.remove(h);
    }

    /// M: Register for IMS support ECC updating. @{
    protected RegistrantList mImsEccSupportRegistrants = new RegistrantList();
    /// @}

    /**
     * Register for IMS ECC support event. (RIL_UNSOL_IMS_SUPPORT_ECC)
     *
     * @param h handler
     * @param what message
     * @param obj object
     * @hide
     */
    public void registerForImsEccSupport(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mImsEccSupportRegistrants.add(r);
    }

    public void unregisterForImsEccSupport(Handler h) {
        mImsEccSupportRegistrants.remove(h);
    }

    /// M: Register for IMS support Speech codec. @{
    protected RegistrantList mSpeechCodecInfoRegistrant = new RegistrantList();

    public void registerForSpeechCodecInfo(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mSpeechCodecInfoRegistrant.add(r);
    }

    public void unregisterForSpeechCodecInfo(Handler h) {
        mSpeechCodecInfoRegistrant.remove(h);
    }
    ///@}

    /**
     * Register a registrant for 'ImsConferenceInfo'
     * @param h
     * @param what
     * @param obj
     */
    @Override
    public void registerForImsConfInfoUpdate(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mImsConfInfoRegistrants.add(r);
    }

    /**
     * Unregister a registrant for 'ImsConferenceInfo'
     * @param h
     */
    public void unregisterForImsConfInfoUpdate(Handler h) {
        mImsConfInfoRegistrants.remove(h);
    }

    /**
     * Register a registrant for 'LteMsgWaiting'
     * @param h
     * @param what
     * @param obj
     */
    public void registerForLteMsgWaiting(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mLteMsgWaitingRegistrants.add(r);
    }

    /**
     * Unregister a registrant for 'ImsEventPackage'
     * @param h
     */
    public void unregisterForLteMsgWaiting(Handler h) {
        mLteMsgWaitingRegistrants.remove(h);
    }

    /// M: Register for IMS Dialog event package indication. @{
    protected RegistrantList mImsDialogRegistrant = new RegistrantList();

    public void registerForImsDialog(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mImsDialogRegistrant.add(r);
    }

    public void unregisterForImsDialog(Handler h) {
        mImsDialogRegistrant.remove(h);
    }
    ///@}

    /**
     * Register for Ims config dynamic ims switch complete indication
     * @param h
     * @param what
     * @param obj
     */
    public void registerForImsCfgDynamicImsSwitchComplete(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mImsCfgDynamicImsSwitchCompleteRegistrants.add(r);
    }

    /**
     * Unregister for Ims config dynamic ims switch complete indication
     * @param h
     */
    public void unregisterForImsCfgDynamicImsSwitchComplete(Handler h) {
        mImsCfgDynamicImsSwitchCompleteRegistrants.remove(h);
    }

    /**
     * Register for Ims config feature changed indication
     * @param h
     * @param what
     * @param obj
     */
    public void registerForImsCfgFeatureChanged(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mImsCfgFeatureChangedRegistrants.add(r);
    }

    /**
     * Unregister for Ims config feature changed indication
     * @param h
     */
    public void unregisterForImsCfgFeatureChanged(Handler h) {
        mImsCfgFeatureChangedRegistrants.remove(h);
    }

    /**
     * Register for Ims config config changed indication
     * @param h
     * @param what
     * @param obj
     */
    public void registerForImsCfgConfigChanged(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mImsCfgConfigChangedRegistrants.add(r);
    }

    /**
     * Unregister for Ims config config changed indication
     * @param h
     */
    public void unregisterForImsCfgConfigChanged(Handler h) {
        mImsCfgConfigChangedRegistrants.remove(h);
    }

    /**
     * Register for Ims config config loaded indication
     * @param h
     * @param what
     * @param obj
     */
    public void registerForImsCfgConfigLoaded(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mImsCfgConfigLoadedRegistrants.add(r);
    }

    /**
     * Unregister for Ims config config changed indication
     * @param h
     */
    public void unregisterForImsCfgConfigLoaded(Handler h) {
        mImsCfgConfigLoadedRegistrants.remove(h);
    }

    // Register for SMS status report and new sms
    protected Registrant mSmsStatusRegistrant;
    protected Registrant mNewSmsRegistrant;
    protected Registrant mCdmaSmsRegistrant;

    /**
     * Register for SMS status report indication
     * @param h
     * @param what
     * @param obj
     */
    public void setOnSmsStatus(Handler h, int what, Object obj) {
        mSmsStatusRegistrant = new Registrant (h, what, obj);
    }

    /**
     * Unregister for SMS status report indication
     * @param h
     */
    public void unSetOnSmsStatus(Handler h) {
        if (mSmsStatusRegistrant != null && mSmsStatusRegistrant.getHandler() == h) {
            mSmsStatusRegistrant.clear();
            mSmsStatusRegistrant = null;
        }
    }

    /**
     * Register for SMS status report indication
     * @param h
     * @param what
     * @param obj
     */
    public void setOnNewSms(Handler h, int what, Object obj) {
        mNewSmsRegistrant = new Registrant (h, what, obj);
    }

    /**
     * Unregister for SMS status report indication
     * @param h
     */
    public void unSetOnNewSms(Handler h) {
        if (mNewSmsRegistrant != null && mNewSmsRegistrant.getHandler() == h) {
            mNewSmsRegistrant.clear();
            mNewSmsRegistrant = null;
        }
    }

    public void setOnNewCdmaSms(Handler h, int what, Object obj) {
        mCdmaSmsRegistrant = new Registrant(h, what, obj);
    }

    public void unSetOnNewCdmaSms(Handler h) {
        if (mCdmaSmsRegistrant != null && mCdmaSmsRegistrant.getHandler() == h) {
            mCdmaSmsRegistrant.clear();
            mCdmaSmsRegistrant = null;
        }
    }

    protected RegistrantList mImsRedialEccIndRegistrants = new RegistrantList();

    /**
     * Register for redial IMS ECC indication event. (RIL_UNSOL_REDIAL_EMERGENCY_INDICATION)
     *
     * @param h handler
     * @param what message
     * @param obj object
     * @hide
     */
    public void registerForImsRedialEccInd(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mImsRedialEccIndRegistrants.add(r);
    }

    public void unregisterForImsRedialEccInd(Handler h) {
        mImsRedialEccIndRegistrants.remove(h);
    }

    // Protected Method Below

    protected void onRadioAvailable() {
    }

    /**
     * Store a new RadioState and send notification base on the <br/>
     * the changes when the RIL_UNSOL_RADIO_STATE_CHANGED comes.</br>
     * <br/>
     * This function is only called by the inheritance class while <br/>
     * receiving unsolicited event
     * @param newState New Radio State
     */
    protected void setRadioState(int newState) {
        int oldState;

        synchronized (mStateMonitor) {
            oldState = mState;
            mState = newState;

            if (oldState == mState) {
                // no state transition
                Rlog.d("ImsBaseCommands", "no state transition: " + mState);
                return;
            }

            mRadioStateChangedRegistrants.notifyRegistrants();

            if (mState != TelephonyManager.RADIO_POWER_UNAVAILABLE
                    && oldState == TelephonyManager.RADIO_POWER_UNAVAILABLE) {
                mAvailRegistrants.notifyRegistrants();
            }

            if (mState == TelephonyManager.RADIO_POWER_UNAVAILABLE
                    && oldState != TelephonyManager.RADIO_POWER_UNAVAILABLE) {
                mNotAvailRegistrants.notifyRegistrants();
            }

            if (mState == TelephonyManager.RADIO_POWER_ON
                    && oldState != TelephonyManager.RADIO_POWER_ON) {
                mOnRegistrants.notifyRegistrants();
            }

            if ((mState == TelephonyManager.RADIO_POWER_OFF
                    || mState == TelephonyManager.RADIO_POWER_UNAVAILABLE)
                    && (oldState == TelephonyManager.RADIO_POWER_ON)) {
                mOffOrNotAvailRegistrants.notifyRegistrants();
            }

            if (mState == TelephonyManager.RADIO_POWER_OFF) {
                mOffRegistrants.notifyRegistrants();
            }
        }
    }

    /**
     * Broadcast after the change of radio state
     * MTK Proprietary Logical
     * @param newState New Change
     */
    protected void notifyRadioStateChanged(int newState) {
        // Do Notify Here
    }

    // ========= RTT ========================================================
    /**
     * Registers the handler for RTT capability changed event.
     * @param h Handler for notification message.
     * @param what User-defined message code.
     * @param obj User object.
     *
     */
    @Override
    public void registerForRttCapabilityIndicator(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mRttCapabilityIndicatorRegistrants.add(r);
    }

    /**
     * Unregisters the handler for RTT capability changed event.
     *
     * @param h Handler for notification message.
     *
     */
    @Override
    public void unregisterForRttCapabilityIndicator(Handler h) {
        mRttCapabilityIndicatorRegistrants.remove(h);
    }

    /**
     * Registers the handler for Rtt Modify Response event.
     * @param h Handler for notification message.
     * @param what User-defined message code.
     * @param obj User object.
     *
     */
    @Override
    public void registerForRttModifyResponse(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mRttModifyResponseRegistrants.add(r);
    }

    /**
     * Unregisters the handler for Rtt Modify Response event.
     *
     * @param h Handler for notification message.
     *
     */
    @Override
    public void unregisterForRttModifyResponse(Handler h) {
        mRttModifyResponseRegistrants.remove(h);
    }

    /**
     * Registers the handler for Rtt Text Receive event.
     * @param h Handler for notification message.
     * @param what User-defined message code.
     * @param obj User object.
     *
     */
    @Override
    public void registerForRttTextReceive(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mRttTextReceiveRegistrants.add(r);
    }

    /**
     * Unregisters the handler for Rtt Text Receive event.
     *
     * @param h Handler for notification message.
     *
     */
    @Override
    public void unregisterForRttTextReceive(Handler h) {
        mRttTextReceiveRegistrants.remove(h);
    }

    /**
     * Registers the handler for Rtt Modify Request Receive event.
     * @param h Handler for notification message.
     * @param what User-defined message code.
     * @param obj User object.
     *
     */
    @Override
    public void registerForRttModifyRequestReceive(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mRttModifyRequestReceiveRegistrants.add(r);
    }

    /**
     * Unregisters the handler for Rtt Modify Request Receive event.
     *
     * @param h Handler for notification message.
     *
     */
    @Override
    public void unregisterForRttModifyRequestReceive(Handler h) {
        mRttModifyRequestReceiveRegistrants.remove(h);
    }

    /**
     * Registers the handler for Rtt Audio.
     * @param h Handler for notification message.
     * @param what User-defined message code.
     * @param obj User object.
     *
     */
    @Override
    public void registerForRttAudioIndicator(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mRttAudioIndicatorRegistrants.add(r);
    }

    /**
     * Unregisters the handler for Rtt Audio.
     *
     * @param h Handler for notification message.
     *
     */
    @Override
    public void unregisterForRttAudioIndicator(Handler h) {
        mRttAudioIndicatorRegistrants.remove(h);
    }

    /**
     * Register for VoPS status indication
     * @param h
     * @param what
     * @param obj
     */
    public void registerForVopsStatusInd(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mVopsStatusIndRegistrants.add(r);
    }

    /**
     * Unregister for VoPS status indication
     * @param h
     */
    public void unregisterForVopsStatusInd(Handler h) {
        mVopsStatusIndRegistrants.remove(h);
    }

    /**
     * Register for IMS registration status report indication
     * @param h
     * @param what
     * @param obj
     */
    public void registerForImsRegStatusInd(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mImsRegStatusIndRistrants.add(r);
    }

    /**
     * Unregister for IMS registration status report indication
     * @param h
     */
    public void unregisterForImsRegStatusInd(Handler h) {
        mImsRegStatusIndRistrants.remove(h);
    }

    /**
     * Register for Sip header report indication
     * @param h
     * @param what
     * @param obj
     */
    public void registerForSipHeaderInd(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mImsSipHeaderRegistrants.add(r);
    }

    /**
     * Unregister for Sip header report indication
     * @param h
     */
    public void unregisterForSipHeaderInd(Handler h) {
        mImsSipHeaderRegistrants.remove(h);
    }

    /**
     * Register for the detail IMS registration information indication
     * @param h
     * @param what
     * @param obj
     */
    public void registerForDetailImsRegistrationInd(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mEiregIndRegistrants.add(r);
    }

    /**
     * Unregister for the detail IMS registration information indication
     * @param h
     */
    public void unregisterForDetailImsRegistrationInd(Handler h) {
        mEiregIndRegistrants.remove(h);
    }

    /**
     * Register for the SSAC state indication
     * @param h
     * @param what
     * @param obj
     */
    public void registerForSsacStateInd(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);
        mSsacIndRegistrants.add(r);
    }

    /**
     * Unregister for the SSAC state indication
     * @param h
     */
    public void unregisterForSsacStateInd(Handler h) {
        mSsacIndRegistrants.remove(h);
    }
}
