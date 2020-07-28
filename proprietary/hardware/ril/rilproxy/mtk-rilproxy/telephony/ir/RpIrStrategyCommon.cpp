/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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

/*****************************************************************************
 * Include
 *****************************************************************************/

#include "RfxAction.h"
#include "RfxLog.h"
#include "RpIrStrategyCommon.h"
#include "RpIrCdmaStatusListener.h"
#include "RpIrLwgStatusListener.h"
#include "RpIrMccTable.h"
#include "RpIrUtils.h"
#include <cutils/properties.h>
#include <libmtkrilutils.h>

using namespace std;

#define RFX_LOG_TAG "[IRC][RpIrStrategyCommon]"
/*****************************************************************************
 * Class RpIrStrategyCommon
 *****************************************************************************/

RFX_IMPLEMENT_VIRTUAL_CLASS("RpIrStrategyCommon", RpIrStrategyCommon, RpIrStrategy);

int RpIrStrategyCommon::sWatchdogDelayTime = NO_SEERVICE_WATCHDOG_DELAY_TIME;
int RpIrStrategyCommon::sSwitchModeOrResumeDelayTime = SWITCH_RESUME_DELAY_TIME;

RpIrStrategyCommon::RpIrStrategyCommon(RpIrController* ric, IRpIrNwController* lwgController,
                               IRpIrNwController* cdmaController) :
        RpIrStrategy(ric, lwgController, cdmaController),
        mCdmaNwsMode(NWS_MODE_UNKNOWN),
        mLwgServiceState(OUT_OF_SERVICE),
        mCdmaServiceState(OUT_OF_SERVICE),
        mNoServiceTimeStamp(0),
        mWatchdogStartTime(0),
        mContinousRetryCount(0),
        m_lwgListener(NULL),
        m_cdmaListener(NULL),
        m_timer_handle_watchDog(NULL),
        m_timer_handle_watchDog_started(false),
        mLastInServiceNwsMode(NWS_MODE_UNKNOWN),
        mLastInServiceIccId("") {
}

void RpIrStrategyCommon::onInit() {
    RpIrStrategy::onInit();  // Required: invoke super class implementation

    char strProp[PROPERTY_VALUE_MAX] = {0};
    char strTemp[20] = {0};

    sprintf(strTemp, "%d", NO_SEERVICE_WATCHDOG_DELAY_TIME);
    memset(strProp, 0, sizeof(char)*PROPERTY_VALUE_MAX);
    property_get("persist.vendor.sys.ct.ir.wd", strProp, strTemp);
    RpIrStrategyCommon::sWatchdogDelayTime = atoi(strProp);

    sprintf(strTemp, "%d", SWITCH_RESUME_DELAY_TIME);
    memset(strProp, 0, sizeof(char)*PROPERTY_VALUE_MAX);
    property_get("persist.vendor.sys.ct.ir.rnsd", strProp, strTemp);
    RpIrStrategyCommon::sSwitchModeOrResumeDelayTime = atoi(strProp);

    RpIrLwgStatusListener *lwgListener;
    RFX_OBJ_CREATE_EX(lwgListener, RpIrLwgStatusListener, this, (this, m_IrController));
    m_lwgListener = (IRpIrNwControllerListener *)lwgListener;

    RpIrCdmaStatusListener *cdmaListener;
    RFX_OBJ_CREATE_EX(cdmaListener, RpIrCdmaStatusListener, this, (this, m_IrController));
    m_cdmaListener = (IRpIrNwControllerListener *)cdmaListener;

    m_LwgController->registerListener(m_lwgListener);
    m_CdmaController->registerListener(m_cdmaListener);
}

RpIrStrategyCommon::~RpIrStrategyCommon() {
}

bool RpIrStrategyCommon::onPreviewLwgPlmn(const String8 &plmn) {
    logD(RFX_LOG_TAG, "onPreviewLwgPlmn plmn: %s", plmn.string());

    if (RpIrUtils::isSupportRoaming(getSlotId())) {
        NwsMode targetMode = getNwsModeByPlmn(plmn);
        logD(RFX_LOG_TAG, "onPreviewLwgPlmn targetMode: %s", Nws2Str(targetMode));
        if ((targetMode == NWS_MODE_CDMALTE || targetMode == NWS_MODE_LTEONLY)
                && m_IrController->getNwsMode() == NWS_MODE_CSFB) {
            m_LwgController->setNwsMode(targetMode);
            return true;
        }
    }
    return false;
}

