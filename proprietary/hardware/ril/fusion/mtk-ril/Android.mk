# Copyright 2006 The Android Open Source Project

# XXX using libutils for simulator build only...
#

ifeq ($(MTK_RIL_MODE), c6m_1rild)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    ril_callbacks.c \
    framework/base/RfxCallState.cpp \
    framework/base/RfxDebugInfo.cpp \
    framework/base/RfxIntsData.cpp \
    framework/base/RfxMclMessage.cpp \
    framework/base/RfxMessage.cpp \
    framework/base/RfxNwServiceState.cpp \
    framework/base/RfxPreciseCallState.cpp \
    framework/base/RfxRawData.cpp \
    framework/base/RfxStringData.cpp \
    framework/base/RfxStringsData.cpp \
    framework/base/RfxTokUtils.cpp \
    framework/base/RfxVariant.cpp \
    framework/base/RfxVoidData.cpp \
    framework/base/RfxHardwareConfigData.cpp \
    framework/base/RfxMisc.cpp \
    framework/base/RfxRilUtils.cpp \
    framework/base/RfxNeighboringCellData.cpp \
    framework/base/RfxCellInfoData.cpp \
    framework/base/RfxSsrcData.cpp \
    framework/base/RfxSscData.cpp \
    framework/base/RfxNetworkScanData.cpp \
    framework/base/RfxNetworkScanResultData.cpp \
    framework/base/RfxVoiceRegStateData.cpp \
    framework/base/RfxDataRegStateData.cpp \
    framework/base/RfxIaApnData.cpp \
    framework/base/RfxFdModeData.cpp \
    framework/base/RfxIdToMsgIdUtils.cpp \
    framework/base/RfxActivityData.cpp \
    framework/base/RfxOpUtils.cpp \
    framework/base/RfxRsuRequestData.cpp \
    framework/base/RfxRsuResponseData.cpp \
    framework/core/Rfx.cpp \
    framework/core/RfxAction.cpp \
    framework/core/RfxAsyncSignal.cpp \
    framework/core/RfxBaseHandler.cpp \
    framework/core/RfxChannel.cpp \
    framework/core/RfxChannelManager.cpp \
    framework/core/RfxClassInfo.cpp \
    framework/core/RfxController.cpp \
    framework/core/RfxControllerFactory.cpp \
    framework/core/RfxDataCloneManager.cpp \
    framework/core/RfxDispatchThread.cpp \
    framework/core/RfxHandlerManager.cpp \
    framework/core/RfxMainThread.cpp \
    framework/core/RfxMclDispatcherThread.cpp \
    framework/core/RfxMclStatusManager.cpp \
    framework/core/RfxObject.cpp \
    framework/core/RfxReader.cpp \
    framework/core/RfxRilAdapter.cpp \
    framework/core/RfxRootController.cpp \
    framework/core/RfxSignal.cpp \
    framework/core/RfxSlotRootController.cpp \
    framework/core/RfxStatusManager.cpp \
    framework/core/RfxTestBasicController.cpp \
    framework/core/RfxTestSuitController.cpp \
    framework/core/RfxTimer.cpp \
    framework/core/RfxAtLine.cpp \
    framework/core/RfxAtResponse.cpp \
    framework/core/RfxSender.cpp \
    framework/core/RfxChannelContext.cpp \
    mdcomm/vt/RmcVtReqHandler.cpp \
    mdcomm/vt/RmcVtCommonReqHandler.cpp \
    mdcomm/vt/RmcVtMsgParser.cpp \
    mdcomm/vt/RmcVtDataThreadController.cpp \
    mdcomm/vt/RmcVtUrcHandler.cpp \
    telcore/vt/RtcVtController.cpp \
    framework/base/RfxVtCallStatusData.cpp \
    framework/base/RfxVtSendMsgData.cpp \
    mdcomm/rcs/RmcRcsReqHandler.cpp \
    mdcomm/rcs/RmcRcsDataThreadController.cpp \
    framework/base/RfxRcsSendMsgData.cpp \
    framework/core/RfxFragmentEncoder.cpp \
    framework/port/android/rfx_properties.cpp \
    mdcomm/sim/base64.cpp \
    mdcomm/sim/usim_fcp_parser.c \
    mdcomm/sim/RmcSimBaseHandler.cpp \
    mdcomm/sim/RmcCdmaSimRequestHandler.cpp \
    mdcomm/sim/RmcCdmaSimUrcHandler.cpp \
    mdcomm/sim/RmcCommSimRequestHandler.cpp \
    mdcomm/sim/RmcCommSimUrcHandler.cpp \
    mdcomm/sim/RmcGsmSimRequestHandler.cpp \
    mdcomm/sim/RmcGsmSimUrcHandler.cpp \
    mdcomm/sim/RmcSimRequestEntryHandler.cpp \
    mdcomm/sim/RmcSimUrcEntryHandler.cpp \
    mdcomm/cc/RmcEccNumberUrcHandler.cpp \
    telcore/sim/RtcCommSimController.cpp \
    telcore/sim/RtcCommSimRadioConfigController.cpp \
    framework/base/RfxSimIoData.cpp \
    framework/base/RfxSimIoRspData.cpp \
    framework/base/RfxSimStatusData.cpp \
    framework/base/RfxSimSlotStatusData.cpp \
    framework/base/RfxSimAuthData.cpp \
    framework/base/RfxSimOpenChannelData.cpp \
    framework/base/RfxSimGenAuthData.cpp \
    framework/base/RfxSimApduData.cpp \
    framework/base/RfxSimCarrRstData.cpp \
    framework/base/RfxVsimOpEventData.cpp \
    framework/base/RfxVsimOpIndicationData.cpp \
    framework/base/RfxVsimEventData.cpp \
    framework/base/RfxSimMeLockCatData.cpp \
    framework/base/RfxDialData.cpp \
    framework/base/RfxEmergencyDialData.cpp \
    framework/base/RfxRedialData.cpp \
    framework/base/RfxCallListData.cpp \
    framework/base/RfxCallFailCauseData.cpp \
    framework/base/RfxCdmaInfoRecData.cpp \
    framework/base/RfxCdmaWaitingCallData.cpp \
    framework/base/RfxDataCallResponseData.cpp \
    framework/base/RfxLceStatusResponseData.cpp \
    framework/base/RfxLceDataResponseData.cpp \
    framework/base/RfxPcoData.cpp \
    framework/base/RfxPcoIaData.cpp \
    framework/base/RfxLinkCapacityEstimateData.cpp \
    framework/base/RfxLinkCapacityReportingCriteriaData.cpp \
    framework/base/RfxEmergencyNumberListData.cpp \
    telcore/data/RtcDataController.cpp \
    mdcomm/data/RmcDcPdnManager.cpp \
    mdcomm/data/RmcDcCommonReqHandler.cpp \
    mdcomm/data/RmcDcDefaultReqHandler.cpp \
    mdcomm/data/RmcDcOnDemandReqHandler.cpp \
    mdcomm/data/RmcDcReqHandler.cpp \
    mdcomm/data/RmcDcUrcHandler.cpp \
    mdcomm/data/RmcDcUtility.cpp \
    mdcomm/data/RmcDcMiscHandler.cpp \
    mdcomm/data/RmcDcMiscImpl.cpp \
    framework/base/RfxCrssNotificationData.cpp \
    telcore/capabilityswitch/RtcCapabilityGetController.cpp \
    telcore/capabilityswitch/RtcCapabilitySwitchController.cpp \
    telcore/capabilityswitch/RtcCapabilitySwitchUtil.cpp \
    telcore/capabilityswitch/RtcCapabilitySwitchChecker.cpp \
    mdcomm/oem/RmcOemRequestHandler.cpp \
    mdcomm/oem/RmcOemUrcHandler.cpp \
    framework/base/embms/RfxEmbmsActiveSessionNotifyData.cpp \
    framework/base/embms/RfxEmbmsCellInfoNotifyData.cpp \
    framework/base/embms/RfxEmbmsDisableRespData.cpp \
    framework/base/embms/RfxEmbmsEnableRespData.cpp \
    framework/base/embms/RfxEmbmsGetCoverageRespData.cpp \
    framework/base/embms/RfxEmbmsGetTimeRespData.cpp \
    framework/base/embms/RfxEmbmsLocalEnableRespData.cpp \
    framework/base/embms/RfxEmbmsLocalOosNotifyData.cpp \
    framework/base/embms/RfxEmbmsLocalSaiNotifyData.cpp \
    framework/base/embms/RfxEmbmsLocalSessionNotifyData.cpp \
    framework/base/embms/RfxEmbmsLocalStartSessionReqData.cpp \
    framework/base/embms/RfxEmbmsLocalStartSessionRespData.cpp \
    framework/base/embms/RfxEmbmsLocalStopSessionReqData.cpp \
    framework/base/embms/RfxEmbmsModemEeNotifyData.cpp \
    framework/base/embms/RfxEmbmsOosNotifyData.cpp \
    framework/base/embms/RfxEmbmsSaiNotifyData.cpp \
    framework/base/embms/RfxEmbmsStartSessionReqData.cpp \
    framework/base/embms/RfxEmbmsStartSessionRespData.cpp \
    framework/base/embms/RfxEmbmsStopSessionReqData.cpp \
    telcore/embms/RtcEmbmsControllerProxy.cpp \
    telcore/embms/RtcEmbmsAtController.cpp \
    telcore/embms/RtcEmbmsSessionInfo.cpp \
    telcore/embms/RtcEmbmsUtils.cpp \
    telcore/embms/SNTPClient.cpp \
    telcore/cc/RtcEccNumberController.cpp \
    telcore/cc/EccNumberSource.cpp \
    mdcomm/embms/RmcEmbmsRequestHandler.cpp \
    mdcomm/embms/RmcEmbmsUtils.cpp \
    mdcomm/embms/RmcEmbmsURCHandler.cpp \
    mdcomm/common/RmcData.cpp \
    mdcomm/common/RmcMessageHandler.cpp \
    mdcomm/common/RmcChannelHandler.cpp \
    mdcomm/ims/RmcImsControlRequestHandler.cpp \
    mdcomm/ims/RmcImsControlUrcHandler.cpp \
    mdcomm/ims/RmcImsProvisioningRequestHandler.cpp \
    mdcomm/ims/RmcImsProvisioningUrcHandler.cpp \
    mdcomm/ims/ConfigUtil.cpp \
    mdcomm/ims/ImsCustomizedUtils.cpp \
    telcore/ims/RtcImsController.cpp \
    telcore/ims/RtcImsConferenceController.cpp \
    telcore/ims/RtcImsConferenceHandler.cpp \
    telcore/ims/RtcImsConferenceCallMessageHandler.cpp \
    framework/base/RfxImsCallInfo.cpp \
    framework/base/RfxImsConfData.cpp \
    framework/core/RfxXmlParser.cpp \
    telcore/ims/config/ImsConfigDataHelper.cpp \
    telcore/ims/config/ImsConfigUtils.cpp \
    telcore/ims/config/RtcImsConfigController.cpp \
    framework/port/android/rfx_gt_log.cpp \
    mdcomm/capabilityswitch/RmcCapabilitySwitchRequestHandler.cpp \
    mdcomm/capabilityswitch/RmcCapabilitySwitchURCHandler.cpp \
    mdcomm/capabilityswitch/RmcCapabilitySwitchUtil.cpp \
    framework/base/RfxRadioCapabilityData.cpp \
    framework/base/RfxIdToStringUtils.cpp \
    mdcomm/oem/RmcHardwareConfigRequestHandler.cpp \
    telcore/oem/RtcOemController.cpp \
    mdcomm/nw/RmcNetworkHandler.cpp \
    mdcomm/nw/RmcNetworkRequestHandler.cpp \
    mdcomm/nw/RmcNetworkNrtRequestHandler.cpp \
    mdcomm/nw/RmcNetworkRealTimeRequestHandler.cpp \
    mdcomm/nw/RmcNetworkUrcHandler.cpp \
    mdcomm/nw/RmcRatSwitchHandler.cpp \
    mdcomm/nw/RmcNrSwitchHandler.cpp \
    telcore/nw/RtcRatSwitchController.cpp \
    telcore/nw/RtcNetworkController.cpp \
    telcore/nw/RtcNrSwitchController.cpp \
    telcore/data/RtcDataAllowController.cpp \
    mdcomm/data/RmcDcMsimReqHandler.cpp \
    framework/base/RfxSuppServNotificationData.cpp \
    framework/base/RfxViaUtils.cpp \
    framework/base/RfxSmsRspData.cpp \
    framework/base/RfxSmsParamsData.cpp \
    framework/base/RfxSmsWriteData.cpp \
    framework/base/RfxSmsSimMemStatusCnfData.cpp \
    framework/base/RfxImsSmsData.cpp \
    framework/base/RfxGsmCbSmsCfgData.cpp \
    framework/base/RfxEtwsNotiData.cpp \
    telcore/sms/RtcImsSmsController.cpp \
    telcore/sms/RtcGsmSmsController.cpp \
    telcore/sms/RtcCdmaSmsController.cpp \
    telcore/sms/RtcSmsAddress.cpp \
    telcore/sms/RtcSmsHeader.cpp \
    telcore/sms/RtcSmsMessage.cpp \
    telcore/sms/RtcConcatSms.cpp \
    telcore/sms/RtcSmsNSlotController.cpp \
    telcore/sms/RtcSmsThread.cpp \
    telcore/sms/RtcSmsUtil.cpp \
    telcore/sms/parser/ConcentratedSms.cpp \
    telcore/sms/parser/GsmSmsAddress.cpp \
    telcore/sms/parser/PduParser.cpp \
    telcore/sms/parser/SmsAddress.cpp \
    telcore/sms/parser/SmsHeader.cpp \
    telcore/sms/parser/SmsMessage.cpp \
    telcore/sms/parser/SmsParserUtils.cpp \
    telcore/sms/parser/SuplMsgDispatcher.cpp \
    mdcomm/sms/RmcGsmSmsBaseHandler.cpp \
    mdcomm/sms/RmcGsmSmsRequestHandler.cpp \
    mdcomm/sms/RmcGsmSmsUrcHandler.cpp \
    mdcomm/sms/RmcCommSmsUrcHandler.cpp\
    mdcomm/sms/RmcCommSmsRequestHandler.cpp \
    mdcomm/sms/RmcCdmaSmsHandler.cpp \
    mdcomm/sms/RmcCdmaSmsConverter.cpp \
    mdcomm/sms/RmcCdmaMoSms.cpp \
    mdcomm/sms/RmcCdmaMtSms.cpp \
    mdcomm/sms/RmcCdmaSmsAck.cpp \
    mdcomm/sms/RmcCdmaSmsMemFull.cpp \
    mdcomm/sms/RmcCdmaBcActivate.cpp \
    mdcomm/sms/RmcCdmaBcRangeParser.cpp \
    mdcomm/sms/RmcCdmaBcConfigSet.cpp \
    mdcomm/sms/RmcCdmaBcConfigGet.cpp \
    mdcomm/sms/RmcRuimSmsWrite.cpp \
    mdcomm/sms/RmcRuimSmsDelete.cpp \
    mdcomm/sms/RmcRuimSmsMem.cpp \
    mdcomm/sms/RmcCdmaEsnMeid.cpp \
    framework/base/RfxPhbEntryData.cpp \
    framework/base/RfxPhbEntriesData.cpp \
    framework/base/RfxPhbEntryExtData.cpp \
    framework/base/RfxPhbEntriesExtData.cpp \
    framework/base/RfxPhbMemStorageData.cpp \
    mdcomm/phb/RmcPhbURCHandler.cpp \
    mdcomm/phb/RmcPhbRequestHandler.cpp \
    telcore/phb/RtcPhbController.cpp \
    mdcomm/power/RmcRadioRequestHandler.cpp \
    telcore/power/RtcRadioController.cpp \
    telcore/power/RtcModemController.cpp \
    mdcomm/power/RmcModemRequestHandler.cpp \
    telcore/modecontroller/RtcCardTypeReadyController.cpp \
    telcore/modecontroller/RtcModeSwitchController.cpp \
    telcore/wp/RtcWpController.cpp \
    mdcomm/wp/RmcWpURCHandler.cpp \
    mdcomm/wp/RmcWpRequestHandler.cpp \
    mdcomm/wp/RmcWpModemRequestHandler.cpp \
    framework/base/RfxVersionManager.cpp \
    framework/base/RfxCallForwardInfoData.cpp \
    framework/base/RfxCallForwardInfosData.cpp \
    framework/base/RfxCallForwardInfoExData.cpp \
    framework/base/RfxCallForwardInfosExData.cpp \
    telcore/ss/RtcSuppServController.cpp \
    telcore/ss/RtcSuppServQueue.cpp \
    telcore/ss/RtcSuppServUtil.cpp \
    mdcomm/ss/GsmUtil.cpp \
    mdcomm/ss/SSUtil.cpp \
    mdcomm/ss/SSConfig.cpp \
    mdcomm/ss/RmcSuppServRequestBaseHandler.cpp \
    mdcomm/ss/RmcSuppServRequestHandler.cpp \
    mdcomm/ss/RmcSuppServRequestSpecialHandler.cpp \
    mdcomm/ss/RmcSuppServUrcEntryHandler.cpp \
    mdcomm/ss/RmcSuppServUssdBaseHandler.cpp \
    mdcomm/ss/RmcSuppServUssdHandler.cpp \
    mdcomm/ss/RmcSuppServUssdDomainSelector.cpp \
    mdcomm/ss/RmcSuppServRequestNRTHandler.cpp \
    mdcomm/gba/RmcGbaRequestHandler.cpp \
    framework/base/RfxImsBearerNotifyData.cpp \
    framework/base/RfxImsDataInfoNotifyData.cpp \
    mdcomm/data/RmcDcImsReqHandler.cpp \
    mdcomm/data/RmcDcImsDataChannel2ReqHandler.cpp \
    telcore/cat/RtcCatController.cpp \
    telcore/carrierconfig/RtcCarrierConfigController.cpp \
    mdcomm/cat/RmcCatUrcHandler.cpp \
    mdcomm/cat/RmcCatCommonRequestHandler.cpp \
    framework/base/RfxSimRefreshData.cpp \
    mdcomm/cc/RmcCallControlCommonRequestHandler.cpp \
    mdcomm/cc/RmcCallControlChldRequestHandler.cpp \
    mdcomm/cc/RmcCallControlSpecialRequestHandler.cpp \
    mdcomm/cc/RmcCallControlUrcHandler.cpp \
    mdcomm/cc/RmcCallControlImsRequestHandler.cpp \
    mdcomm/cc/RmcCallControlImsUrcHandler.cpp \
    mdcomm/cc/RmcCallControlBaseHandler.cpp \
    mdcomm/cc/RmcRadioRelatedRequestHandler.cpp \
    telcore/cc/RtcCallController.cpp \
    telcore/cc/RtcRedialController.cpp \
    telcore/client/RtcRilClientController.cpp \
    telcore/client/RilClient.cpp \
    telcore/client/RilClientQueue.cpp \
    telcore/client/RilOemClient.cpp \
    telcore/client/RilMttsClient.cpp \
    telcore/client/RilAtciClient.cpp \
    telcore/client/RilParcelUtils.cpp \
    telcore/atci/RtcAtciController.cpp \
    mdcomm/atci/RmcAtciCommonRequestHandler.cpp \
    mdcomm/atci/RmcAtciRequestHandler.cpp \
    mdcomm/atci/RmcAtciSpecialRequestHandler.cpp \
    framework/base/RfxSetDataProfileData.cpp \
    mdcomm/mwi/RmcMobileWifiUrcHandler.cpp \
    mdcomm/mwi/RmcMobileWifiRequestHandler.cpp \
    telcore/mwi/RtcMobileWifiController.cpp \
    mdcomm/agps/RmcAgpsRequestHandler.cpp \
    mdcomm/agps/RmcAgpsURCHandler.cpp \
    telcore/agps/RtcAgpsNSlotController.cpp \
    telcore/agps/RtcAgpsThread.cpp \
    telcore/agps/RtcAgpsdAdapter.cpp \
    telcore/agps/RtcAgpsUtil.cpp \
    mdcomm/power/RmcRadioUrcHandler.cpp \
    framework/base/RfxDialogInfoData.cpp \
    framework/base/RfxDialog.cpp \
    telcore/ims/RtcImsDialogHandler.cpp \
    mdcomm/phb/RmcPhbSimIoRequestHandler.cpp \
    telcore/phb/RtcPhbSimIoController.cpp \
    framework/base/RfxDefaultValueTable.cpp \
    telcore/client/RilCommandsTable.cpp \
    mdcomm/gwsd/RfxGwsdUtils.cpp \
    mdcomm/gwsd/RmcGwsdRequestHandler.cpp \
    telcore/ims/RtcImsRttController.cpp \
    mdcomm/ims/RmcImsRttControlRequestHandler.cpp \
    mdcomm/ims/RmcImsRttControlUrcHandler.cpp \
    framework/base/RfxPhoneCapabilityData.cpp \
    framework/base/RfxSipRegInfoData.cpp \
    framework/base/RfxImsRegInfoData.cpp \
    framework/base/RfxEiregData.cpp \
    framework/base/RfxAuthMsgData.cpp \
    framework/base/RfxCertMsgData.cpp \
    framework/base/RfxAuthResponseData.cpp \
    framework/base/RfxCapabilityData.cpp \
    framework/base/RfxCertResponseData.cpp \
    framework/base/RfxQueryCapData.cpp \
    framework/base/RfxKeepaliveRequestData.cpp \
    framework/base/RfxKeepaliveStatusData.cpp \

