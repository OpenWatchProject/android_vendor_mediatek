/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include "property/mtk_properties.h"
#include <stdlib.h>
#include "libwifitest.h"
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <unistd.h>
#include "libnvram.h"
#include "libfile_op.h"

#include "atcid_serial.h"
#include "atcid_util.h"
#include "atcid_cust_cmd_platform.h"
#include "at_tok.h"

#define MAC_ADDR_SIZE 6

int wifiEnableState = -1;
int wifiBand = -1;
uint32_t wifiFreq = 0;
uint32_t wifiRate = 0;
uint32_t wifiGain = 0;
int wifiChannelBandwidth = -1;
int wifiDataBandwidth = -1;
int wifiGain2 = 0;
int wifiTxPacketLength = 0;
int wifiTxPacketCount = 0;
int wifiTxPacketinterval = 0;
int wifiCompen = 0;
int wifiPrimaryChannelOffset = 0;
int wifiTxDataRate = 0;

int gRateCodeWiFi = 0;

extern int sendATCommandToServiceWithResult(char* line);
extern int sendDataToRildSync(char* line, char* response);

ATRESPONSE_t pas_wienable_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0, cmdID = 0;
    bool b = false;
    int r = -1;

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &cmdID);

            if (err < 0) return -1;
            switch (cmdID) {
                case 0:
                    if (wifiEnableState == 1) {
                        r = sendATCommandToServiceWithResult("AT+WITOF=1");
                        if(r != -1)
                            wifiEnableState = 0;
                        else
                            return AT_ERROR;
                    } else {
#ifdef MTK_WLAN_FEATURE
                        b = WIFI_TEST_CloseDUT();
#endif
                        if (b)
                            wifiEnableState = 0;
                        else
                            return AT_ERROR;
                    }
                    return AT_OK;
                case 1:
                    r = sendATCommandToServiceWithResult("AT+WITOF=2");
                    if (r != -1)
                        wifiEnableState = 1;
                    else
                        return AT_ERROR;
                    return AT_OK;
                case 2:
#ifdef MTK_WLAN_FEATURE
                    b = WIFI_TEST_OpenDUT();
#endif
                    if (b)
                        wifiEnableState = 2;
                    else
                        return AT_ERROR;
                    return AT_OK;
            }
            break;
        case AT_READ_OP:
            sprintf(response, "%d", wifiEnableState);
            return AT_OK;
        default:
            break;

    }

    return AT_ERROR;
}

ATRESPONSE_t pas_wimode_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0, cmdID = 0;
    bool b = false;
    uint32_t mode = 0;

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &cmdID);

            if (err < 0) return -1;
            switch (cmdID) {
                case 0:
#ifdef MTK_WLAN_FEATURE
                    b = WIFI_TEST_SetMode(WIFI_TEST_MODE_BY_API_CONTROL);
#endif
                    break;
                case 1:
#ifdef MTK_WLAN_FEATURE
                    b = WIFI_TEST_SetMode(WIFI_TEST_MODE_CW_ONLY);
#endif
                    break;
                case 2:
#ifdef MTK_WLAN_FEATURE
                    b = WIFI_TEST_SetMode(WIFI_TEST_MODE_80211A_ONLY);
#endif
                    break;
                case 3:
#ifdef MTK_WLAN_FEATURE
                    b = WIFI_TEST_SetMode(WIFI_TEST_MODE_80211B_ONLY);
#endif
                    break;
                case 4:
#ifdef MTK_WLAN_FEATURE
                    b = WIFI_TEST_SetMode(WIFI_TEST_MODE_80211G_ONLY);
#endif
                    break;
                case 5:
#ifdef MTK_WLAN_FEATURE
                    b = WIFI_TEST_SetMode(WIFI_TEST_MODE_80211N_ONLY);
#endif
                    break;
                case 6:
#ifdef MTK_WLAN_FEATURE
                    b = WIFI_TEST_SetMode(WIFI_TEST_MODE_80211AC_ONLY);
#endif
                    break;
            }
            if (b)
                return AT_OK;
            else
                return AT_ERROR;
        case AT_TEST_OP:
#ifdef MTK_WLAN_FEATURE
            b = WIFI_TEST_GetSupportedMode(&mode);
#endif
            sprintf(response, "%d", mode);
            break;
        default:
            break;
    }
    if (b)
        return AT_OK;
    else
        return AT_ERROR;
}

ATRESPONSE_t pas_wiband_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0, cmdID = 0;
    bool b = false;
    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &cmdID);
            if (err < 0) return -1;
            switch (cmdID) {
                case 0:
#ifdef MTK_WLAN_FEATURE
                    b = WIFI_TEST_SetBandwidth(WIFI_TEST_BW_20MHZ);
#endif
                    if (b) {
                        sprintf(response, "20MHZ");
                        wifiBand = 0;
                    }
                    break;
                case 1:
#ifdef MTK_WLAN_FEATURE
                    b = WIFI_TEST_SetBandwidth(WIFI_TEST_BW_40MHZ);
#endif
                    if (b) {
                        sprintf(response, "40MHZ");
                        wifiBand = 1;
                    }
                    break;
            }
            if (b)
                return AT_OK;
            else
                return AT_ERROR;
        case AT_READ_OP:
            if (wifiBand == 0) {
                sprintf(response, "20MHZ");
                return AT_OK;
            } else if(wifiBand == 1) {
                sprintf(response, "40MHZ");
                return AT_OK;
            } else {
                return AT_ERROR;
            }
            break;
        default:
            break;
    }
    return AT_ERROR;
}

ATRESPONSE_t pas_wifreq_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0;
    bool b = false;
    uint32_t freq = 0, offset = 0;

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, (int *)&freq);
            if (err < 0) return -1;
#ifdef MTK_WLAN_FEATURE
            b = WIFI_TEST_SetFrequency(freq, offset);
