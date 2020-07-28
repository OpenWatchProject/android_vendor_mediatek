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

#include "MdmAdapter.h"
#include "SecurityAssociation.h"
#include "Logs.h"

#include <MonitorDefs.h>

#define CAPTUREPACKETS_RECEIVER_HOST_NAME "CAPTUREPACKETS_Trap_Receiver"

#define UNUSED(x) (void)(x)

using std::vector;

using com::mediatek::diagnostic::MdmReceiver;
using com::mediatek::diagnostic::MdmEventCallback;

using libmdmonitor::FrameDecoder;
using libmdmonitor::MCPReceiver;
using libmdmonitor::FRAME_INFO;

const static char* TAG = "PKM-MDM";

const auth_algo_struct MdmReceiver::mAuthAlgos[] = {
{ 1, 96, "hmac96", "md5"},
{ 2, 96, "hmac96", "sha1"},
{12, 128, "hmac96", "sha2-256"},  // hmac128, libdecrypt not support
{13, 192, "hmac96", "sha2-384"},  // hmac192, libdecrypt not support
{14, 256, "hmac96", "sha2-512"},  // hmac256, libdecrypt not support
};

const enc_algo_struct MdmReceiver::mEspEncAlgos[] = {
{2, 0, 64, "des-cbc"},
{3, 192, 64, "3des-cbc"},
{12, 128, 128, "aes-128-cbc"},
{12, 256, 128, "aes-256-cbc"},
{13, 128, 128, "aes-128-ctr"},
{13, 256, 128, "aes-256-ctr"}
};

const enc_algo_struct MdmReceiver::mIkeEncAlgos[] = {
{2, 0, 64, "des-cbc"},
{3, 0, 64, "3des-cbc"},
{12, 128, 128, "aes-128-cbc"},
{12, 256, 128, "aes-256-cbc"},
{13, 128, 128, "aes-128-ctr"},
{13, 256, 128, "aes-256-ctr"}
};

const enc_algo_struct MdmReceiver::mImcIpsecEncAlgos[] = {
{0, 0, 0, "NULL"},
{1, 192, 64, "3des-cbc"},
{2, 128, 128, "AES-128-CBC"},
};

const auth_algo_struct MdmReceiver::mImcIpsecAuthAlgos[] =
{
{0, 96, "hmac96","md5"},
{1, 96, "hmac96","sha1"}
};

MdmReceiver::MdmReceiver(SecurityAssociation *sa, MdmEventCallback *callback):
                         mCmdClient(MONITOR_COMMAND_SERVICE_ABSTRACT_NAME),
                         mSessionId(mCmdClient.CreateSession()),
                         mTrapReceiver(mSessionId, CAPTUREPACKETS_RECEIVER_HOST_NAME),
                         mSa(NULL), mCallback(NULL) {

    PKMM_LOGD("MdmReceiver::MdmReceiver");
    mSa = sa;
    mCallback = callback;

    // Register for IPSec keys from MDM
    mTrapReceiver.SetTrapHandler(TrapCB, this);
    mCmdClient.SetTrapReceiver(mSessionId, mSessionId, CAPTUREPACKETS_RECEIVER_HOST_NAME);
    mCmdClient.EnableTrap(mSessionId, mSessionId);

    subscribe(MSG_ID_EM_WO_IKE_DECRYPT_INFO_ADD_IND);
    subscribe(MSG_ID_EM_WO_IKE_DECRYPT_INFO_DEL_IND);
    subscribe(MSG_ID_EM_WO_ESP_DECRYPT_INFO_ADD_IND);
    subscribe(MSG_ID_EM_WO_ESP_DECRYPT_INFO_DEL_IND);
    subscribe(MSG_ID_EM_LTECSR_RTP_EVENT_IND);
    subscribe(MSG_ID_EM_IMC_IPSEC_INFO_FLUSH_IND);
    subscribe(MSG_ID_EM_IMC_IPSEC_INFO_ADD_IND);
    subscribe(MSG_ID_EM_IMC_IPSEC_INFO_DELETE_IND);
}

MdmReceiver::~MdmReceiver() {
    PKMM_LOGD("MdmReceiver disableTrap");
    try {
        mCmdClient.DisableTrap(mSessionId, mSessionId);
    } catch (const std::runtime_error&) {
        PKMM_LOGE("DisableTrap() failed!");
    }

    PKMM_LOGD("MdmReceiver deleted");
}

