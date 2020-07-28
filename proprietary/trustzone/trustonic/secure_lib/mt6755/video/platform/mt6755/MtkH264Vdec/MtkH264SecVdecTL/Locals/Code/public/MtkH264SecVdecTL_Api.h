/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#ifndef MTKH264SECVDECTL_H_
#define MTKH264SECVDECTL_H_

#include "tci.h"

//#define ENABLE_VDEC_SYSTRACE

#define ENABLE_M4U
#define ENABLE_DAPC

typedef struct{
    uint32_t id1;
    uint32_t id2;
} MtkH264VdecShare_t;


/**
 * Command ID's for communication Trustlet Connector -> Trustlet.
 */
#define CMD_MTKH264SECVDEC_INIT                        1
#define CMD_MTKH264SECVDEC_DECODE                  2
#define CMD_MTKH264SECVDEC_DEINIT                   3
#define CMD_MTKH264SECVDEC_ALLOC_SECMEM    4
#define CMD_MTKH264SECVDEC_DUMP_SECMEM    5
#define CMD_MTKH264SECVDEC_FILL_SECMEM       6
#define CMD_MTKH264SECVDEC_PING                      100
#define CMD_MTKH264SECVDEC_SYSTRACE_MAP      101
#define CMD_MTKH264SECVDEC_SYSTRACE_UNMAP 102
#define CMD_MTKH264SECVDEC_SYSTRACE_TRANSACT 103




/**
 * Termination codes
 */
#define EXIT_ERROR                  ((uint32_t)(-1))

/**
 * command message.
 *
 * @param len Lenght of the data to process.
 * @param data Data to processed (cleartext or ciphertext).
 */
typedef struct {
    tciCommandHeader_t  header;     /**< Command header */
    uint32_t            len;        /**< Length of data to process or buffer */
    uint32_t            respLen;    /**< Length of response buffer */
} cmd_t;

/**
 * Response structure Trustlet -> Trustlet Connector.
 */
typedef struct {
    tciResponseHeader_t header;     /**< Response header */
    uint32_t            len;
} rsp_t;

/**
 * TCI message data.
 */
typedef struct {
    union {
      cmd_t     cmdH264SecVdec;
      rsp_t     rspH264SecVdec;
    };

    // sec mem api related
    uint32_t alignment;
    uint32_t size;
    uint32_t sec_mem_handle;  // the handle of sec mem (to be freed)

    // for debugging , dump frame (secmem)
    uint32_t sec_mem_handle_source;  // the handle of sec mem (to be freed)
    uint32_t mem_output_target_va;
    uint32_t mem_output_length;

    // for UT, fill secmem (bitstream)
    uint32_t sec_mem_handle_target;  // the handle of sec mem (to be freed)
    uint32_t mem_input_source_va;
    uint32_t mem_input_length;

    // decoder related
    uint32_t    ShareData0;  // pVdec_H264_Drv_data
    uint32_t    ShareData1;  // pVdec_H264_InstanceTemp
    uint32_t    ShareData2;  // pH264_DEC_PRM_DataInst
    uint32_t    ShareData3;
#if 1
    uint32_t    ShareData0Len;  // pVdec_H264_Drv_data
    uint32_t    ShareData1Len;  // pVdec_H264_InstanceTemp
    uint32_t    ShareData2Len;  // pH264_DEC_PRM_DataInst
    uint32_t    ShareData3Len;
#endif

    uint32_t    systrace_buf_va;   // VA in trustlet
    uint32_t    systrace_buf_size;

    uint32_t    pred_buffer_handle;
    uint32_t    mv_buffer_handle;
    uint32_t    pp_wrapper_handle;
    uint32_t    vld_wrapper_handle;
    uint32_t    bs_working_buf_handle;

    unsigned long    bs_buffer_nwd_va;    // input bitstream buffer normal world VA (VA of real data)
    unsigned long    frm_buffer_nwd_va; // output frame buffer normal world VA (VA of real data)

    unsigned long    bs_nwd_va;    // input bitstream normal world VA (VA of ring buffer)
    unsigned long    frm_nwd_va; // output frame normal world VA (VA of frame buffer)

    uint32_t    bs_buf_handle; // input bitstream secure handle
    uint32_t    frm_buf_handle; // output frame buffer secure handle

    uint32_t    ResultData;
} tciMessage_t;



/**
 * Trustlet UUID.
 */
#define MTKH264SECVDECTL_UUID { { 8, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }

#endif // MTKH264SECVDECTL_H_