LOCAL_SHARED_LIBRARIES := \
    libmtkutils librilfusion libmtknetutils libratconfig libmtksysutils libmtkhardware_legacy \
    libsysenv libmtkrilutils libcrypto libmtkcutils libmtkproperty libmtkrillog \
    libmtkconfigutils libmtktinyxml \
    libmtkares \
    libmtknetcap \
    libwpfa\

LOCAL_STATIC_LIBRARIES := \
    libprotobuf-c-nano-enable_malloc \

LOCAL_HEADER_LIBRARIES := libWpfaHeaders

# for asprinf
LOCAL_CFLAGS := -D_GNU_SOURCE

LOCAL_CFLAGS += -DMTK_SYSENV_SUPPORT

#used to check if support telephonyware
LOCAL_CFLAGS += -DMTK_TELEPHONYWARE_SUPPORT

ifneq ($(MTK_NUM_MODEM_PROTOCOL),1)
    LOCAL_CFLAGS += -DANDROID_MULTI_SIM
endif

ifeq ($(MTK_NUM_MODEM_PROTOCOL), 2)
    LOCAL_CFLAGS += -DANDROID_SIM_COUNT_2
endif

ifeq ($(MTK_NUM_MODEM_PROTOCOL), 3)
    LOCAL_CFLAGS += -DANDROID_SIM_COUNT_3
