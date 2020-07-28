/*
 * Copyright (C) 2015-2017 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */

#define IMSG_TAG "microkeymaster"

#include <errno.h>
#include <imsg_log.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>

#include "ut_km_def.h"
#include "ut_km_ioctl.h"
#include "ut_km_api.h"

#define DEV_NAME "/dev/ut_keymaster"

#define BUFFER_SIZE (128 * 1024)

static int keymaster_fd = -1;
unsigned char* keymaster_buffer = NULL;

int TEE_Alloc_KM_Buffer_And_Reset() {
    /* allocate for the first time */
    if (keymaster_buffer == NULL) {
        keymaster_buffer = malloc(BUFFER_SIZE);

        if (keymaster_buffer == NULL) {
            IMSG_ERROR("malloc keymaster buffer failed");
            return -1;
        }
    }

    /* always reset */
    memset(keymaster_buffer, 0, BUFFER_SIZE);
    return 0;
}

void TEE_Free_KM_Buffer(void) {
    if (keymaster_buffer != NULL) {
        free(keymaster_buffer);
        keymaster_buffer = NULL;
    }
}

int TEE_OPEN(void) {
    keymaster_fd = open(DEV_NAME, O_RDWR);

    if (keymaster_fd == -1) {
        IMSG_ERROR("utkeymaster dev open error");
        IMSG_ERROR("errno=%d\n", errno);
        return -1;
    }

    TEE_Alloc_KM_Buffer_And_Reset();
    return 0;
}

void TEE_CLOSE(void) {
    close(keymaster_fd);
    TEE_Free_KM_Buffer();
}

int TEE_InvokeCommand(void) {
    int ioresult = ioctl(keymaster_fd, CMD_KM_MEM_SEND, keymaster_buffer);

    if (ioresult != 0) {
        IMSG_ERROR("utkeymaster invokecommand error,%d", ioresult);
        return ioresult;
    }

    return 0;
}
