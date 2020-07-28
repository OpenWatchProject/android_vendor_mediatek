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

/*
 * TlPinpad: Sample Trustlet for Trusted UI functionality
 */
#include "tlStd.h"
#include "TlApi/TlApi.h"
#include "mcVersionHelper.h"

#include "tlPinpadApi.h"
#include "tlPinpad.h"

#define TLTAG "<t-Pinpad: "

DECLARE_TRUSTLET_MAIN_STACK(2048);

/** Screen information */
tlApiTuiScreenInfo_t screen;

/** Pin entered */
uint8_t typedPin[MAX_PIN_LENGTH];
uint8_t typedPinLength;

/** Current position of pin input */
uint8_t index;

uint8_t currentTouchedButton;


/** Big secure memory buffer used for pictures */
uint8_t secureMemory[MAX_DATA_LEN];
#define UDATA_PLAIN_LEN  0      /**< No plaintext needed for the moment */
#define UDATA_ENC_LEN    200000  /**< Size of the biggest pictures */

/** Directory of items in secureMemory[] during launch command */
struct {
    uint8_t *p;         /**< Start of item */
    uint32_t length;    /**< Length of item */
} provisionedData[IDX_PINPAD_END];  /**< Defined for all items of launch command. */

/* Secret PIN */
static uint8_t secretPIN[] = {1, 2, 3, 4};

/** static_initTui() - Get information from TUI driver.
 *
 * Gets screen informations and stores them in Trustlet.
 *
 * @return TLAPI_OK upon success or specific error
 */
static tlApiResult_t static_initTui() {
    tlApiResult_t res;

    res = tlApiTuiGetScreenInfo(&screen);
    if (res != TLAPI_OK) {
        tlDbgPrintf(TLTAG "Get Screen Info failed. Cannot access TUI 0x%08X\n", res);
    } else {
        tlDbgPrintf(TLTAG "Screen (%d x %d, red:%d, green:%d, blue:%d,\n",
	        screen.width,
	        screen.height,
            screen.redBitDepth,
            screen.greenBitDepth,
            screen.blueBitDepth);
        tlDbgPrintf(TLTAG "gray:%d, density:%d x %d)\n",
	        screen.grayscaleBitDepth,
	        screen.wDensity,
	        screen.hDensity);
    }
    return res;
}

/** static_getTouchedKey() - Translate x,y coordinates into key
 *
 * @param x                 x coordinate of touch event
 * @param y                 y coordinate of touch event
 * @param[out] touched_key  key structure of touched key
 *
 * Loops over the layout structure to find the pressed key.
 * If found, copies the key description into touched_key.
 * Otherwise, <NULL> key is returned.
 *
 * @return  TLAPI_OK upon success or specific error
 */
static tciReturnCode_t static_getTouchedKey(uint32_t x, uint32_t y, sPinPadKey_t* const touched_key) {
    const sPinPadKey_t *key;
    tciReturnCode_t ret = RET_OK;
    sLayout_t *layout = (sLayout_t *)provisionedData[IDX_PINPAD_LAYOUT].p;
    *touched_key = layout->buttons_array[0];
    for (int i = 1; i <= PINPAD_SIZE; i++) {
        key = &layout->buttons_array[i];
        if (x > key->xLeft  && x < key->xRight &&
            y < key->yBottom && y > key->yTop) {
            memcpy(touched_key, key, sizeof(sPinPadKey_t));
            break;
        }
    }
    return ret;
}

/** static_setNextChar() - Display character at index
 *
 * @param keynum    Digit that was entered
 * @param index     Position in input field where to display
 *
 * Prepares the tlApiTuiSetImage() call to show star character at index.
 * Also advances the index by one character.
 *
 * @return  TLAPI_OK upon success or specific error
 */
static tlApiResult_t static_setNextChar(uint8_t keynum, uint8_t* const index) {
    tlDbgPrintf(TLTAG "Key %d entered, display '*' at %d\n", keynum, *index+1);
    tlApiResult_t res;

    tlApiTuiImage_t sImage;
    sImage.imageFile =          provisionedData[IDX_PINPAD_STAR].p;
    sImage.imageFileLength =    provisionedData[IDX_PINPAD_STAR].length;

    tlApiTuiCoordinates_t sCoordinates;
    sLayout_t *layout = (sLayout_t *)provisionedData[IDX_PINPAD_LAYOUT].p;
    sCoordinates.xOffset = layout->textarea.taLeft + (*index * layout->textarea.taCharsize);
    sCoordinates.yOffset = layout->textarea.taTop + ((3-(*index)) * layout->textarea.taMargin);
    res = tlApiTuiSetImage(&sImage, sCoordinates);
    if (res != TLAPI_OK) {
        tlDbgPrintf(TLTAG "Set image failed with 0x%x, cancelling.\n", res);
    }
    (*index)++;
    return res;
}

