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
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>
#include <unistd.h>

#include "atcid_serial.h"
#include "atcid_util.h"
#include "atcid_cust_cmd.h"
#include "at_tok.h"

#define ATCI_SIM "persist.vendor.service.atci.sim"
#define PROPERTY_SYS_USB_STATE "sys.usb.state"
#define PROPERTY_VENDOR_USB_CONFIG "vendor.usb.config"
#define ENABLE_URC_PROP "persist.vendor.service.atci_urc.enable"
#define MULTI_SIM "persist.radio.multisim.config"

#define VZWAPNE_SYNC_TO_AP 0
#define VZWAPNE_BYPASS 1

#define CMEC_DEFAULT 2

int gCsdfMode = 1;
int gCsdfAuxmode = 1;
int gCclkTz = 0xFFFF;

int gVzwapneConfig = VZWAPNE_SYNC_TO_AP;

int gCmecConfig[CMEC_CONFIG_COUNT] = {CMEC_DEFAULT, CMEC_DEFAULT, CMEC_DEFAULT, CMEC_DEFAULT};

extern int s_fdService_command;
/*misc global vars */
extern Serial serial;

extern void setSocketConnect(SocketId socketId);
extern SocketId getSocketConnect();
extern void connectTarget(ATCI_DataType dataType);
extern int sendDataToGenericService(char* line);
extern int serviceReaderLoopWithResult(char* line);
extern  bool isCdmaLteDcSupport();

ATRESPONSE_t pas_csdf_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0;
    int mode = 0;
    int auxmode = 0;

    LOGATCI(LOG_DEBUG, "[pas_csdf_handler] cmdline %s", cmdline);

    switch(opType) {
        case AT_READ_OP:
            sprintf(response, "+CSDF: %d,%d\n",
                    gCsdfMode, gCsdfAuxmode);
            LOGATCI(LOG_DEBUG, "[pas_csdf_handler] response %s", response);

            return AT_OK;
            break;
        case AT_SET_OP:
            if (!at_tok_hasmore(&cmdline)) {
                return AT_ERROR;
            }

            err = at_tok_nextint(&cmdline, &mode);
            if (err < 0) {
                mode = 1;
            } else if (!(mode >= 1 && mode <= 7)) {
                mode = -1;
            }

            err = at_tok_nextint(&cmdline, &auxmode);
            if (err < 0) {
                auxmode = 1;
            } else if (!(auxmode >= 1 && auxmode <= 2)) {
                auxmode = -1;
            }

            if (mode != -1 && auxmode != -1) {
                gCsdfMode = mode;
                gCsdfAuxmode = auxmode;
                LOGATCI(LOG_DEBUG, "[pas_csdf_handler] After set: mode %d, auxmode %d",
                        gCsdfMode, gCsdfAuxmode);
            } else {
                return AT_ERROR;
            }

            return AT_OK;
            break;
        case AT_TEST_OP:
            sprintf(response, "+CSDF: (1,2,3,4,5,6,7),(1,2)\n");
            LOGATCI(LOG_DEBUG, "[pas_csdf_handler] response %s", response);

            return AT_OK;
            break;
        default:
            break;
    }

    return AT_ERROR;
}

