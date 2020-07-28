/**
 * @file   tlDriverApi.h
 * @brief  Contains trustlet API definitions
 *
 */

#ifndef __CMDQ_SEC_TLDRIVERAPI_H__
#define __CMDQ_SEC_TLDRIVERAPI_H__

//#include "tlStd.h"
#include "TlApi/TlApiError.h"
#include "cmdq_sec_iwc_common.h"


#define CMDQ_SEC_DRV_ID DRIVER_ID


/**
 * IPC Function id definitions (TL to driver)
 */

/* IPC session id */
#define FID_CMDQ_TZ_OPEN_SESSION   (1)
#define FID_CMDQ_TZ_CLOSE_SESSION  (2)
#define FID_CMDQ_TZ_INIT_DATA	   (3)
#define FID_CMDQ_TZ_EXEC		   (4)
/* session execute id, prefix is 4x */
#define FID_CMDQ_TZ_EXEC_TASK_ASYNC		 (41) /* submit and execute task via GCE */
#define FID_CMDQ_TZ_EXEC_RES_RELEASE  (42) /* reserved id, currently no used */
#define FID_CMDQ_TZ_EXEC_CANCEL_TASK  (43)
#define FID_CMDQ_TZ_EXEC_PATH_RES_ALLOCATE (44)
#define FID_CMDQ_TZ_EXEC_PATH_RES_RELEASE  (45)
#define FID_CMDQ_TZ_EXEC_PATH_RES_USE_PREALLOCATE  (46)

/* debug */
#define FID_CMDQ_TZ_TEST_DUMMY	(4001)
#define FID_CMDQ_TZ_TEST_SMI_DUMP (4002)
#define FID_CMDQ_TZ_DEBUG_SW_COPY (4003)
#define FID_CMDQ_TZ_TRAP_DR_INFINITELY (4004)
#define FID_CMDQ_TZ_DUMP (4005)

/* sectrace profile */
#define FID_CMDQ_TZ_SECTRACE_MAP      (3000)
#define FID_CMDQ_TZ_SECTRACE_UNMAP    (3001)
#define FID_CMDQ_TZ_SECTRACE_TRANSACT (3002)


typedef struct {
	uint32_t* pSecFdAddr;
	uint32_t pSecFdCount;
	uint16_t ovl_handle[CMDQ_IWC_MAX_ADDR_LIST_LENGTH];
	uint16_t block_layer[CMDQ_IWC_MAX_ADDR_LIST_LENGTH];
	uint32_t write_ovl_src_inst[CMDQ_IWC_MAX_ADDR_LIST_LENGTH];
	uint32_t ovl_mode;	/* 0:primary/dc 1:extend ~0:pass */
}tlApiCmdqExecMetadata_t, *tlApiCmdqExecMetadata_ptr;

typedef struct {
	uint32_t secPA; /* start address of command buffer in secure path*/
	uint32_t size; /* buffer size */
}tlApiCmdqPathResMetadata_t, *tlApiCmdqPathResMetadata_ptr;

typedef struct {
	unsigned long PA;
	uint32_t size;
}tlApiCmdqSectraceBuffer_t, *tlApiCmdqSectraceBuffer_ptr;

/**
 * IPC execution data (between TL* and cmdqSecDr)
 *
 * @paramc ommandId [IN]  cmdqSecDr operation code
 * @param result	[OUT] cmdqSecDr execution result, 0 for success, < 0 for err
 * @param execData  [IN]  cmdqSecDr execution metadata about secure buffer address etc
 *
 */
typedef struct {
	uint32_t commandId;
	int32_t result;

	void* pTci;
	union {
		tlApiCmdqExecMetadata_t execData;
		tlApiCmdqPathResMetadata_t resource;
		tlApiCmdqSectraceBuffer_t sectraceBuffer;
   	} metadata;
} drMarshalingData_t, *drMarshalingData_ptr;


/**
 * tbase Marashaling struct DESIGN LIMITATION
 *
 * @param
 *   1. functionId MUST be the first one
 *   2. retVal is behind the functionId, and it should be TRAPI_OK, i.e. 0
 *	  - so we query pData.result to get cmdqSecDr's execution result
 *
 * @NOTE:
 *   make sure we follow the design rule to prevent strange exeption
 *
 */
typedef struct {
	uint32_t functionId; // Function identifier for IPC between tl* to cmdqSecDr
	union {
		tlApiResult_t retVal; // IPC transtion result, must be OK
		drMarshalingData_ptr pData; // IPC data
	} payload;
	uint32_t sid; // Session identifier
} drMarshalingParam_t, *drMarshalingParam_ptr;




/**
 * Open session to the driver with given data
 *
 * @return  session id
 */
_TLAPI_EXTERN_C uint32_t tlApiCmdqOpenSession( void );


/**
 * Close session
 *
 * @param sid  session id
 *
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C tlApiResult_t tlApiCmdqCloseSession( uint32_t sid );



/**
 * Executes command
 *
 * @param sid		session id
 * @param commandId  command id
 *
 * @return  TLAPI_OK upon success or specific error
 */

_TLAPI_EXTERN_C tlApiResult_t tlApiCmdqExecute(
		uint32_t sid,
		drMarshalingData_ptr pData);


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

/**
 * Sectrace related IPC
 * @return  TLAPI_OK upon success or specific error
 */
_TLAPI_EXTERN_C tlApiResult_t tlApiCmdqSectraceMap(
		unsigned long pa,
		unsigned long size);

_TLAPI_EXTERN_C tlApiResult_t tlApiCmdqSectraceUnmap(
		unsigned long pa,
		unsigned long size);

_TLAPI_EXTERN_C tlApiResult_t tlApiCmdqSectraceTransact(void);

#endif // __CMDQ_SEC_TLDRIVERAPI_H__