bool MdmReceiver::subscribe(MSG_ID_EM_ENUM id) {

    const char *message = NULL;
    switch(id) {
        case MSG_ID_EM_WO_IKE_DECRYPT_INFO_ADD_IND:
            message = "MSG_ID_EM_WO_IKE_DECRYPT_INFO_ADD_IND";
            break;
        case MSG_ID_EM_WO_IKE_DECRYPT_INFO_DEL_IND:
            message = "MSG_ID_EM_WO_IKE_DECRYPT_INFO_DEL_IND";
            break;
        case MSG_ID_EM_WO_ESP_DECRYPT_INFO_ADD_IND:
            message = "MSG_ID_EM_WO_ESP_DECRYPT_INFO_ADD_IND";
            break;
        case MSG_ID_EM_WO_ESP_DECRYPT_INFO_DEL_IND:
            message = "MSG_ID_EM_WO_ESP_DECRYPT_INFO_DEL_IND";
            break;
        case MSG_ID_EM_LTECSR_RTP_EVENT_IND:
            message = "MSG_ID_EM_LTECSR_RTP_EVENT_IND";
            break;
        case MSG_ID_EM_IMC_IPSEC_INFO_FLUSH_IND:
            message = "MSG_ID_EM_IMC_IPSEC_INFO_FLUSH_IND";
            break;
        case MSG_ID_EM_IMC_IPSEC_INFO_ADD_IND:
            message = "MSG_ID_EM_IMC_IPSEC_INFO_ADD_IND";
            break;
        case MSG_ID_EM_IMC_IPSEC_INFO_DELETE_IND:
            message = "MSG_ID_EM_IMC_IPSEC_INFO_DELETE_IND";
            break;

        default:
            PKM_LOGE("[%s] subscribed failure, no such message.", message);
            return false;
    }

    MSGID msgId;
    TRAP_TYPE trapType;
    FrameDecoder &decoder = *(FrameDecoder::GetInstance());

    // Delete SA when IMS Register
    if (decoder.GetMsgByString(message, trapType, msgId)) {
        PKMM_LOGI("[%s][%d] subscribed successfully.", message, (int)msgId);
        mCmdClient.SubscribeTrap(mSessionId, mSessionId, trapType, msgId, false);
        mMsgIdsList[msgId] = id;
        return true;
    }

    return false;
}

void MdmReceiver::TrapCB(void *param, uint32_t timestamp,
                         TRAP_TYPE trapType, size_t len,
                         const uint8_t *data, size_t discard_count)
{
    UNUSED(timestamp);
    UNUSED(discard_count);

    MdmReceiver *me = static_cast<MdmReceiver *>(param);
    if (NULL != me) {
        me->decode(trapType, len, data);
    }
}

void MdmReceiver::decode(TRAP_TYPE trapType, size_t len, const uint8_t *data) {

    FrameDecoder &decoder = *(FrameDecoder::GetInstance());
    FRAME_INFO frameInfo;

    if (!decoder.ParseTrapInfo(trapType, 0, data, len, frameInfo)) {
        PKM_LOGE("decode: Failed to parse trap info. trapType: [%d], len: [%d]", (int)trapType
                                                                               , (int)len);
        return;
    }

    MSG_ID_EM_ENUM msgType = MSG_ID_EM_MAX;
    MSGID msgID = frameInfo.msgID;
    auto itr = mMsgIdsList.find(msgID);

    if (itr == mMsgIdsList.end()) {
        PKM_LOGE("decode: Can't find type for [%" PRIu64"]", frameInfo.msgID);
        return;
    }

    bool ret = false;
    msgType = itr->second;
    switch(msgType) {
        case MSG_ID_EM_WO_IKE_DECRYPT_INFO_ADD_IND:
            ret = decodeEmWoIkeDecryptInfoAdd(msgID, data, len);
            break;

        case MSG_ID_EM_WO_IKE_DECRYPT_INFO_DEL_IND:
            ret = decodeEmWoIkeDecryptInfoDel(msgID, data, len);
            break;

        case MSG_ID_EM_WO_ESP_DECRYPT_INFO_ADD_IND:
            ret = decodeEmWoEspDecryptInfoAdd(msgID, data, len);
            break;

        case MSG_ID_EM_WO_ESP_DECRYPT_INFO_DEL_IND:
            ret = decodeEmWoEspDecryptInfoDel(msgID, data, len);
            break;

        case MSG_ID_EM_LTECSR_RTP_EVENT_IND:
            ret = decodeEmLtecsrRtpEvent(msgID, data, len);
            break;

        case MSG_ID_EM_IMC_IPSEC_INFO_FLUSH_IND:
            ret = decodeEmImcIpsecInfoFlush(msgID, data, len);
            break;

        case MSG_ID_EM_IMC_IPSEC_INFO_ADD_IND:
            ret = decodeEmImcIpsecInfoAdd(msgID, data, len);
            break;

        case MSG_ID_EM_IMC_IPSEC_INFO_DELETE_IND:
            ret = decodeEmImcIpsecInfoDelete(msgID, data, len);
            break;

        default:
            PKM_LOGE("decode: Invalid msgType %d", msgType);
    }

    if(ret) {
        // do nothing
    }
    else {
        PKM_LOGE("decode: decode fail %d", msgType);
    }
}

