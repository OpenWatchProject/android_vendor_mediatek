/*
 * Copyright (C) 2011-2014 MediaTek Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef MTK_GRALLOC_EXTRA_H
#define MTK_GRALLOC_EXTRA_H

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>

#include <hardware/gralloc1.h>

__BEGIN_DECLS

/*
 * GE_API 1: add ge_query(), ge_perform(), support ge_device
 * GE_API 2: add need_resident(), get_platform_format()
 */
#define GE_API_VERSION 2

enum {
	GRALLOC_EXTRA_OK,
	GRALLOC_EXTRA_UNKNOWN,
	GRALLOC_EXTRA_NOT_SUPPORTED,
	GRALLOC_EXTRA_NO_IMPLEMENTATION,
	GRALLOC_EXTRA_NOT_INIT,
	GRALLOC_EXTRA_INVALIDE_PARAMS,
	GRALLOC_EXTRA_INVALIDE_OPERATION,
	GRALLOC_EXTRA_ERROR,
};

typedef int GRALLOC_EXTRA_RESULT;

/* enum for query() */
typedef enum {
	GRALLOC_EXTRA_GET_ION_FD = 1,    /* int */
	GRALLOC_EXTRA_GET_FB_MVA,        /* uintptr_t, deprecated */
	GRALLOC_EXTRA_GET_SECURE_HANDLE, /* uint32_t */

	/* output: int */
	GRALLOC_EXTRA_GET_WIDTH = 10,
	GRALLOC_EXTRA_GET_HEIGHT,
	GRALLOC_EXTRA_GET_STRIDE,
	GRALLOC_EXTRA_GET_VERTICAL_STRIDE,
	GRALLOC_EXTRA_GET_ALLOC_SIZE,
	GRALLOC_EXTRA_GET_FORMAT,
	GRALLOC_EXTRA_GET_USAGE,
	GRALLOC_EXTRA_GET_VERTICAL_2ND_STRIDE,
	GRALLOC_EXTRA_GET_BYTE_2ND_STRIDE,

	/* output: uint64_t */
	GRALLOC_EXTRA_GET_ID = 50,
	GRALLOC_EXTRA_GET_CONSUMER_USAGE,
	GRALLOC_EXTRA_GET_PRODUCER_USAGE,

	/* output: gralloc_extra_sf_info_t */
	GRALLOC_EXTRA_GET_SF_INFO = 100,
	GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO = GRALLOC_EXTRA_GET_SF_INFO, /* deprecated */

	/* output: gralloc_extra_smvr_info_t */
	GRALLOC_EXTRA_GET_SMVR_INFO,

	/* output: gralloc_extra_buf_debug_t */
	GRALLOC_EXTRA_GET_IOCTL_ION_DEBUG,

	/* output: uint32_t */
	GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC,

	/* output: ge_rotate_info_t */
	GRALLOC_EXTRA_GET_ROTATE_INFO,

	/* output: gralloc_gpu_compression_info_t */
	GRALLOC_EXTRA_GET_GPU_COMPRESSION_INFO,

	/* output: gralloc_gpu_yuyv rotation: int32_t */
	GRALLOC_EXTRA_GET_ORIENTATION,

	/* output: ge_hdr_info_t */
	GRALLOC_EXTRA_GET_HDR_INFO,

	/* output: ge_video_info_t */
	GRALLOC_EXTRA_GET_VIDEO_INFO,

	/* output: ge_hwc_info_t */
	GRALLOC_EXTRA_GET_HWC_INFO,

	/* output: buffer timestmp: uint64_t */
	GRALLOC_EXTRA_GET_TIMESTAMP,

	/* output: ge_timestmp_info_t */
	GRALLOC_EXTRA_GET_TIMESTAMP_INFO,

	/* output: ge_pq__scltm_info_t */
	GRALLOC_EXTRA_GET_PQ_SCLTM_INFO,

	/* output: ge_pq_mira_vision_info_t */
	GRALLOC_EXTRA_GET_PQ_MIRA_VISION_INFO,

} GRALLOC_EXTRA_ATTRIBUTE_QUERY;

