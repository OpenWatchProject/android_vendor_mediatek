/*
 * Copyright (c) 2013-2016 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#ifndef _TLC_PINPAD_H_
#define _TLC_PINPAD_H_

/** File paths for secure objects created by provisioning step */
const char securePinpadSOPadImage[] = "/data/app/pinpadSO.padImage";
const char securePinpadSOStar[] = "/data/app/pinpadSO.star";
const char securePinpadSOBlank[] = "/data/app/pinpadSO.blank";
const char securePinpadSOKeyLayout[] = "/data/app/pinpadSO.keylayout";
const char securePinpadSOPin[] = "/data/app/pinpadSO.pin";

/** Secure pinpad API */

/** Connect to secure pinpad functionality */
bool securePinpadConnect();

/** Get resolution of display */
bool securePinpadGetResolution(uint32_t *width, uint32_t *height);

/** Create secured objects of the images, key layout and correct pin */
bool securePinpadProvisionItem(uint8_t itemID, const char* path,
                                const uint8_t* item, uint32_t size);
bool securePinpadProvisionPadImage(const uint8_t* image, uint32_t length);
bool securePinpadProvisionStar(const uint8_t* image, uint32_t length);
bool securePinpadProvisionBlank(const uint8_t* image, uint32_t length);
bool securePinpadProvisionKeyLayout(const uint8_t* layoutArray, uint32_t length);
bool securePinpadProvisionPin(const uint8_t pin[4]);

/** securePinpadVerify() - Get a PIN from user
 * Show pinpad and process user input until completion or cancellation.
 *
 * @return: string telling if correct pin was entered.
 */
const char *securePinpadVerify(void);

/** Disconnect from secure pinpad functionality */
void securePinpadDisconnect();

/** readFile() - helper function for reading files
 *
 * @param dst       buffer where to put the file content
 * @param maxLen    size of buffer
 * @param[out] len  actual size of file
 * @param filename  name and path of file to open
 *
 * @return true, if file was correcty read, false otherwise
 */
bool readFile(uint8_t *dst, const uint32_t maxLen, uint32_t *len, const char* filename);

/**
 * @brief Retrieve content from the apk
 * @param assetName
 * @param pContent
 * @param maxSize
 * @return
 */
ssize_t getAssetContent(const char *assetName, uint8_t* pContent, size_t maxSize);

#endif // _TLC_PINPAD_H_
