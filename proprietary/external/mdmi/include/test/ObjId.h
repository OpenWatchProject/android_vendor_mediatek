
/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef __OBJ_ID_H__
#define __OBJ_ID_H__

#include <vector>
#include <inttypes.h>


class ObjId {
public:
    ObjId(const char *id_string = NULL) {
        // init id
        memset(id, 0, sizeof(id));
        id_length = 0;

        // set ID
        if (NULL != id_string) {
         setIdString(id_string);
        }
    }

    int setIdString(const char *id_string_) {
        if (id_string_ == NULL) return 0;

        memcpy(id_string, id_string_, strlen(id_string_) + 1);
        // reset
        memset(id, 0, sizeof(id));
        id_length = 0;

        if (NULL != id_string_) {
            const char *p = id_string_;
            char *pp = NULL;
            do {
                long int v = strtol(p, &pp, 10);
                id[id_length++] = v;
                p = pp+1;
            } while('\0' != *pp);
        }

        // dump id for debugging
        //for (int i = 0 ; i < id_length ; ++i) {
        //    DMC_LOGD(TAG, "version %d = %d.", i , id[i]);
        //}
        return 0;
    }

    int setIdsVec(unsigned int len, std::vector<uint32_t> ids) {
        char *ptr = &id_string[0];
        memset(ptr, 0, sizeof(id_string));
        if (len > ids.size()) {
            len = ids.size();
        }
        id_length = len;
        for (unsigned int i = 0; i < id_length; i++) {
            id[i] = ids[i];
            if (i == 0) {
                sprintf(ptr, "%" PRIu32, id[i]);
            } else {
                sprintf(ptr, ".%" PRIu32, id[i]);
            }
            ptr += strlen(ptr)*sizeof(char);
        }
        return 0;
    }

    uint32_t *getObjId() const {
        return (uint32_t *)id;
    }

    uint32_t getObjLength() const {
        return id_length;
    }

    char *getOidString() const {
        return (char *)id_string;
    }

    bool IsPrefixIncluded(const ObjId &oid) const {
        // input oid is predix of this OID.
        uint32_t *pOid_in = oid.getObjId();
        uint32_t *pOid_me = this->getObjId();

        if (oid.getObjLength() > this->getObjLength()) {
            return false;
        }
        for (int i = 0; i < oid.getObjLength(); i++) {
            if (pOid_in[i] != pOid_me[i]) {
                return false;
            }
        }
        return true;
    }

    ObjId& operator=(const ObjId& other) {
        this->id_length = other.id_length;
        memcpy(this->id, other.id, sizeof(this->id));
        return *this;
    }

    bool operator==(const ObjId & lhs) {
        if (lhs.getObjLength() != this->getObjLength()) {
            return false;
        }
        if (memcmp(lhs.getObjId(), this->getObjId(),
                lhs.getObjLength()*sizeof(uint32_t)) != 0) {
            return false;
        }
        return true;
    }

    bool operator!=(const ObjId & lhs) {
        return !(*this == lhs);
    }

private:
    char id_string[50];
    uint32_t id[10]; // max : 10 digit
    uint32_t id_length;
};

#endif