#endif
            if (b) {
                wifiFreq = freq;
                return AT_OK;
            } else {
                wifiFreq = -1;
                return AT_ERROR;
            }
        case AT_READ_OP:
            sprintf(response, "%d", wifiFreq);
            return AT_OK;
        default:
            break;
    }
    return AT_ERROR;
}

ATRESPONSE_t pas_widatarate_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0;
    bool b = false;
    uint32_t rate = 0;

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, (int *)&rate);
            if (err < 0) return -1;
#ifdef MTK_WLAN_FEATURE
            b = WIFI_TEST_SetRate(rate);
#endif
            if (b) {
                wifiRate = rate;
                LOGATCI(LOG_DEBUG, "pas_widatarate_handler [rate=%d] \n", wifiRate);
                gRateCodeWiFi = wifiRate;
                return AT_OK;
            } else {
                wifiRate = -1;
                return AT_ERROR;
            }
        case AT_READ_OP:
            sprintf(response, "%d", wifiRate);
            return AT_OK;
        default:
            break;
    }
    return AT_ERROR;
}

ATRESPONSE_t pas_wipow_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0;
    bool b = false;
    uint32_t gain = 0;

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, (int *)&gain);
            if (err < 0) return -1;
#ifdef MTK_WLAN_FEATURE
            b = WIFI_TEST_SetTXPower(gain);
#endif
            if (b) {
                wifiGain = gain;
                return AT_OK;
            } else {
                wifiGain = -1;
                return AT_ERROR;
            }
        case AT_READ_OP:
            sprintf(response, "%d", wifiGain);
            return AT_OK;
        default:
            break;
    }
    return AT_ERROR;
}

ATRESPONSE_t pas_witxpow_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0, gain = 0;
    bool b = false;

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &gain);
            if (err < 0) return -1;
#ifdef MTK_WLAN_FEATURE
            b = WIFI_TEST_TxGain(gain);
#endif
            wifiGain2 = (b ? gain : -1);
            return (b ? AT_OK : AT_ERROR);
        case AT_READ_OP:
            sprintf(response, "%d", wifiGain2);
            return AT_OK;
        default:
            break;
    }
    return AT_ERROR;
}

ATRESPONSE_t pas_witx_handler(char* cmdline, ATOP_t opType, char* response) {
    UNUSED(response);
    int err = 0, cmd = -1;
    bool b = false;

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &cmd);
            if (err < 0) return -1;
#ifdef MTK_WLAN_FEATURE
            b = WIFI_TEST_SetTX(cmd == 0 ? false : true);
#endif
            if (b)
                return AT_OK;
            else
                return AT_ERROR;
        default:
            break;
    }
    return AT_ERROR;
}

ATRESPONSE_t pas_wirx_handler(char* cmdline, ATOP_t opType, char* response) {
    UNUSED(response);
    int err = 0, cmd = -1;
    bool b = false;
    char srcAddr[MAC_ADDR_SIZE] = {0};
    char dstAddr[MAC_ADDR_SIZE] = {0};
    int64_t tmp = 0;

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &cmd);
            if (err < 0) return -1;
            switch (cmd) {
                case 0:
#ifdef MTK_WLAN_FEATURE
                    b = WIFI_TEST_SetRX(false, srcAddr, dstAddr);
#endif
                    break;
                case 1:
                    err = at_tok_nextint64(&cmdline, &tmp);
                    memcpy(srcAddr, (char *)(&tmp), MAC_ADDR_SIZE);
                    if (err < 0) return -1;
                    err = at_tok_nextint64(&cmdline, &tmp);
                    memcpy(dstAddr, (char *)(&tmp), MAC_ADDR_SIZE);
                    if (err < 0) return -1;
#ifdef MTK_WLAN_FEATURE
                    b = WIFI_TEST_SetRX(true, srcAddr, dstAddr);
#endif
                    break;
            }
        default:
            break;
    }
    if(b)
        return AT_OK;
    else
        return AT_ERROR;
}

ATRESPONSE_t pas_wirpckg_handler(char* cmdline, ATOP_t opType, char* response) {
    UNUSED(cmdline);
    bool b = false;
    uint32_t pu4GoodFrameCount = 0;
    uint32_t pu4BadFrameCount = 0;

    switch (opType) {
        case AT_SET_OP:
#ifdef MTK_WLAN_FEATURE
            b = WIFI_TEST_ClearResult();
#endif
            if (b) {
                return AT_OK;
            } else {
                return AT_ERROR;
            }
        case AT_READ_OP:
#ifdef MTK_WLAN_FEATURE
            b = WIFI_TEST_GetResult(&pu4GoodFrameCount, &pu4BadFrameCount);
#endif
            sprintf(response, "%d,%d OK", pu4GoodFrameCount, pu4BadFrameCount);
            if (b)
                return AT_OK;
            else
                return AT_ERROR;
        default:
            break;
    }
    return AT_ERROR;
}

ATRESPONSE_t pas_wirssi_handler(char* cmdline, ATOP_t opType, char* response) {
    UNUSED(cmdline);
    int rssi = -127, rxok = -1, rxerror = -1;
    bool b = false;

    switch (opType) {
        case AT_READ_OP:
#ifdef MTK_WLAN_FEATURE
            WIFI_TEST_FRGood(&rxok);
            WIFI_TEST_FRError(&rxerror);
            LOGATCI(LOG_DEBUG, "RX ok:%d, error:%d", rxok, rxerror);
            b = WIFI_TEST_RSSI(&rssi);
            LOGATCI(LOG_DEBUG, "WIFI_TEST_RSSI got retrun: %s", b ? "true" : "false");
#endif
            sprintf(response, "Current RSSI:%d, RX OK/ERR = %d/%d", rssi, rxok, rxerror);
            return (b ? AT_OK : AT_ERROR);
        default:
            break;
    }
    return AT_ERROR;
}

ATRESPONSE_t pas_wigi_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0, guardInterval = 0;
    bool b = false;

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &guardInterval);
            if (err < 0) return -1;
