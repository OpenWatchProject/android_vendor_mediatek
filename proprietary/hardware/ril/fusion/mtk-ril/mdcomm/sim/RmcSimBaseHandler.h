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

#ifndef __RMC_SIM_BASE_HANDLER_H__
#define __RMC_SIM_BASE_HANDLER_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "RfxBaseHandler.h"
#include "RmcCommSimDefs.h"
#include "ratconfig.h"

#define SINGLE_MODE_SIM_CARD  10
#define SINGLE_MODE_USIM_CARD  20
#define SINGLE_MODE_RUIM_CARD  30
#define DUAL_MODE_CG_CARD  40
#define DUAL_MODE_UG_CARD  50
#define CT_NATIONAL_ROAMING_CARD  41
#define CU_DUAL_MODE_CARD  42
#define DUAL_MODE_TELECOM_LTE_CARD  43
#define UNKNOWN_CARD  -1

#define MAX_RETRY_COUNT  30

/*****************************************************************************
 * Class RpSimController
 *****************************************************************************/

class RmcSimBaseHandler : public RfxBaseHandler {
RFX_DECLARE_HANDLER_CLASS(RmcSimBaseHandler);


protected:
    enum SIM_HANDLE_RESULT {
        RESULT_NEED,
        RESULT_IGNORE,

        RESULT_END
    };

    String8 mTag;

public:
    RmcSimBaseHandler(int slot_id, int channel_id);
    virtual ~RmcSimBaseHandler();

    // Check if the handler have to process the Request or not
    virtual SIM_HANDLE_RESULT needHandle(const sp<RfxMclMessage>& msg);

    // Process URC here
    virtual void handleUrc(const sp<RfxMclMessage>& msg, RfxAtLine *urc);

    // Process Request here
    virtual void handleRequest(const sp<RfxMclMessage>& msg);

    virtual void handleEvent(const sp<RfxMclMessage>& msg) {RFX_UNUSED(msg);}

    // Return the string of request or event
    virtual char* handleIdToString(int id) {
        RFX_UNUSED(id);
        return NULL;
    }

    // Return the request list which you want to reqister
    virtual const int* queryTable(int channel_id, int *record_num);

    virtual const char** queryUrcTable(int *record_num);

    // Return the event list which you want to reqister
    virtual const int* queryEventTable(int channel_id, int *record_num) {
        RFX_UNUSED(channel_id);
        RFX_UNUSED(record_num);
        return NULL;
    }

// Override
protected:
    pthread_mutex_t simStatusMutex = PTHREAD_MUTEX_INITIALIZER;

    int queryAppTypeId(String8 aid);

    void setTag(String8 s);
    bool bIsTc1();
    bool isCommontSlotSupport();
    bool isSimInserted();
    bool isSimSlotLockSupport();
    bool getIccId(char* value);
    RmcSimPinPukCount* getPinPukRetryCount(void);
    void setPinPukRetryCountProp(RmcSimPinPukCount *retry);
    RmcSimSlotLockDeviceLockInfo* getSimSlotLockDeviceLockInfo(void);
    void setSimSlotLockDeviceLockInfoProp(RmcSimSlotLockDeviceLockInfo *retry);
    UICC_Status getSimStatus();
    bool isOP09AProject();
    // Convert all the lower case characters in string to upper case
    char* stringToUpper(char *str);
    void sendSimStatusChanged();
    bool isAOSPPropSupport();
    bool isSimIoFcp(char* response);
    void handleCdmaCardType(const char *iccid);
    bool isOp09Card(const char *iccid);
    int isApplicationIdExist(const char *aid);
    void makeSimRspFromUsimFcp(unsigned char ** simResponse);
    void resetSimPropertyAndStatusKey();
    RIL_SimSlotStatus** decodeEslotsinfo(RfxAtLine *line, int *decodeerr, int *slotnum);
};
#endif /* __RMC_SIM_BASE_HANDLER_H__ */