/** static_setButtonImage() - set Image of one pinpad button
 *
 * @param touched_key    Button that was touched
 * @param pressed        Pressed or released
 *
 * Prepares the tlApiTuiSetImage() call to show the negative picture of the button.
 *
 * @return  TLAPI_OK upon success or specific error
 */
static tlApiResult_t static_setButtonImage(sPinPadKey_t key, bool pressed) {
    tlDbgPrintf(TLTAG "static_setButtonImage for key = %d  -  pressed = %d\n", key.val, pressed);
    tlApiResult_t res;
    uint8_t       provIdx;

    /* Calculate the provisionning index with the button value */
    provIdx = key.val + IDX_PINPAD_0;
    if (pressed) {
        provIdx += IDX_DELTA_PRESSED;
    }

    tlApiTuiImage_t sImage;
    sImage.imageFile =          provisionedData[provIdx].p;
    sImage.imageFileLength =    provisionedData[provIdx].length;

    tlApiTuiCoordinates_t sCoordinates;
    sCoordinates.xOffset = key.xLeft;
    sCoordinates.yOffset = key.yTop;
    res = tlApiTuiSetImage(&sImage, sCoordinates);
    if (res != TLAPI_OK) {
        tlDbgPrintf(TLTAG "Set image failed with 0x%x, cancelling.\n", res);
    }

    return res;
}

/** static_setReleasedButtonImage() - set Image of one pinpad button
 *
 * @param value    Value of the button to display
 *
 * Prepares the tlApiTuiSetImage() call to show the picture of the released button.
 *
 * @return  TLAPI_OK upon success or specific error
 */
static tlApiResult_t static_setReleasedButtonImage(uint8_t value) {
    const sPinPadKey_t *key;
    tciReturnCode_t ret = RET_OK;
    sLayout_t *layout = (sLayout_t *)provisionedData[IDX_PINPAD_LAYOUT].p;
    for (int i = 1; i <= PINPAD_SIZE; i++) {
        key = &layout->buttons_array[i];
        if (key->val == value) {
            ret = static_setButtonImage(*key, false);
            break;
        }
    }
    return ret;
}

/** static_removeChar() - Display blank image at index
 *
 * @param index     Position in input field where remove a character
 *
 * Prepares the tlApiTuiSetImage() call to show blank character at index.
 * Also moves the index back one character.
 *
 * @return  TLAPI_OK upon success or specific error
 */
static tlApiResult_t static_removeChar(uint8_t* const index) {
    (*index)--;
    tlDbgPrintf(TLTAG "Clear field %d\n", *index+1);
    tlApiResult_t res;

    tlApiTuiImage_t sImage;
    sImage.imageFile =          provisionedData[IDX_PINPAD_BLANK].p;
    sImage.imageFileLength =    provisionedData[IDX_PINPAD_BLANK].length;

    tlApiTuiCoordinates_t sCoordinates;
    sLayout_t *layout = (sLayout_t *)provisionedData[IDX_PINPAD_LAYOUT].p;
    sCoordinates.xOffset = layout->textarea.taLeft + (*index * layout->textarea.taCharsize);
    sCoordinates.yOffset = layout->textarea.taTop + ((3-(*index)) * layout->textarea.taMargin);
    res = tlApiTuiSetImage(&sImage, sCoordinates);
    if (res != TLAPI_OK) {
        tlDbgPrintf(TLTAG "Set image failed with 0x%x, cancelling.\n", res);
    }
    return res;
}