#ifdef MTK_WLAN_FEATURE
            b = WIFI_TEST_GI(guardInterval);
            sprintf(response, "Set guard interval:%d", guardInterval);
#endif
            return (b ? AT_OK : AT_ERROR);
        default:
            break;
    }
    return AT_ERROR;
}


ATRESPONSE_t pas_wipreamble_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0, preamble = 0;
    bool b = false;

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &preamble);
            if (err < 0) return -1;
#ifdef MTK_WLAN_FEATURE
            b = WIFI_TEST_SetPreamble(preamble);
            sprintf(response, "Set preamble:%d", preamble);
#endif
            return (b ? AT_OK : AT_ERROR);
        default:
            break;
    }
    return AT_ERROR;
}

ATRESPONSE_t pas_witxpktlen_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0, txPacketLength = 0;
    bool b = false;

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &txPacketLength);
            if (err < 0) return -1;
#ifdef MTK_WLAN_FEATURE
            b = WIFI_TEST_TxPayloadLength(txPacketLength);
#endif
            wifiTxPacketLength = (b ? txPacketLength : -1);
            return (b ? AT_OK : AT_ERROR);
        case AT_READ_OP:
            sprintf(response, "%d", wifiTxPacketLength);
            return AT_OK;
        default:
            break;
    }
    return AT_ERROR;
}

ATRESPONSE_t pas_witxpktcnt_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0, txPacketCount = 0;
    bool b = false;

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &txPacketCount);
            if (err < 0) return -1;
#ifdef MTK_WLAN_FEATURE
            b = WIFI_TEST_TxBurstFrames(txPacketCount);
#endif
            wifiTxPacketCount = (b ? txPacketCount : -1);
            return (b ? AT_OK : AT_ERROR);
        case AT_READ_OP:
            sprintf(response, "%d", wifiTxPacketCount);
            return AT_OK;
        default:
            break;
    }
    return AT_ERROR;
}

ATRESPONSE_t pas_witxpktinterval_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0, txPacketInterval = 0;
    bool b = false;

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &txPacketInterval);
            if (err < 0) return -1;
#ifdef MTK_WLAN_FEATURE
            b = WIFI_TEST_TxBurstInterval(txPacketInterval);
#endif
            wifiTxPacketinterval = (b ? txPacketInterval : -1);
            return (b ? AT_OK : AT_ERROR);
        case AT_READ_OP:
            sprintf(response, "%d", wifiTxPacketinterval);
            return AT_OK;
        default:
            break;
    }
    return AT_ERROR;
}

ATRESPONSE_t pas_wichbandwidth_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0, cmdID = 0;
    bool b = false;

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &cmdID);
            if (err < 0) return -1;
            switch (cmdID) {
                case 0:
#ifdef MTK_WLAN_FEATURE
                    b = WIFI_TEST_SetBandwidthV2(WIFI_TEST_CH_BW_20MHZ);
#endif
                    if (b) {
                        sprintf(response, "Set channel bandwidth: 20MHZ");
                        wifiChannelBandwidth = 0;
                    }
                    break;
                case 1:
#ifdef MTK_WLAN_FEATURE
                    b = WIFI_TEST_SetBandwidthV2(WIFI_TEST_CH_BW_40MHZ);
#endif
                    if (b) {
                        sprintf(response, "Set channel bandwidth: 40MHZ");
                        wifiChannelBandwidth = 1;
                    }
                    break;
                case 2:
#ifdef MTK_WLAN_FEATURE
                    b = WIFI_TEST_SetBandwidthV2(WIFI_TEST_CH_BW_80MHZ);
#endif
                    if (b) {
                        sprintf(response, "Set channel bandwidth: 80MHZ");
                        wifiChannelBandwidth = 2;
                    }
                    break;
                case 3:
#ifdef MTK_WLAN_FEATURE
                    b = WIFI_TEST_SetBandwidthV2(WIFI_TEST_CH_BW_160MHZ);
#endif
                    if (b) {
                        sprintf(response, "Set channel bandwidth: 160MHZ");
                        wifiChannelBandwidth = 3;
                    }
                    break;
            }
            return (b ? AT_OK : AT_ERROR);
        case AT_READ_OP:
            if (wifiChannelBandwidth >= 0 && wifiChannelBandwidth < WIFI_TEST_CH_BW_NUM) {
                sprintf(response, "Channel bandwidth: %d", wifiChannelBandwidth);
            } else {
                sprintf(response, "No define for channel band");
                return AT_OK;
            }
            break;
        default:
            break;
    }
    return AT_ERROR;
}

ATRESPONSE_t pas_widatabandwidth_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0, cmdID = 0;
    bool b = false;

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &cmdID);
            if (err < 0) return -1;
            switch (cmdID) {
                case 0:
#ifdef MTK_WLAN_FEATURE
                    b = WIFI_TEST_SetTxBandwidth(WIFI_TEST_CH_BW_20MHZ);
#endif
                    if (b) {
                        sprintf(response, "Set data bandwidth: 20MHZ");
                        wifiDataBandwidth= 0;
                    }
                    break;
                case 1:
#ifdef MTK_WLAN_FEATURE
                    b = WIFI_TEST_SetTxBandwidth(WIFI_TEST_CH_BW_40MHZ);
#endif
                    if (b) {
                        sprintf(response, "Set data bandwidth: 40MHZ");
                        wifiDataBandwidth = 1;
                    }
                    break;
                case 2:
#ifdef MTK_WLAN_FEATURE
                    b = WIFI_TEST_SetTxBandwidth(WIFI_TEST_CH_BW_80MHZ);
#endif
                    if (b) {
                        sprintf(response, "Set data bandwidth: 80MHZ");
                        wifiDataBandwidth = 2;
                    }
                    break;
                case 3:
#ifdef MTK_WLAN_FEATURE
                    b = WIFI_TEST_SetTxBandwidth(WIFI_TEST_CH_BW_160MHZ);
#endif
                    if (b) {
                        sprintf(response, "Set data bandwidth: 160MHZ");
                        wifiDataBandwidth = 3;
                    }
                    break;
            }
            return (b ? AT_OK : AT_ERROR);
        case AT_READ_OP:
            if (wifiDataBandwidth >= 0 && wifiDataBandwidth < WIFI_TEST_CH_BW_NUM) {
                sprintf(response, "Data bandwidth: %d", wifiDataBandwidth);
            } else {
                sprintf(response, "No define for data bandwidth");
                return AT_OK;
            }
            break;
        default:
            break;
    }
    return AT_ERROR;
}


