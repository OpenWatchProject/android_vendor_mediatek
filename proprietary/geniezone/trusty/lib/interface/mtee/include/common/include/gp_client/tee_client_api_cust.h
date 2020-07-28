/*
 * This header file corresponds to V1.0 of the GlobalPlatform
 * TEE Client API Specification
 */
#ifndef __TEE_CLIENT_API_CUST_H__
#define __TEE_CLIENT_API_CUST_H__

/*
 * DATA TYPES
 */
#define MTEE_TEEC_UUID teec_uuid
#define MTEE_TEEC_Context teec_context
#define MTEE_TEEC_Session teec_session
#define MTEE_TEEC_SharedMemory teec_shared_memory
#define MTEE_TEEC_TempMemoryReference teec_temp_memory_reference
#define MTEE_TEEC_RegisteredMemoryReference teec_registered_memory_reference
#define MTEE_TEEC_Value teec_value
#define MTEE_TEEC_Parameter teec_parameter
#define MTEE_TEEC_Operation teec_operation

/*
 * FUNCTIONS
 */
#define MTEE_TEEC_InitializeContext teec_initialize_context
#define MTEE_TEEC_FinalizeContext teec_finalize_context
#define MTEE_TEEC_RegisterSharedMemory teec_register_shared_memory
#define MTEE_TEEC_AllocateSharedMemory teec_allocate_shared_memory
#define MTEE_TEEC_ReleaseSharedMemory teec_release_shared_memory
#define MTEE_TEEC_OpenSession teec_open_session
#define MTEE_TEEC_CloseSession teec_close_session
#define MTEE_TEEC_InvokeCommand teec_invoke_command
#define MTEE_TEEC_RequestCancellation teec_request_cancellation

#endif /* __TEE_CLIENT_API_CUST_H__ */