ATRESPONSE_t pas_cclk_handler(char* cmdline, ATOP_t opType, char* response) {
    LOGATCI(LOG_DEBUG, "cmdline %s", cmdline);

    switch (opType) {
        case AT_READ_OP: {
            time_t timep;
            struct tm *p;
            struct timeval tv;
            struct timezone tz;
            int qTz = 0;

            time(&timep);
            //p=localtime(&timep);
            p = gmtime(&timep);
            gettimeofday(&tv, &tz);
            LOGATCI(LOG_DEBUG, "timezone: %d", tz.tz_minuteswest);
            if (gCclkTz == 0xFFFF) {
                qTz = (0 - tz.tz_minuteswest) / 60 * 4;
            } else {
                qTz = gCclkTz;
            }

            if (gCsdfAuxmode == 1) {
                if (qTz >= 0) {
                    sprintf(response, "+CCLK: \"%02d/%02d/%02d,%02d:%02d:%02d+%02d\"\n",
                            (1900+p->tm_year)%100, (1+p->tm_mon), p->tm_mday,
                            p->tm_hour, p->tm_min, p->tm_sec, qTz);
                } else {
                    sprintf(response, "+CCLK: \"%02d/%02d/%02d,%02d:%02d:%02d%03d\"\n",
                            (1900+p->tm_year)%100, (1+p->tm_mon), p->tm_mday,
                            p->tm_hour, p->tm_min, p->tm_sec, qTz);
                }
            } else if (gCsdfAuxmode == 2) {
                if (qTz >= 0) {
                    sprintf(response, "+CCLK: \"%04d/%02d/%02d,%02d:%02d:%02d+%02d\"\n",
                            1900+p->tm_year, (1+p->tm_mon), p->tm_mday,
                            p->tm_hour, p->tm_min, p->tm_sec, qTz);
                } else {
                    sprintf(response, "+CCLK: \"%04d/%02d/%02d,%02d:%02d:%02d%03d\"\n",
                            1900+p->tm_year, (1+p->tm_mon), p->tm_mday,
                            p->tm_hour, p->tm_min, p->tm_sec, qTz);
                }
            } else {
                return AT_ERROR;
            }
            return AT_OK;
            break;
        }
        case AT_SET_OP: {
            char * line;
            struct timeval tv = {0,0};
            time_t mktimeFromInput;
            unsigned int i = 0;
            struct tm setclk;
            char cclkSetCmd[40] = {0};
            bool hasInputTimeZone = false;
            int inputTimeZoneValue = 0;

            line = cclkSetCmd;
            for (i = 0; i < strlen(cmdline); i++) {
                if (cmdline[i] != ' ' && cmdline[i] != '\"' &&
                        cmdline[i] != '\''&& cmdline[i] != '\n') {
                    *(line++) = cmdline[i];
                }
            }
            *line = '\0';

            if (strlen(cclkSetCmd) == strlen("yy/MM/dd,hh:mm:ss+tz")) {
                hasInputTimeZone = true;
            } else if (strlen(cclkSetCmd) == strlen("yy/MM/dd,hh:mm:ss")) {
                hasInputTimeZone = false;
            } else {
                sprintf(response, "CME ERROR: Invalid parameters: [%d] cmdline:%s; [%d] cclkSetCmd:%s",
                        (int) strlen(cmdline), cmdline, (int) strlen(cclkSetCmd), cclkSetCmd);
                return AT_ERROR;
            }

            LOGATCI(LOG_DEBUG, "hasInputTimeZone %d", hasInputTimeZone);

            memset(&setclk, 0, sizeof(setclk));
            if (hasInputTimeZone) {
                sscanf(cclkSetCmd, "%2d/%2d/%2d,%2d:%2d:%2d%03d", &setclk.tm_year, &setclk.tm_mon,
                        &setclk.tm_mday, &setclk.tm_hour, &setclk.tm_min, &setclk.tm_sec, &inputTimeZoneValue);
                if (inputTimeZoneValue >= -96 && inputTimeZoneValue <= 96) {
                    gCclkTz = inputTimeZoneValue;
                } else {
                    return AT_ERROR;
                }
                LOGATCI(LOG_DEBUG, "Input time zone:%02d", inputTimeZoneValue);
            } else {
                sscanf(cclkSetCmd, "%2d/%2d/%2d,%2d:%2d:%2d", &setclk.tm_year, &setclk.tm_mon,
                        &setclk.tm_mday, &setclk.tm_hour, &setclk.tm_min, &setclk.tm_sec);
            }
            if (setclk.tm_year < 0 ||
                    setclk.tm_mon < 1 ||
                    setclk.tm_mon > 12 ||
                    setclk.tm_mday < 1 ||
                    setclk.tm_mday > 31 ||
                    setclk.tm_hour < 0 ||
                    setclk.tm_hour > 24 ||
                    setclk.tm_min < 0 ||
                    setclk.tm_min > 60 ||
                    setclk.tm_sec < 0 ||
                    setclk.tm_sec > 60) {
                LOGATCI(LOG_ERR, "Input format error");
                return AT_ERROR;
            }

            // convert tm format with input data
            setclk.tm_year += 2000; // make 2000 year
            setclk.tm_year -= 1900; // UTC -1900 year
            setclk.tm_mon  -= 1;    // from 0 to 11
            // Get UTC version tm
            char *envTz = getenv("TZ");
            char *mEnvTz = NULL;
            setenv("TZ", "UTC", 1);
            tzset();
            mktimeFromInput = mktime(&setclk);
            if (envTz) {
                mEnvTz = strdup(envTz);
                setenv("TZ", mEnvTz, 1);
                free(mEnvTz);
            } else {
                unsetenv("TZ");
            }
            tzset();
            if (mktimeFromInput < (time_t) 0) {
                LOGATCI(LOG_DEBUG, "mktime error: %d %s\n", errno, strerror(errno));
                return AT_ERROR;
            }
            /* time_t change to timeval */
            tv.tv_sec = mktimeFromInput;

            if (settimeofday(&tv, NULL) != 0) {
                LOGATCI(LOG_DEBUG, "Unable to set clock to %d.%d: %s\n",
                        (int)tv.tv_sec, (int)tv.tv_usec, strerror(errno));
                return AT_ERROR;
            }

            return AT_OK;
            break;
        }
        case AT_TEST_OP:
            snprintf(response, strlen("+CCLK: yy/MM/dd,hh:mm:ss[+/-tz]") + 1, "%s",
                    "+CCLK: yy/MM/dd,hh:mm:ss[+/-tz]");
            return AT_OK;
            break;
        default:

            break;
    }

    return AT_ERROR;
}

