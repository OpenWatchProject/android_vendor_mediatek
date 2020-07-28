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
#include "ipc/keymaster_ipc.h"

unsigned char* keymaster_buffer = NULL;

int TEE_Alloc_KM_Buffer_And_Reset() {
    /* allocate for the first time */
    if (keymaster_buffer == NULL) {
        keymaster_buffer = (unsigned char*)malloc(KEYMASTER_MAX_BUFFER_LENGTH);

        if (keymaster_buffer == NULL) {
            IMSG_ERROR("malloc keymaster buffer failed");
            return -1;
        }
    }

    /* always reset */
    memset(keymaster_buffer, 0, KEYMASTER_MAX_BUFFER_LENGTH);
    return 0;
}

void TEE_Free_KM_Buffer(void) {
    if (keymaster_buffer != NULL) {
        free(keymaster_buffer);
        keymaster_buffer = NULL;
    }
}

