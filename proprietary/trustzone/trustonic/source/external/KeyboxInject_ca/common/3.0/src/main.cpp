/*
 * Copyright (c) 2013-2016 TRUSTONIC LIMITED
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

#include <stdlib.h>
#include <string.h>

#include "KeyboxInject.h"

#include "log.h"
#include "mcSo.h"

#if !RPMB_SUPPORT
#define ATTEST_KEYBOX_FILE "/mnt/vendor/persist/attest_keybox.so"
#endif

/**
 * Enhanced exit function
 *
 * @param[in] exitCode reason to exit
 *
 * @return MC_DRV_OK or error.
 */
static void returnExitCode(int exitCode);

/**
 * Extract attestation data from XML file and save it as Secure Object.
 *
 * @param[in] filename path to store the Secure Object.
 *
 * @return MC_DRV_OK or error.
 */
static int storeAttestationData(const char *in_filename);

/**
 * Check attestation data from XML file.
 *
 * @param[in] filename path to check the Secure Object.
 *
 * @return MC_DRV_OK or error.
 */
static int checkAttestationData(const char *in_filename);

/**
 * allocate/fill a buffer with file content
 *
 * @param[in] pPath path/filename to read
 * @param[out] ppContent pointer to buffer which contain extracted data.
 *
 * @return size of extracted data.
 */
static long getFileContent(const char* pPath, uint8_t** ppContent);

#if !RPMB_SUPPORT
/**
 * Save content to file
 *
 * @param[in] pPath path/filename to write in.
 * @param[in] ppContent pointer to data to write.
 * @param[in] size size of data to write.
 *
 * @return 0 if OK, 1 if KO.
 *
 */
static int saveFile(const char *pPath, uint8_t* ppContent, size_t size);
#endif

/**
 * Implementation of the set attestation data from Google.
 */
int main(int argc, char *argv[])
{
    int ret = 0;
    LOG_I("Keybox Injection/Verification\n");

    if (argc != 3)
        returnExitCode(42);

    if (argv[1][0] == '-' && argv[1][1] == 'i')
        ret = storeAttestationData(argv[2]);
    else if (argv[1][0] == '-' && argv[1][1] == 'c')
        ret = checkAttestationData(argv[2]);
    else
        returnExitCode(42);

    returnExitCode(ret);
    return ret;
}

static int storeAttestationData(const char *in_filename)
{
   int ret = 0;
    uint8_t *data_p = NULL;
    uint32_t data_len = 0;
#if !RPMB_SUPPORT
    uint8_t *blob_p = NULL;
    uint32_t blob_len = 0;
#endif

    data_len = getFileContent(in_filename, &data_p);
    if (data_len == 0) {
        LOG_E("getFileContent failed!\n");
        fprintf(stderr, "Could not open attestation key file.\n");
        return -1;
    }

#if !RPMB_SUPPORT
    /* first param = 0 : no clear text part in the secure object */
    blob_len = MC_SO_SIZE(0, data_len);
    blob_p = (uint8_t *)malloc(blob_len);
    if (blob_p == NULL) {
        LOG_E("Allocate blob buffer of size %u failed!\n", blob_len);
        fprintf(stderr, "Allocate blob buffer of size %u failed!\n", blob_len);
        free(data_p);
        return -2;
    }

    memset(blob_p, 0, blob_len);
#endif

    ret = tlcOpen(MC_SPID_SYSTEM);
    if (ret != MC_DRV_OK) {
        LOG_E("open TL session failed (0x%X)!\n", ret);
        fprintf(stderr, "Could not open Trusted Application session (0x%X).\n", ret);
        goto exit;
    }

#if RPMB_SUPPORT
    ret = tlcSetAttestationData(data_p, data_len);
#else
    ret = tlcSetAttestationData(data_p, data_len, blob_p, &blob_len);
#endif
    tlcClose();
    if (ret != MC_DRV_OK) {
        LOG_E("Set Attestation Data failed: 0x%X (%d)!\n", ret, ret);
        fprintf(stderr, "Set Attestation Data failed: 0x%X (%d)!\n", ret, ret);
        goto exit;
    }

#if !RPMB_SUPPORT
    /* store the secure object */
    ret = saveFile(ATTEST_KEYBOX_FILE, blob_p, (size_t)blob_len);
    if (ret != 0) {
        LOG_E("Writing Secure object failed (%d)!\n", ret);
        fprintf(stderr, "Writing Secure object failed (%d)!\n", ret);
        goto exit;
    }
#endif

exit:
    free(data_p);
#if !RPMB_SUPPORT
    free(blob_p);
#endif

    return ret;
}

