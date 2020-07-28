/*
* Copyright (c) 2014 - 2016 MediaTek Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files
* (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/**
 * @file   tlDriverApi.c
 * @brief  Implements driver APIs
 *
 * The APIs allow trustlets to make requests to the driver
 *
 */
#include <tlStd.h>
#include <tee_type.h>
#include <tee_error.h>
#include <tee_internal_api.h>


#include "tlRpmbDriverApi.h"
#include "drRpmbOps.h"

#define TATAG "TA RPMB "
#define RPMB_DRV_ID DRIVER_ID

/* Debug message event */
#define DBG_NONE        (0)       /* No event */
#define DBG_CMD         (1 << 0)  /* SEC CMD related event */
#define DBG_FUNC        (1 << 1)  /* SEC function event */
#define DBG_INFO        (1 << 2)  /* SEC information event */
#define DBG_DATA        (1 << 3)  /* SEC CMD related event */
#define DBG_DBG         (1 << 29)  /* SEC information event */
#define DBG_WRN         (1 << 30) /* Warning event */
#define DBG_ERR         (1 << 31) /* Error event */
#define DBG_ALL         (0xffffffff)

#define DBG_MASK        (DBG_ALL)

#define DBGOUT(evt, fmt, args...) \
do {    \
	if ((DBG_##evt) & DBG_MASK) { \
		TEE_LogPrintf("[Driver rpmb Api:%s] "fmt, __func__, ##args); \
	}   \
} while(0)

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

//according to the emmc spec, at most ext_csd[222]*512/256 blocks can be written to rpmb in one RPMB write request.
//most emmc can only support write 2 rpmb blocks one time(ext_csd[222] -> 0x01)
//the max value can not be bigger than MAX_RPMB_TRANSFER_BLK(16).
#define MAX_RPMB_WRITE_BLKS_ONE_TIME   MAX_RPMB_TRANSFER_BLK

static tlApiRpmb_t RpmbData;

uint16_t cpu_to_be16p(uint16_t *p)
{
    return (((*p << 8)&0xFF00) | (*p >> 8));
}

uint32_t cpu_to_be32p(uint32_t *p)
{
    return (((*p & 0xFF) << 24) | ((*p & 0xFF00) << 8) | ((*p & 0xFF0000) >> 8) | (*p & 0xFF000000) >> 24 );
}

static int rpmb_handle(uint32_t uid)
{
    int drv_handle;
    int tlRet;

    DBGOUT(INFO, "Process with rpmb operation tpye(%d) in SWd", RpmbData.commandId);
    do {
        drv_handle = mdrv_open(RPMB_DRV_ID, NULL);
        tlRet = mdrv_ioctl(drv_handle, RpmbData.commandId, &RpmbData);
        mdrv_close(drv_handle);
        break;
    } while (0);

    return tlRet;
}

/**
 * Open session to the driver with given data
 *
 * @return  session id
 */
_TLAPI_EXTERN_C uint32_t TEE_RpmbOpenSession(uint32_t uid)
{
    int ret;

    DBGOUT(INFO, "start\n");

    RpmbData.uid = uid;

    if (RpmbData.uid > RPMB_MAX_USER_NUM) {
        TEE_DbgPrintLnf(TATAG "User id:%d is beyond max user number\n", RpmbData.uid);
        return 0xFFFFFFFF;
    }
    RpmbData.commandId = FID_DRV_OPEN_SESSION;
    ret = rpmb_handle(uid);
    if (ret == TLAPI_OK) {
        if (-1 == RpmbData.ret) {
            DBGOUT(INFO, "Failed\n");
            return 0xFFFFFFFF;
        }
    }

    DBGOUT(INFO, "end\n");
    return uid;
}

/**
 * Close session
 *
 * @param sid  session id
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C TEE_Result TEE_RpmbCloseSession(
    uint32_t sid)
{
    int ret;

    DBGOUT(INFO, "start");
    RpmbData.commandId = FID_DRV_CLOSE_SESSION;
    ret = rpmb_handle(sid);
    if (ret == TLAPI_OK) {
        if (-1 == RpmbData.ret) {
            DBGOUT(INFO, "Failed\n");
            return 0xFFFFFFFF;
        }
    }
    DBGOUT(INFO, "end");
    return 0;
}

TEE_Result TEE_HmacSha256(const uint8_t* mac_key, const uint8_t* message,
                          size_t message_length, uint8_t* signature, size_t* signature_length)
{

    TEE_Result ret;
    TEE_ObjectHandle hmac_key = (TEE_ObjectHandle)NULL;
    TEE_OperationHandle OperationHandle = (TEE_OperationHandle)NULL;
    TEE_Attribute Attribute;
    uint32_t keySize = 256;

    DBGOUT(INFO, "start!!\n");

    do {

        ret = TEE_AllocateTransientObject(TEE_TYPE_HMAC_SHA256, keySize, &hmac_key);
        if (ret) {
            DBGOUT(ERR, "Failed to alloc transient object handle : 0x%x", ret);
            break;
        }

        TEE_InitRefAttribute(&Attribute, TEE_ATTR_SECRET_VALUE, mac_key, 32);

        ret = TEE_PopulateTransientObject(hmac_key, &Attribute, 1);
        if (ret) {
            DBGOUT(ERR, "TEE_PopulateTransientObject error, ret=%x\n", ret);
            break;
        }

        ret = TEE_AllocateOperation(&OperationHandle,
                                    TEE_ALG_HMAC_SHA256,
                                    TEE_MODE_MAC,
                                    keySize
                                   );
        if (ret) {
            DBGOUT(ERR, "TEE_AllocateOperation error, ret=%x\n", ret);
            break;
        }

        ret = TEE_SetOperationKey(OperationHandle, hmac_key);
        if (ret) {
            DBGOUT(ERR, "TEE_SetOperationKey error, ret=%x\n", ret);
            break;
        }

        TEE_MACInit(OperationHandle, NULL, 0);

        ret = TEE_MACComputeFinal(OperationHandle,
                                  message,
                                  message_length,
                                  signature,
                                  signature_length
                                 );
        if (ret) {
            DBGOUT(ERR, "TEE_MACComputeFinal error, ret=%x\n", ret);
            break;
        }

    } while (0);

    TEE_FreeOperation(OperationHandle);
    TEE_FreeTransientObject(hmac_key);

    DBGOUT(INFO, "end!!\n");

    return ret;
}

_TLAPI_EXTERN_C TEE_Result TEE_RpmbReadDatabyOffset(
    uint32_t sid,
    uint32_t offset,
    uint8_t *buf,
    uint32_t bufSize,
    int *result)
{
    TEE_Result ret = TEE_SUCCESS;
    uint8_t hmac[RPMB_SZ_MAC], *dataBuf, *dataBuf_ptr;
    uint8_t nonce[RPMB_SZ_NONCE];
    size_t hmac_len = RPMB_SZ_MAC, nonce_len = RPMB_SZ_NONCE;
    uint16_t type = RPMB_READ_DATA;
    uint16_t iCnt, total_blkcnt, tran_blkcnt, left_blkcnt, rTotal, rType, rWC, rAddr, start_blkIdx;
    const uint8_t *mac_key;
    uint32_t tran_size, left_size = bufSize, align_size;
    uint32_t offset_in_blk = offset % RPMB_SZ_DATA;
    int i = 0;
    struct rpmb_t *frame;

    DBGOUT(INFO, "start!!\n");

    *result = 0;
    TEE_MemFill(RpmbData.buf, 0, 512);
    /* get rpmb size for this session. */
    RpmbData.commandId = FID_DRV_GET_PART_SIZE;
    ret = rpmb_handle(sid);
    if (ret) {
        DBGOUT(ERR, "rpmb_handle get part size error (%x)\n", ret);
        *result = -1;
        return ret;
    }

    DBGOUT(DBG, "session: %d, part_size:0x%x \n", sid, RpmbData.part_size);

    /* check parameters valid. */
    if (buf == NULL || bufSize == 0 || ((offset + bufSize) > RpmbData.part_size)) {
        DBGOUT(ERR, "Check parameters invalid!!\n");
        *result = -1;
        return TEE_ERROR_BAD_PARAMETERS;
    }
    /* get key */
    RpmbData.commandId = FID_DRV_GET_KEY;
    ret = rpmb_handle(sid);
    if (ret) {
        DBGOUT(ERR, "rpmb_handle get key error (%x)\n", ret);
        return TEE_ERROR_GENERIC;
    }
    mac_key = RpmbData.mac_key;
    /* get partition address */
    RpmbData.commandId = FID_DRV_GET_PART_ADDR;
    ret = rpmb_handle(sid);
    if (ret) {
        DBGOUT(ERR, "rpmb_handle get address error (%x)\n", ret);
        return TEE_ERROR_GENERIC;
    }
    start_blkIdx = RpmbData.part_addr + offset / RPMB_SZ_DATA;
    left_size = align_size = offset_in_blk + bufSize;

    DBGOUT(DBG, "session: %d, part_addr:0x%x \n", sid, RpmbData.part_addr);

#if RPMB_MULTI_BLOCK_ACCESS

    left_blkcnt = total_blkcnt = ((align_size % RPMB_SZ_DATA) ? (align_size/RPMB_SZ_DATA + 1) : (align_size/RPMB_SZ_DATA));
    DBGOUT(DBG, "total_blkcnt = %x\n", total_blkcnt);
    while (left_blkcnt) {
        if (left_blkcnt >= MAX_RPMB_TRANSFER_BLK)
            tran_blkcnt = MAX_RPMB_TRANSFER_BLK;
        else
            tran_blkcnt = left_blkcnt;
        DBGOUT(DBG, "left_blkcnt=%x, left_size=%x\n", left_blkcnt, left_size);
        /*
         * initial buffer. (since HMAC computation of multi block needs multi buffer, pre-alloced it)
         */
        frame = (struct rpmb_t *)RpmbData.buf;
        TEE_MemFill(frame, 0, tran_blkcnt * 512);
        rType = cpu_to_be16p(&type);
        rAddr = cpu_to_be16p(&start_blkIdx);
        dataBuf_ptr = dataBuf = TEE_Malloc(tran_blkcnt*512, 0);
        if (!dataBuf) {
            DBGOUT(ERR, "tlApiMalloc error!!!\n");
            return TEE_ERROR_OUT_OF_MEMORY;
        }
        /*
         * STEP 1, prepare request read data frame. we need address and nonce.
         */
        frame->request = rType;
        frame->address = rAddr;

        TEE_GenerateRandom(nonce, nonce_len);
        TEE_MemMove(frame->nonce, nonce, RPMB_SZ_NONCE);
        /*
         * STEP 2, send read data request.
         */
        RpmbData.commandId = FID_DRV_READ_DATA;
        RpmbData.start_blk = start_blkIdx;
        RpmbData.blks = tran_blkcnt;
        ret = rpmb_handle(sid);
        if (ret) {
            DBGOUT(ERR, "rpmb_handle read data error (%x)\n", ret);
            TEE_Free(dataBuf_ptr);
            return TEE_ERROR_GENERIC;
        }
        /*
         * STEP 3, retrieve every data frame one by one.
         */
        for (iCnt = 0; iCnt < tran_blkcnt; iCnt++) {
            if (left_size >= RPMB_SZ_DATA)
                tran_size = RPMB_SZ_DATA;
            else
                tran_size = left_size;
            TEE_MemMove(dataBuf, frame->data, 284);
            dataBuf = dataBuf + 284;
            //
            // sorry, I shouldn't copy read data to user's buffer now, it should be later after checking no problem,
            // but for convenience...you know...
            //
            if ((i == 0) && (iCnt == 0)) /* first block, handle partial data. */
                TEE_MemMove(buf, frame->data + offset_in_blk, tran_size - offset_in_blk);
            else
                TEE_MemMove(buf - offset_in_blk + i * MAX_RPMB_TRANSFER_BLK * RPMB_SZ_DATA + (iCnt * RPMB_SZ_DATA),
                            frame->data,
                            tran_size);
            left_size -= tran_size;
            frame++;
        }
        frame--;

        /*
         * STEP 4. authenticate last frame result response.
         */
        ret = TEE_HmacSha256(mac_key, dataBuf_ptr, 284 * tran_blkcnt, hmac, &hmac_len);
        if (TEE_MemCompare(frame->mac, hmac, RPMB_SZ_MAC) != 0) {
            *result = 1;
            DBGOUT(ERR, "read data result hmac compare error.\n");
            ret = TEE_ERROR_SIGNATURE_INVALID;
            break;
        }
        if (TEE_MemCompare(frame->nonce, nonce, RPMB_SZ_NONCE) != 0) {
            *result = 1;
            DBGOUT(ERR, "read data nonce compare error!!!\n");
            ret = TEE_ERROR_GENERIC;
            break;
        }
        if (frame->result) {
            *result = cpu_to_be16p(&frame->result);
            DBGOUT(ERR, "read data result = %x\n", *result);
            ret = TEE_ERROR_GENERIC;
            break;
        }
        start_blkIdx += tran_blkcnt;
        left_blkcnt -= tran_blkcnt;
        i++;
        TEE_Free(dataBuf_ptr);
    };

    if (ret)
        TEE_Free(dataBuf_ptr);

    if (left_blkcnt || left_size) {
        DBGOUT(ERR, "left_blkcnt or left_size is not empty!!!!!!\n");
        *result = 1;
        return TEE_ERROR_GENERIC;
    }


#else // single block access.

    total_blkcnt = ((bufSize % RPMB_SZ_DATA) ? (bufSize/RPMB_SZ_DATA + 1) : (bufSize/RPMB_SZ_DATA));
    frame = (struct rpmb_t *)RpmbData.buf;
    rType = cpu_to_be16p(&type);

    for (iCnt = 0; iCnt < total_blkcnt; iCnt++) {
        TEE_MemFill(frame, 0, 512);
        /*
         * STEP 1: prepare read data request. we need nonce and address.
         */
        rAddr = cpu_to_be16p(&RpmbData.part_addr);
        frame->request = rType;
        frame->address = rAddr;
        TEE_GenerateRandom(nonce, nonce_len);
        TEE_MemMove(frame->nonce, nonce, RPMB_SZ_NONCE);

        RpmbData.commandId = FID_DRV_READ_DATA;
        RpmbData.blks = 1;
        ret = rpmb_handle(sid);
        if (ret) {
            DBGOUT(ERR, "rpmb_handle read data error (%x)\n", ret);
            return 1;
        }
        /*
         * STEP 2: retrieve read data response. we need check
         *         1. authenticate hmac is corrent or not.
         *         2. check nonce is the same or not.
         *         3. check result if no error or something else.
         */
        ret = TEE_HmacSha256(mac_key, frame->data, 284, hmac, &hmac_len);
        if (TEE_MemCompare(frame->mac, hmac, RPMB_SZ_MAC) != 0) {
            *result = 1;
            DBGOUT(ERR, "read data hmac compare error!!!\n");
            return TEE_ERROR_SIGNATURE_INVALID;
        }
        if (TEE_MemCompare(frame->nonce, nonce, RPMB_SZ_NONCE) != 0) {
            *result = 1;
            DBGOUT(ERR, "read data nonce compare error!!!\n");
            return TEE_ERROR_GENERIC;
        }
        if (frame->result) {
            *result = cpu_to_be16p(&frame->result);
            DBGOUT(ERR, "read data result error(%x)!!!\n", *result);
            return TEE_ERROR_GENERIC;
        }
        /*
         * STEP 3: finally, we can copy the read data to user's buffer.
         */
        if (left_size >= RPMB_SZ_DATA)
            tran_size = RPMB_SZ_DATA;
        else
            tran_size = left_size;
        TEE_MemMove(buf + RPMB_SZ_DATA * iCnt, frame->data, tran_size);
        left_size -= tran_size;
        RpmbData.start_addr++;
    };
#endif
    DBGOUT(INFO, "end!!");

    return TEE_SUCCESS;



}

_TLAPI_EXTERN_C TEE_Result TEE_RpmbWriteDatabyOffset(
    uint32_t sid,
    uint32_t offset,
    uint8_t *buf,
    uint32_t bufSize,
    int *result)
{
    TEE_Result ret = TEE_SUCCESS;
    const uint8_t *mac_key;
    uint8_t hmac[RPMB_SZ_MAC], *dataBuf, *dataBuf_ptr;
    uint8_t nonce[RPMB_SZ_NONCE];
    size_t hmac_len, nonce_len;
    uint16_t iCnt, type;
    uint16_t total_blkcnt, tran_blkcnt, left_blkcnt, rBlks, rType, rAddr;
    uint16_t first_blk_offset, last_blk_offset, start_blkIdx;
    uint32_t rWC, WC, tran_size, left_size, align_size;
    uint32_t offset_in_blk;
    struct rpmb_t *frame, *first_frame;
    int i = 0, retry = 10;
    uint8_t write_blks_one_time = 0;
    uint8_t last_block_buf[RPMB_SZ_DATA];
    uint8_t *boundary_buf, *tmp_user_buf, *tmp_user_buf_ptr;

    DBGOUT(INFO, "start!!\n");

_retry:
    /* init variables */
    hmac_len = RPMB_SZ_MAC;
    nonce_len = RPMB_SZ_NONCE;

    left_size = bufSize;
    offset_in_blk = offset % RPMB_SZ_DATA;
    TEE_MemFill(last_block_buf, 0, RPMB_SZ_DATA);

    boundary_buf = NULL;
    tmp_user_buf = NULL;
    tmp_user_buf_ptr = NULL;

    /*
     * STEP 0, initial default value.
     */
    *result = 0;
    TEE_MemFill(RpmbData.buf, 0, 512);

    /* get part size for this session. */
    RpmbData.commandId = FID_DRV_GET_PART_SIZE;
    ret = rpmb_handle(sid);
    if (ret) {
        DBGOUT(ERR, "get part size error (%x)\n", ret);
        *result = -1;
        return ret;
    }

    DBGOUT(DBG, "session: %d, part_size:0x%x \n", sid, RpmbData.part_size);
    /* check parameters valid. */
    if (buf == NULL || bufSize == 0 || ((offset + bufSize) > RpmbData.part_size)) {
        DBGOUT(ERR, "Check parameters invalid!!\n");
        *result = -1;
        return TEE_ERROR_BAD_PARAMETERS;
    }

    /*
     * we should consider that if the write data length is not half-block(256B) aligned,
     * the non aliged first block and last block should be read from rpmb first,
     * and then, modify the scope corresponding to the to be written data.
     */
    align_size = offset_in_blk + bufSize;

    if (offset_in_blk || (align_size % RPMB_SZ_DATA)) {

        boundary_buf = TEE_Malloc(bufSize + 2*RPMB_SZ_DATA, 0);
        if (!boundary_buf) {
            DBGOUT(ERR, "tlApiMalloc error!!!\n");
            *result = -1;
            return TEE_ERROR_OUT_OF_MEMORY;
        }
        TEE_MemFill(boundary_buf, 0, bufSize + 2*RPMB_SZ_DATA);
    }

    first_blk_offset = (offset / RPMB_SZ_DATA) * RPMB_SZ_DATA;
    last_blk_offset = ((offset + bufSize) / RPMB_SZ_DATA) * RPMB_SZ_DATA;

    DBGOUT(DBG, "Write Data, offset: %d, bufSize: %d\n", offset, bufSize);
    DBGOUT(DBG, "Write Data, first block offset: %d, last block offset: %d\n", first_blk_offset, last_blk_offset);

    if (offset_in_blk || (first_blk_offset == last_blk_offset)) {
        ret = TEE_RpmbReadDatabyOffset(sid, first_blk_offset, boundary_buf, RPMB_SZ_DATA, result);
        if (ret) {
            TEE_Free(boundary_buf);
            return ret;
        }
    }

    if (boundary_buf)
        TEE_MemMove(boundary_buf + offset_in_blk, buf, bufSize);

    if ((align_size % RPMB_SZ_DATA) && last_blk_offset > first_blk_offset) {
        ret = TEE_RpmbReadDatabyOffset(sid, last_blk_offset, last_block_buf, RPMB_SZ_DATA, result);
        if (ret) {
            TEE_Free(boundary_buf);
            return ret;
        }
        TEE_MemMove(boundary_buf + align_size,
                    last_block_buf + align_size%RPMB_SZ_DATA,
                    RPMB_SZ_DATA - align_size%RPMB_SZ_DATA);
    }

    if (boundary_buf) {
        align_size += (RPMB_SZ_DATA - align_size%RPMB_SZ_DATA);
        tmp_user_buf_ptr = boundary_buf;
    } else {
        align_size = bufSize;
        tmp_user_buf_ptr = buf;
    }

    RpmbData.commandId = FID_DRV_GET_KEY;
    ret = rpmb_handle(sid);
    if (ret) {
        DBGOUT(ERR, "rpmb_handle get key error (%x)\n", ret);
        return 1;
    }

    mac_key = RpmbData.mac_key;
#if RPMB_MULTI_BLOCK_ACCESS
    /*
     * For RPMB write data, the elements we need in the data frame is
     * 1. address.
     * 2. write counter.
     * 3. data.
     * 4. block count.
     * 5. MAC
     *
     */

    /*
     * STEP 1(Address), search partition table to find out where is my address.
     */
    RpmbData.commandId = FID_DRV_GET_PART_ADDR;
    ret = rpmb_handle(sid);
    if (ret) {
        DBGOUT(ERR, "rpmb_handle get address error (%x)\n", ret);
        return 1;
    }

    DBGOUT(DBG, "session: %d, part_addr:0x%x \n", sid, RpmbData.part_addr);

    start_blkIdx = RpmbData.part_addr + offset / RPMB_SZ_DATA;
    left_blkcnt = total_blkcnt = ((align_size % RPMB_SZ_DATA) ? (align_size/RPMB_SZ_DATA + 1) : (align_size/RPMB_SZ_DATA));
    left_size = align_size;

    DBGOUT(DBG, "total_blkcnt = %x\n", total_blkcnt);
    first_frame = (struct rpmb_t *)RpmbData.buf;
    /*
     * get emmc ext_csd[222] (EXT_CSD_REL_WR_SEC_C)
     */
    RpmbData.commandId = FID_DRV_GET_REL_WR_SEC_C;
    ret = rpmb_handle(sid);
    if (ret) {
        DBGOUT(ERR, "get REL_WR_SEC_C error (%x)\n", ret);
        //if (tmp_user_buf != NULL)
        //    tlApiFree(tmp_user_buf);
        *result = -1;
        return ret;
    }

    write_blks_one_time = MIN(MAX_RPMB_WRITE_BLKS_ONE_TIME, RpmbData.emmc_rel_wr_sec_c*2);
    DBGOUT(DBG, "write_blks_one_time: %d\n", write_blks_one_time);
    while (left_blkcnt) {
        if (left_blkcnt >= write_blks_one_time)
            tran_blkcnt = write_blks_one_time;
        else
            tran_blkcnt = left_blkcnt;

        if (start_blkIdx % 2)
            tran_blkcnt = 1;

        DBGOUT(DBG, "left_blkcnt=%x, left_size=%x\n", left_blkcnt, left_size);
        rAddr = cpu_to_be16p(&start_blkIdx);
        frame = (struct rpmb_t *)RpmbData.buf;
        TEE_MemFill(frame, 0, 512);

        /*
         * STEP 2-1(write counter), prepare get wcounter request. we only need nonce.
         */
        type = RPMB_GET_WRITE_COUNTER;
        rType = cpu_to_be16p(&type);
        frame->request = rType;

        TEE_GenerateRandom(nonce, nonce_len);
        TEE_MemMove(frame->nonce, nonce, nonce_len);

        RpmbData.commandId = FID_DRV_GET_WCOUNTER;
        ret = rpmb_handle(sid);
        if (ret) {
            DBGOUT(ERR, "rpmb_handle get wc error (%x)\n", ret);
            return TEE_ERROR_GENERIC;
        }
        /*
         * STEP 2-2(write counter), authenticate wc data frame is legal or not.
         */
        TEE_HmacSha256(mac_key, frame->data, 284, hmac, &hmac_len);

        if (TEE_MemCompare(frame->mac, hmac, RPMB_SZ_MAC) != 0) {
            DBGOUT(ERR, "Get wcounter hmac compare error.\n");
            *result = 1;
            return TEE_ERROR_SIGNATURE_INVALID;
        }
        if (TEE_MemCompare(frame->nonce, nonce, RPMB_SZ_NONCE) != 0) {
            *result = 1;
            DBGOUT(ERR, "Get wcounter nonce compare error!!!\n");
            return TEE_ERROR_GENERIC;
        }
        if (frame->result) {
            *result = cpu_to_be16p(&frame->result);
            DBGOUT(ERR, "Get wcounter result error(%x)!!!\n", *result);
            return TEE_ERROR_GENERIC;
        }
        rWC = frame->write_counter;
        WC = cpu_to_be32p(&rWC);
        /*
         * initial buffer. (since HMAC computation of multi block needs multi buffer, pre-alloced it)
         */
        TEE_MemFill(frame, 0, tran_blkcnt * 512);
        dataBuf_ptr = dataBuf = TEE_Malloc(tran_blkcnt*512, 0);
        if (!dataBuf) {
            DBGOUT(ERR, "tlApiMalloc error!!!\n");
            return TEE_ERROR_OUT_OF_MEMORY;
        }
        type = RPMB_WRITE_DATA;
        rType = cpu_to_be16p(&type);
        rBlks = cpu_to_be16p(&tran_blkcnt);

        /*
         * STEP 3(data), prepare every data frame one by one and hook HMAC to the last.
         */
        for (iCnt = 0; iCnt < tran_blkcnt; iCnt++) {

            frame->request = rType;
            frame->block_count = rBlks;
            frame->address = rAddr;
            frame->write_counter = rWC;
            TEE_MemFill(frame->nonce, 0, RPMB_SZ_NONCE);

            if (left_size >= RPMB_SZ_DATA)
                tran_size = RPMB_SZ_DATA;
            else
                tran_size = left_size;
            TEE_MemMove(frame->data,
                        tmp_user_buf_ptr,
                        tran_size);
            tmp_user_buf_ptr += tran_size;
            left_size -= tran_size;
            TEE_MemMove(dataBuf, frame->data, 284);
            dataBuf = dataBuf + 284;

            frame++;
        }
        frame--;
        TEE_HmacSha256(mac_key, dataBuf_ptr, 284 * tran_blkcnt, frame->mac, &hmac_len);
        /*
         * STEP 4, send write data request.
         */
        RpmbData.commandId = FID_DRV_WRITE_DATA;
        RpmbData.start_blk = start_blkIdx;
        RpmbData.blks = tran_blkcnt;
        ret = rpmb_handle(sid);
        if (ret) {
            DBGOUT(ERR, "rpmb_handle write data error (%x)\n", ret);
            TEE_Free(dataBuf_ptr);
            return TEE_ERROR_GENERIC;
        }
        /*
         * STEP 5. authenticate write result response.
         *	   1. authenticate hmac.
         *	   2. check result.
         *	   3. compare write counter is increamented.
         */
        TEE_HmacSha256(mac_key, first_frame->data, 284, hmac, &hmac_len);
        if (TEE_MemCompare(first_frame->mac, hmac, RPMB_SZ_MAC) != 0) {
            DBGOUT(ERR, "write data result hmac compare error.\n");
            *result = 1;
            TEE_Free(dataBuf_ptr);
            return TEE_ERROR_SIGNATURE_INVALID;
        }
        if (first_frame->result) {
            *result = cpu_to_be16p(&first_frame->result);
            DBGOUT(ERR, "write data result error(%x)!!!, retry = %d\n", *result, retry);
            TEE_Free(dataBuf_ptr);
            /* only retry when write counter miss match */
            /* this may due to other TEE accessing rpmb at the same time */
            if (*result == 3 && --retry)
                goto _retry;
            return TEE_ERROR_GENERIC;
        }
        rWC = cpu_to_be32p(&first_frame->write_counter);
        if (rWC != (WC + 1)) {
            *result = 1;
            DBGOUT(ERR, "write data write counter mismatch error(last:%x)(current:%x)!!!\n", WC, rWC);
            TEE_Free(dataBuf_ptr);
            return TEE_ERROR_GENERIC;
        }
        start_blkIdx += tran_blkcnt;
        left_blkcnt -= tran_blkcnt;
        i++;
        TEE_Free(dataBuf_ptr);
    };

    if (boundary_buf)
        TEE_Free(boundary_buf);
    if (left_blkcnt || left_size) {
        DBGOUT(ERR, "left_blkcnt or left_size is not empty!!!!!!\n");
        *result = 1;
        return TEE_ERROR_GENERIC;
    }
#else
    total_blkcnt = ((bufSize % RPMB_SZ_DATA) ? (bufSize/RPMB_SZ_DATA + 1) : (bufSize/RPMB_SZ_DATA));
    /*
     * For RPMB write data, the elements we need in the data frame is
     * 1. address.
     * 2. write counter.
     * 3. data.
     * 4. block count.
     * 5. MAC
     *
     */

    /*
     * STEP 1(Address), search partition table to find out where is my address.
     */
    RpmbData.commandId = FID_DRV_GET_PART_ADDR;
    ret = rpmb_handle(sid);
    frame = (struct rpmb_t *)RpmbData.buf;

    for (iCnt = 0; iCnt < total_blkcnt; iCnt++) {
        TEE_MemFill(frame, 0, 512);
        /*
         * STEP 2-1(write counter), prepare get wcounter request. we only need nonce.
         */
        type = RPMB_GET_WRITE_COUNTER;
        rType = cpu_to_be16p(&type);
        frame->request = rType;
        //tlApiRandomGenerateData(TLAPI_ALG_SECURE_RANDOM, nonce, &nonce_len);
        TEE_GenerateRandom(nonce, nonce_len);
        TEE_MemMove(frame->nonce, nonce, RPMB_SZ_NONCE);
        RpmbData.commandId = FID_DRV_GET_WCOUNTER;
        RpmbData.blks = 1;
        ret = rpmb_handle(sid);
        if (ret) {
            DBGOUT(ERR, "rpmb_handle get wc error (%x)\n", ret);
            return 1;
        }
        /*
         * STEP 2-2(write counter), authenticate the wc data frame is legal or not.
         *			  1. authenticate hmac is correct or not.
         *			  2. check nonce is the same or not.
         *			  3. check result if no error or something else.
         */
        TEE_HmacSha256(mac_key, frame->data, 284, hmac, &hmac_len);
        if (TEE_MemCompare(frame->mac, hmac, RPMB_SZ_MAC) != 0) {
            DBGOUT(ERR, "Get wcounter hmac compare error.\n");
            *result = 1;
            return 1;
        }
        if (TEE_MemCompare(frame->nonce, nonce, RPMB_SZ_NONCE) != 0) {
            *result = 1;
            DBGOUT(ERR, "Get wcounter nonce compare error!!!\n");
            return 1;
        }
        if (frame->result) {
            *result = cpu_to_be16p(&frame->result);
            DBGOUT(ERR, "Get wcounter result error(%x)!!!\n", *result);
            return 1;
        }
        /*
         * STEP 3(write data), prepare write data frame. we need
         *		       1. write counter.
         *		       2. block count.
         *		       3. address.
         *		       4. nonce is zero.
         *		       5. data.
         *		       6. HMAC.
         */
        rWC = frame->write_counter;
        WC = cpu_to_be32p(&rWC);
        type = RPMB_WRITE_DATA;
        rType = cpu_to_be16p(&type);
        rAddr = cpu_to_be16p(&RpmbData.part_addr);

        frame->request = rType;
        frame->block_count = 0x0100; // it means 1 block every time.
        frame->address = rAddr;
        TEE_MemFill(frame->nonce, 0, RPMB_SZ_NONCE);
        /*
         * STEP 3-5, copy user's data to my frame.
         */
        if (left_size >= RPMB_SZ_DATA)
            tran_size = RPMB_SZ_DATA;
        else
            tran_size = left_size;
        TEE_MemMove(frame->data, buf + iCnt * RPMB_SZ_DATA, tran_size);

        /*
         * STEP 3-6, HMAC computation.
         */
        TEE_HmacSha256(mac_key, frame->data, 284, frame->mac, &hmac_len);

        RpmbData.commandId = FID_DRV_WRITE_DATA;
        RpmbData.blks = 1;
        ret = rpmb_handle(sid);
        if (ret) {
            DBGOUT(ERR, "rpmb_handle write data error (%x)\n", ret);
            return 1;
        }
        /*
         * STEP 4. authenticate write result response. we need
         *	   1. authenticate hmac.
         *	   2. check result.
         *	   3. compare write counter is increamented.
         */
        TEE_HmacSha256(mac_key, frame->data, 284, hmac, &hmac_len);
        if (TEE_MemCompare(frame->mac, hmac, RPMB_SZ_MAC) != 0) {
            *result = 1;
            DBGOUT(ERR, "write data result hmac compare error.\n");
            return 1;
        }
        if (frame->result) {
            *result = cpu_to_be16p(&frame->result);
            DBGOUT(ERR, "write data result error(%x)!!!, retry = %d\n", *result, retry);
            /* only retry when write counter miss match */
            /* this may due to other TEE accessing rpmb at the same time */
            if (*result == 3 && --retry)
                goto _retry;
            return 1;
        }
        rWC = cpu_to_be32p(&frame->write_counter);
        if (rWC != (WC + 1)) {
            *result = 1;
            DBGOUT(ERR, "write data write counter mismatch error(last:%x)(current:%x)!!!\n", WC, rWC);
            return 1;
        }
        left_size -= tran_size;
        RpmbData.start_addr++;
    };
#endif
    return TEE_SUCCESS;
}


/**
 * read data
 *
 * @param sid        session id
 * @param commandId  command id
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C TEE_Result TEE_RpmbReadData(
    uint32_t sid,
    uint8_t *buf,
    uint32_t bufSize,
    int *result)
{
    TEE_Result ret = TEE_SUCCESS;

    DBGOUT(INFO, "start!!\n");
    ret = TEE_RpmbReadDatabyOffset(sid, 0, buf, bufSize, result);
    DBGOUT(INFO, "end!!\n");
    return ret;
}


/**
 * write data
 *
 * @param sid        session id
 * @param commandId  command id
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C TEE_Result TEE_RpmbWriteData(
    uint32_t sid,
    uint8_t *buf,
    uint32_t bufSize,
    int *result)
{
    TEE_Result ret = TEE_SUCCESS;

    DBGOUT(INFO, "start!!\n");
    ret = TEE_RpmbWriteDatabyOffset(sid, 0, buf, bufSize, result);
    DBGOUT(INFO, "end!!\n");
    return ret;
}



#if (CFG_RPMB_KEY_PROGRAMED_IN_KERNEL == 1)
/**
 * program key
 *
 * @param sid        session id
 * @param commandId  command id
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C TEE_Result TEE_RpmbProgramKey(
	uint32_t sid,
	int *result)
{
	TEE_Result ret = TEE_SUCCESS;
	const uint8_t *mac_key;
	uint16_t type;
	uint16_t rType;
	struct rpmb_t *frame;
	uint32_t flag;

	DBGOUT(INFO, "start!!\n");

	*result = 0;
	TEE_MemFill(&RpmbData, 0, sizeof(tlApiRpmb_t));
	frame = (struct rpmb_t *)RpmbData.buf;

	//check the flag to determine whether programming the key or not
	RpmbData.commandId = FID_DRV_CHECK_FLAG;
	ret = rpmb_handle(sid);
	if (ret) {
		DBGOUT(ERR, "tlApi_callDriver check flag error (%x)\n", ret);
		*result = 1;
		return 1;
	}
	flag = RpmbData.result;
	DBGOUT(INFO, "check the flag: %d\n", flag);

	if (flag == 0) {
		DBGOUT(INFO, "Auth key had been programmed before\n");
		DBGOUT(INFO, "NOT allow to program again\n");
		*result = 2;
		return 1;
	} else {
		RpmbData.commandId = FID_DRV_GET_KEY;
		ret = rpmb_handle(sid);
		if (ret) {
			DBGOUT(ERR, "tlApi_callDriver get key error (%x)\n", ret);
			*result = 1;
			return 1;
		}
		mac_key = RpmbData.mac_key;

		type = RPMB_PROGRAM_KEY;
		rType = cpu_to_be16p(&type);
		frame->request = rType;
		TEE_MemMove(frame->mac, mac_key, RPMB_SZ_MAC);

		RpmbData.commandId = FID_DRV_PROGRAM_KEY;
		ret = rpmb_handle(sid);
		if (ret) {
			DBGOUT(ERR, "tlApi_callDriver program key error (%x)\n", ret);
			return 1;
		}

		if (frame->result) {
			*result = cpu_to_be16p(&frame->result);
			DBGOUT(ERR, "Program key result error(%x)!!!\n", *result);
			return 1;
		}
		/* After programmed key successfully
		 * needs to modify key status to
		 * not allowed to program again
		 */
		/* result: */
		/* 0: program key is not allowed */
		/* 1: program key is allowed */
		RpmbData.result = 0;
		RpmbData.commandId = FID_DRV_SET_KEY_FLAG;
		ret = rpmb_handle(sid);
		if (ret) {
			DBGOUT(ERR, "tlApi_callDriver set flag error (%x)\n", ret);
			*result = 2;
			return 1;
		}
	}
	DBGOUT(INFO, "end!!\n");

	return ret;
}
#endif
