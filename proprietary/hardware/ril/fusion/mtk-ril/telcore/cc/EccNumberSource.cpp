/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
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

/*****************************************************************************
 * Include
 *****************************************************************************/
#include "EccNumberSource.h"
#include "tinyxml.h"
#include <libmtkrilutils.h>
#include "RfxLog.h"
#include "rfx_properties.h"

#define RFX_LOG_TAG "EccNumberSource"

#define MAX_PROP_CHARS       50
#define MCC_CHAR_LEN         3
#define PLMN_CHAR_LEN        6
#define PLMN_CHAR_MIN_LEN    5
#define MAX_ECC_NUM          16
#define MAX_ECC_BUF_SIZE     (MAX_ECC_NUM * 8 + 1)
#define MAX_REGION_SIZE      10

typedef struct {
    string name;    // Region name
    string mccList; // Mcc list in this region separate by ',', Ex: "440,460"
} Region;

// Customize region here
static Region sRegionTable[MAX_REGION_SIZE] = {
    {"APAC", "505"}, // Australia
    {"LATAM", "724"},// Brazilian
    {"EMEA", "234"}  // UK
};

/*****************************************************************************
 * Class EccNumberSource
 *****************************************************************************/
EccNumberSource::EccNumberSource(int slotId) {
    mSlotId = slotId;
    mEccList.clear();
}

EccNumberSource::~EccNumberSource() {
}

bool EccNumberSource::addEmergencyNumber(EmergencyNumber ecc, string plmn, bool isSimInsert) {
    if (ECC_DEBUG == 1) {
        RFX_LOG_V(RFX_LOG_TAG, "[%d][%s] current PLMN: %s, eccNumber:[%s,%s,%s,%d,%d]",
                mSlotId, __FUNCTION__, plmn.c_str(),
                ecc.number.c_str(),
                ecc.mcc.c_str(),
                ecc.mnc.c_str(),
                ecc.categories,
                ecc.condition);
    }

    // PLMN match for:
    // 1. Specific PLMN ECC
    // 2. MCC ECC for one country
    // 3. Common ECC (No PLMN info in xml)
    // And condition matches
    if (((plmn == (ecc.mcc + ecc.mnc)) ||
            ((ecc.mnc == "FF" || ecc.mnc == "FFF") &&
            ecc.mcc == plmn.substr(0, MCC_CHAR_LEN)) ||
            (ecc.mcc == "" && ecc.mnc == "")) &&
            ((isSimInsert && ecc.condition > 0) ||
            (!isSimInsert && ecc.condition != CONDITION_SIM_ONLY))) {
        if (ECC_DEBUG == 1) {
            RFX_LOG_V(RFX_LOG_TAG, "[%d][%s] eccNumber match, add: %s", mSlotId, __FUNCTION__,
                ecc.number.c_str());
        }
        mEccList.push_back(ecc);
        return true;
    }
    return false;
}

void EccNumberSource::addToEccList(vector<EmergencyNumber> &list) {
    for (int i = 0; i < (int)mEccList.size(); i++) {
        if (!findAndMerge(mEccList[i], list)) {
            list.push_back(mEccList[i]);
        }
    }
}

bool EccNumberSource::findAndMerge(EmergencyNumber number, vector<EmergencyNumber> &list) {
    for (int i = 0; i < (int)list.size(); i++) {
        if (isSimilar(number, list[i])) {
            if (getPriority(number) > getPriority(list[i])) {
                list[i].categories = number.categories;
                list[i].condition = number.condition;
                list[i].mcc = number.mcc;
                list[i].mnc = number.mnc;
            } else if (getPriority(number) == getPriority(list[i])) {
                // Operator specific ECC has higher priperity then country ECC
                if (list[i].mcc == number.mcc &&
                        number.mnc != "" && number.mnc != "FFF" && number.mnc != "FF") {
                    list[i].categories = number.categories;
                    list[i].condition = number.condition;
                    list[i].mnc = number.mnc;
                }
            } else {
                // Over CDMA SIM ECC service category by XML configure
                if (((number.sources & SOURCE_CONFIG)
                        && (number.categories != ECC_CATEGORY_NOT_DEFINED))
                        && ((list[i].sources & SOURCE_SIM)
                        && (list[i].categories == ECC_CATEGORY_NOT_DEFINED))) {
                    list[i].categories = number.categories;
                }
            }

            // merge sources
            list[i].sources |= number.sources;
            return true;
        }
    }
    return false;
}

