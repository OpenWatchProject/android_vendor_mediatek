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

/**
 * @file   tlDriverApi.h
 * @brief  Contains trustlet API definitions
 *
 */

#ifndef __TLH264SECVDECDRIVERAPI_H__
#define __TLH264SECVDECDRIVERAPI_H__

#include "TlApi/TlApiError.h"

typedef struct {
    uint32_t data0;    // pVdec_H264_Drv_data
    uint32_t data1;    // pVdec_H264_InstanceTemp
    uint32_t data2;    // pH264_DEC_PRM_DataInst
    uint32_t data3;    // bitstream VA
    uint32_t data3Len;

    // HW working buffers (for decoder init)
    uint32_t pred_buffer_pa;
    uint32_t mv_buffer_pa;
    uint32_t pp_wrapper_buf_pa;
    uint32_t vld_wrapper_buf_pa;
    uint32_t bs_working_buf_pa;

    unsigned long    bs_buffer_nwd_va;    // input bitstream buffer normal world VA (VA of real data)
    unsigned long    frm_buffer_nwd_va; // output frame buffer normal world VA (VA of real data)
    unsigned long    bs_nwd_va;    // input bitstream normal world VA (VA of ring buffer)
    unsigned long    frm_nwd_va; // output frame normal world VA (VA of frame buffer)

    uint32_t bs_buf_pa;    // input bitstream secure buffer PA
    uint32_t frm_buf_pa; // output frame secure buffer PA

    // for debugging, dump frame
    uint32_t mem_source_pa;                // input source (PA of secure handle)
    uint32_t mem_output_target_va;   // output target
    uint32_t mem_output_length;

    // for UT, fill secmem (bitstream)
    uint32_t mem_target_pa;  // outoput target (PA of secure handle)
    uint32_t mem_input_source_va;
    uint32_t mem_input_length;

    // for systrace
    uint32_t sys_trace_buffer_pa;           // address
    uint32_t sys_trace_buffer_size;
} tlApiH264VdecShareData_t, *tlApiH264VdecShareData_ptr;



/**
 * Open session to the driver with given data
 *
 * @return  session id
 */
_TLAPI_EXTERN_C uint32_t tlH264SecVdecApiOpenSession( void );


/**
 * Close session
 *
 * @param sid  session id
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C tlApiResult_t tlH264SecVdecApiCloseSession( uint32_t sid );

 #if 0
 /**
 * Executes command
 *
 * @param sid        session id
 * @param commandId  command id
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C tlApiResult_t tlApiTest(
        uint32_t sid,
        uint32_t decoderData);
#endif

_TLAPI_EXTERN_C tlApiResult_t tlApiH264SecVdecInit(uint32_t sid, tlApiH264VdecShareData_ptr decoderData);
_TLAPI_EXTERN_C tlApiResult_t tlApiH264SecVdecDecode(uint32_t sid, tlApiH264VdecShareData_ptr decoderData);
_TLAPI_EXTERN_C tlApiResult_t tlApiH264SecVdecDeinit(uint32_t sid, tlApiH264VdecShareData_ptr decoderData);
_TLAPI_EXTERN_C tlApiResult_t tlApiH264SecVdecDumpMem(uint32_t sid, tlApiH264VdecShareData_ptr decoderData);
_TLAPI_EXTERN_C tlApiResult_t tlApiH264SecVdecDumpSecMem(uint32_t sid, tlApiH264VdecShareData_ptr decoderData);
_TLAPI_EXTERN_C tlApiResult_t tlApiH264SecVdecFillSecMem(uint32_t sid, tlApiH264VdecShareData_ptr decoderData);

_TLAPI_EXTERN_C tlApiResult_t tlApiH264SecVdecSystraceMap(unsigned long pa, unsigned long size);
_TLAPI_EXTERN_C tlApiResult_t tlApiH264SecVdecSystraceUnmap(unsigned long pa, unsigned long size);
_TLAPI_EXTERN_C tlApiResult_t tlApiH264SecVdecSystraceTransact();



/** tlApi function to call driver via IPC.
 * Sends a MSG_RQ message via IPC to a MobiCore driver.
 *
 * @param driverID The driver to send the IPC to.
 * @param pMarParam MPointer to marshaling parameters to send to the driver.
 *
 * @return TLAPI_OK
 * @return E_TLAPI_COM_ERROR in case of an IPC error.
 */
_TLAPI_EXTERN_C tlApiResult_t tlApi_callDriver(
        uint32_t driver_ID,
        void* pMarParam);


#endif // __TLDRIVERAPI_H__
