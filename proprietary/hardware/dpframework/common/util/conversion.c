#include <arm_neon.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "libmtkconv.h"

#include <string.h>
#if 0
#include "md5.c"

static int frame_test(uint8_t *pY, uint8_t *pC, size_t w, size_t h)
{
	static int count;
	static char *md5_path = NULL;
	static int md5_mode;
	static char *out_path = NULL;
	static int out_num;
	static int check = 0;

	char *path;
	char *mode;

	if (md5_path || out_path)
		goto test;
	if (check)
		return 0;
	check = 1;

	path = getenv("MTK_MD5_PATH");
	if (path) {
		mode = getenv("MTK_MD5_MODE");
		if (!mode) {
			printf("Invalid mode\n");
			return -1;
		}
		if (strcmp(mode, "check") == 0)
			md5_mode = 0;
		else if (strcmp(mode, "gen") == 0)
			md5_mode = 1;
		else {
			printf("Invalid mode\n");
			return -1;
		}
		md5_path = path;
		printf("MD5 Test path: %s, mode: %s\n", md5_path, mode);
	}

	path = getenv("MTK_OUT_PATH");
	if (path) {
		mode = getenv("MTK_OUT_NUM");
		if (!mode) {
			printf("Invalid mode\n");
			return -1;
		}
		if (strcmp(mode, "all") == 0)
			out_num = -1;
		else if (sscanf(mode, "%d", &out_num) > 0)
			;
		else {
			printf("Invalid mode\n");
			return -1;
		}
		out_path = path;
		printf("Frame out path: %s, num: %s\n", out_path, mode);
	}
	count = 0;
test:
	if (out_path && (out_num == -1 || out_num == count))
		if (mtk_out_frame(out_path, pY, pC, w, h, count))
			return -1;

	if (md5_path)
		if (md5_mode ? mtk_md5_gen(md5_path, pY, pC, w, h, count) :
			mtk_md5_check(md5_path, pY, pC, w, h, count))
			return -1;
	count++;
	return 0;
}
#endif

int do_b_to_nv12(uint8_t *pO0, uint8_t *pO1, uint8_t *pY, uint8_t *pC,
		  size_t bufWidth, size_t bufHeight)
{
	const size_t blocksWidth = bufWidth / 16;
	const size_t blocksHeight = bufHeight / 32;
	const size_t stride = blocksWidth * 16;
	const intptr_t deltaO0 = stride;
	const intptr_t deltaO01 = 16 - stride * 32;
	const intptr_t deltaO11 = 16 - stride * 16;
	const intptr_t deltaO02 = stride * (32 - 1);
	const intptr_t deltaO12 = stride * (16 - 1);

	size_t i, j, k;
	for (i = blocksHeight; i != 0; i--) {
		for (j = blocksWidth; j != 0; j--) {
			for (k = 16; k != 0; k--) {
#if defined(__aarch64__)
				asm volatile (

"	ld2	{v4.8b-v5.8b}, [%[pC]], #16		\n"

"	ld2	{v0.8b-v1.8b}, [%[pY]], #16		\n"
"	ld2	{v2.8b-v3.8b}, [%[pY]], #16		\n"

"	st2	{v4.8b-v5.8b}, [%[pO1]], %[deltaO]	\n"

"	st2	{v0.8b-v1.8b}, [%[pO0]], %[deltaO]	\n"
"	st2	{v2.8b-v3.8b}, [%[pO0]], %[deltaO]	\n"

	: [pO0] "+r" (pO0), [pO1] "+r" (pO1), [pY] "+r" (pY), [pC] "+r" (pC)
	: [deltaO] "r" (deltaO0)
	: "memory", "v0", "v1", "v2", "v3", "v4", "v5"

				);
#elif defined(__ARM_ARCH_7A__)
				asm volatile (

"	vld2.8	{d20-d21}, [%[pC] :128]!		\n"

"	pld	[%[pC], #32]				\n"

"	vld2.8	{d16-d17}, [%[pY] :128]!		\n"
"	vld2.8	{d18-d19}, [%[pY] :128]!		\n"

"	pld	[%[pY], #32]				\n"

"	vst2.8	{d20-d21}, [%[pO1] :128], %[deltaO]	\n"

"	vst2.8	{d16-d17}, [%[pO0] :128], %[deltaO]	\n"
"	vst2.8	{d18-d19}, [%[pO0] :128], %[deltaO]	\n"

	: [pO0] "+r" (pO0), [pO1] "+r" (pO1), [pY] "+r" (pY), [pC] "+r" (pC)
	: [deltaO] "r" (deltaO0)
	: "memory", "d16", "d17", "d18", "d19", "d20", "d21"

				);
#else
				return -1;
#endif
			}
			pO0 += deltaO01;
			pO1 += deltaO11;
		}
		pO0 += deltaO02;
		pO1 += deltaO12;
	}
	return 0;
}