bool EccNumberSource::isSimilar(EmergencyNumber number1, EmergencyNumber number2) {
    // Currently only check if number is equal, may also consider other parameters
    if (number1.number == number2.number) {
        return true;
    } else {
        return false;
    }
}

int EccNumberSource::getPriority(EmergencyNumber number) {
    int priority = 0;
    if (number.sources & SOURCE_NETWORK) {
        priority += 1 << 3;
    }
    if (number.sources & SOURCE_SIM) {
        priority += 1 << 2;
    }
    if (number.sources & SOURCE_CONFIG ||
            number.sources & SOURCE_OEM_PROPERTY ||
            number.sources & SOURCE_FRAMEWORK ||
            number.sources & SOURCE_TEST ||
            number.sources & SOURCE_CTA) {
        priority += 1 << 1;
    }
    if (number.sources & SOURCE_DEFAULT) {
        priority += 1 << 0;
    }
    return priority;
}

/*****************************************************************************
 * Class SimEccNumberSource
 *****************************************************************************/
SimEccNumberSource::SimEccNumberSource(int slotId) : EccNumberSource(slotId) {

}

SimEccNumberSource::~SimEccNumberSource() {
}

bool SimEccNumberSource::update(string gsmEccList, string cdmaEccList) {
    char eccList[MAX_ECC_BUF_SIZE] = {0};
    char* ecc = NULL;
    char* eccCategory = NULL;

    mEccList.clear();

    if (gsmEccList != "") {
        strncpy(eccList, (const char *)gsmEccList.c_str(), MAX_ECC_BUF_SIZE - 1);
        if (strlen(eccList) > 0) {
            ecc = strtok(eccList, ",;");
            while (ecc != NULL) {
                eccCategory = strtok(NULL, ",;");
                if (eccCategory != NULL) {
                    EmergencyNumber en;
                    en.number = ecc;
                    en.mcc = "";
                    en.mnc = "";
                    en.categories = atoi(eccCategory);
                    en.sources = SOURCE_SIM;
                    en.condition = CONDITION_ALWAYS;
                    mEccList.push_back(en);
                }
                ecc = strtok(NULL, ",;");
            }
        }
    }
    if (cdmaEccList != "") {
        strncpy(eccList, (const char *)cdmaEccList.c_str(), MAX_ECC_BUF_SIZE - 1);
        if (strlen(eccList) > 0) {
            ecc = strtok(eccList, ",");
            while (ecc != NULL) {
                EmergencyNumber en;
                en.number = ecc;
                en.mcc = "";
                en.mnc = "";
                // CDMA ECC don't have service category
                en.categories = ECC_CATEGORY_NOT_DEFINED;
                en.sources = SOURCE_SIM;
                en.condition = CONDITION_ALWAYS;
                mEccList.push_back(en);
                ecc = strtok(NULL, ",");
            }
        }
    }

    return true;
}

/*****************************************************************************
 * Class NetworkEccNumberSource
 *****************************************************************************/
NetworkEccNumberSource::NetworkEccNumberSource(int slotId) : EccNumberSource(slotId) {
}

NetworkEccNumberSource::~NetworkEccNumberSource() {
}

