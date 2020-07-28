/****************************************************************************
 ****************************************************************************
 ***
 ***   This header was automatically generated from a Linux kernel header
 ***   of the same name, to make information necessary for userspace to
 ***   call into the kernel available to libc.  It contains only constants,
 ***   structures, and macros generated from the original header, and thus,
 ***   contains no copyrightable information.
 ***
 ***   To edit the content of this header, modify the corresponding
 ***   source file (e.g. under external/kernel-headers/original/) then
 ***   run bionic/libc/kernel/tools/update_all.py
 ***
 ***   Any manual change here will be lost the next time this script will
 ***   be run. You've been warned!
 ***
 ****************************************************************************
 ****************************************************************************/

#ifndef _HAL_TYPES_PRIVATE_H_
#define _HAL_TYPES_PRIVATE_H_

#include "val_types_private.h"
#include "hal_types_public.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ADD_QUEUE(queue, index, q_type, q_address, q_offset, q_value, q_mask)       \
	{                                                                                   \
		queue[index].type     = q_type;                                                   \
		queue[index].address  = q_address;                                                \
		queue[index].offset   = q_offset;                                                 \
		queue[index].value    = q_value;                                                  \
		queue[index].mask     = q_mask;                                                   \
		index = index + 1;                                                                \
	}       /* /< ADD QUEUE command */


/**
 * @par Enumeration
 *   HAL_CODEC_TYPE_T
 * @par Description
 *   This is the item used for codec type
 */
typedef enum __HAL_CODEC_TYPE_T {
	HAL_CODEC_TYPE_VDEC,                /* /< VDEC */
	HAL_CODEC_TYPE_VENC,                /* /< VENC */
	HAL_CODEC_TYPE_MAX = 0xFFFFFFFF     /* /< MAX Value */
}
HAL_CODEC_TYPE_T;


/**
 * @par Enumeration
 *   HAL_CMD_T
 * @par Description
 *   This is the item used for hal command type
 */
typedef enum _HAL_CMD_T {
	HAL_CMD_SET_CMD_QUEUE,              /* /< set command queue */
	HAL_CMD_SET_POWER,                  /* /< set power */
	HAL_CMD_SET_ISR,                    /* /< set ISR */
	HAL_CMD_GET_CACHE_CTRL_ADDR,        /* /< get cahce control address */
	HAL_CMD_MAX = 0xFFFFFFFF            /* /< MAX value */
} HAL_CMD_T;


/**
 * @par Enumeration
 *   REGISTER_GROUP_T
 * @par Description
 *   This is the item used for register group
 */
typedef enum _REGISTER_GROUP_T {
	VDEC_SYS,           /* /< VDEC_SYS */
	VDEC_MISC,          /* /< VDEC_MISC */
	VDEC_VLD,           /* /< VDEC_VLD */
	VDEC_VLD_TOP,       /* /< VDEC_VLD_TOP */
	VDEC_MC,            /* /< VDEC_MC */
	VDEC_AVC_VLD,       /* /< VDEC_AVC_VLD */
	VDEC_AVC_MV,        /* /< VDEC_AVC_MV */
	VDEC_HEVC_VLD,      /* /< VDEC_HEVC_VLD */
	VDEC_HEVC_MV,       /* /< VDEC_HEVC_MV */
	VDEC_PP,            /* /< VDEC_PP */
	/* VDEC_SQT, */
	VDEC_VP8_VLD,       /* /< VDEC_VP8_VLD */
	VDEC_VP6_VLD,       /* /< VDEC_VP6_VLD */
	VDEC_VP8_VLD2,      /* /< VDEC_VP8_VLD2 */
	VENC_HW_BASE,       /* /< VENC_HW_BASE */
	VENC_LT_HW_BASE,    /* /< VENC_HW_LT_BASE */
	VENC_MP4_HW_BASE,   /* /< VENC_MP4_HW_BASE */
	VDEC_VP9_VLD,       /* /< VDEC_VP9_VLD*/
	VDEC_UFO,           /* /< VDEC_UFO*/
	VCODEC_MAX          /* /< VCODEC_MAX */
} REGISTER_GROUP_T;


/**
 * @par Enumeration
 *   REGISTER_GROUP_T
 * @par Description
 *   This is the item used for driver command type
 */
typedef enum _VCODEC_DRV_CMD_TYPE {
	ENABLE_HW_CMD,              /* /< ENABLE_HW_CMD */
	DISABLE_HW_CMD,             /* /< DISABLE_HW_CMD */
	WRITE_REG_CMD,              /* /< WRITE_REG_CMD */
	READ_REG_CMD,               /* /< READ_REG_CMD */
	WRITE_SYSRAM_CMD,           /* /< WRITE_SYSRAM_CMD */
	READ_SYSRAM_CMD,            /* /< READ_SYSRAM_CMD */
	MASTER_WRITE_CMD,           /* /< MASTER_WRITE_CMD */
	WRITE_SYSRAM_RANGE_CMD,     /* /< WRITE_SYSRAM_RANGE_CMD */
	READ_SYSRAM_RANGE_CMD,      /* /< READ_SYSRAM_RANGE_CMD */
	SETUP_ISR_CMD,              /* /< SETUP_ISR_CMD */
	WAIT_ISR_CMD,               /* /< WAIT_ISR_CMD */
	TIMEOUT_CMD,                /* /< TIMEOUT_CMD */
	MB_CMD,                     /* /< MB_CMD */
	POLL_REG_STATUS_CMD,        /* /< POLL_REG_STATUS_CMD */
	END_CMD                     /* /< END_CMD */
} VCODEC_DRV_CMD_TYPE;


