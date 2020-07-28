/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#include "SecurityAssociation.h"
#include "Logs.h"

using com::mediatek::diagnostic::SecurityAssociation;

static const char * TAG = "PKM-SA";

SecurityAssociation::SecurityAssociation() : mMutex(PTHREAD_MUTEX_INITIALIZER) {
    PKMM_LOGD("SecurityAssociation");
}

SecurityAssociation::~SecurityAssociation() {
    PKMM_LOGD("SecurityAssociation deleted");
    pthread_mutex_lock(&mMutex);
    for (int i = 0, size = mSecurityInfoList.size(); i < size; i++) {
        delete mSecurityInfoList[i];
    }
    mSecurityInfoList.clear();
    pthread_mutex_unlock(&mMutex);
}

/***************************************************************************************************
 * Make a ESP Secret String and Fill them to buffer
 * @param encType Encryption Type
 * @param buffer String buffer to write ESP secret
 *
 **************************************************************************************************/
bool SecurityAssociation::findEspSecret(EncryptionType encType, char *buf, int len,
                                        int *encBlockSize, int *authSize,
                                        char *spi1, char *spi2, int initiator) {

    PKMM_LOGD("findEspSecret: spi1: %s, spi2: %s", spi1, spi2);
    if(encType == EncryptionTypeIKev2) {
        pthread_mutex_lock(&mMutex);
        for (int i = 0, size = mSecurityInfoList.size(); i < size; i++) {
            if (mSecurityInfoList[i]->encType == EncryptionTypeIKev2) {
                if (!strcasecmp(mSecurityInfoList[i]->data.ikev2.spi_i, spi1) &&
                   ((spi2[2] == '0') || !strcasecmp(mSecurityInfoList[i]->data.ikev2.spi_r, spi2)))
                {
                    IKev2Info *ik2 = &(mSecurityInfoList[i]->data.ikev2);
                    if (initiator) {
                        snprintf(buf, len, "ikev2 I 0x%s 0x%s %s:0x%s %s:0x%s", ik2->spi_i,
                                                                                ik2->spi_r,
                                                                                ik2->int_algo_ikev2,
                                                                                ik2->sk_ai,
                                                                                ik2->enc_algo_name,
                                                                                ik2->sk_ei);
                    } else {
                        snprintf(buf, len, "ikev2 R 0x%s 0x%s %s:0x%s %s:0x%s", ik2->spi_i,
                                                                                ik2->spi_r,
                                                                                ik2->int_algo_ikev2,
                                                                                ik2->sk_ar,
                                                                                ik2->enc_algo_name,
                                                                                ik2->sk_er);
                    }

                    pthread_mutex_unlock(&mMutex);
                    return true;
                }
            }
        }
        pthread_mutex_unlock(&mMutex);
    }
    else if(encType == EncryptionTypeEsp || encType == EncryptionTypeEspImc) {

        pthread_mutex_lock(&mMutex);
        for (int i = 0, size = mSecurityInfoList.size(); i < size; i++)
        {
            if (mSecurityInfoList[i]->encType == EncryptionTypeEsp ||
                mSecurityInfoList[i]->encType == EncryptionTypeEspImc)
            {
                if (!strcasecmp(mSecurityInfoList[i]->data.esp.spi, spi1)) {
                    EspInfo *esp = &(mSecurityInfoList[i]->data.esp);
                    snprintf(buf, len, "0x%s@%s %s-%s:0x%s", esp->spi, esp->dst_addr,
                                                             esp->enc_algo_name,
                                                             esp->int_algo_esp,
                                                             esp->enc_key);

                    *authSize = esp->auth_block;
                    *encBlockSize = esp->block;

                    pthread_mutex_unlock(&mMutex);
                    return true;
                }
            }
        }
        pthread_mutex_unlock(&mMutex);
    }

    return false;
}

/***************************************************************************************************
 * Search SecurityInfo by Index and Remove it
 * @param encType Encryption Type
 * @param index Index from MDM message
 *
 **************************************************************************************************/
