

typedef struct _MtkVencIonBufferInfo
{
    ion_user_handle_t ion_handle;
    int ion_share_fd;
    void* va;
    int secure_handle;
    int value[4];
    ion_user_handle_t ion_handle_4_enc;
    int ion_share_fd_4_enc;
    void* va_4_enc;
    int secure_handle_4_enc;
    int value_4_enc[4];
} MtkVencIonBufferInfo;

// for SECURE VIDEO PATH && TRUSTONIC TEE SUPPORT [
typedef struct _MtkH264Venc_TLC
{
    void *tlcLib;

    void *(*tlcHandleCreate)();
    void (*tlcHandleRelease)(void *);

    int (*tlcDumpSecMem)(void *, uint32_t, void *, uint32_t);
    int (*tlcFillSecMem)(void *, void *, uint32_t, uint32_t);
} MtkH264Venc_TLC;
// ]

OMX_BUFFERHEADERTYPE* pLastFrameBuffer;
OMX_BUFFERHEADERTYPE* pEosFrameBuffer;

void MtkOmxVencBuffer();
void deMtkOmxVencBuffer();

void onEmptyThisBuffer(OMX_BUFFERHEADERTYPE *pBuffHdr);
void onFillThisBuffer(OMX_BUFFERHEADERTYPE *pBuffHdr);

void waitForFillBuffer(unsigned long timeoutMs=0);
pthread_cond_t mWaitFTBCond;
pthread_mutex_t mWaitFTBCondLock;

void Buffer_OnEnqueBuffer(OMX_BUFFERHEADERTYPE** ppInputBuf);
void Buffer_DequeOnGetOutputBuffer(OMX_BUFFERHEADERTYPE* pOutputBuf, void* dqbuf);
OMX_BOOL shouldNotifyFlushDoneBuffer(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf);
OMX_BOOL notifyFlushDoneBuffer();
OMX_BOOL onStartFlushBuffer(OMX_BUFFERHEADERTYPE* pInputBuf);
OMX_BOOL onStartDequeBuffer(OMX_BUFFERHEADERTYPE* pInputBuf);

OMX_ERRORTYPE CheckInputBufferPortAvailbility();
OMX_ERRORTYPE CheckOutputBufferPortAvailbility();

void HandleInputPortPopulated();
void HandleOutputPortPopulated();

OMX_ERRORTYPE InputBufferHeaderAllocate(OMX_BUFFERHEADERTYPE**, OMX_U32, OMX_PTR, OMX_U32);
OMX_ERRORTYPE EpilogueInputBufferHeaderAllocate();
OMX_ERRORTYPE InputBufferHeadersResize(int count);

OMX_ERRORTYPE OutputBufferHeaderAllocate(OMX_BUFFERHEADERTYPE**, OMX_U32, OMX_PTR, OMX_U32);
OMX_ERRORTYPE EpilogueOutputBufferHeaderAllocate();
OMX_ERRORTYPE OutputBufferHeadersResize(int count);

OMX_ERRORTYPE InputBufferHeaderUse(OMX_BUFFERHEADERTYPE**, OMX_U32, OMX_PTR, OMX_U32, OMX_U8*);
OMX_ERRORTYPE EpilogueInputBufferHeaderUse();

OMX_ERRORTYPE OutputBufferHeaderUse(OMX_BUFFERHEADERTYPE**, OMX_U32, OMX_PTR, OMX_U32, OMX_U8*);
OMX_ERRORTYPE EpilogueOutputBufferHeaderUse();

OMX_BOOL AllowToFreeBuffer(OMX_U32, OMX_STATETYPE);
bool bufferReadyState(OMX_STATETYPE);

OMX_ERRORTYPE UnmapInputMemory(OMX_BUFFERHEADERTYPE*);
OMX_ERRORTYPE UnmapOutputMemory(OMX_BUFFERHEADERTYPE*);
OMX_ERRORTYPE releaseOutputNativeHandle(OMX_BUFFERHEADERTYPE*);

OMX_ERRORTYPE FreeInputBuffers(OMX_BUFFERHEADERTYPE*);
OMX_ERRORTYPE FreeOutputBuffers(OMX_BUFFERHEADERTYPE*);

OMX_BOOL GetMetaHandleFromOmxHeader(OMX_BUFFERHEADERTYPE *pBufHdr, OMX_U32 *pBufferHandle);
OMX_BOOL GetMetaHandleFromBufferPtr(OMX_U8 *pBuffer, OMX_U32 *pBufferHandle);

Vector<OMX_U8*> mFreeConvertBufferList;
pthread_mutex_t mFreeConvertBufferListLock;

