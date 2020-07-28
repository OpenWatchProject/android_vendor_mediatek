/*
 * Copyright (c) 2016 TRUSTONIC LIMITED
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

/*
 * Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
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



#ifndef __PLAT_TBASE_H__
#define __PLAT_TBASE_H__


/* Defines */
#define PLAT_TBASE_INPUT_HWIDENTITY                         (0x1)
#define PLAT_TBASE_INPUT_HWKEY                              (0x2)
#define PLAT_TBASE_INPUT_RNG                                (0x3)
#define PLAT_TBASE_INPUT_FREQ                               (0x4)
#define PLAT_TBASE_INPUT_FIRMWARE_VERSION                   (0x5)
#define PLAT_TBASE_INPUT_FIRMWARE_BINARY_VERSION            (0x6)
#define PLAT_TBASE_INPUT_SYS_TA_RP_ENABLED                  (0x7)

#define PLAT_TBASE_INPUT_OK              ((uint32_t)0)
#define PLAT_TBASE_INPUT_ERROR           ((uint32_t)-1)

/*
 * Commands issued by Trustonic TEE that must be implemented by the integrator.
 *
 * param DataId: the command (see below for details)
 * param Length: Size of the requested information (passed by Kinibi and already set to the expected size, 16 bytes).
 * param Out: Buffer which will contain the requested information, already allocated.
 *
 * return PLAT_TBASE_INPUT_OK if success.
 *        PLAT_TBASE_INPUT_ERROR in case of error.
 *
 * Supported command:
 *   PLAT_TBASE_INPUT_HWIDENTITY, request to get the HW unique key
 *   (could be public, but must be cryptographically unique).
 *   PLAT_TBASE_INPUT_HWKEY, request to get the HW key (must be secret, better for security if unique but not critical)
 *   PLAT_TBASE_INPUT_RNG, request to get random number (used as seed in Trustonic TEE)
 */
#if defined(ATF_1_5)
uint32_t plat_tbase_input(u_register_t DataId, u_register_t *Length, void *Out);
#else
uint32_t plat_tbase_input(uint64_t DataId, uint64_t *Length, void *Out);
#endif /* ATF_1_5 */

/*
 * Abstraction API that must be customized by the integrator if "FIQ Forward" feature is supported.
 * Else, Implementation can remain empty.
 */
void plat_tbase_fiqforward_init(void);

/* Only for hikey FIQ Forward Testing */
uint32_t plat_tbase_fiqforward_init_test(
	uint32_t smc_fid,
	uint64_t x1,
	uint64_t x2,
	uint64_t x3,
	uint64_t x4,
	void *handle);

/*
 * Abstraction API that must be customized by the integrator if "Blacklist memory" feature is supported.
 * Else, Implementation can remain empty.
 */
void plat_blacklist_memory_init(void);



/*
 * Callback called by Trustonic TEE if a "Forwared FIQ" has catch by the TEE
 * (Kinibi handler for EL1->EL3 forwarded FIQs).
 *
 * param fiqId: the number of the FIQ catch by the TEE.
 *
 * return PLAT_TBASE_INPUT_OK if success.
 *        PLAT_TBASE_INPUT_ERROR in case of error.
 */
uint32_t plat_tbase_forward_fiq(uint32_t fiqId);


/*
 * Callback for platform to handle fastcall from NWd
 * @return PLAT_TBASE_INPUT_ERROR if fastcall was not handled
 */
uint32_t plat_tbase_handle_fastcall(uint32_t smc_fid,
				    uint64_t x1,
				    uint64_t x2,
				    uint64_t x3,
				    uint64_t x4,
				    void *handle);

#endif /* __PLAT_TBASE_H__ */