int do_b_to_i420(uint8_t *pO0, uint8_t *pO1, uint8_t *pO2, uint8_t *pY, uint8_t *pC,
		  size_t bufWidth, size_t bufHeight)
{
	const size_t blocksWidth = bufWidth / 16;
	const size_t blocksHeight = bufHeight / 32;
	const size_t stride = blocksWidth * 16;
	const intptr_t deltaO0 = stride;
	const intptr_t deltaO1 = stride/2;
	const intptr_t deltaO01 = 16 - stride * 32;
	const intptr_t deltaO11 = 16/2 - stride/2 * 16;
	const intptr_t deltaO02 = stride * (32 - 1);
	const intptr_t deltaO12 = stride/2 * (16 - 1);

	size_t i, j, k;
	for (i = blocksHeight; i != 0; i--) {
		for (j = blocksWidth; j != 0; j--) {
			for (k = 16; k != 0; k--) {
#if defined(__aarch64__)
				asm volatile (

"	ld2	{v4.8b-v5.8b}, [%[pC]], #16		\n"

"	ld2	{v0.8b-v1.8b}, [%[pY]], #16		\n"
"	ld2	{v2.8b-v3.8b}, [%[pY]], #16		\n"

"	st1	{v4.8b}, [%[pO1]], %[deltaO1]		\n"
"	st1	{v5.8b}, [%[pO2]], %[deltaO1]		\n"

"	st2	{v0.8b-v1.8b}, [%[pO0]], %[deltaO0]	\n"
"	st2	{v2.8b-v3.8b}, [%[pO0]], %[deltaO0]	\n"

	: [pO0] "+r" (pO0), [pO1] "+r" (pO1), [pO2] "+r" (pO2), [pY] "+r" (pY), [pC] "+r" (pC)
	: [deltaO0] "r" (deltaO0), [deltaO1] "r" (deltaO1)
	: "memory", "v0", "v1", "v2", "v3", "v4", "v5"

				);
#elif defined(__ARM_ARCH_7A__)
				asm volatile (

"	vld2.8	{d20-d21}, [%[pC] :128]!		\n"

"	pld	[%[pC], #32]				\n"

"	vld2.8	{d16-d17}, [%[pY] :128]!		\n"
"	vld2.8	{d18-d19}, [%[pY] :128]!		\n"

"	pld	[%[pY], #32]				\n"

"	vst1.8	{d20}, [%[pO1] :64], %[deltaO1]		\n"
"	vst1.8	{d21}, [%[pO2] :64], %[deltaO1]		\n"

"	vst2.8	{d16-d17}, [%[pO0] :128], %[deltaO0]	\n"
"	vst2.8	{d18-d19}, [%[pO0] :128], %[deltaO0]	\n"

	: [pO0] "+r" (pO0), [pO1] "+r" (pO1), [pO2] "+r" (pO2), [pY] "+r" (pY), [pC] "+r" (pC)
	: [deltaO0] "r" (deltaO0), [deltaO1] "r" (deltaO1)
	: "memory", "d16", "d17", "d18", "d19", "d20", "d21"

				);
#else
				return -1;
#endif
			}
			pO0 += deltaO01;
			pO1 += deltaO11;
			pO2 += deltaO11;
		}
		pO0 += deltaO02;
		pO1 += deltaO12;
		pO2 += deltaO12;
	}
	return 0;
}

