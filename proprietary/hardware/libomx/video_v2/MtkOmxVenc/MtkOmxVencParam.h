
void MtkOmxVencParam();
void ComponentInitParam();

/* Set Param Handle */
OMX_ERRORTYPE HandleSetPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE*); // inport outport
OMX_ERRORTYPE SetInputPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE*);
OMX_ERRORTYPE SetOutputPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE*);

OMX_ERRORTYPE HandleSetVideoPortFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE*); // inport outport
OMX_ERRORTYPE HandleSetStandardComponentRole(OMX_PARAM_COMPONENTROLETYPE*);

OMX_ERRORTYPE HandleSetVideoAvc(OMX_VIDEO_PARAM_AVCTYPE*); //copy outport
OMX_ERRORTYPE HandleSetVideoHevc(OMX_VIDEO_PARAM_HEVCTYPE*); //copy outport
OMX_ERRORTYPE HandleSetVideoH263(OMX_VIDEO_PARAM_H263TYPE*); //copy outport
OMX_ERRORTYPE HandleSetVideoMpeg4(OMX_VIDEO_PARAM_MPEG4TYPE*); //copy outport
OMX_ERRORTYPE HandleSetVideoVp8(OMX_VIDEO_PARAM_VP8TYPE*); //copy outport

OMX_BOOL mSetStreamingMode;
OMX_BOOL mSetConstantBitrateMode;
OMX_VIDEO_PARAM_BITRATETYPE mBitrateType;
OMX_VIDEO_CONTROLRATETYPE mABitrateMode;
OMX_U32 mABitrateModeValue;
OMX_ERRORTYPE HandleSetVideoBitrate(OMX_VIDEO_PARAM_BITRATETYPE*); // outport

OMX_ERRORTYPE HandleSetVideoQuantization(OMX_VIDEO_PARAM_QUANTIZATIONTYPE*); //copy outport
OMX_ERRORTYPE HandleSetVideoVBSMC(OMX_VIDEO_PARAM_VBSMCTYPE*); //copy outport
OMX_ERRORTYPE HandleSetVideoMotionVector(OMX_VIDEO_PARAM_MOTIONVECTORTYPE*); //copy outport
OMX_ERRORTYPE HandleSetVideoIntraRefresh(OMX_VIDEO_PARAM_INTRAREFRESHTYPE*); //copy outport
OMX_ERRORTYPE HandleSetVideoSliceFMO(OMX_VIDEO_PARAM_AVCSLICEFMO*); //copy outport
OMX_ERRORTYPE HandleSetVideoErrorCorrection(OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE*); //copy outport

OMX_ERRORTYPE HandleSetGoogleStoreMetaDataInBuffers(StoreMetaDataInBuffersParams*); // inport outport
OMX_ERRORTYPE HandleSetGoogleStoreANWBufferInMetadata(StoreMetaDataInBuffersParams*); // inport
OMX_ERRORTYPE HandleSetGoogleEnableAndroidNativeHandle(AllocateNativeHandleParams*); // outport
OMX_ERRORTYPE HandleSetGoogleTemporalLayering(OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE*); // outport

OMX_ERRORTYPE HandleSetVendor3DVideoRecode(OMX_VIDEO_H264FPATYPE*);
OMX_ERRORTYPE HandleSetVendorTimelapseMode(OMX_BOOL*);
OMX_ERRORTYPE HandleSetVendorWhiteboardEffectMode(OMX_BOOL*);
OMX_ERRORTYPE HandleSetVendorSetMCIMode(OMX_BOOL*); // use MVAMgr
OMX_ERRORTYPE HandleSetVendorSetScenario(OMX_PARAM_U32TYPE*);
OMX_ERRORTYPE HandleSetVendorPrependSPSPPS(PrependSPSPPSToIDRFramesParams*);
OMX_ERRORTYPE HandleSetVendorNonRefPOp(OMX_VIDEO_NONREFP*);
OMX_ERRORTYPE HandleSetVendorSetClientLocally(OMX_CONFIG_BOOLEANTYPE*);

/* Get Param Handle */
OMX_ERRORTYPE HandleGetPortDefinition(OMX_PARAM_PORTDEFINITIONTYPE*);
OMX_ERRORTYPE HandleGetVideoInit(OMX_PORT_PARAM_TYPE*);
OMX_ERRORTYPE HandleGetAudioInit(OMX_PORT_PARAM_TYPE*);
OMX_ERRORTYPE HandleGetImageInit(OMX_PORT_PARAM_TYPE*);
OMX_ERRORTYPE HandleGetOtherInit(OMX_PORT_PARAM_TYPE*);

OMX_ERRORTYPE HandleGetVideoPortFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE*);
OMX_ERRORTYPE GetInputPortFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE*);
OMX_ERRORTYPE GetOutputPortFormat(OMX_VIDEO_PARAM_PORTFORMATTYPE*);

OMX_ERRORTYPE HandleGetStandardComponentRole(OMX_PARAM_COMPONENTROLETYPE*);
OMX_ERRORTYPE HandleGetVideoAvc(OMX_VIDEO_PARAM_AVCTYPE*);
OMX_ERRORTYPE HandleGetVideoHevc(OMX_VIDEO_PARAM_HEVCTYPE*);
OMX_ERRORTYPE HandleGetVideoH263(OMX_VIDEO_PARAM_H263TYPE*);
OMX_ERRORTYPE HandleGetVideoMpeg4(OMX_VIDEO_PARAM_MPEG4TYPE*);
OMX_ERRORTYPE HandleGetVideoVp8(OMX_VIDEO_PARAM_VP8TYPE*);
OMX_ERRORTYPE HandleGetVideoProfileLevelQuerySupported(OMX_VIDEO_PARAM_PROFILELEVELTYPE*);
OMX_ERRORTYPE HandleGetVideoBitrate(OMX_VIDEO_PARAM_BITRATETYPE*);
OMX_ERRORTYPE HandleGetVideoQuantization(OMX_VIDEO_PARAM_QUANTIZATIONTYPE*);
OMX_ERRORTYPE HandleGetVideoVBSMC(OMX_VIDEO_PARAM_VBSMCTYPE*);
OMX_ERRORTYPE HandleGetVideoMotionVector(OMX_VIDEO_PARAM_MOTIONVECTORTYPE*);
OMX_ERRORTYPE HandleGetVideoIntraRefresh(OMX_VIDEO_PARAM_INTRAREFRESHTYPE*);
OMX_ERRORTYPE HandleGetVideoSliceFMO(OMX_VIDEO_PARAM_AVCSLICEFMO*);
OMX_ERRORTYPE HandleGetVideoErrorCorrection(OMX_VIDEO_PARAM_ERRORCORRECTIONTYPE*);

OMX_ERRORTYPE HandleGetVendorNonRefPOp(OMX_VIDEO_NONREFP*);
OMX_ERRORTYPE HandleGetVendorOmxHandle(OMX_U32*);
OMX_ERRORTYPE HandleGetVendorQueryDriverVersion(OMX_VIDEO_PARAM_DRIVERVER*);
OMX_ERRORTYPE HandleGetVendorQueryCodecsSizes(OMX_VIDEO_PARAM_SPEC_QUERY*);

OMX_ERRORTYPE HandleGetGoogleDescribeColorFormat(DescribeColorFormatParams*);
OMX_ERRORTYPE HandleGetGoogleDescribeColorFormat2(DescribeColorFormat2Params*);
OMX_ERRORTYPE HandleGetGoogleTemporalLayering(OMX_VIDEO_PARAM_ANDROID_TEMPORALLAYERINGTYPE*);