ATRESPONSE_t pas_echo_handler(char* cmdline, ATOP_t opType, char* response) {
    int len = 0, i = 0, flag = -1;

    LOGATCI(LOG_DEBUG, "cmdline %s", cmdline);

    len = strlen(cmdline);
    flag = serial.echo[serial.currDevice];;

    switch (opType) {
        case AT_BASIC_OP:
        case AT_SET_OP:
            for (i = 0; i < len; i ++) {
                if ( cmdline[i] == 'E' || cmdline[i] == 'e') {
                    if (i < len-1) {
                        flag = cmdline[i+1] - '0';
                        break;
                    }
                } else {
                    i++;
                }
            }

            if (flag == 0 || flag == 1) {
                setEchoOption(flag);
                return AT_OK;
            }
            break;
        case AT_READ_OP:
            sprintf(response, "%d\r\n", flag);
            return AT_OK;
            break;
        case AT_TEST_OP:
            sprintf(response, "0:disable echo, 1:enable echo\r\n");
            return AT_OK;
            break;
        default:
            break;
    }

    return AT_ERROR;
}

ATRESPONSE_t pas_modem_handler(char* cmdline, ATOP_t opType, char* response) {
    UNUSED(cmdline);
    UNUSED(opType);
    UNUSED(response);
    return AT_ERROR;
}