/* enum for perform() */
typedef enum {
	/* input: gralloc_extra_sf_info_t * */
	GRALLOC_EXTRA_SET_SF_INFO = 100,
	GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO = GRALLOC_EXTRA_SET_SF_INFO, /* depreacted */

	/* output: gralloc_extra_smvr_info_t */
	GRALLOC_EXTRA_SET_SMVR_INFO,

	/* input: gralloc_extra_buf_debug_t * */
	GRALLOC_EXTRA_SET_IOCTL_ION_DEBUG,

	/* input: NULL */
	GRALLOC_EXTRA_ALLOC_SECURE_BUFFER_HWC,
	GRALLOC_EXTRA_FREE_SECURE_BUFFER_HWC,
	GRALLOC_EXTRA_FREE_SEC_BUFFER_HWC = GRALLOC_EXTRA_FREE_SECURE_BUFFER_HWC, /* deprecated */

	/* input: gralloc_gpu_yuyv rotation */
	GRALLOC_EXTRA_SET_ORIENTATION,

	/* input: ge_hdr_info_t */
	GRALLOC_EXTRA_SET_HDR_INFO,

	/* input: ge_video_info_t */
	GRALLOC_EXTRA_SET_VIDEO_INFO,

	/* input ge_hwc_info_t */
	GRALLOC_EXTRA_SET_HWC_INFO,

	/* input: buffer timestmp: uint64_t */
	GRALLOC_EXTRA_SET_TIMESTAMP,

	/* input: ge_timestmp_info_t */
	GRALLOC_EXTRA_SET_TIMESTAMP_INFO,

	/* input: ge_pq__scltm_info_t */
	GRALLOC_EXTRA_SET_PQ_SCLTM_INFO,

	/* input: ge_pq_mira_vision_info_t */
	GRALLOC_EXTRA_SET_PQ_MIRA_VISION_INFO,

} GRALLOC_EXTRA_ATTRIBUTE_PERFORM;

int gralloc_extra_query(buffer_handle_t handle, GRALLOC_EXTRA_ATTRIBUTE_QUERY attribute, void *out_pointer);

int gralloc_extra_perform(buffer_handle_t handle, GRALLOC_EXTRA_ATTRIBUTE_PERFORM attribute, void *in_pointer);

#define GRALLOC_EXTRA_MAKE_BIT(start_bit, index)        ( (index##u) << (start_bit) )
#define GRALLOC_EXTRA_MAKE_MASK(start_bit, end_bit)     ( ( ((unsigned int)-1) >> (sizeof(int) * __CHAR_BIT__ - 1 - (end_bit) + (start_bit) ) ) << (start_bit) )

#define GRALLOC_EXTRA_UNMAKE_BIT(start_bit, index)        ( (index##u) >> (start_bit) )

/* bits in sf_info.status */
enum {
	/* TYPE: bit 0~1 */
	GRALLOC_EXTRA_BIT_TYPE_CPU          = GRALLOC_EXTRA_MAKE_BIT(0,0),
	GRALLOC_EXTRA_BIT_TYPE_GPU          = GRALLOC_EXTRA_MAKE_BIT(0,1),
	GRALLOC_EXTRA_BIT_TYPE_VIDEO        = GRALLOC_EXTRA_MAKE_BIT(0,2),
	GRALLOC_EXTRA_BIT_TYPE_CAMERA       = GRALLOC_EXTRA_MAKE_BIT(0,3),
	GRALLOC_EXTRA_MASK_TYPE             = GRALLOC_EXTRA_MAKE_MASK(0,1),

	/* UFO alignment: bit 2~3 Temporary add back to prevent 6795 build fail.
	   Remove after 6795 SF/HWC finishes porting
	 */
	GRALLOC_EXTRA_BIT_UFO_16_32         = GRALLOC_EXTRA_MAKE_BIT(2,1),
	GRALLOC_EXTRA_BIT_UFO_32_32         = GRALLOC_EXTRA_MAKE_BIT(2,2),
	GRALLOC_EXTRA_BIT_UFO_64_64         = GRALLOC_EXTRA_MAKE_BIT(2,3),
	GRALLOC_EXTRA_MASK_UFO_ALIGN        = GRALLOC_EXTRA_MAKE_MASK(2,3),