bool MdmReceiver::decodeEmWoIkeDecryptInfoAdd(MSGID msgID, const uint8_t *data, size_t len) {

    PKM_LOGD("decodeEmWoIkeDecryptInfoAdd: %d, len = %d", (int)msgID, (int)len);

    FrameDecoder &decoder = *(FrameDecoder::GetInstance());
    size_t out_Len = 0;

    // SPI_I
    const uint8_t* spi_i = NULL;
    if (NULL == (spi_i = decoder.GetEMFieldBuf(msgID, data, len, "spi_i[0]", 20, out_Len))) {
        PKM_LOGE("decodeEmWoIkeDecryptInfoAdd SPI_I fail");
        return false;
    }

    // SPI_R
    const uint8_t* spi_r = NULL;
    if (NULL == (spi_r = decoder.GetEMFieldBuf(msgID, data, len, "spi_r[0]", 20, out_Len))) {
        PKM_LOGE("decodeEmWoIkeDecryptInfoAdd SPI_R fail");
        return false;
    }

    // SK_EI
    const uint8_t* sk_ei = NULL;
    if (NULL == (sk_ei = decoder.GetEMFieldBuf(msgID, data, len, "sk_ei[0]", 256, out_Len))) {
        PKM_LOGE("decodeEmWoIkeDecryptInfoAdd SK_EI fail");
        return false;
    }

    // SK_ER
    const uint8_t* sk_er = NULL;
    if (NULL == (sk_er = decoder.GetEMFieldBuf(msgID, data, len, "sk_er[0]", 256, out_Len))) {
        PKM_LOGE("decodeEmWoIkeDecryptInfoAdd SK_ER fail");
        return false;
    }

    // ENC_ALGO
    int64_t enc_algo = 0;
    if(!decoder.GetEMFieldVal(msgID, data, 2, "enc_algo", false, enc_algo)) {
        PKM_LOGE("decodeEmWoIkeDecryptInfoAdd ENC_ALGO fail");
        return false;
    }

    // ENC_KEY_SIZE
    int64_t enc_key_size = 0;
    if(!decoder.GetEMFieldVal(msgID, data, 2, "enc_key_size", false, enc_key_size)) {
        PKM_LOGE("decodeEmWoIkeDecryptInfoAdd ENC_KEY_SIZE fail");
        return false;
    }

    // SK_AI
    const uint8_t* sk_ai = NULL;
    if (NULL == (sk_ai = decoder.GetEMFieldBuf(msgID, data, len, "sk_ai[0]", 256, out_Len))) {
        PKM_LOGE("decodeEmWoIkeDecryptInfoAdd SK_EI fail");
        return false;
    }

    // SK_AR
    const uint8_t* sk_ar = NULL;
    if (NULL == (sk_ar = decoder.GetEMFieldBuf(msgID, data, len, "sk_ar[0]", 256, out_Len))) {
        PKM_LOGE("decodeEmWoIkeDecryptInfoAdd SK_AR fail");
        return false;
    }

    // INT_ALGO
    int64_t int_algo = 0;
    if(!decoder.GetEMFieldVal(msgID, data, 2, "int_algo", false, int_algo)) {
        PKM_LOGE("decodeEmWoIkeDecryptInfoAdd INT_ALGO fail");
        return false;
    }

    SecurityInfo *obj = new SecurityInfo;

    IKev2Info *p = &(obj->data.ikev2);
    const auth_algo_struct *auth_algo_s = NULL;
    const enc_algo_struct *enc_algo_s = NULL;
    bool done = false;

    do {
        snprintf(p->spi_i, sizeof(p->spi_i), "%s", spi_i);
        snprintf(p->spi_r, sizeof(p->spi_r), "%s", spi_r);
        snprintf(p->sk_ei, sizeof(p->sk_ei), "%s", sk_ei);
        snprintf(p->sk_er, sizeof(p->sk_er), "%s", sk_er);

        if(NULL == (enc_algo_s = findEncAlgo(EncryptionTypeIKev2, enc_algo, enc_key_size)))
            break;

        snprintf(p->enc_algo_name, sizeof(p->enc_algo_name), "%s", enc_algo_s->enc_algo_name);

        snprintf(p->sk_ai, sizeof(p->sk_ai), "%s", sk_ai);
        snprintf(p->sk_ar, sizeof(p->sk_ar), "%s", sk_ar);

        if (NULL == (auth_algo_s = findAuthAlgo(EncryptionTypeIKev2, int_algo)))
            break;

        snprintf(p->int_algo_esp, sizeof(p->int_algo_esp), "%s", auth_algo_s->int_algo_esp);
        snprintf(p->int_algo_ikev2, sizeof(p->int_algo_ikev2), "%s", auth_algo_s->int_algo_ikev2);

        done = true;
    } while (0);

    if (!done) {
        PKM_LOGE(TAG, "decodeEmWoIkeDecryptInfoAdd: failure");
        delete(obj);
        return false;
    }

    mSa->addInfo(EncryptionTypeIKev2, obj);
    return true;
}