/** processEventFromDriver() - Get touch event data and advance pinpad state
 *
 * @param[out] tryAgain     Boolean set to false if no more events are pending
 *
 * Collect data of one touch event from TUI driver.
 * Do error handling:
 *  - cancel pinpad entry on any error (i.e. go to cancel state)
 *  - stop processing if no event was pending.
 * Do basic event filtering: ignore press events.
 *
 * Determine pressed key of pinpad:
 *  - On no key, remain in default state
 *  - On 'cancel' key, go to cancel state
 *  - On 'correct' key, if possible, remove one character, remain in default state.
 *  - On 'number' keys, if possible, display character, remain in default state.
 *  - On 'validate' key, if enough characters, go to validate state.
 *
 * @return  new state of pinpad
 */
static transactionState_t processEventFromDriver(bool *tryAgain) {
    tlApiTuiTouchEvent_t touchEvent;
    sPinPadKey_t touched_key;
    tlApiResult_t ret;
    uint32_t x,y;
    *tryAgain = true;
    transactionState_t st;

    memset(&touchEvent, 0, sizeof(touchEvent));

    ret = tlApiTuiGetTouchEvent(&touchEvent);
    if (ret == E_TLAPI_TUI_NO_EVENT) {
        //tlDbgPrintf(TLTAG "No event to process.\n");
        *tryAgain = false;
        return ST_IMAGE_SET;
    }
    if (ret == E_TLAPI_TUI_NO_SESSION) {
        tlDbgPrintf(TLTAG "Session was closed.\n");
        return ST_CANCELLED;
    }
    if (ret != TLAPI_OK) {
        tlDbgPrintf(TLTAG "Cancelling, because getEvent() = 0x%x.\n", ret);
        return ST_CANCELLED;
    }

    x = touchEvent.coordinates.xOffset;
    y = touchEvent.coordinates.yOffset;
    bool pressed = touchEvent.type != TUI_TOUCH_EVENT_RELEASED;

    static_getTouchedKey(x, y, &touched_key);

    /* Basic event filtering */
    if (pressed) {
        /* Set the pressed button image. But do not set it continuously during long press */
        if (touched_key.val != currentTouchedButton) {
            if (touched_key.type != NOKEY) {
                ret = static_setButtonImage(touched_key, pressed);
            } else {
                /* Reset the picture of the previous touched button */
                ret = static_setReleasedButtonImage(currentTouchedButton);
            }
            currentTouchedButton = touched_key.val;
        } else {
            //tlDbgPrintf(TLTAG "Pressed same key. Do nothing.\n");
        }
        /* Return: don't process data on pressing event */
        st = ret == RET_OK ? ST_IMAGE_SET : ST_CANCELLED;
        return st;
    } else {
        /* Reset the picture of the previous touched button */
        if (currentTouchedButton != PAD_NOKEY_VAL) {
            ret = static_setReleasedButtonImage(currentTouchedButton);
            currentTouchedButton = PAD_NOKEY_VAL;
        } else {
            tlDbgPrintf(TLTAG "Release but no button to reset. Do nothing.\n");
        }
        /* Return Cancel if a setImage function failed */
        if (ret != TLAPI_OK) {
            return ST_CANCELLED;
        }
    }


    //tlDbgPrintf(TLTAG "Touch release event, %d numbers already entered\n", *index);
    switch(touched_key.type) {
        case PAD_NUM:
            if (index < MAX_PIN_LENGTH) {
                typedPin[index] = touched_key.val;
                ret = static_setNextChar(touched_key.val, &index);
                if (ret != TLAPI_OK) {
                    return ST_CANCELLED;
                }
            } else {
                tlDbgPrintf(TLTAG "Ignoring entered key.\n");
            }
            break;
        case PAD_CORRECT:
            if (index > 0) {
                ret = static_removeChar(&index);
                if (ret != TLAPI_OK) {
                    return ST_CANCELLED;
                }
            } else {
                tlDbgPrintf(TLTAG "Nothing to remove.\n");
            }
            break;
        case PAD_CANCEL:
            tlDbgPrintf(TLTAG "CANCEL\n");
            return ST_CANCELLED;
        case PAD_VALIDATE:
            if (index >= MIN_PIN_LENGTH) {
                tlDbgPrintf(TLTAG "VALIDATED\n");
                typedPinLength = index;
                return ST_VALIDATED;
            } else {
                tlDbgPrintf(TLTAG "Not enough characters for VALIDATE, (have %d)\n", index);
            }
            break;
        default:
            tlDbgPrintf(TLTAG "No Key was touched\n");
            break;
    } /* switch(key) */
    return ST_IMAGE_SET;
}

