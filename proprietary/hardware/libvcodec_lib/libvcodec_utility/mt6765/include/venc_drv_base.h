#include "venc_drv_if_private.h"
#include "vcodec_if_v2.h"
#include "val_algo.h"

#include <sys/time.h>
#include <stdio.h>

#ifndef _VENC_DRV_BASE_
#define _VENC_DRV_BASE_

#define DO_VCODEC_RESET(cmd, index)                                                             \
	{                                                                                               \
	}

typedef enum __VDDRV_MRESULT_T {
	VDDRV_MRESULT_SUCCESS = VAL_TRUE,  /* /< Represent success */
	VDDRV_MRESULT_FAIL = VAL_FALSE     /* /< Represent failure */
} VDDRV_MRESULT_T;

typedef struct __VENC_DRV_BASE_T {
	VAL_UINT32_T(*Init)(
		VAL_HANDLE_T * handle,
		VAL_HANDLE_T halhandle,
		VAL_HANDLE_T valhandle
	);
	VAL_UINT32_T(*Encode)(
		VAL_HANDLE_T handle,
		VENC_DRV_START_OPT_T eOpt,
		P_VENC_DRV_PARAM_FRM_BUF_T pFrameBuf,
		P_VENC_DRV_PARAM_BS_BUF_T pBitstreamBuf,
		VENC_DRV_DONE_RESULT_T * pResult
	);
	VAL_UINT32_T(*GetParam)(
		VAL_HANDLE_T handle,
		VENC_DRV_GET_TYPE_T a_eType,
		VAL_VOID_T * a_pvInParam,
		VAL_VOID_T * a_pvOutParam
	);
	VAL_UINT32_T(*SetParam)(
		VAL_HANDLE_T handle,
		VENC_DRV_SET_TYPE_T a_eType,
		VAL_VOID_T * a_pvInParam,
		VAL_VOID_T * a_pvOutParam
	);
	VAL_UINT32_T(*DeInit)(
		VAL_HANDLE_T handle
	); /* /< Function to do driver de-initialization */
} VENC_DRV_BASE_T;

/**
 * @par Structure
 *   mhalVdoDrv_t
 * @par Description
 *   This is a structure which store common video enc driver information
 */
typedef struct mhalVdoDrv_s {
	VAL_VOID_T                      *prCodecHandle;
	VAL_UINT32_T                    u4EncodedFrameCount;
	VCODEC_ENC_CALLBACK_T           rCodecCb;
	VIDEO_ENC_API_T                 *prCodecAPI;
	VENC_BS_T                       pBSBUF;

	VCODEC_ENC_BUFFER_INFO_T        EncoderInputParamNC;
	VENC_DRV_PARAM_BS_BUF_T         BSout;
	VENC_HYBRID_ENCSETTING          rVencSetting;
	VAL_UINT8_T                     *ptr;
} mhalVdoDrv_t;

typedef struct mhalVdoDrv_x64_s {
	union {
		VAL_VOID_T	               *prCodecHandle;
		VAL_UINT64_T 		        prCodecHandle_dummy;
	};
	VAL_UINT32_T                    u4EncodedFrameCount;
	VCODEC_ENC_CALLBACK_X64_T       rCodecCb;
	/* VIDEO_ENC_API_T                 *prCodecAPI; */
	union {
		VIDEO_ENC_API_X64_T         *prCodecAPI;
		VAL_UINT64_T 		        prCodecAPI_dummy;
	};
	VENC_BS_X64_T                   pBSBUF;

	VCODEC_ENC_BUFFER_INFO_X64_T    EncoderInputParamNC;
	VENC_DRV_PARAM_BS_BUF_T         BSout;
	VENC_HYBRID_ENCSETTING          rVencSetting;
	union {
		VAL_UINT8_T 				*ptr;
		VAL_UINT64_T 		        ptr_dummy;
	};
} mhalVdoDrv_x64_t;

typedef struct __VENC_HYBRID_HANDLE_T {
	mhalVdoDrv_t                    rMhalVdoDrv;
	VAL_MEMORY_T                    rBSDrvWorkingMem;
	VAL_UINT32_T                    nOmxTids;
	VAL_VCODEC_THREAD_ID_T          rThreadID;
	VIDEO_ENC_WRAP_HANDLE_T         hWrapper;
	VAL_VOID_T                      *pDrvModule;    /* /< used for dlopen and dlclose */
	// TODO: for lastframe buf yxz
	union {
	   VAL_HANDLE_T 			   va_last_frame_buf;
	   VAL_UINT64_T 			   va_last_frame_buf_dummy;
	};
        VAL_UINT32_T                   u4PrependHeaderStoreSize;
      	VAL_VOID_T	               *pHeaderbuf;
} VENC_HYBRID_HANDLE_T;

