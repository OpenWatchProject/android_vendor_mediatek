 /*
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
#include "RfxRilUtils.h"
#include "RfxStatusManager.h"
#include "RfxRootController.h"
#include "RfxMclMessage.h"
#include "RfxMessage.h"
#include "RfxDispatchThread.h"
#include <libmtkrilutils.h>
#include "utils/String8.h"
#include <mtkconfigutils.h>
#ifdef HAVE_AEE_FEATURE
#include "aee.h"
#endif
#include <unistd.h>

#define RFX_LOG_TAG "RfxRilUtils"

#define PROP_NAME_OPERATOR_ID_SIM1 "persist.radio.sim.opid"
#define PROP_NAME_OPERATOR_ID_SIM2 "persist.radio.sim.opid_1"
#define PROP_NAME_OPERATOR_ID_SIM3 "persist.radio.sim.opid_2"
#define PROP_NAME_OPERATOR_ID_SIM4 "persist.radio.sim.opid_3"

int RfxRilUtils::m_isEngLoad = -1;
int RfxRilUtils::m_isChipTest = -1;
int RfxRilUtils::m_isInternalLoad = -1;
int RfxRilUtils::m_isUserLoad = -1;
int RfxRilUtils::mIsC2kSupport = -1;
int RfxRilUtils::mIsLteSupport = -1;
int RfxRilUtils::mIsImsSupport = -1;
int RfxRilUtils::mIsMultiIms = -1;
int RfxRilUtils::mIsRsimAuthOngoing[MAX_SIM_COUNT] = {-1,-1,-1,-1};

/// M: add for op09 volte setting @{
int RfxRilUtils::mIsOp09 = -1;
int RfxRilUtils::mIsCtVolteSupport = -1;
int RfxRilUtils::mIsCtMixVolteSupport = -1;
/// @}

RilRunMode RfxRilUtils::m_rilRunMode = RilRunMode::RIL_RUN_MODE_NORMORL;
STATUSCALLBACK RfxRilUtils::s_statusCallback = NULL;

static const RfxStatusKeyEnum mHideStatusKeyLog[] = {
    RFX_STATUS_KEY_SIM_ICCID,
    RFX_STATUS_KEY_GSM_IMSI,
    RFX_STATUS_KEY_C2K_IMSI,
    RFX_STATUS_KEY_XUI_INFO,
    RFX_STATUS_KEY_IMS_REG_IND_URI
};

int RfxRilUtils::rfxGetSimCount() {
    return getSimCount();
}

int RfxRilUtils::isEngLoad() {
    if (RfxRilUtils::m_isEngLoad == -1) {
        char property_value[RFX_PROPERTY_VALUE_MAX] = { 0 };
        rfx_property_get("ro.build.type", property_value, "");
        RfxRilUtils::m_isEngLoad = (strcmp("eng", property_value) == 0);
    }
    return RfxRilUtils::m_isEngLoad;
}

int RfxRilUtils::isUserLoad() {
    if (RfxRilUtils::m_isUserLoad == -1) {
        char property_value_emulation[RFX_PROPERTY_VALUE_MAX] = { 0 };
        char property_value[RFX_PROPERTY_VALUE_MAX] = { 0 };
        rfx_property_get("vendor.ril.emulation.userload", property_value_emulation, "0");
        if(strcmp("1", property_value_emulation) == 0) {
            return 1;
        }
        rfx_property_get("ro.build.type", property_value, "");
        RfxRilUtils::m_isUserLoad = (strcmp("user", property_value) == 0);
    }
    return RfxRilUtils::m_isUserLoad;
}

int RfxRilUtils::isLteSupport() {
    if (RfxRilUtils::mIsLteSupport == -1) {
        char tempstr[RFX_PROPERTY_VALUE_MAX];
        memset(tempstr, 0, sizeof(tempstr));
        rfx_property_get("ro.vendor.mtk_lte_support", tempstr, "0");
        RfxRilUtils::mIsLteSupport = atoi(tempstr);
    }
    return RfxRilUtils::mIsLteSupport;
}

int RfxRilUtils::isImsSupport() {
    if (RfxRilUtils::mIsImsSupport == -1) {
        char tempstr[RFX_PROPERTY_VALUE_MAX];
        memset(tempstr, 0, sizeof(tempstr));
        rfx_property_get("persist.vendor.ims_support", tempstr, "0");
        RfxRilUtils::mIsImsSupport = atoi(tempstr);
    }
    return RfxRilUtils::mIsImsSupport;
}

int RfxRilUtils::isMultipleImsSupport() {
    if (RfxRilUtils::mIsMultiIms == -1) {
        char tempstr[RFX_PROPERTY_VALUE_MAX] = { 0 };
        rfx_property_get("persist.vendor.mims_support", tempstr, "0");
        RfxRilUtils::mIsMultiIms = atoi(tempstr);
    }
    return (RfxRilUtils::mIsMultiIms > 1) ? 1 : 0;
}

int RfxRilUtils::triggerCCCIIoctlEx(int request, int *param) {
    int ret_ioctl_val = -1;
    int ccci_sys_fd = -1;
    char dev_node[32] = {0};
    int enableMd1 = 0;
    char prop_value[RFX_PROPERTY_VALUE_MAX] = { 0 };
    int retryCount = 0;

    rfx_property_get("ro.vendor.mtk_md1_support", prop_value, "0");
    enableMd1 = atoi(prop_value);

#if defined(PURE_AP_USE_EXTERNAL_MODEM)
    RFX_LOG_D(RFX_LOG_TAG, "Open CCCI MD1 ioctl port[%s]",CCCI_MD1_POWER_IOCTL_PORT);
    ccci_sys_fd = open(CCCI_MD1_POWER_IOCTL_PORT, O_RDWR);
    while (ccci_sys_fd < 0 && retryCount < MAX_RETRY_COUNT) {
        RFX_LOG_E(RFX_LOG_TAG, "Open CCCI ioctl port failed [%d], retrying...", ccci_sys_fd);
        usleep(100*1000);
        retryCount++;
        ccci_sys_fd = open(CCCI_MD1_POWER_IOCTL_PORT, O_RDWR);
    }
    if (ccci_sys_fd < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "Exceed retry count and still open fail, return");
        return -1;
    }
#else
    snprintf(dev_node, 32, "%s", ccci_get_node_name(USR_RILD_IOCTL, MD_SYS1));
    RFX_LOG_D(RFX_LOG_TAG, "MD1/SYS1 IOCTL [%s, %d]", dev_node, request);
    ccci_sys_fd = open(dev_node, O_RDWR | O_NONBLOCK);
    while (ccci_sys_fd < 0 && retryCount < MAX_RETRY_COUNT) {
        RFX_LOG_E(RFX_LOG_TAG, "Open CCCI ioctl port failed [%d], retrying...", ccci_sys_fd);
        usleep(100*1000);
        retryCount++;
        ccci_sys_fd = open(dev_node, O_RDWR | O_NONBLOCK);
    }
    if (ccci_sys_fd < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "Exceed retry count and still open fail, return");
        return -1;
    }
#endif

#if defined(PURE_AP_USE_EXTERNAL_MODEM)
    if(request == CCCI_IOC_ENTER_DEEP_FLIGHT) {
        int pid = findPid("vendor.gsm0710muxd");
        RFX_LOG_D(RFX_LOG_TAG, "MUXD pid=%d",pid);
        if(pid != -1) kill(pid,SIGUSR2);
        RFX_LOG_D(RFX_LOG_TAG, "send SIGUSR2 to MUXD done");
        sleepMsec(100);    // make sure MUXD have enough time to close channel and FD
    }
#endif

    ret_ioctl_val = ioctl(ccci_sys_fd, request, param);
    if (ret_ioctl_val < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "CCCI ioctl result: ret_val=%d, request=%d, param=%d",
                ret_ioctl_val, request, *param);
    } else {
        RFX_LOG_D(RFX_LOG_TAG, "CCCI ioctl result: ret_val=%d, request=%d, param=%d",
                ret_ioctl_val, request, *param);
    }

    int ret = close(ccci_sys_fd);
    if (ret < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "close ccci node fail: %s (%d)", strerror(errno), errno);
    }
    return ret_ioctl_val;
}

int RfxRilUtils::triggerCCCIIoctl(int request) {
    int param = -1;
    int ret_ioctl_val;

    ret_ioctl_val = triggerCCCIIoctlEx(request, &param);

    return ret_ioctl_val;
}

char RfxRilUtils::convertCharToHex(char ch)
{
    char returnType;
    switch(ch)
    {
        case '0':
            returnType = 0;
            break;
        case '1' :
            returnType = 1;
            break;
        case '2':
            returnType = 2;
            break;
        case '3':
            returnType = 3;
            break;
        case '4' :
            returnType = 4;
            break;
        case '5':
            returnType = 5;
            break;
        case '6':
            returnType = 6;
            break;
        case '7':
            returnType = 7;
            break;
        case '8':
            returnType = 8;
            break;
        case '9':
            returnType = 9;
            break;
        case 'A':
        case 'a':
            returnType = 10;
            break;
        case 'B':
        case 'b':
            returnType = 11;
            break;
        case 'C':
        case 'c':
            returnType = 12;
            break;
        case 'D':
        case 'd':
            returnType = 13;
            break;
        case 'E':
        case 'e':
            returnType = 14;
            break;
        case 'F':
        case 'f':
            returnType = 15;
            break;
        default:
            returnType = 0;
            break;
    }
    return returnType;
}

int RfxRilUtils::triggerPhantomPacket(String8 s) {
    int fd;
    uint32_t ret = 0;
    int local_errno = 0;

    char mode4_msg[MNGMT_PACKET_LENGTH+1];

    if (s.size() %2 != 0 || s.size() > MNGMT_PACKET_LENGTH) return -1;

    for (std::size_t i = 0; i != s.size()/2; ++i) {
        mode4_msg[i] = 16 * convertCharToHex(s[2*i]) + convertCharToHex(s[2*i+1]);
    }

    RFX_LOG_D(RFX_LOG_TAG, "Phantom pkt content(%zu): %s", s.size()/2, mode4_msg);

    fd = open("/dev/ttyC6", O_RDWR);
    if (fd < 0) {
        RFX_LOG_E(RFX_LOG_TAG, "Fail to open /dev/ttyC6: %d", errno);
        return -1;
    } else {
        RFX_LOG_D(RFX_LOG_TAG, "/dev/ttyC6 is opened.");
    }

    ret = write(fd, mode4_msg, s.size()/2);
    if (ret != s.size()/2) {
        local_errno = errno;
        RFX_LOG_E(RFX_LOG_TAG, "Failed to write phantom pkt(%zu)!! (%d/%d)",
                s.size()/2, ret, local_errno);
        close(fd);
        return -1;
    }
    RFX_LOG_D(RFX_LOG_TAG, "Write %d bytes to /dev/ttyC6", ret);
    close(fd);
    return 0;
}

RilRunMode RfxRilUtils::getRilRunMode() {
    return m_rilRunMode;
}

void RfxRilUtils::setRilRunMode(RilRunMode mode) {
    m_rilRunMode = mode;
}

void RfxRilUtils::setStatusValueForGT(int slotId, const RfxStatusKeyEnum key, const RfxVariant &value) {
    RFX_LOG_D(RFX_LOG_TAG, "setStatusValueForGT, updateValueMdComm, slot_id = %d, key = %s, value = %s", slotId,
            RfxStatusManager::getKeyString(key), value.toString().string());
    sp<RfxMessage> msg = RfxMessage::obtainStatusSync(slotId, key, value, false, false, true);
    RFX_OBJ_GET_INSTANCE(RfxRilAdapter)->requestToMcl(msg);

    RFX_LOG_D(RFX_LOG_TAG, "setStatusValueForGT, updateValueToTelCore, slot_id = %d, key = %s, value = %s", slotId,
            RfxStatusManager::getKeyString(key), value.toString().string());
    sp<RfxMclMessage> msgToTcl = RfxMclMessage::obtainStatusSync(slotId, key, value, false, false, true);
    RfxDispatchThread::enqueueStatusSyncMessage(msgToTcl);
}

void RfxRilUtils::setStatusCallbackForGT(STATUSCALLBACK statusCallback) {
    s_statusCallback = statusCallback;
}

void RfxRilUtils::updateStatusToGT(int slotId, const RfxStatusKeyEnum key, const RfxVariant &value) {
    if (s_statusCallback != NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "updateStatusToGT is not null");
        s_statusCallback(slotId, key, value);
    } else {
        RFX_LOG_E(RFX_LOG_TAG, "updateStatusToGT is NULL");
    }
}

/// M: add for op09 volte setting @{
bool RfxRilUtils::isOp09() {
    if (mIsOp09 == -1) {
        char optrStr[RFX_PROPERTY_VALUE_MAX] = { 0 };
        rfx_property_get("persist.vendor.operator.optr", optrStr, "");
        if (strncmp(optrStr, "OP09", 4) == 0) {
            mIsOp09 = 1;
        } else {
            mIsOp09 = 0;
        }
    }
    return (mIsOp09 == 1);
}

bool RfxRilUtils::isCtVolteSupport() {
    if (mIsCtVolteSupport == -1) {
        char ctStr[RFX_PROPERTY_VALUE_MAX] = { 0 };
        rfx_property_get("persist.vendor.mtk_ct_volte_support", ctStr, "");
        if (strcmp(ctStr, "1") == 0) {
            mIsCtVolteSupport = 1;
        } else {
            mIsCtVolteSupport = 0;
        }
    }
    return (mIsCtVolteSupport == 1);
}

bool RfxRilUtils::isCtMixVolteSupport() {
    if (mIsCtMixVolteSupport == -1) {
        char ctStr[RFX_PROPERTY_VALUE_MAX] = { 0 };
        rfx_property_get("persist.vendor.mtk_ct_volte_support", ctStr, "");
        if (strcmp(ctStr, "2") == 0 || strcmp(ctStr, "3") == 0) {
            mIsCtMixVolteSupport = 1;
        } else {
            mIsCtMixVolteSupport = 0;
        }
    }
    return (mIsCtMixVolteSupport == 1);
}
/// @}

int RfxRilUtils::getMajorSim() {
    char tmp[RFX_PROPERTY_VALUE_MAX] = { 0 };
    int simId = 0;

    rfx_property_get("persist.vendor.radio.simswitch", tmp, "1");
    simId = atoi(tmp);
    RFX_LOG_D(RFX_LOG_TAG, "getMajorSim, simId=%d", simId);
    return simId;
}

void RfxRilUtils::printLog(int level, String8 tag, String8 log, int slot) {
    switch (level) {
        case VERBOSE:
            RFX_LOG_V(tag.string(), "[%d] %s", slot, log.string());
            break;
        case DEBUG:
            RFX_LOG_D(tag.string(), "[%d] %s", slot, log.string());
            break;
        case INFO:
            RFX_LOG_I(tag.string(), "[%d] %s", slot, log.string());
            break;
        case WARN:
            RFX_LOG_W(tag.string(), "[%d] %s", slot, log.string());
            break;
        case ERROR:
            RFX_LOG_E(tag.string(), "[%d] %s", slot, log.string());
            break;
        default:
            RFX_LOG_E(tag.string(), "undefine log level: %s", log.string());
    }
}

bool RfxRilUtils::isInLogReductionList(int reqId) {
    const int logReductionRequest[] = {
        RFX_MSG_REQUEST_SIM_IO,
        RFX_MSG_REQUEST_READ_EMAIL_ENTRY,
        RFX_MSG_REQUEST_READ_SNE_ENTRY,
        RFX_MSG_REQUEST_READ_ANR_ENTRY,
        RFX_MSG_REQUEST_READ_UPB_GRP,
        RFX_MSG_REQUEST_QUERY_PHB_STORAGE_INFO,
        RFX_MSG_REQUEST_WRITE_PHB_ENTRY,
        RFX_MSG_REQUEST_READ_PHB_ENTRY,
        RFX_MSG_REQUEST_QUERY_UPB_CAPABILITY,
        RFX_MSG_REQUEST_EDIT_UPB_ENTRY,
        RFX_MSG_REQUEST_DELETE_UPB_ENTRY,
        RFX_MSG_REQUEST_READ_UPB_GAS_LIST,
        RFX_MSG_REQUEST_WRITE_UPB_GRP,
        RFX_MSG_REQUEST_QUERY_UPB_AVAILABLE,
        RFX_MSG_REQUEST_READ_UPB_AAS_LIST,
        RFX_MSG_REQUEST_GSM_SMS_BROADCAST_ACTIVATION,
        RFX_MSG_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION,
        RFX_MSG_REQUEST_DEACTIVATE_DATA_CALL,
        RFX_MSG_REQUEST_SYNC_DATA_SETTINGS_TO_MD,
        RFX_MSG_REQUEST_ALLOW_DATA,
        RFX_MSG_REQUEST_SETUP_DATA_CALL,
        RFX_MSG_REQUEST_RESET_MD_DATA_RETRY_COUNT,
        RFX_MSG_REQUEST_GSM_SMS_BROADCAST_ACTIVATION,
        RFX_MSG_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION,
    };

    size_t count = sizeof(logReductionRequest)/sizeof(int);
    for (size_t i = 0; i < count; i++) {
        if (reqId == logReductionRequest[i]) {
            return true;
        }
    }
    return false;
}

int RfxRilUtils::handleAee(const char *modem_warning, const char *modem_version) {
#ifdef HAVE_AEE_FEATURE
    return aee_modem_warning("Modem Warning", NULL, DB_OPT_DUMMY_DUMP, modem_warning,
            modem_version);
#else
    RFX_UNUSED(modem_warning);
    RFX_UNUSED(modem_version);
    RFX_LOG_D(RFX_LOG_TAG, "[handleOemUnsolicited]HAVE_AEE_FEATURE is not defined");
    return 1;
#endif
}

// External SIM [Start]
#define PROPERTY_MODEM_VSIM_CAPABILITYY "vendor.gsm.modem.vsim.capability"
#define MODEM_VSIM_CAPABILITYY_EANBLE 0x01
#define MODEM_VSIM_CAPABILITYY_HOTSWAP 0x02

void RfxRilUtils::setRsimAuthOngoing(int slot, int ongoing) {
    RfxRilUtils::mIsRsimAuthOngoing[slot] = ongoing;
    RFX_LOG_D(RFX_LOG_TAG, "[VSIM] setRsimAuthOngoing rid:%d is %d.", slot, mIsRsimAuthOngoing[slot]);
}

int RfxRilUtils::getRsimAuthOngoing(int slot) {
    return RfxRilUtils::mIsRsimAuthOngoing[slot];
}

int RfxRilUtils::isExternalSimSupport() {
    char property_value[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("ro.vendor.mtk_external_sim_support", property_value, "0");
    return atoi(property_value);
}

int RfxRilUtils::isExternalSimOnlySlot(int slot) {
    char property_value[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("ro.vendor.mtk_external_sim_only_slots", property_value, "0");
    int supported = atoi(property_value) & (1 << slot);

    RFX_LOG_D(RFX_LOG_TAG, "[isExternalSimOnlySlot] vsimOnlySlots:%d, supported:%d",
            atoi(property_value), supported);
    return ((supported > 0) ? 1 : 0);
}

int RfxRilUtils::isPersistExternalSimDisabled() {
    char property_value[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("ro.vendor.mtk_persist_vsim_disabled", property_value, "0");
    return atoi(property_value);
}

int RfxRilUtils::isNonDsdaRemoteSupport() {
    char property_value[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("ro.vendor.mtk_non_dsda_rsim_support", property_value, "0");
    return atoi(property_value);
}

int RfxRilUtils::isSwitchVsimWithHotSwap() {
    int enabled = 0;
    char vsim_hotswap[RFX_PROPERTY_VALUE_MAX] = {0};

    for (int index = 0; index < RfxRilUtils::rfxGetSimCount(); index++) {
        getMSimProperty(index, (char*)PROPERTY_MODEM_VSIM_CAPABILITYY, vsim_hotswap);
        if ((atoi(vsim_hotswap) & MODEM_VSIM_CAPABILITYY_HOTSWAP) > 1) {
            enabled = 1;
            break;
        }
    }

    RFX_LOG_D(RFX_LOG_TAG, "[VSIM] isSwitchVsimWithHotSwap: %d.", enabled);

    return enabled;
}

int RfxRilUtils::isVsimEnabledBySlot(int slot) {
    int enabled = 0;
    char vsim_enabled_prop[RFX_PROPERTY_VALUE_MAX] = {0};
    char vsim_inserted_prop[RFX_PROPERTY_VALUE_MAX] = {0};

    getMSimProperty(slot, (char*)"vendor.gsm.external.sim.enabled", vsim_enabled_prop);
    getMSimProperty(slot, (char*)"vendor.gsm.external.sim.inserted", vsim_inserted_prop);

    if ((atoi(vsim_enabled_prop) > 0 && atoi(vsim_inserted_prop) > 0) || isExternalSimOnlySlot(slot)) {
        enabled = 1;
    }

    RFX_LOG_D(RFX_LOG_TAG, "[VSIM] isVsimEnabled slot:%d is %d.", slot, enabled);

    return enabled;
}

int RfxRilUtils::isVsimEnabledNotInserted(int slot) {
    int result = 0;
    char vsim_enabled_prop[RFX_PROPERTY_VALUE_MAX] = {0};
    char vsim_inserted_prop[RFX_PROPERTY_VALUE_MAX] = {0};

    getMSimProperty(slot, (char*)"vendor.gsm.external.sim.enabled", vsim_enabled_prop);
    getMSimProperty(slot, (char*)"vendor.gsm.external.sim.inserted", vsim_inserted_prop);

    if ((atoi(vsim_enabled_prop) > 0 && atoi(vsim_inserted_prop) <= 0)) {
        RFX_LOG_I(RFX_LOG_TAG, "[VSIM] isVsimEnabledNotInserted slot:%d is 1.", slot);
        result = 1;
    }
    return result;
}

bool RfxRilUtils::isVsimEnabled() {
    bool enabled = false;

    for (int index = 0; index < RfxRilUtils::rfxGetSimCount(); index++) {
        if (1 == isVsimEnabledBySlot(index)) {
            enabled = true;
            break;
        }
    }
    RFX_LOG_D(RFX_LOG_TAG, "[VSIM] isVsimEnabled=%d", enabled);
    return enabled;
}

bool RfxRilUtils::isVsimClientConnected() {
    char property_value[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("vendor.gsm.external.sim.connected", property_value, "0");
    return (atoi(property_value) != 0);
}

bool RfxRilUtils::isExternalSimManagerExisted() {
    char property_value[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("vendor.gsm.external.sim.connected", property_value, "-1");
    // "-1" indicates no such property because we assure no place set it to "-1"
    return (atoi(property_value) != -1);
}

int RfxRilUtils::getRemoteSimSlot() {
    char property_value[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("vendor.gsm.prefered.rsim.slot", property_value, "-1");
    return atoi(property_value);
}
// External SIM [End]

bool RfxRilUtils::isTplusWSupport() {
    char tmp[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("vendor.ril.simswitch.tpluswsupport", tmp, "0");
    return (atoi(tmp) != 0);
}

int RfxRilUtils::getKeep3GMode() {
    char tmp[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("vendor.ril.nw.worldmode.keep_3g_mode", tmp, "0");
    return (atoi(tmp));
}

bool RfxRilUtils::isEmciSupport() {
    char tmp[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("vendor.ril.call.emci_support", tmp, "0");
    return (atoi(tmp) != 0);
}

bool RfxRilUtils::isWfcEnable(int slotId) {
    char wfcEnable[MTK_PROPERTY_VALUE_MAX] = { 0 };

    bool wfcSupport = isWfcSupport();
    rfx_property_get("persist.vendor.mtk.wfc.enable", wfcEnable, "0");
    RFX_LOG_D(RFX_LOG_TAG, "isWfcEnable(), slotId: %d, wfcSupport: %d, wfcEnable %s",
            slotId, wfcSupport, wfcEnable);

    if (wfcSupport) {
        /* wfcEnable is a bitmask for VoLTE/ViLTE/WFC. Maximun sim is 4.
         * ex: 0001 means SIM 1 enable, 0101 means SIM 1 and SIM 3 enable
         */
        if (RfxRilUtils::isMultipleImsSupport() == 1) {
            /* Check if the "current SIM's WFC"" has been switch on */
            if (((atoi(wfcEnable) >> slotId) & 0x01) == 1) {
                return true;
            }
        } else {
            /* Only need to check main slot */
            if (atoi(wfcEnable) == 1) {
                return true;
            }
        }
    }
    return false;
}