ATRESPONSE_t pas_eurc_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0, cmdID = 0;
    char enable_urc_prop[MTK_PROPERTY_VALUE_MAX];
    int urc_enabled = 0;
    LOGATCI(LOG_DEBUG, "pas_eurc_handler enter with opType: %d", opType);

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &cmdID);

            if (err < 0) return -1;

            if (1 == cmdID) {
                mtk_property_set(ENABLE_URC_PROP, "1");
            } else if (0 == cmdID) {
                mtk_property_set(ENABLE_URC_PROP, "0");
            }
            return AT_OK;
        case AT_TEST_OP:
            writeDataToSerial("+EURC:(0,1)", strlen("+EURC:(0,1)"));
            return AT_OK;
        case AT_READ_OP:
            mtk_property_get(ENABLE_URC_PROP, enable_urc_prop, "0");
            urc_enabled = atoi(enable_urc_prop);
            if (1 == urc_enabled) {
                snprintf(response, strlen("+EURC:1\r\n") + 1, "%s", "+EURC:1\r\n");
            } else if (0 == urc_enabled) {
                snprintf(response, strlen("+EURC:0\r\n") + 1, "%s", "+EURC:0\r\n");
            } else {
                snprintf(response, strlen("+EURC:invalid value.\r\n") + 1, "%s",
                        "+EURC:invalid value.\r\n");
            }
            return AT_OK;
        default:
            break;
    }
    return AT_ERROR;
}

ATRESPONSE_t pas_atci_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0, cmdID = 0;
    char atci_usermode[MTK_PROPERTY_VALUE_MAX];
    int usermode = 0;
    LOGATCI(LOG_DEBUG, "pas_atci_handler enter with opType: %d", opType);

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &cmdID);

            if (err < 0) return -1;

            if (1 == cmdID)
                mtk_property_set(ATCI_IN_USERMODE_PROP, "1");
            else if (0 == cmdID)
                mtk_property_set(ATCI_IN_USERMODE_PROP, "0");
            return AT_OK;
            break;
        case AT_TEST_OP:
            writeDataToSerial("+ATCI:(0,1)",strlen("+ATCI:(0,1)"));
            return AT_OK;
            break;
        case AT_READ_OP:
            mtk_property_get(ATCI_IN_USERMODE_PROP, atci_usermode, "0");
            usermode = atoi(atci_usermode);
            if(1 == usermode) {
                snprintf(response, strlen("+ATCI:1\r\n") + 1, "%s", "+ATCI:1\r\n");
            } else if (0 == usermode) {
                snprintf(response, strlen("+ATCI:0\r\n") + 1, "%s", "+ATCI:0\r\n");
            } else {
                snprintf(response, strlen("+ATCI:invalid value.\r\n") + 1, "%s",
                        "+ATCI:invalid value.\r\n");
            }
            return AT_OK;
            break;
        default:
            break;
    }
    return AT_ERROR;
}

ATRESPONSE_t pas_pct_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0, cmdID = 0;
    char pct_mode[MTK_PROPERTY_VALUE_MAX];
    int pctmode = 0;
    LOGATCI(LOG_DEBUG, "pas_pct_handler enter with opType: %d", opType);

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &cmdID);
            if (err < 0) return -1;

            if (1 == cmdID)
                mtk_property_set(ATCI_IN_PCT_PROP, "1");
            else if (0 == cmdID)
                mtk_property_set(ATCI_IN_PCT_PROP, "0");
            return AT_OK;
            break;
        case AT_TEST_OP:
            writeDataToSerial("+PCT:(0,1)",strlen("+PCT:(0,1)"));
            return AT_OK;
            break;
        case AT_READ_OP:
            mtk_property_get(ATCI_IN_PCT_PROP, pct_mode, "0");
            pctmode = atoi(pct_mode);
            if (1 == pctmode) {
                snprintf(response, strlen("+PCT:1\r\n") + 1, "%s", "+PCT:1\r\n");
            } else if (0 == pctmode) {
                snprintf(response, strlen("+PCT:0\r\n") + 1, "%s", "+PCT:0\r\n");
            } else {
                snprintf(response, strlen("+PCT:invalid value.\r\n") + 1, "%s",
                        "+PCT:invalid value.\r\n");
            }
            return AT_OK;
            break;
        default:
            break;
    }
    return AT_ERROR;
}