int do_b_to_yuy2(uint8_t *pO, uint8_t *pY, uint8_t *pC,
		  size_t bufWidth, size_t bufHeight)
{
	const size_t blocksWidth = bufWidth / 16;
	const size_t blocksHeight = bufHeight / 32;
	const size_t bpp = 2;
	const size_t stride = blocksWidth * 16 * bpp;
	const intptr_t deltaO0 = stride;
	const intptr_t deltaO1 = 16 * bpp - stride * 32;
	const intptr_t deltaO2 = stride * (32 - 1);

	size_t i, j, k;
	for (i = blocksHeight; i != 0; i--) {
		for (j = blocksWidth; j != 0; j--) {
			for (k = 16; k != 0; k--) {
#if defined(__aarch64__)
				asm volatile (

"	ld2	{v6.8b-v7.8b}, [%[pC]], #16		\n"

"	ld2	{v0.8b-v1.8b}, [%[pY]], #16		\n"
"	ld2	{v4.8b-v5.8b}, [%[pY]], #16		\n"

"	mov	v3.8b, v7.8b				\n"
"	mov	v2.8b, v1.8b				\n"
"	mov	v1.8b, v6.8b				\n"
"	mov	v6.8b, v5.8b				\n"
"	mov	v5.8b, v1.8b				\n"

"	st4	{v0.8b-v3.8b}, [%[pO]], %[deltaO]	\n"
"	st4	{v4.8b-v7.8b}, [%[pO]], %[deltaO]	\n"

	: [pO] "+r" (pO), [pY] "+r" (pY), [pC] "+r" (pC)
	: [deltaO] "r" (deltaO0)
	: "memory", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7"

				);
#elif defined(__ARM_ARCH_7A__)
				asm volatile (

"	vld2.8	{d17,d19}, [%[pC] :128]!		\n"

"	pld	[%[pC], #32]				\n"

"	vld2.8	{d16,d18}, [%[pY] :128]!		\n"
"	vld2.8	{d20,d22}, [%[pY] :128]!		\n"

"	pld	[%[pY], #32]				\n"

"	vmov	d21, d17				\n"
"	vmov	d23, d19				\n"

"	vst4.8	{d16-d19}, [%[pO] :256], %[deltaO]	\n"
"	vst4.8	{d20-d23}, [%[pO] :256], %[deltaO]	\n"

	: [pO] "+r" (pO), [pY] "+r" (pY), [pC] "+r" (pC)
	: [deltaO] "r" (deltaO0)
	: "memory", "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23"

				);
#else
				return -1;
#endif
			}
			pO += deltaO1;
		}
		pO += deltaO2;
	}
	return 0;
}