	/* ColorFormat: bit 2~6 */
	GRALLOC_EXTRA_BIT_CM_YV12           = GRALLOC_EXTRA_MAKE_BIT(2,0),
	GRALLOC_EXTRA_BIT_CM_YUYV           = GRALLOC_EXTRA_MAKE_BIT(2,1),
	GRALLOC_EXTRA_BIT_CM_I420           = GRALLOC_EXTRA_MAKE_BIT(2,2),
	GRALLOC_EXTRA_BIT_CM_NV12           = GRALLOC_EXTRA_MAKE_BIT(2,3),
	GRALLOC_EXTRA_BIT_CM_YUV420_FLEX    = GRALLOC_EXTRA_MAKE_BIT(2,4),
	GRALLOC_EXTRA_BIT_CM_NV12_BLK       = GRALLOC_EXTRA_MAKE_BIT(2,5),
	GRALLOC_EXTRA_BIT_CM_NV12_BLK_FCM   = GRALLOC_EXTRA_MAKE_BIT(2,6),
	GRALLOC_EXTRA_BIT_CM_UFO            = GRALLOC_EXTRA_MAKE_BIT(2,7),
	GRALLOC_EXTRA_BIT_CM_PRIVATE        = GRALLOC_EXTRA_MAKE_BIT(2,8),
	GRALLOC_EXTRA_BIT_CM_YV12_10BIT     = GRALLOC_EXTRA_MAKE_BIT(2,9),
	GRALLOC_EXTRA_BIT_CM_YUYV_10BIT     = GRALLOC_EXTRA_MAKE_BIT(2,10),
	GRALLOC_EXTRA_BIT_CM_I420_10BIT     = GRALLOC_EXTRA_MAKE_BIT(2,11),
	GRALLOC_EXTRA_BIT_CM_YUV420_FLEX_10BIT      = GRALLOC_EXTRA_MAKE_BIT(2,12),
	GRALLOC_EXTRA_BIT_CM_NV12_BLK_10BIT_H       = GRALLOC_EXTRA_MAKE_BIT(2,13),
	GRALLOC_EXTRA_BIT_CM_NV12_BLK_10BIT_V       = GRALLOC_EXTRA_MAKE_BIT(2,14),
	GRALLOC_EXTRA_BIT_CM_UFO_10BIT_H    = GRALLOC_EXTRA_MAKE_BIT(2,15),
	GRALLOC_EXTRA_BIT_CM_UFO_10BIT_V    = GRALLOC_EXTRA_MAKE_BIT(2,16),
	GRALLOC_EXTRA_BIT_CM_NV12_PRIVATE_10BIT     = GRALLOC_EXTRA_MAKE_BIT(2,17),
	GRALLOC_EXTRA_MASK_CM               = GRALLOC_EXTRA_MAKE_MASK(2,6),

	/* Secure switch: bit 7 */
	GRALLOC_EXTRA_BIT_NORMAL            = GRALLOC_EXTRA_MAKE_BIT(7,0),
	GRALLOC_EXTRA_BIT_SECURE            = GRALLOC_EXTRA_MAKE_BIT(7,1),
	GRALLOC_EXTRA_MASK_SECURE           = GRALLOC_EXTRA_MAKE_MASK(7,7),

	/* S3D Mode: bit 8~9 */
	GRALLOC_EXTRA_BIT_S3D_2D            = GRALLOC_EXTRA_MAKE_BIT(8,0),
	GRALLOC_EXTRA_BIT_S3D_SBS           = GRALLOC_EXTRA_MAKE_BIT(8,1),
	GRALLOC_EXTRA_BIT_S3D_TAB           = GRALLOC_EXTRA_MAKE_BIT(8,2),
	GRALLOC_EXTRA_MASK_S3D              = GRALLOC_EXTRA_MAKE_MASK(8,9),

