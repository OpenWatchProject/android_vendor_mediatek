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

#ifndef __ECC_NUMBER_SOURCE_H__
#define __ECC_NUMBER_SOURCE_H__

/*****************************************************************************
 * Include
 *****************************************************************************/
#include <string>
#include <vector>
#include <telephony/mtk_ril.h>
#include <utils/String8.h>

#define ECC_PATH           "/system/vendor/etc/"
#define ECC_OM_FILE_PATH   "/system/vendor/etc/ecc_list.xml"
#define ECC_TABLE          "EccTable"
#define ECC_ENTRY          "EccEntry"
#define ECC_NUMBER         "Ecc"
#define ECC_CATEGORY       "Category"
#define ECC_CONDITION      "Condition"
#define ECC_PLMN           "Plmn"
#define ECC_DEFAULT        "112,911"
#define ECC_DEFAULT_NO_SIM "112,911,000,08,110,118,119,999"
#define ECC_CTA            "110,119,120,122"
#define ECC_OPERATOR_PROP  "persist.vendor.operator.optr"

#define PROPERTY_ECC_COUNT           "ro.vendor.semc.ecclist.num"
#define PROPERTY_ECC_NUMBER          "ro.vendor.semc.ecclist.number."
#define PROPERTY_ECC_TYPE            "ro.vendor.semc.ecclist.type."
#define PROPERTY_ECC_PLMN            "ro.vendor.semc.ecclist.plmn."
#define PROPERTY_ECC_CONDITION       "ro.vendor.semc.ecclist.non_ecc."
#define PROPERTY_ECC_TYPE_KEY_POLICE         "police"
#define PROPERTY_ECC_TYPE_KEY_AMBULANCE      "ambulance"
#define PROPERTY_ECC_TYPE_KEY_FIRE_BRIGADE   "firebrigade"
#define PROPERTY_ECC_TYPE_KEY_MARINE_GUARD   "marineguard"
#define PROPERTY_ECC_TYPE_KEY_MOUTAIN_RESCUE "mountainrescue"
#define PROPERTY_ECC_TEST                    "persist.vendor.em.hidl.testecc"

#define ECC_DEBUG (0)

#define ECC_CATEGORY_NOT_DEFINED (-1)

using namespace std;
using namespace android;

static const char PROPERTY_ECC_LIST[MAX_SIM_COUNT][MAX_PROP_CHARS] = {
    "ril.ecclist",
    "ril.ecclist1",
    "ril.ecclist2",
    "ril.ecclist3",
};

static const char PROPERTY_SPECIAL_ECC_LIST[MAX_SIM_COUNT][MAX_PROP_CHARS] = {
    "vendor.ril.special.ecclist",
    "vendor.ril.special.ecclist1",
    "vendor.ril.special.ecclist2",
    "vendor.ril.special.ecclist3",
};

static const char PROPERTY_NW_ECC_LIST[MAX_SIM_COUNT][MAX_PROP_CHARS] = {
    "vendor.ril.ecc.service.category.list",
    "vendor.ril.ecc.service.category.list.1",
    "vendor.ril.ecc.service.category.list.2",
    "vendor.ril.ecc.service.category.list.3",
};

static const char PROPERTY_NW_ECC_MCC[MAX_SIM_COUNT][MAX_PROP_CHARS] = {
    "vendor.ril.ecc.service.category.mcc",
    "vendor.ril.ecc.service.category.mcc.1",
    "vendor.ril.ecc.service.category.mcc.2",
    "vendor.ril.ecc.service.category.mcc.3",
};

typedef enum {
    CONDITION_NO_SIM   = 0, // Only ECC when NO SIM
    CONDITION_ALWAYS   = 1, // Always ECC
    CONDITION_MMI      = 2, // Fake/Special ECC (normal call setup flow)
    CONDITION_SIM_ONLY = 3  // Only ECC when SIM insert
} EmergencyNumberCondition;