endif

ifeq ($(MTK_NUM_MODEM_PROTOCOL), 4)
    LOCAL_CFLAGS += -DANDROID_SIM_COUNT_4
endif

LOCAL_CFLAGS += -Werror

ifeq ($(MTK_TC1_FEATURE),yes)
    LOCAL_CFLAGS += -DMTK_TC1_FEATURE
endif

ifeq ($(HAVE_AEE_FEATURE),yes)
LOCAL_SHARED_LIBRARIES += libaedv
LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../include \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/fusion/mtk-ril/framework/include \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/fusion/mtk-ril/framework/include/base \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/fusion/mtk-ril/framework/include/base/embms \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/fusion/mtk-ril/framework/include/core \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/fusion/mtk-ril/framework/port/android/include \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/fusion/mtk-ril/ \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/fusion/mtk-ril/telcore \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/fusion/mtk-ril/telcore/client \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/fusion/mtk-ril/mdcomm/ \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/fusion/mtk-ril/mdcomm/capabilityswitch \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/netagent/libnetagent/ \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/netagent/libnetagent/na/ \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/fusion/mtk-ril/telcore/wp \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/fusion/mtk-ril/mdcomm/wp \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/fusion/mtk-ril/mdcomm/ss \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/fusion/mtk-ril/mdcomm/gba \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/fusion/libcarrierconfig \
    $(TOP)/frameworks/native/include/binder \
    $(TOP)/vendor/mediatek/proprietary/hardware/ims/include/volte_header/volte_imcb/sap \
    $(TOP)/vendor/mediatek/proprietary/hardware/ims/include/volte_header/volte_core/public_header \
    $(MTK_ROOT)/external/aee/binary/inc \
    $(MTK_PATH_SOURCE)/hardware/ccci/include \
    $(MTK_PATH_SOURCE)/external/libsysenv \
    $(MTK_PATH_SOURCE)/system/netdagent/include \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/utils \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/property \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/log \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/mtkrilutils \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/config \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/tinyxml \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/fusion/mtk-ril/mdcomm/sim \
    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/fusion/mtk-ril/mdcomm/gwsd/include/gwsd \

