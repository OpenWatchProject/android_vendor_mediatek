/*
 * Copyright (c) 2013-2016 TRUSTONIC LIMITED
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

#ifndef __TLCNATIVEWINDOW_H__
#define __TLCNATIVEWINDOW_H__

#include "tui_ioctl.h"

/*
 * Init native Window
 */
uint32_t tlcAnwInit(uint32_t max_dequeued_buffers,
                    int *min_undequeued_buffers, uint32_t width, uint32_t height);

/*
 * Dequeue n number of buffers
 */
uint32_t tlcAnwDequeueBuffers(int *ionHandle,
                              uint32_t nBuffers,
                              uint32_t *pWidth,
                              uint32_t *pHeight,
                              uint32_t *pStride);

/*
 * Queue a native buffer
 */
uint32_t tlcAnwQueueBuffer(int buffId);

/*
 * Cancel a native buffer buffer (give it back to the system, without it being displayed)
 */
uint32_t tlcAnwCancelBuffer(int buffId);

/*
 * Queue buffer with a specific id, buffId
 * And dequeue another one.
 */
uint32_t tlcAnwQueueDequeueBuffer(int buffId);

/*
 * Hide the native window
 */
void tlcAnwHide();

/*
 * Clear the native window
 */
void tlcAnwClear();

#endif /* ____TLCNATIVEWINDOW_H___H__ */

