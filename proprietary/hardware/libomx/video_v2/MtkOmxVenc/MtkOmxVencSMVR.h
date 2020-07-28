
OMX_U32 mSubFrameTotalCount;
OMX_U32 mSubFrameIndex;
OMX_TICKS mSubFrameTimeStamp;
OMX_TICKS mSubFrameLastTimeStamp;
OMX_TICKS mLastFrameTimeStamp;
OMX_TICKS mBurstTicksPerFrame;
OMX_BOOL mSMVROn;
OMX_U32 mOperationRate;
OMX_U32 mCollectRate;
OMX_U32 mSubFrameRate;
Vector<int> mCollectedBs;
OMX_BOOL mFlushAll;
OMX_BOOL mSMVRDumpInputFrame;

void MtkOmxVencSMVR();
void deMtkOmxVencSMVR();

OMX_BOOL SMVR_OnInit(video_encode_param* param);
OMX_ERRORTYPE SMVR_SetMode();
OMX_ERRORTYPE SMVR_SetConfigOperatingRate(OMX_PARAM_U32TYPE* pOperationRate);

void SMVR_UpdateInputV4L2Buffer(OMX_BUFFERHEADERTYPE *pInputBuf, void* smvr, struct v4l2_buffer* qbuf);
void SMVR_UpdateInputV4L2Buffer(OMX_BUFFERHEADERTYPE *pInputBuf, buffer_handle_t handle, struct v4l2_buffer* qbuf);
inline void SMVR_UpdateInputV4L2Buffer(OMX_BUFFERHEADERTYPE *pInputBuf, struct v4l2_buffer* qbuf)
{
    if(mSMVROn == OMX_FALSE) return;
    if(pInputBuf == NULL) return;
    if(pInputBuf->nFilledLen == 0) return;

    OMX_U8 *aInputBuf = pInputBuf->pBuffer + pInputBuf->nOffset;

    OMX_U32 handle = 0;
    GetMetaHandleFromBufferPtr(aInputBuf, &handle);

    return SMVR_UpdateInputV4L2Buffer(pInputBuf, (buffer_handle_t)handle, qbuf);
}

void SMVR_UpdateOutputV4L2Buffer(OMX_BUFFERHEADERTYPE *pOutputBuf, struct v4l2_buffer* qbuf);

void SMVR_BufferModifyBeforeEnc(OMX_BUFFERHEADERTYPE** ppInputBuf, void* smvr);
void SMVR_BufferModifyBeforeEnc(OMX_BUFFERHEADERTYPE** ppInputBuf, buffer_handle_t handle);
inline void SMVR_BufferModifyBeforeEnc(OMX_BUFFERHEADERTYPE** ppInputBuf)
{
    if(mSMVROn == OMX_FALSE) return;
    if (!ppInputBuf || !*ppInputBuf) return;
    OMX_BUFFERHEADERTYPE* pInputBuf = *ppInputBuf;

    if(pInputBuf == NULL) return;
    if(pInputBuf->nFilledLen == 0) return;

    OMX_U8 *aInputBuf = pInputBuf->pBuffer + pInputBuf->nOffset;

    OMX_U32 handle = 0;
    GetMetaHandleFromBufferPtr(aInputBuf, &handle);

    return SMVR_BufferModifyBeforeEnc(ppInputBuf, (buffer_handle_t)handle);
}


void SMVR_CollectBsBuffer(OMX_BUFFERHEADERTYPE** ppOutputBuf);

void SMVR_OnEnqueBuffer(OMX_BUFFERHEADERTYPE **ppInputBuf, void* smvr);
void SMVR_OnEnqueBuffer(OMX_BUFFERHEADERTYPE** ppInputBuf, buffer_handle_t handle);
inline void SMVR_OnEnqueBuffer(OMX_BUFFERHEADERTYPE** ppInputBuf)
{
    if(mSMVROn == OMX_FALSE) return;

    OMX_BUFFERHEADERTYPE* pInputBuf = *ppInputBuf;
    OMX_U8 *aInputBuf = pInputBuf->pBuffer + pInputBuf->nOffset;

    OMX_U32 handle = 0;

    if(pInputBuf->nFilledLen == 0) return;

    GetMetaHandleFromBufferPtr(aInputBuf, &handle);

    return SMVR_OnEnqueBuffer(ppInputBuf, (buffer_handle_t)handle);
}

void SMVR_OnEnqueOutputBuffer(OMX_BUFFERHEADERTYPE** ppInputBuf);

void SMVR_DequeOnGetOutputBuffer(OMX_BUFFERHEADERTYPE* pOutputBuf, void* dqbuf);

void SMVR_DumpInputBuffer(OMX_BUFFERHEADERTYPE *pInputBuf, OMX_U8 *aInputBuf, OMX_U32 aInputSize);