bool MdmReceiver::decodeEmWoIkeDecryptInfoDel(MSGID msgID, const uint8_t *data, size_t len) {

    PKM_LOGD("decodeEmWoIkeDecryptInfoDel: %d, len = %d", (int)msgID, (int)len);

    FrameDecoder &decoder = *(FrameDecoder::GetInstance());
    size_t out_Len = 0;

    // SPI_I
    const uint8_t* spi_i = NULL;
    if (NULL == (spi_i = decoder.GetEMFieldBuf(msgID, data, len, "spi_i[0]", 20, out_Len))) {
        PKM_LOGE("decodeEmWoIkeDecryptInfoDel SPI_I fail");
        return false;
    }

    // SPI_R
    const uint8_t* spi_r = NULL;
    if (NULL == (spi_r = decoder.GetEMFieldBuf(msgID, data, len, "spi_r[0]", 20, out_Len))) {
        PKM_LOGE("decodeEmWoIkeDecryptInfoDel SPI_R fail");
        return false;
    }

    mSa->removeInfoBySpi(EncryptionTypeIKev2, (char *)spi_i, (char *)spi_r);
    return true;
}

bool MdmReceiver::decodeEmWoEspDecryptInfoAdd(MSGID msgID, const uint8_t *data, size_t len) {

    PKM_LOGD("decodeEmWoEspDecryptInfoAdd: %d, len = %d", (int)msgID, (int)len);

    FrameDecoder &decoder = *(FrameDecoder::GetInstance());
    size_t out_Len = 0;

    // IP_PROTO_VERSION
    int64_t ip_proto_version = 0;
    if(!decoder.GetEMFieldVal(msgID, data, 2, "ip_proto_version", false, ip_proto_version)) {
        PKM_LOGE("decodeEmWoEspDecryptInfoAdd IP_PROTO_VERSION fail");
        return false;
    }

    // SPI
    const uint8_t* spi = NULL;
    if (NULL == (spi = decoder.GetEMFieldBuf(msgID, data, len, "spi[0]", 20, out_Len))) {
        PKM_LOGE("decodeEmWoEspDecryptInfoAdd SPI_R fail");
        return false;
    }

    // SRC_IP
    const uint8_t* src_ip = NULL;
    if (NULL == (src_ip = decoder.GetEMFieldBuf(msgID, data, len, "src_ip[0]", 64, out_Len))) {
        PKM_LOGE("decodeEmWoEspDecryptInfoAdd SRC_IP fail");
        return false;
    }

    // DST_IP
    const uint8_t* dst_ip = NULL;
    if (NULL == (dst_ip = decoder.GetEMFieldBuf(msgID, data, len, "dst_ip[0]", 64, out_Len))) {
        PKM_LOGE("decodeEmWoEspDecryptInfoAdd DST_IP fail");
        return false;
    }

    // ENC_ALGO
    int64_t enc_algo = 0;
    if(!decoder.GetEMFieldVal(msgID, data, 2, "enc_algo", false, enc_algo)) {
        PKM_LOGE("decodeEmWoEspDecryptInfoAdd ENC_ALGO fail");
        return false;
    }

    // ENC_KEY_SIZE
    int64_t enc_key_size = 0;
    if(!decoder.GetEMFieldVal(msgID, data, 2, "enc_key_size", false, enc_key_size)) {
        PKM_LOGE("decodeEmWoEspDecryptInfoAdd ENC_KEY_SIZE fail");
        return false;
    }

    // ENC_KEY
    const uint8_t* enc_key = NULL;
    if (NULL == (enc_key = decoder.GetEMFieldBuf(msgID, data, len, "enc_key[0]", 256, out_Len))) {
        PKM_LOGE("decodeEmWoEspDecryptInfoAdd ENC_KEY fail");
        return false;
    }

    // INT_ALGO
    int64_t int_algo = 0;
    if(!decoder.GetEMFieldVal(msgID, data, 2, "int_algo", false, int_algo)) {
        PKM_LOGE("decodeEmWoEspDecryptInfoAdd INT_ALGO fail");
        return false;
    }

    // INT_KEY
    const uint8_t* int_key = NULL;
    if (NULL == (int_key = decoder.GetEMFieldBuf(msgID, data, len, "int_key[0]", 256, out_Len))) {
        PKM_LOGE("decodeEmWoEspDecryptInfoAdd INT_KEY fail");
        return false;
    }

    SecurityInfo *obj = new SecurityInfo;

    EspInfo *p = &(obj->data.esp);
    const auth_algo_struct *auth_algo_s = NULL;
    const enc_algo_struct *enc_algo_s = NULL;
    bool done = false;

    do
    {
        p->ip_family = ip_proto_version;

        snprintf(p->spi, sizeof(p->spi), "%s", spi);
        snprintf(p->src_addr, sizeof(p->src_addr), "%s", src_ip);
        snprintf(p->dst_addr, sizeof(p->dst_addr), "%s", dst_ip);

        PKM_LOGD("decodeEmWoEspDecryptInfoAdd: encAlgo: %lld, keyLen = %lld", enc_algo, enc_key_size);

        if (NULL == (enc_algo_s = findEncAlgo(EncryptionTypeEsp, enc_algo, enc_key_size)))
            break;

        snprintf(p->enc_algo_name, sizeof(p->enc_algo_name), "%s", enc_algo_s->enc_algo_name);
        snprintf(p->enc_key, sizeof(p->enc_key), "%s", enc_key);
        p->block = enc_algo_s->block_size;

        PKM_LOGD("decodeEmWoEspDecryptInfoAdd: initAlgo: %d", (int)int_algo);

        if (NULL == (auth_algo_s = findAuthAlgo(EncryptionTypeEsp, int_algo)))
            break;

        snprintf(p->int_algo_esp, sizeof(p->int_algo_esp), "%s", auth_algo_s->int_algo_esp);
        snprintf(p->int_algo_ikev2, sizeof(p->int_algo_ikev2), "%s", auth_algo_s->int_algo_ikev2);
        snprintf(p->int_key, sizeof(p->int_key), "%s", int_key);

        // Set Authentication Data Block Size
        p->auth_block = auth_algo_s->size;

        done = true;

    } while (0);

    if (!done) {
        PKM_LOGE(TAG, "decodeEmWoEspDecryptInfoAdd: failure");
        delete(obj);
        return false;
    }

    mSa->addInfo(EncryptionTypeEsp, obj);
    if(mCallback != NULL) {
        mCallback->onAddKeyEvent(EncryptionTypeEsp, obj->data.esp.spi, NULL);
    }

    return true;
}