typedef enum {
    /**
     * Indicates the number is from the network signal.
     */
    SOURCE_NETWORK      = 1 << 0,
    /**
     * Indicates the number is from the sim card.
     */
    SOURCE_SIM          = 1 << 1,
    /**
     * Indicates the number is from the XML config.
     * Will mapping to AOSP source type MODEM_CONFIG
     */
    SOURCE_CONFIG       = 1 << 2,
    /**
     * Indicates the number is available as default. Per the reference, 112, 911 must always be
     * available; additionally, 000, 08, 110, 999, 118 and 119 must be available when sim is not
     * present.
     */
    SOURCE_DEFAULT      = 1 << 3,
    /**
     * Indicates the number is from the OEM property.
     * Will mapping to AOSP source type MODEM_CONFIG
     */
    SOURCE_OEM_PROPERTY = 1 << 4,
    /**
     * Indicates the number is from framework
     * Will mapping to AOSP source type MODEM_CONFIG
     */
    SOURCE_FRAMEWORK    = 1 << 5,
    /**
     * Indicates the number is from test property which add through
     * Engineer Mode -> Telephony -> Emergency Numbers
     * Will mapping to AOSP source type MODEM_CONFIG
     */
    SOURCE_TEST         = 1 << 6,
    /**
     * Indicates the number is from China CTA requirement
     * Only add when no SIM insert and China PLMN detected on other slot
     * Will mapping to AOSP source type MODEM_CONFIG
     */
    SOURCE_CTA         = 1 << 7
} EmergencyNumberSource;

struct EmergencyNumber{
    string number;
    string mcc;
    string mnc;
    int categories;
    int condition;
    int sources;
};

class EccNumberSource {
public:
    EccNumberSource(int slotId);
    virtual ~EccNumberSource();

// Override
public:

    // Add emergency number list to whole list
    //
    // RETURNS:
    virtual void addToEccList(vector<EmergencyNumber> &list);

protected:
    bool findAndMerge(EmergencyNumber number, vector<EmergencyNumber> &list);
    bool isSimilar(EmergencyNumber number1, EmergencyNumber number2);
    bool addEmergencyNumber(EmergencyNumber ecc, string plmn, bool isSimInsert);
    int getPriority(EmergencyNumber number);

protected:
    vector<EmergencyNumber> mEccList;
    int mSlotId;
};

class SimEccNumberSource : public EccNumberSource {
public:
    SimEccNumberSource(int slotId);
    virtual ~SimEccNumberSource();

    // Update SIM ECC list
    //
    // RETURNS: true if SIM ECC changes
    bool update(string gsmEccList, string cdmaEccList);
};

class NetworkEccNumberSource : public EccNumberSource {
public:
    NetworkEccNumberSource(int slotId);
    virtual ~NetworkEccNumberSource();

    // Update network ECC list
    //
    // RETURNS: true if ECC changes
    bool update();

    // Clear network ECC list
    //
    // RETURNS:
    void clear();
};

class DefaultEccNumberSource : public EccNumberSource {
public:
    DefaultEccNumberSource(int slotId);
    virtual ~DefaultEccNumberSource();

    // Update default ECC list when SIM status changes
    //
    // RETURNS: true if ECC changes
    bool update(bool isSimInsert);
};

class XmlEccNumberSource : public EccNumberSource {
public:
    XmlEccNumberSource(int slotId);
    virtual ~XmlEccNumberSource();

    // Update XML ECC list when PLMN changes or SIM status changes
    //
    // RETURNS: true if ECC changes
    bool update(string plmn, bool isSimInsert);

private:
    string getConfigFilePath();
};

// ECC number from framework
class FrameworkEccNumberSource : public EccNumberSource {
public:
    FrameworkEccNumberSource(int slotId);
    virtual ~FrameworkEccNumberSource();

    // Set framework ECC lis
    //
    // RETURNS: void
    void set(const char *eccWithSim, const char *eccNoSim);

    // Update framework ECC list SIM status changes
    //
    // RETURNS: true if ECC changes
    bool update(bool isSimInsert);

private:
    String8 mEccListWithCard;
    String8 mEccListNoCard;
};

// ECC number from OEM property
class OemPropertyEccNumberSource : public EccNumberSource {
public:
    OemPropertyEccNumberSource(int slotId);
    virtual ~OemPropertyEccNumberSource();

    // Update OEM property ECC list
    //
    // RETURNS: true if ECC changes
    bool update(string currentPlmn, bool isSimInsert);
};

// ECC number from EM test
class TestEccNumberSource : public EccNumberSource {
public:
    TestEccNumberSource(int slotId);
    virtual ~TestEccNumberSource();

    // Update test ECC list
    //
    // RETURNS: true if ECC changes
    bool update(bool isSimInsert);
};

// ECC number from CTA requirement
class CtaEccNumberSource : public EccNumberSource {
public:
    CtaEccNumberSource(int slotId);
    virtual ~CtaEccNumberSource();

    // Update CTA ECC list
    //
    // RETURNS: true if ECC changes
    bool update(bool isNeed);
};

#endif /* __ECC_NUMBER_SOURCE_H__ */
