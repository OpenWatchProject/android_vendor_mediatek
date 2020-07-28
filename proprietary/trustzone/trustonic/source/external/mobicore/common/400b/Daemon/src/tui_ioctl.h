/*
 * Copyright (c) 2013-2018 TRUSTONIC LIMITED
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

#define MAX_BUFFER_NUMBER 3

#ifndef u32
#define u32 uint32_t
#endif

/* Command header */
struct tlc_tui_command_t {
	u32     id;
	u32     data[2];
};

/* Response header */
struct tlc_tui_response_t {
	u32	id;
	u32	return_code;
	int	ion_fd[MAX_BUFFER_NUMBER];
	u32	screen_metrics[3];
};

/* Command IDs */
/*  */
#define TLC_TUI_CMD_NONE                0
/* Start TUI session */
#define TLC_TUI_CMD_START_ACTIVITY      1
/* Stop TUI session */
#define TLC_TUI_CMD_STOP_ACTIVITY       2
/*
 * Queue a buffer
 * IN: index of buffer to be queued
 */
#define TLC_TUI_CMD_QUEUE               3
/*
 * Queue a new buffer and dequeue the buffer currently displayed
 * IN: indexes of buffer to be queued
 */
#define TLC_TUI_CMD_QUEUE_DEQUEUE       4
/*
 * Alloc buffers
 * IN: number of buffers
 * OUT: ion fd
 */
#define TLC_TUI_CMD_ALLOC_FB            5
/* Free buffers */
#define TLC_TUI_CMD_FREE_FB             6
/* hide secure surface */
#define TLC_TUI_CMD_HIDE_SURFACE        7
#define TLC_TUI_CMD_GET_RESOLUTION      8

/* Return codes */
#define TLC_TUI_OK                  0
#define TLC_TUI_ERROR               1
#define TLC_TUI_ERR_UNKNOWN_CMD     2

/*
 * defines for the ioctl TUI driver module function call from user space.
 */
#define TUI_DEV_NAME	"t-base-tui"

#define TUI_IO_MAGIC	't'

#define TUI_IO_NOTIFY	_IOW(TUI_IO_MAGIC, 1, u32)
#define TUI_IO_WAITCMD	_IOR(TUI_IO_MAGIC, 2, struct tlc_tui_command_t)
#define TUI_IO_ACK	_IOW(TUI_IO_MAGIC, 3, struct tlc_tui_response_t)
#define TUI_IO_INIT_DRIVER	_IO(TUI_IO_MAGIC, 4)

#ifdef INIT_COMPLETION
#define reinit_completion(x) INIT_COMPLETION(*(x))
#endif

#endif /* TUI_IOCTL_H_ */