int do_b_to_xrgb(uint8_t *pO, uint8_t *pY, uint8_t *pC,
		  size_t bufWidth, size_t bufHeight)
{
#if defined(__ARM_ARCH_7A__)
	const int16x4_t coefficient = { 22970, -11272, -23396, 29032 };
	const int16x8_t m_128 = { -128, -128, -128, -128, -128, -128, -128, -128 };

	const size_t blocksWidth = bufWidth / 16;
	const size_t blocksHeight = bufHeight / 32;
	const size_t bpp = 4;
	const size_t stride = blocksWidth * 16 * bpp;
	const intptr_t deltaO0 = stride - 8 * bpp;
	const intptr_t deltaO1 = 16 * bpp - stride * 32;
	const intptr_t deltaO2 = stride * (32 - 1);

	size_t i, j, k;
	for (i = blocksHeight; i != 0; i--) {
		for (j = blocksWidth; j != 0; j--) {
			for (k = 16; k != 0; k--) {
				asm volatile (

"	vld2.8	{d20-d21}, [%[pC] :128]!		\n"

"	vaddw.u8	q8, %q[m_128], d20		\n"
"	vaddw.u8	q9, %q[m_128], d21		\n"

"	pld	[%[pC], #32]				\n"

"	vmull.s16	q12, d16, %P[coefficient][1]	\n"
"	vmull.s16	q13, d17, %P[coefficient][1]	\n"
"	vmull.s16	q14, d16, %P[coefficient][3]	\n"
"	vmull.s16	q15, d17, %P[coefficient][3]	\n"
"	vmull.s16	q10, d18, %P[coefficient][0]	\n"
"	vmull.s16	q11, d19, %P[coefficient][0]	\n"
"	vmlal.s16	q12, d18, %P[coefficient][2]	\n"
"	vmlal.s16	q13, d19, %P[coefficient][2]	\n"

"	vld2.8	{d16-d17}, [%[pY] :128]!		\n"
"	vld2.8	{d18-d19}, [%[pY] :128]!		\n"

"	vrshrn.s32	d20, q10, #14			\n"
"	vrshrn.s32	d21, q11, #14			\n"
"	vrshrn.s32	d22, q12, #15			\n"
"	vrshrn.s32	d23, q13, #15			\n"
"	vrshrn.s32	d24, q14, #14			\n"
"	vrshrn.s32	d25, q15, #14			\n"

"	pld	[%[pY], #32]				\n"

"	vaddw.u8	q13, q10, d16			\n"
"	vaddw.u8	q14, q11, d16			\n"
"	vaddw.u8	q15, q12, d16			\n"
"	vqmovun.s16	d10, q13			\n"
"	vqmovun.s16	d9, q14				\n"
"	vqmovun.s16	d8, q15				\n"
"	vaddw.u8	q13, q10, d17			\n"
"	vaddw.u8	q14, q11, d17			\n"
"	vaddw.u8	q15, q12, d17			\n"
"	vqmovun.s16	d14, q13			\n"
"	vqmovun.s16	d13, q14			\n"
"	vqmovun.s16	d12, q15			\n"
"	vzip.8	d10, d14				\n"
"	vzip.8	d9, d13					\n"
"	vzip.8	d8, d12					\n"

"	vst4.8	{d8-d11}, [%[pO] :256]!			\n"
"	vst4.8	{d12-d15}, [%[pO] :256], %[deltaO]	\n"

"	vaddw.u8	q13, q10, d18			\n"
"	vaddw.u8	q14, q11, d18			\n"
"	vaddw.u8	q15, q12, d18			\n"
"	vqmovun.s16	d10, q13			\n"
"	vqmovun.s16	d9, q14				\n"
"	vqmovun.s16	d8, q15				\n"
"	vaddw.u8	q13, q10, d19			\n"
"	vaddw.u8	q14, q11, d19			\n"
"	vaddw.u8	q15, q12, d19			\n"
"	vqmovun.s16	d14, q13			\n"
"	vqmovun.s16	d13, q14			\n"
"	vqmovun.s16	d12, q15			\n"
"	vzip.8	d10, d14				\n"
"	vzip.8	d9, d13					\n"
"	vzip.8	d8, d12					\n"

"	vst4.8	{d8-d11}, [%[pO] :256]!			\n"
"	vst4.8	{d12-d15}, [%[pO] :256], %[deltaO]	\n"

	: [pO] "+r" (pO), [pY] "+r" (pY), [pC] "+r" (pC)
	: [deltaO] "r" (deltaO0),
	  [coefficient] "w" (coefficient), [m_128] "w" (m_128)
	: "memory", "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15",
	  "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23",
	  "d24", "d25", "d26", "d27", "d28", "d29", "d30", "d31"

				);
			}
			pO += deltaO1;
		}
		pO += deltaO2;
	}
	return 0;
#else
	return -1;
#endif
}

