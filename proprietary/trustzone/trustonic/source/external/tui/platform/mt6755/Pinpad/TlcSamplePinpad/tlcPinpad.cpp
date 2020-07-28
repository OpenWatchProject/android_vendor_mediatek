/*
 * Copyright (c) 2013-2014 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

/**
 * Connector library for accessing TUI pinpad Trusted Application.
 */

#include <stdlib.h>
#include <errno.h>

#include "MobiCoreDriverApi.h"
#include "tlPinpadApi.h"
#include "tlcPinpad.h"

#define LOG_TAG "TLC_PIN"
#include "log.h"

/** Static variables */
/** Array of pinpad secure objects path */
static char* listOfSOFile[IDX_PINPAD_END];
/** Temporary buffer used to manipulate paths */
static char pathOfSO[100];
/** Secure objects file name */
static const char* securePinpadSOFilenames[] = {"pinpadSO.padImage",
                                        "pinpadSO.star",
                                        "pinpadSO.blank",
                                        "pinpadSO.but0",
                                        "pinpadSO.but1",
                                        "pinpadSO.but2",
                                        "pinpadSO.but3",
                                        "pinpadSO.but4",
                                        "pinpadSO.but5",
                                        "pinpadSO.but6",
                                        "pinpadSO.but7",
                                        "pinpadSO.but8",
                                        "pinpadSO.but9",
                                        "pinpadSO.correct",
                                        "pinpadSO.cancel",
                                        "pinpadSO.valid",
                                        "pinpadSO.but0_pressed",
                                        "pinpadSO.but1_pressed",
                                        "pinpadSO.but2_pressed",
                                        "pinpadSO.but3_pressed",
                                        "pinpadSO.but4_pressed",
                                        "pinpadSO.but5_pressed",
                                        "pinpadSO.but6_pressed",
                                        "pinpadSO.but7_pressed",
                                        "pinpadSO.but8_pressed",
                                        "pinpadSO.but9_pressed",
                                        "pinpadSO.correct_pressed",
                                        "pinpadSO.cancel_pressed",
                                        "pinpadSO.valid_pressed",
                                        "pinpadSO.keylayout"};
/** Global variables */
tciMessage_t *tci;
mcSessionHandle_t tlSessionHandle;