/** processEventsFromDriver() - Get all touch events and advance pinpad state
 *
 * Call processEventFromDriver() as long as events are pending and pinpad entry
 *  was not finished.
 *
 * @return  new state of pinpad
 */
static transactionState_t processEventsFromDriver() {
    transactionState_t state;
    bool tryAgain = true;
    do {
        state = processEventFromDriver(&tryAgain);
    } while(tryAgain && state == ST_IMAGE_SET);
    return state;
}

/** processNotificationsFromDriver() - Process events and respond to TLC
 *
 * @param[out] message     TCI message that will contain the result of pin verification
 *
 * Call processEventsFromDriver() and prepare response to TLC if pin entry was
 *  finished.
 * Validates that typed pin equals provisioned pin.
 *
 * @return  new state of pinpad
 */
static transactionState_t processNotificationsFromDriver(
    tciMessage_t *message) {

    transactionState_t state = processEventsFromDriver();
    if (state == ST_VALIDATED) {
        uint32_t distance;
        tlDbgPrintf(TLTAG "Entered PIN: ");
        for (uint8_t i = 0; i < typedPinLength; i++) {
            tlDbgPrintf(TLTAG "%d ", typedPin[i]);
        }
        tlDbgPrintf(TLTAG "Size of secret PIN = %d", sizeof(secretPIN));
        distance = (typedPinLength == sizeof(secretPIN)) ?
                memcmp(typedPin, secretPIN, typedPinLength) : -1;
        if (distance == 0) {
            message->rspPinpad.launch.res = RES_ID_PINPAD_PIN_OK;
        } else {
            message->rspPinpad.launch.res = RES_ID_PINPAD_PIN_NOK;
        }
    } else if (state == ST_CANCELLED) {
        message->rspPinpad.launch.res = RES_ID_PINPAD_CANCELLED;
    }
    return state;
}

/** processCmdGetResolution() - Store resolution in TCI message for TLC
 *
 * @param[out] pmessage     TCI message
 *
 * @return  RET_OK
 */
static tciReturnCode_t processCmdGetResolution(tciMessage_t* pmessage) {
    tlDbgPrintf(TLTAG "Get Resolution (%d x %d)\n",screen.width,screen.height);
    pmessage->rspPinpad.getResolution.screenWidth = screen.width;
    pmessage->rspPinpad.getResolution.screenHeight = screen.height;
    return RET_OK;
}

/** processCmdProvisionImage() - Apply wrap operation to data in TCI
 *
 * @param[in/out] pmessage     TCI message
 *
 * Copy data from TCI to secure memory.
 * Create secure object of data into TCI.
 *
 * @return  RET_OK upon success or specific error
 */
static tciReturnCode_t processCmdProvisionImage(tciMessage_t* pmessage) {
    tciReturnCode_t ret = RET_OK;
    uint32_t len = pmessage->len;
    size_t soLength = MC_SO_SIZE_F22(UDATA_PLAIN_LEN, UDATA_ENC_LEN);
    tlDbgPrintf(TLTAG "Provision: input len = %d, output len=%d\n", len, soLength);
    if (soLength > MAX_DATA_LEN || len > UDATA_ENC_LEN) {
        return RET_ERR_PINPAD_INVALID_LENGTH;
    }
    /* Copy the data into secure memory. */
    memmove(secureMemory, pmessage->data, len);

    /* Wrap into TCI / world shared memory. */
    ret = tlApiWrapObject(
            secureMemory,
            0,
            len,
            (mcSoHeader_t *)pmessage->data,
            &soLength,
            MC_SO_CONTEXT_TLT,
            MC_SO_LIFETIME_PERMANENT,
            NULL,
            TLAPI_WRAP_DEFAULT); //private wrapping

    if (ret != TLAPI_OK) {
        tlDbgPrintf(TLTAG "Provision: tlApiWrapObject() = 0x%x\n", ret);
        return RET_ERR_PINPAD_WRAP;
    }

    pmessage->len = soLength;
    return ret;
}

/** static_unwrap_images() - Unwrap objects from TCI to secure memory
 *
 * @param[in] pmessage     TCI message
 *
 * Copy encrypted data from TCI to secure memory.
 * Copy directory information from TCI to secure memory.
 * Unwrap five secure objects:
 *  - Use directory information to find object limits
 *  - Unwrap in place
 *  - Store new directory information in global variable
 *
 * @return  RET_OK upon success or specific error
 */
