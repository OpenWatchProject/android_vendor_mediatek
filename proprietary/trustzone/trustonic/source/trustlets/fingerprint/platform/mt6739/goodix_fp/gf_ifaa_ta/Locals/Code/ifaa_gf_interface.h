/*
 * Copyright (C) 2013-2017, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */

#ifndef __IFAA_GF_INTERFACE_H__
#define __IFAA_GF_INTERFACE_H__

#include "ifaa_ta_common.h"
#include "gf_type_define.h"

#ifdef __cplusplus__
extern C {
#endif

#ifndef MAX_FINGERS_PER_USER
#define MAX_FINGERS_PER_USER 10   // previous the max finger count;
#endif

#ifndef TA_VERSION_INFO_LEN
#define TA_VERSION_INFO_LEN 64    // max authenticator version length
#endif


#define FINGER_ID_LENGTH    sizeof(int32_t)

#define MIN_GET_ID_LIST_BUFFER_LENGTH    ((sizeof(uint32_t) + sizeof(int32_t)) * 10)

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
    IFAA_Result ifaa_gf_getFpLastIdentifiedResult(uint8_t *pIdBuf, uint32_t *pBufLen);

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
    IFAA_Result ifaa_gf_getAuthenticatorVersion(int32_t *version);

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
    IFAA_Result ifaa_gf_getIdList(uint8_t *pIdListBuf, uint32_t *pBufLen);

    IFAABoolean ifaa_gf_compareId(const uint8_t *pBuf_l, const uint8_t *pBuf_r, uint32_t buf_len);

#ifdef __cplusplus__
}
#endif

#endif  // __IFAA_GF_INTERFACE_H__