/* for hybrid H264 encoder use which for sync 64bit tee*/
typedef struct __VENC_HYBRID_HANDLE_X64_T {
	mhalVdoDrv_x64_t                rMhalVdoDrv;
	VAL_MEMORY_T                    rBSDrvWorkingMem;
	VAL_UINT32_T                    nOmxTids;
	VAL_VCODEC_THREAD_ID_T          rThreadID;
	VIDEO_ENC_WRAP_HANDLE_X64_T     hWrapper;
	//VAL_VOID_T                      *pDrvModule;    /* /< used for dlopen and dlclose */
	union {
		VAL_VOID_T	               *pDrvModule;
		VAL_UINT64_T 		        pDrvModule_dummy;
	};
	// TODO: for lastframe buf yxz
	union {
	   VAL_HANDLE_T 			   va_last_frame_buf;
	   VAL_UINT64_T 			   va_last_frame_buf_dummy;
	};
	VAL_UINT32_T  u4LogLevel;
	VAL_UINT32_T  u4ThreadNum;
	VAL_UINT32_T  u4NonCacheMem;
        VAL_UINT32_T                   u4PrependHeaderStoreSize;
        union {
		VAL_VOID_T	               *pHeaderbuf;
		VAL_UINT64_T 		        pHeaderbuf_dummy;
	};
} VENC_HYBRID_HANDLE_X64_T;

typedef struct __VENC_HANDLE_T {
	VENC_DRV_VIDEO_FORMAT_T CodecFormat;
	VENC_DRV_BASE_T         rFuncPtr;      /* /< Point to driver's proprietary function. */
	VAL_HANDLE_T            hDrvHandle;    /* /< Handle of each format driver */
	VAL_HANDLE_T            hHalHandle;    /* /< HAL handle */
	VAL_HANDLE_T            hValHandle;    /* /< VAL handle */
	VAL_BOOL_T              fgValInitFlag; /* /< hValHandle is available or not */
	VAL_MEMORY_T            rHandleMem;    /* /< Memory for venc handle */
	VAL_VOID_T              *prExtraData;  /* /< Driver private data pointer. */
	VAL_MEMORY_T            rExtraDataMem; /* /< Save extra data memory information to be used in release. */
	VENC_HYBRID_HANDLE_T    rHybridHandle; /* /< Hybrid handle */
	FILE *pfDump;
	VAL_UINT32_T            u4ShowInfo;    /* /< Flag for show FPS and BitRate */
	VAL_UINT32_T            u4FPS;         /* /< FPS */
	VAL_UINT32_T            u4Bitrate;     /* /< Bitrate */
	struct timeval          tStart;        /* /< Start time counting FPS and bitrate */
	VENC_DRV_SCENARIO_T     eScenario;     /* /< VENC Senario */
	VAL_INT32_T             nPerfServiceHandle; /* /< Used by performace service */
	VAL_UINT32_T            u4RecFrmWidth;  /* /< Recoded frame width, (may not 16 byte-align) */
	VAL_UINT32_T            u4RecFrmHeight; /* /< Recoded frame height, (may not 16 byte-align) */
	VAL_UINT32_T            u4Scenario;     /* /< For special scenario */
    // TODO: yxz for hybrid
	//VENC_HYBRID_HANDLE_X64_T    rHybridHandleX64; /* /< Hybrid64 will use dynamic allocated for memory share */
	union {
		VAL_HANDLE_T	    hHalHandleHybrid;
		VAL_UINT64_T 		hHalHandleHybrid_dummy;
	};
    CirQueue_s              *profiler;
    VAL_BOOL_T              bIsSlowMotion;
    VAL_BOOL_T              bIsVfpsDisable;
    VAL_BOOL_T              bIsVfpsMorelog;
} VENC_HANDLE_T;

VENC_DRV_MRESULT_T ParseConfig(const char *cfgFileName, const char *ParameterItem, VAL_UINT32_T *val);


#endif
