/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <cutils/properties.h>
#include <cutils/sockets.h>
#include <android/log.h>
#include "simlock11_ipc.h"

#define LOGD(format, ...)  \
    __android_log_print(ANDROID_LOG_DEBUG, "SIMLOCK11_IPC", (format), ##__VA_ARGS__ )

#define LOGW(format, ...)  \
    __android_log_print(ANDROID_LOG_WARN, "SIMLOCK11_IPC", (format), ##__VA_ARGS__ )

#define LOGE(format, ...)  \
    __android_log_print(ANDROID_LOG_ERROR, "SIMLOCK11_IPC", (format), ##__VA_ARGS__ )

#define LOGI(format, ...)  \
    __android_log_print(ANDROID_LOG_INFO, "SIMLOCK11_IPC", (format), ##__VA_ARGS__ )

#define RILD_SOCKET_NAME "rild-oem"

int toByte(char c) {
    if (c >= '0' && c <= '9') return (c - '0');
    if (c >= 'A' && c <= 'F') return (c - 'A' + 10);
    if (c >= 'a' && c <= 'f') return (c - 'a' + 10);

    LOGE("[RSU-SIMLOCK] toByte: Error: %c", c);
    return 0;
}

int checkByte(unsigned char* hexString) {
    unsigned int i = 0;

    for (i = 0; i < strlen((const char*)hexString); i++) {
        if (!((hexString[i] >= '0' && hexString[i] <= '9')
                ||(hexString[i] >= 'A' && hexString[i] <= 'F')
                ||(hexString[i] >= 'a' && hexString[i] <= 'f'))) {
            LOGE("[RSU-SIMLOCK] checkByte: Error: %c", hexString[i]);
            return -1;
        }
    }

    return 0;
}

int hexStringToByteArray(unsigned char* hexString, unsigned char* dest, unsigned int destLength) {
    unsigned int length = strlen((char*)hexString);
    unsigned char* buffer = malloc(length / 2);
    unsigned int i = 0;

    assert(buffer != NULL);
    LOGI("[RSU-SIMLOCK] hexStringToByteArray: hexString=%d, destLength=%d", length, destLength);

    if (dest == NULL || destLength < (length / 2)) {
        free(buffer);
        LOGE("[RSU-SIMLOCK] hexStringToByteArray: buffer size is too small");
        return MODEM_STATUS_BUFFER_TOO_SHORT;
    }

    for (i = 0; i < length; i += 2) {
        buffer[i / 2] = (unsigned char)((toByte(hexString[i]) << 4) | toByte(hexString[i+1]));
        // LOGI("hexStringToByteArray: buffer=%d, index=%d", buffer[i / 2], i/2);
    }

    memcpy(dest, buffer, (length / 2));
    free(buffer);

    return (length / 2);
}

int intToByteArray(uint32_t value, unsigned char* dest, unsigned int destLength) {
    unsigned char* buffer = malloc(destLength);
    int i = 0;

    assert(buffer != NULL);
    LOGI("[RSU-SIMLOCK] intToByteArray: value=%d, destLength=%d", value, destLength);

    if (dest == NULL) {
        free(buffer);
        LOGE("[RSU-SIMLOCK] intToByteArray: buffer size is too small");
        return MODEM_STATUS_BUFFER_TOO_SHORT;
    } else {
        for (i = 0; i < destLength; i++) {
            if (value == 0) {
                buffer[i] = 0;
            } else {
                buffer[i] = (unsigned char) (value & 0xff);
                value = value >> 8;
            }
        }
    }

    memcpy(dest, buffer, destLength);
    free(buffer);

    return destLength;
}

int parse_response(char* result) {
    int ret = ((result == NULL) ? MODEM_STATUS_COMMAND_FAILED : (int)strlen(result));
    char *error_string = NULL;

    if (NULL == result) {
        ret = MODEM_STATUS_COMMAND_FAILED;
    } else if (NULL != result && 0 == strcmp(result, "(null)")) {
        ret = MODEM_STATUS_OK;
    } else {
        error_string = strtok(result, ":");

        // Parse ERROR:X (prefix is ERROR and X represent the error code)
        if (NULL != error_string && 0 == strcmp(error_string, "ERROR")) {
            error_string = strtok(NULL, ":");

            // Parse error code
            int error_code = atoi(error_string);
            switch (error_code) {
                case 3000:
                    ret = MODEM_STATUS_VERIFICATION_FAILED;
                    break;
                case 3001:
                    ret = MODEM_STATUS_REBOOT_REQUEST;
                    break;
                default:
                    ret = MODEM_STATUS_COMMAND_FAILED;
            }
        }
    }

    LOGI("[RSU-SIMLOCK] [parse_response]: ERROR: %d", ret);
    return ret;
}

int connect_socket() {
    int fd = socket_local_client(RILD_SOCKET_NAME,
            ANDROID_SOCKET_NAMESPACE_RESERVED, SOCK_STREAM);
    if (fd < 0) {
        LOGE("[RSU-SIMLOCK] connect_socket: Fail to connect to socket rild-ocm. fd: %d", fd);
        return MODEM_STATUS_CONNECTION_FAILED;
    }

    return fd;
}

int send_data(int fd, int arg_count, uint32_t data_length, const void *data) {
    int ret = MODEM_STATUS_OK;
    // Use "rild-oem" communication protocol, you must send data as following rules:
    // First, send the argument count
    // Second, the data length of following data
    // Third, the data by char* format
    // That is, arg. counts(=n)->length of arg[0]->arg[0]->...->length of arg[n]->arg[n]

    // (send-1) send argCount
    LOGI("[RSU-SIMLOCK] send_data: (send-1). arg count: %d", arg_count);
    if (send(fd, (const void*)&arg_count, sizeof(int), 0) != sizeof(int)) {
        ret = MODEM_STATUS_CONNECTION_FAILED;
        goto done;
    }

    // (send-2) send data length
    LOGI("[RSU-SIMLOCK] send_data: (send-2). data length: %d", data_length);
    if (send(fd, (const void*)&data_length, sizeof(int), 0) != sizeof(int)) {
        ret = MODEM_STATUS_CONNECTION_FAILED;
        goto done;
    }

    // (send-3) send SIM Lock Blob data
    //     The operation format is "COMMAND,SIM_ID,[DATA]"
    // COMMAND:
    //     ATT_SIMLOCK_GETKEY             indicates to get key
    //     ATT_SIMLOCK_SET                indicates to set sim lock information
    //     ATT_SIMLOCK_GET_VERSION        indicates to get sim lock version
    //     ATT_SIMLOCK_RESET              indicates to reset sim lock information
    //     ATT_SIMLOCK_GET_STATUS         indicates to get sim status
    //     ATT_SIMLOCK_GET_CONFIG         indicates to get modem config
    //     ATT_SIMLOCK_GET_API_VER        indicates to get api version
    //     ATT_SIMLOCK_GET_SLOTS          indicates to get number of sim slots supported
    // SIM:
    //     0        sim slot 1
    //     1        sim slot 2
    // DATA:
    //     SIM Lock Blob     in case of the COMMAND is ATT_SIMLOCK_SET
    //                            DATA is SIM Lock Blob in hexadecimal format.
    //                            For example, "3A01FB69520B92104C6A" represents
    //                            a 10 byte data 3A 01 FB 69 52 0B 92 10 4C 6A.
    // sprintf(strData, "SIMLOCK_SET,1,%s", data);
    // sprintf(data, "SIMLOCK_SET,1,3A01FB69520B92104C6A");
    LOGI("[RSU-SIMLOCK] send_data: (send-3). data: %s", (char *)data);
    if (send(fd, (const void*)data, data_length, 0) != (int)data_length) {
        ret = MODEM_STATUS_CONNECTION_FAILED;
        goto done;
    }

done:
    LOGI("[RSU-SIMLOCK] send_data: ret:%d.", ret);
    return ret;
}

int recv_data(int fd, uint32_t data_length, void *data, uint32_t request_id) {
    int ret = MODEM_STATUS_OK;
    int recv_length = 0;
    int resp_length = 0;
    unsigned char* outBuffer = NULL;

    if (data == NULL) {
        LOGI("[RSU-SIMLOCK] recv_data: data is null or data length is wrong.");
        return MODEM_STATUS_BUFFER_TOO_SHORT;
    }

    // Use "rild-oem" communication protocol, you must receive data as following rules:
    // First, the response data length of following response
    // Second, the reponse data by char* format
    // (recv-1) receive response data length
    if (recv(fd, &recv_length, sizeof(recv_length), 0) != sizeof(recv_length)) {
        ret = MODEM_STATUS_CONNECTION_FAILED;
        goto done;
    }

    LOGI("[RSU-SIMLOCK] recv_data: (recv-1). recv length: %d, data length: %d", recv_length,
            data_length);
    outBuffer = malloc((sizeof(char) * recv_length) + 1);
    assert(outBuffer != NULL);
    memset(outBuffer, 0, (sizeof(char) * recv_length) + 1);
    // The response format is "RESULT,[DATA]"
    // RESULT:
    //     0                Success
    //     TBD             Error code
    // DATA:
    //     Shared Key      the response data when the previous COMMAND is SIMLOCK_GETKEY
    //                          Modem shared key in hexadecimal format.
    //                          For example, "3A01FB69520B92104C6A" represents
    //                          a 10 byte data 3A 01 FB 69 52 0B 92 10 4C 6A.
    // (recv-2) receive response data
    if (recv(fd, (void *)outBuffer, recv_length, 0) != recv_length) {
        ret = MODEM_STATUS_CONNECTION_FAILED;
        goto done;
    }

    LOGI("[RSU-SIMLOCK] recv_data: (recv-2). outBuffer: %s", (unsigned char *)outBuffer);

    // recv_length = 0;
    if (checkByte(outBuffer) >= 0 && strlen((const char*)outBuffer) > 0) {
        resp_length = hexStringToByteArray(outBuffer, data, data_length);

        // outBuffer = (unsigned char*)data;
        // LOGI("(recv-3). data: %02X%02X%02X...", outBuffer[0], outBuffer[1], outBuffer[2]);
        if (resp_length < 0) {
            ret = MODEM_STATUS_BUFFER_TOO_SHORT;

            if (request_id == MODEM_REQUEST_GET_MODEM_CONFIG) {
                intToByteArray(recv_length/2, data, data_length);
            }
            goto done;
        }
    } else {
        ret = parse_response((char *)outBuffer);
        if (ret < 0) {
            goto done;
        }
    }

done:
    if (outBuffer != NULL) {
        free(outBuffer);
    }

    LOGI("[RSU-SIMLOCK] recv_data: ret:%d.", ((ret == 0) ? resp_length : ret));
    return ((ret == 0) ? resp_length : ret);
}

int disconnect_socket(int fd) {
    if (fd < 0) {
        LOGE("[RSU-SIMLOCK] disconnect_socket: Invalid fd: %d", fd);
        return MODEM_STATUS_CONNECTION_FAILED;
    }

    return close(fd);
}


int32_t rsu_get_shared_key(uint8_t sim_id, uint8_t *key_buffer, uint32_t key_buffer_length) {
    int ret = MODEM_STATUS_OK;
    int fd = -1;
    int command_len = strlen("ATT_SIMLOCK_GETKEY") + COMMAND_SEPERATOR_LENGTH +
            COMMAND_SIM_ID_LENGTH;
    char *command = NULL;

    if (key_buffer == NULL) {
        ret = MODEM_STATUS_BUFFER_TOO_SHORT;
        goto done;
    }
    LOGD("[RSU-SIMLOCK] rsu_get_shared_key: %d, %s, %d", sim_id, key_buffer, key_buffer_length);

    fd = connect_socket();
    if (fd < 0) {
        ret = MODEM_STATUS_CONNECTION_FAILED;
        goto done;
    }

    command_len = command_len + 1;
    command = (char *)malloc(sizeof(char) * command_len);
    assert(command != NULL);
    memset(command, 0, sizeof(char) * command_len);
    sprintf(command, "ATT_SIMLOCK_GETKEY,%d", sim_id);

    ret = send_data(fd, 1, command_len, command);
    if (ret != MODEM_STATUS_OK) {
        goto done;
    }

    ret = recv_data(fd, key_buffer_length, key_buffer, MODEM_REQUEST_GET_SHARED_KEY);
    if (ret < MODEM_STATUS_OK) {
        goto done;
    }

    LOGI("[RSU-SIMLOCK] rsu_get_shared_key: [key_buffer]%02X%02X%02X", key_buffer[0],
            key_buffer[1], key_buffer[2]);

done:
    if (command != NULL) {
        free(command);
    }
    disconnect_socket(fd);

    return ret;
}

int32_t rsu_update_simlock_settings(uint8_t sim_id, const uint8_t *simlock_blob,
        uint32_t simlock_blob_length) {
    int ret = MODEM_STATUS_OK;
    char *result = NULL;
    unsigned int i = 0;
    int fd = -1;
    unsigned int command_len = strlen("ATT_SIMLOCK_SET") + (COMMAND_SEPERATOR_LENGTH*2) +
            COMMAND_SIM_ID_LENGTH;
    char *command = NULL;
    unsigned int slb_len = (simlock_blob_length * 2) + 1;
    char *slb = (char *)malloc(sizeof(char) * slb_len);
    assert(slb != NULL);

    if (simlock_blob == NULL) {
        ret = MODEM_STATUS_BUFFER_TOO_SHORT;
        goto done;
    }
    LOGD("[RSU-SIMLOCK] rsu_update_simlock_settings: %d, %d", sim_id, simlock_blob_length);

    memset(slb, 0, sizeof(char) * slb_len);
    for (i = 0; i < simlock_blob_length; i++) {
        sprintf((slb + (i*2)), "%02X", simlock_blob[i]);
    }
    LOGD("[RSU-SIMLOCK] rsu_update_simlock_settings: slb(char array) %s", slb);

    fd = connect_socket();
    if (fd < 0) {
        ret = MODEM_STATUS_CONNECTION_FAILED;
        goto done;
    }

    command_len = command_len + strlen((const char*)slb) + 1;
    command = (char *)malloc(sizeof(char) * command_len);
    assert(command != NULL);
    memset(command, 0, sizeof(char) * command_len);
    sprintf(command, "ATT_SIMLOCK_SET,%d,%s", sim_id, slb);

    ret = send_data(fd, 1, command_len, command);
    if (ret != MODEM_STATUS_OK) {
        goto done;
    }

    result = (char *)malloc(sizeof(char) * COMMAND_RESULT_CODE_LENGTH);
    assert(result != NULL);
    memset(result, 0, sizeof(char) * COMMAND_RESULT_CODE_LENGTH);

    ret = recv_data(fd, COMMAND_RESULT_CODE_LENGTH, result, MODEM_REQUEST_UPDATE_SIMLOCK_SETTINGS);
    if (ret < MODEM_STATUS_OK) {
        goto done;
    }

done:
    if (slb != NULL) {
        free(slb);
    }
    if (command != NULL) {
        free(command);
    }
    if (result != NULL) {
        free(result);
    }
    disconnect_socket(fd);

    return ret;
}

int32_t rsu_get_simlock_version(uint8_t sim_id, uint8_t *version_buffer,
        uint32_t version_buffer_length) {
    int ret = MODEM_STATUS_OK;
    int command_len = strlen("ATT_SIMLOCK_GET_VERSION") + COMMAND_SEPERATOR_LENGTH +
            COMMAND_SIM_ID_LENGTH;
    char *command = NULL;
    int fd = -1;

    fd = connect_socket();
    if (fd < 0) {
        ret = MODEM_STATUS_CONNECTION_FAILED;
        goto done;
    }

    command_len = command_len + 1;
    command = (char *)malloc(sizeof(char) * command_len);
    assert(command != NULL);
    memset(command, 0, sizeof(char) * command_len);
    sprintf(command, "ATT_SIMLOCK_GET_VERSION,%d", sim_id);

    ret = send_data(fd, 1, command_len, command);
    if (ret != MODEM_STATUS_OK) {
        goto done;
    }

    ret = recv_data(fd, version_buffer_length, version_buffer, MODEM_REQUEST_GET_SIMLOCK_VERSION);
    if (ret < MODEM_STATUS_OK) {
        goto done;
    }

done:
    if (command != NULL) {
        free(command);
    }
    disconnect_socket(fd);

    return ret;
}

int32_t rsu_reset_simlock_settings(uint8_t sim_id) {
    char *result = NULL;
    int ret = MODEM_STATUS_OK;
    int command_len = strlen("ATT_SIMLOCK_RESET") + COMMAND_SEPERATOR_LENGTH +
            COMMAND_SIM_ID_LENGTH;
    char *command = NULL;
    int fd = -1;

    fd = connect_socket();
    if (fd < 0) {
        ret = MODEM_STATUS_CONNECTION_FAILED;
        goto done;
    }

    command_len = command_len + 1;
    command = (char *)malloc(sizeof(char) * command_len);
    assert(command != NULL);
    memset(command, 0, sizeof(char) * command_len);
    sprintf(command, "ATT_SIMLOCK_RESET,%d", sim_id);

    ret = send_data(fd, 1, command_len, command);
    if (ret != MODEM_STATUS_OK) {
        goto done;
    }

    result = (char *)malloc(sizeof(char) * COMMAND_RESULT_CODE_LENGTH);
    assert(result != NULL);
    memset(result, 0, sizeof(char) * COMMAND_RESULT_CODE_LENGTH);

    ret = recv_data(fd, COMMAND_RESULT_CODE_LENGTH, result, MODEM_REQUEST_RESET_SIMLOCK_SETTINGS);
    if (ret < MODEM_STATUS_OK) {
        goto done;
    }

done:
    if (command != NULL) {
        free(command);
    }
    if (result != NULL) {
        free(result);
    }
    disconnect_socket(fd);

    return ret;
}

int32_t rsu_get_modem_status(uint8_t sim_id, uint8_t *status_buffer,
        uint32_t status_buffer_length) {
    int ret = MODEM_STATUS_OK;
    int fd = -1;
    int command_len = strlen("ATT_SIMLOCK_GET_STATUS") + COMMAND_SEPERATOR_LENGTH +
            COMMAND_SIM_ID_LENGTH;
    char *command = NULL;

    if (status_buffer == NULL) {
        ret = MODEM_STATUS_BUFFER_TOO_SHORT;
        goto done;
    }
    LOGD("[RSU-SIMLOCK] rsu_get_modem_status: %d, %s, %d", sim_id, status_buffer,
            status_buffer_length);

    fd = connect_socket();
    if (fd < 0) {
        ret = MODEM_STATUS_CONNECTION_FAILED;
        goto done;
    }

    command_len = command_len + 1;
    command = (char *)malloc(sizeof(char) * command_len);
    assert(command != NULL);
    memset(command, 0, sizeof(char) * command_len);
    sprintf(command, "ATT_SIMLOCK_GET_STATUS,%d", sim_id);

    ret = send_data(fd, 1, command_len, command);
    if (ret != MODEM_STATUS_OK) {
        goto done;
    }

    ret = recv_data(fd, status_buffer_length, status_buffer, MODEM_REQUEST_GET_MODEM_STATUS);
    if (ret < MODEM_STATUS_OK) {
        goto done;
    }

    LOGI("[RSU-SIMLOCK] rsu_get_modem_status: [status_buffer]%02X%02X%02X", status_buffer[0],
            status_buffer[1], status_buffer[2]);

done:
    if (command != NULL) {
        free(command);
    }
    disconnect_socket(fd);

    return ret;
}

int32_t rsu_get_modem_config(uint8_t sim_id, uint8_t *status_buffer,
        uint32_t status_buffer_length) {
    int ret = MODEM_STATUS_OK;
    int fd = -1;
    int command_len = strlen("ATT_SIMLOCK_GET_CONFIG") + COMMAND_SEPERATOR_LENGTH +
            COMMAND_SIM_ID_LENGTH;
    char *command = NULL;

    if (status_buffer == NULL) {
        ret = MODEM_STATUS_BUFFER_TOO_SHORT;
        goto done;
    }
    LOGD("[RSU-SIMLOCK] rsu_get_modem_config: %d, %s, %d", sim_id, status_buffer,
            status_buffer_length);

    fd = connect_socket();
    if (fd < 0) {
        ret = MODEM_STATUS_CONNECTION_FAILED;
        goto done;
    }

    command_len = command_len + 1;
    command = (char *)malloc(sizeof(char) * command_len);
    assert(command != NULL);
    memset(command, 0, sizeof(char) * command_len);
    sprintf(command, "ATT_SIMLOCK_GET_CONFIG,%d", sim_id);

    ret = send_data(fd, 1, command_len, command);
    if (ret != MODEM_STATUS_OK) {
        goto done;
    }

    ret = recv_data(fd, status_buffer_length, status_buffer, MODEM_REQUEST_GET_MODEM_CONFIG);
    if (ret < MODEM_STATUS_OK) {
        goto done;
    }

    LOGI("[RSU-SIMLOCK] rsu_get_modem_config: [status_buffer]%02X%02X%02X%02X", status_buffer[0],
            status_buffer[1], status_buffer[2], status_buffer[3]);

done:
    if (command != NULL) {
        free(command);
    }
    disconnect_socket(fd);

    return ret;
}

static int32_t rsu_get_api_version(uint8_t sim_id, uint8_t *status_buffer,
        uint32_t status_buffer_length) {
    int ret = MODEM_STATUS_OK;
    int fd = -1;
    int command_len = strlen("ATT_SIMLOCK_GET_API_VER") + COMMAND_SEPERATOR_LENGTH +
            COMMAND_SIM_ID_LENGTH;
    char *command = NULL;

    if (status_buffer == NULL) {
        ret = MODEM_STATUS_BUFFER_TOO_SHORT;
        goto done;
    }
    LOGD("[RSU-SIMLOCK] rsu_get_api_version: %d, %s, %d", sim_id, status_buffer,
            status_buffer_length);

    fd = connect_socket();
    if (fd < 0) {
        ret = MODEM_STATUS_CONNECTION_FAILED;
        goto done;
    }

    command_len = command_len + 1;
    command = (char *)malloc(sizeof(char) * command_len);
    assert(command != NULL);
    memset(command, 0, sizeof(char) * command_len);
    sprintf(command, "ATT_SIMLOCK_GET_API_VER,%d", sim_id);

    ret = send_data(fd, 1, command_len, command);
    if (ret != MODEM_STATUS_OK) {
        goto done;
    }

    ret = recv_data(fd, status_buffer_length, status_buffer, MODEM_REQUEST_GET_API_VERSION);
    if (ret < MODEM_STATUS_OK) {
        goto done;
    }

    LOGI("[RSU-SIMLOCK] rsu_get_api_version: [status_buffer]%02X%02X%02X%02X", status_buffer[0],
            status_buffer[1], status_buffer[2], status_buffer[3]);

done:
    if (command != NULL) {
        free(command);
    }
    disconnect_socket(fd);

    return ret;
}

static int32_t rsu_get_sim_slots(uint8_t sim_id, uint8_t *status_buffer,
        uint32_t status_buffer_length) {
    int ret = MODEM_STATUS_OK;
    int fd = -1;
    int command_len = strlen("ATT_SIMLOCK_GET_SLOTS") + COMMAND_SEPERATOR_LENGTH +
            COMMAND_SIM_ID_LENGTH;
    char *command = NULL;

    if (status_buffer == NULL) {
        ret = MODEM_STATUS_BUFFER_TOO_SHORT;
        goto done;
    }
    LOGD("[RSU-SIMLOCK] rsu_get_sim_slots: %d, %s, %d", sim_id, status_buffer,
            status_buffer_length);

    fd = connect_socket();
    if (fd < 0) {
        ret = MODEM_STATUS_CONNECTION_FAILED;
        goto done;
    }

    command_len = command_len + 1;
    command = (char *)malloc(sizeof(char) * command_len);
    assert(command != NULL);
    memset(command, 0, sizeof(char) * command_len);
    sprintf(command, "ATT_SIMLOCK_GET_SLOTS,%d", sim_id);

    ret = send_data(fd, 1, command_len, command);
    if (ret != MODEM_STATUS_OK) {
        goto done;
    }

    ret = recv_data(fd, status_buffer_length, status_buffer, MODEM_REQUEST_GET_SIM_SLOTS);
    if (ret < MODEM_STATUS_OK) {
        goto done;
    }

    LOGI("[RSU-SIMLOCK] rsu_get_sim_slots: [status_buffer]%02X%02X%02X%02X", status_buffer[0],
            status_buffer[1], status_buffer[2], status_buffer[3]);

done:
    if (command != NULL) {
        free(command);
    }
    disconnect_socket(fd);

    return ret;
}

int32_t ModemAPI_Send_request(
        uint32_t request_id,    // Message request ID
        uint8_t *buffer,        // Buffer used in input and output
        uint32_t buffer_length,
        uint32_t payload_length) {
    int ret = MODEM_STATUS_OK;

    switch (request_id) {
        case MODEM_REQUEST_GET_SHARED_KEY:
            LOGD("[RSU-SIMLOCK] MODEM_REQUEST_GET_SHARED_KEY: buffer_length:%d", buffer_length);
            ret = rsu_get_shared_key(0, buffer, buffer_length);
            break;
        case MODEM_REQUEST_UPDATE_SIMLOCK_SETTINGS:
            LOGD("[RSU-SIMLOCK] MODEM_REQUEST_UPDATE_SIMLOCK_SETTINGS: buffer_len:%d, \
                    payload_len:%d", buffer_length, payload_length);
            ret = rsu_update_simlock_settings(0, buffer, buffer_length);
            break;
        case MODEM_REQUEST_GET_SIMLOCK_VERSION:
            LOGD("[RSU-SIMLOCK] MODEM_REQUEST_GET_SIMLOCK_VERSION: buffer_len:%d",
                    buffer_length);
            ret = rsu_get_simlock_version(0, buffer, buffer_length);
            break;
        case MODEM_REQUEST_RESET_SIMLOCK_SETTINGS:
            LOGD("[RSU-SIMLOCK] MODEM_REQUEST_RESET_SIMLOCK_SETTINGS: buffer_len:%d",
                    buffer_length);
            ret = rsu_reset_simlock_settings(0);
            break;
        case MODEM_REQUEST_GET_MODEM_STATUS:
            LOGD("[RSU-SIMLOCK] MODEM_REQUEST_GET_MODEM_STATUS: buffer_len:%d", buffer_length);
            ret = rsu_get_modem_status(0, buffer, buffer_length);
            break;
        case MODEM_REQUEST_GET_MODEM_CONFIG:
            LOGD("[RSU-SIMLOCK] MODEM_REQUEST_GET_MODEM_CONFIG: buffer_len:%d, \
                    payload_len:%d", buffer_length, payload_length);
            ret = rsu_get_modem_config(0, buffer, buffer_length);
            break;
        case MODEM_REQUEST_GET_API_VERSION:
            LOGD("[RSU-SIMLOCK] MODEM_REQUEST_GET_API_VERSION: buffer_len:%d, \
                                payload_len:%d", buffer_length, payload_length);
            ret = rsu_get_api_version(0, buffer, buffer_length);
            break;
        /* Modem does not support this request yet.
        case MODEM_REQUEST_GET_SIM_SLOTS:
            LOGD("[RSU-SIMLOCK] MODEM_REQUEST_GET_SIM_SLOTS: buffer_len:%d, \
                                payload_len:%d", buffer_length, payload_length);
            ret = rsu_get_sim_slots(0, buffer, buffer_length);
            break;
        */
        default:
            LOGW("[RSU-SIMLOCK] MODEM_STATUS_UNSUPPORTED_COMMAND: request_ID:%d", request_id);
            return MODEM_STATUS_UNSUPPORTED_COMMAND;
    }
    return ret;
}