bool NetworkEccNumberSource::update() {
    char eccList[RFX_PROPERTY_VALUE_MAX] = {0};
    char* ecc = NULL;
    char* eccCategory = NULL;

    mEccList.clear();

    // get from Network ECC
    rfx_property_get(PROPERTY_NW_ECC_LIST[mSlotId], eccList, "");
    if (strlen(eccList) > 0) {
        ecc = strtok(eccList, ",;");
        while (ecc != NULL) {
            eccCategory = strtok(NULL, ",;");
            if (eccCategory != NULL) {
                EmergencyNumber en;
                en.number = ecc;
                en.mcc = "";
                en.mnc = "";
                en.categories = atoi(eccCategory);
                en.sources = SOURCE_NETWORK;
                en.condition = CONDITION_ALWAYS;
                mEccList.push_back(en);
            }
            ecc = strtok(NULL, ",;");
        }
    }
    return true;
}

void NetworkEccNumberSource::clear() {
    mEccList.clear();
}

/*****************************************************************************
 * Class DefaultEccNumberSource
 *****************************************************************************/
DefaultEccNumberSource::DefaultEccNumberSource(int slotId) : EccNumberSource(slotId) {
}

DefaultEccNumberSource::~DefaultEccNumberSource() {
}

bool DefaultEccNumberSource::update(bool isSimInsert) {
    char eccList[MAX_ECC_BUF_SIZE] = {0};
    char* ecc = NULL;

    mEccList.clear();

    if (isSimInsert) {
        strncpy(eccList, (const char *)ECC_DEFAULT, MAX_ECC_BUF_SIZE - 1);
    } else {
        strncpy(eccList, (const char *)ECC_DEFAULT_NO_SIM, MAX_ECC_BUF_SIZE - 1);
    }

    ecc = strtok(eccList, ",");
    while (ecc != NULL) {
        EmergencyNumber en;
        en.number = ecc;
        en.mcc = "";
        en.mnc = "";
        en.categories = ECC_CATEGORY_NOT_DEFINED;
        en.sources = SOURCE_DEFAULT;
        en.condition = CONDITION_ALWAYS;
        mEccList.push_back(en);

        ecc = strtok(NULL, ",");
    }
    return true;
}

/*****************************************************************************
 * Class XmlEccNumberSource
 *****************************************************************************/
XmlEccNumberSource::XmlEccNumberSource(int slotId) : EccNumberSource(slotId) {
}

XmlEccNumberSource::~XmlEccNumberSource() {
}

