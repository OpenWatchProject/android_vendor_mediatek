#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <trusty_std.h>

#include <mtee_server.h>
#include <tz_cross/trustzone.h>
#include <tz_private/sys_ipc.h>
#include <tz_private/log.h>

#include <gp_client/tee_client_api.h>

#define LOG_TAG "TEE_CLIENT"
#define MTEE_REE_SESSION 999 // current max session num is 512

u32 teec_initialize_context(const char *name, struct teec_context *context)
{
	TZ_RESULT ret;
	MTEEC_PARAM ree_param[4];
	uint32_t ree_paramTypes;

	ree_paramTypes = TZ_ParamTypes3(TZPT_MEM_INOUT, TZPT_MEM_INOUT, TZPT_VALUE_INPUT);
	ree_param[0].mem.buffer = name;
	ree_param[0].mem.size = strlen(name) + 1;
	ree_param[1].mem.buffer = context;
	ree_param[1].mem.size = sizeof(struct teec_context);

	ret = MTEE_ReeServiceCall(MTEE_REE_SESSION, REE_SERVICE_CMD_TEE_INIT_CTX, ree_paramTypes, ree_param);
	if (ret)
		return TEEC_ERROR_GENERIC;

	memcpy(context, ree_param[1].mem.buffer, ree_param[1].mem.size);

	return ree_param[2].value.a;
}

void teec_finalize_context(struct teec_context *context)
{
	TZ_RESULT ret;
	MTEEC_PARAM ree_param[4];
	uint32_t ree_paramTypes;

	ree_paramTypes = TZ_ParamTypes1(TZPT_MEM_INOUT);
	ree_param[0].mem.buffer = context;
	ree_param[0].mem.size = sizeof(struct teec_context);

	ret = MTEE_ReeServiceCall(MTEE_REE_SESSION, REE_SERVICE_CMD_TEE_FINAL_CTX, ree_paramTypes, ree_param);
	if (ret)
		ERR_LOG("[ERROR] REE_SERVICE_CMD_TEE_FINAL_CTX fail \n");
}

u32 teec_open_session(struct teec_context *context, struct teec_session *session, const struct teec_uuid *destination,
		u32 connection_method, const void *connection_data, struct teec_operation *operation, u32 *return_origin)
{
	TZ_RESULT ret;
	MTEEC_PARAM ree_param[4];
	uint32_t ree_paramTypes;

	if(connection_method != TEEC_LOGIN_PUBLIC || connection_data != NULL || operation != NULL || return_origin != NULL) {
		ERR_LOG("[ERROR] connection_method and connection_data and operation and return_origin should be NULL \n");
		return TEEC_ERROR_GENERIC;
	}

	ree_paramTypes = TZ_ParamTypes4(TZPT_MEM_INOUT, TZPT_MEM_INOUT, TZPT_MEM_INOUT, TZPT_VALUE_INPUT);
	ree_param[0].mem.buffer = context;
	ree_param[0].mem.size = sizeof(struct teec_context);
	ree_param[1].mem.buffer = session;
	ree_param[1].mem.size = sizeof(struct teec_session);
	ree_param[2].mem.buffer = destination;
	ree_param[2].mem.size = sizeof(struct teec_uuid);

	ret = MTEE_ReeServiceCall(MTEE_REE_SESSION, REE_SERVICE_CMD_TEE_OPEN_SE, ree_paramTypes, ree_param);
	if (ret)
		return TEEC_ERROR_GENERIC;

	memcpy(session, ree_param[1].mem.buffer, ree_param[1].mem.size);

	return ree_param[3].value.a;
}

void teec_close_session(struct teec_session *session)
{
	TZ_RESULT ret;
	MTEEC_PARAM ree_param[4];
	uint32_t ree_paramTypes;

	ree_paramTypes = TZ_ParamTypes1(TZPT_MEM_INOUT);
	ree_param[0].mem.buffer = session;
	ree_param[0].mem.size = sizeof(struct teec_session);

	ret = MTEE_ReeServiceCall(MTEE_REE_SESSION, REE_SERVICE_CMD_TEE_CLOSE_SE, ree_paramTypes, ree_param);
	if (ret)
		ERR_LOG("[ERROR] REE_SERVICE_CMD_TEE_CLOSE_SE fail \n");
}

// FIXME!! currently param only support value
u32 teec_invoke_command(struct teec_session *session, u32 command_id, struct teec_operation *operation, u32 *return_origin)
{
	TZ_RESULT ret;
	MTEEC_PARAM ree_param[4];
	uint32_t ree_paramTypes;

	if(return_origin != NULL) {
		ERR_LOG("[ERROR] return_origin should be NULL \n");
		return TEEC_ERROR_GENERIC;
	}

	ree_paramTypes = TZ_ParamTypes4(TZPT_MEM_INOUT, TZPT_VALUE_OUTPUT, TZPT_MEM_INOUT, TZPT_VALUE_INPUT);
	ree_param[0].mem.buffer = session;
	ree_param[0].mem.size = sizeof(struct teec_session);
	ree_param[1].value.a = command_id;
	ree_param[2].mem.buffer = operation;
	ree_param[2].mem.size = sizeof(struct teec_operation);

	ret = MTEE_ReeServiceCall(MTEE_REE_SESSION, REE_SERVICE_CMD_TEE_INVOK_CMD, ree_paramTypes, ree_param);
	if (ret)
		return TEEC_ERROR_GENERIC;

	memcpy(operation, ree_param[2].mem.buffer, ree_param[2].mem.size);

	return ree_param[3].value.a;
}


