/*
 * Copyright (c) 2013-2014 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TUI_IOCTL_H_
#define TUI_IOCTL_H_



/* Response header */
struct tlc_tui_response_t {
	uint32_t	id;
	uint32_t	return_code;
};

/* Command IDs */
#define TLC_TUI_CMD_NONE                0
#define TLC_TUI_CMD_START_ACTIVITY      1
#define TLC_TUI_CMD_STOP_ACTIVITY       2

/* Return codes */
#define TLC_TUI_OK                  0
#define TLC_TUI_ERROR               1
#define TLC_TUI_ERR_UNKNOWN_CMD     2


/*
 * defines for the ioctl TUI driver module function call from user space.
 */
#define TUI_DEV_NAME		"t-base-tui"

#define TUI_IO_MAGIC		't'

#define TUI_IO_NOTIFY	_IOW(TUI_IO_MAGIC, 1, uint32_t)
#define TUI_IO_WAITCMD	_IOR(TUI_IO_MAGIC, 2, uint32_t)
#define TUI_IO_ACK	_IOW(TUI_IO_MAGIC, 3, struct tlc_tui_response_t)

#endif /* TUI_IOCTL_H_ */