ATRESPONSE_t pas_wiprimarychset_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0, primaryChannelOffset = 0;
    bool b = false;

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &primaryChannelOffset);
            if (err < 0) return -1;
#ifdef MTK_WLAN_FEATURE
            b = WIFI_TEST_SetPriChannelSetting(primaryChannelOffset);
#endif
            wifiPrimaryChannelOffset = (b ? primaryChannelOffset : -1);
            return (b ? AT_OK : AT_ERROR);
        case AT_READ_OP:
            sprintf(response, "%d", wifiPrimaryChannelOffset);
            return AT_OK;
        default:
            break;
    }
    return AT_ERROR;
}

ATRESPONSE_t pas_witxdatarate_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0, txDataRate = 0;
    bool b = false;

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &txDataRate);
            if (err < 0) return -1;
#ifdef MTK_WLAN_FEATURE
            b = WIFI_TEST_TxDataRate(txDataRate);
#endif
            wifiTxDataRate = (b ? txDataRate : -1);
            return (b ? AT_OK : AT_ERROR);
        case AT_READ_OP:
            sprintf(response, "%d", wifiTxDataRate);
            return AT_OK;
        default:
            break;
    }
    return AT_ERROR;
}

ATRESPONSE_t pas_wirxstart_handler(char* cmdline, ATOP_t opType, char* response) {
    UNUSED(response);
    int err = 0, cmd = -1;
    bool b = false;

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &cmd);
            if (err < 0) return -1;
#ifdef MTK_WLAN_FEATURE
            b = (cmd == 0 ? WIFI_TEST_RxStop() : WIFI_TEST_RxStart());
#endif
            return (b ? AT_OK : AT_ERROR);
        default:
            break;
    }
    return AT_ERROR;
}

ATRESPONSE_t pas_witestset_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0, index = -1, data = -1;
#ifdef MTK_WLAN_FEATURE
    int operation = -1;
#endif

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &index);
            if (err < 0) {
                return AT_ERROR;
            }

            err = at_tok_nextint(&cmdline, &data);
            if (err < 0) {
                return AT_ERROR;
            }
#ifdef MTK_WLAN_FEATURE
            operation = WIFI_TEST_set(index, data, NULL, NULL);
            sprintf(response, "Set index:%d to %d %s", index, data,
                    (operation == 0 ? "successful" : "failed"));
            return (operation == 0 ? AT_OK : AT_ERROR);
#else
            LOGATCI(LOG_DEBUG,
                "AT_SET_OP failed - MTK_WLAN_FEATURE is disabled.\n");
            sprintf(response, "Set index:%d to %d failed", index, data);
            return AT_ERROR;
#endif
        case AT_READ_OP:
            err = at_tok_nextint(&cmdline, &index);
            if (err < 0) {
                return AT_ERROR;
            }
#ifdef MTK_WLAN_FEATURE
            operation = WIFI_TEST_get(index, 0, NULL, (uint32_t *)&data);
            sprintf(response, "Get index:%d --> data:%d %s", index, data,
                (operation == 0 ? "successful" : "failed"));
            return (operation == 0 ? AT_OK : AT_ERROR);
#else
            LOGATCI(LOG_DEBUG,
                "AT_READ_OP failed - MTK_WLAN_FEATURE is disabled.\n");
            sprintf(response, "Get index:%d --> data:%d failed", index, data);
            return AT_ERROR;
#endif
        default:
            break;
    }
    return AT_ERROR;
}

#ifndef MTK_TC1_FEATURE
ATRESPONSE_t pas_witestmode_handler(char* cmdline, ATOP_t opType, char* response) {
    UNUSED(cmdline);
    UNUSED(opType);
    UNUSED(response);
    return AT_ERROR;
}

ATRESPONSE_t pas_witx2_handler(char* cmdline, ATOP_t opType, char* response) {
    UNUSED(cmdline);
    UNUSED(opType);
    UNUSED(response);
    return AT_ERROR;
}

ATRESPONSE_t pas_wirx2_handler(char* cmdline, ATOP_t opType, char* response) {
    UNUSED(cmdline);
    UNUSED(opType);
    UNUSED(response);
    return AT_ERROR;
}

ATRESPONSE_t pas_wimac_handler(char* cmdline, ATOP_t opType, char* response) {
    UNUSED(cmdline);
    UNUSED(opType);
    UNUSED(response);
    return AT_ERROR;
}

ATRESPONSE_t pas_wimacck_handler(char* cmdline, ATOP_t opType, char* response) {
    UNUSED(cmdline);
    UNUSED(opType);
    UNUSED(response);
    return AT_ERROR;
}
#endif

