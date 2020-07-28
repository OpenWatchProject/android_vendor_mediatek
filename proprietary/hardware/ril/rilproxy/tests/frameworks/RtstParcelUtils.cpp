/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2016. All rights reserved.
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
#include <stdlib.h>
#include <cutils/jstring.h>
#include <sys/epoll.h>
#include <poll.h>
#include "RtstUtils.h"
#include "RfxBasics.h"
//#include "RfxRilUtils.h"
//#include "RtstEnv.h"
#include <sys/types.h>
#include <telephony/mtk_ril.h>
#include <telephony/ril.h>
#include "RtstParcelUtils.h"
/*****************************************************************************
 * Define
 *****************************************************************************/
#define RFX_LOG_TAG "RtstPrclUtil"

using android::status_t;
using android::NO_ERROR;
using android::NO_MEMORY;

/* Negative values for private RIL errno's */
#define RIL_ERRNO_INVALID_RESPONSE -1
#define RIL_ERRNO_NO_MEMORY -12

#define MIN(a,b) ((a)<(b) ? (a) : (b))

#define closeRequest
#define startRequest
#define startResponse
#define closeResponse
#define printBuf
#define removeLastChar
#define appendPrintBuf(a, ...)
#define printRequest(a, b)
/*****************************************************************************
 * Parcel utils api
 *****************************************************************************/
RIL_RadioFunctions s_callbacks = {0, NULL, NULL, NULL, NULL, NULL, NULL};
#if defined(ANDROID_MULTI_SIM)
#define CALL_ONREQUEST(a, b, c, d, e) s_callbacks.onRequest((a), (b), (c), (d), (e))
#else
#define CALL_ONREQUEST(a, b, c, d, e) s_callbacks.onRequest((a), (b), (c), (d))
#endif
const char *
requestToString(int request __unused) {
    return NULL;
}

 /** Callee expects NULL */
 void
 dispatchVoid(Parcel& p __unused, RequestInfo *pRI) {
     CALL_ONREQUEST(pRI->pCI->requestNumber, NULL, 0, pRI, pRI->socket_id);
 }

char *
strdupReadString(Parcel &p) {
    size_t stringlen;
    const char16_t *s16;

    s16 = p.readString16Inplace(&stringlen);

    return strndup16to8(s16, stringlen);
}

status_t
readStringFromParcelInplace(Parcel &p, char *str, size_t maxLen) {
    size_t s16Len;
    const char16_t *s16;

    s16 = p.readString16Inplace(&s16Len);
    if (s16 == NULL) {
        return NO_MEMORY;
    }
    size_t strLen = strnlen16to8(s16, s16Len);
    if ((strLen + 1) > maxLen) {
        return NO_MEMORY;
    }
    if (strncpy16to8(str, s16, strLen) == NULL) {
        return NO_MEMORY;
    } else {
        return NO_ERROR;
    }
}

void writeStringToParcel(Parcel &p, const char *s) {
    char16_t *s16;
    size_t s16_len;
    s16 = strdup8to16(s, &s16_len);
    p.writeString16(s16, s16_len);
    free(s16);
}

void
invalidCommandBlock(RequestInfo *pRI) {
    RFX_LOG_E(RFX_LOG_TAG, "invalid command block for token %d request %d",
        pRI->token, pRI->pCI->requestNumber);
}

/** Callee expects const char * */
void
dispatchString(Parcel& p, RequestInfo *pRI) {
    char *string8 = NULL;

    string8 = strdupReadString(p);
    CALL_ONREQUEST(pRI->pCI->requestNumber, (void *)string8,
        strlen(string8), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
memsetString(string8);
#endif

free(string8);
return;
}

 /** Callee expects const char ** */
void
dispatchStrings(Parcel &p, RequestInfo *pRI) {
    int32_t countStrings;
    status_t status;
    size_t datalen;
    char **pStrings;

    status = p.readInt32 (&countStrings);

    if (status != NO_ERROR) {
        goto invalid;
    }

    if (countStrings == 0) {
        // just some non-null pointer
        pStrings = (char **)calloc(1, sizeof(char *));
        if (pStrings == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "Memory allocation failed for request %s",
                    requestToString(pRI->pCI->requestNumber));
            return;
        }

        datalen = 0;
    } else if (countStrings < 0) {
        pStrings = NULL;
        datalen = 0;
    } else {
        datalen = sizeof(char *) * countStrings;

        pStrings = (char **)calloc(countStrings, sizeof(char *));
        if (pStrings == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "Memory allocation failed for request %s",
                    requestToString(pRI->pCI->requestNumber));
            return;
        }

        for (int i = 0 ; i < countStrings ; i++) {
            pStrings[i] = strdupReadString(p);
        }
    }
    CALL_ONREQUEST(pRI->pCI->requestNumber, pStrings, datalen, pRI, pRI->socket_id);

    if (pStrings != NULL) {
        for (int i = 0 ; i < countStrings ; i++) {
#ifdef MEMSET_FREED
            memsetString(pStrings[i]);
#endif
            free(pStrings[i]);
        }

#ifdef MEMSET_FREED
        memset(pStrings, 0, datalen);
#endif
        free(pStrings);
     }

     return;
 invalid:
     invalidCommandBlock(pRI);
     return;
 }

 /** Callee expects const int * */