/**
 * @par Structure
 *  P_VCODEC_DRV_CMD_T
 * @par Description
 *  Pointer of VCODEC_DRV_CMD_T
 */
typedef struct __VCODEC_DRV_CMD_T *P_VCODEC_DRV_CMD_T;


/**
 * @par Structure
 *  VCODEC_DRV_CMD_T
 * @par Description
 *  driver command information
 */
typedef struct __VCODEC_DRV_CMD_T {
	VAL_UINT32_T type;          /* /< type */
	VAL_ULONG_T  address;       /* /< address */
	VAL_ULONG_T  offset;        /* /< offset */
	VAL_ULONG_T  value;         /* /< value */
	VAL_ULONG_T  mask;          /* /< mask */
} VCODEC_DRV_CMD_T;


struct vcodec_log_dbg_info {
	VAL_UINT32_T vcodec_internal_dbg;
	VAL_UINT32_T vcodec_get_input_dbg;
	VAL_UINT32_T vcodec_get_output_dbg;
	VAL_CHAR_T   vcodec_get_file_dbg[512];
    VAL_CHAR_T   vcodec_golden_file_dbg[MAX_FILE_NAME_SIZE];
};

typedef struct __INPUT_CONFIG_T
{
    VAL_DRIVER_TYPE_T   eCodecType;
    VAL_CHAR_T          cInFile[MAX_FILE_NAME_SIZE];
    VAL_CHAR_T          cOutFile[MAX_FILE_NAME_SIZE];
    VAL_CHAR_T          cGoldenPath[MAX_FILE_NAME_SIZE];
    VAL_BOOL_T          bUseGolden;
    float               fTestErrorRate;
    VAL_CHAR_T          pvExtraParam[MAX_FILE_NAME_SIZE];
} INPUT_CONFIG_T;


/**
* @par Structure
* VCODEC_DRV_WAITISR_CB_T
* @par Description
* VCODEC HW control callback function for VPUD
*/
typedef struct __VCODEC_DRV_HW_CTRL_CB_T_ { // union extend 64bits for TEE
	union {
		VAL_VOID_T* pHandle;
		VAL_UINT64_T pHandle_ext64;
	};
	union {
		VAL_UINT32_T u4CodecId;
		VAL_UINT64_T u4CodecId_ext64;
	};
	union {
		VAL_VOID_T (*pfnHwCtrl)(VAL_VOID_T*, VAL_UINT32_T, VAL_UINT32_T);
		VAL_UINT64_T pfnHwCtrl_ext64;
	};
} VCODEC_DRV_HW_CTRL_CB_T;
typedef VCODEC_DRV_HW_CTRL_CB_T *P_VCODEC_DRV_HW_CTRL_CB_T;


/**
* @par Structure
* VDEC_DRV_WAITISR_CB_T
* @par Description
* VDEC wait ISR callback function for VPUD
*/
typedef struct _VCODEC_DRV_WAIT_ISR_CB_T_ {
	union {
		VAL_VOID_T* pHandle;
		VAL_UINT64_T pHandle_ext64;
	};
	union {
		VAL_UINT32_T u4CodecId;
		VAL_UINT64_T u4CodecId_ext64;
	};
	union {
		VAL_UINT32_T (*pfnWaitISR)(VAL_VOID_T*, VAL_UINT32_T, VAL_UINT32_T *);
		VAL_UINT64_T pfnWaitISR_ext64;
	};
} VCODEC_DRV_WAIT_ISR_CB_T;
typedef VCODEC_DRV_WAIT_ISR_CB_T *P_VCODEC_DRV_WAIT_ISR_CB_T;

/**
 * @par Structure
 *  HAL_HANDLE_T
 * @par Description
 *  hal handle information
 */
typedef struct _HAL_HANDLE_T_ {
	VAL_INT32_T     fd_vdec;            /* /< fd_vdec */
	VAL_INT32_T     fd_venc;            /* /< fd_venc */
	VAL_MEMORY_T    rHandleMem;         /* /< rHandleMem */
	VAL_ULONG_T     mmap[VCODEC_MAX];   /* /< mmap[VCODEC_MAX] */
	VAL_DRIVER_TYPE_T    driverType;    /* /< driverType */
	VAL_UINT32_T    u4TimeOut;          /* /< u4TimeOut */
	VAL_UINT32_T    u4FrameCount;       /* /< u4FrameCount */
#ifdef DumpReg__
	FILE *pf_out;
#endif
	VAL_BOOL_T      bProfHWTime;        /* /< bProfHWTime */
	VAL_UINT64_T    u8HWTime[2];        /* /< u8HWTime */
	VCODEC_DRV_WAIT_ISR_CB_T rCallbackWaitISR;
	VCODEC_DRV_HW_CTRL_CB_T rCallbackHWCtrl;
} HAL_HANDLE_T;


#ifdef __cplusplus
}
#endif

#endif /* #ifndef _HAL_TYPES_PRIVATE_H_ */
