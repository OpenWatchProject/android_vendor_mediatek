#ifndef __MTK_VIDEO_SECURE_AL__
#define __MTK_VIDEO_SECURE_AL__
#include "val_types_public.h"

//#define BIN_SEC_SELF_TEST 1

typedef enum
{
    MTK_SECURE_AL_FAIL      = 0,
    MTK_SECURE_AL_SUCCESS   = 1,
} MTK_SECURE_AL_RESULT;

typedef enum
{
    MTK_VENC_h264      = 0,
    MTK_VDEC     = 1,
} MTK_SECURE_MEM_SESSION;

#if 0
typedef struct _VdecH264SecInitStruct {
    void* mem;
    int size;
} VdecH264SecInitStruct;
#else
typedef struct _VdecH264SecMemInfoStruct {
    void* mem;
    int size;
} VdecH264SecMemInfoStruct, VideoH264SecMemInfoStruct;

typedef struct _VdecVP9SecMemInfoStruct {
    void* mem;
    int size;
} VdecVP9SecMemInfoStruct, VideoVP9SecMemInfoStruct;
typedef struct _VdecH265SecMemInfoStruct {
    void* mem;
    int size;
} VdecH265SecMemInfoStruct, VideoH265SecMemInfoStruct;

typedef struct _VdecH264SecInitStruct {
    unsigned long vdec_h264_drv_data_share_handle;
    unsigned long vdec_h264_instanceTemp_share_handle;
    unsigned long h264_DEC_PRM_DataInst_share_handle;
    unsigned long bitstream_va_share_handle;   // UT only
    unsigned long bitstream_secure_handle;
    unsigned long bitstream_length;   // valid size of bitstream
} VdecH264SecInitStruct;
typedef struct _VdecVP9SecInitStruct {
    unsigned long vdec_vp9_drv_data_share_handle;
    unsigned long vdec_vp9_prcommon_share_handle;
    unsigned long bitstream_va_share_handle;   // UT only
    unsigned long bitstream_secure_handle;
    unsigned long bitstream_length;   // valid size of bitstream
} VdecVP9SecInitStruct;

// test only
typedef struct _VdecFrameDumpStruct {
    unsigned long frame_secure_handle;
    unsigned long frame_va_share_handle;
} VdecFrameDumpStruct;
#endif


/*
    void* mem1;    // pVdec_H264_InstanceInst
    int size1;          // sizeof(Vdec_H264_InstanceInst)

    void* mem2;   // pH264_DEC_PRM_DataInst
    int size2;         // sizeof(H264_DEC_PRM_DataInst)
*/
typedef struct _VdecH264SecDecStruct {
    unsigned long pVdec_H264_InstanceTemp_share_handle;   // share memory handle of pVdec_H264_InstanceTemp
    unsigned long pH264_DEC_PRM_DataInst_share_handle;    // share memory handle of pH264_DEC_PRM_DataInst

    unsigned long va1;         // pBitstream
    unsigned long handle1;  // handle of pBitstream
    unsigned long bitstream_length;   // valid size of bitstream

    unsigned long va2;         // pFrame
    unsigned long handle2;  // handle of pFrame

    unsigned long va3;    //  &pVdec_H264_InstanceInst->mCurrentBitstream

    unsigned long va4;         //  pVdec_H264_InstanceInst->pCurrBitstream
    unsigned long handle4;  // handle of pVdec_H264_InstanceInst->pCurrBitstream

    unsigned long handle5;  // ringbuffer VA share handle, for UT only
    unsigned long handle6;  // framebuffer VA share handle, for UT only
} VdecH264SecDecStruct;


typedef struct _VdecH264SecDeinitStruct {
    unsigned long pVdec_H264_InstanceTemp_share_handle;   // share memory handle of pVdec_H264_InstanceTemp
    unsigned long pH264_DEC_PRM_DataInst_share_handle;    // share memory handle of pH264_DEC_PRM_DataInst
    unsigned long pVdec_H264_Drv_dataInst_share_handle;    // share memory handle of pVdec_H264_Drv_dataInst
    unsigned long pDecStruct2_share_handle;                            // share memory handle of pDecStruct2
} VdecH264SecDeinitStruct;