bool MdmReceiver::decodeEmWoEspDecryptInfoDel(MSGID msgID, const uint8_t *data, size_t len) {
    PKM_LOGD("decodeEmWoEspDecryptInfoDel: %d, len = %d", (int)msgID, (int)len);

    FrameDecoder &decoder = *(FrameDecoder::GetInstance());
    size_t out_Len = 0;

    // SPI
    const uint8_t* spi = NULL;
    if (NULL == (spi = decoder.GetEMFieldBuf(msgID, data, len, "spi[0]", 20, out_Len))) {
        PKM_LOGE("decodeEmWoEspDecryptInfoDel SPI_R fail");
        return false;
    }

    mSa->removeInfoBySpi(EncryptionTypeEsp, (char *)spi, NULL);
    return true;
}

bool MdmReceiver::decodeEmLtecsrRtpEvent(MSGID msgID, const uint8_t *data, size_t len) {
    PKM_LOGD("decodeEmLtecsrRtpEvent: %d, len = %d", (int)msgID, (int)len);

    FrameDecoder &decoder = *(FrameDecoder::GetInstance());
    size_t out_Len = 0;

    // Event
    const uint8_t* buf = NULL;
    if (NULL == (buf = decoder.GetEMFieldBuf(msgID, data, len, "event", 1, out_Len))) {
        PKM_LOGE("decodeEmLtecsrRtpEvent EVENT fail");
        return false;
    }

    uint8_t event = buf[0];

    // RTP SRC PORT
    int64_t rtp_src_port = 0;
    if(!decoder.GetEMFieldVal(msgID, data, 2, "rtp_src_port", false, rtp_src_port)) {
        PKM_LOGE("decodeEmLtecsrRtpEvent RTP SRC PORT fail");
        return false;
    }

    // RTP DST PORT
    int64_t rtp_dst_port = 0;
    if(!decoder.GetEMFieldVal(msgID, data, 2, "rtp_dst_port", false, rtp_dst_port)) {
        PKM_LOGE("decodeEmLtecsrRtpEvent RTP DST PORT fail");
        return false;
    }

    if(mCallback != NULL) {
        mCallback->onLtecsrRtpEvent(event, (uint32_t)rtp_src_port, (uint32_t)rtp_dst_port);
    }

    return true;
}

