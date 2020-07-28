/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifndef __RMC_NETWORK_HANDLER_H__
#define __RMC_NETWORK_HANDLER_H__

#undef NDEBUG
#ifdef LOG_NDEBUG
#undef LOG_NDEBUG
#endif

#include "RfxBaseHandler.h"
#include "RfxStringData.h"
#include "RfxStringsData.h"
#include "RfxVoidData.h"
#include "RfxIntsData.h"
#include "RfxMessageId.h"
#include <telephony/mtk_ril.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "RmcNwHdlr"

#define MAX_OPER_NAME_LENGTH 50

typedef enum {
    CACHE_GROUP_GSM        = 0,
    CACHE_GROUP_C2K        = 1,
    CACHE_GROUP_1XRTT      = 2,
    CACHE_GROUP_EVDO       = 3,
    CACHE_GROUP_NR         = 4,
    CACHE_GROUP_COMMON_REQ = 5,
    CACHE_GROUP_ALL        = 6
} RIL_CACHE_GROUP;

// HIDL v1.4
typedef struct {
    int gsm_signal_strength;  // 1
    int gsm_bit_error_rate;
    int gsm_timing_advance;
    int cdma_dbm;
    int cdma_ecio;  // 5
    int evdo_dbm;
    int evdo_ecio;
    int evdo_snr;
    int lte_signal_strength;
    int lte_rsrp;  // 10
    int lte_rsrq;
    int lte_rssnr;
    int lte_cqi;
    int lte_timing_advance;
    int tdscdma_signal_strength;  // 15
    int tdscdma_bit_error_rate;
    int tdscdma_rscp;
    int wcdma_signal_strength;
    int wcdma_bit_error_rate;
    int wcdma_scdma_rscp;  // 20
    int wcdma_ecno;
    // nr
    int ssRsrp;
    int ssRsrq;
    int ssSinr;
    int csiRsrp;  // 25
    int csiRsrq;
    int csiSinr;
} RIL_SIGNAL_STRENGTH_CACHE;

typedef struct {
    int sig1;
    int sig2;
    int rssi_in_qdbm;
    int rscp_in_qdbm;
    int ecn0_in_qdbm;
    int rsrq_in_qdbm;
    int rsrp_in_qdbm;
    int act;
    int sig3;
    int serv_band;
    int csiRsrq_in_qdb;
    int csiRsrp_in_qdbm;
    int csiSinr_in_qdb;
} MD_SIGNAL_STRENGTH;

typedef struct {
    int registration_state;
    unsigned int lac;
    unsigned long long cid;
    int radio_technology;
    int base_station_id;
    int base_station_latitude;
    int base_station_longitude;
    int css;
    int system_id;
    int network_id;
    int roaming_indicator;
    int is_in_prl;
    int default_roaming_indicator;
    int denied_reason;
    int psc;
    int network_exist;
    // MTK_TC1_FEATURE
    char *sector_id;
    char *subnet_mask;
} RIL_VOICE_REG_STATE_CACHE;

typedef struct {
    int registration_state;
    unsigned int lac;
    unsigned long long cid;
    int radio_technology;
    int denied_reason;
    int max_simultaneous_data_call;
    int tac;
    int physical_cid;
    int eci;
    int csgid;
    int tadv;
    // NR none-standalone
    int dcnr_restricted;
    int endc_sib;
    int endc_available;
    // MTK_TC1_FEATURE - ERI
    int roaming_indicator;
    int is_in_prl;
    int default_roaming_indicator;
} RIL_DATA_REG_STATE_CACHE;

typedef struct {
    int ca_info;
    int pcell_bw;
    int scell_bw1;
    int scell_bw2;
    int scell_bw3;
    int scell_bw4;
} RIL_CA_CACHE;

typedef struct {
    char mccMnc[8];
    char longName[MAX_OPER_NAME_LENGTH];
    char shortName[MAX_OPER_NAME_LENGTH];
} RIL_OPERATOR_INFO_CACHE;