static tciReturnCode_t static_unwrap_images(tciMessage_t* pMessage) {
    tlApiResult_t res;

    /* Copy everything to secure memory. */
    uint32_t len = pMessage->len;
    tlDbgPrintf(TLTAG "unwrap images: total length = %d\n", len);
    if (len > MAX_DATA_LEN) {
        return  RET_ERR_PINPAD_INVALID_LENGTH;
    }

    memmove(secureMemory, pMessage->data, len);

    secureObjectInData_t objects[IDX_PINPAD_END];
    memmove(objects, pMessage->cmdPinpad.objects, sizeof(objects));

    //-------------------------------------------------------------------------
    /* Unwrap secure objects */
    size_t objlen;
    uint32_t objoffset;
    uint8_t *objStart;

    for (int i=IDX_PINPAD_PINPAD; i<IDX_PINPAD_END; i++) {
        objlen      = objects[i].length;
        objoffset   = objects[i].offset;

        //tlDbgPrintf(TLTAG "here %i; offset=%x\n",i, objoffset);

        if (objlen > MAX_DATA_LEN || objoffset > MAX_DATA_LEN || objlen+objoffset > MAX_DATA_LEN) {
            return  RET_ERR_PINPAD_INVALID_LENGTH;
        }

        objStart = &secureMemory[objoffset];
        res = tlApiUnwrapObjectExt( (mcSoHeader_t*)objStart,
                objlen,
                objStart,
                &objlen,
                TLAPI_UNWRAP_PERMIT_DELEGATED | TLAPI_UNWRAP_DEFAULT);
        tlDbgPrintf(TLTAG "tlApiUnwrap()=%i, image(len=%i, [0]=0x%x)\n",
            res, objlen, objStart[0]);
        if (res != TLAPI_OK) {
            return RET_ERR_PINPAD_UNWRAP;
        }
        provisionedData[i].length = objlen;
        provisionedData[i].p = objStart;
    }

    return RET_OK;
}
/** processCmdLaunchPinPad() - Show secure pinpad and start waiting for events
 *
 * @param[in] message     TCI message
 * @param[out] pState     New state after this command
 *
 * Process launch command
 *  - unwrap secure objects and store in secure memory
 *  - init TUI session and set background image
 *  - advance state machine to wait for secure input
 *
 * @return  RET_OK upon success or specific error
 */

static tciReturnCode_t processCmdLaunchPinPad(
    tciMessage_t* message,
    transactionState_ptr pState) {

    tciReturnCode_t ret;
    tlApiResult_t   res;
    ret = static_unwrap_images(message);
    if (ret != RET_OK) {
        tlDbgPrintf(TLTAG "Cannot get the stored image\n");
        return ret;
    }

    res = tlApiTuiOpenSession();
    if (res != TLAPI_OK) {
        tlDbgPrintf(TLTAG "Cannot open session, err=0x%x\n", res);
        return RET_ERR_PINPAD_NO_RESOURCE;
    }
    tlApiTuiImage_t sImage;
    tlApiTuiCoordinates_t sCoordinates;
    sImage.imageFile =          provisionedData[IDX_PINPAD_PINPAD].p;
    sImage.imageFileLength =    provisionedData[IDX_PINPAD_PINPAD].length;
    sCoordinates.xOffset = 0;
    sCoordinates.yOffset = 0;
    tlDbgPrintf(TLTAG "TUI session is opened, set image.\n");
    *pState = ST_INITIALIZED;
    res = tlApiTuiSetImage(&sImage, sCoordinates);
    if (res != TLAPI_OK) {
        tlDbgPrintf(TLTAG "Set image failed with 0x%x, cancelling.\n", res);
        return RET_ERR_PINPAD_NO_RESOURCE;
    }
    *pState =  ST_IMAGE_SET;
    tlDbgPrintf(TLTAG "Image set, waiting for inputs.\n");
    tlDbgPrintf(TLTAG "------------------------------\n");
    return RET_OK;
}


/**
 * Trustlet entry.
 */
