#include <SpeechMessageID.h>

#include <errno.h>

#include <log/log.h>

#include <SpeechType.h>

#include <AudioAssert.h>



#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "SpeechMessageID"


namespace android {


uint16_t getSyncType(const uint16_t msg_id) {
    sph_msg_handshake_t handshake = SPH_MSG_HANDSHAKE_INVALID;

    switch (msg_id) {
    /**
     * =========================================================================
     *                     ap control
     * =========================================================================
     */
    /* ap bypass ack */
    case MSG_A2M_SPH_DL_DIGIT_VOLUME:
    case MSG_A2M_SPH_UL_DIGIT_VOLUME:
    case MSG_A2M_SPH_DL_ENH_REF_DIGIT_VOLUME:
    case MSG_A2M_MUTE_SPH_DL_CODEC:
    case MSG_A2M_SET_LPBK_POINT_DVT:
    case MSG_A2M_CTRL_SPH_ENH:
    case MSG_A2M_ENH_CTRL_SUPPORT:
    case MSG_A2M_CTM_DUMP_DEBUG_FILE:
    case MSG_A2M_BGSND_CONFIG:
    case MSG_A2M_VOIP_RX_CONFIG:
    case MSG_A2M_TELEPHONY_TX_CONFIG:
        handshake = SPH_MSG_HANDSHAKE_AP_CTRL_BYPASS_ACK;
        break;
    /* ap need ack */
    case MSG_A2M_MUTE_SPH_UL:
    case MSG_A2M_MUTE_SPH_DL:
    case MSG_A2M_MUTE_SPH_UL_SOURCE:
    case MSG_A2M_SPH_ON:
    case MSG_A2M_SPH_OFF:
    case MSG_A2M_SPH_DEV_CHANGE:
    case MSG_A2M_PNW_ON:
    case MSG_A2M_PNW_OFF:
    case MSG_A2M_VM_REC_ON:
    case MSG_A2M_VM_REC_OFF:
    case MSG_A2M_RECORD_RAW_PCM_ON:
    case MSG_A2M_RECORD_RAW_PCM_OFF:
    case MSG_A2M_CTM_ON:
    case MSG_A2M_CTM_OFF:
    case MSG_A2M_BGSND_ON:
    case MSG_A2M_BGSND_OFF:
    case MSG_A2M_EM_DYNAMIC_SPH:
    case MSG_A2M_DYNAMIC_PAR_IN_STRUCT_SHM:
    case MSG_A2M_VIBSPK_PARAMETER:
    case MSG_A2M_VOIP_RX_ON:
    case MSG_A2M_VOIP_RX_OFF:
    case MSG_A2M_TELEPHONY_TX_ON:
    case MSG_A2M_TELEPHONY_TX_OFF:
        handshake = SPH_MSG_HANDSHAKE_AP_CTRL_NEED_ACK;
        break;
    /* modem ack back */
    case MSG_M2A_MUTE_SPH_UL_ACK:
    case MSG_M2A_MUTE_SPH_DL_ACK:
    case MSG_M2A_MUTE_SPH_UL_SOURCE_ACK:
    case MSG_M2A_SPH_ON_ACK:
    case MSG_M2A_SPH_OFF_ACK:
    case MSG_M2A_SPH_DEV_CHANGE_ACK:
    case MSG_M2A_PNW_ON_ACK:
    case MSG_M2A_PNW_OFF_ACK:
    case MSG_M2A_VM_REC_ON_ACK:
    case MSG_M2A_VM_REC_OFF_ACK:
    case MSG_M2A_RECORD_RAW_PCM_ON_ACK:
    case MSG_M2A_RECORD_RAW_PCM_OFF_ACK:
    case MSG_M2A_CTM_ON_ACK:
    case MSG_M2A_CTM_OFF_ACK:
    case MSG_M2A_BGSND_ON_ACK:
    case MSG_M2A_BGSND_OFF_ACK:
    case MSG_M2A_EM_DYNAMIC_SPH_ACK:
    case MSG_M2A_DYNAMIC_PAR_IN_STRUCT_SHM_ACK:
    case MSG_M2A_VIBSPK_PARAMETER_ACK:
    case MSG_M2A_SMARTPA_PARAMETER_ACK:
    case MSG_M2A_VOIP_RX_ON_ACK:
    case MSG_M2A_VOIP_RX_OFF_ACK:
    case MSG_M2A_TELEPHONY_TX_ON_ACK:
    case MSG_M2A_TELEPHONY_TX_OFF_ACK:
        handshake = SPH_MSG_HANDSHAKE_MD_ACK_BACK_AP_CTRL;
        break;

    /**
     * =========================================================================
     *                     md control
     * =========================================================================
     */
    /* md bypass ack */
    case MSG_M2A_NETWORK_STATUS_NOTIFY:
    case MSG_M2A_EM_DATA_REQUEST:
        handshake = SPH_MSG_HANDSHAKE_MD_CTRL_BYPASS_ACK;
        break;
    /* md need ack */
    case MSG_M2A_EPOF_NOTIFY:
    case MSG_M2A_MD_ALIVE:
    case MSG_M2A_NW_CODEC_INFO_NOTIFY:
        handshake = SPH_MSG_HANDSHAKE_MD_CTRL_NEED_ACK;
        break;
    /* ap ack back */
    case MSG_A2M_EPOF_ACK:
    case MSG_A2M_MD_ALIVE_ACK_BACK:
    case MSG_A2M_NW_CODEC_INFO_READ_ACK:
        handshake = SPH_MSG_HANDSHAKE_AP_ACK_BACK_MD_CTRL;
        break;

    /**
     * =========================================================================
     *                     ap data
     * =========================================================================
     */
    /* modem request data */
    case MSG_M2A_PNW_DL_DATA_REQUEST:
    case MSG_M2A_BGSND_DATA_REQUEST:
    case MSG_M2A_VOIP_RX_DL_DATA_REQUEST:
    case MSG_M2A_VOIP_RX_UL_DATA_REQUEST:
    case MSG_M2A_TELEPHONY_TX_UL_DATA_REQUEST:
        handshake = SPH_MSG_HANDSHAKE_MD_REQUEST_DATA;
        break;
    /* ap notify data */
    case MSG_A2M_PNW_DL_DATA_NOTIFY:
    case MSG_A2M_BGSND_DATA_NOTIFY:
    case MSG_A2M_VOIP_RX_DL_DATA_NOTIFY:
    case MSG_A2M_VOIP_RX_UL_DATA_NOTIFY:
    case MSG_A2M_TELEPHONY_TX_UL_DATA_NOTIFY:
        handshake = SPH_MSG_HANDSHAKE_AP_NOTIFY_DATA;
        break;

    /**
     * =========================================================================
     *                     md data
     * =========================================================================
     */
    /* md notify data */
    case MSG_M2A_PNW_UL_DATA_NOTIFY:
    case MSG_M2A_CTM_DEBUG_DATA_NOTIFY:
    case MSG_M2A_VM_REC_DATA_NOTIFY:
    case MSG_M2A_RAW_PCM_REC_DATA_NOTIFY:
    case MSG_M2A_CUST_DUMP_NOTIFY:
        handshake = SPH_MSG_HANDSHAKE_MD_NOTIFY_DATA;
        break;
    /* ap read data done */
    case MSG_A2M_PNW_UL_DATA_READ_ACK:
    case MSG_A2M_CTM_DEBUG_DATA_READ_ACK:
    case MSG_A2M_VM_REC_DATA_READ_ACK:
    case MSG_A2M_RAW_PCM_REC_DATA_READ_ACK:
    case MSG_A2M_CUST_DUMP_READ_ACK:
        handshake = SPH_MSG_HANDSHAKE_AP_READ_DATA_DONE;
        break;
    /**
     * =========================================================================
     *                     error handling
     * =========================================================================
     */
    default:
        ALOGD("%s(), not supported msg_id 0x%x", __FUNCTION__, msg_id);
        handshake = SPH_MSG_HANDSHAKE_INVALID;
    }

    return handshake;
}


bool isApMsgBypassQueue(const struct sph_msg_t *p_sph_msg) {
    bool retval = false;

    if (p_sph_msg == NULL) {
        ALOGW("%s(), p_sph_msg == NULL!! return", __FUNCTION__);
        return 0;
    }

    switch (getSyncType(p_sph_msg->msg_id)) {
    case SPH_MSG_HANDSHAKE_AP_ACK_BACK_MD_CTRL:
    case SPH_MSG_HANDSHAKE_AP_NOTIFY_DATA:
    case SPH_MSG_HANDSHAKE_AP_READ_DATA_DONE:
        retval = true;
        break;
    default:
        retval = false;
    }

    return retval;
}


bool isApNeedAck(const struct sph_msg_t *p_sph_msg) {
    if (p_sph_msg == NULL) {
        ALOGW("%s(), p_sph_msg == NULL!! return", __FUNCTION__);
        return false;
    }

    return (getSyncType(p_sph_msg->msg_id) == SPH_MSG_HANDSHAKE_AP_CTRL_NEED_ACK);
}

bool isMdAckBack(const struct sph_msg_t *p_sph_msg) {
    if (p_sph_msg == NULL) {
        ALOGW("%s(), p_sph_msg == NULL!! return", __FUNCTION__);
        return false;
    }

    return (getSyncType(p_sph_msg->msg_id) == SPH_MSG_HANDSHAKE_MD_ACK_BACK_AP_CTRL);
}


bool isApMsg(const struct sph_msg_t *p_sph_msg) {
    if (p_sph_msg == NULL) {
        ALOGW("%s(), p_sph_msg == NULL!! return", __FUNCTION__);
        return false;
    }

    return ((p_sph_msg->msg_id & 0xFF00) == 0x2F00);
}


bool isMdMsg(const struct sph_msg_t *p_sph_msg) {
    if (p_sph_msg == NULL) {
        ALOGW("%s(), p_sph_msg == NULL!! return", __FUNCTION__);
        return false;
    }

    return ((p_sph_msg->msg_id & 0xFF00) == 0xAF00);
}


int makeFakeMdAckMsgFromApMsg(struct sph_msg_t *p_sph_msg) {
    int retval = 0;

    if (p_sph_msg == NULL) {
        ALOGW("%s(), p_sph_msg == NULL!! return", __FUNCTION__);
        return -EFAULT;
    }

    if (isApMsg(p_sph_msg) == false) {
        ALOGW("%s(), 0x%x not AP msg!! return", __FUNCTION__, p_sph_msg->msg_id);
        return -EINVAL;
    }

    if (getSyncType(p_sph_msg->msg_id) != SPH_MSG_HANDSHAKE_AP_CTRL_NEED_ACK) {
        ALOGW("%s(), 0x%x no need ack!! return", __FUNCTION__, p_sph_msg->msg_id);
        return -EINVAL;
    }

    p_sph_msg->msg_id |= 0x8000;
    if (getSyncType(p_sph_msg->msg_id) != SPH_MSG_HANDSHAKE_MD_ACK_BACK_AP_CTRL) {
        ALOGE("%s(), 0x%x not MD msg ack!!", __FUNCTION__, p_sph_msg->msg_id);
        ASSERT(getSyncType(p_sph_msg->msg_id) == SPH_MSG_HANDSHAKE_MD_ACK_BACK_AP_CTRL);
        retval = -EBADMSG;
    }

    return retval;
}


bool isAckMessageInPair(const struct sph_msg_t *p_sph_msg,
                        const struct sph_msg_t *p_sph_msg_ack) {
    if (p_sph_msg == NULL) {
        ALOGW("%s(), p_sph_msg == NULL!! return", __FUNCTION__);
        return false;
    }
    if (p_sph_msg_ack == NULL) {
        ALOGW("%s(), p_sph_msg_ack == NULL!! return", __FUNCTION__);
        return false;
    }

    if (isApMsg(p_sph_msg) && isMdMsg(p_sph_msg_ack)) {
        return ((p_sph_msg->msg_id | 0x8000) == p_sph_msg_ack->msg_id);
    } else {
        return false;
    }
}


bool isAckMessageInPairByID(const uint16_t ap_msg_id, const uint16_t md_msg_id) {
    /* for ack match when AP recovery */
    if ((ap_msg_id & 0xFF00) == 0x2F00 && (md_msg_id & 0xFF00) == 0xAF00) {
        return ((ap_msg_id | 0x8000) == md_msg_id);
    } else {
        return false;
    }
}


uint32_t getBlockThreadTimeMsByID(struct sph_msg_t *p_sph_msg) {
    uint32_t block_thread_ms = 0;
    if (p_sph_msg == NULL) {
        ALOGW("%s(), p_sph_msg == NULL!! return", __FUNCTION__);
        return 0;
    }

    switch (p_sph_msg->msg_id) {
    case MSG_A2M_SPH_ON:
    case MSG_A2M_SPH_OFF:
        block_thread_ms = 500;
        break;
    case MSG_A2M_SPH_DEV_CHANGE:
        block_thread_ms = 300;
        break;
    case MSG_A2M_BGSND_ON:
    case MSG_A2M_BGSND_OFF:
    case MSG_A2M_VOIP_RX_ON:
    case MSG_A2M_VOIP_RX_OFF:
    case MSG_A2M_TELEPHONY_TX_ON:
    case MSG_A2M_TELEPHONY_TX_OFF:
        block_thread_ms = 100;
        break;
    case MSG_A2M_VM_REC_ON:
    case MSG_A2M_VM_REC_OFF:
        block_thread_ms = 300;
        break;
    case MSG_A2M_EM_DYNAMIC_SPH:
    case MSG_A2M_DYNAMIC_PAR_IN_STRUCT_SHM:
    case MSG_A2M_VIBSPK_PARAMETER:
        block_thread_ms = 300;
        break;
    case MSG_A2M_CTRL_SPH_ENH:
    case MSG_A2M_ENH_CTRL_SUPPORT:
        block_thread_ms = 0;
        break;
    case MSG_A2M_PNW_ON:
    case MSG_A2M_PNW_OFF:
    case MSG_A2M_RECORD_RAW_PCM_ON:
    case MSG_A2M_RECORD_RAW_PCM_OFF:
    case MSG_A2M_CTM_ON:
    case MSG_A2M_CTM_OFF:
        block_thread_ms = 50;
        break;
    case MSG_A2M_EPOF_ACK:
        block_thread_ms = 50;
        break;
    case MSG_A2M_SET_LPBK_POINT_DVT:
        block_thread_ms = 50;
        break;
    case MSG_A2M_CTM_DUMP_DEBUG_FILE:
        block_thread_ms = 50;
        break;
    case MSG_A2M_PNW_DL_DATA_NOTIFY:
    case MSG_A2M_BGSND_DATA_NOTIFY:
    case MSG_A2M_VOIP_RX_DL_DATA_NOTIFY:
    case MSG_A2M_VOIP_RX_UL_DATA_NOTIFY:
        block_thread_ms = 0;
        break;
    case MSG_A2M_MUTE_SPH_UL:
    case MSG_A2M_MUTE_SPH_DL:
    case MSG_A2M_MUTE_SPH_UL_SOURCE:
    case MSG_A2M_MUTE_SPH_DL_CODEC:
        block_thread_ms = 50;
        break;
    case MSG_A2M_PNW_UL_DATA_READ_ACK:
    case MSG_A2M_CTM_DEBUG_DATA_READ_ACK:
    case MSG_A2M_VM_REC_DATA_READ_ACK:
    case MSG_A2M_RAW_PCM_REC_DATA_READ_ACK:
    case MSG_A2M_CUST_DUMP_READ_ACK:
    case MSG_A2M_NW_CODEC_INFO_READ_ACK:
        block_thread_ms = 0;
        break;
    case MSG_A2M_SPH_DL_DIGIT_VOLUME:
    case MSG_A2M_SPH_UL_DIGIT_VOLUME:
    case MSG_A2M_SPH_DL_ENH_REF_DIGIT_VOLUME:
    case MSG_A2M_BGSND_CONFIG:
    case MSG_A2M_VOIP_RX_CONFIG:
    case MSG_A2M_TELEPHONY_TX_CONFIG:
        block_thread_ms = 0;
        break;

    default:
        ALOGW("%s(), non-block for msg_id 0x%x", __FUNCTION__, p_sph_msg->msg_id);
        block_thread_ms = 0;
    }

    return block_thread_ms;
}



bool isNeedDumpMsg(const struct sph_msg_t *p_sph_msg) {
    bool retval = true;

    if (p_sph_msg == NULL) {
        ALOGW("%s(), p_sph_msg == NULL!! return", __FUNCTION__);
        return 0;
    }

    switch (getSyncType(p_sph_msg->msg_id)) {
    /* Do not print msg w/o ack!! */
    case SPH_MSG_HANDSHAKE_AP_CTRL_BYPASS_ACK:
    case SPH_MSG_HANDSHAKE_MD_CTRL_BYPASS_ACK:
        retval = false;
        break;
    /* Do not print data message!! */
    case SPH_MSG_HANDSHAKE_AP_NOTIFY_DATA:
    case SPH_MSG_HANDSHAKE_AP_READ_DATA_DONE:
    case SPH_MSG_HANDSHAKE_MD_REQUEST_DATA:
    case SPH_MSG_HANDSHAKE_MD_NOTIFY_DATA:
        retval = false;
        break;
    default:
        retval = true;
    }

#ifndef CONFIG_MT_ENG_BUILD
    /* reduce log */
    if (p_sph_msg->msg_id == MSG_A2M_MUTE_SPH_UL ||
        p_sph_msg->msg_id == MSG_A2M_MUTE_SPH_DL ||
        p_sph_msg->msg_id == MSG_A2M_MUTE_SPH_UL_SOURCE) {
        retval = false;
    }
#endif

    return retval;
}




} /* end namespace android */