typedef struct {
    int domain;
    int state;
    int plmn_id;
    int act;
    int is_femtocell;
    int is_csg_cell;
    int csg_id;
    int csg_icon_type;
    char hnbName[MAX_OPER_NAME_LENGTH];
    int cause;
    // MTK_TC1_FEATURE - ERI
    int is_1x_femtocell;
    int is_evdo_femtocell;
} RIL_FEMTO_CELL_CACHE;

/* EONS status reported from modem */
typedef enum
{
    EONS_INFO_NOT_RECEIVED,
    EONS_INFO_RECEIVED_DISABLED,
    EONS_INFO_RECEIVED_ENABLED
} RIL_EonsStatusInfo;

// Defines EONS network feature support info.
typedef struct {
    RIL_EonsStatusInfo eons_status;
    unsigned int lac;
} RIL_EonsNetworkFeatureInfo;

typedef struct SPNTABLE {
    char mccMnc[8];
    char spn[MAX_OPER_NAME_LENGTH+2];
    char short_name[MAX_OPER_NAME_LENGTH+2];
} SpnTable;

typedef enum {
    RAT_GROUP_UNKNOWN,
    RAT_GROUP_2G,
    RAT_GROUP_3G,
    RAT_GROUP_4G
} RIL_RAT_GROUP;

/* +EREG: <n>,<stat>,<lac>,<ci>,<eAct>,<nwk_existence >,<roam_indicator>,<cause_type>,<reject_cause> */
typedef struct {
    int n;
    int stat;
    unsigned int lac;
    unsigned long long ci;
    int eAct;
    int nwk_existence;
    int roam_indicator;
    int cause_type;
    int reject_cause;
} MD_EREG;

/* +EGREG: <n>, <stat>, <lac>, <cid>, <eAct>, <rac>,
        <nwk_existence>, <roam_indicator>, <cause_type>, <reject_cause> */
typedef struct {
    int n;
    int stat;
    unsigned int lac;
    unsigned long long ci;
    int eAct;
    int rac;
    int nwk_existence;
    int roam_indicator;
    int cause_type;
    int reject_cause;
    // NR
    int dcnr_restricted;
    int endc_sib;
    int endc_available;
} MD_EGREG;

typedef struct {
    int mode;
    int format;
    char oper[10];
} MD_EOPS;

typedef struct {
    int act;
    long long cid;
    int mcc;
    int mnc;
    int lacTac;
    int pscPci;
    int sig1;
    int sig2;
    int rsrp;
    int rsrq;
    int timingAdvance;
    int rssnr;
    int cqi;
    int bsic;
    int arfcn;
} MD_ECELL;

typedef struct {
    // +VLOCINFO:<rev>,<mcc>,<mnc>,<sid>,<nid>,<bs_id>,<bs_lat>,<bs_long>,<sector_id>,<subnet_mask>
    int mcc;
    int mnc;
    int system_id;
    int network_id;
    int base_station_id;
    int base_station_latitude;
    int base_station_longitude;
    char *sector_id;
    char *subnet_mask;
} CDMA_CELL_LOCATION_INFO;


#define PROPERTY_NITZ_OPER_CODE     "persist.vendor.radio.nitz_oper_code"
#define PROPERTY_NITZ_OPER_LNAME    "persist.vendor.radio.nitz_oper_lname"
#define PROPERTY_NITZ_OPER_SNAME    "persist.vendor.radio.nitz_oper_sname"

#define PROPERTY_WFC_STATE          "persist.vendor.radio.wfc_state"
#define PROPERTY_MTK_DATA_TYPE      "persist.vendor.radio.mtk_data_type"

#define PROPERTY_MTK_SIGNAL_BAND    "persist.vendor.radio.signal_band"

#define LIFE_VALID_PLMN 30000000000 // 30 seconds

// Use int max, as -1 is a valid value in signal strength
#define CELLINFO_INVALID 0x7FFFFFFF

