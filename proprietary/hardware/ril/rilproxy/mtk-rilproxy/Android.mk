# Copyright 2006 The Android Open Source Project

# XXX using libutils for simulator build only...
#

ifneq ($(MTK_RIL_MODE), c6m_1rild)

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= \
    ril_callbacks.c \
    atchannel.c \
    misc.c \
    at_tok.c \
    socket_channel.cpp \
    socket_util.cpp \
    SapSocketManager.cpp \
    framework/base/RfxVariant.cpp \
    framework/base/RfxMessage.cpp \
    framework/base/RfxSocketState.cpp \
    framework/core/Rfx.cpp \
    framework/core/RfxObject.cpp \
    framework/core/RfxClassInfo.cpp \
    framework/core/RfxAction.cpp \
    framework/core/RfxAsyncSignal.cpp \
    framework/core/RfxSignal.cpp \
    framework/core/RfxTimer.cpp \
    framework/core/RfxMainThread.cpp \
    framework/core/RfxController.cpp \
    framework/core/RfxRootController.cpp \
    framework/core/RfxSlotRootController.cpp \
    framework/core/RfxControllerFactory.cpp \
    framework/core/RfxStatusManager.cpp \
    framework/core/RfxRilAdapter.cpp \
    framework/core/RfxDispatchThread.cpp \
    framework/core/RfxSocketStateManager.cpp \
    framework/base/RfxDebugInfo.cpp \
    framework/base/RfxDefaultDestUtils.cpp \
    framework/base/RfxTransferUtils.cpp \
    framework/base/RfxRilUtils.cpp \
    telephony/RfxHelloController.cpp \
    telephony/RfxSampleController.cpp \
    telephony/call/RpCallController.cpp \
    telephony/call/RpCallCenterController.cpp \
    telephony/call/RpRedialHandler.cpp \
    telephony/call/RpAudioControlHandler.cpp \
    telephony/sim/RpSimController.cpp \
    telephony/sim/RpSimControllerBase.cpp \
    telephony/sim/RpGsmSimController.cpp \
    telephony/sim/RpC2kSimController.cpp \
    framework/base/RfxCallState.cpp \
    framework/base/RfxPreciseCallState.cpp \
    telephony/cat/RpCatController.cpp \
    telephony/nw/RpNwController.cpp \
    telephony/nw/RpNwRatController.cpp \
    telephony/nw/RpBaseNwRatSwitchHandler.cpp \
    telephony/nw/RpGsmNwRatSwitchHandler.cpp \
    telephony/nw/RpCdmaNwRatSwitchHandler.cpp \
    telephony/nw/RpCdmaLteNwRatSwitchHandler.cpp \
    telephony/nw/RpNwStateController.cpp \
    telephony/nw/RpNwPhoneTypeUpdater.cpp \
    telephony/nw/RpNwWwopRatController.cpp \
    framework/base/RfxNwServiceState.cpp \
    framework/base/RfxDataSettings.cpp \
    telephony/data/RpDataController.cpp \
    telephony/data/RpIratController.cpp \
    telephony/data/RpDcApnHandler.cpp \
    telephony/data/RpDataUtils.cpp \
    telephony/data/RpDataConnectionInfo.cpp \
    telephony/data/RpDataInterfaceManager.cpp \
    telephony/data/RpDataAllowController.cpp \
    telephony/data/RpDataOpController.cpp \
    telephony/data/RpSetupDataCallParam.cpp \
    telephony/capabilityswitch/RpCapabilitySwitchController.cpp \
    telephony/capabilityswitch/RpCapabilityGetController.cpp \
    telephony/oem/RpCdmaOnlyController.cpp \
    framework/core/RfxTestBasicController.cpp \
    framework/core/RfxTestSuitController.cpp \
    telephony/phonenumber/RpPhoneNumberController.cpp \
    telephony/phonenumber/EccNumberSource.cpp \
    telephony/phb/RpPhbController.cpp \
    telephony/ss/RpSsController.cpp \
    telephony/ir/RpIrBaseHandler.cpp \
    telephony/ir/RpIrCdmaHandler.cpp \
    telephony/ir/RpIrNwRatSwitchCallback.cpp \
    telephony/ir/RpIrCdmaStatusListener.cpp \
    telephony/ir/RpIrController.cpp \
    telephony/ir/RpIrLwgHandler.cpp \
    telephony/ir/RpIrLwgStatusListener.cpp \
    telephony/ir/RpIrStrategyCommon.cpp \
    telephony/ir/RpIrStrategy.cpp \
    telephony/ir/RpIrMccTable.cpp \
    telephony/ir/utils/RpIrUtils.cpp \
    telephony/ir/utils/RpIrMccConvertMappingTables.cpp \
    telephony/ir/utils/RpIrInvalidMccConvert.cpp \
    telephony/util/RpFeatureOptionUtils.cpp \
    telephony/agps/RpAgpsSlotController.cpp \
    telephony/agps/RpAgpsNSlotController.cpp \
    telephony/agps/RpAgpsThread.cpp \
    telephony/agps/RpAgpsMessage.cpp \
    telephony/agps/RpAgpsdAdapter.cpp \
    telephony/agps/RpAgpsHandler.cpp \
    telephony/agps/RpAgpsLog.cpp \
    telephony/sms/RpSmsCtrlBase.cpp \
    telephony/sms/RpSmsCtrl.cpp \
    telephony/sms/RpGsmSmsCtrl.cpp \
    telephony/sms/RpCdmaSmsCtrl.cpp \
    telephony/sms/RpCdmaSmsCodec.cpp \
    telephony/sms/RpCdmaSmsLocalReqRsp.cpp \
    telephony/sms/RpCdmaSmsMtUrc.cpp \
    telephony/sms/RpSmsNSlotController.cpp \
    telephony/sms/RpSmsThread.cpp \
    telephony/sms/RpSmsUtil.cpp \
    telephony/sms/parser/ConcentratedSms.cpp \
    telephony/sms/parser/GsmSmsAddress.cpp \
    telephony/sms/parser/PduParser.cpp \
    telephony/sms/parser/SmsAddress.cpp \
    telephony/sms/parser/SmsHeader.cpp \
    telephony/sms/parser/SmsMessage.cpp \
    telephony/sms/parser/SmsParserUtils.cpp \
    telephony/sms/parser/SuplMsgDispatcher.cpp \
    telephony/power/RpModemController.cpp \
    telephony/power/RpRadioController.cpp \
    telephony/power/RpCdmaRadioController.cpp \
    telephony/power/RadioMessage.cpp \
    telephony/power/ModemMessage.cpp \
    telephony/oem/RpCdmaOemController.cpp \
    telephony/modecontroller/RpCardModeControllerFactory.cpp \
    telephony/modecontroller/RpCardTypeReadyController.cpp \
    telephony/modecontroller/RpModeInitController.cpp \
    telephony/modecontroller/RpOMCardModeController.cpp \
    telephony/modecontroller/RpCdmaLteModeController.cpp \
    telephony/atci/RpAtciController.cpp \
    telephony/ims/RpImsController.cpp \
    telephony/lwx/RpLwxController.cpp \
    telephony/embms/RpEmbmsControllerProxy.cpp \
    telephony/embms/RpEmbmsAtController.cpp \
    telephony/embms/RpEmbmsSessionInfo.cpp \
    telephony/embms/RpEmbmsUtils.cpp \
    telephony/embms/SNTPClient.cpp \
    telephony/wp/RpWpController.cpp \
    telephony/client/RilClient.cpp \
    telephony/client/RilClientQueue.cpp \
    telephony/client/RpRilClientController.cpp \
    telephony/client/RilAtciClient.cpp \
    telephony/client/RilMalClient.cpp \
    telephony/client/RpMalController.cpp