bool MdmReceiver::decodeEmImcIpsecInfoFlush(MSGID msgID, const uint8_t *data, size_t len) {
    UNUSED(data);
    PKM_LOGD("decodeEmImcIpsecInfoFlush: %d, len = %d", (int)msgID, (int)len);

    mSa->removeInfoBySpi(EncryptionTypeEspImc, NULL, NULL);
    return true;
}

bool MdmReceiver::decodeEmImcIpsecInfoAdd(MSGID msgID, const uint8_t *data, size_t len) {
    PKM_LOGD("decodeEmImcIpsecInfoAdd: %d, len = %d", (int)msgID, (int)len);

    FrameDecoder &decoder = *(FrameDecoder::GetInstance());
    size_t out_Len = 0;

    // SA Management Index
    const uint8_t* index = NULL;
    if (NULL == (index = decoder.GetEMFieldBuf(msgID, data, len, "index", 1, out_Len))) {
        return false;
    }

    // IPsec Encrypt Algoruthm: 0:NULL/1:3des_cbc/2:aes_128_cbc
    const uint8_t* encry_algo = NULL;
    if (NULL == (encry_algo = decoder.GetEMFieldBuf(msgID, data, len, "encry_algo", 1, out_Len)))
    {
        return false;
    }

    const enc_algo_struct *enc_algo = NULL;
    if (NULL == (enc_algo = findEncAlgo(EncryptionTypeEspImc, encry_algo[0] & 0xFF, 0))) {
        return false;
    }

    int block = enc_algo->block_size;
    PKM_LOGD("IPSEC_INFO_ADD_IND:encry_algo: %s, block = %d, %d", enc_algo->enc_algo_name
                                                                , block, out_Len);

    const uint8_t* integrity_algo = NULL;
    if (NULL == (integrity_algo = decoder.GetEMFieldBuf(msgID, data, len
                                                             , "integrity_algo" , 1, out_Len)))
    {
        return false;
    }

    const auth_algo_struct *auth_algo = NULL;
    if (NULL == (auth_algo = findAuthAlgo(EncryptionTypeEspImc, integrity_algo[0] & 0xFF))) {
        return false;
    }

    PKM_LOGD("IPSEC_INFO_ADD_IND:integrity_algo: %s:%s, block = %d,",auth_algo->int_algo_esp
                                                                    ,auth_algo->int_algo_ikev2
                                                                    ,auth_algo->size);

    const uint8_t* ck = NULL;
    if (NULL == (ck = decoder.GetEMFieldBuf(msgID, data, len, "ck[0]", 1, out_Len))) {
        return false;
    }

    // Remove '0x' if need
    char *colun = NULL;
    if(NULL != (colun = strstr((char *)ck, "0x"))) {
        ck = ck + 2;
    }

    PKMM_LOGD("IPSEC_INFO_ADD_IND:ck: %s %zu", ck, out_Len);

    const uint8_t* ik = NULL;
    if (NULL == (ik = decoder.GetEMFieldBuf(msgID, data, len, "ik[0]", 1, out_Len))) {
        return false;
    }

    // Remove '0x' if need
    if(NULL != (colun = strstr((char *)ik, "0x"))) {
        ik = ik+ 2;
    }

    PKMM_LOGD("IPSEC_INFO_ADD_IND:ik: %s %d", ik, out_Len);

    size_t out_len = 0;
    bool is_done = false;

    SecurityInfo *obj = NULL;
    EspInfo *p = NULL;
    SecurityInfo *objs[4] = {0};

    const uint8_t* ipsec_info = NULL;
    int spi = 0;

    char name[25] = {0};
    for(int i = 0; i < 4; i++)  {
        is_done = false;
        obj = new SecurityInfo;
        obj->encType = EncryptionTypeEspImc;
        p = (EspInfo *)(&(obj->data.esp));

        while(1) {

            p->ip_family = 0; // IPv4
            p->index = index[0] & 0xFF;

            ipsec_info = NULL;
            memset(name,0,25);
            out_len = 0;
            snprintf(name, sizeof(name), "ipsec_info[%d].src_ip[0]", i);
            if (NULL == (ipsec_info = decoder.GetEMFieldBuf(msgID, data, len, name, 64, out_len))) {
                break;
            }

            snprintf(p->src_addr, sizeof(p->src_addr), "%s", ipsec_info);
            PKMM_LOGD("IPSEC_INFO_ADD_IND::ipsec_info[%d].src_ip: %s %d", i, (char *)ipsec_info
                                                                         , (int)out_len);

            ipsec_info = NULL;
            memset(name,0,25);
            out_len = 0;
            snprintf(name, sizeof(name), "ipsec_info[%d].dst_ip[0]", i);
            if (NULL == (ipsec_info = decoder.GetEMFieldBuf(msgID, data, len, name, 64, out_len))) {
                break;
            }

            snprintf(p->dst_addr, sizeof(p->dst_addr), "%s", ipsec_info);
            PKMM_LOGD("IPSEC_INFO_ADD_IND::ipsec_info[%d].dist_ip: %s %d", i,(char *)ipsec_info
                                                                              , (int)out_len);

            ipsec_info = NULL;
            memset(name,0,25);
            out_len = 0;
            snprintf(name, sizeof(name), "ipsec_info[%d].spi[0]", i);
            if (NULL == (ipsec_info = decoder.GetEMFieldBuf(msgID, data, len, name, 32, out_len))) {
                break;
            }

            spi = atoi((char *)ipsec_info);
            snprintf(p->spi, sizeof(p->spi), "%08X", spi);
            PKM_LOGD("IPSEC_INFO_ADD_IND::ipsec_info[%d].spi: %s %d", i, (char *)(p->spi)
                                                                       , (int)out_len);

            // Set Encryption Algorithm (3des_cbc/aes_cbc)
            snprintf(p->enc_algo_name, sizeof(p->enc_algo_name), "%s", enc_algo->enc_algo_name);

            // Set Integrity ESP (hmac/hmac96)
            snprintf(p->int_algo_esp, sizeof(p->int_algo_esp), "%s", auth_algo->int_algo_esp);

            // Set Integrity Digest Algorithm (sha1/md5)
            snprintf(p->int_algo_ikev2, sizeof(p->int_algo_ikev2), "%s", auth_algo->int_algo_ikev2);

            // Set Authentication Data Block Size
            p->auth_block = auth_algo->size;

            // Set Encrypt Key
            snprintf(p->enc_key, sizeof(p->enc_key), "%s", ck);

            // Set Integrity Key
            snprintf(p->int_key, sizeof(p->int_key), "%s", ik);

            // Set Block size (Initial Vector Size)
            p->block = block;

            is_done = true;
            break;

        }

        if(is_done) {
            objs[i] = obj;
        }
        else {
            delete(obj);
            obj = NULL;
        }
    }

    for(int j = 0; j < 4; j++) {
        if(objs[j] != NULL) {
            mSa->addInfo(EncryptionTypeEspImc, objs[j]);
            PKM_LOGD("decodeEmImcIpsecInfoAdd: spi = %s", objs[j]->data.esp.spi);

            if(mCallback != NULL) {
                mCallback->onAddKeyEvent(EncryptionTypeEsp, objs[j]->data.esp.spi, NULL);
            }
        }
    }

    return true;
}

