/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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

/*
**
** Copyright 2016, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#define LOG_NDEBUG 0
#define LOG_TAG "WhiteListService"
#include <utils/Log.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <libexpat/expat.h>

#include "IMtkWLService.h"
#include "MtkWLService.h"
#include <utils/Errors.h>  // for status_t
#include <utils/String8.h>
#include <utils/String16.h>
#include <utils/SystemClock.h>
#include <utils/Vector.h>
#include <cutils/properties.h>
#include <cutils/atomic.h>
#include <cutils/properties.h> // for property_get

#include <utils/misc.h>
#include <binder/IServiceManager.h>

#define WL_CONFIG_FILE "/etc/whitelist_cfg.xml"

//#define ALOGV ALOGE

namespace android {

static void StartElementHandlerWrapper(
        void *me, const char *name, const char **attrs) {
    static_cast<WhiteListService *>(me)->startElementHandler(name, attrs);
}

static void EndElementHandlerWrapper(void *me, const char *name) {
    static_cast<WhiteListService *>(me)->endElementHandler(name);
}

void WhiteListService::instantiate() {
    ALOGV("Instantiate white list service...");
    defaultServiceManager()->addService(
            String16("WhiteListService"), new WhiteListService());
}

WhiteListService::WhiteListService()
{
    ALOGV("WhiteListService created");
    mCurrentSection         = SECTION_TOPLEVEL;
    mDepth                  = 0;
    
    memset(mFeature, 0, sizeof(char) * 256);
    memset(mApplication, 0, sizeof(char) * 256);
    memset(mCallerApp, 0, sizeof(char) * 256);

    mFeatureCount           = 0;
    mOptionCount            = 0;
    memset(mOptionList, 0, sizeof(P_Option) * 256);
    memset(mFeatureList, 0, sizeof(P_Feature) * 256);
    
    pOptionTemp             = NULL;
    pFeatureTemp            = NULL;
    ParsingWhiteListConfig();
}

WhiteListService::~WhiteListService()
{
    ALOGV("WhiteListService destructor.");
    P_Feature   FeatureTemp;
    P_Application ApplicationTemp;
    P_Option    OptionTemp;
    for (int i = 0; i < mFeatureCount; i++) {
        FeatureTemp = mFeatureList[i];
        for (int j = 0; j < FeatureTemp->mApplicationCount; j++) {
            ApplicationTemp = FeatureTemp->mApplicationList[j];
            for (int k = 0; k < ApplicationTemp->mOptionCount; k++) {
                if (ApplicationTemp->mOptionList[k] != NULL) {
                    delete(ApplicationTemp->mOptionList[k]);
                    ApplicationTemp->mOptionList[k] = NULL;
                }
            }
            delete(ApplicationTemp);
        }
        delete(FeatureTemp);
    }

    memset(mFeature, 0, sizeof(char) * 256);
    memset(mApplication, 0, sizeof(char) * 256);
    memset(mCallerApp, 0, sizeof(char) * 256);

    mFeatureCount           = 0;
    mOptionCount            = 0;
    memset(mOptionList, 0, sizeof(P_Option) * 256);
    memset(mFeatureList, 0, sizeof(P_Feature) * 256);
}

void WhiteListService::startElementHandler(const char *name, const char **attrs)
{
    size_t i;

    switch (mCurrentSection) {
        case SECTION_TOPLEVEL:
        {
            ALOGV("SECTION_TOPLEVEL name : %s\n", name);
            mCurrentSection = SECTION_WHITELIST;
            break;
        }

        case SECTION_WHITELIST:
        {
            ALOGV("SECTION_WHITELIST name : %s\n", name);
            CreateFeature(name);
            mCurrentSection = SECTION_FEATURE;
            break;
        }

        case SECTION_FEATURE:
        {
            ALOGV("SECTION_FEATURE name : %s\n", name);
            if (!strcmp(name, "Application")) {
                i = 0;
                while (attrs[i] != NULL) {
                    ALOGV("%s : %s\n",attrs[i], attrs[i + 1]);
                    if (!strcmp(attrs[i], "name")) {
                        if (attrs[i + 1] == NULL) {
                            break;
                        }
                        CreateApplicationForFeature(attrs[i+1]);
                        mCurrentSection = SECTION_APPLICATION;
                        ++i;
                    } else {
                        ALOGV("return\n");
                        break;
                    }

                    ++i;
                }
            }
            break;
        }

        case SECTION_APPLICATION:
        {
            ALOGV("SECTION_APPLICATION name : %s\n", name);
            if (!strcmp(name, "option")) {
                mCurrentSection = SECTION_OPTION;
                i = 0;
                while (attrs[i] != NULL) {
                    ALOGV("%s : %s\n",attrs[i], attrs[i + 1]);
                    if (!strcmp(attrs[i], "name")) {
                        if (attrs[i + 1] == NULL) {
                            break;
                        }
                        // create option handle here
                        CreateOptionForApplication(attrs[i+1]);
                        ++i;
                    }
                    else if (!strcmp(attrs[i], "value")) {
                        if (attrs[i + 1] == NULL) {
                            break;
                        }
                        // add option value here
                        AddOptionValue(attrs[i+1]);
                        ++i;
                    } else {
                        ALOGV("return\n");
                        break;
                    }

                    ++i;
                }
            }
            break;
        }
        case SECTION_OPTION:
        {
            ALOGV("SECTION_OPTION name : %s\n", name);
            i = 0;
            while (attrs[i] != NULL) {
                ALOGV("%s : %s\n",attrs[i], attrs[i + 1]);
                if (!strcmp(attrs[i], "name")) {
                    if (attrs[i + 1] == NULL) {
                        break;
                    }
                    // nothing to do now
                    ++i;
                } else {
                    ALOGV("return\n");
                    break;
                }

                ++i;
            }
            break;
        }

        default:
            break;
    }

    ++mDepth;
}

void WhiteListService::endElementHandler(const char *name)
{
    switch (mCurrentSection) {
        case SECTION_TOPLEVEL:
        {
            ALOGV("endElementHandler SECTION_TOPLEVEL in %s\n", name);
        }

        case SECTION_WHITELIST:
        {
            ALOGV("endElementHandler SECTION_WHITELIST in %s\n", name);
            mCurrentSection = SECTION_TOPLEVEL;
            break;
        }
        case SECTION_FEATURE:
        {
            ALOGV("endElementHandler SECTION_FEATURE in %s\n", name);
            mCurrentSection = SECTION_WHITELIST;
            break;
        }

        case SECTION_APPLICATION:
        {
            ALOGV("endElementHandler SECTION_APPLICATION in %s\n", name);
            mCurrentSection = SECTION_FEATURE;
            break;
        }

        case SECTION_OPTION:
        {
            ALOGV("endElementHandler SECTION_OPTION in %s\n", name);
            mCurrentSection = SECTION_APPLICATION;
            break;
        }

        default:
            break;
    }

    --mDepth;
}

bool WhiteListService::ParsingWhiteListConfig()
{
    bool bSuccess = true;
    FILE *file = fopen(WL_CONFIG_FILE, "r");

    if (file == NULL) {
        ALOGE("unable to open white list configuration xml file: %s", WL_CONFIG_FILE);
        bSuccess = false;
        return bSuccess;
    }
    
    XML_Parser parser = ::XML_ParserCreate(NULL);
    if (parser == NULL) {
        ALOGE("unable to create XML parser");
        bSuccess = false;
        return bSuccess;
    }


    ::XML_SetUserData(parser, (void *)this);
    ::XML_SetElementHandler(parser, StartElementHandlerWrapper, EndElementHandlerWrapper);

    const int BUFF_SIZE = 512;
    while (1) {
        void *buff = ::XML_GetBuffer(parser, BUFF_SIZE);
        if (buff == NULL) {
            ALOGE("failed in call to XML_GetBuffer()");
            break;
        }

        int bytes_read = ::fread(buff, 1, BUFF_SIZE, file);
        if (bytes_read < 0) {
            ALOGE("failed in call to read");
            break;
        }

        XML_Status status = ::XML_ParseBuffer(parser, bytes_read, bytes_read == 0);
        if (status != XML_STATUS_OK) {
            ALOGE("malformed (%s)", ::XML_ErrorString(::XML_GetErrorCode(parser)));
            break;
        }

        if (bytes_read == 0) {
            break;
        }
    }

    ::XML_ParserFree(parser);

    fclose(file);
    file = NULL;

    return bSuccess;
}

void WhiteListService::CreateOptionForApplication(const char *name)
{
    pFeatureTemp = mFeatureList[mFeatureCount-1];
    P_Application pApplicationTemp = pFeatureTemp->mApplicationList[pFeatureTemp->mApplicationCount-1];

    pOptionTemp = NULL;

    pOptionTemp = (P_Option) malloc(sizeof(Option));
    if (pOptionTemp != NULL) {
        memset(pOptionTemp, 0x00, sizeof(Option));
        strcpy(pOptionTemp->_name, name);
        pApplicationTemp->mOptionList[pApplicationTemp->mOptionCount] = pOptionTemp;
        ALOGV("pOptionTemp address 0x%08x (%d) \n", pOptionTemp, pApplicationTemp->mOptionCount);
        pApplicationTemp->mOptionCount++;
    }
    else {
        ALOGE("CreateOptionForApplication() alloc memory fail.\n");
    }
}

void WhiteListService::AddOptionValue(const char *value)
{
    strcpy(pOptionTemp->_value, value);
    ALOGV("pOptionTemp address 0x%08x (%d) (%s , %s) \n", pOptionTemp, mOptionCount, pOptionTemp->_name, pOptionTemp->_value);
}

void WhiteListService::CreateFeature(const char *name)
{
    pFeatureTemp = NULL;

    pFeatureTemp = (P_Feature) malloc(sizeof(Feature));
    if (pFeatureTemp != NULL) {
        memset(pFeatureTemp, 0x00, sizeof(Feature));
        strcpy(pFeatureTemp->_name, name);
        mFeatureList[mFeatureCount] = pFeatureTemp;
        ALOGV("pFeatureTemp address 0x%08x (%d) \n", pFeatureTemp, mFeatureCount);
        mFeatureCount++;
    }
    else {
        ALOGE("CreateFeature() alloc memory fail.\n");
    }
}

void WhiteListService::CreateApplicationForFeature(const char *name)
{
    pFeatureTemp = mFeatureList[mFeatureCount-1];
    P_Application pApplicationTemp = NULL;

    pApplicationTemp = (P_Application) malloc(sizeof(Application));
    if (pApplicationTemp != NULL) {
        memset(pApplicationTemp, 0x00, sizeof(Application));
        strcpy(pApplicationTemp->_name, name);
        pFeatureTemp->mApplicationList[pFeatureTemp->mApplicationCount] = pApplicationTemp;
        ALOGV("pApplicationTemp address 0x%08x (%d) \n", pApplicationTemp, pFeatureTemp->mApplicationCount);
        pFeatureTemp->mApplicationCount++;
    }
    else {
        ALOGE("CreateApplicationForFeature() alloc memory fail.\n");
    }
}

void WhiteListService::CopyOptionList(const P_Application pApplication)
{
    int i = 0;
    for (i = 0; i < pApplication->mOptionCount; i++) {
        mOptionList[i] = pApplication->mOptionList[i];
    }
    mOptionCount = pApplication->mOptionCount;
}

bool WhiteListService::CheckOptionOnorOff(String8 FeatureName, String8 AppcationName, String8 CallerApp)
{
    bool mApplicationExist = true;
    P_Feature pFeatureTemp;
    P_Application pApplicationTemp;

    ALOGV("Feature count is %d \n", mFeatureCount);
    strcpy(mFeature, FeatureName.string());
    strcpy(mApplication, AppcationName.string());
    strcpy(mCallerApp, CallerApp.string());
    for (int i = 0; i < mFeatureCount; i++) {
        pFeatureTemp = mFeatureList[i];
        ALOGV("Feature is %s \n", pFeatureTemp->_name);
        if (!strcmp(mFeature, pFeatureTemp->_name)) {
            mApplicationExist = false;
            for (int j= 0; j < pFeatureTemp->mApplicationCount; j++) {
                pApplicationTemp = pFeatureTemp->mApplicationList[j];
                ALOGV("application is %s \n", pApplicationTemp->_name);
                if (!strcmp(pApplicationTemp->_name, mApplication) || !strcmp(pApplicationTemp->_name, mCallerApp)) {
                    mApplicationExist = true;
                    CopyOptionList(pApplicationTemp);
                }
                if (!strcmp(pApplicationTemp->_name, ALLENABLE) || !strcmp(mApplication, ALLENABLE)) {
                    mApplicationExist = true;
                }
                if (!strcmp(pApplicationTemp->_name, ALLDISABLE) || !strcmp(mApplication, ALLDISABLE)) {
                    mApplicationExist = false;
                }
            }
        }
    }
    return mApplicationExist;
}

bool WhiteListService::GetOptionForApp(int32_t index, String8 *name, String8 *value)
{
    if (index < mOptionCount) {
        *name = mOptionList[index]->_name;
        *value = mOptionList[index]->_value;
        return true;
    }
    return false;
}

};