LOCAL_SHARED_LIBRARIES := \
    liblog libbinder libcutils libutils librilproxy librilproxyutils libratconfig libladder

LOCAL_HEADER_LIBRARIES := libaudioclient_headers libaudio_system_headers

LOCAL_SHARED_LIBRARIES += libcrypto libhidlbase vendor.mediatek.hardware.netdagent@1.0 libmtktinyxml


LOCAL_STATIC_LIBRARIES := \
    libprotobuf-c-nano-enable_malloc \

# for asprinf
LOCAL_CFLAGS := -D_GNU_SOURCE

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

ifeq ($(MTK_TC1_FEATURE),yes)
    LOCAL_CFLAGS += -DMTK_TC1_FEATURE
endif

ifeq ($(HAVE_AEE_FEATURE),yes)
LOCAL_SHARED_LIBRARIES += libaedv
LOCAL_CFLAGS += -DHAVE_AEE_FEATURE
endif

LOCAL_C_INCLUDES := \
  $(TOP)/system/core/include/utils \
  $(TOP)/frameworks/native/include/binder \
  $(TOP)/frameworks/av/include \
  $(LOCAL_PATH)/../../include/telephony \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/rilproxy/libril \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/rilproxy/mtk-rilproxy/framework/include \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/rilproxy/mtk-rilproxy/framework/include/base \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/rilproxy/mtk-rilproxy/framework/include/core \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/rilproxy/mtk-rilproxy/telephony \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/rilproxy/mtk-rilproxy/telephony/ir/include \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/rilproxy/mtk-rilproxy/telephony/nw \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/rilproxy/mtk-rilproxy/telephony/wp \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/ims/include/mal_header/include/ \
  $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/compiler/ \
  $(MTK_ROOT)/external/aee/binary/inc \
  $(MTK_ROOT)/external/libudf/libladder/