#define DISPLAY_ALL 0x07
#define DISPLAY_TS25 0x01
#define DISPLAY_NITZ 0x02
#define DISPLAY_EONS 0x04

class RmcNetworkHandler : public RfxBaseHandler {
    public:
        RmcNetworkHandler(int slot_id, int channel_id);
        virtual ~RmcNetworkHandler() {}

        /**
         * Convert registration state
         * @param uiRegState registration state from modem
         * @param isVoiceState whether is voice state
         * @return registration state to framework
         */
        unsigned int convertRegState(unsigned int uiRegState, bool isVoiceState);
        int getSignalStrength(RfxAtLine *line);
        void resetSignalStrengthCache(RIL_SIGNAL_STRENGTH_CACHE *sigCache, RIL_CACHE_GROUP source);
        bool isTdd3G();
        int isFemtocellSupport();
        void updateSignalStrengthProperty();
        static String8 getCurrentLteSignal(int slotId);
        int isOp12Plmn(const char* plmn);
        bool isMatchOpid(const char* opid);
        int getCellInfoListV12(RfxAtLine* line, int num, RIL_CellInfo_v12 * response);
        int convert3GRssiValue(int rscp_in_dbm);
        int convertToModUtf8Encode(int src);
        void convertToUtf8String(char *src);
        unsigned int convertCSNetworkType(unsigned int uiResponse);
        unsigned int convertPSNetworkType(unsigned int uiResponse);
        void updateWfcState(int status);
        int getPLMNNameFromNumeric(char *numeric,
            char *longname, char *shortname, int max_length);
        int getEonsNamesFromNumericCode(char *code,
            unsigned int lac, char *longname, char *shortname, int max_length);
        int getOperatorNamesFromNumericCode(char *code,
            char *longname, char *shortname, int max_length);
        int getOperatorNamesFromNumericCode(char *code,
            unsigned int lac, char *longname, char *shortname, int max_length);
        int getOperatorNamesFromNumericCodeByDisplay(char *code,
            unsigned int lac, char *longname, char *shortname, int max_length, int display);
        void updateFemtoCellInfo();
        void isFemtoCell(int regState, int cid, int act);
        void printSignalStrengthCache(char *source);
        void isNeedNotifyStateChanged();

        char const *sourceToString(int srcId);
        RIL_RAT_GROUP getVoiceRatGroup(int act);
        RIL_RAT_GROUP getDataRatGroup(int act);

        void setLastValidPlmn(const char* plmn);
        bool getLastValidPlmn(char* plmn, int size);
        bool isRilTestMode();
        bool supportMccMncUrc();

        bool isInService(int regState) {
            if (regState == 1 || regState == 5) {
                return true;
            }
            return false;
        }

        bool isNrSupported();
        int isGcfTestMode();
        int isTestSim(int slotId);
        bool isAPInCall();

        void refreshEcellCache();
        void freeOperatorNameOfCellInfo(int num, RIL_CellInfo_v12 *response);
        void dumpEregCache(const char* fun, MD_EREG *mMdEreg, MD_ECELL *mMdEcell);
        void dumpEregCache(const char* fun, MD_EREG *mMdEreg, MD_ECELL *mMdEcell, bool debugV);
        void dumpEgregCache(const char* fun, MD_EGREG *mMdEgreg, MD_ECELL *mMdEcell);
        void dumpEgregCache(const char* fun, MD_EGREG *mMdEgreg, MD_ECELL *mMdEcell, bool debugV);
        void dumpEcellCache(const char* fun, MD_ECELL *mMdEcell);
        char* getMask(int i);

        static RIL_EonsNetworkFeatureInfo eons_info[MAX_SIM_COUNT];
        /* modem ECELL ext3 ext4 support
         * value: 0 not surrpot
         *        1 support
         **/
        static int ECELLext3ext4Support;
        static RIL_FEMTO_CELL_CACHE *femto_cell_cache[MAX_SIM_COUNT];