#define MMC_DEV            "/dev/block/mmcblk1"
#define EMMC_SIZE          "/sys/block/mmcblk0/size"
#define MEM_INFO           "/proc/meminfo"
#define BATTERY_VOLTAGE    "/sys/class/power_supply/battery/voltage_now"
#define BATTERY_TEMPRATURE "/sys/class/power_supply/battery/temp"
#define BSN_FILE_ID 61
#define BSN_RECORD_ID 1
#define BSN_OFFSET 0
#define BSN_SIZE 64
#define SN_FILE_ID 61
#define SN_RECORD_ID 1
#define SN_OFFSET 116
#define SN_SIZE 20
#define BT_FILE_ID 1
#define BT_RECORD_ID 1
#define BT_OFFSET 0
#define BT_SIZE 6
#define WLAN_FILE_ID 55
#define WLAN_RECORD_ID 1
#define WLAN_OFFSET 4
#define WLAN_SIZE 6
#define FAC_FILE_ID 61
#define FAC_RECORD_ID 1
#define FAC_OFFSET 512
#define FAC_SIZE 150
#define META_FLAG_FILE_ID 61
#define META_FLAG_RECORD_ID 1
#define META_FLAG_OFFSET 338
#define META_FLAG_SIZE 12

// offset = 0, size = 0 for read whole record
bool readNvram(int fileId, int recordId, int offset, int size, char **buffer) {
    F_ID fd;
    F_INFO fileInfo;
    int recSize = 0, readSize;

    LOGATCI(LOG_INFO, "readNvram fileId = %d, recordId = %d", fileId, recordId);
    fd = NVM_GetFileDesc(fileId, &(fileInfo.i4RecSize), &(fileInfo.i4RecNum), true);
    if (fd.iFileDesc == -1) {
        LOGATCI(LOG_ERR, "readNvram(), can't open file");
        return false;
    }
    if (recordId > fileInfo.i4RecNum || size > fileInfo.i4RecSize) {
        LOGATCI(LOG_ERR, "readNvram(), recordId: %d, i4RecNum: %d, size: %d, i4RecSize: %d",
                recordId, fileInfo.i4RecNum, size, fileInfo.i4RecSize);
        NVM_CloseFileDesc(fd);
        return false;
    }

    recSize = fileInfo.i4RecSize;
    *buffer = (char *)calloc(1, recSize + 1);
    if (*buffer == NULL) {
        LOGATCI(LOG_ERR, "readNvram(), malloc error");
        NVM_CloseFileDesc(fd);
        return false;
    }
    LOGATCI(LOG_INFO, "readNvram(), seek to (%d - 1) * %d + %d", recordId, recSize, offset);
    lseek(fd.iFileDesc, (recordId - 1) * recSize + offset, SEEK_CUR);

    if (size == 0) {
        size = recSize;
    }
    readSize = read(fd.iFileDesc, *buffer, size);
    LOGATCI(LOG_INFO, "readNvram(), read, size=%d, readSize=%d, buffer=%s", size, readSize, *buffer);
    for (int i = 0; i < readSize; i++) {
        LOGATCI(LOG_INFO, "readNvram(), buffer[%d]=0x%X", i, (*buffer)[i]);
    }
    if (readSize != size) {
        LOGATCI(LOG_ERR, "readNvram(), Read size not match: readSize=%d, size=%d, errno=%s",
                readSize, size, strerror(errno));
        NVM_CloseFileDesc(fd);
        free(*buffer);
        *buffer = NULL;
        return false;
    }
    NVM_CloseFileDesc(fd);
    LOGATCI(LOG_INFO, "readNvram(), %s, readSize = %d", *buffer, readSize);
    return true;
}

bool writeNvram(int fileId, int recordId, int offset, int size, char *buffer) {
    F_INFO fileInfo;
    F_ID fd;
    int recSize = 0, writeSize;

    LOGATCI(LOG_INFO, "writeNvram(), %s, fileId = %d, recordId = %d", buffer, fileId, recordId);
    if (buffer == NULL) {
        LOGATCI(LOG_ERR, "writeNvram(), buffer error\n");
        return false;
    }
    fd = NVM_GetFileDesc(fileId, &(fileInfo.i4RecSize), &(fileInfo.i4RecNum), false);
    if (fd.iFileDesc == -1) {
        LOGATCI(LOG_ERR, "writeNvram(), can't open file");
        return false;
    }

    if ((recordId > fileInfo.i4RecNum) || (size > fileInfo.i4RecSize)) {
        LOGATCI(LOG_ERR, "writeNvram(), recordId=%d, i4RecNum=%d, size=%d, i4RecSize=%d",
                recordId, fileInfo.i4RecNum, size, fileInfo.i4RecSize);
        NVM_CloseFileDesc(fd);
        return false;
    }

    recSize = fileInfo.i4RecSize;
    LOGATCI(LOG_INFO, "writeNvram(), seek to (%d - 1) * %d + %d", recordId, recSize, offset);
    lseek(fd.iFileDesc, (recordId - 1) * recSize + offset, SEEK_CUR);

    if (size == 0) {
        size = recSize;
    }
    writeSize = write(fd.iFileDesc, buffer, size);
    LOGATCI(LOG_INFO, "writeNvram(), write, size=%d, writeSize=%d, buffer=%s", size, writeSize, buffer);
    if (writeSize != size) {
        LOGATCI(LOG_ERR, "writeNvram(), write size not match: writeSize=%d, recSize=%d, errno=%s",
                writeSize, size, strerror(errno));
        NVM_CloseFileDesc(fd);
        return false;
    }
    NVM_CloseFileDesc(fd);
    LOGATCI(LOG_INFO, "writeNvram(), writeSize = %d", writeSize);

    LOGATCI(LOG_INFO, "AddBackupFileNum Begin");
    NVM_AddBackupFileNum(fileId);
    LOGATCI(LOG_INFO, "AddBackupFileNum End");
    return true;
}

ATRESPONSE_t bsn_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0;
    char* value = NULL;
    LOGATCI(LOG_DEBUG, "cmdline: %s", cmdline);

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextstr(&cmdline, &value);
            if (err < 0) {
                return AT_ERROR;
            }

            if (writeNvram(BSN_FILE_ID, BSN_RECORD_ID, BSN_OFFSET, BSN_SIZE, value)) {
                snprintf(response, MAX_DATA_SIZE, "\r\n^BSN:");
                return AT_OK;
            }
            break;
        case AT_READ_OP:
            if (readNvram(BSN_FILE_ID, BSN_RECORD_ID, BSN_OFFSET, BSN_SIZE, &value)) {
                snprintf(response, MAX_DATA_SIZE, "\r\n^BSN:%s", value);
                free(value);
                return AT_OK;
            }
            break;
        default:
            break;
    }
    return AT_ERROR;
}