static int checkAttestationData(const char *in_filename)
{
    int ret = 0;
    uint8_t *data_p = NULL;
    uint32_t data_len = 0;
#if !RPMB_SUPPORT
    uint8_t *blob_p = NULL;
    uint32_t blob_len = 0;
#endif

    data_len = getFileContent(in_filename, &data_p);
    if (data_len == 0) {
        LOG_E("Open attestation key file failed!\n");
        fprintf(stderr, "Open attestation key file failed!\n");
        return -1;
    }

#if !RPMB_SUPPORT
    blob_len = getFileContent(ATTEST_KEYBOX_FILE, &blob_p);
    if (blob_len == 0) {
        LOG_E("Retrieve attestation key failed!\n");
        fprintf(stderr, "Retrieve attestation key failed!\n");
        free(data_p);
        return -2;
    }
#endif

    ret = tlcOpen(MC_SPID_SYSTEM);
    if (ret != MC_DRV_OK) {
        LOG_E("open TL session failed (0x%X)!\n", ret);
        fprintf(stderr, "Could not open Trusted Application session (0x%X).\n", ret);
        goto exit;
    }

#if RPMB_SUPPORT
    ret = tlcCheckAttestationData(data_p, data_len);
#else
    ret = tlcCheckAttestationData(data_p, data_len, blob_p, blob_len);
#endif
    tlcClose();
    if (ret != MC_DRV_OK) {
        LOG_E("Verify Attestation Data failed: 0x%X (%d)!\n", ret, ret);
        fprintf(stderr, "Verify Attestation Data failed: 0x%X (%d)!\n", ret, ret);
        goto exit;
    }

exit:
    free(data_p);
#if !RPMB_SUPPORT
    free(blob_p);
#endif

    return ret;
}

static long getFileContent(const char* pPath, uint8_t** ppContent)
{
    FILE*   pStream;
    long    filesize;
    uint8_t* content = NULL;

    /* Open the file */
    pStream = fopen(pPath, "rb");
    if (pStream == NULL)
    {
        fprintf(stderr, "Error: Cannot open file: %s.\n", pPath);
        goto error;
    }

    if (fseek(pStream, 0L, SEEK_END) != 0)
    {
        fprintf(stderr, "Error: Cannot read file: %s.\n", pPath);
        goto error;
    }

    filesize = ftell(pStream);
    if (filesize < 0)
    {
        fprintf(stderr, "Error: Cannot get the file size: %s.\n", pPath);
        goto error;
    }

    if (filesize == 0)
    {
        fprintf(stderr, "Error: Empty file: %s.\n", pPath);
        goto error;
    }

    /* Set the file pointer at the beginning of the file */
    if (fseek(pStream, 0L, SEEK_SET) != 0)
    {
        fprintf(stderr, "Error: Cannot read file: %s.\n", pPath);
        goto error;
    }

    /* Allocate a buffer for the content */
    content = (uint8_t*)malloc(filesize);
    if (content == NULL)
    {
        fprintf(stderr, "Error: Cannot read file: Out of memory.\n");
        goto error;
    }

    /* Read data from the file into the buffer */
    if (fread(content, (size_t)filesize, 1, pStream) != 1)
    {
        fprintf(stderr, "Error: Cannot read file: %s.\n", pPath);
        goto error;
    }

    /* Close the file */
    fclose(pStream);
    *ppContent = content;

    /* Return number of bytes read */
    return filesize;

error:
    if (content != NULL)
        free(content);

    if (pStream != NULL)
        fclose(pStream);

    return 0;
}

#if !RPMB_SUPPORT
static int saveFile(const char *pPath, uint8_t* ppContent, size_t size)
{
    size_t res = 0;
    FILE *f = fopen(pPath, "wb");
    if (f == NULL) {
        printf("Error opening file: %s\n", pPath);
        return -3;
    }

    res = fwrite(ppContent, sizeof(uint8_t), size, f);
    fclose(f);
    if (res != size) {
        fprintf(stderr, "Saving keybox failed: %zu != %zu\n", res, size);
        return -4;
    }

    return 0;
}
#endif

static void returnExitCode(int exitCode)
{
    if (0 != exitCode) {
        LOG_E("Failure");
    } else {
        LOG_I("Success");
    }
    if (exitCode == 42) {
        fprintf(stderr, "Usage: <command> <in_option> <in_filename>\n");
        fprintf(stderr, "Install Example: kmsetkey_ca.trustonic -i kb.bin\n");
        fprintf(stderr, "Check Example: kmsetkey_ca.trustonic -c kb.bin\n");
    }
    fprintf(stderr, "TLC exit code: 0x%08X\n", exitCode);
    exit(exitCode);
}