typedef struct _VdecVP9SecDecStruct {
    unsigned long pVdec_VP9_Drv_data_share_handle;
    unsigned long va1;         // pBitstream
    unsigned long handle1;  // handle of pBitstream
    unsigned long bitstream_length;   // valid size of bitstream

    unsigned long va2;         // pFrame
    unsigned long handle2;  // handle of pFrame

    unsigned long handle3;  // ringbuffer VA share handle, for UT only
    unsigned long handle4;  // framebuffer VA share handle, for UT only
} VdecVP9SecDecStruct;
typedef struct _VdecVP9SecDeinitStruct {
    unsigned long pVdec_VP9_Drv_data_share_handle;
    unsigned long pVdec_prcommon_share_handle;    // share memory handle of pVdec_H264_Drv_dataInst
    unsigned long pDecStruct2_share_handle;                            // share memory handle of pDecStruct2
} VdecVP9SecDeinitStruct;

typedef struct _VdecH265SecInitStruct {
    unsigned long vdec_h265_drv_data_share_handle;
    unsigned long vdec_h265_instanceTemp_share_handle;
    unsigned long h265_DEC_PRM_DataInst_share_handle;
    unsigned long bitstream_va_share_handle;
    unsigned long bitstream_secure_handle;
    unsigned long bitstream_length;
} VdecH265SecInitStruct;

typedef struct _VdecH265SecDecStruct {
    unsigned long pVdec_H265_InstanceTemp_share_handle; // share memory handle of pVdec_H265_InstanceTemp
    unsigned long pH265_DEC_PRM_DataInst_share_handle;  // share memory handle of pH265_DEC_PRM_DataInst

    unsigned long va1;              // pBitstream
    unsigned long handle1;          // handle of pBitstream
    unsigned long bitstream_length; // valid size of bitstream

    unsigned long va2;      // pFrame
    unsigned long handle2;  // handle of pFrame

    unsigned long va3;      // &pVdec_H264_InstanceInst->mCurrentBitstream

    unsigned long va4;      //  pVdec_H265_InstanceInst->pCurrBitstream
    unsigned long handle4;  // handle of pVdec_H265_InstanceInst->pCurrBitstream

    unsigned long handle5;  // ringbuffer VA share handle, for UT only
    unsigned long handle6;  // framebuffer VA share handle, for UT only
} VdecH265SecDecStruct;

typedef struct _VdecH265SecDeinitStruct {
    unsigned long pVdec_H265_InstanceTemp_share_handle;   // share memory handle of pVdec_H262_InstanceTemp
    unsigned long pH265_DEC_PRM_DataInst_share_handle;    // share memory handle of pH262_DEC_PRM_DataInst
    unsigned long pVdec_H265_Drv_dataInst_share_handle;   // share memory handle of pVdec_H262_Drv_dataInst
    unsigned long pDecStruct2_share_handle;               // share memory handle of pDecStruct2
} VdecH265SecDeinitStruct;

// secure memory allocation related apis
MTK_SECURE_AL_RESULT MtkVideoSecureMemAllocatorInit(int codecIndex);
MTK_SECURE_AL_RESULT MtkVideoSecureMemAllocatorDeinit(int codecIndex);
unsigned long MtkVideoAllocateSecureBuffer(int size, int align, const char* tag,int codecindex);
MTK_SECURE_AL_RESULT MtkVideoFreeSecureBuffer(unsigned long memHandle,int codecindex);
unsigned long MtkVideoAllocateSecureFrameBuffer(int size, int align, const char* tag,int codecindex);
MTK_SECURE_AL_RESULT MtkVideoFreeSecureFrameBuffer(unsigned long memHandle,int codecindex);
unsigned long MtkVideoRegisterSharedMemory(void* buffer, int size, const char *tag, int codecindex);
MTK_SECURE_AL_RESULT MtkVideoUnregisterSharedMemory(unsigned long sharedHandle,int codecIndex);


#define MTK_SEC_VCODEC_IN_HOUSE_LIB_NAME "lib_uree_mtk_video_secure_al.so"
#define MTK_SEC_VCODEC_IN_HOUSE_INIT_NAME "MtkVideoSecureMemAllocatorInit"
#define MTK_SEC_VCODEC_IN_HOUSE_DEINIT_NAME "MtkVideoSecureMemAllocatorDeInit"
#define MTK_SEC_VCODEC_IN_HOUSE_ALLOCATE_SECURE_FRAME_BUFFER "MtkVideoAllocateSecureFrameBuffer"
#define MTK_SEC_VCODEC_IN_HOUSE_REGISTER_SHARED_MEMORY "MtkVideoRegisterSharedMemory"
#define MTK_SEC_VCODEC_IN_HOUSE_UNREGISTER_SHARED_MEMORY "MtkVideoUnregisterSharedMemory"
#define MTK_SEC_VCODEC_IN_HOUSE_ALLOCATE_SECURE_BUFFER "MtkVideoAllocateSecureBuffer"
#define MTK_SEC_VCODEC_IN_HOUSE_FREE_SECURE_BUFFER "MtkVideoFreeSecureBuffer"