#if 0
int do_b_to_rgb16(uint8_t *pO, uint8_t *pY, uint8_t *pC,
		  size_t bufWidth, size_t bufHeight)
{
#if defined(__ARM_ARCH_7A__)
	const int16x4_t coefficient = { 22970, -11272, -23396, 29032 };
	const int16x8_t m_128 = { -128, -128, -128, -128, -128, -128, -128, -128 };

	const size_t blocksWidth = bufWidth / 16;
	const size_t blocksHeight = bufHeight / 32;
	const size_t bpp = 2;
	const size_t stride = blocksWidth * 16 * bpp;
	const intptr_t deltaO0 = stride - 8 * bpp;
	const intptr_t deltaO1 = 16 * bpp - stride * 32;
	const intptr_t deltaO2 = stride * (32 - 1);

	size_t i, j, k;

	if (frame_test(pY, pC, bufWidth, bufHeight))
		return -1;

	for (i = blocksHeight; i != 0; i--) {
		for (j = blocksWidth; j != 0; j--) {
			for (k = 16; k != 0; k--) {
				asm volatile (

"	vld2.8	{d20-d21}, [%[pC] :128]!		\n"

"	vaddw.u8	q8, %q[m_128], d20		\n"
"	vaddw.u8	q9, %q[m_128], d21		\n"

"	pld	[%[pC], #32]				\n"

"	vmull.s16	q12, d16, %P[coefficient][1]	\n"
"	vmull.s16	q13, d17, %P[coefficient][1]	\n"
"	vmull.s16	q14, d16, %P[coefficient][3]	\n"
"	vmull.s16	q15, d17, %P[coefficient][3]	\n"
"	vmull.s16	q10, d18, %P[coefficient][0]	\n"
"	vmull.s16	q11, d19, %P[coefficient][0]	\n"
"	vmlal.s16	q12, d18, %P[coefficient][2]	\n"
"	vmlal.s16	q13, d19, %P[coefficient][2]	\n"

"	vld2.8	{d16-d17}, [%[pY] :128]!		\n"
"	vld2.8	{d18-d19}, [%[pY] :128]!		\n"

"	vrshrn.s32	d20, q10, #14			\n"
"	vrshrn.s32	d21, q11, #14			\n"
"	vrshrn.s32	d22, q12, #15			\n"
"	vrshrn.s32	d23, q13, #15			\n"
"	vrshrn.s32	d24, q14, #14			\n"
"	vrshrn.s32	d25, q15, #14			\n"

"	pld	[%[pY], #32]				\n"

"	vaddw.u8	q13, q10, d16			\n"
"	vaddw.u8	q14, q11, d16			\n"
"	vaddw.u8	q15, q12, d16			\n"
"	vqmovun.s16	d10, q13			\n"
"	vqmovun.s16	d9, q14				\n"
"	vqmovun.s16	d8, q15				\n"
"	vaddw.u8	q13, q10, d17			\n"
"	vaddw.u8	q14, q11, d17			\n"
"	vaddw.u8	q15, q12, d17			\n"
"	vqmovun.s16	d14, q13			\n"
"	vqmovun.s16	d13, q14			\n"
"	vqmovun.s16	d12, q15			\n"
"	vzip.8	d10, d14				\n"
"	vzip.8	d9, d13					\n"
"	vzip.8	d8, d12					\n"

"	vshr.u8	d8, d8, #3				\n"
"	vshr.u8	d9, d9, #2				\n"
"	vshr.u8	d10, d10, #3				\n"
"	vmovl.u8	q15, d8				\n"
"	vmovl.u8	q14, d9				\n"
"	vmovl.u8	q13, d10			\n"
"	vshl.u16	q14, q14, #5			\n"
"	vshl.u16	q13, q13, #11			\n"
"	vorr	q15, q15, q14				\n"
"	vorr	q15, q15, q13				\n"
"	vst1.8	{q15}, [%[pO] :128]!			\n"

"	vshr.u8	d12, d12, #3				\n"
"	vshr.u8	d13, d13, #2				\n"
"	vshr.u8	d14, d14, #3				\n"
"	vmovl.u8	q15, d12			\n"
"	vmovl.u8	q14, d13			\n"
"	vmovl.u8	q13, d14			\n"
"	vshl.u16	q14, q14, #5			\n"
"	vshl.u16	q13, q13, #11			\n"
"	vorr	q15, q15, q14				\n"
"	vorr	q15, q15, q13				\n"
"	vst1.8	{q15}, [%[pO] :128], %[deltaO]		\n"

"	vaddw.u8	q13, q10, d18			\n"
"	vaddw.u8	q14, q11, d18			\n"
"	vaddw.u8	q15, q12, d18			\n"
"	vqmovun.s16	d10, q13			\n"
"	vqmovun.s16	d9, q14				\n"
"	vqmovun.s16	d8, q15				\n"
"	vaddw.u8	q13, q10, d19			\n"
"	vaddw.u8	q14, q11, d19			\n"
"	vaddw.u8	q15, q12, d19			\n"
"	vqmovun.s16	d14, q13			\n"
"	vqmovun.s16	d13, q14			\n"
"	vqmovun.s16	d12, q15			\n"
"	vzip.8	d10, d14				\n"
"	vzip.8	d9, d13					\n"
"	vzip.8	d8, d12					\n"

"	vshr.u8	d8, d8, #3				\n"
"	vshr.u8	d9, d9, #2				\n"
"	vshr.u8	d10, d10, #3				\n"
"	vmovl.u8	q15, d8				\n"
"	vmovl.u8	q14, d9				\n"
"	vmovl.u8	q13, d10			\n"
"	vshl.u16	q14, q14, #5			\n"
"	vshl.u16	q13, q13, #11			\n"
"	vorr	q15, q15, q14				\n"
"	vorr	q15, q15, q13				\n"
"	vst1.8	{q15}, [%[pO] :128]!			\n"

"	vshr.u8	d12, d12, #3				\n"
"	vshr.u8	d13, d13, #2				\n"
"	vshr.u8	d14, d14, #3				\n"
"	vmovl.u8	q15, d12			\n"
"	vmovl.u8	q14, d13			\n"
"	vmovl.u8	q13, d14			\n"
"	vshl.u16	q14, q14, #5			\n"
"	vshl.u16	q13, q13, #11			\n"
"	vorr	q15, q15, q14				\n"
"	vorr	q15, q15, q13				\n"
"	vst1.8	{q15}, [%[pO] :128], %[deltaO]		\n"

	: [pO] "+r" (pO), [pY] "+r" (pY), [pC] "+r" (pC)
	: [deltaO] "r" (deltaO0),
	  [coefficient] "w" (coefficient), [m_128] "w" (m_128)
	: "memory", "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15",
	  "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23",
	  "d24", "d25", "d26", "d27", "d28", "d29", "d30", "d31"

				);
			}
			pO += deltaO1;
		}
		pO += deltaO2;
	}
	return 0;
#else
	return -1;
#endif
}
#endif

