/* MediaTek Inc. (C) 2016. All rights reserved.
 *
 * Copyright Statement:
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 */

/*
 * Description:
 *   Implement convert hex XML to dec format tool
 */

#include "AudioParamParser.h"
#include "AudioParamParserPriv.h"

#include <stdio.h>
#include <string.h>

static const char *INPUT_HEX_DIR[] = {
    ".\\hex_xml\\",
    NULL
};
#define OUTPUT_DEC_DIR ".\\dec_xml"

int main() {
    size_t num, i;
    AppHandle *appHandle = NULL;
    AppHandle Handle;

    /* Set the debug level, default is INFO_LEVEL */
    appSetDebugLevel(INFO_LEVEL);

    /* For Tuning Tool debug usage, used to show the APP lib message to the console */
    appHandleRedirectIOToConsole();

    /* Init app handle */
    appHandleInit(&Handle);
    appHandle = &Handle;

    /* Save XML with Dec mode */
    appHandle->saveXmlWithHexMode = 0;

    /* Parse the xml in default and cus folder,
      if cus folder has the same name of XML file,
      parser will load the cus folder xml instead of default xml folder */
    appHandleParseXml(appHandle, INPUT_HEX_DIR, NULL);

    /* Save all Xml */
    num = appHandleGetNumOfAudioType(appHandle);
    i = 0;

    for (i = 0; i < num; i++) {
        AudioType *audioType = appHandleGetAudioTypeByIndex(appHandle, i);

        /* Read lock */
        audioTypeReadLock(audioType, __FUNCTION__);

        /* Save changed AudioType to XML */
        audioTypeSaveAudioParamXml(audioType, OUTPUT_DEC_DIR, 1);

        /* Unlock */
        audioTypeUnlock(audioType);
    }

    /* Release appHandle resources */
    appHandleUninit(appHandle);

    return 0;
}