	/* DIRTY: bit 10, 11 */
	GRALLOC_EXTRA_BIT_UNDIRTY           = GRALLOC_EXTRA_MAKE_BIT(10,0),
	GRALLOC_EXTRA_BIT_DIRTY             = GRALLOC_EXTRA_MAKE_MASK(10,11),
	GRALLOC_EXTRA_MASK_DIRTY            = GRALLOC_EXTRA_MAKE_MASK(10,11),
	/* DIRTY: SF, 10 */
	GRALLOC_EXTRA_BIT_SF_DIRTY          = GRALLOC_EXTRA_MAKE_MASK(10,10),
	GRALLOC_EXTRA_MASK_SF_DIRTY         = GRALLOC_EXTRA_MAKE_MASK(10,10),
	/* DIRTY: AUX, 11 */
	GRALLOC_EXTRA_BIT_AUX_DIRTY         = GRALLOC_EXTRA_MAKE_MASK(11,11),
	GRALLOC_EXTRA_MASK_AUX_DIRTY        = GRALLOC_EXTRA_MAKE_MASK(11,11),

	/* Orientation: bit 12~15 */
	GRALLOC_EXTRA_MASK_ORIENT           = GRALLOC_EXTRA_MAKE_MASK(12,15),

	/* Alpha: bit 16~23 */
	GRALLOC_EXTRA_MASK_ALPHA            = GRALLOC_EXTRA_MAKE_MASK(16,23),

	/* Blending: bit 24~25 */
	GRALLOC_EXTRA_MASK_BLEND            = GRALLOC_EXTRA_MAKE_MASK(24,25),

	/* Dirty Param: bit 26 */
	GRALLOC_EXTRA_MASK_DIRTY_PARAM      = GRALLOC_EXTRA_MAKE_MASK(26,26),

	/* FLUSH: bit 27 */
	GRALLOC_EXTRA_BIT_FLUSH             = GRALLOC_EXTRA_MAKE_BIT(27,0),
	GRALLOC_EXTRA_BIT_NOFLUSH           = GRALLOC_EXTRA_MAKE_BIT(27,1),
	GRALLOC_EXTRA_MASK_FLUSH            = GRALLOC_EXTRA_MAKE_MASK(27,27),

	/* YUV Color SPACE: bit 29~31 */
	GRALLOC_EXTRA_BIT_YUV_NOT_SET       = GRALLOC_EXTRA_MAKE_BIT(29,0),
	GRALLOC_EXTRA_BIT_YUV_BT601_NARROW  = GRALLOC_EXTRA_MAKE_BIT(29,1),
	GRALLOC_EXTRA_BIT_YUV_BT601_FULL    = GRALLOC_EXTRA_MAKE_BIT(29,2),
	GRALLOC_EXTRA_BIT_YUV_BT709_NARROW  = GRALLOC_EXTRA_MAKE_BIT(29,3),
	GRALLOC_EXTRA_BIT_YUV_BT709_FULL    = GRALLOC_EXTRA_MAKE_BIT(29,4),
	GRALLOC_EXTRA_BIT_YUV_BT2020_NARROW = GRALLOC_EXTRA_MAKE_BIT(29,5),
	GRALLOC_EXTRA_BIT_YUV_BT2020_FULL = GRALLOC_EXTRA_MAKE_BIT(29,6),
	GRALLOC_EXTRA_MASK_YUV_COLORSPACE   = GRALLOC_EXTRA_MAKE_MASK(29,31),
};

/* bits in sf_info.status2 */
enum {
	/* TYPE: bit 0~2 */
	GRALLOC_EXTRA_BIT2_LAYER_NORMAL      = 0x0,
	GRALLOC_EXTRA_BIT2_LAYER_NAV         = 0x1, /* Navigation bar */
	GRALLOC_EXTRA_MASK2_LAYER_TYPE       = GRALLOC_EXTRA_MAKE_MASK(0,2),