void SecurityAssociation::removeInfoByIndex(EncryptionType encType, int index) {

    PKMM_LOGD("removeInfoByIndex: type: %d, index: %d", (int)encType, index);

    // Remove On Delete Items
    removeOnDelete();

    if (EncryptionTypeEspImc == encType) {
        pthread_mutex_lock(&mMutex);
        for (int i = 0, size = mSecurityInfoList.size(); i < size; i++) {
            if(mSecurityInfoList[i]->encType == encType &&
                mSecurityInfoList[i]->data.esp.index == index) {
                delete mSecurityInfoList[i];
                mSecurityInfoList.erase(mSecurityInfoList.begin() + i);
                PKMM_LOGD("removeInfoByIndex: type: %d, index: %d removed", (int)encType, index);
                break;
            }
        }
        pthread_mutex_unlock(&mMutex);
    }
}

/***************************************************************************************************
 * Workaround for add/del key in a short time
 * @param encType Encryption Type
 * @param spi1 SPI
 * @param spi2 SPI
 *
 **************************************************************************************************/
 void SecurityAssociation::removeOnDelete() {

    pthread_mutex_lock(&mMutex);

    // Remove item which marked as on delete
    if(mSecurityInfoList.size() > 0) {
        for (int i = mSecurityInfoList.size() - 1 ; i >= 0; i--) {
            if(mSecurityInfoList[i]->onDelete) {
                delete mSecurityInfoList[i];
                mSecurityInfoList.erase(mSecurityInfoList.begin() + i);
                PKMM_LOGD("Remove item(%d) marked as delete", i);
            }
        }
    }

    pthread_mutex_unlock(&mMutex);
 }

/***************************************************************************************************
 * Search SecurityInfo by SPI and Remove it
 * @param encType Encryption Type
 * @param spi1 SPI
 * @param spi2 SPI
 *
 **************************************************************************************************/
void SecurityAssociation::removeInfoBySpi(EncryptionType encType, char *spi1, char *spi2) {

    PKMM_LOGD("removeInfoBySpi: type: %d, spi1: %s, spi2: %s", (int)encType, spi1, spi2);

    // Remove On Delete Items
    removeOnDelete();

    pthread_mutex_lock(&mMutex);
    if (EncryptionTypeIKev2 == encType) {
        for (int i = 0, size = mSecurityInfoList.size(); i < size; i++) {
            if(mSecurityInfoList[i]->encType == encType &&
                !strcmp(mSecurityInfoList[i]->data.ikev2.spi_i, spi1) &&
                !strcmp(mSecurityInfoList[i]->data.ikev2.spi_r, spi2))
            {
                if(mSecurityInfoList[i]->onDelete) {
                    delete mSecurityInfoList[i];
                    mSecurityInfoList.erase(mSecurityInfoList.begin() + i);
                    PKMM_LOGD("Remove: type: %d, spi1: %s, spi2: %s done", (int)encType, spi1, spi2);
                }
                else {
                    // Mark this item as on delete (Item will be deleted by next remove event)
                    mSecurityInfoList[i]->onDelete = true;
                    PKMM_LOGD("Marked: type: %d, spi1: %s, spi2: %s done", (int)encType, spi1, spi2);
                }
                break;
            }
        }
    }
    else if(EncryptionTypeEsp == encType) {
        for (int i = 0, size = mSecurityInfoList.size(); i < size; i++) {
            if(mSecurityInfoList[i]->encType == encType &&
               !strcmp(mSecurityInfoList[i]->data.esp.spi, spi1)) {
                if(mSecurityInfoList[i]->onDelete) {
                    delete mSecurityInfoList[i];
                    mSecurityInfoList.erase(mSecurityInfoList.begin() + i);
                    PKMM_LOGD("Remove: type: %d, spi1: %s, spi2: %s done", (int)encType, spi1, spi2);
                }
                else {
                    mSecurityInfoList[i]->onDelete = true;
                    PKMM_LOGD("Marked: type: %d, spi1: %s, spi2: %s done", (int)encType, spi1, spi2);
                }
                break;
            }
        }
    }
    else if(EncryptionTypeEspImc == encType) {
        if(spi1 == NULL && spi2 == NULL) {
            for (int i = 0, size = mSecurityInfoList.size(); i < size; i++) {
                if(mSecurityInfoList[i]->encType == encType) {
                    if(mSecurityInfoList[i]->onDelete) {
                        delete mSecurityInfoList[i];
                        mSecurityInfoList.erase(mSecurityInfoList.begin() + i);
                        PKMM_LOGD("Remove: type: %d, spi1: %s, spi2: %s done", (int)encType
                                                                           , spi1, spi2);
                    }
                    else {
                        mSecurityInfoList[i]->onDelete = true;
                        PKMM_LOGD("Marked: type: %d, spi1: %s, spi2: %s done", (int)encType
                                                                           , spi1, spi2);
                    }
                    break;
                }
            }
        }
    }
    pthread_mutex_unlock(&mMutex);
}