inline bool doColorConvertSequential()
{
    return (OMX_TRUE == NeedConversion());
}
inline void InitConvertFreeBufferList(int maxCount)
{
    InitConvertFreeBufferList(maxCount, 3);
}
void InitConvertFreeBufferList(int maxCount, int initCount);
void DeinitConvertFreeBufferList();
void GetFreeConvertBuffer(OMX_U8** ppCnvtBuffer, unsigned* pCnvtBufferSize);
void SetFreeConvertBuffer(OMX_U8* pCnvtBuffer);

inline OMX_BUFFERHEADERTYPE** EncodeT_Input_GetBufferHeaders()
{
    return mInputBufferHdrs;
}

inline OMX_BUFFERHEADERTYPE** EncodeT_Output_GetBufferHeaders()
{
    return mOutputBufferHdrs;
}

OMX_BUFFERHEADERTYPE* EncodeT_Input_GetBufferHeaderPtr(int index);
OMX_BUFFERHEADERTYPE* EncodeT_Output_GetBufferHeaderPtr(int index);
inline int EncodeT_Input_GetBufferCountActual()
{
    return mInputBufferPopulatedCnt;
}

inline int EncodeT_Output_GetBufferCountActual()
{
    return mOutputBufferPopulatedCnt;
}
int EncodeT_Input_GetBufferIndex(OMX_BUFFERHEADERTYPE *pInputBuf);
int EncodeT_Output_GetBufferIndex(OMX_BUFFERHEADERTYPE *pOutputBuf);
int EncodeT_GetBufferIndex(OMX_BUFFERHEADERTYPE *pBuffer, OMX_BUFFERHEADERTYPE** ppBufferHeaders, int size);

inline void InitConvertBuffer()
{
    InitConvertBuffer(&mCnvtBuffer, &mCnvtBufferSize, 1);
}

inline void InitConvertBuffer(OMX_U8 **ppCnvtBuffer, unsigned *pCnvtBufferSize)
{
    InitConvertBuffer(ppCnvtBuffer, pCnvtBufferSize, EncodeT_Input_GetBufferCountActual());
}
void InitConvertBuffer(OMX_U8 **ppCnvtBuffer, unsigned *pCnvtBufferSize, int maxCount);

int InitConvertBufferSize(unsigned *pCnvtBufferSize);

inline void DeinitConvertBuffer()
{
    DeinitConvertBuffer(&mCnvtBuffer);
}
void DeinitConvertBuffer(OMX_U8** ppCnvtBuffer);

OMX_U32 getHWLimitSize(OMX_U32 bufferSize);

int findBufferHeaderIndex(OMX_U32 nPortIndex, OMX_BUFFERHEADERTYPE *pBuffHdr);

OMX_BOOL        CheckBufferAvailabilityAdvance(MtkOmxBufQ *pvInputBufQ, MtkOmxBufQ *pvOutputBufQ);
int             DequeueBufferAdvance(MtkOmxBufQ *pvBufQ);
void            QueueBufferAdvance(MtkOmxBufQ *pvBufQ, OMX_BUFFERHEADERTYPE *pBuffHdr);


inline OMX_BOOL GetVEncDrvBSBuffer(OMX_U8 *aOutputBuf, OMX_U32 aOutputSize)
{
    return GetVEncDrvBSBuffer(aOutputBuf, aOutputSize, &mBitStreamBuf);
}
OMX_BOOL GetVEncDrvBSBuffer(OMX_U8 *aOutputBuf, OMX_U32 aOutputSize, VENC_DRV_PARAM_BS_BUF_T* pBitStreamBuf);
OMX_BOOL ClearVEncDrvBSBuffer(OMX_U8 *aOutputBuf);

inline OMX_BOOL GetVEncDrvFrmBuffer(OMX_U8 *aInputBuf, OMX_U32 aInputSize)
{
    OMX_U8* pCvtBuf = 0;
    return GetVEncDrvFrmBuffer(aInputBuf, aInputSize, pCvtBuf, 0, &mFrameBuf);
}
inline OMX_BOOL GetVEncDrvFrmBuffer(OMX_U8 *aInputBuf, OMX_U32 aInputSize, VENC_DRV_PARAM_FRM_BUF_T* pFrameBuf)
{
    OMX_U8* pCvtBuf = 0;
    return GetVEncDrvFrmBuffer(aInputBuf, aInputSize, pCvtBuf, 0, pFrameBuf);
}
OMX_BOOL GetVEncDrvFrmBuffer(OMX_U8 *aInputBuf, OMX_U32 aInputSize, OMX_U8* aCvtBuf, OMX_U32 aCvtBufSize, VENC_DRV_PARAM_FRM_BUF_T* pFrameBuf);

