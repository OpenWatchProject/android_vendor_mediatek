#define MTK_AutoGridMode
#define MTK_MAX_SUPPORT_GRID_NUM    256 // 64M
#define MTK_GRID_INFO_SIZE  2048 // 2048/4 (uint: 4-byte)

OMX_VIDEO_PARAM_ANDROID_IMAGEGRIDTYPE mImageGrid;

OMX_U32 mOutputCnt;
OMX_U32 mInputCnt;
OMX_U32 mGridTotalCount;
OMX_U32 mGridIndex;
OMX_U32 mGridOffset;
OMX_BOOL mHeifOn;

OMX_U32 mGridInfo[(MTK_GRID_INFO_SIZE>>2)];

sem_t mHeifSem;

pthread_mutex_t mHeifOutputLock;
Vector<int> mHeifOutputBufList;

void MtkOmxVencHeif();
void deMtkOmxVencHeif();

void ComponentInitHeif();
void ComponentDeInitHeif();

// InitHW before stream on
void Heif_OnInit(video_encode_param* param);
OMX_ERRORTYPE Heif_SetMode();

// DeInitHW after stream off
void Heif_OnDeInit();

bool Heif_Enabled();

void Heif_EmptyThisBuffer(OMX_BUFFERHEADERTYPE* pInputBuf);
void Heif_FillThisBuffer(OMX_BUFFERHEADERTYPE* pOutputBuf);

// before generating v4l2_buffer from omx buffer (may idendical to Heif_UpdateInputV4L2Buffer)
void Heif_BufferModifyBeforeEnc(OMX_BUFFERHEADERTYPE** ppInputBuf);

void Heif_CollectBsBuffer(OMX_BUFFERHEADERTYPE** ppOutputBuf);

// before enque: after v4l2_buffer is generated
void Heif_UpdateInputV4L2Buffer(OMX_BUFFERHEADERTYPE *pInputBuf, struct v4l2_buffer* qbuf);
void Heif_UpdateOutputV4L2Buffer(OMX_BUFFERHEADERTYPE *pOutputBuf, struct v4l2_buffer* qbuf);

// after perform qbuf
void Heif_OnEnqueBuffer(OMX_BUFFERHEADERTYPE** ppInputBuf);
void Heif_OnEnqueOutputBuffer(OMX_BUFFERHEADERTYPE** ppOutputBuf);

// after acquire deque buffer, before calling Empty/FillBufferDone
void Heif_DequeOnGetInputBuffer(OMX_BUFFERHEADERTYPE* pInputBuf, struct v4l2_buffer* dqbuf);
void Heif_DequeOnGetOutputBuffer(OMX_BUFFERHEADERTYPE* pOutputBuf, struct v4l2_buffer* dqbuf);

OMX_ERRORTYPE Heif_SetConfig(OMX_INDEXTYPE nConfigIndex, OMX_PTR pCompConfig);
OMX_ERRORTYPE Heif_GetConfig(OMX_INDEXTYPE nConfigIndex, OMX_PTR pCompConfig);

OMX_ERRORTYPE Heif_GetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pCompParam);
OMX_ERRORTYPE Heif_SetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pCompParam);
OMX_ERRORTYPE Heif_GetExtensionIndex(OMX_STRING parameterName, OMX_INDEXTYPE *pIndexType);

OMX_ERRORTYPE HandleSetVideoImgeGrid(OMX_VIDEO_PARAM_ANDROID_IMAGEGRIDTYPE* pImageGrid);
OMX_ERRORTYPE HandleGetVideoImgeGrid(OMX_VIDEO_PARAM_ANDROID_IMAGEGRIDTYPE*);