	/* YUY2 for Camera Rotation : bit 3~5 */
	GRALLOC_EXTRA_BIT2_YUY2_ROT0         = GRALLOC_EXTRA_MAKE_BIT(3,0),
	GRALLOC_EXTRA_BIT2_YUY2_R1_FLIP_H    = GRALLOC_EXTRA_MAKE_BIT(3,1),
	GRALLOC_EXTRA_BIT2_YUY2_R2_FLIP_V    = GRALLOC_EXTRA_MAKE_BIT(3,2),
	GRALLOC_EXTRA_BIT2_YUY2_R3_ROT180    = GRALLOC_EXTRA_MAKE_BIT(3,3),
	GRALLOC_EXTRA_BIT2_YUY2_R4_ROT90     = GRALLOC_EXTRA_MAKE_BIT(3,4),
	GRALLOC_EXTRA_BIT2_YUY2_R5           = GRALLOC_EXTRA_MAKE_BIT(3,5),
	GRALLOC_EXTRA_BIT2_YUY2_R6           = GRALLOC_EXTRA_MAKE_BIT(3,6),
	GRALLOC_EXTRA_BIT2_YUY2_R7_ROT270    = GRALLOC_EXTRA_MAKE_BIT(3,7),
	GRALLOC_EXTRA_MASK2_YUY2_ROT         = GRALLOC_EXTRA_MAKE_MASK(3,5),

	/* Video Width for Ultra resuolution : bit 6~17 (12bits, 0-4095) */
	/* Video original width = ((v & mask) >> shift); */
	GRALLOC_EXTRA_SHIFT2_VIDEO_WIDTH     = 6,
	GRALLOC_EXTRA_MASK2_VIDEO_WIDTH      = GRALLOC_EXTRA_MAKE_MASK(6,17),

#define GRALLOC_EXTRA_BIT2_TO_VIDEO_WIDTH(status2) \
	(((status2) & GRALLOC_EXTRA_MASK2_VIDEO_WIDTH) >> GRALLOC_EXTRA_SHIFT2_VIDEO_WIDTH)
#define GRALLOC_EXTRA_VIDEO_WIDTH_TO_BIT2(video_width) \
	(((video_width) << GRALLOC_EXTRA_SHIFT2_VIDEO_WIDTH) & GRALLOC_EXTRA_MASK2_VIDEO_WIDTH)

	/* UI PQ enable/disable support : bit 18 (default off) */
	GRALLOC_EXTRA_BIT2_UI_PQ_OFF        = GRALLOC_EXTRA_MAKE_BIT(18,0),
	GRALLOC_EXTRA_BIT2_UI_PQ_ON         = GRALLOC_EXTRA_MAKE_BIT(18,1),
	GRALLOC_EXTRA_MASK2_UI_PQ           = GRALLOC_EXTRA_MAKE_MASK(18,18),

	/* Video PQ enable/disable support : bit 19 (default on) */
	GRALLOC_EXTRA_BIT2_VIDEO_PQ_ON      = GRALLOC_EXTRA_MAKE_BIT(19,0),
	GRALLOC_EXTRA_BIT2_VIDEO_PQ_OFF     = GRALLOC_EXTRA_MAKE_BIT(19,1),
	GRALLOC_EXTRA_MASK2_VIDEO_PQ        = GRALLOC_EXTRA_MAKE_MASK(19,19),

	/* UI PQ ALGO reset flag : bit 20 (default off) */
	GRALLOC_EXTRA_BIT2_UI_PQ_ALGO_RESET_OFF = GRALLOC_EXTRA_MAKE_BIT(20,0),
	GRALLOC_EXTRA_BIT2_UI_PQ_ALGO_RESET_ON  = GRALLOC_EXTRA_MAKE_BIT(20,1),
	GRALLOC_EXTRA_MASK2_UI_PQ_ALGO_RESET    = GRALLOC_EXTRA_MAKE_MASK(20,20),

	/* Disable buffer compression : bit 21 (default off) */
	GRALLOC_EXTRA_BIT2_DISABLE_BUF_COMPRESSION = GRALLOC_EXTRA_MAKE_BIT(21,1),
	GRALLOC_EXTRA_MASK2_DISABLE_BUF_COMPRESSION = GRALLOC_EXTRA_MAKE_MASK(21,21),

	/* compressed buffer flag : bit 22 (default off) */
	GRALLOC_EXTRA_BIT2_BUF_NOT_COMPRESSED      = GRALLOC_EXTRA_MAKE_BIT(22,0),
	GRALLOC_EXTRA_BIT2_BUF_COMPRESSED          = GRALLOC_EXTRA_MAKE_BIT(22,1),
	GRALLOC_EXTRA_MASK2_BUF_COMPRESSION_STATUS = GRALLOC_EXTRA_MAKE_MASK(22,22),