bool MdmReceiver::decodeEmImcIpsecInfoDelete(MSGID msgID, const uint8_t *data, size_t len) {
    PKM_LOGD("decodeEmImcIpsecInfoDelete: %d, len = %d", (int)msgID, (int)len);
    UNUSED(len);

    FrameDecoder &decoder = *(FrameDecoder::GetInstance());

    // INDEX
    int64_t index = 0;
    if(!decoder.GetEMFieldVal(msgID, data, 1, "index", false, index)) {
        PKM_LOGE("decodeEmImcIpsecInfoDelete INDEX fail");
        return false;
    }

    mSa->removeInfoByIndex(EncryptionTypeEspImc, index);
    return true;
}

const auth_algo_struct* MdmReceiver::findAuthAlgo(EncryptionType encType, int id)
{
    int i, arrSize;
    if(EncryptionTypeEspImc == encType) {
        for (i = 0, arrSize = sizeof(mImcIpsecAuthAlgos) / sizeof(mImcIpsecAuthAlgos[0]);
             i < arrSize; i++)
        {
            if (mImcIpsecAuthAlgos[i].id == id)
                return &mImcIpsecAuthAlgos[i];
        }
    }
    else {
        for (i = 0, arrSize = sizeof(mAuthAlgos) / sizeof(mAuthAlgos[0]); i < arrSize; i++) {
            if (mAuthAlgos[i].id == id)
                return &mAuthAlgos[i];
        }
    }

    PKMM_LOGD("findAuthAlgo: type = %d, id = %d", encType, id);

    return NULL;
}