ATRESPONSE_t pas_esuo_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0, cmdID = 0;
    char simIDProperty[MTK_PROPERTY_VALUE_MAX] = {0};
    char multiSIMProperty[MTK_PROPERTY_VALUE_MAX] = {0};
    int simID = 0;

    mtk_property_get(MULTI_SIM, multiSIMProperty, "dsds");
    LOGATCI(LOG_DEBUG, "pas_esuo_handler enter with opType: %d, multiSIM: %s", opType, multiSIMProperty);

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &cmdID);
            if (err < 0) return -1;

            //start for c2k
            if (isCdmaLteDcSupport()) {
                    LOGATCI(LOG_DEBUG, "C2K mode");
                if (cmdID == 4) {
                    LOGATCI(LOG_DEBUG, "Set default to SIM1, setSocket to MD1");
                    mtk_property_set(ATCI_SIM, "0");
                } else if (cmdID == 9) {
                    LOGATCI(LOG_DEBUG, "C2K, setSocket to MD3");
                    mtk_property_set(ATCI_SIM, "9");
                }
                else if (cmdID == 5) {
                    LOGATCI(LOG_DEBUG, "Set default to SIM2, setSocket to MD1");
                    mtk_property_set(ATCI_SIM, "1");
                }
                else {
                    return AT_ERROR;
                }
                return AT_OK;
            } else {
                if (cmdID == 9) {
                    return AT_ERROR;
                }
            }
            //end for c2k
            if(cmdID == 4) { //Configure the default SIM to SIM1
                mtk_property_set(ATCI_SIM, "0");
                LOGATCI(LOG_DEBUG, "Set default to SIM1");
                return AT_OK;
            } else if(cmdID == 5) {
                //Configure the default SIM to SIM2
                if (strcmp(multiSIMProperty,"ss") != 0) {
                    mtk_property_set(ATCI_SIM, "1");
                    LOGATCI(LOG_DEBUG, "Set default to SIM2");
                    return AT_OK;
                } else {
                    LOGATCI(LOG_DEBUG, "Error single SIM.");
                }
            } else if(cmdID == 6) {
                //Configure the default SIM to SIM3
                if (strcmp(multiSIMProperty,"tsts") == 0) {
                    mtk_property_set(ATCI_SIM, "2");
                    LOGATCI(LOG_DEBUG, "Set default to SIM3");
                    return AT_OK;
                } else {
                    LOGATCI(LOG_DEBUG, "Error: Not triple SIM.");
                }
            } else {
                 LOGATCI(LOG_DEBUG, "Error: not supported SIM Id.");
            }
            break;
        case AT_TEST_OP:
            if (strcmp(multiSIMProperty,"ss") != 0) {
                if (strcmp(multiSIMProperty,"tsts") == 0) {
                    writeDataToSerial("+ESUO: (4-6)", strlen("+ESUO: (4-6)"));
                } else {
                    writeDataToSerial("+ESUO: (4-5)", strlen("+ESUO: (4-5)"));
                }
            } else {
                writeDataToSerial("+ESUO: (4)", strlen("+ESUO: (4)"));
            }
            return AT_OK;
            break;
        case AT_READ_OP:
            mtk_property_get(ATCI_SIM, simIDProperty, "0");
            simID = atoi(simIDProperty);
            if (simID == 1) {
                snprintf(response, strlen("+ESUO:5\r\n") + 1, "%s", "+ESUO:5\r\n");
            } else if (simID == 2) {
                snprintf(response, strlen("+ESUO:6\r\n") + 1, "%s", "+ESUO:6\r\n");
            } else if (simID == 3) {
                snprintf(response, strlen("+ESUO:7\r\n") + 1, "%s", "+ESUO:7\r\n");
            } else if (simID == 9) {
                snprintf(response, strlen("+ESUO:9\r\n") + 1, "%s", "+ESUO:9\r\n");
            } else if (simID == 0) {
                snprintf(response, strlen("+ESUO:4\r\n") + 1, "%s", "+ESUO:4\r\n");
            } else {
                snprintf(response, strlen("+ESUO:Value unsupport\r\n") + 1, "%s",
                        "+ESUO:Value unsupport\r\n");
            }
            return AT_OK;
            break;
        default:
            break;

    }
    return AT_ERROR;
}

