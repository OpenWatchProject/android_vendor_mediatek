/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#define HWKEY_PORT "com.android.trusty.hwkey"

#define HWKEY_GET_KEYSLOT_PROTOCOL_VERSION 0
#define HWKEY_DERIVE_PROTOCOL_VERSION      0

#define HWKEY_KDF_VERSION_BEST 0
#define HWKEY_KDF_VERSION_1    1

/**
 * enum hwkey_cmd - command identifiers for hwkey functions
 */
enum hwkey_cmd {
	HWKEY_RESP_BIT     = 1,
	HWKEY_REQ_SHIFT    = 1,

	HWKEY_GET_KEYSLOT  = (0 << HWKEY_REQ_SHIFT),
	HWKEY_DERIVE       = (1 << HWKEY_REQ_SHIFT),
};

/**
 * enum hwkey_err - error codes for hwkey protocol
 * @HWKEY_NO_ERROR:             all OK
 * @HWKEY_ERR_GENERIC:          unknown error. Can occur when there's an internal server
 *                              error, e.g. the server runs out of memory or is in a bad state.
 * @HWKEY_ERR_NOT_VALID:        input not valid. May occur if the non-buffer arguments passed
 *                              into the command are not valid, for example if the KDF
 *                              version passed to derive is not any supported version.
 * @HWKEY_ERR_BAD_LEN:          buffer is unexpected or unaccepted length.
 *                              May occur if received message is not at least
 *                              the length of the header, or if the payload length
 *                              does not meet constraints for the function.
 * @HWKEY_ERR_NOT_IMPLEMENTED:  requested command not implemented
 * @HWKEY_ERR_NOT_FOUND:        requested keyslot not found
 */
enum hwkey_err {
	HWKEY_NO_ERROR            = 0,
	HWKEY_ERR_GENERIC         = 1,
	HWKEY_ERR_NOT_VALID       = 2,
	HWKEY_ERR_BAD_LEN         = 3,
	HWKEY_ERR_NOT_IMPLEMENTED = 4,
	HWKEY_ERR_NOT_FOUND       = 5,
};

/**
 * hwkey protocol:
 * -  Client opens channel to the server, then sends one or more
 *    requests and receives replies.
 *
 * -  Client is allowed to keep the channel opened for the duration
 *    of the session.
 *
 * -  Client is allowed to open multiple channels, all such channels
 *    should be treated independently.
 *
 * -  Client is allowed to issue multiple requests over the same channel
 *    and may receive responses in any order. Client must check op_id
 *    to determine corresponding request.
 *
 * - The request and response structure is shared among all API calls.
 *   The data required for each call is as follows:
 *
 * hwkey_get_keyslot:
 *
 * Request:
 * @cmd:     HWKEY_REQ_GET_KEYSLOT
 * @op_id:   client specified operation identifier. Echoed
 *           in response.
 * @status:  must be 0.
 * @arg1:    unused
 * @arg2:    unused
 * @payload: string identifier of requested keyslot, not null-terminated
 *
 * Response:
 * @cmd:     HWKEY_RESP_GET_KEYSLOT
 * @op_id:   echoed from request
 * @status:  operation result, one of enum hwkey_err
 * @arg1:    unused
 * @arg2:    unused
 * @payload: unencrypted keyslot data, or empty on error
 *
 * hwkey_derive:
 *
 * Request:
 * @cmd:     HWKEY_REQ_DERIVE
 * @op_id:   client specified operation identifier. Echoed
 *           in response.
 * @status:  must be 0.
 * @arg1:    requested key derivation function (KDF) version.
 *           Use HWKEY_KDF_VERSION_BEST for best version.
 * @arg2:    unused
 * @payload: seed data for key derivation. Size must be equal
 *           to size of requested key.
 *
 * Response:
 * @cmd:     HWKEY_RESP_DERIVE
 * @op_id:   echoed from request
 * @status:  operation result, one of enum hwkey_err.
 * @arg1:    KDF version used. Always different from request if
 *           request contained HWKEY_KDF_VERSION_BEST.
 * @arg2:    unused
 * @payload: derived key
 */

/**
 * struct hwkey_msg - common request/response structure for hwkey
 * @cmd:     command identifier
 * @op_id:   operation identifier, set by client and echoed by server.
 *           Used to identify a single operation. Only used if required
 *           by the client.
 * @status:  operation result. Should be set to 0 by client, set to
 *           a enum hwkey_err value by server.
 * @arg1:    first argument, meaning determined by command issued.
 *           Must be set to 0 if unused.
 * @arg2:    second argument, meaning determined by command issued
 *           Must be set to 0 if unused.
 * @payload: payload buffer, meaning determined by command issued
 */
struct hwkey_msg {
	uint32_t cmd;
	uint32_t op_id;
	uint32_t status;
	uint32_t arg1;
	uint32_t arg2;
	uint8_t payload[0];
};