void RpIrStrategyCommon::onLwgPlmnChanged(const String8 &plmn) {
    logD(RFX_LOG_TAG, "onLwgPlmnChanged plmn: %s", plmn.string());

    if (RpIrUtils::isSupportRoaming(getSlotId())) {
        NwsMode targetMode = getNwsModeByPlmn(plmn);

        logD(RFX_LOG_TAG, "onLwgPlmnChanged mCdmaNwsMode: %s targetMode: %s",
                          Nws2Str(mCdmaNwsMode), Nws2Str(targetMode));

        if (targetMode == m_IrController->getNwsMode()) {
            m_LwgController->resumeNetwork();
        }
        m_LwgController->setNwsMode(targetMode);
    } else {
        m_LwgController->resumeNetwork();
        m_LwgController->setNwsMode(NWS_MODE_CDMALTE);
    }
}

void RpIrStrategyCommon::onCdmaPlmnChanged(const String8 &plmn) {
    logD(RFX_LOG_TAG, "onCdmaPlmnChanged plmn: %s", plmn.string());
    // record cdma latest roaming mode for LWG
    // to decide its roaming status
    mCdmaNwsMode = getNwsModeByPlmn(plmn);

    if (RpIrUtils::isSupportRoaming(getSlotId())) {
        // these are IR common stratrgy deails
        // cdma roaming mode has higher priority
        if (mCdmaNwsMode == m_IrController->getNwsMode()
            && mCdmaNwsMode != NWS_MODE_CSFB) {
            m_CdmaController->resumeNetwork();
        }
        m_CdmaController->setNwsMode(mCdmaNwsMode);
    }
    else if (getStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA_CARD_TYPE) == CT_3G_UIM_CARD
             && mCdmaNwsMode == NWS_MODE_CSFB) {
        // 3g uim card (not dual mode) + roaming = do not resume.
        logD(RFX_LOG_TAG, "onCdmaPlmnChanged 3g uim card (not dual mode) + roaming, do not resume.");
        return;
    }
    else {
        m_CdmaController->resumeNetwork();
        m_CdmaController->setNwsMode(NWS_MODE_CDMALTE);
    }
}

void RpIrStrategyCommon::onSetIfEnabled(bool enabled) {
    RFX_UNUSED(enabled);
    updateWatchdog();
}

int RpIrStrategyCommon::getHomeMcc() {
    if (m_IrController->isCtSimCard() || RpIrUtils::isViceOfTwoCtDualVolte(getSlotId())) {
        // Heji card mcc is 454, but as CT spec, we should
        // process it as CT card, so can't only check mcc
        // Vice card of Two CT dual volte can not get C2K IMSI
        // use CT MCC directly.
        return CHINA_TELECOM_MAINLAND_MCC;
    }
    String8 home_imsi = getStatusManager()->getString8Value(RFX_STATUS_KEY_C2K_IMSI);
    if (home_imsi == String8("") && NWS_MODE_CSFB == m_IrController->getNwsMode()
            && getStatusManager()->getBoolValue(RFX_STATUS_KEY_CT3G_DUALMODE_CARD)) {
        /*********************************
        * For CDMA 3G dual mode card, RFX_STATUS_KEY_C2K_IMSI and RFX_STATUS_KEY_CDMA_CARD_TYPE
        * will be cleared in CSFB mode, and keep no value until MD3 reports them.
        * However, IR will stay in CSFB mode but never power on MD3 when they contain no value.
        * This invited a dead lock.
        * For this case, IR will record the last valid C2K IMSI and CDMA card type by itself, and
        * use last valid values in CSFB mode.
        **********************************/
        home_imsi = m_IrController->getLastValidImsi();
    }
    if (home_imsi == String8("")) {
        logD(RFX_LOG_TAG, "No C2K IMSI");
        return 0;
    }
    if (home_imsi.length() < 3) {
        logD(RFX_LOG_TAG, "Invalid C2K IMSI %s", home_imsi.string());
        RFX_ASSERT(0);
        return 0;
    }
    const char *imsi = home_imsi.string();
    char home_mcc_str[4];
    home_mcc_str[0] = imsi[0];
    home_mcc_str[1] = imsi[1];
    home_mcc_str[2] = imsi[2];
    home_mcc_str[3] = 0;
    int home_mcc = atoi(home_mcc_str);
    logD(RFX_LOG_TAG, "Home mcc = %d", home_mcc);
    return home_mcc;
}