        static pthread_mutex_t ril_nw_femtoCell_mutex;

        static pthread_mutex_t s_signalStrengthMutex[MAX_SIM_COUNT];
        static RIL_SIGNAL_STRENGTH_CACHE *signal_strength_cache[MAX_SIM_COUNT];
        static pthread_mutex_t s_voiceRegStateMutex[MAX_SIM_COUNT];
        static RIL_VOICE_REG_STATE_CACHE *voice_reg_state_cache[MAX_SIM_COUNT];
        static RIL_DATA_REG_STATE_CACHE *data_reg_state_cache[MAX_SIM_COUNT];
        static RIL_OPERATOR_INFO_CACHE *op_info_cache[MAX_SIM_COUNT];

        static pthread_mutex_t s_urc_voiceRegStateMutex[MAX_SIM_COUNT];
        static RIL_VOICE_REG_STATE_CACHE *urc_voice_reg_state_cache[MAX_SIM_COUNT];
        static pthread_mutex_t s_urc_dataRegStateMutex[MAX_SIM_COUNT];
        static RIL_DATA_REG_STATE_CACHE *urc_data_reg_state_cache[MAX_SIM_COUNT];
        static char last_valid_plmn[MAX_SIM_COUNT][8];
        static nsecs_t last_valid_plmn_time[MAX_SIM_COUNT];

        static pthread_mutex_t s_caMutex[MAX_SIM_COUNT];
        static RIL_CA_CACHE *ca_cache[MAX_SIM_COUNT];
        static bool physicalConfigSwitch[MAX_SIM_COUNT];
        static int physicalConfig_cache[MAX_SIM_COUNT][4];

        /* WFC registration status */
        static int ril_wfc_reg_status[MAX_SIM_COUNT];
        static pthread_mutex_t s_wfcRegStatusMutex[MAX_SIM_COUNT];

        static bool mIsNeedNotifyState[MAX_SIM_COUNT];

        static int mPlmnListOngoing;
        static int mPlmnListAbort;

        // cache SPN_TABLE
        static SPNTABLE cache_spn_table[MAX_SIM_COUNT];

        static bool m_dc_support[MAX_SIM_COUNT];
        static bool ims_ecc_state[MAX_SIM_COUNT];
        static bool ca_filter_switch[MAX_SIM_COUNT];
        static int data_context_ids[MAX_SIM_COUNT][16];

        static MD_EREG *mdEreg[MAX_SIM_COUNT];
        static pthread_mutex_t mdEregMutex[MAX_SIM_COUNT];
        static MD_EGREG *mdEgreg[MAX_SIM_COUNT];
        static pthread_mutex_t mdEgregMutex[MAX_SIM_COUNT];
        static MD_EOPS *mdEops[MAX_SIM_COUNT];
        static pthread_mutex_t mdEopsMutex[MAX_SIM_COUNT];

        static MD_ECELL *mdEcell[MAX_SIM_COUNT];
        static pthread_mutex_t mdEcellMutex[MAX_SIM_COUNT];

        static int mLastSignalBand[MAX_SIM_COUNT];

    protected:
        static pthread_mutex_t ril_nw_nitzName_mutex[MAX_SIM_COUNT];
        static char m_ril_nw_nitz_oper_code[MAX_SIM_COUNT][MAX_OPER_NAME_LENGTH];
        static char m_ril_nw_nitz_oper_lname[MAX_SIM_COUNT][MAX_OPER_NAME_LENGTH];
        static char m_ril_nw_nitz_oper_sname[MAX_SIM_COUNT][MAX_OPER_NAME_LENGTH];
        // Support 5G
        static int nr_support;

    private:
        static String8 mCurrentLteSignal[MAX_SIM_COUNT];
        static bool nwHandlerInit[MAX_SIM_COUNT];
        static pthread_mutex_t ril_handler_init_mutex[MAX_SIM_COUNT];
};

#endif
