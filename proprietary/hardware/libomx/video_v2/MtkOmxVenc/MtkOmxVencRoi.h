
#ifdef MTK_ROINET_SUPPORT
    Saliency *mNet;
#endif

int mRoiWidth;
int mRoiHeight;
int mRoiSize;
int mEnabledROI; /*0:disable, 1:platform solution 2:apk solution*/
int mEnableDumpROI;

MtkVenc::MtkRoiConfig<OMX_BUFFERHEADERTYPE*, MtkVenc::MtkRoiConfigEntry>* mConfig;
OmxMVAManager *mRoiMVAMgr;

pthread_mutex_t mRoiBufferTableLock;
pthread_mutex_t mFreeRoiBufferListLock;
DefaultKeyedVector<int, OMX_U8*> mRoiBufferTable = DefaultKeyedVector<int, OMX_U8*>(0);
Vector<OMX_U8*> mFreeRoiBufferList;

OMX_VIDEO_CONFIG_ROI_INFO mRoiInfo;
OMX_U8* mRoiLicense;
OMX_U32 mRoiLicenseSize;

void MtkOmxVencRoi();
void deMtkOmxVencRoi();
void ComponentInitRoi();
void ComponentDeInitRoi();

// InitHW before stream on
void ROI_OnInit(video_encode_param* param);

// DeInitHW after stream off
void ROI_OnDeInit();

void GetFreeRoiBuffer(OMX_U8** ppRoiBuffer, unsigned* pRoiBufferSize);
void SetFreeRoiBuffer(OMX_U8* pRoiBuffer);

// before enque
void ROI_UpdateInputV4L2Buffer(OMX_BUFFERHEADERTYPE *pInputBuf, struct v4l2_buffer* qbuf);
void ROI_UpdateOutputV4L2Buffer(OMX_BUFFERHEADERTYPE *pOutputBuf, struct v4l2_buffer* qbuf);

// after enque
void ROI_OnEnqueBuffer(OMX_BUFFERHEADERTYPE** ppInputBuf);
void ROI_OnEnqueOutputBuffer(OMX_BUFFERHEADERTYPE** pOutputBuf);

// after deque
void ROI_DequeOnGetInputBuffer(OMX_BUFFERHEADERTYPE* pInputBuf, void* dqbuf);
void ROI_DequeOnGetOutputBuffer(OMX_BUFFERHEADERTYPE* pOutputBuf, void* dqbuf);

void ROI_EmptyThisBuffer(OMX_BUFFERHEADERTYPE *pBuffHdr);

OMX_ERRORTYPE ROI_SetConfig(OMX_INDEXTYPE nConfigIndex, OMX_PTR pCompConfig);
OMX_ERRORTYPE ROI_GetConfig(OMX_INDEXTYPE nConfigIndex, OMX_PTR pCompConfig);
OMX_ERRORTYPE ROI_SetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pCompParam);
OMX_ERRORTYPE ROI_GetExtensionIndex(OMX_STRING parameterName, OMX_INDEXTYPE *pIndexType);
OMX_ERRORTYPE HandleSetParamVencorSetRoiOn(OMX_PARAM_U32TYPE* pCompParam);
OMX_ERRORTYPE HandleSetConfigVendorSetRoiSize(OMX_VIDEO_CONFIG_ROI_INFO* config);
OMX_ERRORTYPE HandleSetConfigVendorSetRoiInfo(char* config);
OMX_ERRORTYPE HandleSetParamVencorSetRoiLicenseSize(OMX_PARAM_U32TYPE* pCompParam);
OMX_ERRORTYPE HandleSetParamVencorSetRoiLicense(OMX_U8* pCompParam);