NwsMode RpIrStrategyCommon::getNwsModeByPlmn(const String8& plmn) {
    if (!plmn.isEmpty()) {
        // For 5m project
        int mcc = atoi(string(plmn.string()).substr(0, 3).c_str());
        int home_mcc = getHomeMcc();
        logD(RFX_LOG_TAG, "getNwsModeByPlmn, home_mcc = %d, mcc=%d, is_sim_ready = %s",
                          home_mcc, mcc, Bool2Str(RpIrUtils::isSimReady(getSlotId())));
        if (home_mcc == 0) {
            // if bootup in CSFB mode, can't get C2K IMSI, so keep CSFB mode,
            // otherwise, it's in CDMALTE mode, should be able to get C2K IMSI
            logD(RFX_LOG_TAG, "getNwsModeByPlmn, plmn=[%s]  ret: [NWS_MODE_CSFB]", plmn.string());
            return NWS_MODE_CSFB;
        }
        // For CT special case
        if ((mcc == CHINA_TELECOM_MAINLAND_MCC || mcc == CHINA_TELECOM_MACAU_MCC) &&
                home_mcc == CHINA_TELECOM_MAINLAND_MCC) {
            if (RpIrUtils::isViceOfTwoCtDualVolte(getSlotId())) {
                // Dual volte C+C, C is vice card: volte on -> LTE only, volte off -> LTE only
                logD(RFX_LOG_TAG, "getNwsModeByPlmn, plmn=[%s], vice of two CT dual volte \
                        ret: [NWS_MODE_LTEONLY]", plmn.string());
                return NWS_MODE_LTEONLY;
            }
            if (m_IrController->getVolteState() == VOLTE_ON) {
                logD(RFX_LOG_TAG, "getNwsModeByPlmn, plmn=[%s], Volte on, ret: [NWS_MODE_LTEONLY]",
                        plmn.string());
                return NWS_MODE_LTEONLY;
            } else {
                logD(RFX_LOG_TAG, "getNwsModeByPlmn, plmn=[%s], Volte off, ret: [NWS_MODE_CDMALTE]",
                        plmn.string());
                return NWS_MODE_CDMALTE;
            }
        }
        bool is_home = RpIrMccTable::isSameCountryCode(home_mcc, mcc);
        logD(RFX_LOG_TAG, "getNwsModeByPlmn, is_home = %s", Bool2Str(is_home));
        if (is_home) {
            logD(RFX_LOG_TAG, "getNwsModeByPlmn, plmn=[%s]  ret: [NWS_MODE_CDMALTE]", plmn.string());
            return NWS_MODE_CDMALTE;
        } else {
            logD(RFX_LOG_TAG, "getNwsModeByPlmn, plmn=[%s]  ret: [NWS_MODE_CSFB]", plmn.string());
            return NWS_MODE_CSFB;
        }
    } else {
        logD(RFX_LOG_TAG, "getNwsModeByPlmn, plmn=[%s]  ret: [NWS_MODE_CSFB]", plmn.string());
        return NWS_MODE_CSFB;
    }
}

bool RpIrStrategyCommon::isDualServiceNotInService() {
    bool ret = (mCdmaServiceState != IN_SERVICE) && (mLwgServiceState != IN_SERVICE);
    logD(RFX_LOG_TAG, "isDualServiceNotInService() : %s", Bool2Str(ret));
    return ret;
}

bool RpIrStrategyCommon::isDualRadioOff() {
    bool dualRadioOff = !m_IsLwgRadioOn && !m_IsCdmaRadioOn;
    bool reqRadioPower = getStatusManager()->getBoolValue(RFX_STATUS_KEY_REQUEST_RADIO_POWER);
    bool ret = dualRadioOff || !reqRadioPower;
    logD(RFX_LOG_TAG, "isDualRadioOff() LwgRadioOn=%s, CdmaRadioOn=%s, reqRadioPower=%s, ret=%s",
                       Bool2Str(m_IsLwgRadioOn), Bool2Str(m_IsCdmaRadioOn),
                       Bool2Str(reqRadioPower), Bool2Str(ret));
    return ret;
}