bool XmlEccNumberSource::update(string plmn, bool isSimInsert) {
    char maskPlmn[PLMN_CHAR_LEN + 1] = {0};
    if (plmn.length() >= PLMN_CHAR_MIN_LEN) {
        maskString((char*)plmn.c_str(), maskPlmn, 2, '*');
    }
    RFX_LOG_D(RFX_LOG_TAG, "[XML][%d][%s] START for plmn: %s, isSimInsert: %d", mSlotId,
            __FUNCTION__, maskPlmn, isSimInsert);

    // Skip update for invalid PLMN case
    if ("FFFFFF" == plmn) {
        return false;
    }

    mEccList.clear();

    TiXmlDocument eccXml;
    string filePath = getConfigFilePath();
    if (!eccXml.LoadFile(filePath.c_str(), TIXML_ENCODING_UTF8)) {
        RFX_LOG_E(RFX_LOG_TAG, "[XML][%d][%s] Load ecc xml error", mSlotId, __FUNCTION__);
        return false;
    }

    TiXmlElement* eccs = eccXml.FirstChildElement(ECC_TABLE);
    if (eccs == NULL) {
        RFX_LOG_E(RFX_LOG_TAG, "[XML][%d][%s] eccs element is NULL", mSlotId, __FUNCTION__);
        return false;
    }

    TiXmlElement* elem = eccs->FirstChildElement(ECC_ENTRY);
    while (elem) {
        TiXmlAttribute* attr = elem->FirstAttribute();
        if (attr == NULL) {
            RFX_LOG_E(RFX_LOG_TAG, "[XML][%d][%s] attr is NULL", mSlotId, __FUNCTION__);
            continue;
        }

        string attrName = "";
        string attrValue = "";
        EmergencyNumber eccNumber = {"", "", "", 0, 0, SOURCE_CONFIG};
        while (attr) {
            attrName = attr->Name();
            attrValue = attr->Value();
            if (attrName == ECC_NUMBER) {
                eccNumber.number = attrValue;
            } else if (attrName == ECC_CATEGORY) {
                eccNumber.categories = atoi(attrValue.c_str());
            } else if (attrName == ECC_CONDITION) {
                int condition = atoi(attrValue.c_str());
                if (!isSimInsert && condition == CONDITION_MMI) {
                    // Convert to ECC when no SIM
                    eccNumber.condition = CONDITION_NO_SIM;
                } else {
                    eccNumber.condition = condition;
                }
            } else if (attrName == ECC_PLMN) {
                // Support customize ECC by region
                string mccList = getMccList(attrValue);
                if (mccList.length() > 0) {
                    if (plmn.length() > MCC_CHAR_LEN &&
                            mccList.find(plmn.substr(0, MCC_CHAR_LEN)) != string::npos) {
                        // Found mcc in region, replace with current PLMN
                        eccNumber.mcc = plmn.substr(0, MCC_CHAR_LEN);
                        eccNumber.mnc = plmn.substr(MCC_CHAR_LEN, string::npos);
                    } else {
                        // Not found, Keep as region name
                        eccNumber.mcc = attrValue;
                    }
                } else {
                    if (attrValue.length() >= PLMN_CHAR_LEN) {
                        eccNumber.mcc = attrValue.substr(0, MCC_CHAR_LEN);
                        // +1 for skip blank in xml (Ex: 460 11)
                        eccNumber.mnc = attrValue.substr(MCC_CHAR_LEN + 1, string::npos);
                    }
                }
            }
            attr = attr->Next();
        }

        addEmergencyNumber(eccNumber, plmn, isSimInsert);

        elem = elem->NextSiblingElement(ECC_ENTRY);
    }

    if (ECC_DEBUG == 1) {
        RFX_LOG_V(RFX_LOG_TAG, "[XML][%d][%s] END size: %d", mSlotId, __FUNCTION__,
                (int)mEccList.size());
    }
    return true;
}

string XmlEccNumberSource::getConfigFilePath() {
    char optr[RFX_PROPERTY_VALUE_MAX] = {0};
    string filePath = string(ECC_OM_FILE_PATH);
    rfx_property_get(ECC_OPERATOR_PROP, optr, "");
    if (strlen(optr) > 0) {
        filePath = string(ECC_PATH) + string("ecc_list_") + string(optr) + string(".xml");
    }

    // Check if exist
    FILE* file = fopen(filePath.c_str(), "r");
    if (file == NULL) {
        // Use default xml
        filePath = string(ECC_OM_FILE_PATH);
    } else {
        fclose(file);
    }

    if (ECC_DEBUG == 1) {
        RFX_LOG_V(RFX_LOG_TAG, "[XML][%d][%s] optr: %s, ECC filePath: %s", mSlotId, __FUNCTION__,
                optr, filePath.c_str());
    }
    return filePath;
}

string XmlEccNumberSource::getMccList(string region) {
    for (int i = 0; i < MAX_REGION_SIZE; i++) {
        if (region == sRegionTable[i].name) {
            RFX_LOG_D(RFX_LOG_TAG, "[XML][%d][%s] found mccList: %s for %s",
                    mSlotId, __FUNCTION__, sRegionTable[i].mccList.c_str(),
                    sRegionTable[i].name.c_str());
            return sRegionTable[i].mccList;
        }
    }
    return "";
}

/*****************************************************************************
 * Class FrameworkEccNumberSource
 *****************************************************************************/
FrameworkEccNumberSource::FrameworkEccNumberSource(int slotId) : EccNumberSource(slotId) {
    mEccListWithCard = String8("");
    mEccListNoCard = String8("");
}

FrameworkEccNumberSource::~FrameworkEccNumberSource() {
}

