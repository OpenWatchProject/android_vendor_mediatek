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

#ifndef __PKM_MDM_ADAPTER_H
#define __PKM_MDM_ADAPTER_H

#include <vector>
#include <queue>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <vector>

#include <FrameDecoder.h>
#include <JsonCmdEncoder.h>
#include <MonitorCmdProxy.h>
#include <MonitorTrapReceiver.h>
#include <MonitorTypes.h>

#include "SecurityAssociation.h"

using std::shared_ptr;
using std::queue;
using std::thread;
using std::mutex;
using std::condition_variable;
using std::vector;

using libmdmonitor::MCPReceiver;
using libmdmonitor::MonitorCmdProxy;
using libmdmonitor::MonitorTrapReceiver;
using libmdmonitor::MSGID;
using libmdmonitor::SID;
using libmdmonitor::TRAP_TYPE;
using libmdmonitor::JsonCmdEncoder;

#define MAX_SPI     21
#define SPI_SIZE    20

typedef struct
{
  int id;
  int size;
  const char *int_algo_esp;
  const char *int_algo_ikev2;
} auth_algo_struct;

typedef struct
{
  int id;
  int size;
  int block_size;
  const char *enc_algo_name;
} enc_algo_struct;

typedef enum
{
    MSG_ID_EM_WO_IKE_DECRYPT_INFO_ADD_IND,
    MSG_ID_EM_WO_IKE_DECRYPT_INFO_DEL_IND,
    MSG_ID_EM_WO_ESP_DECRYPT_INFO_ADD_IND,
    MSG_ID_EM_WO_ESP_DECRYPT_INFO_DEL_IND,
    MSG_ID_EM_LTECSR_RTP_EVENT_IND,
    MSG_ID_EM_IMC_IPSEC_INFO_FLUSH_IND,
    MSG_ID_EM_IMC_IPSEC_INFO_ADD_IND,
    MSG_ID_EM_IMC_IPSEC_INFO_DELETE_IND,
    MSG_ID_EM_MAX

} MSG_ID_EM_ENUM;

namespace com {
    namespace mediatek {
        namespace diagnostic {

            class MdmEventCallback {
                public:
                    virtual ~MdmEventCallback() {};
                    virtual void onLtecsrRtpEvent(uint8_t event, uint16_t src, uint16_t dest) = 0;
                    virtual void onAddKeyEvent(EncryptionType type, char *spi1, char *spi2) = 0;

            };

            class MdmReceiver {
                public:
                    MdmReceiver(SecurityAssociation *sa, MdmEventCallback *callback);
                    virtual ~MdmReceiver();

                private:
                    bool subscribe(MSG_ID_EM_ENUM Id);
                    void decode(TRAP_TYPE trapType, size_t len, const uint8_t *data);
                    static void TrapCB(void *param, uint32_t timestamp,
                                       TRAP_TYPE trapType, size_t len,
                                       const uint8_t *data, size_t discard_count);

                    bool decodeEmWoIkeDecryptInfoAdd(MSGID msgID, const uint8_t *data, size_t len);
                    bool decodeEmWoIkeDecryptInfoDel(MSGID msgID, const uint8_t *data, size_t len);
                    bool decodeEmWoEspDecryptInfoAdd(MSGID msgID, const uint8_t *data, size_t len);
                    bool decodeEmWoEspDecryptInfoDel(MSGID msgID, const uint8_t *data, size_t len);
                    bool decodeEmLtecsrRtpEvent(MSGID msgID, const uint8_t *data, size_t len);
                    bool decodeEmImcIpsecInfoFlush(MSGID msgID, const uint8_t *data, size_t len);
                    bool decodeEmImcIpsecInfoAdd(MSGID msgID, const uint8_t *data, size_t len);
                    bool decodeEmImcIpsecInfoDelete(MSGID msgID, const uint8_t *data, size_t len);

                    const auth_algo_struct* findAuthAlgo(EncryptionType encType, int id);
                    const enc_algo_struct* findEncAlgo(EncryptionType encType, int id, int size);

                private:
                    // MD Monitor, must initialize with this order
                    MonitorCmdProxy<JsonCmdEncoder> mCmdClient;
                    SID mSessionId;
                    MonitorTrapReceiver mTrapReceiver;

                    std::unordered_map<MSGID, MSG_ID_EM_ENUM> mMsgIdsList;

                    static const auth_algo_struct mAuthAlgos[5];
                    static const enc_algo_struct mEspEncAlgos[6];
                    static const enc_algo_struct mIkeEncAlgos[6];

                    static const auth_algo_struct mImcIpsecAuthAlgos[2];
                    static const enc_algo_struct mImcIpsecEncAlgos[3];

                    SecurityAssociation *mSa;
                    MdmEventCallback *mCallback;

            };
        }
    }
}

#endif // MDM_ADAPTER_H