ATRESPONSE_t sn_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0;
    char* value = NULL;
    LOGATCI(LOG_DEBUG, "cmdline: %s", cmdline);

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextstr(&cmdline, &value);
            if (err < 0) {
                return AT_ERROR;
            }

            if (writeNvram(SN_FILE_ID, SN_RECORD_ID, SN_OFFSET, SN_SIZE, value)) {
                snprintf(response, MAX_DATA_SIZE, "\r\n^SN:");
                return AT_OK;
            }
            break;
        case AT_READ_OP:
            if (readNvram(SN_FILE_ID, SN_RECORD_ID, SN_OFFSET, SN_SIZE, &value)) {
                snprintf(response, MAX_DATA_SIZE, "\r\n^SN:%s", value);
                free(value);
                return AT_OK;
            }
            break;
        default:
            break;
    }
    return AT_ERROR;
}

void readImei(char *command, char *ret) {
    char *out = NULL, *response = NULL;
    char buffer[MAX_DATA_SIZE] = {0};
    int err = -1;

    sendDataToRildSync(command, buffer);
    response = buffer;
    err = at_tok_start(&response);
    if (err < 0) {
        strncpy(ret, "ERROR", strlen("ERROR") + 1);
        return;
    }
    err = at_tok_nextstr(&response, &out);
    if (err < 0) {
        strncpy(ret, "ERROR", strlen("ERROR") + 1);
        return;
    }
    if (strncmp(out, "0x", strlen("0x")) == 0) {
        out += strlen("0x");
    }
    char* find = strchr(out, '\r');
    if (find != NULL) {
        *find = '\0';
    }
    strncpy(ret, out, MAX_DATA_SIZE);
}

ATRESPONSE_t phy_num_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0;
    unsigned int i = 0, j = 0;
    char *type = NULL, *value = NULL;
    char imei1[MAX_DATA_SIZE] = {0};
    char imei2[MAX_DATA_SIZE] = {0};
    char meid[MAX_DATA_SIZE] = {0};
    char btMac[MAX_DATA_SIZE] = {0};
    char wlanMac[MAX_DATA_SIZE] = {0};
    LOGATCI(LOG_DEBUG, "cmdline: %s", cmdline);

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextstr(&cmdline, &type);
            if (err < 0) {
                return AT_ERROR;
            }
            err = at_tok_nextstr(&cmdline, &value);
            if (err < 0) {
                return AT_ERROR;
            }
            LOGATCI(LOG_DEBUG, "%s, %s", type, value);
            for (i = 0; i < strlen(value); i++) {
                if ((value[i] >= '0' && value[i] <= '9') || (value[i] >= 'A' && value[i] <= 'F')
                        || (value[i] >= 'a' && value[i] <= 'f')) {
                    value[j] = value[i];
                    j += 1;
                }
            }
            value[j] = '\0';

            if (strcasecmp(type, "MACBT") == 0) {
                if (j < BT_SIZE * 2) {
                    return AT_ERROR;
                }
                hex_to_ascii(value, btMac, BT_SIZE * 2);
                if (writeNvram(BT_FILE_ID, BT_RECORD_ID, BT_OFFSET, BT_SIZE, btMac)) {
                    snprintf(response, MAX_DATA_SIZE, "\r\n^PHYNUM:");
                    return AT_OK;
                }
            } else if (strcasecmp(type, "MACWLAN") == 0) {
                if (j < WLAN_SIZE * 2) {
                    return AT_ERROR;
                }
                hex_to_ascii(value, wlanMac, WLAN_SIZE * 2);
                if (writeNvram(WLAN_FILE_ID, WLAN_RECORD_ID, WLAN_OFFSET, WLAN_SIZE, wlanMac)) {
                    snprintf(response, MAX_DATA_SIZE, "\r\n^PHYNUM:");
                    return AT_OK;
                }
            }
            break;
        case AT_READ_OP:
            readImei("AT+EGMR=0,7", imei1);
            readImei("AT+EGMR=0,10", imei2);
            readImei("AT^MEID", meid);
            if (readNvram(BT_FILE_ID, BT_RECORD_ID, BT_OFFSET, BT_SIZE, &value)) {
                ascii_to_hex(value, btMac, BT_SIZE);
                free(value);
            }
            if (readNvram(WLAN_FILE_ID, WLAN_RECORD_ID, WLAN_OFFSET, WLAN_SIZE, &value)) {
                ascii_to_hex(value, wlanMac, WLAN_SIZE);
                free(value);
            }
            snprintf(response, MAX_DATA_SIZE,
                    "\r\n^PHYNUM:\r\nIMEI1:%s\r\nIMEI2:%s\r\nMEID:%s\r\nMACWLAN:%s\r\nMACBT:%s",
                    imei1, imei2, meid, wlanMac, btMac);
            return AT_OK;
            break;
        default:
            break;
    }
    return AT_ERROR;
}

ATRESPONSE_t get_emmc_handler(char* cmdline, ATOP_t at_op, char* response) {
    UNUSED(at_op);
    char buffer[MAX_DATA_SIZE + 1] = {0};
    LOGATCI(LOG_INFO, "emmc_size_handler cmdline = %s", cmdline);

    int fd = open(EMMC_SIZE, O_RDONLY);
    if (fd < 0) {
        LOGATCI(LOG_ERR, "emmc_size_handler(), can't open file");
        return AT_ERROR;
    }

    if (read(fd, buffer, MAX_DATA_SIZE) <= 0) {
        LOGATCI(LOG_ERR, "emmc_size_handler(), can't read file, errno=%s", strerror(errno));
        close(fd);
        return AT_ERROR;
    }
    close(fd);

    long long size = atoll(buffer) * 512;
    LOGATCI(LOG_INFO, "emmc_size_handler(), size = %lld", size);
    sprintf(response, "\r\n^GETEMMC:%lld", size);
    return AT_OK;
}