void FrameworkEccNumberSource::set(const char *eccWithSim, const char *eccNoSim) {
    if (eccWithSim != NULL) {
        mEccListWithCard = String8(eccWithSim);
    }
    if (eccNoSim != NULL) {
        mEccListNoCard = String8(eccNoSim);
    }
}

bool FrameworkEccNumberSource::update(bool isSimInsert) {
    mEccList.clear();

    String8 eccListString = isSimInsert ? mEccListWithCard : mEccListNoCard;
    char* eccList = (char *)strdup(eccListString.string());
    char* eccCategory = NULL;
    char* ecc = NULL;
    if (eccList != NULL) {
        if (strlen(eccList) > 0) {
            ecc = strtok(eccList, ",");
            while (ecc != NULL) {
                EmergencyNumber en;
                en.number = ecc;
                en.mcc = "";
                en.mnc = "";
                en.categories = ECC_CATEGORY_NOT_DEFINED;
                en.sources = SOURCE_FRAMEWORK;
                en.condition = CONDITION_ALWAYS;

                // Support setting emergency number with category,
                // the format is <service category>+<number>, e.g. 1+110
                char *plusChar = strchr(ecc, '+');
                if (plusChar != NULL) {
                    eccCategory = ecc;
                    *plusChar = '\0';
                    ecc = plusChar + 1;
                    en.categories = atoi(eccCategory);
                    en.number = string(ecc);
                }
                mEccList.push_back(en);
                ecc = strtok(NULL, ",");
            }
        }
        free(eccList);
    }

    return true;
}

/*****************************************************************************
 * Class OemPropertyEccNumberSource
 *****************************************************************************/
OemPropertyEccNumberSource::OemPropertyEccNumberSource(int slotId) : EccNumberSource(slotId) {
}

OemPropertyEccNumberSource::~OemPropertyEccNumberSource() {
}

bool OemPropertyEccNumberSource::update(string currentPlmn, bool isSimInsert) {
    mEccList.clear();

    RFX_LOG_I(RFX_LOG_TAG, "[Property][%d][%s] START for plmn: %s, isSimInsert: %d", mSlotId,
            __FUNCTION__, currentPlmn.c_str(), isSimInsert);

    char eccCount[MAX_PROP_CHARS] = {0};
    char eccNumber[MAX_PROP_CHARS] = {0};
    char eccType[MAX_PROP_CHARS] = {0};
    char eccCondition[MAX_PROP_CHARS] = {0};
    char eccPlmn[MAX_PROP_CHARS] = {0};
    char propertyNumber[MAX_PROP_CHARS] = {0};
    char propertyType[MAX_PROP_CHARS] = {0};
    char propertyCondition[MAX_PROP_CHARS] = {0};
    char propertyPlmn[MAX_PROP_CHARS] = {0};
    rfx_property_get(PROPERTY_ECC_COUNT, eccCount, "0");
    for (int i = 0; i < atoi(eccCount); i++) {
        // Get ECC number from property
        snprintf(propertyNumber, MAX_PROP_CHARS, "%s%d", PROPERTY_ECC_NUMBER, i);
        rfx_property_get(propertyNumber, eccNumber, "");
        if (strlen(eccNumber) > 0) {
            EmergencyNumber en;
            en.number = string(eccNumber);
            en.mcc = "";
            en.mnc = "";
            en.categories = ECC_CATEGORY_NOT_DEFINED;
            en.sources = SOURCE_OEM_PROPERTY;
            en.condition = CONDITION_ALWAYS;

            // Get ECC type from property
            snprintf(propertyType, MAX_PROP_CHARS, "%s%d", PROPERTY_ECC_TYPE, i);
            rfx_property_get(propertyType, eccType, "");
            if (strlen(eccType) > 0) {
                string type = string(eccType);
                int category = 0;
                if (type.find(PROPERTY_ECC_TYPE_KEY_POLICE) != string::npos) {
                    category |= POLICE;
                }
                if (type.find(PROPERTY_ECC_TYPE_KEY_AMBULANCE) != string::npos) {
                    category |= AMBULANCE;
                }
                if (type.find(PROPERTY_ECC_TYPE_KEY_FIRE_BRIGADE) != string::npos) {
                    category |= FIRE_BRIGADE;
                }
                if (type.find(PROPERTY_ECC_TYPE_KEY_MARINE_GUARD) != string::npos) {
                    category |= MARINE_GUARD;
                }
                if (type.find(PROPERTY_ECC_TYPE_KEY_MOUTAIN_RESCUE) != string::npos) {
                    category |= MOUNTAIN_RESCUE;
                }
                en.categories = category;
            }

            // Get ECC condition from property
            snprintf(propertyCondition, MAX_PROP_CHARS, "%s%d", PROPERTY_ECC_CONDITION, i);
            rfx_property_get(propertyCondition, eccCondition, "false");
            if (strcmp(eccCondition, "true") == 0) {
                en.condition = CONDITION_NO_SIM;
            }

            // Get ECC PLMN from property
            snprintf(propertyPlmn, MAX_PROP_CHARS, "%s%d", PROPERTY_ECC_PLMN, i);
            rfx_property_get(propertyPlmn, eccPlmn, "");
            if (strlen(eccPlmn) > PLMN_CHAR_LEN) {
                string plmn = string(eccPlmn);
                en.mcc = plmn.substr(0, MCC_CHAR_LEN);
                // +1 for skip blank in xml (Ex: 460 11)
                en.mnc = plmn.substr(MCC_CHAR_LEN + 1, string::npos);
            }

            addEmergencyNumber(en, currentPlmn, isSimInsert);
        }
    }

    return true;
}