#define MTK_H264_SEC_VDEC_IN_HOUSE_INIT "MtkVdecH264SecInit"
#define MTK_H264_SEC_VDEC_IN_HOUSE_DECODE "MtkVdecH264SecDecode"
#define MTK_H264_SEC_VDEC_IN_HOUSE_DEINIT "MtkVdecH264SecDeinit"

#define MTK_H265_SEC_VDEC_IN_HOUSE_INIT "MtkVdecH265SecInit"
#define MTK_H265_SEC_VDEC_IN_HOUSE_DECODE "MtkVdecH265SecDecode"
#define MTK_H265_SEC_VDEC_IN_HOUSE_DEINIT "MtkVdecH265SecDeinit"

#define MTK_VP9_SEC_VDEC_IN_HOUSE_INIT "MtkVdecVP9SecInit"
#define MTK_VP9_SEC_VDEC_IN_HOUSE_DECODE "MtkVdecVP9SecDecode"
#define MTK_VP9_SEC_VDEC_IN_HOUSE_DEINIT "MtkVdecVP9SecDeinit"


//VDEC MTK In-house function pointer types
typedef MTK_SECURE_AL_RESULT MtkVideoSecureMemAllocatorInit_Ptr(int);
typedef unsigned long MtkVideoAllocateSecureFrameBuffer_Ptr(int , int );
typedef MTK_SECURE_AL_RESULT MtkVideoFreeSecureFrameBuffer_Ptr(unsigned long );
typedef unsigned long MtkVideoVP9RegisterSharedMemory_Ptr(void *, int );
typedef MTK_SECURE_AL_RESULT MtkVideoVP9UnregisterSharedMemory_Ptr(unsigned long );


typedef unsigned long MtkVideoAllocateSecureBuffer_Ptr(int , int);
typedef MTK_SECURE_AL_RESULT MtkVideoFreeSecureBuffer_Ptr(unsigned long );

typedef MTK_SECURE_AL_RESULT MtkVdecH264SecInit_Ptr(VdecH264SecMemInfoStruct *, unsigned long , unsigned long *, unsigned long *, unsigned long *, unsigned long *, unsigned int *);
typedef MTK_SECURE_AL_RESULT MtkVdecH264SecDecode_Ptr(unsigned int, VdecH264SecMemInfoStruct *, unsigned long);
typedef MTK_SECURE_AL_RESULT MtkVdecH264SecDeinit_Ptr(unsigned int, VdecH264SecDeinitStruct *);

typedef MTK_SECURE_AL_RESULT MtkVdecH265SecInit_Ptr(VdecH265SecMemInfoStruct *, unsigned long , unsigned long *, unsigned long *, unsigned long *, unsigned long *, unsigned int *);
typedef MTK_SECURE_AL_RESULT MtkVdecH265SecDecode_Ptr(unsigned int, VdecH265SecMemInfoStruct *, unsigned long);
typedef MTK_SECURE_AL_RESULT MtkVdecH265SecDeinit_Ptr(unsigned int, VdecH265SecDeinitStruct *);

typedef MTK_SECURE_AL_RESULT MtkVdecVP9SecInit_Ptr(VdecVP9SecMemInfoStruct *, unsigned long , unsigned long *, unsigned long *, unsigned long *,unsigned int *);
typedef MTK_SECURE_AL_RESULT MtkVdecVP9SecDecode_Ptr(unsigned int, VdecVP9SecMemInfoStruct *, unsigned long);
typedef MTK_SECURE_AL_RESULT MtkVdecVP9SecDeinit_Ptr(unsigned int, VdecVP9SecDeinitStruct *);

