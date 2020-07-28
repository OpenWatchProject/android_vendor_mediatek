/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <compiler.h>

__BEGIN_CDECLS

typedef handle_t keymaster_session_t;

/**
 * keymaster_open() - Opens a Keymaster session
 *
 * Return: a keymaster_session_t >= 0 on success, or an error code < 0
 * on failure.
 */
int keymaster_open(void);

/**
 * keymaster_close() - Opens a Keymaster session
 * @session: the keymaster_session_t to close.
 *
 */
void keymaster_close(keymaster_session_t session);

/**
 * keymaster_get_auth_token_key() - Retrieves the auth token signature key
 * @session: the keymaster_session_t to close.
 * @key_buf_p: pointer to buffer pointer to be allocated and filled with auth token key.
 *             Ownership of this pointer is transferred to the caller and must be deallocated
 *             with a call to free().
 * @size_p: set to the allocated size of key_buf
 *
 */
int keymaster_get_auth_token_key(keymaster_session_t session, uint8_t** key_buf_p, size_t* size_p);

__END_CDECLS