/*****************************************************************************
 * Class TestEccNumberSource
 *****************************************************************************/
TestEccNumberSource::TestEccNumberSource(int slotId) : EccNumberSource(slotId) {
}

TestEccNumberSource::~TestEccNumberSource() {
}

bool TestEccNumberSource::update(bool isSimInsert) {
    if (ECC_DEBUG == 1) {
        RFX_LOG_V(RFX_LOG_TAG, "[TEST][%d][%s] isSimInsert: %d",
                mSlotId, __FUNCTION__, isSimInsert);
    }
    if (!isSimInsert) {
        // Test ECC only works when SIM insert
        return false;
    }

    char eccList[MAX_PROP_CHARS] = {0};
    char* ecc = NULL;

    mEccList.clear();

    // get from Test ECC
    rfx_property_get(PROPERTY_ECC_TEST, eccList, "");
    if (strlen(eccList) > 0) {
        ecc = strtok(eccList, ",;");
        while (ecc != NULL) {
            EmergencyNumber en;
            en.number = ecc;
            en.mcc = "";
            en.mnc = "";
            en.categories = ECC_CATEGORY_NOT_DEFINED;
            en.sources = SOURCE_TEST;
            en.condition = CONDITION_ALWAYS;
            mEccList.push_back(en);

            ecc = strtok(NULL, ",;");
        }
    }
    return true;
}

/*****************************************************************************
 * Class CtaEccNumberSource
 *****************************************************************************/
CtaEccNumberSource::CtaEccNumberSource(int slotId) : EccNumberSource(slotId) {
}

CtaEccNumberSource::~CtaEccNumberSource() {
}

bool CtaEccNumberSource::update(bool isNeed) {
    char eccList[MAX_ECC_BUF_SIZE] = {0};
    char* ecc = NULL;

    mEccList.clear();

    if (isNeed) {
        strncpy(eccList, (const char *)ECC_CTA, MAX_ECC_BUF_SIZE - 1);
        ecc = strtok(eccList, ",");
        while (ecc != NULL) {
            EmergencyNumber en;
            en.number = ecc;
            en.mcc = "";
            en.mnc = "";
            en.categories = ECC_CATEGORY_NOT_DEFINED;
            en.sources = SOURCE_CTA;
            en.condition = CONDITION_ALWAYS;
            mEccList.push_back(en);

            ecc = strtok(NULL, ",");
        }
    }
    return true;
}