ATRESPONSE_t get_ram_handler(char* cmdline, ATOP_t at_op, char* response) {
    UNUSED(at_op);
    char buffer[MAX_DATA_SIZE + 1] = {0};
    LOGATCI(LOG_INFO, "mem_size_handler cmdline = %s", cmdline);

    int fd = open(MEM_INFO, O_RDONLY);
    if (fd < 0) {
        LOGATCI(LOG_ERR, "mem_size_handler(), can't open file");
        return AT_ERROR;
    }

    memset(buffer, 0, MAX_DATA_SIZE + 1);
    if (read(fd, buffer, MAX_DATA_SIZE) <= 0) {
        LOGATCI(LOG_ERR, "mem_size_handler(), can't read file, errno=%s", strerror(errno));
        close(fd);
        return AT_ERROR;
    }
    close(fd);

    buffer[MAX_DATA_SIZE] = '\0';
    char *memTotal = strstr(buffer, "MemTotal:");
    if (memTotal != NULL) {
        int size = atoi(memTotal + strlen("MemTotal:"));
        LOGATCI(LOG_INFO, "mem_size_handler(), size = %d", size);
        sprintf(response, "\r\n^GETRAM:%d kB", size);
        return AT_OK;
    } else {
        LOGATCI(LOG_INFO, "mem_size_handler(), no MemTotal");
        return AT_ERROR;
    }
}

ATRESPONSE_t swver_handler(char* cmdline, ATOP_t at_op, char* response) {
    UNUSED(at_op);
    char systemVersion[MTK_PROPERTY_VALUE_MAX] = {0};
    char vendorVersion[MTK_PROPERTY_VALUE_MAX] = {0};
    LOGATCI(LOG_DEBUG, "LOG_DEBUG, device_version_handler cmdline:%s", cmdline);
    mtk_property_get("ro.build.version.release", systemVersion, "");
    mtk_property_get("ro.vendor.mediatek.version.release", vendorVersion, "");
    snprintf(response, MAX_DATA_SIZE,
            "\r\n^SWVER:\r\nVER1:ro.build.version.release=%s\r\nVER2:ro.vendor.mediatek.version.release=%s",
            systemVersion, vendorVersion);
    return AT_OK;
}

ATRESPONSE_t fac_info_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0;
    char* value = NULL;
    LOGATCI(LOG_DEBUG, "cmdline: %s", cmdline);

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextstr(&cmdline, &value);
            if (err < 0) {
                return AT_ERROR;
            }

            if (writeNvram(FAC_FILE_ID, FAC_RECORD_ID, FAC_OFFSET, FAC_SIZE, value)) {
                snprintf(response, MAX_DATA_SIZE, "\r\n^FACINFO:");
                return AT_OK;
            }
            break;
        case AT_READ_OP:
            if (readNvram(FAC_FILE_ID, FAC_RECORD_ID, FAC_OFFSET, FAC_SIZE, &value)) {
                for (int i = 0; i < FAC_SIZE; i++) {
                    if (value[i] < ' ' || value[i] > '~') {
                        value[i] = ' ';
                    }
                }
                snprintf(response, MAX_DATA_SIZE, "\r\n^FACINFO:%s", value);
                free(value);
                return AT_OK;
            }
            break;
        default:
            break;
    }
    return AT_ERROR;
}

ATRESPONSE_t dev_boot_state_handler(char* cmdline, ATOP_t at_op, char* response) {
    UNUSED(at_op);
    char buffer[MTK_PROPERTY_VALUE_MAX] = {0};
    LOGATCI(LOG_DEBUG, "cmdline:%s", cmdline);
    mtk_property_get("vendor.mtk.atci.boot_completed", buffer, "0");
    snprintf(response, MAX_DATA_SIZE, "\r\n^DEVBOOTSTATE:%s", buffer);
    return AT_OK;
}

ATRESPONSE_t product_info_handler(char* cmdline, ATOP_t at_op, char* response) {
    UNUSED(at_op);
    char buffer[MTK_PROPERTY_VALUE_MAX] = {0};
    LOGATCI(LOG_DEBUG, "cmdline:%s", cmdline);
    mtk_property_get("persist.vendor.service.atci.product_info", buffer, "");
    snprintf(response, MAX_DATA_SIZE, "\r\n^GETPRODUCTINFO:%s", buffer);
    return AT_OK;
}

ATRESPONSE_t vendor_country_handler(char* cmdline, ATOP_t at_op, char* response) {
    UNUSED(at_op);
    char buffer[MTK_PROPERTY_VALUE_MAX] = {0};
    LOGATCI(LOG_DEBUG, "cmdline:%s", cmdline);

    switch (at_op) {
        case AT_SET_OP:
            mtk_property_set("persist.vendor.service.atci.vendor_country", cmdline);
            snprintf(response, MAX_DATA_SIZE, "\r\n^VENDORCOUNTRY:");
            return AT_OK;
            break;
        case AT_READ_OP:
            mtk_property_get("persist.vendor.service.atci.vendor_country", buffer, "");
            snprintf(response, MAX_DATA_SIZE, "\r\n^VENDORCOUNTRY:%s", buffer);
            return AT_OK;
            break;
        default:
            break;
    }
    return AT_ERROR;
}