_TLAPI_ENTRY void tlMain(
    const addr_t tciBuffer,
    const uint32_t tciBufferLen
) {
    // Initialization sequence
    tciReturnCode_t ret = RET_OK;
    tciCommandId_t commandId=100;
    bool notifyTlc;
    transactionState_t state = ST_NONE;
    tciMessage_t* message;
    tlApiResult_t res = TLAPI_OK;
    index = 0;
    currentTouchedButton = PAD_NOKEY_VAL;

    tlDbgPrintf(TLTAG "Starting\n");

    // Check if the size of the given TCI is sufficient
    if ((NULL == tciBuffer) || (sizeof(tciMessage_t) > tciBufferLen))
    {
        tlDbgPrintf(TLTAG "TCI error, buffer: %p, len=%i, sizeof(tciM_t)=%i\n",
                    tciBuffer, tciBufferLen, sizeof(tciMessage_t));
        // TCI too small -> end Trustlet
        tlApiExit(EXIT_ERROR);
    }
    message = (tciMessage_t*) tciBuffer;

    // Get information about Trusted UI
    res = static_initTui();
    if (res != TLAPI_OK) {
        tlApiExit(EXIT_ERROR);
    }

    // The Trustlet main loop running infinitely
    for (;;) {

        notifyTlc = true;
        // tlDbgPrintf(TLTAG "Wait for notifications\n");
        // Wait for a notification to arrive (INFINITE timeout is recommended -> not polling!)
        //  - Notification can come from TLC, i.e. "get resolution command"
        //  - Notification can come from TUI, i.e. there are one or more events pending
        tlApiWaitNotification(TLAPI_INFINITE_TIMEOUT);

        // Copy commandId to secure memory
        commandId = message->cmdPinpad.header.commandId;
        //tlDbgPrintf("\n" TLTAG "Got a message! Command %d\n",commandId);

        // Check if the message received is (still) a response
        if (!IS_CMD(commandId)) {
            // Tell the NWd a response is still pending (optional)
            tlDbgPrintf(TLTAG "A response is still pending.");
            tlApiNotify();
            continue;
        }

        // Process command message
        switch (commandId) {
        //-----------------------------------------------
        case CMD_ID_PINPAD_DRV:
            //tlDbgPrintf(TLTAG "Notification from the driver\n");
            notifyTlc = false;
            if (state == ST_NONE) {
                tlDbgPrintf(TLTAG "Ignoring driver notification, we have already closed our session.\n");
            } else {
                state = processNotificationsFromDriver(message);
                if (state == ST_VALIDATED || state == ST_CANCELLED) {
                    notifyTlc = true;
                    ret = RET_OK;
                    commandId = CMD_ID_PINPAD_LAUNCH; //reset commandId for Tlc notifications
                }
            }
            break;

        case CMD_ID_PINPAD_GET_RES:
            ret = processCmdGetResolution(message);
            break;

        case CMD_ID_PINPAD_PROVISION:
            if (state == ST_NONE) {
                ret = processCmdProvisionImage(message);
            } else {
                ret = RET_ERR_PINPAD_IN_PROGRESS;
            }
            break;

        case CMD_ID_PINPAD_LAUNCH:
            if (state == ST_NONE) {
                ret = processCmdLaunchPinPad(message, &state);
                notifyTlc = (state != ST_IMAGE_SET);
            } else {
                ret = RET_ERR_PINPAD_IN_PROGRESS;
            }
            break;
        //-----------------------------------------------
        default:
            // Unknown command ID
            dbgBlob(TLTAG "Invalid commandId, dumping TCI:",tciBuffer,32);
            ret = RET_ERR_UNKNOWN_CMD;
            break;
        } // end switch (commandId)

        // Reset command id, so when no command is set, we know it's the driver
        message->cmdPinpad.header.commandId = CMD_ID_PINPAD_DRV;

        // Some transitions require sending a message to TLC
        if (notifyTlc) {
            // Whenever we notify TLC, we make sure TUI session is closed.
            if (state != ST_NONE) {
                tlApiTuiCloseSession();
                tlDbgPrintf(TLTAG "TUI Session is closed.\n");
                state = ST_NONE;
            }
            // Set up response header -> mask response ID and set return code
            message->rspPinpad.header.responseId = RSP_ID(commandId);
            message->rspPinpad.header.returnCode = ret;
            tlDbgPrintf(TLTAG "Notify the TLC, returnCode = %d\n",ret);
            tlDbgPrintf(TLTAG "------------------------------\n");

            // Notify back the TLC
            tlApiNotify();
        }
    }
}
