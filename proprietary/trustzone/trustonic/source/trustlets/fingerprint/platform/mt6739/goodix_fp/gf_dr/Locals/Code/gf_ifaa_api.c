/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#include "tlStd.h"
#include "TlApiDriver.h"
#include "ifaa_gf_interface.h"
#include "gf_error.h"
#include "gf_dr_common.h"
#include "gf_dr.h"

typedef uint32_t TEE_Result;
extern TEE_Result gf_alipay_api_get_auth_id(int32_t* finger_id);
#define TEE_SUCCESS                      ((TEE_Result)0x00000000)

/**
 * previous the type of finger print id is int32_t, so the pIdBuf should be larger than FINGER_ID_LENGTH
 *
 * [out]
 * @param pIdBuf:length of pIdBuf should be larger than FINGER_ID_LENGTH
 *
 * [in out]
 * @param pBufLen: the length of pIdBuf should be larger than FINGER_ID_LENGTH
 *
 * @return:
 *        IFAA_ERR_BAD_PARAM
 *        IFAA_ERR_GET_LAST_IDENTIFIED_RESULT
 *        IFAA_ERR_SUCCESS
 */
IFAA_Result ifaa_gf_getFpLastIdentifiedResult(uint8_t *pIdBuf, uint32_t *pBufLen) {
    IFAA_Result ret = IFAA_ERR_SUCCESS;
    int32_t id = 0;

    do {
        TEE_Result teeResult;
        if (NULL == pIdBuf) {
            ret = IFAA_ERR_BAD_PARAM;
            break;
        }

        if (NULL == pBufLen || *pBufLen < sizeof(int32_t)) {
            ret = IFAA_ERR_BAD_PARAM;
            break;
        }

        teeResult = gf_alipay_api_get_auth_id(&id);
        if (TEE_SUCCESS != teeResult) {
            ret = IFAA_ERR_GET_LAST_IDENTIFIED_RESULT;
            break;
        }

        memcpy(pIdBuf, &id, sizeof(int32_t));
        *pBufLen = sizeof(int32_t);
    } while (0);

    return ret;
}

/**
 * get TA version
 * [out]
 * @pVersionBuf
 *
 * [in out]
 * @pBufLen: length of pVersionBuf, *pBufLen >= TA_VERSION_INFO_LEN
 *
 * @return:
 *        IFAA_ERR_BAD_PARAM
 *        IFAA_ERR_UNKNOWN
 *        IFAA_ERR_SUCCESS
 */
IFAA_Result ifaa_gf_getAuthenticatorVersion(int32_t *version) {
    IFAA_Result ret = IFAA_ERR_SUCCESS;

    do {
        if (NULL == version) {
            ret = IFAA_ERR_BAD_PARAM;
            break;
        }

        gf_dr_cmd_t cmd = { 0 };
        cmd.cmd_id = GF_DR_CMD_IFAA_GET_AUTHENTICATOR_VERSION;
        tlApiResult_t tlResult = tlApi_callDriver(DRIVER_ID, &cmd);
        if (tlResult != TLAPI_OK) {
            ret = IFAA_ERR_BAD_PARAM;
            break;
        }

        ret = IFAA_ERR_SUCCESS;
        *version = cmd.data.get_authenticator_version.version;
    } while (0);

    return ret;
}

/**
 * max finger count is MAX_FINGERS_PER_USER,
 * finger id length is FINGER_ID_LENGTH,
 * type of id length is uint32_t
 * so MIN_GET_ID_LIST_BUFFER_LENGTH is (sizeof(uint32_t) + FINGER_ID_LENGTH) * MAX_FINGERS_PER_USER)
 *      +-------------------------------------+
 *      |id1_len | id_1 | id2_len | id_2 | ...|
 *      +-------------------------------------+
 * [out]
 * @pIdListBuf
 *
 * [in out]
 * @pBufLen: *pBufLen should be equal or larger than MIN_GET_ID_LIST_BUFFER_LENGTH
 */
IFAA_Result ifaa_gf_getIdList(uint8_t *pIdListBuf, uint32_t *pBufLen) {
    IFAA_Result ret = IFAA_ERR_SUCCESS;
    uint32_t i = 0;
    uint32_t len = sizeof(uint32_t);

    do {
        uint32_t *s = NULL;
        if (NULL == pIdListBuf || NULL == pBufLen) {
            ret = IFAA_ERR_BAD_PARAM;
            break;
        }

        gf_dr_cmd_t cmd = { 0 };
        cmd.cmd_id = GF_DR_CMD_IFAA_GET_FINGER_LIST;
        tlApiResult_t tlResult = tlApi_callDriver(DRIVER_ID, &cmd);
        if (tlResult != TLAPI_OK) {
            ret = IFAA_ERR_BAD_PARAM;
            break;
        }

        ret = IFAA_ERR_SUCCESS;
        s = (uint32_t *)cmd.data.get_finger_list.ids;
        while (i < cmd.data.get_finger_list.count) {
            /* length */
            memcpy(pIdListBuf, &len, sizeof(uint32_t));
            pIdListBuf += sizeof(uint32_t);

            /* id*/
            memcpy(pIdListBuf, s + i, sizeof(uint32_t));
            pIdListBuf += sizeof(uint32_t);

            i++;
        }

        *pBufLen = cmd.data.get_finger_list.count * sizeof(uint32_t) * 2;
    } while (0);

    return ret;
}

IFAABoolean ifaa_gf_compareId(const uint8_t *pBuf_l, const uint8_t *pBuf_r, uint32_t buf_len) {
    uint32_t i = 0;
    IFAABoolean ret = IFAA_YES;

    do {
        if (NULL == pBuf_l) {
            ret = IFAA_NO;
            break;
        }

        if (NULL == pBuf_r) {
            ret = IFAA_NO;
            break;
        }

        if (buf_len != sizeof(int32_t)) {
            ret = IFAA_NO;
            break;
        }

        while (i < buf_len) {
            if (*(pBuf_l + i) != *(pBuf_r + i)) {
                ret = IFAA_NO;
                break;
            }

            i++;
        }
    } while (0);

    return ret;
}
