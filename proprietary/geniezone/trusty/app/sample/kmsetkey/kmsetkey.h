/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws.
 * The information contained herein is confidential and proprietary to
 * MediaTek Inc. and/or its licensors.
 * Except as otherwise provided in the applicable licensing terms with
 * MediaTek Inc. and/or its licensors, any reproduction, modification, use or
 * disclosure of MediaTek Software, and information contained herein, in whole
 * or in part, shall be strictly prohibited.
*/

#ifndef __KMSETKEY_H__
#define __KMSETKEY_H__

#include <sys/types.h>

#define KMSETKEY_PORT		"com.mediatek.kmsetkey"
#define MAX_MSG_SIZE		4096
#define IPC_MSG_SIZE		16
#define RESP_FLAG		0x80000000
#define DONE_FLAG		0x40000000

enum kmsetkey_cmd {
	RSA_KEY = 0x00,
	RSA_SIGN = 0x01,
	RSA_VERI = 0x02,
	ECC_KEY = 0x10,
	ECC_SIGN = 0x11,
	ECC_VERI = 0x12,
	AES_128_ENC = 0x20,
	AES_256_ENC = 0x21,
	SHA_HASH = 0x30,
	RSA_TEST = 0x100,
	ECC_TEST = 0x101,
	AES_TEST = 0x102,
	SHA_TEST = 0x103,
};

struct kmsetkey_msg {
	uint32_t cmd;
	uint8_t payload[0];
};

#endif
