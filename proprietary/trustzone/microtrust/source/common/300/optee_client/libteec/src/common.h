#ifndef __COMMON_H_
#define __COMMON_H_

#include <map>
#include <tee_client_api.h>


using std::map;

class Common
{
private:
	map<TEEC_Context *, TEEC_Context *> _contextMap;
	map<TEEC_Session *, TEEC_Session *> _sessionMap;

private:
	TEEC_Result teec_pre_process_operation(
			TEEC_Context*			context,
			TEEC_Operation*			operation,
			struct tee_ioctl_param* params,
			TEEC_SharedMemory*		shareMem);

	TEEC_Result teec_pre_process_tmpref(
			TEEC_Context*			context,
			uint32_t 				param_type,
			TEEC_TempMemoryReference* tmpref,
			struct tee_ioctl_param* params,
			TEEC_SharedMemory*		shareMem);

	void teec_free_temp_refs(
			TEEC_Operation*			operation,
			TEEC_SharedMemory*		shareMem);

public:
	TEEC_Result TEEC_InitializeContext(
			const char*				name,
			TEEC_Context*			context);

	void TEEC_FinalizeContext(
			TEEC_Context*			context);

	TEEC_Result TEEC_RegisterSharedMemory(
			TEEC_Context*			context,
			TEEC_SharedMemory*		shareMem);

	TEEC_Result TEEC_AllocateSharedMemory(
			TEEC_Context*			context,
			TEEC_SharedMemory*		shareMem);

	TEEC_Result TEEC_RegisterSharedMemoryFileDescriptor(
			TEEC_Context*			context,
			TEEC_SharedMemory*		shareMem,
			int						fd);

	void TEEC_ReleaseSharedMemory(
			TEEC_SharedMemory*		shareMem);

	TEEC_Result TEEC_OpenSession(
			TEEC_Context*			context,
			TEEC_Session*			session,
			const TEEC_UUID*		destination,
			uint32_t 				connectionMethod,
			const void*				connectionData,
			TEEC_Operation*			operation,
			uint32_t*				returnOrigin);

	void TEEC_CloseSession(
			TEEC_Session*			session);

	TEEC_Result TEEC_InvokeCommand(
			TEEC_Session*			session,
			uint32_t 				commandID,
			TEEC_Operation*			operation,
			uint32_t*				returnOrigin);

	void TEEC_RequestCancellation(
			TEEC_Operation*			operation);

public:
	~Common();
	static Common& getInstance(void) {
		static Common client;
		return client;
	}
};

#endif // __COMMON_H_