static size_t getFileContent(
    const char* pPath,
    uint8_t** ppContent)
{
    FILE*   pStream;
    long    filesize;
    uint8_t* content = NULL;

   /*
    * The stat function is not used (not available in WinCE).
    */

   /* Open the file */
   pStream = fopen(pPath, "rb");
   if (pStream == NULL)
   {
      fprintf(stderr, "Error: Cannot open file: %s.\n", pPath);
      return 0;
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
   return (size_t)filesize;

error:
   if (content  != NULL)
   {
       free(content);
   }
   fclose(pStream);
   return 0;
}

// -------------------------------------------------------------
bool securePinpadConnect(void) {
    mcResult_t mcRet;
    uint32_t nTrustedAppLength;
    uint8_t* pTrustedAppData;

    LOG_I("Opening <t-base device");
    mcRet = mcOpenDevice(MC_DEVICE_ID_DEFAULT);
    if (mcRet != MC_DRV_OK) {
        LOG_E("Error opening device: %d", mcRet);
        return false;
    }

    LOG_I("Allocating buffer for TCI");
    tci = (tciMessage_t *)malloc(sizeof(tciMessage_t));
    if (tci == NULL) {
        LOG_ERRNO("Allocation of TCI failed");
        return false;
    }

    nTrustedAppLength = getFileContent(
                            securePinpadTrustedApp,
                            &pTrustedAppData);
    if (nTrustedAppLength == 0) {
        LOG_E("Trusted Application not found");
        free(tci);
        tci = NULL;
        return false;
    }

    LOG_I("Opening the Trusted Application session");
    memset(&tlSessionHandle, 0, sizeof(tlSessionHandle)); // Clear the session handle
    tlSessionHandle.deviceId = MC_DEVICE_ID_DEFAULT; // The device ID (default device is used)

    mcRet = mcOpenTrustlet(
                &tlSessionHandle,
                MC_SPID_RESERVED_TEST, /* mcSpid_t */
                pTrustedAppData,
                nTrustedAppLength,
                (uint8_t *) tci,
                (uint32_t) sizeof(tciMessage_t));

    free(pTrustedAppData);

    if (MC_DRV_OK != mcRet) {
        LOG_E("Open session failed: %d", mcRet);
        free(tci);
        tci = NULL;
        return false;
    }

    LOG_I("mcOpenTrustlet() succeeded");
    return true;
}

// -------------------------------------------------------------
/** callTrustedApp() - Notify Trusted Application and wait for response
 *
 * @param commandId     Command that is being send to Trusted Application
 *
 * Encodes the command in global TCI.
 * Notifies Trusted Application.
 * Waits for response from Trusted Application.
 * Checks that response ID is set and matches command ID.
 * Checks that return code is ok.
 *
 * @return  MC_DRV_OK upon success or specific error
 */
static mcResult_t callTrustedApp(const uint32_t commandId) {
    mcResult_t ret;
    tci->cmdPinpad.header.commandId = commandId;

    // -------------------------------------------------------------
    // Step 4.3: signal the Trusted Application
    ret = mcNotify(&tlSessionHandle);
    if (MC_DRV_OK != ret) {
        LOG_E("Notify failed: %d", ret);
        return ret;
    }

    // -------------------------------------------------------------
    // Step 4.4: Wait for the Trusted Application response
    ret = mcWaitNotification(&tlSessionHandle, -1);
    if (MC_DRV_OK != ret) {
        LOG_E("Wait for response notification failed: %d", ret);
        return ret;
    }

    // -------------------------------------------------------------
    // Step 4.5: Verify that the Trusted Application sent a response
    if ((RSP_ID(commandId) != tci->rspPinpad.header.responseId)) {
        LOG_E("Trusted Application did not send a response: %d",
                tci->rspPinpad.header.responseId);
        return  MC_DRV_ERR_INVALID_RESPONSE;
    }

    // -------------------------------------------------------------
    // Step 4.6: Check the Trusted Application return code
    if (RET_OK != tci->rspPinpad.header.returnCode) {
        LOG_E("Trusted Application did not send a valid return code: %d",
                tci->rspPinpad.header.returnCode);
        return tci->rspPinpad.header.returnCode;
    }

    return MC_DRV_OK;
}

// -------------------------------------------------------------
bool securePinpadGetResolution(uint32_t *width, uint32_t *height) {

    LOG_I("Get Resolution");

    if(NULL == tci) {
        LOG_E("TCI has not been set up properly - exiting");
        return !MC_DRV_OK;
    }

    mcResult_t ret = callTrustedApp(CMD_ID_PINPAD_GET_RES);

    if (ret != MC_DRV_OK) {
        LOG_E("Get resolution failed");
        return false;
    }

    *width = tci->rspPinpad.getResolution.screenWidth;
    *height = tci->rspPinpad.getResolution.screenHeight;

    return true;
}

// -------------------------------------------------------------
/** tlcWrapAndStore() - Make Trusted Application wrap data and store it in a file
 *
 * @param data      Buffer containing the data
 * @param length    Length of buffer
 * @param filename  Filename to store the encrypted object
 *
 * Copy data to TCI.
 * Call Trusted Application with provision command.
 * Write encrypted data from TCI to file.
 *
 * @return true, if operation was successful, false otherwise
 */
static bool tlcWrapAndStore(const uint8_t* data, uint32_t length, const char* filename)
{
    FILE* pDataStorageFile = NULL;

    LOG_I(" Provisioning: tlcWrapAndStore(data[0]=0x%x, length=%d)", data[0], length);

    // -------------------------------------------------------------
    // Copy data to TCI buffer
    if(NULL == tci) {
        LOG_E("TCI has not been set up properly - exiting");
        return false;
    }

    tci->len = length;
    memcpy(tci->data, data, length);

    mcResult_t ret = callTrustedApp(CMD_ID_PINPAD_PROVISION);

    if (ret != MC_DRV_OK) {
        LOG_E("Provision image failed");
        return false;
    }

    // -------------------------------------------------------------
    // Write encrypted data from TCI buffer to file
    LOG_I(" Provisioning: tci->len is %i", tci->len);
    pDataStorageFile = fopen((char*)filename, "wb");
    if (pDataStorageFile == NULL) {
        LOG_I("Provisioning: Opening of file %s failed!", filename);
        LOG_ERRNO("Opening");
        return false;
    }
    size_t write_val;
    write_val = fwrite(tci->data, sizeof(uint8_t),
            tci->len , pDataStorageFile);
    LOG_I(" Provisioning: write_val = %i\t\t", (int)write_val);
    fclose(pDataStorageFile);
    return true;
}

bool securePinpadProvisionItem(uint8_t itemID, const char* path,
                                const uint8_t* item, uint32_t size) {

    if((item == NULL) || (size <= 0) || (path == NULL)){
        return false;
    }

    if(itemID >= IDX_PINPAD_END) {
        return false;
    }
    strcpy(pathOfSO, path);
    strcat(pathOfSO, securePinpadSOFilenames[itemID]);
    listOfSOFile[itemID] = (char*) malloc(strlen(pathOfSO) + 1);
    if(listOfSOFile[itemID] == NULL) {
        return false;
    }
    strcpy(listOfSOFile[itemID], pathOfSO);
    LOG_I(" securePinpadProvisionItem: pathOfSO = %s", pathOfSO);
    LOG_I(" securePinpadProvisionItem: listOfSOFile[%d] = %s", itemID, listOfSOFile[itemID]);
    LOG_I(" securePinpadProvisionItem: begin of item data = 0x%X;0x%X;0x%X;0x%X;0x%X",
            item[0], item[1], item[2], item[3], item[4]);
    LOG_I(" securePinpadProvisionItem: end of item data = 0x%X;0x%X;0x%X;0x%X;0x%X",
                item[(size-1) - 4], item[(size-1) - 3], item[(size-1) - 2], item[(size-1) - 1], item[(size-1)]);
    return tlcWrapAndStore(item, size, listOfSOFile[itemID]);
}

bool securePinpadProvisionPadImage(const uint8_t* image, uint32_t length) {
    listOfSOFile[IDX_PINPAD_PINPAD] = (char*)securePinpadSOPadImage;
    return tlcWrapAndStore(image, length, securePinpadSOPadImage);
}

bool securePinpadProvisionStar(const uint8_t* image, uint32_t length) {
    listOfSOFile[IDX_PINPAD_STAR] = (char*)securePinpadSOStar;
    return tlcWrapAndStore(image, length, securePinpadSOStar);
}

bool securePinpadProvisionBlank(const uint8_t* image, uint32_t length) {
    listOfSOFile[IDX_PINPAD_BLANK] = (char*)securePinpadSOBlank;
    return tlcWrapAndStore(image, length, securePinpadSOBlank);
}

bool securePinpadProvisionKeyLayout(const uint8_t* layoutArray, uint32_t length) {
    listOfSOFile[IDX_PINPAD_LAYOUT] = (char*)securePinpadSOKeyLayout;
    return tlcWrapAndStore(layoutArray, length, securePinpadSOKeyLayout);
}

// -------------------------------------------------------------
bool readFile(uint8_t *dst, const uint32_t maxLen, uint32_t *len, const char* filename) {
    long int fileSize; //the file holding the secure object
    size_t read_val = 0;
    FILE* pDataStorageFile = NULL;
    int tmp;

    *len = 0;

    pDataStorageFile = fopen((char*)filename, "rb");
    if (pDataStorageFile == NULL) {
        LOG_E("File %s does not exist", filename);
        return false;
    }
    tmp = fseek(pDataStorageFile , 0 , SEEK_END);
    if (tmp != 0) {
        LOG_E("fseek error: %s", strerror(errno));
        fclose(pDataStorageFile);
        return false;
    }
    fileSize = ftell(pDataStorageFile);
    if (fileSize == -1) {
        LOG_E("ftell error: %s", strerror(errno));
        fclose(pDataStorageFile);
        return false;
    }

    LOG_I("File %s has size %li", filename, fileSize);
    if (fileSize > maxLen) {
        LOG_E("File is too big.");
        fclose(pDataStorageFile);
        return false;
    }
    rewind(pDataStorageFile);
    read_val = fread(dst, sizeof(uint8_t), fileSize, pDataStorageFile);
    if (read_val == -1) {
        LOG_E("fread error: %s", strerror(errno));
        fclose(pDataStorageFile);
        return false;
    }
    LOG_I(" File starts with: 0x%x   ", dst[0]);
    *len = read_val;
    fclose(pDataStorageFile);
    return true;
}

// -------------------------------------------------------------
const char *securePinpadVerify(void) {
    mcResult_t ret;

    if(NULL == tci) {
        LOG_E("TCI has not been set up properly - exiting");
        return "wrong set up";
    }

    // -------------------------------------------------------------
    // Read encrypted data from files to TCI buffer
    // Store offset and length in directory at TCI->objects

    /** Start offset in TCI buffer */
    uint32_t start = 0;
    /** Remaining size in TCI buffer */
    uint32_t remaining_size = MAX_DATA_LEN;
    /** Size of current object */
    uint32_t image_size = 0;
    /** Could the file be read correctly? */
    bool bFileRead = false;
    // For all pinpad items...
    for (int i=0; i<IDX_PINPAD_END; i++) {
        // Read-in file into TCI->data at offset start
        //  Use remaining size as limit for filesize
        //  listOfSOFile[i] contains the filename
        bFileRead = readFile(&tci->data[start], remaining_size, &image_size, listOfSOFile[i]);
        if (!bFileRead) {
            LOG_E("Failed to read file");
            return "Failed to read file";
        }
        // Store offset and length in directory
        tci->cmdPinpad.objects[i].offset = start;
        tci->cmdPinpad.objects[i].length = image_size;

        // Make sure offset is word-aligned to avoid problems in Trusted Application
        uint32_t align4 = image_size%4;
        image_size += 4-align4;

        // Increase offset and reduce remaining size
        start += image_size;
        remaining_size -= image_size;
    }
    // Store overall size
    tci->len = start;

    // -------------------------------------------------------------
    // Call Trusted Application
    ret = callTrustedApp(CMD_ID_PINPAD_LAUNCH);

    // -------------------------------------------------------------
    // Handle Trusted Application response
    if (ret != MC_DRV_OK) {
        LOG_E("Launch pinpad failed");
        return "Launching pinpad failed";
    }
    switch(tci->rspPinpad.launch.res) {
    case RES_ID_PINPAD_PIN_OK:
        LOG_I("Trusted Application Response: PIN verification: correct");
        return "PIN verification. correct";
    case RES_ID_PINPAD_PIN_NOK:
        LOG_I("Trusted Application Response: PIN is not correct");
        return "PIN verification. incorrect PIN";
    case RES_ID_PINPAD_CANCELLED:
        LOG_I("Trusted Application Response: TUI session was cancelled");
        return "PIN verification. PIN entry was cancelled";
    default:
        LOG_I("Trusted Application Response: Unknown response data");
        return "PIN verification. Invalid response from Trusted Application";
    }
}
// -------------------------------------------------------------
void securePinpadDisconnect() {
    mcResult_t ret;

    // -------------------------------------------------------------
    // Step 5: Close session with the Trusted Application
    LOG_I("Closing the Trusted Application session");
    ret = mcCloseSession(&tlSessionHandle);
    if (MC_DRV_OK != ret) {
        LOG_E("Closing Trusted Application session failed: %d", ret);
    }
    free(tci);

    LOG_I("mcCloseSession() succeeded");
    // -------------------------------------------------------------
    // Step 6: Close the MobiCore device
    LOG_I("Closing MobiCore device");
    ret = mcCloseDevice(MC_DEVICE_ID_DEFAULT);
    if (MC_DRV_OK != ret) {
        LOG_E("Closing MobiCore device failed: %d", ret);
        return;
    }

    LOG_I("mcCloseDevice() succeeded");
}

