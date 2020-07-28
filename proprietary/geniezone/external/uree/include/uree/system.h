/*
 * Header files for basic UREE functions.
 */

#ifndef __UREE_H__
#define __UREE_H__

#include "tz_cross/trustzone.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/// UREE session handle type.
typedef unsigned int UREE_SESSION_HANDLE;

/* Session Management */
/**
 *  Create a new TEE sesssion
 *
 * @param ta_uuid UUID of the TA to connect to.
 * @param pHandle Handle for the new session. Return UREE_SESSION_HANDLE_FAIL if fail.
 * @return return code
 */
TZ_RESULT UREE_CreateSession(const char *ta_uuid, UREE_SESSION_HANDLE *pHandle);

/**
 * Close TEE session
 *
 * @param handle Handle for session to close.
 * @return return code
 */
TZ_RESULT UREE_CloseSession(UREE_SESSION_HANDLE handle);


/**
 * Make a TEE service call
 *
 * @param handle      Session handle to make the call
 * @param command     The command to call.
 * @param paramTypes  Types for the parameters, use TZ_ParamTypes() to consturct.
 * @param param       The parameters to pass to TEE. Maximum 4 params.
 * @return            Return value from TEE service.
 */
TZ_RESULT UREE_TeeServiceCall(UREE_SESSION_HANDLE handle, uint32_t command,
                              uint32_t paramTypes, MTEEC_PARAM param[4]);

/* for CA avoid mcdi issue*/
TZ_RESULT UREE_DeepIdle_Mask(bool flag);

#ifdef __cplusplus
}
#endif

#endif /* __UREE_H__ */