ATRESPONSE_t pas_reboot_handler(char* cmdline, ATOP_t opType, char* response) {
    UNUSED(response);
    LOGATCI(LOG_DEBUG, "handle cmdline:%s", cmdline);

    if (opType != AT_ACTION_OP) {
        return AT_ERROR;
    }

    switch (opType) {
        case AT_ACTION_OP:
            system("shutdown -r -t 3");
            return AT_OK;
            break;
        default:
            break;
    }

    return AT_ERROR;
}

int sendATCommandToServiceWithResult(char* line) {
    if (s_fdService_command < 0) {
        connectTarget(GENERIC_TYPE);
    }
    sendDataToGenericService(line);
    return serviceReaderLoopWithResult(line);
}

ATRESPONSE_t pas_usbconfig_handler(char* cmdline, ATOP_t opType, char* response) {
    char config[MTK_PROPERTY_VALUE_MAX] = {0};
    int retval = -1;

    LOGATCI(LOG_DEBUG, "handle cmdline:%s", cmdline);

    if (opType != (AT_SET_OP) && (opType != AT_READ_OP)) {
        return AT_ERROR;
    }

    switch (opType) {
        /*
         * Runtime switch USB HIF through MTK-defined ATCMD
         *
         * AT@USBCONFIG=<config>
         * AT@USBCONFIG=rndis,acm              --> RNDIS + AT COM
         * AT@USBCONFIG=rndis,adb,acm          --> RNDIS + ADB + ELT COM
         * AT@USBCONFIG=rndis,gs3,dual_acm     --> RNDIS + AT COM + ELT COM
         * AT@USBCONFIG=rndis,adb,gs3,dual_acm --> RNDIS + ADB + AT COM + ELT COM
         */
        case AT_SET_OP: {
            LOGATCI(LOG_DEBUG, "opType = AT_SET_OP");

            // Eliminates any [ \'\"] in cmdline and changes to lower case
            unsigned int i = 0;
            char *line;
            line = config;
            for (i = 0; i < strlen(cmdline); i++) {
                if (i == MTK_PROPERTY_VALUE_MAX - 1) {
                    break;
                }

                if (cmdline[i] != ' ' && cmdline[i] != '\"' &&
                        cmdline[i] != '\'' && cmdline[i] != '\n') {
                    *(line++) = tolower(cmdline[i]);
                }
            }
            *line = '\0';
            LOGATCI(LOG_DEBUG, "reads cmdline as config: %s", config);

            if (strlen(config) <= 0) {
                LOGATCI(LOG_DEBUG, "skip setting %s property due to length=%d is abnormal",
                        PROPERTY_SYS_USB_STATE, (int) strlen(config));
                sprintf(response, "\r\n%d\r\nERROR\r\n\r\n", retval);
                return AT_ERROR;
            }

            retval = mtk_property_set(PROPERTY_VENDOR_USB_CONFIG, config);

            if (retval == 0) {
                LOGATCI(LOG_DEBUG, "set %s successful", PROPERTY_VENDOR_USB_CONFIG);
                sprintf(response, "\r\n@USBCONFIG=\"%s\"\r\n\r\nOK\r\n", config);
                return AT_OK;
            } else {
                LOGATCI(LOG_DEBUG, "set %s property failed, length=%d is abnormal",
                        PROPERTY_VENDOR_USB_CONFIG, retval);
                sprintf(response, "\r\n@USBCONFIG=\"%s\"\r\n\r\nERROR\r\n", config);
                return AT_ERROR;
            }

            break;
        }
        /*
         * Get USB HIF through MTK-defined ATCMD
         *
         * AT@USBCONFIG?
         */
        case AT_READ_OP: {
            LOGATCI(LOG_DEBUG, "opType = AT_READ_OP");
            retval = mtk_property_get(PROPERTY_SYS_USB_STATE, config, NULL);

            if (retval > 0) {
                LOGATCI(LOG_DEBUG, "get %s property successful, config=%s, length=%d are decent",
                        PROPERTY_SYS_USB_STATE, config, retval);
                sprintf(response, "\r\n@USBCONFIG: \"%s\"\r\n\r\nOK\r\n", config);
                return AT_OK;
            } else {
                LOGATCI(LOG_DEBUG, "get %s property fail, config=%s, length=%d is abnormal",
                        PROPERTY_SYS_USB_STATE, config, retval);
                sprintf(response, "\r\n@USBCONFIG: \"%s\"\r\n\r\nERROR\r\n", config);
                return AT_ERROR;
            }
        }
        default:
            LOGATCI(LOG_DEBUG, "unknown opType :%d", opType);
            break;
    }
    return AT_ERROR;
}