ifneq ($(wildcard vendor/mediatek/proprietary/operator/hardware/ril/fusion/Android.mk),)
    LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/operator/hardware/ril/fusion/mtk-ril/framework/include/
    LOCAL_CFLAGS += -DMTK_OPERATOR_ADDON
endif

LOCAL_CFLAGS += -DMTK_MUX_CHANNEL_64
LOCAL_CFLAGS += -DMTK_IMS_CHANNEL_SUPPORT

#use android binder
LOCAL_CFLAGS += -DMTK_USE_HIDL
LOCAL_SHARED_LIBRARIES += libbinder

#LOCAL_LDFLAGS := -Wl,--unresolved-symbols=ignore-all

LOCAL_C_INCLUDES += $(TARGET_OUT_HEADER)libmtkrilutils
LOCAL_C_INCLUDES += external/nanopb-c

LOCAL_STATIC_LIBRARIES := \
    libprotobuf-c-nano-enable_malloc \
    libnetagent \

# for vilte & imcb needed
LOCAL_CFLAGS += -D __IMCF_MTK_VA__=1 -D __IMCF_NO_UA__=1

# for ss & xcap needed
LOCAL_REQUIRED_MODULES := libutinterface_md

# External SIM
ifeq ($(strip $(MTK_EXTERNAL_SIM_SUPPORT)), yes)
    LOCAL_STATIC_LIBRARIES += libmtk-fusion-ril-prop-vsim
    LOCAL_CFLAGS += -DMTK_EXTERNAL_SIM_SUPPORT
#else ifeq ($(strip $(RELEASE_BRM)), yes)
#    LOCAL_STATIC_LIBRARIES += libmtk-fusion-ril-prop-vsim
endif

ifneq ($(filter address, $(strip $(SANITIZE_TARGET))),)
LOCAL_CFLAGS += -fno-sanitize-address-use-after-scope
endif

ifeq (foo,foo)
  #build shared library
  LOCAL_CFLAGS += -DRIL_SHLIB
  LOCAL_MODULE:= libmtk-ril
  LOCAL_PROPRIETARY_MODULE := true
  LOCAL_MODULE_OWNER := mtk
  LOCAL_MULTILIB := first
  include $(MTK_SHARED_LIBRARY)
else
#build executable
  LOCAL_SHARED_LIBRARIES += \
      libril
  LOCAL_MODULE:= libmtk-ril
  LOCAL_PROPRIETARY_MODULE := true
  LOCAL_MODULE_OWNER := mtk
  LOCAL_MULTILIB := first
  include $(MTK_EXECUTABLE)
endif

endif
