/*
 * Copyright (c) 2015, ARM Limited and Contributors. All rights reserved.
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

#ifndef __UFS_H
#define __UFS_H

#include <types.h>

/* UFS crypto control flags */
#define UFS_AES_CTL_FDE_INIT    (1 << 0)
#define UFS_AES_CTL_SUSPEND     (1 << 1)
#define UFS_AES_CTL_RESUME      (1 << 2)
#define UFS_AES_CTL_SET_KEY     (1 << 3)
#define UFS_AES_CTL_FBE_INIT    (1 << 8)

/* UFS generic control flags */
#define UFS_MPHY_VA09_CG_CTL    (1 << 0)

int32_t  ufs_crypto_ctl(uint32_t ctl, uint32_t param2, uint32_t param3);
void     ufs_crypto_hie_init(void);
uint32_t ufs_crypto_hie_init_request(uint32_t hie_para);
void     ufs_crypto_hie_cfg_request(unsigned int para1, unsigned int para2, unsigned int para3);
void     ufs_crypto_hie_program_key(uint32_t addr, uint32_t hie_para, uint32_t key1, uint32_t key2, uint32_t key3);
int32_t  ufs_generic_ctl(uint32_t ctl, uint32_t param2, uint32_t param3);

#endif /* __UFS_H */