ATRESPONSE_t pas_vzwatcicfg_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0, value = 0;
    LOGATCI(LOG_DEBUG, "pas_vzwatcicfg_handler enter with opType: %d", opType);

    switch (opType) {
        case AT_SET_OP:
            err = at_tok_nextint(&cmdline, &value);
            if (err < 0) {
                return AT_ERROR;
            }
            if (VZWAPNE_BYPASS == gVzwapneConfig
                    || VZWAPNE_SYNC_TO_AP == gVzwapneConfig) {
                gVzwapneConfig = value;
                return AT_OK;
            }
            return AT_ERROR;
            break;
        case AT_TEST_OP:
            writeDataToSerial("+VZWATCICFG:(0,1)", strlen("+VZWATCICFG:(0,1)"));
            return AT_OK;
            break;
        case AT_READ_OP:
            if (VZWAPNE_BYPASS == gVzwapneConfig) {
                snprintf(response, strlen("+VZWATCICFG:1\r\n") + 1, "%s", "+VZWATCICFG:1\r\n");
            } else if (VZWAPNE_SYNC_TO_AP == gVzwapneConfig) {
                snprintf(response, strlen("+VZWATCICFG:0\r\n") + 1, "%s", "+VZWATCICFG:0\r\n");
            } else {
                snprintf(response, strlen("+VZWATCICFG:invalid value.\r\n") + 1, "%s",
                        "+VZWATCICFG:invalid value.\r\n");
            }
            return AT_OK;
            break;
        default:
            break;
    }
    return AT_ERROR;
}

/* For VzW. AT+CMEC (3GPP TS 27.007 Section 8.6) */
ATRESPONSE_t pas_cmec_handler(char* cmdline, ATOP_t opType, char* response) {
    int err = 0;
    int values[CMEC_CONFIG_COUNT] = {0};
    LOGATCI(LOG_DEBUG, "pas_cmec_handler enter with opType: %d", opType);

    switch (opType) {
        case AT_SET_OP:
            for (int i = 0; i < CMEC_CONFIG_COUNT; i++) {
                err = at_tok_nextint(&cmdline, &values[i]);
                if (err < 0 || values[i] < 0 || values[i] > 2) {
                    return AT_ERROR;
                }
            }
            for (int i = 0; i < CMEC_CONFIG_COUNT; i++) {
                gCmecConfig[i] = values[i];
            }
            return AT_OK;
        case AT_TEST_OP:
            writeDataToSerial("+CMEC:(0-2),(0-2),(0-2),(0-2)",
                    strlen("+CMEC:(0-2),(0-2),(0-2),(0-2)"));
            return AT_OK;
        case AT_READ_OP:
            snprintf(response, strlen("+CMEC:0,0,0,0\r\n") + 1, "+CMEC:%1d,%1d,%1d,%1d\r\n",
                    gCmecConfig[CMEC_INDEX_CKPD], gCmecConfig[CMEC_INDEX_CDIS],
                    gCmecConfig[CMEC_INDEX_CIND], gCmecConfig[CMEC_INDEX_CTSA]);
            return AT_OK;
        default:
            break;
    }
    return AT_ERROR;
}