bool RpIrStrategyCommon::switchForNoService(bool forceSwitch) {
    logD(RFX_LOG_TAG, "switchForNoService mLwgServiceState: %s, mCdmaServiceState: %s",
                      ServiceType2Str(mLwgServiceState), ServiceType2Str(mCdmaServiceState));
    bool reallySwitchForNoService = false;
    int state = getNonSlotScopeStatusManager()->getIntValue(RFX_STATUS_KEY_WORLD_MODE_STATE, 1);
    if (state == 0) {
        logD(RFX_LOG_TAG, "switchForNoService, world mode is switching, return.");
        return reallySwitchForNoService;
    }

    bool ready = RpIrUtils::isSimReady(getSlotId());
    bool is3G = getStatusManager()->getBoolValue(RFX_STATUS_KEY_CT3G_DUALMODE_CARD);
    if (is3G) {
        RfxNwServiceState cdmaServiceState
                = getStatusManager()->getServiceStateValue(RFX_STATUS_KEY_SERVICE_STATE);
        if (RpIrUtils::isSimLocked(getSlotId())) {
            logD(RFX_LOG_TAG, "switchForNoService, PIN locked");
            ready = true;
        } else if (getStatusManager()->getIntValue(RFX_STATUS_KEY_CDMA_CARD_TYPE, -1) == -1
                && cdmaServiceState.getCdmaNetworkExist()
                && m_IrController->getNwsMode() == NWS_MODE_CDMALTE) {
            // After switching to home, MD may immediately report limited service because card
            // is not ready yet. Ignore it to not switch back to roaming in this case.
            logD(RFX_LOG_TAG, "switchForNoService, limited service");
            ready = false;
        }
    }
    if (!isDualRadioOff() && RpIrUtils::isSupportRoaming(getSlotId()) && ready) {
        nsecs_t curTime = systemTime();

        nsecs_t duration = curTime - mNoServiceTimeStamp;
        stopNoServiceWatchdog();

        // prevent no service switch happens too
        // frequently, use a time stamp
        if (mNoServiceTimeStamp == 0 || duration > ms2ns(sWatchdogDelayTime) || forceSwitch) {
            logD(RFX_LOG_TAG, "switchForNoService really switched");
            sp<RfxAction> action = new RfxAction0(this, &RpIrStrategyCommon::onActionNwsModeChanged);

            if (m_IrController->getNwsMode() == NWS_MODE_CSFB) {
                NwsMode targetMode = NWS_MODE_CDMALTE;
                if (RpIrUtils::isViceOfTwoCtDualVolte(getSlotId())) {
                    // Dual volte C+C, C is vice card: volte on -> LTE only, volte off -> LTE only
                    targetMode = NWS_MODE_LTEONLY;
                } else {
                    targetMode = m_IrController->getVolteState() ? NWS_MODE_LTEONLY : NWS_MODE_CDMALTE;
                }
                m_IrController->setNwsMode(targetMode, action);
            } else {
                m_IrController->setNwsMode(NWS_MODE_CSFB, action);
            }

            mNoServiceTimeStamp = curTime;

            mCdmaNwsMode = NWS_MODE_UNKNOWN;  // clear cdma record
            reallySwitchForNoService = true;
        } else {
            logD(RFX_LOG_TAG, "switchForNoService delay switch, sWatchdogDelayTime=%dms, duration=%dms",
                           sWatchdogDelayTime, (int)ns2ms(duration));
            RfxTimer::stop(m_timer_handle_watchDog);
            m_timer_handle_watchDog = RfxTimer::start(RfxCallback0(this,&RpIrStrategyCommon::triggerNoServiceWatchdog),
                    ms2ns(sWatchdogDelayTime) - duration);
            m_timer_handle_watchDog_started = true;
        }
    }
    logD(RFX_LOG_TAG, "switchForNoService reallySwitchForNoService: %s",
                      Bool2Str(reallySwitchForNoService));
    return reallySwitchForNoService;
}

void RpIrStrategyCommon::updateWatchdog() {
    if (mCdmaServiceState == IN_SERVICE ||
        mLwgServiceState == IN_SERVICE ||
        isDualRadioOff()) {
        mContinousRetryCount = 0;
    }

    if (!isDualRadioOff() && isDualServiceNotInService() && getIfEnabled()) {
        startNoServiceWatchdog();
    } else {
        stopNoServiceWatchdog();
    }
}

void RpIrStrategyCommon::onRadioStateChanged() {
    NwsMode nwsMode = m_IrController->getNwsMode();

    if (isDualRadioOff()) {
        mCdmaNwsMode = NWS_MODE_UNKNOWN; // clear cdma record
    }

    updateWatchdog();
}

