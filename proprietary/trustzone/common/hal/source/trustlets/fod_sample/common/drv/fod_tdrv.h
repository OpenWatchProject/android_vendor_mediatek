/*
 * Copyright (c) 2018 MediaTek Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef FOD_TDRV_H
#define FOD_TDRV_H


#define SIZE_1KB (0x400)
#define SAPU_TEE_BUF_SIZE	(3*SIZE_1KB)	//only use for 3K, final 1K is for tee<->sDSP parameter
#define SAPU_TEE_PROP_SIZE	SIZE_1KB		//only use for 3K, final 1K is for tee<->sDSP parameter
#define IMAGE_WIDTH (1280)
#define IMAGE_HEIGHT (720)
#define IMAGE_HD_SIZE (IMAGE_WIDTH*IMAGE_HEIGHT)

/*
typedef struct {
	uint32_t operation;
	uint32_t output_size;
	uint64_t output_pa;
	uint64_t input1_pa;
	uint64_t input2_pa;
	uint32_t input1_size;
	uint32_t input2_size;
} __attribute__((packed)) sdsp_buf_t ;
*/

typedef enum VPU_DATA_FMT_ENUM{
    VPU_DATA_FMT_IMG_NV12  = 0x00, 	/* 420 2P Y/UV  */
    VPU_DATA_FMT_IMG_NV21,         	/* 420 2P Y/VU  */
    VPU_DATA_FMT_IMG_IYUV,         	/* 420 3P Y/U/V */
    VPU_DATA_FMT_IMG_I420,         	/* 420 3P Y/U/V */
    VPU_DATA_FMT_IMG_YV12,         	/* 420 3P Y/V/U */

    VPU_DATA_FMT_IMG_YUY2,         	/* 422 1P YUYV  */
    VPU_DATA_FMT_IMG_YVYU,         	/* 422 1P YVYU  */
    VPU_DATA_FMT_IMG_YV16,         	/* 422 3P Y/V/U */

    VPU_DATA_FMT_IMG_Y,         	/* Y only  		*/
    VPU_DATA_FMT_IMG_420_CHROME,  	/* u or v only	*/
    VPU_DATA_FMT_IMG_422_CHROME,   	/* u or v only	*/
    VPU_DATA_FMT_IMG_420_UV_IL,   	/* uv interleave*/
    VPU_DATA_FMT_IMG_422_UV_IL,   	/* uv interleave*/

    VPU_DATA_FMT_DATA,				/* pure data 	*/
    VPU_DATA_FMT_NULL				/* no data 		*/
} VPU_DATA_FMT_ENUM_T;

typedef uint8_t vpu_id_t;

struct vpu_plane {
	uint32_t stride;         /* if buffer type is image */
	uint32_t length;
	uint64_t ptr;            /* mva which is accessible by VPU */
};

typedef struct {
	vpu_id_t port_id;
	uint8_t format;
	uint8_t plane_count;
	uint32_t width;
	uint32_t height;
	struct vpu_plane planes[3];
} vpu_buffer_t;

typedef struct {
	uint32_t cmd;
	uint32_t result;
} vpu_prop_t;

#endif /* end of FOD_TDRV */