typedef struct _MtkH264Venc_Inhouse
{
     void *gH264SecVencMtkLib;
     MTK_SECURE_AL_RESULT (*pfnVencH264SecInit_Ptr)(VideoH264SecMemInfoStruct*, VAL_UINT32_T *);
     MTK_SECURE_AL_RESULT (*pfnVencH264SecDeinit_Ptr)(VAL_UINT32_T);
     MTK_SECURE_AL_RESULT (*pfnVencH264SecEncode_Ptr)(VAL_UINT32_T , VideoH264SecMemInfoStruct* , VideoH264SecMemInfoStruct* ,\
                                                          unsigned long , unsigned char );
     MTK_SECURE_AL_RESULT (*pfnVencH264SecAllocWorkBuf_Ptr)(VAL_UINT32_T );
     MTK_SECURE_AL_RESULT (*pfnVencH264SecFreeWorkBuf_Ptr)(VAL_UINT32_T );
     MTK_SECURE_AL_RESULT (*pfnVencH264SecShareWorkbuf_Ptr)(VAL_UINT32_T , VideoH264SecMemInfoStruct* , unsigned long* );
     MTK_SECURE_AL_RESULT (*pfnVencH264SecUnshareWorkBuf_Ptr)(VAL_UINT32_T , VAL_UINT32_T* );
     MTK_SECURE_AL_RESULT (*pfnVencH264SecCopyWorkBuf_Ptr)(VAL_UINT32_T );
} MtkH264Venc_Inhouse;

/* Begin added by mtk09845 UT_TEST_DEV  */
typedef struct _VdecFillSecMemStruct
{
    uint32_t bs_share_handle;
    uint32_t bs_secure_handle;
    uint32_t bs_size;
    uint32_t direction; //0: to mtee, 1: from mtee
} VdecFillSecMemStruct;

unsigned long MtkVideoAllocateSecureBitstreamBuffer(int size, int align, const char *tag,int codecindex);
unsigned long MtkVdecFillSecureBuffer(uint32_t bs_sec_handle, void *bs_va, uint32_t size, uint32_t bdirection);
/* End added by mtk09845 UT_TEST_DEV  */

// video decoding related apis
MTK_SECURE_AL_RESULT MtkVdecH264SecInit(VdecH264SecMemInfoStruct* pInitStruct, unsigned long bitstreamMemHandle, unsigned long* Vdec_H264_InstanceTemp_share_handle, unsigned long* H264_DEC_PRM_DataInst_share_handle, unsigned long* H264_Drv_data_share_handle, unsigned long* DecStruct2_share_handle, unsigned int* pH264_Sec_session);
MTK_SECURE_AL_RESULT MtkVdecH264SecDeinit(unsigned int H264_Sec_session, VdecH264SecDeinitStruct* pDeinitStruct);

MTK_SECURE_AL_RESULT MtkVdecH264SecDecode(unsigned int H264_Sec_session, VdecH264SecMemInfoStruct* pDecStruct1, unsigned long pDecStruct2_share_handle);
void MTKVdecDataPaddingAndValueSizeTest();
MTK_SECURE_AL_RESULT MtkVdecVP9SecInit(VdecVP9SecMemInfoStruct* pInitStruct, unsigned long bitstreamMemHandle,unsigned long* VP9_Drv_data_share_handle, unsigned long* DecStruct2_share_handle, unsigned long* prCommon_share_handle, unsigned int* pVP9_Sec_session);
MTK_SECURE_AL_RESULT MtkVdecVP9SecDeinit(unsigned int VP9_Sec_session,VdecVP9SecDeinitStruct* pDeinitStruct);
MTK_SECURE_AL_RESULT MtkVdecVP9SecDecode(unsigned int VP9_Sec_session,VdecVP9SecMemInfoStruct* pDecStruct1, unsigned long pDecStruct2_share_handle);

MTK_SECURE_AL_RESULT MtkVdecH265SecInit(VdecH265SecMemInfoStruct* pInitStruct, unsigned int bitstreamMemHandle, unsigned int* Vdec_H264_InstanceTemp_share_handle, unsigned int* H264_DEC_PRM_DataInst_share_handle, unsigned int* H264_Drv_data_share_handle, unsigned int* DecStruct2_share_handle, unsigned int* pH265_Sec_session);

MTK_SECURE_AL_RESULT MtkVdecH265SecDeinit(unsigned int H265_Sec_session, VdecH265SecDeinitStruct* pDeinitStruct);

MTK_SECURE_AL_RESULT MtkVdecH265SecDecode(unsigned int H265_Sec_session, VdecH265SecMemInfoStruct* pDecStruct1, unsigned long pDecStruct2_share_handle);


//#if defined(MTK_IN_HOUSE_TEE_SUPPORT) && defined(MTK_SEC_VIDEO_PATH_SUPPORT)