int do_rgbx_to_nv12(uint8_t *pY, uint8_t *pC, uint8_t *pSrc,
		    size_t width, size_t height)
{
#if defined(__ARM_ARCH_7A__)
	if (!pSrc || !pY || !pC || !width || !height ||
	    (width & 0xF) || (height & 0x1))
		return -1;

	const int16x4_t coeff0 = { 46, 157, 15, -25 };
	const int16x4_t coeff1 = { -86, 112, -102, -10 };
	const uint8x16_t m_16 = { 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 };
	const uint8x8_t m_128 = { 128, 128, 128, 128, 128, 128, 128, 128 };
	uint8_t *pY0_end = pY + width;
	uint8_t *pY1 = pY0_end;
	uint8_t *pSrc1 = pSrc + width * 4;

	while (height > 0) {
		while (pY < pY0_end) {
			asm volatile (
	/* load_rgbx_16x1 rgb0 */
"	vld4.8		{d6, d8, d10, d12}, [%[pSrc]]!		\n"
"	vld4.8		{d7, d9, d11, d13}, [%[pSrc]]!		\n"
"	pld		[%[pSrc], #64]				\n"

	/* pad_rgb_8x2 */
"	vpaddl.u8	q7, q6					\n"
"	vpaddl.u8	q8, q5					\n"
"	vpaddl.u8	q9, q4					\n"

	/* calc_y_16x1 */
"	vmovl.u8	q11, d12				\n"
"	vmovl.u8	q12, d13				\n"
"	vmul.s16	q13, q11, %P[coeff0][0]			\n"
"	vmul.s16	q14, q12, %P[coeff0][0]			\n"

"	vmovl.u8	q11, d10				\n"
"	vmovl.u8	q12, d11				\n"
"	vmla.s16	q13, q11, %P[coeff0][1]			\n"
"	vmla.s16	q14, q12, %P[coeff0][1]			\n"

"	vmovl.u8	q11, d8					\n"
"	vmovl.u8	q12, d9					\n"
"	vmla.s16	q13, q11, %P[coeff0][2]			\n"
"	vmla.s16	q14, q12, %P[coeff0][2]			\n"

"	vrshrn.i16	d30, q13, #8				\n"
"	vrshrn.i16	d31, q14, #8				\n"
"	vadd.u8		q15, q15, %q[m_16]			\n"

	/* write_y8_16x1 y0*/
"	vstmia		%[pY]!, {q15}				\n"

	/* load_rgbx_16x1 rgb1 */
"	vld4.8		{d6, d8, d10, d12}, [%[pSrc1]]!		\n"
"	vld4.8		{d7, d9, d11, d13}, [%[pSrc1]]!		\n"
"	pld		[%[pSrc1], #64]				\n"

	/* padal_rgb_8x2 */
"	vpadal.u8	q7, q6					\n"
"	vpadal.u8	q8, q5					\n"
"	vpadal.u8	q9, q4					\n"

	/* down_sample_rgb_8x2 */
"	vrshr.u16	q7, q7, #2				\n"
"	vrshr.u16	q8, q8, #2				\n"
"	vrshr.u16	q9, q9, #2				\n"

	/* calc_y_16x1 */
"	vmovl.u8	q11, d12				\n"
"	vmovl.u8	q12, d13				\n"
"	vmul.s16	q13, q11, %P[coeff0][0]			\n"
"	vmul.s16	q14, q12, %P[coeff0][0]			\n"

"	vmovl.u8	q11, d10				\n"
"	vmovl.u8	q12, d11				\n"
"	vmla.s16	q13, q11, %P[coeff0][1]			\n"
"	vmla.s16	q14, q12, %P[coeff0][1]			\n"

"	vmovl.u8	q11, d8					\n"
"	vmovl.u8	q12, d9					\n"
"	vmla.s16	q13, q11, %P[coeff0][2]			\n"
"	vmla.s16	q14, q12, %P[coeff0][2]			\n"

"	vrshrn.i16	d30, q13, #8				\n"
"	vrshrn.i16	d31, q14, #8				\n"
"	vadd.u8		q15, q15, %q[m_16]			\n"

	/* write_y8_16x1 y1*/
"	vstmia		%[pY1]!, {q15}				\n"

	/* calc_c_8x1  u, U */
"	vmul.s16	q15, q7, %P[coeff0][3]			\n"
"	vmla.s16	q15, q8, %P[coeff1][0]			\n"
"	vmla.s16	q15, q9, %P[coeff1][1]			\n"
"	vrshrn.i16	d20, q15, #8				\n"
"	vadd.u8		d20, d20, %P[m_128]			\n"

	/* calc_c_8x1  v, V */
"	vmul.s16	q15, q7, %P[coeff1][1]			\n"
"	vmla.s16	q15, q8, %P[coeff1][2]			\n"
"	vmla.s16	q15, q9, %P[coeff1][3]			\n"
"	vrshrn.i16	d21, q15, #8				\n"
"	vadd.u8		d21, d21, %P[m_128]			\n"

	/* write_c_nv12_8x1 */
"	vst2.i8		{d20-d21}, [%[pC]]!			\n"

	: [pY] "+r" (pY), [pY1] "+r" (pY1), [pC] "+r" (pC), [pSrc] "+r" (pSrc),
	  [pSrc1] "+r" (pSrc1)
	: [coeff0] "w" (coeff0), [coeff1] "w" (coeff1), [m_128] "w" (m_128), [m_16] "w" (m_16)
	: "memory", "d6", "d7", "d8", "d9", "d10", "d11", "d12", "d13", "d14", "d15",
	  "d16", "d17", "d18", "d19", "d20", "d21", "d22", "d23", "d24", "d25", "d26",
	  "d27", "d28", "d29", "d30", "d31"
			);
		}

		pY = pY1;
		pY0_end = pY1 + width;
		pSrc = pSrc1;
		pY1 = pY + width;
		pSrc1 = pSrc + width * 4;

		height -= 2;
	}

	return 0;
#else
	return -1;
#endif
}