/***************************************************************************************************
 * Add Security Info
 * @param encType Encryption Type
 * @param obj Security Info Object
 *
 **************************************************************************************************/
void SecurityAssociation::addInfo(EncryptionType encType, SecurityInfo *obj) {

    obj->encType = encType;
    PKMM_LOGD("addInfo: encType: %d", (int)encType);

    if(encType == EncryptionTypeIKev2) {
        pthread_mutex_lock(&mMutex);
        for (int i = 0, size = mSecurityInfoList.size(); i < size; i++) {
            if (mSecurityInfoList[i]->encType == EncryptionTypeIKev2) {
                if (!strcasecmp(mSecurityInfoList[i]->data.ikev2.spi_i, obj->data.ikev2.spi_i) &&
                   ((obj->data.ikev2.spi_r[2] == '0') ||
                    !strcasecmp(mSecurityInfoList[i]->data.ikev2.spi_r, obj->data.ikev2.spi_r)))
                {
                    delete mSecurityInfoList[i];
                    mSecurityInfoList[i] = obj;

                    pthread_mutex_unlock(&mMutex);
                    return;
                }
            }
        }

        mSecurityInfoList.push_back(obj);
        pthread_mutex_unlock(&mMutex);
    }
    else if(encType == EncryptionTypeEsp || encType == EncryptionTypeEspImc) {

        pthread_mutex_lock(&mMutex);
        for (int i = 0, size = mSecurityInfoList.size(); i < size; i++)
        {
            if (mSecurityInfoList[i]->encType == EncryptionTypeEsp ||
                mSecurityInfoList[i]->encType == EncryptionTypeEspImc)
            {
                if (!strcasecmp(mSecurityInfoList[i]->data.esp.spi, obj->data.esp.spi)) {
                    delete mSecurityInfoList[i];
                    mSecurityInfoList[i] = obj;

                    pthread_mutex_unlock(&mMutex);
                    return;
                }
            }
        }

        mSecurityInfoList.push_back(obj);
        pthread_mutex_unlock(&mMutex);
    }
}

/***************************************************************************************************
 * Is ESP Record in SA List
 * @param srcIp Source IP
 * @param destIp Destination IP
 * @return True, if ESP record is presented
 *
 **************************************************************************************************/
bool SecurityAssociation::isEspRecordPresent(char *srcIp, char * destIp) {

    pthread_mutex_lock(&mMutex);

    for (int i = 0, size = mSecurityInfoList.size(); i < size; i++) {
        if (mSecurityInfoList[i]->encType == EncryptionTypeEsp &&
            !strcmp(mSecurityInfoList[i]->data.esp.src_addr, srcIp) &&
            !strcmp(mSecurityInfoList[i]->data.esp.dst_addr, destIp)) {

            // Release Mutex
            pthread_mutex_unlock(&mMutex);
            return true;
        }
    }

    pthread_mutex_unlock(&mMutex);
    return false;
}