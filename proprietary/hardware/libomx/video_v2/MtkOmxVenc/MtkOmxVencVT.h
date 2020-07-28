
typedef struct {
    int width;
    int height;
    int framerate;
    int resolutionchange;
} video_resolutionchange;

video_encode_param* pParam;

bool mIsViLTE;
OMX_BOOL mPrepareToResolutionChange;
bool bDynamicFps;

OMX_CONFIG_SLICE_LOSS_INDICATION mSLI;
OMX_BOOL mGotSLI;

#define MTK_VENC_TIMESTAMP_WINDOW_SIZE 4
OMX_TICKS mLastQueuedFrameTimeStamp;  // it's for MPEG4 SHIFT (hybrid codec lib)
Vector<VAL_UINT32_T> mInputTimeStampDiffs;

void MtkOmxVencVT();
void ComponentInitVT();
OMX_ERRORTYPE VT_ResolutionChange(OMX_BUFFERHEADERTYPE *pInputBuf);
OMX_ERRORTYPE VT_DumpOutputBuffer(OMX_U8 *aOutputBuf, OMX_U32 aOutputSize);
OMX_ERRORTYPE VT_BufferModifyBeforeEnc(OMX_BUFFERHEADERTYPE** ppInputBuf);
OMX_ERRORTYPE VT_OnEnqueBuffer(OMX_BUFFERHEADERTYPE** ppInputBuf);
OMX_ERRORTYPE VT_SetMode();
OMX_ERRORTYPE VT_SetConfig(OMX_INDEXTYPE nConfigIndex, OMX_PTR pCompConfig);
OMX_ERRORTYPE VT_GetConfig(OMX_INDEXTYPE nConfigIndex, OMX_PTR pCompConfig);
OMX_ERRORTYPE VT_GetParameter(OMX_INDEXTYPE nConfigIndex, OMX_PTR pCompParam);
OMX_ERRORTYPE VT_SetParameter(OMX_INDEXTYPE nParamIndex, OMX_PTR pCompParam);

OMX_ERRORTYPE VT_SetScenario(bool on);
OMX_ERRORTYPE VT_SetConfigSli(OMX_CONFIG_SLICE_LOSS_INDICATION* pSLI);
OMX_ERRORTYPE VT_SetConfigResolutionChange(OMX_VIDEO_PARAM_RESOLUTION* config);
OMX_ERRORTYPE VT_GetExtensionIndex(OMX_STRING parameterName, OMX_INDEXTYPE *pIndexType);
OMX_ERRORTYPE VT_GetResolutionChange(video_resolutionchange* pResolutionChange);

OMX_ERRORTYPE HandleVTSetVideoBitrate(OMX_VIDEO_PARAM_BITRATETYPE* pBitrateType);
OMX_ERRORTYPE HandleSetConfigVendorSliceLossIndication(OMX_CONFIG_SLICE_LOSS_INDICATION*);
OMX_ERRORTYPE HandleSetConfigVendorResolutionChange(OMX_VIDEO_PARAM_RESOLUTION*);

OMX_BOOL VT_OnInit(video_encode_param* param);