void RpIrStrategyCommon::onActionNwsModeChanged() {
    m_LwgController->startNewSearchRound();
    m_CdmaController->startNewSearchRound();
}

void RpIrStrategyCommon::triggerNoServiceWatchdog() {
    logD(RFX_LOG_TAG, "triggerNoServiceWatchdog mLwgServiceState: %s mCdmaServiceState: %s",
                   ServiceType2Str(mLwgServiceState), ServiceType2Str(mCdmaServiceState));
    if (isDualServiceNotInService()) {
        switchForNoService(false);
    }
    updateWatchdog();
}

void RpIrStrategyCommon::startNoServiceWatchdog() {
    logD(RFX_LOG_TAG, "startNoServiceWatchdog");
    if (!m_timer_handle_watchDog_started) {
        char strProp[PROPERTY_VALUE_MAX] = {0};
        char strTemp[20] = {0};

        sprintf(strTemp, "%d", NO_SEERVICE_WATCHDOG_DELAY_TIME);
        memset(strProp, 0, sizeof(char)*PROPERTY_VALUE_MAX);
        property_get("persist.vendor.sys.ct.ir.wd", strProp, strTemp);
        RpIrStrategyCommon::sWatchdogDelayTime = atoi(strProp);

        sWatchdogDelayTime += (mContinousRetryCount * WATCHDOG_RETRY_DELAY_STEP);
        if (sWatchdogDelayTime > MAX_WATCHDOG_RETRY_DELAY) {
            sWatchdogDelayTime = MAX_WATCHDOG_RETRY_DELAY;
        }
        mWatchdogStartTime = systemTime();
        logD(RFX_LOG_TAG, "really start watchdog sWatchdogDelayTime = %dms, mContinousRetryCount = %d",
                        sWatchdogDelayTime, mContinousRetryCount);
        m_timer_handle_watchDog = RfxTimer::start(RfxCallback0(this, &RpIrStrategyCommon::triggerNoServiceWatchdog),
                                                  ms2ns(sWatchdogDelayTime));
        m_timer_handle_watchDog_started = true;
        mContinousRetryCount++;
    }
}

void RpIrStrategyCommon::stopNoServiceWatchdog() {
    logD(RFX_LOG_TAG, "stopNoServiceWatchdog");
    mWatchdogStartTime = 0;
    RfxTimer::stop(m_timer_handle_watchDog);
    m_timer_handle_watchDog = NULL;
    m_timer_handle_watchDog_started = false;
}

void RpIrStrategyCommon::postponeNoServiceWatchdogIfNeeded() {
    logD(RFX_LOG_TAG, "postponeNoServiceWatchdogIfNeeded");
    if (m_timer_handle_watchDog_started) {
        RfxTimer::stop(m_timer_handle_watchDog);
        nsecs_t remainingTime = ms2ns(sWatchdogDelayTime) -
                             (systemTime() - mWatchdogStartTime);
        if (remainingTime < 0) {
            remainingTime = 0;
        }
        nsecs_t newDelay = remainingTime + ms2ns(sSwitchModeOrResumeDelayTime);
        logD(RFX_LOG_TAG, "remainingTime = %dms newDelay = %dms", (int)ns2ms(remainingTime), (int)ns2ms(newDelay));
        m_timer_handle_watchDog = RfxTimer::start(RfxCallback0(this,
                &RpIrStrategyCommon::triggerNoServiceWatchdog), newDelay);
    }
}

void RpIrStrategyCommon::restartNoSerivceWatchdogIfNeeded() {
    logD(RFX_LOG_TAG, "restartNoSerivceWatchdogIfNeeded");
    if (m_timer_handle_watchDog_started) {
        stopNoServiceWatchdog();
        if (mContinousRetryCount > 0) {
            mContinousRetryCount--;  // restarting need revert count
        }
        startNoServiceWatchdog();
    }
}

void RpIrStrategyCommon::setLwgServiceState(ServiceType serviceType) {
    mLwgServiceState = serviceType;
    if (serviceType == IN_SERVICE) {
        mLastInServiceNwsMode = m_IrController->getNwRatController()->getNwsMode();
        mLastInServiceIccId = getIccId();
    }
}

void RpIrStrategyCommon::setCdmaServiceState(ServiceType serviceType) {
    mCdmaServiceState = serviceType;
    if (serviceType == IN_SERVICE) {
        mLastInServiceNwsMode = m_IrController->getNwRatController()->getNwsMode();
        mLastInServiceIccId = getIccId();
    }
}

