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

#include "atcid_serial.h"
#include "atcid_cmd_dispatch.h"
#include "atcid_cust_cmd_process.h"
#include "atcid_audio_cmd.h"
#include <string.h>
#include "atci_battery_cmd_dispatch.h"
#include "stdio.h"
#include "property/mtk_properties.h"

#define VZWAPNE_SYNC_TO_AP 0
#define VZWAPNE_BYPASS 1

extern int gVzwapneConfig;

extern int gCmecConfig[];

At_Ril_Mmi_t atRilMmi_info = {{{0},}, 0, 0, 0, {0}, 0, 0, 0, {0}, 0};

#define ISMMIRSPCMD(OP) (OP&AT_OP_MMI_GROUP)

int generic_command_hdlr(char* line) {
    int i = 0;
    int table_size = 0;
    char* line_cut = cut_cmd_line(line);
    char pct_mode[MTK_PROPERTY_VALUE_MAX] = {0};
    table_size = (sizeof(generic_cmd_table)/sizeof(generic_cmd_type));

    if (line_cut == NULL) {
        return 0;
    }

    mtk_property_get(ATCI_IN_PCT_PROP, pct_mode, "1");
    LOGATCI(LOG_DEBUG, "pct_mode is %s", pct_mode);

    if (strcmp(pct_mode, "1") == 0 && (strcmp(line_cut, "AT+CPMS") == 0 ||
                                       strcmp(line_cut, "AT+CMSS") == 0 ||
                                       strcmp(line_cut, "AT+CMGL") == 0 ||
                                       strcmp(line_cut, "AT+CMGR") == 0 ||
                                       strcmp(line_cut, "AT+CMGF") == 0)) {

        LOGATCI(LOG_DEBUG, "PCT mode, sms command send to modem");
        free(line_cut);
        return 0;
    }

    if (strlen(line_cut) > strlen(AT_STRING) && isalpha(line_cut[strlen(AT_STRING)])) {
        // Basic command
        line_cut[strlen(AT_STRING) + 1] = '\0';
    }

    LOGATCI(LOG_DEBUG, "The size of generic_cmd_table is %d", table_size);
    for (i = 0; i < table_size; i++) {
        if (strcmp(line_cut, generic_cmd_table[i].cmdName) == 0) {
            if ((strcmp(line_cut, "AT+VZWAPNE") == 0 || strcmp(line_cut, "AT+VZWAPNETMR") == 0)
                    && (getAtOp(&line) != AT_SET_OP || gVzwapneConfig == VZWAPNE_BYPASS)) {
                // Bypass command to MD
                break;
            }
            //Write the AT command to generic service by socket interface
            free(line_cut);
            return 1;
        }
    }
    free(line_cut);
    return 0;
}

int ril_command_hdlr(char* line) {
    int i = 0;
    int table_size = 0;
    char* line_cut = cut_cmd_line(line);

    if (line_cut == NULL) {
        return 0;
    }
    table_size = (sizeof(ril_mmi_cmd_table)/sizeof(rilmmicmd_type));
    for (i = 0; i< table_size; i++) {
        LOGATCI(LOG_DEBUG, "ril_mmi_cmd_table[%d].cmdName = %s", i, ril_mmi_cmd_table[i].cmdName);
        if (strcmp(line_cut, ril_mmi_cmd_table[i].cmdName) == 0) {
            ATOP_t at_op = getAtOp(&line);
            LOGATCI(LOG_DEBUG, "The cus mmi command op is 0x%x; Support op is 0x%x", at_op,
                ril_mmi_cmd_table[i].opType);
            if((at_op & ril_mmi_cmd_table[i].opType)) {
                if(ISMMIRSPCMD((at_op << 4) & ril_mmi_cmd_table[i].opType)) {
                    if (atRilMmi_info.cmdNameEIdx <= MAX_QUEUE_MMI_CMD_IDX) {
                        LOGATCI(LOG_DEBUG, "Match ril mmi command.EIdx[%d]",
                            atRilMmi_info.cmdNameEIdx);
                        atRilMmi_info.atOp = at_op << 4;
                        atRilMmi_info.hasRilMMICmd = 1;
                        memset(atRilMmi_info.cmdName[atRilMmi_info.cmdNameEIdx], 0,
                                MAX_AT_RIL_COMMAND_LEN);
                        if (sizeof(*line_cut) <= MAX_AT_RIL_COMMAND_LEN) {
                            strncpy(atRilMmi_info.cmdName[atRilMmi_info.cmdNameEIdx], line_cut,
                                    strlen(line_cut));
                        }
                        if (AT_READ_OP == at_op) {
                            strncat(atRilMmi_info.cmdName[atRilMmi_info.cmdNameEIdx], "?", 1);
                        } else if (AT_TEST_OP == at_op) {
                            strncat(atRilMmi_info.cmdName[atRilMmi_info.cmdNameEIdx], "=?", 2);
                        } else if (AT_SET_OP == at_op) {
                            strncat(atRilMmi_info.cmdName[atRilMmi_info.cmdNameEIdx], "=", 1);
                        }

                        strncat(atRilMmi_info.cmdName[atRilMmi_info.cmdNameEIdx], MTKATCIRILPATTERN,
                                strlen(MTKATCIRILPATTERN));
                        atRilMmi_info.cmdNameEIdx++;
                        if(MAX_QUEUE_MMI_CMD_IDX <= atRilMmi_info.cmdNameEIdx)
                            atRilMmi_info.cmdNameEIdx = 0;
                    }
                }
            } else {
                LOGATCI(LOG_DEBUG, "This is a normal RIL AT command.");
            }
            free(line_cut);
            return 1; //The command is treated as RIL_TYPE
        }
    }

    free(line_cut);
    return 0;
}

