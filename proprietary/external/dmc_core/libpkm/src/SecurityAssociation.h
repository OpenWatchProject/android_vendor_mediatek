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

#ifndef __PKM_SECURITY_ASSOCIATION_H
#define __PKM_SECURITY_ASSOCIATION_H

#include <memory>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

#include <FrameDecoder.h>
#include <MonitorCmdProxy.h>
#include <JsonCmdEncoder.h>
#include <MonitorTrapReceiver.h>

using std::shared_ptr;
using std::queue;
using std::thread;
using std::mutex;
using std::condition_variable;

using libmdmonitor::MonitorCmdProxy;
using libmdmonitor::JsonCmdEncoder;
using libmdmonitor::SID;

typedef enum
{
    EncryptionTypeNone = -1,
    EncryptionTypeIKev2Unencrypted,
    EncryptionTypeIKev2,
    EncryptionTypeEsp,
    EncryptionTypeEspImc,

} EncryptionType;

typedef struct
{
    char        spi_i[20];
    char        spi_r[20];
    char        sk_ei[256];
    char        sk_er[256];
    char        enc_algo_name[20];
    char        sk_ai[256];
    char        sk_ar[256];
    char        int_algo_esp[20];
    char        int_algo_ikev2[20];

} IKev2Info;

typedef struct
{
    uint16_t    ip_family;
    char        spi[20];
    char        src_addr[64];
    char        dst_addr[64];
    char        enc_algo_name[20];
    char        enc_key[256];
    char        int_algo_esp[20];
    char        int_algo_ikev2[20];
    char        int_key[256];
    int         index;
    int         block;
    int         auth_block;

} EspInfo;

typedef struct
{
    EncryptionType encType;
    bool onDelete;
    union
    {
        IKev2Info ikev2;
        EspInfo esp;
    } data;

} SecurityInfo;

namespace com {
    namespace mediatek {
        namespace diagnostic {

            class SecurityAssociation {
                public:
                    SecurityAssociation();
                    virtual ~SecurityAssociation();

                    bool findEspSecret(EncryptionType encType, char *buf, int len,
                                       int *encBlockSize, int *authSize,
                                       char *spi1, char *spi2, int initiator);


                    void addInfo(EncryptionType encType, SecurityInfo *data);
                    void removeInfoByIndex(EncryptionType encType, int index);
                    void removeInfoBySpi(EncryptionType encType, char *spi1, char *spi2);

                    bool isEspRecordPresent(char *srcIp, char * destIp);


                private:
                    std::vector <SecurityInfo *> mSecurityInfoList;
                    pthread_mutex_t mMutex;

                    void removeOnDelete();
            };
        }
    }
}

#endif // CP_MDM_SECURITY_ASSOCIATION_H