OMX_BOOL ClearVEncDrvFrmBuffer(OMX_U8 *aInputBuf, OMX_U8* aCvtBuf);

OMX_BOOL SetVEncDrvFrmBufferFlag(OMX_BUFFERHEADERTYPE *pInputBuf);

#ifdef SUPPORT_NATIVE_HANDLE
        OMX_BOOL mIsAllocateOutputNativeBuffers;
        int mIonDevFd;
        Vector< native_handle* >  mStoreMetaOutNativeHandle;
        Vector< MtkVencIonBufferInfo > mIonBufferInfo;
        int mRecordBitstream;
        int mWFDLoopbackMode;

        OMX_BOOL mIsChangeBWC4WFD;
        void SetBitstreamSize4Framework(
            OMX_BUFFERHEADERTYPE *pOutputBuf,
            OMX_U8 *aOutputBuf,
            OMX_U32 bistreamSize);
        OMX_BOOL SetDbgInfo2Ion(
            ion_user_handle_t& ion_handle,
            int& ion_share_fd,
            void* va,
            int secure_handle,
            int value1,
            int value2,
            int value3,
            int value4,
            ion_user_handle_t& ion_handle_4_enc,
            int& ion_share_fd_4_enc,
            void* va_4_enc,
            int secure_handle_4_enc,
            int value5,
            int value6,
            int value7,
            int value8
            );
        void DumpCorruptedMem(char* name, char* startAddr, int size);
        void checkMemory(char* startAddr, int bufferSize, char* name);
        void DumpBitstream(char* name, char* startAddr, int size);
#endif

/* Only for tz infor debug */
#ifdef MTK_DUM_SEC_ENC
    unsigned char* pTmp_buf;
    int Tmp_bufsz;
     /* 0 -> to, 1 -> from*/
    VAL_UINT32_T Dump_buf_sec_and_normal(OMX_U32 hSec_buf, OMX_U8* pTemp_Buf,OMX_U32 ui4_sz, unsigned  dir);
#endif

OMX_BOOL isBufferSec(OMX_U8 *aInputBuf, OMX_U32 aInputSize, int *aBufferType);

OMX_BOOL InitSecCnvtBuffer(int num);
inline OMX_BOOL DeInitSecCnvtBuffer(void)
{
    mSecConvertBufs.clear();
    return OMX_TRUE;
}

OMX_U32 mQHandle;
ge_smvr_info_t mQVrInfo;
OMX_TICKS mPreviousFrameTimeStamp;
OMX_TICKS mPreviousGeTimeStamp;
void BufferModifyBeforeEnc(OMX_BUFFERHEADERTYPE** ppInputBuf);

int checkSecSwitchInEnc(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf);
int checkSecSwitchInCC(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf);
bool dumpSecBuffer(char *name, int hSecHandle, int size);
bool dumpOutputBuffer(OMX_BUFFERHEADERTYPE *pOutputBuf, OMX_U8 *aOutputBuf, OMX_U32 aOutputSize);
bool dumpInputBuffer(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_U8 *aInputBuf, OMX_U32 aInputSize);
bool dumpBuffer(char *name, unsigned char *data, int size);
bool dumpYUVBuffer(char *name, unsigned char *y, unsigned char *u, unsigned char *v,
                   int width, int height);
// for SVP
OMX_BOOL mIsSecureSrc;  // when it's true, means this obj runs secure path at the moment.
OMX_BOOL mIsSecureInst; // when it's true, means this obj supports secure path.
Vector< sp<GraphicBuffer> > mSecConvertBufs;
void normalSecureSwitchHndling(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_BUFFERHEADERTYPE *pOutputBuf);
//for SEC VIDEO PATH, no matter TRUSTONIC TEE SUPPORT or in house [
sp<GraphicBuffer> mTestSecInput;
int mTestSecInputHandle;
// ]
// for SEC VIDEO PATH && TRUSTONIC TEE SUPPORT
MtkH264Venc_TLC tlc;
bool            bHasSecTlc;
// ]
OMX_BOOL mCnvtPortReconfigInProgress;
//outptu meta mode
OMX_BOOL        mStoreMetaDataInOutBuffers;
Vector< sp<GraphicBuffer> >  mStoreMetaOutHandle;
VAL_UINT32_T mReconfigCount;
OMX_BOOL InitSecEncParams(void);
OMX_BOOL DeInitSecEncParams(void);

inline int checkSecCnvtBufferNum(void)
{
    return (int)mSecConvertBufs.size();
}
enum teeEnvType
{
    NONE_TEE = 0,
    TRUSTONIC_TEE = 1,
    INHOUSE_TEE = 2,
    MICROTRUST_TEE = 3,
};
int mTeeEncType;