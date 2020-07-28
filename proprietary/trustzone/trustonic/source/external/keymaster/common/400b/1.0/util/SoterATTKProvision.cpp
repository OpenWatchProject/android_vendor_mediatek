/*
 * Copyright (c) 2013-2015 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string.h>

#include "tee_keymaster_device.h"

#undef  LOG_ANDROID
#undef  LOG_TAG
#define LOG_TAG "SoterATTKProvision"

extern struct keystore_module HAL_MODULE_INFO_SYM;

void hexdump(const char *hdr, const void *addr, uint32_t len)
{
    bool morelines = len > 16;
    uint32_t blocklen = 16;
    uint32_t offset = 0;
    uint32_t i = 0;
    unsigned char *p = (unsigned char *) addr;

    if (hdr != NULL) {
        printf("%s\n", hdr);
    }

    if (len == 0) {
        return;
    }

    do {
        if (len < 16) {
            blocklen = len;
        }

        printf("%s%04x%s", "| ", offset, " | ");
        offset += 16;

        for (i = 0; i < blocklen; ++i) {
            printf("%02x ", p[i]);
        }

        if (blocklen < 16 && morelines) {
            for (i = blocklen; i < 16; i++) {
                printf("   ");
            }
        }

        printf("| ");

        for (i = 0; i < blocklen; i++) {
            uint8_t c[2];
            c[0] = p[i];
            c[1] = 0;
            //if (isprint(c)) {
            if (c[0] > 0x20 && c[0] < 0x7f) {
                printf("%s", c);
            } else {
                printf(".");
            }
        }

        if (!morelines) {
            printf("\n");
            return;
        }

        if ((blocklen < 16) && morelines) {
            for (i = blocklen; i < 16; i++) {
                printf(" ");
            }
        }

        printf(" |\n");
        len -= blocklen;
        p += blocklen;

    } while (len > 0);
}


int main(void)
{
    keymaster_error_t res = KM_ERROR_OK;
    TeeKeymasterDevice *device = new TeeKeymasterDevice(&HAL_MODULE_INFO_SYM.common);
    mtk_keymaster1_device_t *keymaster_device = (mtk_keymaster1_device_t *)device->keymaster_device();

    uint8_t* pub_key_data = NULL;
    size_t pub_key_data_length = 0;
    uint8_t* device_id = NULL;
    size_t device_id_length = 0;

    printf("Keymaster Module\n");
    printf("Name: %s\n", keymaster_device->common.module->name);
    printf("Author: %s\n", keymaster_device->common.module->author);
    printf("API version: %d\n" ,keymaster_device->common.module->module_api_version);
    printf("ATTK Generating ...\n");

    res = keymaster_device->generate_attk_key_pair(keymaster_device, 10);

    if (res != KM_ERROR_OK)
        printf("generate_attk_key_pair() FAIL (%d)\n", res);
    else
        printf("generate_attk_key_pair() PASS\n");

    res = keymaster_device->verify_attk_key_pair(keymaster_device);

    if (res != KM_ERROR_OK)
        printf("verify_attk_key_pair() FAIL (%d)\n", res);
    else
        printf("verify_attk_key_pair() PASS\n");

    res = keymaster_device->export_attk_public_key(keymaster_device,
            (const uint8_t**)&pub_key_data, &pub_key_data_length);

    if (res != KM_ERROR_OK)
        printf("export_attk_public_key() FAIL (%d)\n", res);
    else
        printf("export_attk_public_key() PASS\n");

    hexdump("attk_public_key", pub_key_data, pub_key_data_length);

    res = keymaster_device->get_device_id(keymaster_device,
            (const uint8_t**)&device_id, &device_id_length);

    if (res != KM_ERROR_OK)
        printf("get_device_id() FAIL (%d)\n", res);
    else
        printf("get_device_id() PASS\n");

    hexdump("device_id", device_id, device_id_length);

    if (pub_key_data != NULL)
        free(pub_key_data);

    if (device_id != NULL)
        free(device_id);

    return res;
}
