/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#ifndef __GF_QUEUE_H__
#define __GF_QUEUE_H__

#include "gf_error.h"
#include "gf_common.h"

#ifdef __cplusplus
extern "C" {
#endif

extern gf_error_t gf_queue_init();
extern gf_error_t gf_queue_exit();

extern gf_error_t gf_queue_clear_unused_irq();
extern uint8_t gf_empty_queue();
extern gf_error_t gf_enqueue(uint8_t cmd_type);
extern gf_error_t gf_dequeue(uint8_t *cmd_type);

#ifdef __cplusplus
}
#endif

#endif  // __GF_QUEUE_H__