void
dispatchInts(Parcel &p, RequestInfo *pRI) {
    int32_t count = 0;
    status_t status = NO_ERROR;
    size_t datalen = 0;
    int *pInts = NULL;

    status = p.readInt32 (&count);

    if (status != NO_ERROR || count <= 0) {
        goto invalid;
    }

    datalen = sizeof(int) * count;
    pInts = (int *)calloc(count, sizeof(int));
    if (pInts == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "Memory allocation failed for request %s", requestToString(pRI->pCI->requestNumber));
        return;
    }

    for (int i = 0 ; i < count ; i++) {
        int32_t t = 0;

        status = p.readInt32(&t);
        pInts[i] = (int)t;

        if (status != NO_ERROR) {
            free(pInts);
            goto invalid;
        }
    }
    CALL_ONREQUEST(pRI->pCI->requestNumber, const_cast<int *>(pInts),
           datalen, pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memset(pInts, 0, datalen);
#endif
    free(pInts);
    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

/**
 * Callee expects const RIL_SMS_WriteArgs *
 * Payload is:
 *   int32_t status
 *   String pdu
 */
void
dispatchSmsWrite (Parcel &p, RequestInfo *pRI) {
    RIL_SMS_WriteArgs args;
    int32_t t;
    status_t status;

    RFX_LOG_D(RFX_LOG_TAG, "dispatchSmsWrite");
    memset (&args, 0, sizeof(args));

    status = p.readInt32(&t);
    args.status = (int)t;

    args.pdu = strdupReadString(p);

    if (status != NO_ERROR || args.pdu == NULL) {
        goto invalid;
    }

    args.smsc = strdupReadString(p);

    startRequest;
    appendPrintBuf("%s%d,%s,smsc=%s", printBuf, args.status,
        (char*)args.pdu,  (char*)args.smsc);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &args, sizeof(args), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memsetString (args.pdu);
#endif

    free (args.pdu);

#ifdef MEMSET_FREED
    memset(&args, 0, sizeof(args));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

/**
 * Callee expects const RIL_Dial *
 * Payload is:
 *   String address
 *   int32_t clir
 */
void
dispatchDial (Parcel &p, RequestInfo *pRI) {
    RIL_Dial dial;
    RIL_UUS_Info uusInfo;
    int32_t sizeOfDial;
    int32_t t;
    int32_t uusPresent;
    status_t status;

    RFX_LOG_D(RFX_LOG_TAG, "dispatchDial");
    memset (&dial, 0, sizeof(dial));

    dial.address = strdupReadString(p);

    status = p.readInt32(&t);
    dial.clir = (int)t;

    if (status != NO_ERROR || dial.address == NULL) {
        goto invalid;
    }

    if (s_callbacks.version < 3) { // Remove when partners upgrade to version 3
        uusPresent = 0;
        sizeOfDial = sizeof(dial) - sizeof(RIL_UUS_Info *);
    } else {
        status = p.readInt32(&uusPresent);

        if (status != NO_ERROR) {
            goto invalid;
        }

        if (uusPresent == 0) {
            dial.uusInfo = NULL;
        } else {
            int32_t len;

            memset(&uusInfo, 0, sizeof(RIL_UUS_Info));

            status = p.readInt32(&t);
            uusInfo.uusType = (RIL_UUS_Type) t;

            status = p.readInt32(&t);
            uusInfo.uusDcs = (RIL_UUS_DCS) t;

            status = p.readInt32(&len);
            if (status != NO_ERROR) {
                goto invalid;
            }

            // The java code writes -1 for null arrays
            if (((int) len) == -1) {
                uusInfo.uusData = NULL;
                len = 0;
            } else {
                uusInfo.uusData = (char*) p.readInplace(len);
            }

            uusInfo.uusLength = len;
            dial.uusInfo = &uusInfo;
        }
        sizeOfDial = sizeof(dial);
    }

    startRequest;
    appendPrintBuf("%snum=%s,clir=%d", printBuf, dial.address, dial.clir);
    if (uusPresent) {
        appendPrintBuf("%s,uusType=%d,uusDcs=%d,uusLen=%d", printBuf,
                dial.uusInfo->uusType, dial.uusInfo->uusDcs,
                dial.uusInfo->uusLength);
    }
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &dial, sizeOfDial, pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memsetString (dial.address);
#endif

    free (dial.address);

#ifdef MEMSET_FREED
    memset(&uusInfo, 0, sizeof(RIL_UUS_Info));
    memset(&dial, 0, sizeof(dial));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

void
dispatchEmergencyDial(Parcel &p, RequestInfo *pRI) {
    RIL_Emergency_Dial emergencyDial;
    RIL_Dial dial;
    RIL_UUS_Info uusInfo;
    int32_t sizeOfEmergencyDial;
    int32_t t;
    int32_t uusPresent;
    status_t status;

    RFX_LOG_D(RFX_LOG_TAG, "dispatchEmergencyDial");
    memset(&emergencyDial, 0, sizeof(emergencyDial));

    memset(&dial, 0, sizeof(dial));

    dial.address = strdupReadString(p);

    status = p.readInt32(&t);
    dial.clir = (int)t;

    if (status != NO_ERROR || dial.address == NULL) {
        goto invalid;
    }

    status = p.readInt32(&uusPresent);

    if (status != NO_ERROR) {
        goto invalid;
    }

    if (uusPresent == 0) {
        dial.uusInfo = NULL;
    } else {
        int32_t len;

        memset(&uusInfo, 0, sizeof(RIL_UUS_Info));

        status = p.readInt32(&t);
        uusInfo.uusType = (RIL_UUS_Type) t;

        status = p.readInt32(&t);
        uusInfo.uusDcs = (RIL_UUS_DCS) t;

        status = p.readInt32(&len);
        if (status != NO_ERROR) {
            goto invalid;
        }

        // The java code writes -1 for null arrays
        if (((int) len) == -1) {
            uusInfo.uusData = NULL;
            len = 0;
        } else {
            uusInfo.uusData = (char*) p.readInplace(len);
        }

        uusInfo.uusLength = len;
        dial.uusInfo = &uusInfo;
    }

    emergencyDial.dialData = &dial;

    status = p.readInt32(&t);
    emergencyDial.serviceCategory = (EmergencyServiceCategory)t;

    status = p.readInt32(&t);
    emergencyDial.routing = (EmergencyCallRouting)t;

    status = p.readInt32(&t);
    emergencyDial.isTesting = (t != 0);

    sizeOfEmergencyDial = sizeof(emergencyDial);

    startRequest;
    appendPrintBuf("%snum=%s,clir=%d", printBuf, dial.address, dial.clir);
    if (uusPresent) {
        appendPrintBuf("%s,uusType=%d,uusDcs=%d,uusLen=%d", printBuf,
                dial.uusInfo->uusType, dial.uusInfo->uusDcs,
                dial.uusInfo->uusLength);
    }
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &emergencyDial, sizeOfEmergencyDial, pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memsetString(dial.address);
#endif

    free(dial.address);

#ifdef MEMSET_FREED
    memset(&uusInfo, 0, sizeof(RIL_UUS_Info));
    memset(&dial, 0, sizeof(dial));
    memset(&emergencyDial, 0, sizeof(emergencyDial));
#endif

    return;

invalid:
    invalidCommandBlock(pRI);
    if (dial.address != NULL) {
#ifdef MEMSET_FREED
        memsetString(dial.address);
#endif
        free(dial.address);
    }

#ifdef MEMSET_FREED
    memset(&uusInfo, 0, sizeof(RIL_UUS_Info));
    memset(&dial, 0, sizeof(dial));
    memset(&emergencyDial, 0, sizeof(emergencyDial));
#endif
    return;
}

/**
 * Callee expects const RIL_SIM_IO *
 * Payload is:
 *   int32_t command
 *   int32_t fileid
 *   String path
 *   int32_t p1, p2, p3
 *   String data
 *   String pin2
 *   String aidPtr
 */
void
dispatchSIM_IO (Parcel &p, RequestInfo *pRI) {
    union RIL_SIM_IO {
        RIL_SIM_IO_v6 v6;
        RIL_SIM_IO_v5 v5;
    } simIO;

    int32_t t;
    int size;
    status_t status;

#if VDBG
    RFX_LOG_D(RFX_LOG_TAG, "dispatchSIM_IO");
#endif
    memset (&simIO, 0, sizeof(simIO));

    // note we only check status at the end

    status = p.readInt32(&t);
    simIO.v6.command = (int)t;

    status = p.readInt32(&t);
    simIO.v6.fileid = (int)t;

    simIO.v6.path = strdupReadString(p);

    status = p.readInt32(&t);
    simIO.v6.p1 = (int)t;

    status = p.readInt32(&t);
    simIO.v6.p2 = (int)t;

    status = p.readInt32(&t);
    simIO.v6.p3 = (int)t;

    simIO.v6.data = strdupReadString(p);
    simIO.v6.pin2 = strdupReadString(p);
    simIO.v6.aidPtr = strdupReadString(p);

    startRequest;
    appendPrintBuf("%scmd=0x%X,efid=0x%X,path=%s,%d,%d,%d,%s,pin2=%s,aid=%s", printBuf,
        simIO.v6.command, simIO.v6.fileid, (char*)simIO.v6.path,
        simIO.v6.p1, simIO.v6.p2, simIO.v6.p3,
        (char*)simIO.v6.data,  (char*)simIO.v6.pin2, simIO.v6.aidPtr);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    if (status != NO_ERROR) {
        goto invalid;
    }

    size = (s_callbacks.version < 6) ? sizeof(simIO.v5) : sizeof(simIO.v6);
    CALL_ONREQUEST(pRI->pCI->requestNumber, &simIO, size, pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memsetString (simIO.v6.path);
    memsetString (simIO.v6.data);
    memsetString (simIO.v6.pin2);
    memsetString (simIO.v6.aidPtr);
#endif

    free (simIO.v6.path);
    free (simIO.v6.data);
    free (simIO.v6.pin2);
    free (simIO.v6.aidPtr);

#ifdef MEMSET_FREED
    memset(&simIO, 0, sizeof(simIO));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

/**
 * Callee expects const RIL_SIM_APDU *
 * Payload is:
 *   int32_t sessionid
 *   int32_t cla
 *   int32_t instruction
 *   int32_t p1, p2, p3
 *   String data
 */
void
dispatchSIM_APDU (Parcel &p, RequestInfo *pRI) {
    int32_t t;
    status_t status;
    RIL_SIM_APDU apdu;

#if VDBG
    RFX_LOG_D(RFX_LOG_TAG, "dispatchSIM_APDU");
#endif
    memset (&apdu, 0, sizeof(RIL_SIM_APDU));

    // Note we only check status at the end. Any single failure leads to
    // subsequent reads filing.
    status = p.readInt32(&t);
    apdu.sessionid = (int)t;

    status = p.readInt32(&t);
    apdu.cla = (int)t;

    status = p.readInt32(&t);
    apdu.instruction = (int)t;

    status = p.readInt32(&t);
    apdu.p1 = (int)t;

    status = p.readInt32(&t);
    apdu.p2 = (int)t;

    status = p.readInt32(&t);
    apdu.p3 = (int)t;

    apdu.data = strdupReadString(p);

    startRequest;
    appendPrintBuf("%ssessionid=%d,cla=%d,ins=%d,p1=%d,p2=%d,p3=%d,data=%s",
        printBuf, apdu.sessionid, apdu.cla, apdu.instruction, apdu.p1, apdu.p2,
        apdu.p3, (char*)apdu.data);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    if (status != NO_ERROR) {
        goto invalid;
    }

    CALL_ONREQUEST(pRI->pCI->requestNumber, &apdu, sizeof(RIL_SIM_APDU), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memsetString(apdu.data);
#endif
    free(apdu.data);

#ifdef MEMSET_FREED
    memset(&apdu, 0, sizeof(RIL_SIM_APDU));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}


/**
 * Callee expects const RIL_CallForwardInfo *
 * Payload is:
 *  int32_t status/action
 *  int32_t reason
 *  int32_t serviceCode
 *  int32_t toa
 *  String number  (0 length -> null)
 *  int32_t timeSeconds
 */
void
dispatchCallForward(Parcel &p, RequestInfo *pRI) {
    RIL_CallForwardInfo cff;
    int32_t t;
    status_t status;

    RFX_LOG_D(RFX_LOG_TAG, "dispatchCallForward");
    memset (&cff, 0, sizeof(cff));

    // note we only check status at the end

    status = p.readInt32(&t);
    cff.status = (int)t;

    status = p.readInt32(&t);
    cff.reason = (int)t;

    status = p.readInt32(&t);
    cff.serviceClass = (int)t;

    status = p.readInt32(&t);
    cff.toa = (int)t;

    cff.number = strdupReadString(p);

    status = p.readInt32(&t);
    cff.timeSeconds = (int)t;

    if (status != NO_ERROR) {
        goto invalid;
    }

    // special case: number 0-length fields is null

    if (cff.number != NULL && strlen (cff.number) == 0) {
        cff.number = NULL;
    }

    startRequest;
    appendPrintBuf("%sstat=%d,reason=%d,serv=%d,toa=%d,%s,tout=%d", printBuf,
        cff.status, cff.reason, cff.serviceClass, cff.toa,
        (char*)cff.number, cff.timeSeconds);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &cff, sizeof(cff), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memsetString(cff.number);
#endif

    free (cff.number);

#ifdef MEMSET_FREED
    memset(&cff, 0, sizeof(cff));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

/**
 * Callee expects const RIL_CallForwardInfoEx *
 * Payload is:
 *  int32_t status/action
 *  int32_t reason
 *  int32_t serviceCode
 *  int32_t toa
 *  String number  (0 length -> null)
 *  int32_t timeSeconds
 *  String timeSlotBegin  (0 length -> null)
 *  String timeSlotEnd  (0 length -> null)
 */
void
dispatchCallForwardEx(Parcel &p __unused, RequestInfo *pRI __unused) {
#if 0
    RIL_CallForwardInfoEx cff;
    int32_t t;
    status_t status;

    RFX_LOG_D(RFX_LOG_TAG, "dispatchCallForwardEx");
    memset (&cff, 0, sizeof(cff));

    // note we only check status at the end

    status = p.readInt32(&t);
    cff.status = (int)t;

    status = p.readInt32(&t);
    cff.reason = (int)t;

    status = p.readInt32(&t);
    cff.serviceClass = (int)t;

    status = p.readInt32(&t);
    cff.toa = (int)t;

    cff.number = strdupReadString(p);

    status = p.readInt32(&t);
    cff.timeSeconds = (int)t;

    cff.timeSlotBegin = strdupReadString(p);
    cff.timeSlotEnd = strdupReadString(p);

    if (status != NO_ERROR) {
        goto invalid;
    }

    // special case: number 0-length fields is null

    if (cff.number != NULL && strlen (cff.number) == 0) {
        cff.number = NULL;
    }

    if (cff.timeSlotBegin != NULL && strlen (cff.timeSlotBegin) == 0) {
        cff.timeSlotBegin = NULL;
    }
    if (cff.timeSlotEnd != NULL && strlen (cff.timeSlotEnd) == 0) {
        cff.timeSlotEnd = NULL;
    }

    startRequest;
    appendPrintBuf("%sstat=%d,reason=%d,serv=%d,toa=%d,%s,tout=%d,timeSlot=%s,%s", printBuf,
        cff.status, cff.reason, cff.serviceClass, cff.toa,
        (char*)cff.number, cff.timeSeconds, cff.timeSlotBegin, cff.timeSlotEnd);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &cff, sizeof(cff), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memsetString(cff.number);
    memsetString(cff.timeSlotBegin);
    memsetString(cff.timeSlotEnd);
#endif

    free (cff.number);
    free (cff.timeSlotBegin);
    free (cff.timeSlotEnd);

#ifdef MEMSET_FREED
    memset(&cff, 0, sizeof(cff));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
#endif
    return;
}

void
dispatchRaw(Parcel &p, RequestInfo *pRI) {
    int32_t len;
    status_t status;
    const void *data;

    status = p.readInt32(&len);

    if (status != NO_ERROR) {
        goto invalid;
    }

    // The java code writes -1 for null arrays
    if (((int)len) == -1) {
        data = NULL;
        len = 0;
    }

    data = p.readInplace(len);

    startRequest;
    appendPrintBuf("%sraw_size=%d", printBuf, len);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    CALL_ONREQUEST(pRI->pCI->requestNumber, const_cast<void *>(data), len, pRI, pRI->socket_id);

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

status_t
constructCdmaSms(Parcel &p, RequestInfo *pRI __unused, RIL_CDMA_SMS_Message& rcsm) {
    int32_t  t;
    uint8_t ut;
    status_t status;
    int32_t digitCount;
    int digitLimit;

    memset(&rcsm, 0, sizeof(rcsm));

    status = p.readInt32(&t);
    rcsm.uTeleserviceID = (int) t;

    status = p.read(&ut,sizeof(ut));
    rcsm.bIsServicePresent = (uint8_t) ut;

    status = p.readInt32(&t);
    rcsm.uServicecategory = (int) t;

    status = p.readInt32(&t);
    rcsm.sAddress.digit_mode = (RIL_CDMA_SMS_DigitMode) t;

    status = p.readInt32(&t);
    rcsm.sAddress.number_mode = (RIL_CDMA_SMS_NumberMode) t;

    status = p.readInt32(&t);
    rcsm.sAddress.number_type = (RIL_CDMA_SMS_NumberType) t;

    status = p.readInt32(&t);
    rcsm.sAddress.number_plan = (RIL_CDMA_SMS_NumberPlan) t;

    status = p.read(&ut,sizeof(ut));
    rcsm.sAddress.number_of_digits= (uint8_t) ut;

    digitLimit= MIN((rcsm.sAddress.number_of_digits), RIL_CDMA_SMS_ADDRESS_MAX);
    for(digitCount =0 ; digitCount < digitLimit; digitCount ++) {
        status = p.read(&ut,sizeof(ut));
        rcsm.sAddress.digits[digitCount] = (uint8_t) ut;
    }

    status = p.readInt32(&t);
    rcsm.sSubAddress.subaddressType = (RIL_CDMA_SMS_SubaddressType) t;

    status = p.read(&ut,sizeof(ut));
    rcsm.sSubAddress.odd = (uint8_t) ut;

    status = p.read(&ut,sizeof(ut));
    rcsm.sSubAddress.number_of_digits = (uint8_t) ut;

    digitLimit= MIN((rcsm.sSubAddress.number_of_digits), RIL_CDMA_SMS_SUBADDRESS_MAX);
    for(digitCount =0 ; digitCount < digitLimit; digitCount ++) {
        status = p.read(&ut,sizeof(ut));
        rcsm.sSubAddress.digits[digitCount] = (uint8_t) ut;
    }

    status = p.readInt32(&t);
    rcsm.uBearerDataLen = (int) t;

    digitLimit= MIN((rcsm.uBearerDataLen), RIL_CDMA_SMS_BEARER_DATA_MAX);
    for(digitCount =0 ; digitCount < digitLimit; digitCount ++) {
        status = p.read(&ut, sizeof(ut));
        rcsm.aBearerData[digitCount] = (uint8_t) ut;
    }

    if (status != NO_ERROR) {
        return status;
    }

    startRequest;
    appendPrintBuf("%suTeleserviceID=%d, bIsServicePresent=%d, uServicecategory=%d, \
            sAddress.digit_mode=%d, sAddress.Number_mode=%d, sAddress.number_type=%d, ",
            printBuf, rcsm.uTeleserviceID,rcsm.bIsServicePresent,rcsm.uServicecategory,
            rcsm.sAddress.digit_mode, rcsm.sAddress.number_mode,rcsm.sAddress.number_type);
    closeRequest;

    printRequest(pRI->token, pRI->pCI->requestNumber);

    return status;
}

void
dispatchCdmaSms(Parcel &p, RequestInfo *pRI) {
    RIL_CDMA_SMS_Message rcsm;

    RFX_LOG_D(RFX_LOG_TAG, "dispatchCdmaSms");
    if (NO_ERROR != constructCdmaSms(p, pRI, rcsm)) {
        goto invalid;
    }

    CALL_ONREQUEST(pRI->pCI->requestNumber, &rcsm, sizeof(rcsm),pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memset(&rcsm, 0, sizeof(rcsm));
#endif

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

void
dispatchImsCdmaSms(Parcel &p, RequestInfo *pRI, uint8_t retry, int32_t messageRef) {
    RIL_IMS_SMS_Message rism;
    RIL_CDMA_SMS_Message rcsm;

    RFX_LOG_D(RFX_LOG_TAG, "dispatchImsCdmaSms: retry=%d, messageRef=%d", retry, messageRef);

    if (NO_ERROR != constructCdmaSms(p, pRI, rcsm)) {
        goto invalid;
    }
    memset(&rism, 0, sizeof(rism));
    rism.tech = RADIO_TECH_3GPP2;
    rism.retry = retry;
    rism.messageRef = messageRef;
    rism.message.cdmaMessage = &rcsm;

    CALL_ONREQUEST(pRI->pCI->requestNumber, &rism,
            sizeof(RIL_RadioTechnologyFamily)+sizeof(uint8_t)+sizeof(int32_t)
            +sizeof(rcsm),pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memset(&rcsm, 0, sizeof(rcsm));
    memset(&rism, 0, sizeof(rism));
#endif

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

void
dispatchImsGsmSms(Parcel &p, RequestInfo *pRI, uint8_t retry, int32_t messageRef) {
    RIL_IMS_SMS_Message rism;
    int32_t countStrings;
    status_t status;
    size_t datalen;
    char **pStrings;
    RFX_LOG_D(RFX_LOG_TAG, "dispatchImsGsmSms: retry=%d, messageRef=%d", retry, messageRef);

    status = p.readInt32 (&countStrings);

    if (status != NO_ERROR) {
        goto invalid;
    }

    memset(&rism, 0, sizeof(rism));
    rism.tech = RADIO_TECH_3GPP;
    rism.retry = retry;
    rism.messageRef = messageRef;

    startRequest;
    appendPrintBuf("%stech=%d, retry=%d, messageRef=%d, ", printBuf,
                    (int)rism.tech, (int)rism.retry, rism.messageRef);
    if (countStrings == 0) {
        // just some non-null pointer
        pStrings = (char **)calloc(1, sizeof(char *));
        if (pStrings == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "Memory allocation failed for request %s",
                    requestToString(pRI->pCI->requestNumber));
            closeRequest;
            return;
        }

        datalen = 0;
    } else if (countStrings < 0) {
        pStrings = NULL;
        datalen = 0;
    } else {
        if ((size_t)countStrings > (INT_MAX/sizeof(char *))) {
            RFX_LOG_E(RFX_LOG_TAG, "Invalid value of countStrings: \n");
            closeRequest;
            return;
        }
        datalen = sizeof(char *) * countStrings;

        pStrings = (char **)calloc(countStrings, sizeof(char *));
        if (pStrings == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "Memory allocation failed for request %s",
                    requestToString(pRI->pCI->requestNumber));
            closeRequest;
            return;
        }

        for (int i = 0 ; i < countStrings ; i++) {
            pStrings[i] = strdupReadString(p);
            appendPrintBuf("%s%s,", printBuf, pStrings[i]);
        }
        RFX_LOG_E(RFX_LOG_TAG, "ImsGsmSms smsc %s, pdu %s", ((pStrings[0] != NULL)? pStrings[0] : "null"),
            ((pStrings[1] != NULL)? pStrings[1] : "null"));
    }
    removeLastChar;
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    rism.message.gsmMessage = pStrings;
    CALL_ONREQUEST(pRI->pCI->requestNumber, &rism,
            sizeof(RIL_RadioTechnologyFamily)+sizeof(uint8_t)+sizeof(int32_t)
            +datalen, pRI, pRI->socket_id);

    if (pStrings != NULL) {
        for (int i = 0 ; i < countStrings ; i++) {
#ifdef MEMSET_FREED
            memsetString (pStrings[i]);
#endif
            free(pStrings[i]);
        }

#ifdef MEMSET_FREED
        memset(pStrings, 0, datalen);
#endif
        free(pStrings);
    }

#ifdef MEMSET_FREED
    memset(&rism, 0, sizeof(rism));
#endif
    return;
invalid:
    RFX_LOG_E(RFX_LOG_TAG, "dispatchImsGsmSms invalid block");
    invalidCommandBlock(pRI);
    return;
}

void
dispatchImsSms(Parcel &p, RequestInfo *pRI) {
    int32_t  t;
    status_t status = p.readInt32(&t);
    RIL_RadioTechnologyFamily format;
    uint8_t retry;
    int32_t messageRef;

    RFX_LOG_D(RFX_LOG_TAG, "dispatchImsSms");
    if (status != NO_ERROR) {
        goto invalid;
    }
    format = (RIL_RadioTechnologyFamily) t;
    RFX_LOG_E(RFX_LOG_TAG, "tech %d", t);

    // read retry field
    status = p.read(&retry,sizeof(retry));
    if (status != NO_ERROR) {
        goto invalid;
    }
    RFX_LOG_E(RFX_LOG_TAG, "retry %d", retry);

    // read messageRef field
    status = p.read(&messageRef,sizeof(messageRef));
    if (status != NO_ERROR) {
        goto invalid;
    }
    RFX_LOG_E(RFX_LOG_TAG, "messageRef %d", messageRef);

    if (RADIO_TECH_3GPP == format) {
        dispatchImsGsmSms(p, pRI, retry, messageRef);
    } else if (RADIO_TECH_3GPP2 == format) {
        dispatchImsCdmaSms(p, pRI, retry, messageRef);
    } else {
        RFX_LOG_E(RFX_LOG_TAG, "requestImsSendSMS invalid format value =%d", format);
    }

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

void
dispatchCdmaSmsAck(Parcel &p, RequestInfo *pRI) {
    RIL_CDMA_SMS_Ack rcsa;
    int32_t  t;
    status_t status;

    RFX_LOG_D(RFX_LOG_TAG, "dispatchCdmaSmsAck");
    memset(&rcsa, 0, sizeof(rcsa));

    status = p.readInt32(&t);
    rcsa.uErrorClass = (RIL_CDMA_SMS_ErrorClass) t;

    status = p.readInt32(&t);
    rcsa.uSMSCauseCode = (int) t;

    if (status != NO_ERROR) {
        goto invalid;
    }

    startRequest;
    appendPrintBuf("%suErrorClass=%d, uTLStatus=%d, ",
            printBuf, rcsa.uErrorClass, rcsa.uSMSCauseCode);
    closeRequest;

    printRequest(pRI->token, pRI->pCI->requestNumber);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &rcsa, sizeof(rcsa),pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memset(&rcsa, 0, sizeof(rcsa));
#endif

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

void
dispatchGsmBrSmsCnf(Parcel &p, RequestInfo *pRI) {
    int32_t t;
    status_t status;
    int32_t num;

    status = p.readInt32(&num);
    if (status != NO_ERROR) {
        goto invalid;
    }

    {
        RIL_GSM_BroadcastSmsConfigInfo gsmBci[num];
        RIL_GSM_BroadcastSmsConfigInfo *gsmBciPtrs[num];

        startRequest;
        for (int i = 0 ; i < num ; i++ ) {
            gsmBciPtrs[i] = &gsmBci[i];

            status = p.readInt32(&t);
            gsmBci[i].fromServiceId = (int) t;

            status = p.readInt32(&t);
            gsmBci[i].toServiceId = (int) t;

            status = p.readInt32(&t);
            gsmBci[i].fromCodeScheme = (int) t;

            status = p.readInt32(&t);
            gsmBci[i].toCodeScheme = (int) t;

            status = p.readInt32(&t);
            gsmBci[i].selected = (uint8_t) t;

            appendPrintBuf("%s [%d: fromServiceId=%d, toServiceId =%d, \
                  fromCodeScheme=%d, toCodeScheme=%d, selected =%d]", printBuf, i,
                  gsmBci[i].fromServiceId, gsmBci[i].toServiceId,
                  gsmBci[i].fromCodeScheme, gsmBci[i].toCodeScheme,
                  gsmBci[i].selected);
        }
        closeRequest;

        if (status != NO_ERROR) {
            goto invalid;
        }

        CALL_ONREQUEST(pRI->pCI->requestNumber,
                              gsmBciPtrs,
                              num * sizeof(RIL_GSM_BroadcastSmsConfigInfo *),
                              pRI, pRI->socket_id);

#ifdef MEMSET_FREED
        memset(gsmBci, 0, num * sizeof(RIL_GSM_BroadcastSmsConfigInfo));
        memset(gsmBciPtrs, 0, num * sizeof(RIL_GSM_BroadcastSmsConfigInfo *));
#endif
    }

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

void
dispatchCdmaBrSmsCnf(Parcel &p, RequestInfo *pRI) {
    int32_t t;
    status_t status;
    int32_t num;

    status = p.readInt32(&num);
    if (status != NO_ERROR) {
        goto invalid;
    }

    {
        RIL_CDMA_BroadcastSmsConfigInfo cdmaBci[num];
        RIL_CDMA_BroadcastSmsConfigInfo *cdmaBciPtrs[num];

        startRequest;
        for (int i = 0 ; i < num ; i++ ) {
            cdmaBciPtrs[i] = &cdmaBci[i];

            status = p.readInt32(&t);
            cdmaBci[i].service_category = (int) t;

            status = p.readInt32(&t);
            cdmaBci[i].language = (int) t;

            status = p.readInt32(&t);
            cdmaBci[i].selected = (uint8_t) t;

            appendPrintBuf("%s [%d: service_category=%d, language =%d, \
                  entries.bSelected =%d]", printBuf, i, cdmaBci[i].service_category,
                  cdmaBci[i].language, cdmaBci[i].selected);
        }
        closeRequest;

        if (status != NO_ERROR) {
            goto invalid;
        }

        CALL_ONREQUEST(pRI->pCI->requestNumber,
                              cdmaBciPtrs,
                              num * sizeof(RIL_CDMA_BroadcastSmsConfigInfo *),
                              pRI, pRI->socket_id);

#ifdef MEMSET_FREED
        memset(cdmaBci, 0, num * sizeof(RIL_CDMA_BroadcastSmsConfigInfo));
        memset(cdmaBciPtrs, 0, num * sizeof(RIL_CDMA_BroadcastSmsConfigInfo *));
#endif
    }

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

void dispatchRilCdmaSmsWriteArgs(Parcel &p, RequestInfo *pRI) {
    RIL_CDMA_SMS_WriteArgs rcsw;
    int32_t  t;
    uint8_t  uct;
    status_t status;
    int32_t  digitCount;
    int32_t  digitLimit;

    memset(&rcsw, 0, sizeof(rcsw));

    status = p.readInt32(&t);
    rcsw.status = t;

    status = p.readInt32(&t);
    rcsw.message.uTeleserviceID = (int) t;

    status = p.read(&uct,sizeof(uct));
    rcsw.message.bIsServicePresent = (uint8_t) uct;

    status = p.readInt32(&t);
    rcsw.message.uServicecategory = (int) t;

    status = p.readInt32(&t);
    rcsw.message.sAddress.digit_mode = (RIL_CDMA_SMS_DigitMode) t;

    status = p.readInt32(&t);
    rcsw.message.sAddress.number_mode = (RIL_CDMA_SMS_NumberMode) t;

    status = p.readInt32(&t);
    rcsw.message.sAddress.number_type = (RIL_CDMA_SMS_NumberType) t;

    status = p.readInt32(&t);
    rcsw.message.sAddress.number_plan = (RIL_CDMA_SMS_NumberPlan) t;

    status = p.read(&uct,sizeof(uct));
    rcsw.message.sAddress.number_of_digits = (uint8_t) uct;

    digitLimit = MIN((rcsw.message.sAddress.number_of_digits), RIL_CDMA_SMS_ADDRESS_MAX);

    for(digitCount = 0 ; digitCount < digitLimit; digitCount ++) {
        status = p.read(&uct,sizeof(uct));
        rcsw.message.sAddress.digits[digitCount] = (uint8_t) uct;
    }

    status = p.readInt32(&t);
    rcsw.message.sSubAddress.subaddressType = (RIL_CDMA_SMS_SubaddressType) t;

    status = p.read(&uct,sizeof(uct));
    rcsw.message.sSubAddress.odd = (uint8_t) uct;

    status = p.read(&uct,sizeof(uct));
    rcsw.message.sSubAddress.number_of_digits = (uint8_t) uct;

    digitLimit = MIN((rcsw.message.sSubAddress.number_of_digits), RIL_CDMA_SMS_SUBADDRESS_MAX);

    for(digitCount = 0 ; digitCount < digitLimit; digitCount ++) {
        status = p.read(&uct,sizeof(uct));
        rcsw.message.sSubAddress.digits[digitCount] = (uint8_t) uct;
    }

    status = p.readInt32(&t);
    rcsw.message.uBearerDataLen = (int) t;

    digitLimit = MIN((rcsw.message.uBearerDataLen), RIL_CDMA_SMS_BEARER_DATA_MAX);

    for(digitCount = 0 ; digitCount < digitLimit; digitCount ++) {
        status = p.read(&uct, sizeof(uct));
        rcsw.message.aBearerData[digitCount] = (uint8_t) uct;
    }

    if (status != NO_ERROR) {
        goto invalid;
    }

    startRequest;
    appendPrintBuf("%sstatus=%d, message.uTeleserviceID=%d, message.bIsServicePresent=%d, \
            message.uServicecategory=%d, message.sAddress.digit_mode=%d, \
            message.sAddress.number_mode=%d, \
            message.sAddress.number_type=%d, ",
            printBuf, rcsw.status, rcsw.message.uTeleserviceID, rcsw.message.bIsServicePresent,
            rcsw.message.uServicecategory, rcsw.message.sAddress.digit_mode,
            rcsw.message.sAddress.number_mode,
            rcsw.message.sAddress.number_type);
    closeRequest;

    printRequest(pRI->token, pRI->pCI->requestNumber);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &rcsw, sizeof(rcsw),pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memset(&rcsw, 0, sizeof(rcsw));
#endif

    return;

invalid:
    invalidCommandBlock(pRI);
    return;

}

// For backwards compatibility in RIL_REQUEST_SETUP_DATA_CALL.
// Version 4 of the RIL interface adds a new PDP type parameter to support
// IPv6 and dual-stack PDP contexts. When dealing with a previous version of
// RIL, remove the parameter from the request.
void dispatchDataCall(Parcel& p, RequestInfo *pRI) {
    // In RIL v3, REQUEST_SETUP_DATA_CALL takes 6 parameters.
    const int numParamsRilV3 = 6;

    // The first bytes of the RIL parcel contain the request number and the
    // serial number - see processCommandBuffer(). Copy them over too.
    int pos = p.dataPosition();

    int numParams = p.readInt32();
    if (s_callbacks.version < 4 && numParams > numParamsRilV3) {
      Parcel p2;
      p2.appendFrom(&p, 0, pos);
      p2.writeInt32(numParamsRilV3);
      for(int i = 0; i < numParamsRilV3; i++) {
        p2.writeString16(p.readString16());
      }
      p2.setDataPosition(pos);
      dispatchStrings(p2, pRI);
    } else {
      p.setDataPosition(pos);
      dispatchStrings(p, pRI);
    }
}

// For backwards compatibility with RILs that dont support RIL_REQUEST_VOICE_RADIO_TECH.
// When all RILs handle this request, this function can be removed and
// the request can be sent directly to the RIL using dispatchVoid.
void dispatchVoiceRadioTech(Parcel& p __unused, RequestInfo *pRI __unused) {
#if 0
    RIL_RadioState state = CALL_ONSTATEREQUEST((RIL_SOCKET_ID)pRI->socket_id);

    if ((RADIO_STATE_UNAVAILABLE == state) || (RADIO_STATE_OFF == state)) {
//        RIL_onRequestComplete(pRI, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
    }

    // RILs that support RADIO_STATE_ON should support this request.
    if (RADIO_STATE_ON == state) {
        dispatchVoid(p, pRI);
        return;
    }

    // For Older RILs, that do not support RADIO_STATE_ON, assume that they
    // will not support this new request either and decode Voice Radio Technology
    // from Radio State
    voiceRadioTech = decodeVoiceRadioTechnology(state);

    if (voiceRadioTech < 0)
//        RIL_onRequestComplete(pRI, RIL_E_GENERIC_FAILURE, NULL, 0);
    else
//        RIL_onRequestComplete(pRI, RIL_E_SUCCESS, &voiceRadioTech, sizeof(int));
#endif
}

// For backwards compatibility in RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE:.
// When all RILs handle this request, this function can be removed and
// the request can be sent directly to the RIL using dispatchVoid.
void dispatchCdmaSubscriptionSource(Parcel& p __unused, RequestInfo *pRI __unused) {
#if 0
    RIL_RadioState state = CALL_ONSTATEREQUEST((RIL_SOCKET_ID)pRI->socket_id);

    if ((RADIO_STATE_UNAVAILABLE == state) || (RADIO_STATE_OFF == state)) {
//        RIL_onRequestComplete(pRI, RIL_E_RADIO_NOT_AVAILABLE, NULL, 0);
    }

    // RILs that support RADIO_STATE_ON should support this request.
    if (RADIO_STATE_ON == state) {
        dispatchVoid(p, pRI);
        return;
    }

    // For Older RILs, that do not support RADIO_STATE_ON, assume that they
    // will not support this new request either and decode CDMA Subscription Source
    // from Radio State
    cdmaSubscriptionSource = decodeCdmaSubscriptionSource(state);

    if (cdmaSubscriptionSource < 0)
//        RIL_onRequestComplete(pRI, RIL_E_GENERIC_FAILURE, NULL, 0);
    else
//        RIL_onRequestComplete(pRI, RIL_E_SUCCESS, &cdmaSubscriptionSource, sizeof(int));
#endif
}

void dispatchSetInitialAttachApn(Parcel &p, RequestInfo *pRI)
{
    RIL_InitialAttachApn_v15 pf;
    int32_t  t;
    status_t status;

    memset(&pf, 0, sizeof(pf));

    pf.apn = strdupReadString(p);
    pf.protocol = strdupReadString(p);
    pf.roamingProtocol = strdupReadString(p);
    status = p.readInt32(&t);
    pf.authtype = (int) t;

    pf.username = strdupReadString(p);
    pf.password = strdupReadString(p);

    status = p.readInt32(&t);
    pf.supportedTypesBitmask = (int) t;
    status = p.readInt32(&t);
    pf.bearerBitmask = (int) t;
    status = p.readInt32(&t);
    pf.modemCognitive = (int) t;
    status = p.readInt32(&t);
    pf.mtu = (int) t;

    pf.mvnoType = strdupReadString(p);
    pf.mvnoMatchData = strdupReadString(p);
    status = p.readInt32(&t);
    pf.canHandleIms= (int) t;
    startRequest;
    appendPrintBuf("%sapn=%s, protocol=%s, roamingProtocol=%s, authtype=%d, username=%s, \
password=%s, TypesBitmask=%d, bearerBitmask=%d, modemCognitive=%d, mtu=%d, mvnoType=%s, \
mvnoMatchData=%s, canHandleIms=%d",
            printBuf, pf.apn, pf.protocol, pf.roamingProtocol, pf.authtype, pf.username,
            pf.password, pf.supportedTypesBitmask, pf.bearerBitmask, pf.modemCognitive,
            pf.mtu, pf.mvnoType, pf.mvnoMatchData, pf.canHandleIms);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    if (status != NO_ERROR) {
        goto invalid;
    }
    CALL_ONREQUEST(pRI->pCI->requestNumber, &pf, sizeof(pf), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memsetString(pf.apn);
    memsetString(pf.protocol);
    memsetString(pf.roamingProtocol);
    memsetString(pf.username);
    memsetString(pf.password);
    memsetString(pf.mvnoType);
    memsetString(pf.mvnoMatchData);
#endif

    free(pf.apn);
    free(pf.protocol);
    free(pf.roamingProtocol);
    free(pf.username);
    free(pf.password);
    free(pf.mvnoType);
    free(pf.mvnoMatchData);

#ifdef MEMSET_FREED
    memset(&pf, 0, sizeof(pf));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

void dispatchNVReadItem(Parcel &p, RequestInfo *pRI) {
    RIL_NV_ReadItem nvri;
    int32_t  t;
    status_t status;

    memset(&nvri, 0, sizeof(nvri));

    status = p.readInt32(&t);
    nvri.itemID = (RIL_NV_Item) t;

    if (status != NO_ERROR) {
        goto invalid;
    }

    startRequest;
    appendPrintBuf("%snvri.itemID=%d, ", printBuf, nvri.itemID);
    closeRequest;

    printRequest(pRI->token, pRI->pCI->requestNumber);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &nvri, sizeof(nvri), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memset(&nvri, 0, sizeof(nvri));
#endif

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

void dispatchNVWriteItem(Parcel &p, RequestInfo *pRI) {
    RIL_NV_WriteItem nvwi;
    int32_t  t;
    status_t status;

    memset(&nvwi, 0, sizeof(nvwi));

    status = p.readInt32(&t);
    nvwi.itemID = (RIL_NV_Item) t;

    nvwi.value = strdupReadString(p);

    if (status != NO_ERROR || nvwi.value == NULL) {
        goto invalid;
    }

    startRequest;
    appendPrintBuf("%snvwi.itemID=%d, value=%s, ", printBuf, nvwi.itemID,
            nvwi.value);
    closeRequest;

    printRequest(pRI->token, pRI->pCI->requestNumber);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &nvwi, sizeof(nvwi), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memsetString(nvwi.value);
#endif

    free(nvwi.value);

#ifdef MEMSET_FREED
    memset(&nvwi, 0, sizeof(nvwi));
#endif

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}


void dispatchUiccSubscripton(Parcel &p, RequestInfo *pRI) {
    RIL_SelectUiccSub uicc_sub;
    status_t status;
    int32_t  t;
    memset(&uicc_sub, 0, sizeof(uicc_sub));

    status = p.readInt32(&t);
    if (status != NO_ERROR) {
        goto invalid;
    }
    uicc_sub.slot = (int) t;

    status = p.readInt32(&t);
    if (status != NO_ERROR) {
        goto invalid;
    }
    uicc_sub.app_index = (int) t;

    status = p.readInt32(&t);
    if (status != NO_ERROR) {
        goto invalid;
    }
    uicc_sub.sub_type = (RIL_SubscriptionType) t;

    status = p.readInt32(&t);
    if (status != NO_ERROR) {
        goto invalid;
    }
    uicc_sub.act_status = (RIL_UiccSubActStatus) t;

    startRequest;
    appendPrintBuf("slot=%d, app_index=%d, act_status = %d", uicc_sub.slot, uicc_sub.app_index,
            uicc_sub.act_status);
    RFX_LOG_D(RFX_LOG_TAG, "dispatchUiccSubscription, slot=%d, app_index=%d, act_status = %d", uicc_sub.slot,
            uicc_sub.app_index, uicc_sub.act_status);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    CALL_ONREQUEST(pRI->pCI->requestNumber, &uicc_sub, sizeof(uicc_sub), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memset(&uicc_sub, 0, sizeof(uicc_sub));
#endif
    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

void dispatchSimAuthentication(Parcel &p, RequestInfo *pRI)
{
    RIL_SimAuthentication pf;
    int32_t  t;
    status_t status;

    memset(&pf, 0, sizeof(pf));

    status = p.readInt32(&t);
    pf.authContext = (int) t;
    pf.authData = strdupReadString(p);
    pf.aid = strdupReadString(p);

    startRequest;
    appendPrintBuf("authContext=%s, authData=%s, aid=%s", pf.authContext, pf.authData, pf.aid);
    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    if (status != NO_ERROR) {
        goto invalid;
    }
    CALL_ONREQUEST(pRI->pCI->requestNumber, &pf, sizeof(pf), pRI, pRI->socket_id);

#ifdef MEMSET_FREED
    memsetString(pf.authData);
    memsetString(pf.aid);
#endif

    free(pf.authData);
    free(pf.aid);

#ifdef MEMSET_FREED
    memset(&pf, 0, sizeof(pf));
#endif

    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

void dispatchDataProfile(Parcel &p, RequestInfo *pRI) {
    int32_t t;
    status_t status;
    int32_t num;

    status = p.readInt32(&num);
    if (status != NO_ERROR || num < 0) {
        goto invalid;
    }

    {
        RIL_DataProfileInfo *dataProfiles =
                (RIL_DataProfileInfo *)calloc(num, sizeof(RIL_DataProfileInfo));
        if (dataProfiles == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "Memory allocation failed for request %s",
                    requestToString(pRI->pCI->requestNumber));
            return;
        }
        RIL_DataProfileInfo **dataProfilePtrs =
                (RIL_DataProfileInfo **)calloc(num, sizeof(RIL_DataProfileInfo *));
        if (dataProfilePtrs == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "Memory allocation failed for request %s",
                    requestToString(pRI->pCI->requestNumber));
            free(dataProfiles);
            return;
        }

        startRequest;
        for (int i = 0 ; i < num ; i++ ) {
            dataProfilePtrs[i] = &dataProfiles[i];

            status = p.readInt32(&t);
            dataProfiles[i].profileId = (int) t;

            dataProfiles[i].apn = strdupReadString(p);
            dataProfiles[i].protocol = strdupReadString(p);
            status = p.readInt32(&t);
            dataProfiles[i].authType = (int) t;

            dataProfiles[i].user = strdupReadString(p);
            dataProfiles[i].password = strdupReadString(p);

            status = p.readInt32(&t);
            dataProfiles[i].type = (int) t;

            status = p.readInt32(&t);
            dataProfiles[i].maxConnsTime = (int) t;
            status = p.readInt32(&t);
            dataProfiles[i].maxConns = (int) t;
            status = p.readInt32(&t);
            dataProfiles[i].waitTime = (int) t;

            status = p.readInt32(&t);
            dataProfiles[i].enabled = (int) t;

            appendPrintBuf("%s [%d: profileId=%d, apn =%s, protocol =%s, authType =%d, \
                  user =%s, password =%s, type =%d, maxConnsTime =%d, maxConns =%d, \
                  waitTime =%d, enabled =%d]", printBuf, i, dataProfiles[i].profileId,
                  dataProfiles[i].apn, dataProfiles[i].protocol, dataProfiles[i].authType,
                  dataProfiles[i].user, dataProfiles[i].password, dataProfiles[i].type,
                  dataProfiles[i].maxConnsTime, dataProfiles[i].maxConns,
                  dataProfiles[i].waitTime, dataProfiles[i].enabled);
        }
        closeRequest;
        printRequest(pRI->token, pRI->pCI->requestNumber);

        if (status != NO_ERROR) {
            free(dataProfiles);
            free(dataProfilePtrs);
            goto invalid;
        }
        CALL_ONREQUEST(pRI->pCI->requestNumber,
                              dataProfilePtrs,
                              num * sizeof(RIL_DataProfileInfo *),
                              pRI, pRI->socket_id);

#ifdef MEMSET_FREED
        memset(dataProfiles, 0, num * sizeof(RIL_DataProfileInfo));
        memset(dataProfilePtrs, 0, num * sizeof(RIL_DataProfileInfo *));
#endif
        free(dataProfiles);
        free(dataProfilePtrs);
    }

    return;

invalid:
    invalidCommandBlock(pRI);
    return;
}

void dispatchRadioCapability(Parcel &p, RequestInfo *pRI){
    RIL_RadioCapability rc;
    int32_t t;
    status_t status;

    memset (&rc, 0, sizeof(RIL_RadioCapability));

    status = p.readInt32(&t);
    rc.version = (int)t;
    if (status != NO_ERROR) {
        goto invalid;
    }

    status = p.readInt32(&t);
    rc.session= (int)t;
    if (status != NO_ERROR) {
        goto invalid;
    }

    status = p.readInt32(&t);
    rc.phase= (int)t;
    if (status != NO_ERROR) {
        goto invalid;
    }

    status = p.readInt32(&t);
    rc.rat = (int)t;
    if (status != NO_ERROR) {
        goto invalid;
    }

    status = readStringFromParcelInplace(p, rc.logicalModemUuid, sizeof(rc.logicalModemUuid));
    if (status != NO_ERROR) {
        goto invalid;
    }

    status = p.readInt32(&t);
    rc.status = (int)t;

    if (status != NO_ERROR) {
        goto invalid;
    }

    startRequest;
    appendPrintBuf("%s [version:%d, session:%d, phase:%d, rat:%d, \
            logicalModemUuid:%s, status:%d", printBuf, rc.version, rc.session,
            rc.phase, rc.rat, rc.logicalModemUuid, rc.session);

    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    CALL_ONREQUEST(pRI->pCI->requestNumber,
                &rc,
                sizeof(RIL_RadioCapability),
                pRI, pRI->socket_id);
    return;
invalid:
    invalidCommandBlock(pRI);
    return;
}

/**
 * Callee expects const RIL_CarrierRestrictionsWithPriority *
 */
void dispatchCarrierRestrictions(Parcel &p, RequestInfo *pRI) {
    RIL_CarrierRestrictionsWithPriority cr;
    RIL_Carrier * allowed_carriers = NULL;
    RIL_Carrier * excluded_carriers = NULL;
    int32_t t;
    status_t status;

    memset(&cr, 0, sizeof(RIL_CarrierRestrictionsWithPriority));

    if (s_callbacks.version < 14) {
        RFX_LOG_E(RFX_LOG_TAG, "Unsuppoted RIL version %d, min version expected %d",
              s_callbacks.version, 14);
//        RIL_onRequestComplete(pRI, RIL_E_REQUEST_NOT_SUPPORTED, NULL, 0);
        return;
    }

    status = p.readInt32(&t);
    if (status != NO_ERROR) {
        goto invalid;
    }
    allowed_carriers = (RIL_Carrier *)calloc(t, sizeof(RIL_Carrier));
    if (allowed_carriers == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "Memory allocation failed for request %s", requestToString(pRI->pCI->requestNumber));
        goto exit;
    }
    cr.len_allowed_carriers = t;
    cr.allowed_carriers = allowed_carriers;

    status = p.readInt32(&t);
    if (status != NO_ERROR) {
        goto invalid;
    }
    excluded_carriers = (RIL_Carrier *)calloc(t, sizeof(RIL_Carrier));
    if (excluded_carriers == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "Memory allocation failed for request %s", requestToString(pRI->pCI->requestNumber));
        goto exit;
    }
    cr.len_excluded_carriers = t;
    cr.excluded_carriers = excluded_carriers;

    startRequest;
    appendPrintBuf("%s len_allowed_carriers:%d, len_excluded_carriers:%d,",
                   printBuf, cr.len_allowed_carriers, cr.len_excluded_carriers);

    appendPrintBuf("%s allowed_carriers:", printBuf);
    for (int32_t i = 0; i < cr.len_allowed_carriers; i++) {
        RIL_Carrier *p_cr = allowed_carriers + i;
        p_cr->mcc = strdupReadString(p);
        p_cr->mnc = strdupReadString(p);
        status = p.readInt32(&t);
        p_cr->match_type = static_cast<RIL_CarrierMatchType>(t);
        if (status != NO_ERROR) {
            goto invalid;
        }
        p_cr->match_data = strdupReadString(p);
        appendPrintBuf("%s [%d mcc:%s, mnc:%s, match_type:%d, match_data:%s],",
                       printBuf, i, p_cr->mcc, p_cr->mnc, p_cr->match_type, p_cr->match_data);
    }

    for (int32_t i = 0; i < cr.len_excluded_carriers; i++) {
        RIL_Carrier *p_cr = excluded_carriers + i;
        p_cr->mcc = strdupReadString(p);
        p_cr->mnc = strdupReadString(p);
        status = p.readInt32(&t);
        p_cr->match_type = static_cast<RIL_CarrierMatchType>(t);
        if (status != NO_ERROR) {
            goto invalid;
        }
        p_cr->match_data = strdupReadString(p);
        appendPrintBuf("%s [%d mcc:%s, mnc:%s, match_type:%d, match_data:%s],",
                       printBuf, i, p_cr->mcc, p_cr->mnc, p_cr->match_type, p_cr->match_data);
    }

    status = p.readInt32(&t);
    if (status != NO_ERROR) {
        goto invalid;
    }
    cr.allowedCarriersPrioritized = t;

    status = p.readInt32(&t);
    if (status != NO_ERROR) {
        goto invalid;
    }
    cr.simLockMultiSimPolicy = t;

    closeRequest;
    printRequest(pRI->token, pRI->pCI->requestNumber);

    CALL_ONREQUEST(pRI->pCI->requestNumber,
                &cr,
                sizeof(RIL_CarrierRestrictionsWithPriority),
                pRI, pRI->socket_id);

    goto exit;

invalid:
    invalidCommandBlock(pRI);
//    RIL_onRequestComplete(pRI, RIL_E_INVALID_ARGUMENTS, NULL, 0);
exit:
    if (allowed_carriers != NULL) {
        free(allowed_carriers);
    }
    if (excluded_carriers != NULL) {
        free(excluded_carriers);
    }
    return;
}

void dispatchFdMode(Parcel &p, RequestInfo *pRI) {
    RIL_FdModeStructure args;
    status_t status;
    int t_value = 0;
    memset(&args, 0, sizeof(args));
    status = p.readInt32(&t_value);
    args.paramNumber = t_value;

    /* AT+EFD=<mode>[,<param1>[,<param2>]] */
    /* For all modes: but mode 0 & 1 only has one argument */
    if (args.paramNumber >= 1) {
        status = p.readInt32(&t_value);
        args.mode = t_value;
    }
    /* For mode 2 & 3 */
    if (args.paramNumber >= 2) {
        status = p.readInt32(&t_value);
        args.parameter1 = t_value;
    }
    /* Only mode 2 */
    if (args.paramNumber >=3) {
        status = p.readInt32(&t_value);
        args.parameter2 = t_value;
    }

    CALL_ONREQUEST(pRI->pCI->requestNumber, &args, sizeof(args), pRI, pRI->socket_id);
}

void dispatchLinkCapacityReportingCriteria(Parcel &p, RequestInfo *pRI) {
    status_t status;
    int t = 0;
    int num = 0;

    RIL_LinkCapacityReportingCriteria *data = (RIL_LinkCapacityReportingCriteria *) calloc(1,
            sizeof(RIL_LinkCapacityReportingCriteria));
    if (data == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "setLinkCapacityReportingCriteria: data memory allocation failed"
                " for request %d", pRI->pCI->requestNumber);
        return;
    }

    status = p.readInt32(&t);
    if (status != NO_ERROR) {
        goto invalid;
    }
    data->hysteresisMs = (int)t;

    status = p.readInt32(&t);
    if (status != NO_ERROR) {
        goto invalid;
    }
    data->hysteresisDlKbps = (int)t;

    status = p.readInt32(&t);
    if (status != NO_ERROR) {
        goto invalid;
    }
    data->hysteresisUlKbps = (int)t;

    status = p.readInt32(&t);
    if (status != NO_ERROR) {
        goto invalid;
    }
    num = (int)t;
    data->thresholdDlKbpsNumber = num > MAX_LCE_THRESHOLD_NUMBER ? MAX_LCE_THRESHOLD_NUMBER : num;
    for (int i = 0; i < num; i++) {
        status = p.readInt32(&t);
        if (status != NO_ERROR) {
            goto invalid;
        }
        if (i < MAX_LCE_THRESHOLD_NUMBER) {
            data->thresholdDlKbpsList[i] = (int)t;
        }
    }

    status = p.readInt32(&t);
    if (status != NO_ERROR) {
        goto invalid;
    }
    num = (int)t;
    data->thresholdUlKbpsNumber = num > MAX_LCE_THRESHOLD_NUMBER ? MAX_LCE_THRESHOLD_NUMBER : num;
    for (int i = 0; i < num; i++) {
        status = p.readInt32(&t);
        if (status != NO_ERROR) {
            goto invalid;
        }
        if (i < MAX_LCE_THRESHOLD_NUMBER) {
            data->thresholdUlKbpsList[i] = (int)t;
        }
    }

    status = p.readInt32(&t);
    if (status != NO_ERROR) {
        goto invalid;
    }
    data->accessNetwork = (int)t;

    CALL_ONREQUEST(pRI->pCI->requestNumber, data, sizeof(RIL_LinkCapacityReportingCriteria), pRI,
            pRI->socket_id);

    free(data);
    return;

invalid:
    RFX_LOG_E(RFX_LOG_TAG, "setLinkCapacityReportingCriteria: invalidCommandBlock"
                " for request %d", pRI->pCI->requestNumber);
    free(data);
    invalidCommandBlock(pRI);
    return;
}

int
responseInts(Parcel &p, void *response, size_t responselen) {
    int numInts;

    if (response == NULL && responselen != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }
    if (responselen % sizeof(int) != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "responseInts: invalid response length %d expected multiple of %d\n",
            (int)responselen, (int)sizeof(int));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    int *p_int = (int *) response;

    numInts = responselen / sizeof(int);
    p.writeInt32 (numInts);

    /* each int*/
    startResponse;
    for (int i = 0 ; i < numInts ; i++) {
        appendPrintBuf("%s%d,", printBuf, p_int[i]);
        p.writeInt32(p_int[i]);
    }
    removeLastChar;
    closeResponse;

    return 0;
}

// Response is an int or RIL_LastCallFailCauseInfo.
// Currently, only Shamu plans to use RIL_LastCallFailCauseInfo.
// TODO(yjl): Let all implementations use RIL_LastCallFailCauseInfo.
int responseFailCause(Parcel &p, void *response, size_t responselen) {
    if (response == NULL && responselen != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen == sizeof(int)) {
      startResponse;
      int *p_int = (int *) response;
      appendPrintBuf("%s%d,", printBuf, p_int[0]);
      p.writeInt32(p_int[0]);
      removeLastChar;
      closeResponse;
    } else if (responselen == sizeof(RIL_LastCallFailCauseInfo)) {
      startResponse;
      RIL_LastCallFailCauseInfo *p_fail_cause_info = (RIL_LastCallFailCauseInfo *) response;
      appendPrintBuf("%s[cause_code=%d,vendor_cause=%s]", printBuf, p_fail_cause_info->cause_code,
                     p_fail_cause_info->vendor_cause);
      p.writeInt32(p_fail_cause_info->cause_code);
      writeStringToParcel(p, p_fail_cause_info->vendor_cause);
      removeLastChar;
      closeResponse;
    } else {
      RFX_LOG_E(RFX_LOG_TAG, "responseFailCause: invalid response length %d expected an int or "
            "RIL_LastCallFailCauseInfo", (int)responselen);
      return RIL_ERRNO_INVALID_RESPONSE;
    }

    return 0;
}

/** response is a char **, pointing to an array of char *'s
    The parcel will begin with the version */
int responseStringsWithVersion(int version, Parcel &p, void *response, size_t responselen) {
    p.writeInt32(version);
    return responseStrings(p, response, responselen);
}

/** response is a char **, pointing to an array of char *'s */
int responseStrings(Parcel &p, void *response, size_t responselen) {
    int numStrings;

    if (response == NULL && responselen != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }
    if (responselen % sizeof(char *) != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "responseStrings: invalid response length %d expected multiple of %d\n",
            (int)responselen, (int)sizeof(char *));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RFX_LOG_E(RFX_LOG_TAG, "responseStrings weilun");

    if (response == NULL) {
        p.writeInt32 (0);
    } else {
        char **p_cur = (char **) response;

        numStrings = responselen / sizeof(char *);
        p.writeInt32 (numStrings);

        /* each string*/
        startResponse;
        for (int i = 0 ; i < numStrings ; i++) {
            appendPrintBuf("%s%s,", printBuf, (char*)p_cur[i]);
            writeStringToParcel (p, p_cur[i]);
        }
        removeLastChar;
        closeResponse;
    }
    return 0;
}


/**
 * NULL strings are accepted
 * FIXME currently ignores responselen
 */
int responseString(Parcel &p, void *response, size_t responselen __unused) {
    /* one string only */
    startResponse;
    appendPrintBuf("%s%s", printBuf, (char*)response);
    closeResponse;

    writeStringToParcel(p, (const char *)response);

    return 0;
}

int responseVoid(Parcel &p __unused, void *response __unused, size_t responselen __unused) {
    startResponse;
    removeLastChar;
    return 0;
}

int responseCallList(Parcel &p, void *response, size_t responselen) {
    int num;

    if (response == NULL && responselen != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof (RIL_Call *) != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "responseCallList: invalid response length %d expected multiple of %d\n",
            (int)responselen, (int)sizeof (RIL_Call *));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    startResponse;
    /* number of call info's */
    num = responselen / sizeof(RIL_Call *);
    p.writeInt32(num);

    for (int i = 0 ; i < num ; i++) {
        RIL_Call *p_cur = ((RIL_Call **) response)[i];
        /* each call info */
        p.writeInt32(p_cur->state);
        p.writeInt32(p_cur->index);
        p.writeInt32(p_cur->toa);
        p.writeInt32(p_cur->isMpty);
        p.writeInt32(p_cur->isMT);
        p.writeInt32(p_cur->als);
        p.writeInt32(p_cur->isVoice);
        p.writeInt32(p_cur->isVoicePrivacy);
        writeStringToParcel(p, p_cur->number);
        p.writeInt32(p_cur->numberPresentation);
        writeStringToParcel(p, p_cur->name);
        p.writeInt32(p_cur->namePresentation);
        // Remove when partners upgrade to version 3
        if ((s_callbacks.version < 3) || (p_cur->uusInfo == NULL || p_cur->uusInfo->uusData == NULL)) {
            p.writeInt32(0); /* UUS Information is absent */
        } else {
            RIL_UUS_Info *uusInfo = p_cur->uusInfo;
            p.writeInt32(1); /* UUS Information is present */
            p.writeInt32(uusInfo->uusType);
            p.writeInt32(uusInfo->uusDcs);
            p.writeInt32(uusInfo->uusLength);
            p.write(uusInfo->uusData, uusInfo->uusLength);
        }
        appendPrintBuf("%s[id=%d,%s,toa=%d,",
            printBuf,
            p_cur->index,
            callStateToString(p_cur->state),
            p_cur->toa);
        appendPrintBuf("%s%s,%s,als=%d,%s,%s,",
            printBuf,
            (p_cur->isMpty)?"conf":"norm",
            (p_cur->isMT)?"mt":"mo",
            p_cur->als,
            (p_cur->isVoice)?"voc":"nonvoc",
            (p_cur->isVoicePrivacy)?"evp":"noevp");
        appendPrintBuf("%s%s,cli=%d,name='%s',%d]",
            printBuf,
            p_cur->number,
            p_cur->numberPresentation,
            p_cur->name,
            p_cur->namePresentation);
    }
    removeLastChar;
    closeResponse;

    return 0;
}

int responseSMS(Parcel &p, void *response, size_t responselen) {
    if (response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof (RIL_SMS_Response) ) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response length %d expected %d",
                (int)responselen, (int)sizeof (RIL_SMS_Response));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_SMS_Response *p_cur = (RIL_SMS_Response *) response;

    p.writeInt32(p_cur->messageRef);
    writeStringToParcel(p, p_cur->ackPDU);
    p.writeInt32(p_cur->errorCode);

    startResponse;
    appendPrintBuf("%s%d,%s,%d", printBuf, p_cur->messageRef,
        (char*)p_cur->ackPDU, p_cur->errorCode);
    closeResponse;

    return 0;
}

int responseDataCallListV4(Parcel &p, void *response, size_t responselen)
{
    if (response == NULL && responselen != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof(RIL_Data_Call_Response_v4) != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "responseDataCallListV4: invalid response length %d expected multiple of %d",
                (int)responselen, (int)sizeof(RIL_Data_Call_Response_v4));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    // Write version
    p.writeInt32(4);

    int num = responselen / sizeof(RIL_Data_Call_Response_v4);
    p.writeInt32(num);

    RIL_Data_Call_Response_v4 *p_cur = (RIL_Data_Call_Response_v4 *) response;
    startResponse;
    int i;
    for (i = 0; i < num; i++) {
        p.writeInt32(p_cur[i].cid);
        p.writeInt32(p_cur[i].active);
        writeStringToParcel(p, p_cur[i].type);
        // apn is not used, so don't send.
        writeStringToParcel(p, p_cur[i].address);
        appendPrintBuf("%s[cid=%d,%s,%s,%s],", printBuf,
            p_cur[i].cid,
            (p_cur[i].active==0)?"down":"up",
            (char*)p_cur[i].type,
            (char*)p_cur[i].address);
    }
    removeLastChar;
    closeResponse;

    return 0;
}

int responseDataCallListV6(Parcel &p, void *response, size_t responselen)
{
    if (response == NULL && responselen != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof(RIL_Data_Call_Response_v6) != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "responseDataCallListV6: invalid response length %d expected multiple of %d",
                (int)responselen, (int)sizeof(RIL_Data_Call_Response_v6));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    // Write version
    p.writeInt32(6);

    int num = responselen / sizeof(RIL_Data_Call_Response_v6);
    p.writeInt32(num);

    RIL_Data_Call_Response_v6 *p_cur = (RIL_Data_Call_Response_v6 *) response;
    startResponse;
    int i;
    for (i = 0; i < num; i++) {
        p.writeInt32((int)p_cur[i].status);
        p.writeInt32(p_cur[i].suggestedRetryTime);
        p.writeInt32(p_cur[i].cid);
        p.writeInt32(p_cur[i].active);
        writeStringToParcel(p, p_cur[i].type);
        writeStringToParcel(p, p_cur[i].ifname);
        writeStringToParcel(p, p_cur[i].addresses);
        writeStringToParcel(p, p_cur[i].dnses);
        writeStringToParcel(p, p_cur[i].gateways);
        appendPrintBuf("%s[status=%d,retry=%d,cid=%d,%s,%s,%s,%s,%s,%s],", printBuf,
            p_cur[i].status,
            p_cur[i].suggestedRetryTime,
            p_cur[i].cid,
            (p_cur[i].active==0)?"down":"up",
            (char*)p_cur[i].type,
            (char*)p_cur[i].ifname,
            (char*)p_cur[i].addresses,
            (char*)p_cur[i].dnses,
            (char*)p_cur[i].gateways);
    }
    removeLastChar;
    closeResponse;

    return 0;
}

int responseDataCallListV9(Parcel &p, void *response, size_t responselen)
{
    if (response == NULL && responselen != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof(RIL_Data_Call_Response_v9) != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "responseDataCallListV9: invalid response length %d expected multiple of %d",
                (int)responselen, (int)sizeof(RIL_Data_Call_Response_v9));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    // Write version
    p.writeInt32(10);

    int num = responselen / sizeof(RIL_Data_Call_Response_v9);
    p.writeInt32(num);

    RIL_Data_Call_Response_v9 *p_cur = (RIL_Data_Call_Response_v9 *) response;
    startResponse;
    int i;
    for (i = 0; i < num; i++) {
        p.writeInt32((int)p_cur[i].status);
        p.writeInt32(p_cur[i].suggestedRetryTime);
        p.writeInt32(p_cur[i].cid);
        p.writeInt32(p_cur[i].active);
        writeStringToParcel(p, p_cur[i].type);
        writeStringToParcel(p, p_cur[i].ifname);
        writeStringToParcel(p, p_cur[i].addresses);
        writeStringToParcel(p, p_cur[i].dnses);
        writeStringToParcel(p, p_cur[i].gateways);
        writeStringToParcel(p, p_cur[i].pcscf);
        appendPrintBuf("%s[status=%d,retry=%d,cid=%d,%s,%s,%s,%s,%s,%s,%s],", printBuf,
            p_cur[i].status,
            p_cur[i].suggestedRetryTime,
            p_cur[i].cid,
            (p_cur[i].active==0)?"down":"up",
            (char*)p_cur[i].type,
            (char*)p_cur[i].ifname,
            (char*)p_cur[i].addresses,
            (char*)p_cur[i].dnses,
            (char*)p_cur[i].gateways,
            (char*)p_cur[i].pcscf);
    }
    removeLastChar;
    closeResponse;

    return 0;
}

int responseDataCallListV11(Parcel &p, void *response, size_t responselen) {
    if (response == NULL && responselen != 0) {
                RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
                return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof(MTK_RIL_Data_Call_Response_v11) != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response length %d expected multiple of %d",
        (int)responselen, (int)sizeof(MTK_RIL_Data_Call_Response_v11));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    // Write version
    p.writeInt32(11);

    int num = responselen / sizeof(MTK_RIL_Data_Call_Response_v11);
    p.writeInt32(num);

    MTK_RIL_Data_Call_Response_v11 *p_cur = (MTK_RIL_Data_Call_Response_v11 *) response;
    startResponse;
    int i;
    for (i = 0; i < num; i++) {
        p.writeInt32((int)p_cur[i].status);
        p.writeInt32(p_cur[i].suggestedRetryTime);
        p.writeInt32(p_cur[i].cid);
        p.writeInt32(p_cur[i].active);
        writeStringToParcel(p, p_cur[i].type);
        writeStringToParcel(p, p_cur[i].ifname);
        writeStringToParcel(p, p_cur[i].addresses);
        writeStringToParcel(p, p_cur[i].dnses);
        writeStringToParcel(p, p_cur[i].gateways);
        writeStringToParcel(p, p_cur[i].pcscf);
        p.writeInt32(p_cur[i].mtu);
        p.writeInt32(p_cur[i].rat);
        appendPrintBuf("%s[status=%d,retry=%d,cid=%d,%s,%s,%s,%s,%s,%s,%s,%d,rat=%d],", printBuf,
        p_cur[i].status,
        p_cur[i].suggestedRetryTime,
        p_cur[i].cid,
        (p_cur[i].active==0)?"down":"up",
        (char*)p_cur[i].type,
        (char*)p_cur[i].ifname,
        (char*)p_cur[i].addresses,
        (char*)p_cur[i].dnses,
        (char*)p_cur[i].gateways,
        (char*)p_cur[i].pcscf,
        p_cur[i].mtu,
        p_cur[i].rat);
    }
    removeLastChar;
    closeResponse;

    return 0;
}

int responseDataCallList(Parcel &p, void *response, size_t responselen)
{
    if (s_callbacks.version <= LAST_IMPRECISE_RIL_VERSION) {
        if (s_callbacks.version < 5) {
            RFX_LOG_D(RFX_LOG_TAG, "responseDataCallList: v4");
            return responseDataCallListV4(p, response, responselen);
        } else if (responselen % sizeof(RIL_Data_Call_Response_v6) == 0) {
            return responseDataCallListV6(p, response, responselen);
        } else if (responselen % sizeof(RIL_Data_Call_Response_v9) == 0) {
            return responseDataCallListV9(p, response, responselen);
        } else {
            return responseDataCallListV11(p, response, responselen);
        }
    } else { // RIL version >= 13
        if (responselen % sizeof(MTK_RIL_Data_Call_Response_v11) != 0) {
            RFX_LOG_E(RFX_LOG_TAG, "Data structure expected is MTK_RIL_Data_Call_Response_v11");
#if 0
            if (!isDebuggable()) {
                return RIL_ERRNO_INVALID_RESPONSE;
            } else {
                assert(0);
            }
#endif
        }
        return responseDataCallListV11(p, response, responselen);
    }
}

int responseSetupDataCall(Parcel &p, void *response, size_t responselen)
{
    if (s_callbacks.version < 5) {
        return responseStringsWithVersion(s_callbacks.version, p, response, responselen);
    } else {
        return responseDataCallList(p, response, responselen);
    }
}

int responseRaw(Parcel &p, void *response, size_t responselen) {
    if (response == NULL && responselen != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL with responselen != 0");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    // The java code reads -1 size as null byte array
    if (response == NULL) {
        p.writeInt32(-1);
    } else {
        p.writeInt32(responselen);
        p.write(response, responselen);
    }

    return 0;
}


int responseSIM_IO(Parcel &p, void *response, size_t responselen) {
    if (response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof (RIL_SIM_IO_Response) ) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response length was %d expected %d",
                (int)responselen, (int)sizeof (RIL_SIM_IO_Response));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_SIM_IO_Response *p_cur = (RIL_SIM_IO_Response *) response;
    p.writeInt32(p_cur->sw1);
    p.writeInt32(p_cur->sw2);
    writeStringToParcel(p, p_cur->simResponse);

    startResponse;
    appendPrintBuf("%ssw1=0x%X,sw2=0x%X,%s", printBuf, p_cur->sw1, p_cur->sw2,
        (char*)p_cur->simResponse);
    closeResponse;


    return 0;
}

int responseCallForwards(Parcel &p __unused, void *response __unused,
        size_t responselen __unused) {
#if 0
    int num;

    if (response == NULL && responselen != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof(RIL_CallForwardInfo *) != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "responseCallForwards: invalid response length %d expected multiple of %d",
                (int)responselen, (int)sizeof(RIL_CallForwardInfo *));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    /* number of call info's */
    num = responselen / sizeof(RIL_CallForwardInfo *);
    p.writeInt32(num);

    startResponse;
    for (int i = 0 ; i < num ; i++) {
        RIL_CallForwardInfo *p_cur = ((RIL_CallForwardInfo **) response)[i];

        p.writeInt32(p_cur->status);
        p.writeInt32(p_cur->reason);
        p.writeInt32(p_cur->serviceClass);
        p.writeInt32(p_cur->toa);
        writeStringToParcel(p, p_cur->number);
        p.writeInt32(p_cur->timeSeconds);
        appendPrintBuf("%s[%s,reason=%d,cls=%d,toa=%d,%s,tout=%d],", printBuf,
            (p_cur->status==1)?"enable":"disable",
            p_cur->reason, p_cur->serviceClass, p_cur->toa,
            (char*)p_cur->number,
            p_cur->timeSeconds);
    }
    removeLastChar;
    closeResponse;
#endif
    return 0;
}

int responseCallForwardsEx(Parcel &p __unused, void *response __unused,
        size_t responselen __unused) {
#if 0
    int num;

    if (response == NULL && responselen != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof(RIL_CallForwardInfoEx *) != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "responseCallForwardsEx: invalid response length %d expected multiple of %d",
                (int)responselen, (int)sizeof(RIL_CallForwardInfoEx *));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    /* number of call info's */
    num = responselen / sizeof(RIL_CallForwardInfoEx *);
    p.writeInt32(num);

    startResponse;
    for (int i = 0 ; i < num ; i++) {
        RIL_CallForwardInfoEx *p_cur = ((RIL_CallForwardInfoEx **) response)[i];

        p.writeInt32(p_cur->status);
        p.writeInt32(p_cur->reason);
        p.writeInt32(p_cur->serviceClass);
        p.writeInt32(p_cur->toa);
        writeStringToParcel(p, p_cur->number);
        p.writeInt32(p_cur->timeSeconds);

        writeStringToParcel(p, p_cur->timeSlotBegin);
        writeStringToParcel(p, p_cur->timeSlotEnd);

        appendPrintBuf("%s[%s,reason=%d,cls=%d,toa=%d,%s,tout=%d,timeSlot=%s,%s],", printBuf,
            (p_cur->status==1)?"enable":"disable",
            p_cur->reason, p_cur->serviceClass, p_cur->toa,
            (char*)p_cur->number,
            p_cur->timeSeconds,
            p_cur->timeSlotBegin,
            p_cur->timeSlotEnd);
    }
    removeLastChar;
    closeResponse;
#endif
    return 0;
}

int responseSsn(Parcel &p, void *response, size_t responselen) {
    if (response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof(RIL_SuppSvcNotification)) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response length was %d expected %d",
                (int)responselen, (int)sizeof (RIL_SuppSvcNotification));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_SuppSvcNotification *p_cur = (RIL_SuppSvcNotification *) response;
    p.writeInt32(p_cur->notificationType);
    p.writeInt32(p_cur->code);
    p.writeInt32(p_cur->index);
    p.writeInt32(p_cur->type);
    writeStringToParcel(p, p_cur->number);

    startResponse;
    appendPrintBuf("%s%s,code=%d,id=%d,type=%d,%s", printBuf,
        (p_cur->notificationType==0)?"mo":"mt",
         p_cur->code, p_cur->index, p_cur->type,
        (char*)p_cur->number);
    closeResponse;

    return 0;
}

int responseCellList(Parcel &p, void *response, size_t responselen) {
    int num;

    if (response == NULL && responselen != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof (RIL_NeighboringCell *) != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "responseCellList: invalid response length %d expected multiple of %d\n",
            (int)responselen, (int)sizeof (RIL_NeighboringCell *));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    startResponse;
    /* number of records */
    num = responselen / sizeof(RIL_NeighboringCell *);
    p.writeInt32(num);

    for (int i = 0 ; i < num ; i++) {
        RIL_NeighboringCell *p_cur = ((RIL_NeighboringCell **) response)[i];

        p.writeInt32(p_cur->rssi);
        writeStringToParcel (p, p_cur->cid);

        appendPrintBuf("%s[cid=%s,rssi=%d],", printBuf,
            p_cur->cid, p_cur->rssi);
    }
    removeLastChar;
    closeResponse;

    return 0;
}

/**
 * Marshall the signalInfoRecord into the parcel if it exists.
 */
void marshallSignalInfoRecord(Parcel &p,
            RIL_CDMA_SignalInfoRecord &p_signalInfoRecord) {
    p.writeInt32(p_signalInfoRecord.isPresent);
    p.writeInt32(p_signalInfoRecord.signalType);
    p.writeInt32(p_signalInfoRecord.alertPitch);
    p.writeInt32(p_signalInfoRecord.signal);
}

int responseCdmaInformationRecords(Parcel &p,
            void *response, size_t responselen) {
    int num;
    char* string8 = NULL;
    RIL_CDMA_InformationRecord *infoRec;

    if (response == NULL && responselen != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof (RIL_CDMA_InformationRecords)) {
        RFX_LOG_E(RFX_LOG_TAG, "responseCdmaInformationRecords: invalid response length %d expected multiple of %d\n",
            (int)responselen, (int)sizeof (RIL_CDMA_InformationRecords *));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_CDMA_InformationRecords *p_cur =
                             (RIL_CDMA_InformationRecords *) response;
    num = MIN(p_cur->numberOfInfoRecs, RIL_CDMA_MAX_NUMBER_OF_INFO_RECS);

    startResponse;
    p.writeInt32(num);

    for (int i = 0 ; i < num ; i++) {
        infoRec = &p_cur->infoRec[i];
        p.writeInt32(infoRec->name);
        switch (infoRec->name) {
            case RIL_CDMA_DISPLAY_INFO_REC:
            case RIL_CDMA_EXTENDED_DISPLAY_INFO_REC:
                if (infoRec->rec.display.alpha_len >
                                         CDMA_ALPHA_INFO_BUFFER_LENGTH) {
                    RFX_LOG_E(RFX_LOG_TAG, "invalid display info response length %d \
                          expected not more than %d\n",
                         (int)infoRec->rec.display.alpha_len,
                         CDMA_ALPHA_INFO_BUFFER_LENGTH);
                    return RIL_ERRNO_INVALID_RESPONSE;
                }
                string8 = (char*) calloc(infoRec->rec.display.alpha_len + 1, sizeof(char));
                if (string8 == NULL) {
                    RFX_LOG_E(RFX_LOG_TAG, "Memory allocation failed for responseCdmaInformationRecords");
                    closeRequest;
                    return RIL_ERRNO_NO_MEMORY;
                }
                for (int i = 0 ; i < infoRec->rec.display.alpha_len ; i++) {
                    string8[i] = infoRec->rec.display.alpha_buf[i];
                }
                string8[(int)infoRec->rec.display.alpha_len] = '\0';
                writeStringToParcel(p, (const char*)string8);
                free(string8);
                string8 = NULL;
                break;
            case RIL_CDMA_CALLED_PARTY_NUMBER_INFO_REC:
            case RIL_CDMA_CALLING_PARTY_NUMBER_INFO_REC:
            case RIL_CDMA_CONNECTED_NUMBER_INFO_REC:
                if (infoRec->rec.number.len > CDMA_NUMBER_INFO_BUFFER_LENGTH) {
                    RFX_LOG_E(RFX_LOG_TAG, "invalid display info response length %d \
                          expected not more than %d\n",
                         (int)infoRec->rec.number.len,
                         CDMA_NUMBER_INFO_BUFFER_LENGTH);
                    return RIL_ERRNO_INVALID_RESPONSE;
                }
                string8 = (char*) calloc(infoRec->rec.number.len + 1, sizeof(char));
                if (string8 == NULL) {
                    RFX_LOG_E(RFX_LOG_TAG, "Memory allocation failed for responseCdmaInformationRecords");
                    closeRequest;
                    return RIL_ERRNO_NO_MEMORY;
                }
                for (int i = 0 ; i < infoRec->rec.number.len; i++) {
                    string8[i] = infoRec->rec.number.buf[i];
                }
                string8[(int)infoRec->rec.number.len] = '\0';
                writeStringToParcel(p, (const char*)string8);
                free(string8);
                string8 = NULL;
                p.writeInt32(infoRec->rec.number.number_type);
                p.writeInt32(infoRec->rec.number.number_plan);
                p.writeInt32(infoRec->rec.number.pi);
                p.writeInt32(infoRec->rec.number.si);
                break;
            case RIL_CDMA_SIGNAL_INFO_REC:
                p.writeInt32(infoRec->rec.signal.isPresent);
                p.writeInt32(infoRec->rec.signal.signalType);
                p.writeInt32(infoRec->rec.signal.alertPitch);
                p.writeInt32(infoRec->rec.signal.signal);

                appendPrintBuf("%sisPresent=%X, signalType=%X, \
                                alertPitch=%X, signal=%X, ",
                   printBuf, (int)infoRec->rec.signal.isPresent,
                   (int)infoRec->rec.signal.signalType,
                   (int)infoRec->rec.signal.alertPitch,
                   (int)infoRec->rec.signal.signal);
                removeLastChar;
                break;
            case RIL_CDMA_REDIRECTING_NUMBER_INFO_REC:
                if (infoRec->rec.redir.redirectingNumber.len >
                                              CDMA_NUMBER_INFO_BUFFER_LENGTH) {
                    RFX_LOG_E(RFX_LOG_TAG, "invalid display info response length %d \
                          expected not more than %d\n",
                         (int)infoRec->rec.redir.redirectingNumber.len,
                         CDMA_NUMBER_INFO_BUFFER_LENGTH);
                    return RIL_ERRNO_INVALID_RESPONSE;
                }
                string8 = (char*) calloc(infoRec->rec.redir.redirectingNumber.len + 1,
                        sizeof(char));
                if (string8 == NULL) {
                    RFX_LOG_E(RFX_LOG_TAG, "Memory allocation failed for responseCdmaInformationRecords");
                    closeRequest;
                    return RIL_ERRNO_NO_MEMORY;
                }
                for (int i = 0;
                         i < infoRec->rec.redir.redirectingNumber.len;
                         i++) {
                    string8[i] = infoRec->rec.redir.redirectingNumber.buf[i];
                }
                string8[(int)infoRec->rec.redir.redirectingNumber.len] = '\0';
                writeStringToParcel(p, (const char*)string8);
                free(string8);
                string8 = NULL;
                p.writeInt32(infoRec->rec.redir.redirectingNumber.number_type);
                p.writeInt32(infoRec->rec.redir.redirectingNumber.number_plan);
                p.writeInt32(infoRec->rec.redir.redirectingNumber.pi);
                p.writeInt32(infoRec->rec.redir.redirectingNumber.si);
                p.writeInt32(infoRec->rec.redir.redirectingReason);
                break;
            case RIL_CDMA_LINE_CONTROL_INFO_REC:
                p.writeInt32(infoRec->rec.lineCtrl.lineCtrlPolarityIncluded);
                p.writeInt32(infoRec->rec.lineCtrl.lineCtrlToggle);
                p.writeInt32(infoRec->rec.lineCtrl.lineCtrlReverse);
                p.writeInt32(infoRec->rec.lineCtrl.lineCtrlPowerDenial);

                appendPrintBuf("%slineCtrlPolarityIncluded=%d, \
                                lineCtrlToggle=%d, lineCtrlReverse=%d, \
                                lineCtrlPowerDenial=%d, ", printBuf,
                       (int)infoRec->rec.lineCtrl.lineCtrlPolarityIncluded,
                       (int)infoRec->rec.lineCtrl.lineCtrlToggle,
                       (int)infoRec->rec.lineCtrl.lineCtrlReverse,
                       (int)infoRec->rec.lineCtrl.lineCtrlPowerDenial);
                removeLastChar;
                break;
            case RIL_CDMA_T53_CLIR_INFO_REC:
                p.writeInt32((int)(infoRec->rec.clir.cause));

                appendPrintBuf("%scause%d", printBuf, infoRec->rec.clir.cause);
                removeLastChar;
                break;
            case RIL_CDMA_T53_AUDIO_CONTROL_INFO_REC:
                p.writeInt32(infoRec->rec.audioCtrl.upLink);
                p.writeInt32(infoRec->rec.audioCtrl.downLink);

                appendPrintBuf("%supLink=%d, downLink=%d, ", printBuf,
                        infoRec->rec.audioCtrl.upLink,
                        infoRec->rec.audioCtrl.downLink);
                removeLastChar;
                break;
            case RIL_CDMA_T53_RELEASE_INFO_REC:
                // TODO(Moto): See David Krause, he has the answer:)
                RFX_LOG_E(RFX_LOG_TAG, "RIL_CDMA_T53_RELEASE_INFO_REC: return INVALID_RESPONSE");
                return RIL_ERRNO_INVALID_RESPONSE;
            default:
                RFX_LOG_E(RFX_LOG_TAG, "Incorrect name value");
                return RIL_ERRNO_INVALID_RESPONSE;
        }
    }
    closeResponse;

    return 0;
}

void responseRilSignalStrengthV5(Parcel &p, RIL_SignalStrength_v10 *p_cur) {
    p.writeInt32(p_cur->GW_SignalStrength.signalStrength);
    p.writeInt32(p_cur->GW_SignalStrength.bitErrorRate);
    p.writeInt32(p_cur->CDMA_SignalStrength.dbm);
    p.writeInt32(p_cur->CDMA_SignalStrength.ecio);
    p.writeInt32(p_cur->EVDO_SignalStrength.dbm);
    p.writeInt32(p_cur->EVDO_SignalStrength.ecio);
    p.writeInt32(p_cur->EVDO_SignalStrength.signalNoiseRatio);
}

void responseRilSignalStrengthV6Extra(Parcel &p, RIL_SignalStrength_v10 *p_cur) {
    /*
     * Fixup LTE for backwards compatibility
     */
    // signalStrength: -1 -> 99
    if (p_cur->LTE_SignalStrength.signalStrength == -1) {
        p_cur->LTE_SignalStrength.signalStrength = 99;
    }
    // rsrp: -1 -> INT_MAX all other negative value to positive.
    // So remap here
    if (p_cur->LTE_SignalStrength.rsrp == -1) {
        p_cur->LTE_SignalStrength.rsrp = INT_MAX;
    } else if (p_cur->LTE_SignalStrength.rsrp < -1) {
        p_cur->LTE_SignalStrength.rsrp = -p_cur->LTE_SignalStrength.rsrp;
    }
    // rsrq: -1 -> INT_MAX
    if (p_cur->LTE_SignalStrength.rsrq == -1) {
        p_cur->LTE_SignalStrength.rsrq = INT_MAX;
    }
    // Not remapping rssnr is already using INT_MAX

    // cqi: -1 -> INT_MAX
    if (p_cur->LTE_SignalStrength.cqi == -1) {
        p_cur->LTE_SignalStrength.cqi = INT_MAX;
    }

    p.writeInt32(p_cur->LTE_SignalStrength.signalStrength);
    p.writeInt32(p_cur->LTE_SignalStrength.rsrp);
    p.writeInt32(p_cur->LTE_SignalStrength.rsrq);
    p.writeInt32(p_cur->LTE_SignalStrength.rssnr);
    p.writeInt32(p_cur->LTE_SignalStrength.cqi);
}

void responseRilSignalStrengthV10(Parcel &p, RIL_SignalStrength_v10 *p_cur) {
    responseRilSignalStrengthV5(p, p_cur);
    responseRilSignalStrengthV6Extra(p, p_cur);
    p.writeInt32(p_cur->TD_SCDMA_SignalStrength.rscp);
}

int responseRilSignalStrength(Parcel &p,
                    void *response, size_t responselen) {
    if (response == NULL || responselen != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_SignalStrength_v10 *p_cur;
    if (s_callbacks.version <= LAST_IMPRECISE_RIL_VERSION) {
        if (responselen >= sizeof (RIL_SignalStrength_v5)) {
            p_cur = ((RIL_SignalStrength_v10 *) response);

            responseRilSignalStrengthV5(p, p_cur);

            if (responselen >= sizeof (RIL_SignalStrength_v6)) {
                responseRilSignalStrengthV6Extra(p, p_cur);
                if (responselen >= sizeof (RIL_SignalStrength_v10)) {
                    p.writeInt32(p_cur->TD_SCDMA_SignalStrength.rscp);
                } else {
                    p.writeInt32(INT_MAX);
                }
            } else {
                p.writeInt32(99);
                p.writeInt32(INT_MAX);
                p.writeInt32(INT_MAX);
                p.writeInt32(INT_MAX);
                p.writeInt32(INT_MAX);
                p.writeInt32(INT_MAX);
            }
        } else {
            RFX_LOG_E(RFX_LOG_TAG, "invalid response length");
            return RIL_ERRNO_INVALID_RESPONSE;
        }
    } else { // RIL version >= 13
        if (responselen % sizeof(RIL_SignalStrength_v10) != 0) {
            RFX_LOG_E(RFX_LOG_TAG, "Data structure expected is RIL_SignalStrength_v10");
#if 0
                if (!isDebuggable()) {
                    return RIL_ERRNO_INVALID_RESPONSE;
                } else {
                    assert(0);
                }
#endif
        }
        p_cur = ((RIL_SignalStrength_v10 *) response);
        responseRilSignalStrengthV10(p, p_cur);
    }
    startResponse;
    appendPrintBuf("%s[signalStrength=%d,bitErrorRate=%d,\
            CDMA_SS.dbm=%d,CDMA_SSecio=%d,\
            EVDO_SS.dbm=%d,EVDO_SS.ecio=%d,\
            EVDO_SS.signalNoiseRatio=%d,\
            LTE_SS.signalStrength=%d,LTE_SS.rsrp=%d,LTE_SS.rsrq=%d,\
            LTE_SS.rssnr=%d,LTE_SS.cqi=%d,TDSCDMA_SS.rscp=%d]",
            printBuf,
            p_cur->GW_SignalStrength.signalStrength,
            p_cur->GW_SignalStrength.bitErrorRate,
            p_cur->CDMA_SignalStrength.dbm,
            p_cur->CDMA_SignalStrength.ecio,
            p_cur->EVDO_SignalStrength.dbm,
            p_cur->EVDO_SignalStrength.ecio,
            p_cur->EVDO_SignalStrength.signalNoiseRatio,
            p_cur->LTE_SignalStrength.signalStrength,
            p_cur->LTE_SignalStrength.rsrp,
            p_cur->LTE_SignalStrength.rsrq,
            p_cur->LTE_SignalStrength.rssnr,
            p_cur->LTE_SignalStrength.cqi,
            p_cur->TD_SCDMA_SignalStrength.rscp);
    closeResponse;
    return 0;
}

int responseRilSignalStrength_1_2(Parcel &p, void *response,
        size_t responselen) {
    if (response == NULL && responselen != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }
    if (responselen % sizeof(int) != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "responseInts: invalid response length %d expected multiple of %d\n",
            (int)responselen, (int)sizeof(int));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    int *p_int = (int *) response;
    int numInts = responselen / sizeof(int);

    /* each int*/
    startResponse;
    for (int i = 0 ; i < numInts ; i++) {
        appendPrintBuf("%s%d,", printBuf, p_int[i]);
        p.writeInt32(p_int[i]);
    }
    removeLastChar;
    closeResponse;

    return 0;
}

int responseCallRing(Parcel &p, void *response, size_t responselen) {
    if ((response == NULL) || (responselen == 0)) {
        return responseVoid(p, response, responselen);
    } else {
        return responseCdmaSignalInfoRecord(p, response, responselen);
    }
}

int responseCdmaSignalInfoRecord(Parcel &p, void *response, size_t responselen) {
    if (response == NULL || responselen == 0) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof (RIL_CDMA_SignalInfoRecord)) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response length %d expected sizeof (RIL_CDMA_SignalInfoRecord) of %d\n",
            (int)responselen, (int)sizeof (RIL_CDMA_SignalInfoRecord));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    startResponse;

    RIL_CDMA_SignalInfoRecord *p_cur = ((RIL_CDMA_SignalInfoRecord *) response);
    marshallSignalInfoRecord(p, *p_cur);

    appendPrintBuf("%s[isPresent=%d,signalType=%d,alertPitch=%d\
              signal=%d]",
              printBuf,
              p_cur->isPresent,
              p_cur->signalType,
              p_cur->alertPitch,
              p_cur->signal);

    closeResponse;
    return 0;
}

int responseCdmaCallWaiting(Parcel &p, void *response,
            size_t responselen) {
    if (response == NULL || responselen == 0) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen < sizeof(RIL_CDMA_CallWaiting_v6)) {
        RFX_LOG_W(RFX_LOG_TAG, "Upgrade to ril version %d\n", RIL_VERSION);
    }

    RIL_CDMA_CallWaiting_v6 *p_cur = ((RIL_CDMA_CallWaiting_v6 *) response);

    writeStringToParcel(p, p_cur->number);
    p.writeInt32(p_cur->numberPresentation);
    writeStringToParcel(p, p_cur->name);
    marshallSignalInfoRecord(p, p_cur->signalInfoRecord);

    if (s_callbacks.version <= LAST_IMPRECISE_RIL_VERSION) {
        if (responselen >= sizeof(RIL_CDMA_CallWaiting_v6)) {
            p.writeInt32(p_cur->number_type);
            p.writeInt32(p_cur->number_plan);
        } else {
            p.writeInt32(0);
            p.writeInt32(0);
        }
    } else { // RIL version >= 13
        if (responselen % sizeof(RIL_CDMA_CallWaiting_v6) != 0) {
            RFX_LOG_E(RFX_LOG_TAG, "Data structure expected is RIL_CDMA_CallWaiting_v6");
#if 0
                if (!isDebuggable()) {
                    return RIL_ERRNO_INVALID_RESPONSE;
                } else {
                    assert(0);
                }
#endif
        }
        p.writeInt32(p_cur->number_type);
        p.writeInt32(p_cur->number_plan);
    }

    startResponse;
    appendPrintBuf("%snumber=%s,numberPresentation=%d, name=%s,\
            signalInfoRecord[isPresent=%d,signalType=%d,alertPitch=%d\
            signal=%d,number_type=%d,number_plan=%d]",
            printBuf,
            p_cur->number,
            p_cur->numberPresentation,
            p_cur->name,
            p_cur->signalInfoRecord.isPresent,
            p_cur->signalInfoRecord.signalType,
            p_cur->signalInfoRecord.alertPitch,
            p_cur->signalInfoRecord.signal,
            p_cur->number_type,
            p_cur->number_plan);
    closeResponse;

    return 0;
}

void responseSimRefreshV7(Parcel &p, void *response) {
      RIL_SimRefreshResponse_v7 *p_cur = ((RIL_SimRefreshResponse_v7 *) response);
      p.writeInt32(p_cur->result);
      p.writeInt32(p_cur->ef_id);
      writeStringToParcel(p, p_cur->aid);

      appendPrintBuf("%sresult=%d, ef_id=%d, aid=%s",
            printBuf,
            p_cur->result,
            p_cur->ef_id,
            p_cur->aid);

}

int responseSimRefresh(Parcel &p, void *response, size_t responselen) {
    if (response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "responseSimRefresh: invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    startResponse;
    if (s_callbacks.version <= LAST_IMPRECISE_RIL_VERSION) {
        if (s_callbacks.version >= 7) {
            responseSimRefreshV7(p, response);
        } else {
            int *p_cur = ((int *) response);
            p.writeInt32(p_cur[0]);
            p.writeInt32(p_cur[1]);
            writeStringToParcel(p, NULL);

            appendPrintBuf("%sresult=%d, ef_id=%d",
                    printBuf,
                    p_cur[0],
                    p_cur[1]);
        }
    } else { // RIL version >= 13
        if (responselen % sizeof(RIL_SimRefreshResponse_v7) != 0) {
            RFX_LOG_E(RFX_LOG_TAG, "Data structure expected is RIL_SimRefreshResponse_v7");
#if 0
                if (!isDebuggable()) {
                    return RIL_ERRNO_INVALID_RESPONSE;
                } else {
                    assert(0);
                }
#endif
        }
        responseSimRefreshV7(p, response);

    }
    closeResponse;

    return 0;
}

int responseCellInfoListV6(Parcel &p, void *response, size_t responselen) {
    if (response == NULL && responselen != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof(RIL_CellInfo) != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "responseCellInfoList: invalid response length %d expected multiple of %d",
                (int)responselen, (int)sizeof(RIL_CellInfo));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    int num = responselen / sizeof(RIL_CellInfo);
    p.writeInt32(num);

    RIL_CellInfo *p_cur = (RIL_CellInfo *) response;
    startResponse;
    int i;
    for (i = 0; i < num; i++) {
        p.writeInt32((int)p_cur->cellInfoType);
        p.writeInt32(p_cur->registered);
        p.writeInt32(p_cur->timeStampType);
        p.writeInt64(p_cur->timeStamp);
        switch(p_cur->cellInfoType) {
            case RIL_CELL_INFO_TYPE_GSM: {
                p.writeInt32(p_cur->CellInfo.gsm.cellIdentityGsm.mcc);
                p.writeInt32(p_cur->CellInfo.gsm.cellIdentityGsm.mnc);
                p.writeInt32(p_cur->CellInfo.gsm.cellIdentityGsm.lac);
                p.writeInt32(p_cur->CellInfo.gsm.cellIdentityGsm.cid);
                p.writeInt32(p_cur->CellInfo.gsm.signalStrengthGsm.signalStrength);
                p.writeInt32(p_cur->CellInfo.gsm.signalStrengthGsm.bitErrorRate);
                break;
            }
            case RIL_CELL_INFO_TYPE_WCDMA: {
                p.writeInt32(p_cur->CellInfo.wcdma.cellIdentityWcdma.mcc);
                p.writeInt32(p_cur->CellInfo.wcdma.cellIdentityWcdma.mnc);
                p.writeInt32(p_cur->CellInfo.wcdma.cellIdentityWcdma.lac);
                p.writeInt32(p_cur->CellInfo.wcdma.cellIdentityWcdma.cid);
                p.writeInt32(p_cur->CellInfo.wcdma.cellIdentityWcdma.psc);
                p.writeInt32(p_cur->CellInfo.wcdma.signalStrengthWcdma.signalStrength);
                p.writeInt32(p_cur->CellInfo.wcdma.signalStrengthWcdma.bitErrorRate);
                break;
            }
            case RIL_CELL_INFO_TYPE_CDMA: {
                p.writeInt32(p_cur->CellInfo.cdma.cellIdentityCdma.networkId);
                p.writeInt32(p_cur->CellInfo.cdma.cellIdentityCdma.systemId);
                p.writeInt32(p_cur->CellInfo.cdma.cellIdentityCdma.basestationId);
                p.writeInt32(p_cur->CellInfo.cdma.cellIdentityCdma.longitude);
                p.writeInt32(p_cur->CellInfo.cdma.cellIdentityCdma.latitude);

                p.writeInt32(p_cur->CellInfo.cdma.signalStrengthCdma.dbm);
                p.writeInt32(p_cur->CellInfo.cdma.signalStrengthCdma.ecio);
                p.writeInt32(p_cur->CellInfo.cdma.signalStrengthEvdo.dbm);
                p.writeInt32(p_cur->CellInfo.cdma.signalStrengthEvdo.ecio);
                p.writeInt32(p_cur->CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio);
                break;
            }
            case RIL_CELL_INFO_TYPE_LTE: {
                p.writeInt32(p_cur->CellInfo.lte.cellIdentityLte.mcc);
                p.writeInt32(p_cur->CellInfo.lte.cellIdentityLte.mnc);
                p.writeInt32(p_cur->CellInfo.lte.cellIdentityLte.ci);
                p.writeInt32(p_cur->CellInfo.lte.cellIdentityLte.pci);
                p.writeInt32(p_cur->CellInfo.lte.cellIdentityLte.tac);

                p.writeInt32(p_cur->CellInfo.lte.signalStrengthLte.signalStrength);
                p.writeInt32(p_cur->CellInfo.lte.signalStrengthLte.rsrp);
                p.writeInt32(p_cur->CellInfo.lte.signalStrengthLte.rsrq);
                p.writeInt32(p_cur->CellInfo.lte.signalStrengthLte.rssnr);
                p.writeInt32(p_cur->CellInfo.lte.signalStrengthLte.cqi);
                p.writeInt32(p_cur->CellInfo.lte.signalStrengthLte.timingAdvance);
                break;
            }
            case RIL_CELL_INFO_TYPE_TD_SCDMA: {
                p.writeInt32(p_cur->CellInfo.tdscdma.cellIdentityTdscdma.mcc);
                p.writeInt32(p_cur->CellInfo.tdscdma.cellIdentityTdscdma.mnc);
                p.writeInt32(p_cur->CellInfo.tdscdma.cellIdentityTdscdma.lac);
                p.writeInt32(p_cur->CellInfo.tdscdma.cellIdentityTdscdma.cid);
                p.writeInt32(p_cur->CellInfo.tdscdma.cellIdentityTdscdma.cpid);
                p.writeInt32(p_cur->CellInfo.tdscdma.signalStrengthTdscdma.rscp);
                break;
            }
            case RIL_CELL_INFO_TYPE_NR:
            case RIL_CELL_INFO_TYPE_NONE:
                break;
        }
        p_cur += 1;
    }
    removeLastChar;
    closeResponse;

    return 0;
}

int responseCellInfoListV12(Parcel &p, void *response, size_t responselen) {
    if (response == NULL && responselen != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen % sizeof(RIL_CellInfo_v12) != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "responseCellInfoList: invalid response length %d expected multiple of %d",
                (int)responselen, (int)sizeof(RIL_CellInfo_v12));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    int num = responselen / sizeof(RIL_CellInfo_v12);
    p.writeInt32(num);

    RIL_CellInfo_v12 *p_cur = (RIL_CellInfo_v12 *) response;
    startResponse;
    int i;
    for (i = 0; i < num; i++) {
        p.writeInt32((int)p_cur->cellInfoType);
        p.writeInt32(p_cur->registered);
        p.writeInt32(p_cur->timeStampType);
        p.writeInt64(p_cur->timeStamp);
        switch(p_cur->cellInfoType) {
            case RIL_CELL_INFO_TYPE_GSM: {
                p.writeInt32(p_cur->CellInfo.gsm.cellIdentityGsm.mcc);
                p.writeInt32(p_cur->CellInfo.gsm.cellIdentityGsm.mnc);
                p.writeInt32(p_cur->CellInfo.gsm.cellIdentityGsm.lac);
                p.writeInt32(p_cur->CellInfo.gsm.cellIdentityGsm.cid);
                p.writeInt32(p_cur->CellInfo.gsm.cellIdentityGsm.arfcn);
                p.writeInt32(p_cur->CellInfo.gsm.cellIdentityGsm.bsic);
                p.writeInt32(p_cur->CellInfo.gsm.signalStrengthGsm.signalStrength);
                p.writeInt32(p_cur->CellInfo.gsm.signalStrengthGsm.bitErrorRate);
                p.writeInt32(p_cur->CellInfo.gsm.signalStrengthGsm.timingAdvance);
                break;
            }
            case RIL_CELL_INFO_TYPE_WCDMA: {
                p.writeInt32(p_cur->CellInfo.wcdma.cellIdentityWcdma.mcc);
                p.writeInt32(p_cur->CellInfo.wcdma.cellIdentityWcdma.mnc);
                p.writeInt32(p_cur->CellInfo.wcdma.cellIdentityWcdma.lac);
                p.writeInt32(p_cur->CellInfo.wcdma.cellIdentityWcdma.cid);
                p.writeInt32(p_cur->CellInfo.wcdma.cellIdentityWcdma.psc);
                p.writeInt32(p_cur->CellInfo.wcdma.cellIdentityWcdma.uarfcn);
                p.writeInt32(p_cur->CellInfo.wcdma.signalStrengthWcdma.signalStrength);
                p.writeInt32(p_cur->CellInfo.wcdma.signalStrengthWcdma.bitErrorRate);
                break;
            }
            case RIL_CELL_INFO_TYPE_CDMA: {
                p.writeInt32(p_cur->CellInfo.cdma.cellIdentityCdma.networkId);
                p.writeInt32(p_cur->CellInfo.cdma.cellIdentityCdma.systemId);
                p.writeInt32(p_cur->CellInfo.cdma.cellIdentityCdma.basestationId);
                p.writeInt32(p_cur->CellInfo.cdma.cellIdentityCdma.longitude);
                p.writeInt32(p_cur->CellInfo.cdma.cellIdentityCdma.latitude);

                p.writeInt32(p_cur->CellInfo.cdma.signalStrengthCdma.dbm);
                p.writeInt32(p_cur->CellInfo.cdma.signalStrengthCdma.ecio);
                p.writeInt32(p_cur->CellInfo.cdma.signalStrengthEvdo.dbm);
                p.writeInt32(p_cur->CellInfo.cdma.signalStrengthEvdo.ecio);
                p.writeInt32(p_cur->CellInfo.cdma.signalStrengthEvdo.signalNoiseRatio);
                break;
            }
            case RIL_CELL_INFO_TYPE_LTE: {
                p.writeInt32(p_cur->CellInfo.lte.cellIdentityLte.mcc);
                p.writeInt32(p_cur->CellInfo.lte.cellIdentityLte.mnc);
                p.writeInt32(p_cur->CellInfo.lte.cellIdentityLte.ci);
                p.writeInt32(p_cur->CellInfo.lte.cellIdentityLte.pci);
                p.writeInt32(p_cur->CellInfo.lte.cellIdentityLte.tac);
                p.writeInt32(p_cur->CellInfo.lte.cellIdentityLte.earfcn);

                p.writeInt32(p_cur->CellInfo.lte.signalStrengthLte.signalStrength);
                p.writeInt32(p_cur->CellInfo.lte.signalStrengthLte.rsrp);
                p.writeInt32(p_cur->CellInfo.lte.signalStrengthLte.rsrq);
                p.writeInt32(p_cur->CellInfo.lte.signalStrengthLte.rssnr);
                p.writeInt32(p_cur->CellInfo.lte.signalStrengthLte.cqi);
                p.writeInt32(p_cur->CellInfo.lte.signalStrengthLte.timingAdvance);
                break;
            }
            case RIL_CELL_INFO_TYPE_TD_SCDMA: {
                p.writeInt32(p_cur->CellInfo.tdscdma.cellIdentityTdscdma.mcc);
                p.writeInt32(p_cur->CellInfo.tdscdma.cellIdentityTdscdma.mnc);
                p.writeInt32(p_cur->CellInfo.tdscdma.cellIdentityTdscdma.lac);
                p.writeInt32(p_cur->CellInfo.tdscdma.cellIdentityTdscdma.cid);
                p.writeInt32(p_cur->CellInfo.tdscdma.cellIdentityTdscdma.cpid);
                p.writeInt32(p_cur->CellInfo.tdscdma.signalStrengthTdscdma.rscp);
                break;
            }
            case RIL_CELL_INFO_TYPE_NR:
            case RIL_CELL_INFO_TYPE_NONE:
                break;
        }
        p_cur += 1;
    }
    removeLastChar;
    closeResponse;
    return 0;
}

int responseCellInfoList(Parcel &p, void *response, size_t responselen)
{
    if (s_callbacks.version <= LAST_IMPRECISE_RIL_VERSION) {
        if (s_callbacks.version < 12) {
            RFX_LOG_D(RFX_LOG_TAG, "responseCellInfoList: v6");
            return responseCellInfoListV6(p, response, responselen);
        } else {
            RFX_LOG_D(RFX_LOG_TAG, "responseCellInfoList: v12");
            return responseCellInfoListV12(p, response, responselen);
        }
    } else { // RIL version >= 13
        if (responselen % sizeof(RIL_CellInfo_v12) != 0) {
            RFX_LOG_E(RFX_LOG_TAG, "Data structure expected is RIL_CellInfo_v12");
        #if 0
            if (!isDebuggable()) {
                return RIL_ERRNO_INVALID_RESPONSE;
            } else {
                assert(0);
            }
        #endif
        }
        return responseCellInfoListV12(p, response, responselen);
    }

    return 0;
}

int responseHardwareConfig(Parcel &p, void *response, size_t responselen)
{
   if (response == NULL && responselen != 0) {
       RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
       return RIL_ERRNO_INVALID_RESPONSE;
   }

   if (responselen % sizeof(RIL_HardwareConfig) != 0) {
       RFX_LOG_E(RFX_LOG_TAG, "responseHardwareConfig: invalid response length %d expected multiple of %d",
          (int)responselen, (int)sizeof(RIL_HardwareConfig));
       return RIL_ERRNO_INVALID_RESPONSE;
   }
   RFX_LOG_E(RFX_LOG_TAG, "responseHardwareConfig weilun");

   int num = responselen / sizeof(RIL_HardwareConfig);
   int i;
   RIL_HardwareConfig *p_cur = (RIL_HardwareConfig *) response;

   p.writeInt32(num);

   startResponse;
   for (i = 0; i < num; i++) {
      switch (p_cur[i].type) {
         case RIL_HARDWARE_CONFIG_MODEM: {
            p.writeInt32(p_cur[i].type);
            writeStringToParcel(p, p_cur[i].uuid);
            p.writeInt32((int)p_cur[i].state);
            p.writeInt32(p_cur[i].cfg.modem.rilModel);
            p.writeInt32(p_cur[i].cfg.modem.rat);
            p.writeInt32(p_cur[i].cfg.modem.maxVoice);
            p.writeInt32(p_cur[i].cfg.modem.maxData);
            p.writeInt32(p_cur[i].cfg.modem.maxStandby);

            appendPrintBuf("%s modem: uuid=%s,state=%d,rat=%08x,maxV=%d,maxD=%d,maxS=%d", printBuf,
               p_cur[i].uuid, (int)p_cur[i].state, p_cur[i].cfg.modem.rat,
               p_cur[i].cfg.modem.maxVoice, p_cur[i].cfg.modem.maxData, p_cur[i].cfg.modem.maxStandby);
            break;
         }
         case RIL_HARDWARE_CONFIG_SIM: {
            p.writeInt32(p_cur[i].type);
            writeStringToParcel(p, p_cur[i].uuid);
            p.writeInt32((int)p_cur[i].state);
            writeStringToParcel(p, p_cur[i].cfg.sim.modemUuid);

            appendPrintBuf("%s sim: uuid=%s,state=%d,modem-uuid=%s", printBuf,
               p_cur[i].uuid, (int)p_cur[i].state, p_cur[i].cfg.sim.modemUuid);
            break;
         }
      }
   }
   removeLastChar;
   closeResponse;
   return 0;
}

int responseRadioCapability(Parcel &p, void *response, size_t responselen) {
    if (response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof (RIL_RadioCapability) ) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response length was %d expected %d",
                (int)responselen, (int)sizeof (RIL_SIM_IO_Response));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_RadioCapability *p_cur = (RIL_RadioCapability *) response;
    p.writeInt32(p_cur->version);
    p.writeInt32(p_cur->session);
    p.writeInt32(p_cur->phase);
    p.writeInt32(p_cur->rat);
    writeStringToParcel(p, p_cur->logicalModemUuid);
    p.writeInt32(p_cur->status);

    startResponse;
    appendPrintBuf("%s[version=%d,session=%d,phase=%d,\
            rat=%s,logicalModemUuid=%s,status=%d]",
            printBuf,
            p_cur->version,
            p_cur->session,
            p_cur->phase,
            p_cur->rat,
            p_cur->logicalModemUuid,
            p_cur->status);
    closeResponse;
    return 0;
}

int responseSSData(Parcel &p __unused, void *response __unused, size_t responselen __unused) {
#if 0
    RFX_LOG_D(RFX_LOG_TAG, "In responseSSData");
    int num;

    if (response == NULL && responselen != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response length was %d expected %d",
                (int)responselen, (int)sizeof (RIL_SIM_IO_Response));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof(RIL_StkCcUnsolSsResponse)) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response length %d, expected %d",
               (int)responselen, (int)sizeof(RIL_StkCcUnsolSsResponse));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    startResponse;
    RIL_StkCcUnsolSsResponse *p_cur = (RIL_StkCcUnsolSsResponse *) response;
    p.writeInt32(p_cur->serviceType);
    p.writeInt32(p_cur->requestType);
    p.writeInt32(p_cur->teleserviceType);
    p.writeInt32(p_cur->serviceClass);
    p.writeInt32(p_cur->result);

    if (isServiceTypeCfQuery(p_cur->serviceType, p_cur->requestType)) {
        RFX_LOG_D(RFX_LOG_TAG, "responseSSData CF type, num of Cf elements %d", p_cur->cfData.numValidIndexes);
        if (p_cur->cfData.numValidIndexes > NUM_SERVICE_CLASSES) {
            RFX_LOG_E(RFX_LOG_TAG, "numValidIndexes is greater than max value %d, "
                  "truncating it to max value", NUM_SERVICE_CLASSES);
            p_cur->cfData.numValidIndexes = NUM_SERVICE_CLASSES;
        }
        /* number of call info's */
        p.writeInt32(p_cur->cfData.numValidIndexes);

        for (int i = 0; i < p_cur->cfData.numValidIndexes; i++) {
             RIL_CallForwardInfo cf = p_cur->cfData.cfInfo[i];

             p.writeInt32(cf.status);
             p.writeInt32(cf.reason);
             p.writeInt32(cf.serviceClass);
             p.writeInt32(cf.toa);
             writeStringToParcel(p, cf.number);
             p.writeInt32(cf.timeSeconds);
             appendPrintBuf("%s[%s,reason=%d,cls=%d,toa=%d,%s,tout=%d],", printBuf,
                 (cf.status==1)?"enable":"disable", cf.reason, cf.serviceClass, cf.toa,
                  (char*)cf.number, cf.timeSeconds);
             RFX_LOG_D(RFX_LOG_TAG, "Data: %d,reason=%d,cls=%d,toa=%d,num=%s,tout=%d],", cf.status,
                  cf.reason, cf.serviceClass, cf.toa, (char*)cf.number, cf.timeSeconds);
        }
    } else {
        p.writeInt32 (SS_INFO_MAX);

        /* each int*/
        for (int i = 0; i < SS_INFO_MAX; i++) {
             appendPrintBuf("%s%d,", printBuf, p_cur->ssInfo[i]);
             RFX_LOG_D(RFX_LOG_TAG, "Data: %d",p_cur->ssInfo[i]);
             p.writeInt32(p_cur->ssInfo[i]);
        }
    }
    removeLastChar;
    closeResponse;
#endif
    return 0;
}

bool isServiceTypeCfQuery(RIL_SsServiceType serType, RIL_SsRequestType reqType) {
    if ((reqType == SS_INTERROGATION) &&
        (serType == SS_CFU ||
         serType == SS_CF_BUSY ||
         serType == SS_CF_NO_REPLY ||
         serType == SS_CF_NOT_REACHABLE ||
         serType == SS_CF_ALL ||
         serType == SS_CF_ALL_CONDITIONAL)) {
        return true;
    }
    return false;
}

void sendSimStatusAppInfo(Parcel &p, int num_apps, RIL_AppStatus appStatus[]) {
        p.writeInt32(num_apps);
        startResponse;
        for (int i = 0; i < num_apps; i++) {
            p.writeInt32(appStatus[i].app_type);
            p.writeInt32(appStatus[i].app_state);
            p.writeInt32(appStatus[i].perso_substate);
            writeStringToParcel(p, (const char*)(appStatus[i].aid_ptr));
            writeStringToParcel(p, (const char*)
                                          (appStatus[i].app_label_ptr));
            p.writeInt32(appStatus[i].pin1_replaced);
            p.writeInt32(appStatus[i].pin1);
            p.writeInt32(appStatus[i].pin2);
            appendPrintBuf("%s[app_type=%d,app_state=%d,perso_substate=%d,\
                    aid_ptr=%s,app_label_ptr=%s,pin1_replaced=%d,pin1=%d,pin2=%d],",
                    printBuf,
                    appStatus[i].app_type,
                    appStatus[i].app_state,
                    appStatus[i].perso_substate,
                    appStatus[i].aid_ptr,
                    appStatus[i].app_label_ptr,
                    appStatus[i].pin1_replaced,
                    appStatus[i].pin1,
                    appStatus[i].pin2);
        }
        closeResponse;
}

void responseSimStatusV5(Parcel &p, void *response) {
    RIL_CardStatus_v5 *p_cur = ((RIL_CardStatus_v5 *) response);

    p.writeInt32(p_cur->card_state);
    p.writeInt32(p_cur->universal_pin_state);
    p.writeInt32(p_cur->gsm_umts_subscription_app_index);
    p.writeInt32(p_cur->cdma_subscription_app_index);

    sendSimStatusAppInfo(p, p_cur->num_applications, p_cur->applications);
}

void responseSimStatusV6(Parcel &p, void *response) {
    RIL_CardStatus_v6 *p_cur = ((RIL_CardStatus_v6 *) response);

    p.writeInt32(p_cur->card_state);
    p.writeInt32(p_cur->universal_pin_state);
    p.writeInt32(p_cur->gsm_umts_subscription_app_index);
    p.writeInt32(p_cur->cdma_subscription_app_index);
    p.writeInt32(p_cur->ims_subscription_app_index);

    sendSimStatusAppInfo(p, p_cur->num_applications, p_cur->applications);
}

int responseSimStatus(Parcel &p, void *response, size_t responselen) {
    if (response == NULL && responselen != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (s_callbacks.version <= LAST_IMPRECISE_RIL_VERSION) {
        if (responselen == sizeof (RIL_CardStatus_v6)) {
            responseSimStatusV6(p, response);
        } else if (responselen == sizeof (RIL_CardStatus_v5)) {
            responseSimStatusV5(p, response);
        } else {
            RFX_LOG_E(RFX_LOG_TAG, "responseSimStatus: A RilCardStatus_v6 or _v5 expected\n");
            return RIL_ERRNO_INVALID_RESPONSE;
        }
    } else { // RIL version >= 13
        if (responselen % sizeof(RIL_CardStatus_v6) != 0) {
            RFX_LOG_E(RFX_LOG_TAG, "Data structure expected is RIL_CardStatus_v6");
        #if 0
            if (!isDebuggable()) {
                return RIL_ERRNO_INVALID_RESPONSE;
            } else {
                assert(0);
            }
        #endif
        }
        responseSimStatusV6(p, response);
    }

    return 0;
}

int responseGsmBrSmsCnf(Parcel &p, void *response, size_t responselen) {
    int num = responselen / sizeof(RIL_GSM_BroadcastSmsConfigInfo *);
    p.writeInt32(num);

    startResponse;
    RIL_GSM_BroadcastSmsConfigInfo **p_cur =
                (RIL_GSM_BroadcastSmsConfigInfo **) response;
    for (int i = 0; i < num; i++) {
        p.writeInt32(p_cur[i]->fromServiceId);
        p.writeInt32(p_cur[i]->toServiceId);
        p.writeInt32(p_cur[i]->fromCodeScheme);
        p.writeInt32(p_cur[i]->toCodeScheme);
        p.writeInt32(p_cur[i]->selected);

        appendPrintBuf("%s [%d: fromServiceId=%d, toServiceId=%d, \
                fromCodeScheme=%d, toCodeScheme=%d, selected =%d]",
                printBuf, i, p_cur[i]->fromServiceId, p_cur[i]->toServiceId,
                p_cur[i]->fromCodeScheme, p_cur[i]->toCodeScheme,
                p_cur[i]->selected);
    }
    closeResponse;

    return 0;
}

int responseCdmaBrSmsCnf(Parcel &p, void *response, size_t responselen) {
    RIL_CDMA_BroadcastSmsConfigInfo **p_cur =
               (RIL_CDMA_BroadcastSmsConfigInfo **) response;

    int num = responselen / sizeof (RIL_CDMA_BroadcastSmsConfigInfo *);
    p.writeInt32(num);

    startResponse;
    for (int i = 0 ; i < num ; i++ ) {
        p.writeInt32(p_cur[i]->service_category);
        p.writeInt32(p_cur[i]->language);
        p.writeInt32(p_cur[i]->selected);

        appendPrintBuf("%s [%d: srvice_category=%d, language =%d, \
              selected =%d], ",
              printBuf, i, p_cur[i]->service_category, p_cur[i]->language,
              p_cur[i]->selected);
    }
    closeResponse;

    return 0;
}

int responseCdmaSms(Parcel &p, void *response, size_t responselen) {
    int digitCount;
    int digitLimit;
    uint8_t uct;

    RFX_LOG_D(RFX_LOG_TAG, "Inside responseCdmaSms");

    if (response == NULL && responselen != 0) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof(RIL_CDMA_SMS_Message)) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response length was %d expected %d",
                (int)responselen, (int)sizeof(RIL_CDMA_SMS_Message));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_CDMA_SMS_Message *p_cur = (RIL_CDMA_SMS_Message *) response;
    p.writeInt32(p_cur->uTeleserviceID);
    p.write(&(p_cur->bIsServicePresent),sizeof(uct));
    p.writeInt32(p_cur->uServicecategory);
    p.writeInt32(p_cur->sAddress.digit_mode);
    p.writeInt32(p_cur->sAddress.number_mode);
    p.writeInt32(p_cur->sAddress.number_type);
    p.writeInt32(p_cur->sAddress.number_plan);
    p.write(&(p_cur->sAddress.number_of_digits), sizeof(uct));
    digitLimit= MIN((p_cur->sAddress.number_of_digits), RIL_CDMA_SMS_ADDRESS_MAX);
    for(digitCount =0 ; digitCount < digitLimit; digitCount ++) {
        p.write(&(p_cur->sAddress.digits[digitCount]),sizeof(uct));
    }

    p.writeInt32(p_cur->sSubAddress.subaddressType);
    p.write(&(p_cur->sSubAddress.odd),sizeof(uct));
    p.write(&(p_cur->sSubAddress.number_of_digits),sizeof(uct));
    digitLimit= MIN((p_cur->sSubAddress.number_of_digits), RIL_CDMA_SMS_SUBADDRESS_MAX);
    for(digitCount =0 ; digitCount < digitLimit; digitCount ++) {
        p.write(&(p_cur->sSubAddress.digits[digitCount]),sizeof(uct));
    }

    digitLimit= MIN((p_cur->uBearerDataLen), RIL_CDMA_SMS_BEARER_DATA_MAX);
    p.writeInt32(p_cur->uBearerDataLen);
    for(digitCount =0 ; digitCount < digitLimit; digitCount ++) {
       p.write(&(p_cur->aBearerData[digitCount]), sizeof(uct));
    }

    startResponse;
    appendPrintBuf("%suTeleserviceID=%d, bIsServicePresent=%d, uServicecategory=%d, \
            sAddress.digit_mode=%d, sAddress.number_mode=%d, sAddress.number_type=%d, ",
            printBuf, p_cur->uTeleserviceID,p_cur->bIsServicePresent,p_cur->uServicecategory,
            p_cur->sAddress.digit_mode, p_cur->sAddress.number_mode,p_cur->sAddress.number_type);
    closeResponse;

    return 0;
}

int responseGetSmsSimMemStatusCnf(Parcel &p,void *response, size_t responselen)
{
    if (response == NULL || responselen == 0) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof (RIL_SMS_Memory_Status)) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response length %d expected sizeof (RIL_SMS_Memory_Status) of %d\n",
             (int)responselen, (int)sizeof(RIL_SMS_Memory_Status));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    startResponse;

    RIL_SMS_Memory_Status *mem_status = (RIL_SMS_Memory_Status*)response;

    p.writeInt32(mem_status->used);
    p.writeInt32(mem_status->total);

    appendPrintBuf("%s [used = %d, total = %d]", printBuf, mem_status->used, mem_status->total);

    closeResponse;

    return 0;
}


int responseEtwsNotification(Parcel &p __unused, void *response __unused,
        size_t responselen __unused) {
#if 0
    if(NULL == response) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if(responselen != sizeof(RIL_CbEtwsNotification)) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response length %d expected %d",
            responselen, sizeof(RIL_CbEtwsNotification));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_CbEtwsNotification *p_cur = (RIL_CbEtwsNotification *)response;
    p.writeInt32(p_cur->warningType);
    p.writeInt32(p_cur->messageId);
    p.writeInt32(p_cur->serialNumber);
    writeStringToParcel(p, p_cur->plmnId);
    writeStringToParcel(p, p_cur->securityInfo);

    startResponse;
    appendPrintBuf("%s%d,%d,%d,%s,%s", printBuf, p_cur->waringType, p_cur->messageId,
                   p_cur->serialNumber, p_cur->plmnId, p_cur->securityInfo);
    closeResponse;
#endif
    return 0;
}

int responseDcRtInfo(Parcel &p, void *response, size_t responselen)
{
    int num = responselen / sizeof(RIL_DcRtInfo);
    if ((responselen % sizeof(RIL_DcRtInfo) != 0) || (num != 1)) {
        RFX_LOG_E(RFX_LOG_TAG, "responseDcRtInfo: invalid response length %d expected multiple of %d",
                (int)responselen, (int)sizeof(RIL_DcRtInfo));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    startResponse;
    RIL_DcRtInfo *pDcRtInfo = (RIL_DcRtInfo *)response;
    p.writeInt64(pDcRtInfo->time);
    p.writeInt32(pDcRtInfo->powerState);
    appendPrintBuf("%s[time=%d,powerState=%d]", printBuf,
        pDcRtInfo->time,
        pDcRtInfo->powerState);
    closeResponse;

    return 0;
}

int responseLceStatus(Parcel &p, void *response, size_t responselen) {
  if (response == NULL || responselen != sizeof(RIL_LceStatusInfo)) {
    if (response == NULL) {
      RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
    }
    else {
      RFX_LOG_E(RFX_LOG_TAG, "responseLceStatus: invalid response length %u expecting len: %u",
            (unsigned)sizeof(RIL_LceStatusInfo), (unsigned)responselen);
    }
    return RIL_ERRNO_INVALID_RESPONSE;
  }

  RIL_LceStatusInfo *p_cur = (RIL_LceStatusInfo *)response;
  p.write((void *)p_cur, 1);  // p_cur->lce_status takes one byte.
  p.writeInt32(p_cur->actual_interval_ms);

  startResponse;
  appendPrintBuf("LCE Status: %d, actual_interval_ms: %d",
                 p_cur->lce_status, p_cur->actual_interval_ms);
  closeResponse;

  return 0;
}

int responseLceData(Parcel &p, void *response, size_t responselen) {
  if (response == NULL || responselen != sizeof(RIL_LceDataInfo)) {
    if (response == NULL) {
      RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
    }
    else {
      RFX_LOG_E(RFX_LOG_TAG, "responseLceData: invalid response length %u expecting len: %u",
            (unsigned)sizeof(RIL_LceDataInfo), (unsigned)responselen);
    }
    return RIL_ERRNO_INVALID_RESPONSE;
  }

  RIL_LceDataInfo *p_cur = (RIL_LceDataInfo *)response;
  p.writeInt32(p_cur->last_hop_capacity_kbps);

  /* p_cur->confidence_level and p_cur->lce_suspended take 1 byte each.*/
  p.write((void *)&(p_cur->confidence_level), 1);
  p.write((void *)&(p_cur->lce_suspended), 1);

  startResponse;
  appendPrintBuf("LCE info received: capacity %d confidence level %d \
                  and suspended %d",
                  p_cur->last_hop_capacity_kbps, p_cur->confidence_level,
                  p_cur->lce_suspended);
  closeResponse;

  return 0;
}

int responseActivityData(Parcel &p, void *response, size_t responselen) {
  if (response == NULL || responselen != sizeof(RIL_ActivityStatsInfo)) {
    if (response == NULL) {
      RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
    }
    else {
      RFX_LOG_E(RFX_LOG_TAG, "responseActivityData: invalid response length %u expecting len: %u",
            (unsigned)sizeof(RIL_ActivityStatsInfo), (unsigned)responselen);
    }
    return RIL_ERRNO_INVALID_RESPONSE;
  }

  RIL_ActivityStatsInfo *p_cur = (RIL_ActivityStatsInfo *)response;
  p.writeInt32(p_cur->sleep_mode_time_ms);
  p.writeInt32(p_cur->idle_mode_time_ms);
  for(int i = 0; i < RIL_NUM_TX_POWER_LEVELS; i++) {
    p.writeInt32(p_cur->tx_mode_time_ms[i]);
  }
  p.writeInt32(p_cur->rx_mode_time_ms);

  startResponse;
  appendPrintBuf("Modem activity info received: sleep_mode_time_ms %d idle_mode_time_ms %d \
                  tx_mode_time_ms %d %d %d %d %d and rx_mode_time_ms %d",
                  p_cur->sleep_mode_time_ms, p_cur->idle_mode_time_ms, p_cur->tx_mode_time_ms[0],
                  p_cur->tx_mode_time_ms[1], p_cur->tx_mode_time_ms[2], p_cur->tx_mode_time_ms[3],
                  p_cur->tx_mode_time_ms[4], p_cur->rx_mode_time_ms);
   closeResponse;

  return 0;
}

int responseCarrierRestrictions(Parcel &p, void *response, size_t responselen) {
  if (response == NULL) {
    RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
    return RIL_ERRNO_INVALID_RESPONSE;
  }
  if (responselen != sizeof(RIL_CarrierRestrictionsWithPriority)) {
    RFX_LOG_E(RFX_LOG_TAG, "responseCarrierRestrictions: invalid response length %u expecting len: %u",
          (unsigned)responselen, (unsigned)sizeof(RIL_CarrierRestrictionsWithPriority));
    return RIL_ERRNO_INVALID_RESPONSE;
  }

  RIL_CarrierRestrictionsWithPriority *p_cr = (RIL_CarrierRestrictionsWithPriority *)response;
  startResponse;

  p.writeInt32(p_cr->len_allowed_carriers);
  p.writeInt32(p_cr->len_excluded_carriers);
  appendPrintBuf(" %s len_allowed_carriers: %d, len_excluded_carriers: %d,", printBuf,
                 p_cr->len_allowed_carriers,p_cr->len_excluded_carriers);

  appendPrintBuf(" %s allowed_carriers:", printBuf);
  for(int32_t i = 0; i < p_cr->len_allowed_carriers; i++) {
    RIL_Carrier *carrier = p_cr->allowed_carriers + i;
    writeStringToParcel(p, carrier->mcc);
    writeStringToParcel(p, carrier->mnc);
    p.writeInt32(carrier->match_type);
    writeStringToParcel(p, carrier->match_data);
    appendPrintBuf(" %s [%d mcc: %s, mnc: %s, match_type: %d, match_data: %s],", printBuf,
                   i, carrier->mcc, carrier->mnc, carrier->match_type, carrier->match_data);
  }

  appendPrintBuf(" %s excluded_carriers:", printBuf);
  for(int32_t i = 0; i < p_cr->len_excluded_carriers; i++) {
    RIL_Carrier *carrier = p_cr->excluded_carriers + i;
    writeStringToParcel(p, carrier->mcc);
    writeStringToParcel(p, carrier->mnc);
    p.writeInt32(carrier->match_type);
    writeStringToParcel(p, carrier->match_data);
    appendPrintBuf(" %s [%d mcc: %s, mnc: %s, match_type: %d, match_data: %s],", printBuf,
                   i, carrier->mcc, carrier->mnc, carrier->match_type, carrier->match_data);
  }

  p.writeBool(p_cr->allowedCarriersPrioritized);
  p.writeInt32(p_cr->simLockMultiSimPolicy);

  closeResponse;

  return 0;
}

int responsePcoData(Parcel &p, void *response, size_t responselen) {
  if (response == NULL) {
    RFX_LOG_E(RFX_LOG_TAG, "responsePcoData: invalid NULL response");
    return RIL_ERRNO_INVALID_RESPONSE;
  }
  if (responselen != sizeof(RIL_PCO_Data)) {
    RFX_LOG_E(RFX_LOG_TAG, "responsePcoData: invalid response length %u, expecting %u",
          (unsigned)responselen, (unsigned)sizeof(RIL_PCO_Data));
    return RIL_ERRNO_INVALID_RESPONSE;
  }

  RIL_PCO_Data *p_cur = (RIL_PCO_Data *)response;
  p.writeInt32(p_cur->cid);
  writeStringToParcel(p, p_cur->bearer_proto);
  p.writeInt32(p_cur->pco_id);
  p.writeInt32(p_cur->contents_length);
  writeStringToParcel(p, p_cur->contents);

  startResponse;
      appendPrintBuf("PCO data received: cid %d, id %d, length %d",
                     p_cur->cid, p_cur->pco_id, p_cur->contents_length);
  closeResponse;

  return 0;
}

int responsePcoIaData(Parcel &p, void *response, size_t responselen) {
  if (response == NULL) {
    RFX_LOG_E(RFX_LOG_TAG, "responsePcoIaData: invalid NULL response");
    return RIL_ERRNO_INVALID_RESPONSE;
  }
  if (responselen != sizeof(RIL_PCO_Data_attached)) {
    RFX_LOG_E(RFX_LOG_TAG, "responsePcoIaData: invalid response length %u, expecting %u",
          (unsigned)responselen, (unsigned)sizeof(RIL_PCO_Data_attached));
    return RIL_ERRNO_INVALID_RESPONSE;
  }

  RIL_PCO_Data_attached *p_cur = (RIL_PCO_Data_attached *)response;
  p.writeInt32(p_cur->cid);
  writeStringToParcel(p, p_cur->apn_name);
  writeStringToParcel(p, p_cur->bearer_proto);
  p.writeInt32(p_cur->pco_id);
  p.writeInt32(p_cur->contents_length);
  writeStringToParcel(p, p_cur->contents);

  startResponse;
      appendPrintBuf("PCO data received: cid %d, id %d, length %d",
                     p_cur->cid, p_cur->pco_id, p_cur->contents_length);
  closeResponse;

  return 0;
}

int responseCrssN(Parcel &p, void *response, size_t responselen) {
    if (response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response: NULL");
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    if (responselen != sizeof(RIL_CrssNotification)) {
        RFX_LOG_E(RFX_LOG_TAG, "invalid response length was %d expected %d",
             (int)responselen, (int)sizeof (RIL_CrssNotification));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_CrssNotification *p_cur = (RIL_CrssNotification *) response;
    p.writeInt32(p_cur->code);
    p.writeInt32(p_cur->type);
    writeStringToParcel(p, p_cur->number);
    writeStringToParcel(p, p_cur->alphaid);
    p.writeInt32(p_cur->cli_validity);
    return 0;
}

int responseLinkCapacityEstimate(Parcel &p, void *response, size_t responselen) {
    if (response == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "responseLinkCapacityEstimate: invalid NULL response");
        return RIL_ERRNO_INVALID_RESPONSE;
    }
    if (responselen != sizeof(RIL_LinkCapacityEstimate)) {
        RFX_LOG_E(RFX_LOG_TAG, "responseLinkCapacityEstimate: invalid response length %u, expecting %u",
                (unsigned)responselen, (unsigned)sizeof(RIL_LinkCapacityEstimate));
        return RIL_ERRNO_INVALID_RESPONSE;
    }

    RIL_LinkCapacityEstimate *p_cur = (RIL_LinkCapacityEstimate *)response;
    p.writeInt32(p_cur->downlinkCapacityKbps);
    p.writeInt32(p_cur->uplinkCapacityKbps);

    startResponse;
        appendPrintBuf("LinkCapacityEstimate received: downlinkCapacityKbps %d uplinkCapacityKbps %d",
                       p_cur->downlinkCapacityKbps, p_cur->uplinkCapacityKbps);
    closeResponse;

    return 0;
}