//Sync with TA venc\h264\encode.h
typedef struct _VencH264SecInitStruct {
    VAL_UINT32_T venc_h264_drv_data_share_handle;
    VAL_UINT32_T bitstream_va_share_handle;   // UT only
    VAL_UINT32_T bitstream_secure_handle;
    VAL_UINT32_T bitstream_length;   // valid size of bitstream
    VAL_UINT32_T frame_va_share_handle;   // UT only
    VAL_UINT32_T frame_secure_handle;
    // TODO:for hybrid
    VAL_UINT32_T hybrid_handle;
    VAL_UINT32_T val_hybrid_handle;
    //VAL_UINT32_T hybrid_setting_handle;
    //VAL_UINT32_T val_hybrid_setting_handle;
} VencH264SecInitStruct;

typedef struct _VencH264SecShareWorbufStruct {
    VAL_UINT32_T venc_h264_drv_data_share_handle;
    VAL_UINT32_T venc_H264_RCInfo_share_handle;
    VAL_UINT32_T venc_H264_RCCode_share_handle;
    VAL_UINT32_T venc_H264_RecLuma_share_handle;
    VAL_UINT32_T venc_H264_RecChroma_share_handle;
    VAL_UINT32_T venc_H264_RefLuma_share_handle;
    VAL_UINT32_T venc_H264_RefChroma_share_handle;
    VAL_UINT32_T venc_H264_MVInfo1_handle;
    VAL_UINT32_T venc_H264_MVInfo2_handle;
    VAL_UINT32_T venc_H264_PPSTemp_share_handle;
    VAL_UINT32_T venc_H264_IDRTemp_share_handle;
} VencH264SecShareWorbufStruct;

typedef struct _VencH264SecCmdStruct {
    VAL_UINT32_T venc_h264_drv_data_share_handle;
} VencH264SecCmdStruct;

typedef struct _VencH264SecEncStruct {
    VAL_UINT32_T venc_h264_drv_data_share_handle;
    VAL_UINT32_T encode_option;
    VAL_UINT32_T va1;         // pFrame
    VAL_UINT32_T handle1;  // handle of pFrame
    VAL_UINT32_T va2;         // pBitstream
    VAL_UINT32_T handle2;  // handle of pBitstream
    VAL_UINT32_T handle3;  // handle of presult
    // TODO:hybrid last framebuf
    VAL_UINT32_T va1LastFrame;
} VencH264SecEncStruct;

MTK_SECURE_AL_RESULT MtkVencH264SecInit(VideoH264SecMemInfoStruct* pInitStruct, VAL_UINT32_T * H264_Drv_data_share_handle);
MTK_SECURE_AL_RESULT MtkVencH264SecDeinit(VAL_UINT32_T H264_Drv_data_share_handle);
MTK_SECURE_AL_RESULT MtkVencH264SecEncode(VAL_UINT32_T H264_Drv_data_share_handle, VideoH264SecMemInfoStruct* pEncStruct, VideoH264SecMemInfoStruct* pParStruct, unsigned long EncodeOption, unsigned char fgSecBuffer);
MTK_SECURE_AL_RESULT MtkVencH264SecAllocWorkBuf(VAL_UINT32_T H264_Drv_data_share_handle);
MTK_SECURE_AL_RESULT MtkVencH264SecFreeWorkBuf(VAL_UINT32_T H264_Drv_data_share_handle);
MTK_SECURE_AL_RESULT MtkVencH264SecShareWorkBuf(VAL_UINT32_T H264_Drv_data_share_handle, VideoH264SecMemInfoStruct* pCmdStruct, unsigned long* pShareHandle);
MTK_SECURE_AL_RESULT MtkVencH264SecUnshareWorkBuf(VAL_UINT32_T H264_Drv_data_share_handle, VAL_UINT32_T* pUnsharBufHandle);
MTK_SECURE_AL_RESULT MtkVencH264SecCopyWorkBuf(VAL_UINT32_T H264_Drv_data_share_handle);

// TODO:yxz for hybrid
MTK_SECURE_AL_RESULT MtkVencH264HybridSecInit(VideoH264SecMemInfoStruct* pInitStruct, VAL_UINT32_T * H264_Drv_data_share_handle);
MTK_SECURE_AL_RESULT MtkVencH264HybridSecDeinit(VAL_UINT32_T H264_Drv_data_share_handle, VAL_UINT32_T Hybrid_hal_share_handle, VAL_UINT32_T Hybrid_last_frame_tz_handle);
MTK_SECURE_AL_RESULT MtkVencH264HybridSecEncode(VAL_UINT32_T H264_Drv_data_share_handle, VideoH264SecMemInfoStruct* pEncStruct, VideoH264SecMemInfoStruct* pParStruct, unsigned long EncodeOption, unsigned char fgSecBuffer);

#endif /* __MTK_VDEC_SECURE_AL__ */