	/* GPU to GPU flag : bit 23 (default off) */
	GRALLOC_EXTRA_BIT2_NOT_GPU_TO_GPU = GRALLOC_EXTRA_MAKE_BIT(23,0),
	GRALLOC_EXTRA_BIT2_GPU_TO_GPU     = GRALLOC_EXTRA_MAKE_BIT(23,1),
	GRALLOC_EXTRA_MASK2_GPU_TO_GPU    = GRALLOC_EXTRA_MAKE_MASK(23,23),

	/* GPU to DPU flag : bit 24 (default off) */
	GRALLOC_EXTRA_BIT2_NOT_GPU_TO_DPU = GRALLOC_EXTRA_MAKE_BIT(24,0),
	GRALLOC_EXTRA_BIT2_GPU_TO_DPU     = GRALLOC_EXTRA_MAKE_BIT(24,1),
	GRALLOC_EXTRA_MASK2_GPU_TO_DPU    = GRALLOC_EXTRA_MAKE_MASK(24,24),
};

typedef enum {
	/* normal secure buffer */
	GRALLOC_EXTRA_SECURE_BUFFER_TYPE_NORMAL,

	/* zero-initialized secure buffer */
	GRALLOC_EXTRA_SECURE_BUFFER_TYPE_ZERO
} GRALLOC_EXTRA_SECURE_BUFFER_TYPE;

typedef struct _crop_t {
	int32_t x, y, w, h;
} _crop_t;

/* extension data */
typedef struct ge_smvr_info_t {
	uint32_t frame_count;
	uint64_t timestamp[32];
        uint64_t timestamp_cam2fw;
} ge_smvr_info_t;

typedef struct ge_sf_info_t {
	/* magic number to make sure that obj is created by query() */
	uint32_t magic;

	/* used by SurfaceFlinger*/
	_crop_t src_crop;
	_crop_t dst_crop;

	/* buffer flags, please change this value by sf_set_status() */
	int32_t status;

	/* used for pre-allocate buffer */
	int32_t pool_id;

	/* used for profiling latency */
	uint32_t sequence;

	/* secure handle allocated by GRALLOC_EXTRA_GET_SECURE_HANDLE_HWC */
	uint32_t secure_handle_hwc;

	/* used for video frame */
	uint32_t timestamp;

	/* buffer flags, please change this value by sf_set_status2() */
	int32_t status2;

	/* video buffer status
	 * bit 31: valid bit, 0 = invalid (default), 1 = valid
	 *
	 * bit 30-25: Y stride alignment
	 * bit 24-19: CbCr stride alignment
	 * bit 18-13: height alignment
	 * alignment interpretation
	 *
	 * Y:
	 *   y_align = (videobuffer_status & 0x7FFFFFFF) >> 25;
	 *   if (0 == y_align) y_align = 1;
	 *   else y_align *= 2;
	 * CbCr:
	 *   cbcr_align = (videobuffer_status & 0x01FFFFFF) >> 19;
	 *   if (0 == cbcr_align) cbcr_align = 1;
	 *   else cbcr_align *= 2;
	 * Height:
	 *   h_align = (videobuffer_status & 0x0007FFFF) >> 13;
	 *   if (0 == h_align) h_align = 1;
	 *   else h_align *= 2;
	 *
	 * bit 12: deinterlace 0 = no, 1 = yes
	 */
	int32_t videobuffer_status;
} ge_sf_info_t;


typedef struct ge_hwc_info_t {
	/* for hwc mirror output buffer use*/
	_crop_t mirror_out_roi;
} ge_hwc_info_t;

int gralloc_extra_sf_set_status(ge_sf_info_t *sf_info, int32_t mask, int32_t value);
int gralloc_extra_sf_set_status2(ge_sf_info_t *sf_info, int32_t mask, int32_t value);

int gralloc_extra_is_private_format(int hal_format);
int gralloc_extra_cm_2_hal_format(int cm_format);