String8 RpIrStrategyCommon::getIccId() {
    const char *PROPERTY_ICCID_SIM[] = {
        "vendor.ril.iccid.sim1",
        "vendor.ril.iccid.sim2",
        "vendor.ril.iccid.sim3",
        "vendor.ril.iccid.sim4",
    };
    char prop[PROPERTY_VALUE_MAX] = {0};
    unsigned int slotId = getSlotId();
    if (slotId < sizeof(PROPERTY_ICCID_SIM) / sizeof(const char *)) {
        property_get(PROPERTY_ICCID_SIM[slotId], prop, "");
    }
    return String8(prop);
}

void RpIrStrategyCommon::onRatSwitchStart(const int newPrefNwType, const NwsMode newNwsMode,
        const RatSwitchCaller ratSwitchCaller) {
    // After card inited, if we find that we ever been in roaming with the same card,
    // force switch to roaming to speed up camping on network.
    if (RpIrUtils::is3GPrefNwType(newPrefNwType)
            && ratSwitchCaller == RAT_SWITCH_INIT
            && mLastInServiceNwsMode == NWS_MODE_CSFB
            && newNwsMode == NWS_MODE_CDMALTE
            && getIccId() == mLastInServiceIccId) {
        logD(RFX_LOG_TAG, "force to switch to Roaming mode");
        sp<RfxAction> action = new RfxAction0(this, &RpIrStrategyCommon::onActionNwsModeChanged);
        m_IrController->setNwsMode(NWS_MODE_CSFB, action);
    }
}

void RpIrStrategyCommon::onRatSwitchDone(int prePrefNwType, int curPrefNwType) {
    logD(RFX_LOG_TAG, "onRatSwitchDone, prePrefNwType=%d, curPrefNwType=%d", prePrefNwType, curPrefNwType);
    if (getIfEnabled() &&
        !isDualRadioOff() &&
        isDualServiceNotInService() &&
        prePrefNwType == PREF_NET_TYPE_LTE_ONLY &&
        curPrefNwType != PREF_NET_TYPE_LTE_ONLY &&
        m_IrController->getNwsMode() != NWS_MODE_CDMALTE) {
        // if turned off TDD DATA only mode, always search from Home mode
        logD(RFX_LOG_TAG, "force to switch to Home mode");
        switchForNoService(true);
        updateWatchdog();
    }
}

void RpIrStrategyCommon::onWwopRatSwitchDone(int prePrefNwType, int curPrefNwType) {
    logD(RFX_LOG_TAG, "onWwopRatSwitchDone, prePrefNwType=%d, curPrefNwType=%d", prePrefNwType, curPrefNwType);
    if (m_IrController->isWwop() &&
            (curPrefNwType == PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO ||
            curPrefNwType == PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA) &&
            prePrefNwType != PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO &&
            prePrefNwType != PREF_NET_TYPE_LTE_CMDA_EVDO_GSM_WCDMA &&
            prePrefNwType != -1) {  // unknown
        char strProp[PROPERTY_VALUE_MAX] = {0};
        property_get("gsm.operator.numeric", strProp, "");
        logD(RFX_LOG_TAG, "onWwopRatSwitchDone, gsm.operator.numeric=%s", strProp);
        int slotId = getSlotId();
        int i = 0;
        char* aPlmn = strtok(strProp, ",");
        while (i != slotId && aPlmn != NULL) {
            aPlmn = strtok(NULL, ",");
            i++;
        }
        if (i != slotId || aPlmn == NULL) {
            // this should not happen
            logD(RFX_LOG_TAG, "onWwopRatSwitchDone, i=%d, slotId=%d, return.", i, slotId);
            return;
        }
        // force to switch NWS mdoe in no service when RAT switch to non-global from global mode for Wwop project
        // Or get register plmn to get the NWS mode.
        if (strlen(aPlmn) != 0 && getHomeMcc() != 0) {
            String8 plmn;
            plmn.append(aPlmn);
            NwsMode targetMode = getNwsModeByPlmn(plmn);
            logD(RFX_LOG_TAG, "Switch NWS mode:%s plmn:%s", Nws2Str(targetMode), plmn.string());
            sp<RfxAction> action = new RfxAction0(this, &RpIrStrategyCommon::onActionNwsModeChanged);
            m_IrController->getNwRatController()->setNwsMode(targetMode, action);
        }
    }
}