bool RfxRilUtils::isVilteEnable(int slotId) {
    char vilteEnable[MTK_PROPERTY_VALUE_MAX] = { 0 };

    bool vilteSupport = isVilteSupport();
    rfx_property_get("persist.mtk.vilte.enable", vilteEnable, "0");
    RFX_LOG_D(RFX_LOG_TAG, "isVilteEnable(), slotId: %d, vilteSupport: %d, vilteEnable %s",
            slotId, vilteSupport, vilteEnable);

    if (vilteSupport) {
        /* vilteEnable is a bitmask for VoLTE/ViLTE/WFC. Maximun sim is 4.
         * ex: 0001 means SIM 1 enable, 0101 means SIM 1 and SIM 3 enable
         */
        if (RfxRilUtils::isMultipleImsSupport() == 1) {
            /* Check if the "current SIM's VILTE" has been switch on */
            if (((atoi(vilteEnable) >> slotId) & 0x01) == 1) {
                return true;
            }
        } else {
            /* Only need to check main slot */
            if (atoi(vilteEnable) == 1) {
                return true;
            }
        }
    }
    return false;
}

bool RfxRilUtils::isDigitsSupport() {
    char digitsSupport[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("persist.vendor.mtk_digits_support", digitsSupport, "0");
    return (atoi(digitsSupport) == 1);
}

/*****************************************************************************
 * Utility function
 *****************************************************************************/
const char* RfxRilUtils::boolToString(bool value) {
    return value ? "true" : "false";
}

bool RfxRilUtils::isSmsSupport () {
    FeatureValue feature;
    memset(&feature, 0, sizeof(feature));
    mtkGetFeature(CONFIG_SMS, &feature);
    bool support_sms = (strcmp(feature.value, "1") == 0)? true : false;
    return support_sms;
}

/// M: GSMA TS.27 13.3.7 @{
bool RfxRilUtils::isUiccCLFSupport() {
    char uiccCLF[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("ro.vendor.mtk_uicc_clf", uiccCLF, "0");
    return (atoi(uiccCLF) == 1);
}
/// M: GSMA TS.27 13.3.7 @}

bool RfxRilUtils::isSimSwitchUrc(const char *line) {
    return (strStartsWith(line, "+ESIMMAP:") || strStartsWith(line, "+EPSMAP:"));
}

bool RfxRilUtils::hideStatusLog(const RfxStatusKeyEnum key) {
    for (int i = 0; i < (int) (sizeof(mHideStatusKeyLog) / sizeof(mHideStatusKeyLog[0])); i++) {
        if (key == mHideStatusKeyLog[i]) {
            return true;
        }
    }
    return false;
}

int RfxRilUtils::getProtocolStackId(int slotId) {
    // The major sim is mapped to PS1, the other sims are mapped to PS2~PS3 in ascending order
    int majorSim = getMajorSim();
    if (slotId + 1 == majorSim) {
        return 1;
    } else if (slotId + 1 < majorSim) {
        return slotId + 2;
    } else {
        return slotId + 1;
    }
}

void RfxRilUtils::getLogicalModemId(char *modemId, int size, int slotId) {
    memset(modemId, 0, size);
    sprintf(modemId, "modem_sys%d_ps%d", 1, (getProtocolStackId(slotId) - 1));
}

bool RfxRilUtils::isWfcSupport() {
    FeatureValue feature;
    memset(feature.value, 0, sizeof(feature.value));
    mtkGetFeature(CONFIG_WFC, &feature);
    return strcmp(feature.value, "1") == 0;
}

bool RfxRilUtils::isVilteSupport() {
    FeatureValue feature;
    memset(feature.value, 0, sizeof(feature.value));
    mtkGetFeature(CONFIG_VILTE, &feature);
    return strcmp(feature.value, "1") == 0;
}

bool RfxRilUtils::isVolteSupport() {
    FeatureValue feature;
    memset(feature.value, 0, sizeof(feature.value));
    mtkGetFeature(CONFIG_VOLTE, &feature);
    return strcmp(feature.value, "1") == 0;
}

bool RfxRilUtils::isViwifiSupport() {
    FeatureValue feature;
    memset(feature.value, 0, sizeof(feature.value));
    mtkGetFeature(CONFIG_VIWIFI, &feature);
    return strcmp(feature.value, "1") == 0;
}

bool RfxRilUtils::isGwsdSupport() {
    char gwsd[RFX_PROPERTY_VALUE_MAX] = { 0 };
    rfx_property_get("ro.vendor.mtk_gwsd_support", gwsd, "0");
    return (atoi(gwsd) == 1);
}

const char * RfxRilUtils::pii(const char * tag, const char * hiddenData) {
    static char truncatedData[255] = {0};

    if (hiddenData == NULL) return NULL;

    if (isUserLoad()) {
        truncatedData[0] = '*';
        truncatedData[1] = '*';
        truncatedData[2] = '*';
        truncatedData[3] = '\0';
        return truncatedData;
    }

    char * tagProp = NULL;
    char tagLevel[MTK_PROPERTY_VALUE_MAX] = { 0 };
    asprintf(&tagProp, "persist.log.tag.%s", tag);
    rfx_property_get(tagProp, tagLevel, "I");
    free(tagProp);
    if(strcmp("V", tagLevel) == 0) {  // equal to V
        return hiddenData;
    }

    int i = 0;
    // **
    // 1**456
    // 123***789
    // 123*****90123456
    if (strlen(hiddenData) <= 2) {
        truncatedData[0] = '*';
        truncatedData[1] = '*';
        truncatedData[2] = '\0';
    } else if (strlen(hiddenData) >= 3 && strlen(hiddenData) <= 6) {
        truncatedData[0] = hiddenData[0];
        for (i = 1; i < 3; i++) {
            truncatedData[i] = '*';
        }
        for (i = 3; i < strlen(hiddenData); i++) {
            truncatedData[i] = hiddenData[i];
        }
        truncatedData[i]='\0';
    } else if (strlen(hiddenData) > 6 && strlen(hiddenData) <= 9){
        for (i = 0; i < 3; i++) {
            truncatedData[i] = hiddenData[i];
        }
        truncatedData[3] = '*';
        truncatedData[4] = '*';
        truncatedData[5] = '*';
        for (i = 6; i < strlen(hiddenData); i++) {
            truncatedData[i] = hiddenData[i];
        }
        truncatedData[i]='\0';
    } else if (strlen(hiddenData) > 9){
        for (i = 0; i < 3; i++) {
            truncatedData[i] = hiddenData[i];
        }
        truncatedData[3] = '*';
        truncatedData[4] = '*';
        truncatedData[5] = '*';
        truncatedData[6] = '*';
        truncatedData[7] = '*';
        for (i = 8; i < strlen(hiddenData); i++) {
            truncatedData[i] = hiddenData[i];
        }
        truncatedData[i]='\0';
    }
    return truncatedData;
}
