
OMX_BOOL mSetWFDMode;
OMX_BOOL mWFDMode;

// for WFD dummy NAL
OMX_BOOL mEnableDummy;

pthread_mutex_t mDummyFrmLock;
DefaultKeyedVector<int, OMX_BUFFERHEADERTYPE*> mDummyBufferTable = DefaultKeyedVector<int, OMX_BUFFERHEADERTYPE*>(0);

void MtkOmxVencWFD();
void deMtkOmxVencWFD();
OMX_ERRORTYPE WFD_EnableToggle();
OMX_ERRORTYPE WFD_DisableToggle();
OMX_ERRORTYPE WFD_SetMode();
OMX_ERRORTYPE WFD_SetConfig(OMX_INDEXTYPE nConfigIndex, OMX_PTR pCompConfig);
OMX_ERRORTYPE WFD_GetConfig(OMX_INDEXTYPE nConfigIndex, OMX_PTR pCompConfig);
OMX_ERRORTYPE WFD_GetParameter(OMX_INDEXTYPE nConfigIndex, OMX_PTR pCompParam);
OMX_ERRORTYPE WFD_SetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pCompParam);
OMX_ERRORTYPE WFD_BufferModifyBeforeEnc(OMX_BUFFERHEADERTYPE** ppInputBuf);
OMX_ERRORTYPE WFD_DrawYUVStripeLine(
    unsigned char** pY, unsigned char** pU, unsigned char** pV, OMX_U32 iDrawWidth, OMX_U32 iDrawHeight);

OMX_ERRORTYPE WFD_OnEnqueOutputBuffer(OMX_BUFFERHEADERTYPE** ppOutputBuf);
OMX_ERRORTYPE WFD_OnEnqueBuffer(OMX_BUFFERHEADERTYPE** ppInputBuf);
OMX_ERRORTYPE WFD_DequeOnGetOutputBuffer(OMX_BUFFERHEADERTYPE* pOutputBufHdr, void* dqbuf);
OMX_ERRORTYPE WFD_HandleDequeFrame(OMX_BUFFERHEADERTYPE* pOutputBufHdr);
OMX_ERRORTYPE GenerateDummyBuffer(OMX_BUFFERHEADERTYPE* pDummyOutputBufHdr);
OMX_ERRORTYPE HandleWFDSetVideoBitrate(OMX_VIDEO_PARAM_BITRATETYPE* pBitrateType);