const enc_algo_struct* MdmReceiver::findEncAlgo(EncryptionType encType, int id, int size)
{
    int i, arrSize;

    if(EncryptionTypeEspImc == encType) {
        for (i = 0, arrSize = sizeof(mImcIpsecEncAlgos) / sizeof(mImcIpsecEncAlgos[0]);
             i < arrSize; i++)
        {
            if (mImcIpsecEncAlgos[i].id == id)
                return &mImcIpsecEncAlgos[i];
        }
    }
    else if(EncryptionTypeEsp == encType) {
        for (i = 0, arrSize = sizeof(mEspEncAlgos) / sizeof(mEspEncAlgos[0]); i < arrSize; i++) {
            if ((mEspEncAlgos[i].id == id) && (mEspEncAlgos[i].size == size))
                return &mEspEncAlgos[i];
        }
    }
    else {
        for (i = 0, arrSize = sizeof(mIkeEncAlgos) / sizeof(mIkeEncAlgos[0]); i < arrSize; i++) {
            if ((mIkeEncAlgos[i].id == id) && (mIkeEncAlgos[i].size == size))
                return &mIkeEncAlgos[i];
        }
    }

    PKMM_LOGD("findEncAlgo: type = %d, id = %d, size = %d", encType, id, size);

    return NULL;
}


