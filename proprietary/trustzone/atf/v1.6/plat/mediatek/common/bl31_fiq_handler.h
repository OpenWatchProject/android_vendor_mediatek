/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __BL31_FIQ_HANDLER_H__
#define __BL31_FIQ_HANDLER_H__
#include <gicv3.h>

#define	MAX_FIQ_LINE_NUM				8
#define MT_EDGE_POLARITY_BITFILED_LEN	2
#define MT_EDGE_POLARITY_BITFILED_MASK	((1<<2)-1)

#define INT_EDGE_RISING		((MT_EDGE_SENSITIVE << MT_EDGE_POLARITY_BITFILED_LEN) | MT_POLARITY_HIGH)
#define INT_EDGE_FALLING	((MT_EDGE_SENSITIVE << MT_EDGE_POLARITY_BITFILED_LEN) | MT_POLARITY_LOW)
#define INT_LEVEL_HIGH		((MT_LEVEL_SENSITIVE << MT_EDGE_POLARITY_BITFILED_LEN) | MT_POLARITY_HIGH)
#define INT_LEVEL_LOW		((MT_LEVEL_SENSITIVE << MT_EDGE_POLARITY_BITFILED_LEN) | MT_POLARITY_LOW)

#define GET_INT_EDGE_TYPE(n)	((n >> MT_EDGE_POLARITY_BITFILED_LEN) & MT_EDGE_POLARITY_BITFILED_MASK)
#define GET_INT_POLARITY_TYPE(n)	(n & MT_EDGE_POLARITY_BITFILED_MASK)

typedef void (*fiq_handler_t)(void *cookie);

struct fiq_desc {
	uint32_t fiqnr;
	fiq_handler_t fiq_num_handler;
	uint32_t int_group;
	void *cookie;
};

extern uint64_t bl31_el3_fiq_handler(uint32_t id,
			   uint32_t flags,
			   void *handle,
			   void *cookie);
extern uint32_t bl31_fiq_dispatcher(uint32_t fiqId);
extern uint32_t request_fiq(uint32_t fiq_num, fiq_handler_t handler, uint32_t trigger_type, uint32_t int_group, void *cookie);

#endif /*  __BL31_FIQ_HANDLER_H__ */
