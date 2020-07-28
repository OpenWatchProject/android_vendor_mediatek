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

#ifndef ANDROID_WLSERVICE_H
#define ANDROID_WLSERVICE_H

#include <utils/String8.h>
//#include "MtkWLParser.h"
#include "IMtkWLService.h"

#define ALLENABLE       "all"
#define ALLDISABLE      "all-off"

typedef struct _Option_ {
    char _name[256];
    char _value[256];
} Option;

typedef Option * P_Option;

typedef struct _Application_ {
    char        _name[256];
    int32_t     mOptionCount;
    P_Option    mOptionList[256];
} Application;

typedef Application * P_Application;

typedef struct _Feature_ {
    char        _name[256];
    int32_t     mApplicationCount;
    P_Application mApplicationList[256];
} Feature;

typedef Feature * P_Feature;

namespace android
{

class WhiteListService: public BnWhiteListService
{
    public:
        WhiteListService();
        ~WhiteListService();

        static void instantiate();
        void startElementHandler(const char *name, const char **attrs);
        void endElementHandler(const char *name);
        bool CheckOptionOnorOff(String8 FeatureName, String8 AppcationName, String8 CallerApp);
        bool GetOptionForApp(int32_t index, String8 *name, String8 *value);

    private:

        typedef enum Section {
            SECTION_TOPLEVEL = 0,
            SECTION_WHITELIST = 1,
            SECTION_FEATURE = 10,
            SECTION_APPLICATION = 100,
            SECTION_OPTION = 101,
        } Section;
        
        Section     mCurrentSection;
        int32_t     mDepth;
        
        char        mFeature[256];
        char        mApplication[256];
        char        mCallerApp[256];

        int32_t     mOptionCount;
        P_Option    mOptionList[256];
        P_Feature   mFeatureList[256];

        P_Option    pOptionTemp;
        int32_t     mFeatureCount;
        P_Feature   pFeatureTemp;


        void CreateOptionForApplication(const char *name);
        void AddOptionValue(const char *value);
        void CreateFeature(const char *name);
        void CreateApplicationForFeature(const char *name);
        void CopyOptionList(const P_Application pApplication);
        bool ParsingWhiteListConfig();
};

};
#endif