LOCAL_C_INCLUDES += $(MTK_PATH_SOURCE)/hardware/audio/common/include
LOCAL_SHARED_LIBRARIES += libaudioprimarydevicehalifclient

#ifneq ($(MTK_EMULATOR_SUPPORT),yes)

LOCAL_C_INCLUDES += $(MTK_ROOT)/external/nvram/libnvram \
                    $(LOCAL_PATH)/../../include \
                    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/mtkrilutils \
                    $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/platformlib/include/tinyxml
LOCAL_CFLAGS += -DMTK_NVRAM_SUPPORT

LOCAL_SHARED_LIBRARIES += \
        libnvram
#endif

LOCAL_C_INCLUDES += $(TOP)/$(MTK_PATH_SOURCE)/hardware/ril/rilproxy/librilutils
LOCAL_C_INCLUDES += external/nanopb-c

ifeq ($(TARGET_DEVICE),sooner)
  LOCAL_CFLAGS += -DUSE_TI_COMMANDS
endif

ifeq ($(TARGET_DEVICE),surf)
  LOCAL_CFLAGS += -DPOLL_CALL_STATE -DUSE_QMI
endif

ifeq ($(TARGET_DEVICE),dream)
  LOCAL_CFLAGS += -DPOLL_CALL_STATE -DUSE_QMI
endif

LOCAL_STATIC_LIBRARIES := \
    libprotobuf-c-nano-enable_malloc \

ifeq ("$(wildcard vendor/mediatek/internal/mtkrild_enable)","")
    LOCAL_CFLAGS += -D__PRODUCTION_RELEASE__
endif

ifeq (foo,foo)
  #build shared library
  LOCAL_SHARED_LIBRARIES += \
      libcutils libutils libmtkrilutils
  LOCAL_CFLAGS += -DRIL_SHLIB
  LOCAL_MODULE:= mtk-rilproxy
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
  include $(MTK_SHARED_LIBRARY)
else
  #build executable
  LOCAL_SHARED_LIBRARIES += \
      libril
  LOCAL_MODULE:= mtk-rilproxy
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE_OWNER := mtk
  include $(MTK_EXECUTABLE)
endif

endif