ATRESPONSE_t dev_model_handler(char* cmdline, ATOP_t at_op, char* response) {
    UNUSED(at_op);
    char buffer[MTK_PROPERTY_VALUE_MAX] = {0};
    LOGATCI(LOG_DEBUG, "cmdline:%s", cmdline);

    switch (at_op) {
        case AT_SET_OP:
            mtk_property_set("persist.vendor.service.atci.product_info", cmdline);
            snprintf(response, MAX_DATA_SIZE, "\r\n^DEVMODEL:");
            return AT_OK;
            break;
        case AT_READ_OP:
            mtk_property_get("persist.vendor.service.atci.product_info", buffer, "");
            snprintf(response, MAX_DATA_SIZE, "\r\n^DEVMODEL:%s", buffer);
            return AT_OK;
            break;
        default:
            break;
    }
    return AT_ERROR;
}

ATRESPONSE_t sd_state_handler(char* cmdline, ATOP_t at_op, char* response) {
    UNUSED(at_op);
    LOGATCI(LOG_DEBUG, "cmdline:%s", cmdline);

    if (access(MMC_DEV, 0) < 0) {
        ALOGE("AT_MMC_CMD " "The SD/MMC device isn't exist.");
        sprintf(response, "\r\n^SDSTATE:0");
    } else {
        sprintf(response, "\r\n^SDSTATE:1");
    }

    return AT_OK;
}

ATRESPONSE_t battery_volt_handler(char* cmdline, ATOP_t at_op, char* response) {
    UNUSED(at_op);
    char buffer[MAX_DATA_SIZE + 1] = {0};
    LOGATCI(LOG_INFO, "battery_volt_handler cmdline = %s", cmdline);

    int fd = open(BATTERY_VOLTAGE, O_RDONLY);
    if (fd < 0) {
        LOGATCI(LOG_ERR, "battery_volt_handler(), can't open file");
        return AT_ERROR;
    }

    if (read(fd, buffer, MAX_DATA_SIZE) <= 0) {
        LOGATCI(LOG_ERR, "battery_volt_handler(), can't read file, errno=%s", strerror(errno));
        close(fd);
        return AT_ERROR;
    }
    close(fd);

    int size = atoi(buffer);
    LOGATCI(LOG_INFO, "battery_volt_handler(), size = %d", size);
    sprintf(response, "\r\n^TBATVOLT:%d", size);
    return AT_OK;
}

ATRESPONSE_t battery_temp_handler(char* cmdline, ATOP_t at_op, char* response) {
    UNUSED(at_op);
    char buffer[MAX_DATA_SIZE + 1] = {0};
    LOGATCI(LOG_INFO, "battery_temp_handler cmdline = %s", cmdline);

    int fd = open(BATTERY_TEMPRATURE, O_RDONLY);
    if (fd < 0) {
        LOGATCI(LOG_ERR, "battery_temp_handler(), can't open file");
        return AT_ERROR;
    }

    if (read(fd, buffer, MAX_DATA_SIZE) <= 0) {
        LOGATCI(LOG_ERR, "battery_temp_handler(), can't read file, errno=%s", strerror(errno));
        close(fd);
        return AT_ERROR;
    }
    close(fd);

    int size = atoi(buffer);
    LOGATCI(LOG_INFO, "battery_temp_handler(), size = %d", size);
    sprintf(response, "\r\n^TBATTEMP:%d", size);
    return AT_OK;
}

ATRESPONSE_t power_down_handler(char* cmdline, ATOP_t opType, char* response) {
    UNUSED(response);
    LOGATCI(LOG_DEBUG, "handle cmdline:%s", cmdline);

    if (opType != AT_ACTION_OP) {
        return AT_ERROR;
    }

    switch (opType) {
        case AT_ACTION_OP:
            mtk_property_set("sys.powerctl", "shutdown");
            snprintf(response, MAX_DATA_SIZE, "\r\n^POWERDOWN:");
            return AT_OK;
            break;
        default:
            break;
    }

    return AT_ERROR;
}

ATRESPONSE_t nv_backup_handler(char* cmdline, ATOP_t opType, char* response) {
    UNUSED(opType);
    char buffer[MAX_DATA_SIZE] = {0};
    LOGATCI(LOG_DEBUG, "nvram_handler enter with opType: %s", cmdline);

    time_t timep;
    time(&timep);
    struct tm *p = gmtime(&timep);
    sprintf(buffer, "%04d-%02d-%02d-%02d-%02d-%02d", 1900 + p->tm_year, 1 + p->tm_mon, p->tm_mday,
            p->tm_hour, p->tm_min, p->tm_sec);
    LOGATCI(LOG_DEBUG, "%s", buffer);

    FileOp_BackupToBinRegion_All_Exx((unsigned char *)buffer);
    snprintf(response, MAX_DATA_SIZE, "\r\n^NVBACKUP:");
    return AT_OK;
}

ATRESPONSE_t meta_flag_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0, arg1 = 0, arg2 = 0;
    char buffer[MAX_DATA_SIZE] = {0};
    char* value = NULL;
    LOGATCI(LOG_DEBUG, "cmdline: %s", cmdline);

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &arg1);
            if (err < 0) {
                return AT_ERROR;
            }

            err = at_tok_nextint(&cmdline, &arg2);
            if (err < 0) {
                return AT_ERROR;
            }

            snprintf(buffer, MAX_DATA_SIZE, "FACTORY!%1d--%1d", arg1, arg2);
            if (writeNvram(META_FLAG_FILE_ID, META_FLAG_RECORD_ID, META_FLAG_OFFSET, META_FLAG_SIZE, buffer)) {
                return AT_OK;
            }
            break;
        case AT_READ_OP:
            if (readNvram(META_FLAG_FILE_ID, META_FLAG_RECORD_ID, META_FLAG_OFFSET, META_FLAG_SIZE, &value)) {
                arg1 = atoi(value + strlen("FACTORY!"));
                arg2 = atoi(value + strlen("FACTORY!?--"));
                snprintf(response, MAX_DATA_SIZE, "\r\n^MODEFLAG:%d,%d", arg1, arg2);
                free(value);
                return AT_OK;
            }
            break;
        default:
            break;
    }
    return AT_ERROR;
}