int custom_command_hdlr(char* line) {
    int i = 0, ret = 0;
    int table_size = 0;
    char response[MAX_AT_RESPONSE];
    char* line_cut = cut_cmd_line(line);
    char operator[MTK_PROPERTY_VALUE_MAX] = {0};

    if (line_cut == NULL) {
        return 0;
    }

    mtk_property_get(ATCI_IN_OPERATOR, operator, "OM");
    if (strcmp(operator, "OP12") != 0 && strcmp(line_cut, "AT+CMEC") == 0) {
        free(line_cut);
        return 0;
    }
    if (strcmp(operator, "OP12") == 0) {
        if ((gCmecConfig[CMEC_INDEX_CKPD] == 0 && strcmp(line_cut, "AT+CKPD") == 0)
                || (gCmecConfig[CMEC_INDEX_CTSA] == 0 && strcmp(line_cut, "AT+CTSA") == 0)) {
            if (getAtOp(&line) == AT_SET_OP) {
                LOGATCI(LOG_DEBUG, "Disabled by +CMEC");
                writeDataToserialByResponseType(AT_ERROR);
                free(line_cut);
                return 1;
            }
        }
    }

    memset(response, 0, sizeof(response));
    table_size = (sizeof(custom_cmd_table)/sizeof(customcmd_type));

    for (i = 0; i < table_size; i++) {
        if (strcmp(line_cut, custom_cmd_table[i].cmdName) == 0) {
            ATOP_t at_op = getAtOp(&line);
            LOGATCI(LOG_DEBUG, "The command op is %d; Support op is %d", at_op,
                custom_cmd_table[i].opType);
            if ((at_op & custom_cmd_table[i].opType) == 0) {
                writeDataToserialByResponseType(AT_NOT_IMPL);
            } else {
                ret = custom_cmd_table[i].cmd_handle_func(line, at_op, response);
                if (strlen(response) > 0) {
                    writeDataToSerial(response, strlen(response));
                }

                if (ret != AT_NONE_OP) {
                    writeDataToserialByResponseType(ret);
                }
            }
            free(line_cut);
            return 1; //The command is handled here
        }
    }
    free(line_cut);
    return 0;
}

int audio_command_hdlr(char* line) {
    int table_size = (sizeof(audio_cmd_table) / sizeof(audio_cmd_type));;
    int i = 0;
    for (i = 0; i < table_size; i++) {
        if (strncmp(line, audio_cmd_table[i].cmdName, strlen(audio_cmd_table[i].cmdName)) == 0) {
            return audio_cmd_table[i].dest;
        }
    }
    return 0;
}

int handleBatCmd(char* line, int cmdId) {
    int result = -1;
    char response[MAX_AT_RESPONSE];
    LOGATCI(LOG_DEBUG, "line: %s", line);
    switch(cmdId) {
        case 0:
            result = AT_get_charging_state_flag();
            break;
        case 1:
            result = AT_get_bat_voltage();
            break;
        case 2:
            result = AT_set_Charger_Current(CUT_CHARGER_CURRENT);
            break;
        default:
            LOGATCI(LOG_DEBUG, "atcid not support this at command: %s", line);
    }
    if (result != -1) {
        sprintf(response, "%d\r\nOK\r\n", result);
        writeDataToSerial(response, strlen(response));
    } else {
        sprintf(response, "ERROR\r\n");
        writeDataToSerial(response, strlen(response));
    }
    return result;
}

int bat_command_hdlr(char* line) {
    int i = 0;
    int table_size = 0;
    char* line_cut = cut_cmd_line(line);
    if (line_cut == NULL) {
        return 0;
    }
    table_size = (sizeof(bat_cmd_table)/sizeof(generic_cmd_type));

    for (i = 0; i < table_size; i++) {
        if (strcmp(line_cut, bat_cmd_table[i].cmdName) == 0) {
            handleBatCmd(line, i);
            free(line_cut);
            return 1; //The command is handled here
        }
    }
    free(line_cut);
    return 0;
}

int process_cmd_line(char* line) {
    LOGATCI(LOG_DEBUG,"Enter");

    //Check the command is belonged to customized command table
    LOGATCI(LOG_DEBUG, "handle in custom_command_hdlr");

    if (ril_command_hdlr(line)) {
        return RIL_TYPE;
    }

    if (custom_command_hdlr(line)) {
        return ATCI_TYPE;
    }

    LOGATCI(LOG_DEBUG, "handle in generic_command_hdlr");
    if (generic_command_hdlr(line)) {
        return GENERIC_TYPE;
    }

    LOGATCI(LOG_DEBUG, "handle in audio_command_hdlr");
    int audio_result = audio_command_hdlr(line);
    if (audio_result == Modem_Internal) {
        return AUDIO_TYPE;
    } else if (audio_result == Modem_External) {
        return AUDIO_EXTERNAL_TYPE;
    }

    LOGATCI(LOG_DEBUG, "handle in bat_command_hdlr");
    if (bat_command_hdlr(line)) {
        return BATTERY_TYPE;
    }

    return RIL_TYPE;
}

char* cut_cmd_line(char* line) {
    char* result;
    int i = 0;
    int size = strlen(line);
    int len = size;
    for (i = 0; i < size; i++) {
        if (line[i] == '=' || line[i] == '?') {
            len = i;
            break;
        }
    }
    result = (char*)malloc((len + 1) * sizeof(char));
    if (result == NULL) {
        LOGATCI(LOG_DEBUG, "malloc result fail, return NULL");
        return NULL;
    }
    memcpy(result, line, len);
    result[len] = '\0';
    return result;
}