/* debug data, to facilitate information while debugging */
typedef struct ge_ion_debug_t {
	int data[4];
	char name[48];
} ge_ion_debug_t;

/* s* GPU */
typedef struct gralloc_rotate_info_t {
	int rotate_mode;
	int rotate_dx;
	int rotate_dy;
} gralloc_rotate_info_t;

/* s* GPU */
typedef struct gralloc_gpu_compression_info_t {
	/* GPU info */
	unsigned char ufo_enable;
	unsigned int ufo_length_buffer_base;
	unsigned char ufo_pattern_replace_en;
	unsigned int ufo_pattern;
} gralloc_gpu_compression_info_t;

/* HDR */
typedef struct ge_hdr_info_t {
	uint32_t u4ColorPrimaries; /* colour_primaries emum */
	uint32_t u4TransformCharacter; /* transfer_characteristics emum */
	uint32_t u4MatrixCoeffs; /* matrix_coeffs emum */
	uint32_t u4DisplayPrimariesX[3]; /* display_primaries_x */
	uint32_t u4DisplayPrimariesY[3]; /* display_primaries_y */
	uint32_t u4WhitePointX; /* white_point_x */
	uint32_t u4WhitePointY; /* white_point_y */
	uint32_t u4MaxDisplayMasteringLuminance; /* max_display_mastering_luminance */
	uint32_t u4MinDisplayMasteringLuminance; /* min_display_mastering_luminance */
	uint32_t u4MaxContentLightLevel; /* max_content_light_level */
	uint32_t u4MaxPicAverageLightLevel; /* max_pic_average_light_level */
} ge_hdr_info_t;

/* Video */
typedef struct ge_video_info_t {
	uint32_t width;
	uint32_t height;
} ge_video_info_t;

typedef struct {
	uint32_t orientation; /* camera specify */
} ge_misc_info_t;

typedef struct {
	uint64_t timestamp;
	uint64_t timestamp_queued;

	uint32_t reserved[12];
} ge_timestamp_info_t;

typedef struct {
	uint32_t scltmEnable; // 0:Disable 1:Enable
	uint32_t scltmPosition; // 1 ~ 1024 denote position of x/1024
	uint32_t scltmOrientation; // 0:Vertical 1:Horizontal
	uint32_t scltmTableIndex;
} ge_pq_scltm_info_t;

#define GE_PQ_MIRA_VISION_MAX_REG_COUNT 640

typedef struct {
	uint32_t inc_count;                                    /* 0: No any App settings */
	uint32_t version;                                      /* register value for which version */
	uint32_t reg_values[GE_PQ_MIRA_VISION_MAX_REG_COUNT];  /* register values array */
	uint32_t value_size;                                   /* number of register values (uint32_t) */
} ge_pq_mira_vision_info_t;

/* Number of other fds is set to 2 since IMG driver use total 3 fds */
#define NUM_OTHER_FDS 2

/* MTK private handle */
typedef struct __attribute__((packed)) ge_handle_t {
	int ge_fd;
	int share_fd;
	int dummy_fd[NUM_OTHER_FDS];
	int width;
	int height;
	int format;
	int stride;
	int vertical_stride;
	int vertical_2nd_stride;
	int byte_2nd_stride;
	int alloc_size;
	uint32_t sec_handle;
	uint64_t consumer_usage;
	uint64_t producer_usage;
} ge_handle_t;

inline ge_handle_t* get_ge_handle(buffer_handle_t hnd)
{
	if (NULL != hnd && hnd->version == sizeof(native_handle_t))
	{
		return (ge_handle_t *)(hnd->data);
	}
	else
	{
		return NULL;
	}
}

typedef ge_sf_info_t gralloc_extra_ion_sf_info_t;
typedef ge_hwc_info_t gralloc_extra_ion_hwc_info_t;
typedef ge_ion_debug_t gralloc_extra_ion_debug_t;

int gralloc_extra_get_platform_format(int in_format, uint64_t usage);
int gralloc_extra_need_resident(uint64_t usage);

__END_DECLS

#endif /* MTK_GRALLOC_EXTRA_H */
