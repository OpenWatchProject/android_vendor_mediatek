/*
 * Copyright (c) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RIL_SERVICE_H
#define RIL_SERVICE_H

#include <telephony/mtk_ril.h>
#include <ril_internal.h>

namespace radio {
void registerService(RIL_RadioFunctions *callbacks, android::CommandInfo *commands);

int getIccCardStatusResponse(int slotId, android::ClientId clientId, int responseType,
                            int token, RIL_Errno e, void *response, size_t responselen);

int supplyIccPinForAppResponse(int slotId, android::ClientId clientId,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responselen);

int supplyIccPukForAppResponse(int slotId, android::ClientId clientId,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responselen);

int supplyIccPin2ForAppResponse(int slotId, android::ClientId clientId,
                               int responseType, int serial, RIL_Errno e, void *response,
                               size_t responselen);

int supplyIccPuk2ForAppResponse(int slotId, android::ClientId clientId,
                               int responseType, int serial, RIL_Errno e, void *response,
                               size_t responselen);

int changeIccPinForAppResponse(int slotId, android::ClientId clientId,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responselen);

int changeIccPin2ForAppResponse(int slotId, android::ClientId clientId,
                               int responseType, int serial, RIL_Errno e, void *response,
                               size_t responselen);

int supplyNetworkDepersonalizationResponse(int slotId, android::ClientId clientId,
                                          int responseType, int serial, RIL_Errno e,
                                          void *response, size_t responselen);

int getCurrentCallsResponse(int slotId, android::ClientId clientId,
                           int responseType, int serial, RIL_Errno e, void *response,
                           size_t responselen);

int dialResponse(int slotId, android::ClientId clientId,
                int responseType, int serial, RIL_Errno e, void *response, size_t responselen);

int emergencyDialResponse(int slotId, android::ClientId clientId,
                          int responseType, int serial, RIL_Errno e,
                          void *response, size_t responselen);

int getIMSIForAppResponse(int slotId, android::ClientId clientId, int responseType,
                         int serial, RIL_Errno e, void *response, size_t responselen);

int hangupConnectionResponse(int slotId, android::ClientId clientId, int responseType,
                            int serial, RIL_Errno e, void *response, size_t responselen);

int hangupWaitingOrBackgroundResponse(int slotId, android::ClientId clientId,
                                     int responseType, int serial, RIL_Errno e, void *response,
                                     size_t responselen);

int hangupForegroundResumeBackgroundResponse(int slotId, android::ClientId clientId,
                                            int responseType, int serial, RIL_Errno e,
                                            void *response, size_t responselen);

int switchWaitingOrHoldingAndActiveResponse(int slotId, android::ClientId clientId,
                                           int responseType, int serial, RIL_Errno e,
                                           void *response, size_t responselen);

int conferenceResponse(int slotId, android::ClientId clientId, int responseType,
                      int serial, RIL_Errno e, void *response, size_t responselen);

int rejectCallResponse(int slotId, android::ClientId clientId, int responseType,
                      int serial, RIL_Errno e, void *response, size_t responselen);

int getLastCallFailCauseResponse(int slotId, android::ClientId clientId,
                                int responseType, int serial, RIL_Errno e, void *response,
                                size_t responselen);

int getSignalStrengthResponse(int slotId, android::ClientId clientId,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responseLen);

int getVoiceRegistrationStateResponse(int slotId, android::ClientId clientId,
                                     int responseType, int serial, RIL_Errno e, void *response,
                                     size_t responselen);

int getDataRegistrationStateResponse(int slotId, android::ClientId clientId,
                                    int responseType, int serial, RIL_Errno e, void *response,
                                    size_t responselen);

int getOperatorResponse(int slotId, android::ClientId clientId,
                       int responseType, int serial, RIL_Errno e, void *response,
                       size_t responselen);

int setRadioPowerResponse(int slotId, android::ClientId clientId,
                         int responseType, int serial, RIL_Errno e, void *response,
                         size_t responselen);

int sendDtmfResponse(int slotId, android::ClientId clientId,
                    int responseType, int serial, RIL_Errno e, void *response,
                    size_t responselen);

int sendSmsResponse(int slotId, android::ClientId clientId,
                   int responseType, int serial, RIL_Errno e, void *response,
                   size_t responselen);

int sendSMSExpectMoreResponse(int slotId, android::ClientId clientId,
                             int responseType, int serial, RIL_Errno e, void *response,
                             size_t responselen);

int setupDataCallResponse(int slotId, android::ClientId clientId,
                          int responseType, int serial, RIL_Errno e, void *response,
                          size_t responseLen);

int iccIOForAppResponse(int slotId, android::ClientId clientId,
                       int responseType, int serial, RIL_Errno e, void *response,
                       size_t responselen);

int sendUssdResponse(int slotId, android::ClientId clientId,
                    int responseType, int serial, RIL_Errno e, void *response,
                    size_t responselen);

int cancelPendingUssdResponse(int slotId, android::ClientId clientId,
                             int responseType, int serial, RIL_Errno e, void *response,
                             size_t responselen);

int getClirResponse(int slotId, android::ClientId clientId,
                   int responseType, int serial, RIL_Errno e, void *response, size_t responselen);

int setClirResponse(int slotId, android::ClientId clientId,
                   int responseType, int serial, RIL_Errno e, void *response, size_t responselen);

int getCallForwardStatusResponse(int slotId, android::ClientId clientId,
                                int responseType, int serial, RIL_Errno e, void *response,
                                size_t responselen);

int setCallForwardResponse(int slotId, android::ClientId clientId,
                          int responseType, int serial, RIL_Errno e, void *response,
                          size_t responselen);

int getCallWaitingResponse(int slotId, android::ClientId clientId,
                          int responseType, int serial, RIL_Errno e, void *response,
                          size_t responselen);

int setCallWaitingResponse(int slotId, android::ClientId clientId,
                          int responseType, int serial, RIL_Errno e, void *response,
                          size_t responselen);

int acknowledgeLastIncomingGsmSmsResponse(int slotId, android::ClientId clientId,
                                         int responseType, int serial, RIL_Errno e, void *response,
                                         size_t responselen);

int acceptCallResponse(int slotId, android::ClientId clientId,
                      int responseType, int serial, RIL_Errno e, void *response,
                      size_t responselen);

int deactivateDataCallResponse(int slotId, android::ClientId clientId,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responselen);

int getFacilityLockForAppResponse(int slotId, android::ClientId clientId,
                                 int responseType, int serial, RIL_Errno e, void *response,
                                 size_t responselen);

int setFacilityLockForAppResponse(int slotId, android::ClientId clientId,
                                 int responseType, int serial, RIL_Errno e, void *response,
                                 size_t responselen);

int setBarringPasswordResponse(int slotId, android::ClientId clientId,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responselen);

int getNetworkSelectionModeResponse(int slotId, android::ClientId clientId,
                                   int responseType, int serial, RIL_Errno e, void *response,
                                   size_t responselen);

int setNetworkSelectionModeAutomaticResponse(int slotId, android::ClientId clientId,
                                            int responseType, int serial, RIL_Errno e,
                                            void *response, size_t responselen);

int setNetworkSelectionModeManualResponse(int slotId, android::ClientId clientId,
                                         int responseType, int serial, RIL_Errno e, void *response,
                                         size_t responselen);

int getAvailableNetworksResponse(int slotId, android::ClientId clientId,
                                int responseType, int serial, RIL_Errno e, void *response,
                                size_t responselen);

int startNetworkScanResponse(int slotId, android::ClientId clientId,
                             int responseType, int serial, RIL_Errno e, void *response,
                             size_t responselen);

int stopNetworkScanResponse(int slotId, android::ClientId clientId,
                            int responseType, int serial, RIL_Errno e, void *response,
                            size_t responselen);

int startDtmfResponse(int slotId, android::ClientId clientId,
                     int responseType, int serial, RIL_Errno e, void *response,
                     size_t responselen);

int stopDtmfResponse(int slotId, android::ClientId clientId,
                    int responseType, int serial, RIL_Errno e, void *response,
                    size_t responselen);

int getBasebandVersionResponse(int slotId, android::ClientId clientId,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responselen);

int separateConnectionResponse(int slotId, android::ClientId clientId,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responselen);

int setMuteResponse(int slotId, android::ClientId clientId,
                   int responseType, int serial, RIL_Errno e, void *response,
                   size_t responselen);

int getMuteResponse(int slotId, android::ClientId clientId,
                   int responseType, int serial, RIL_Errno e, void *response,
                   size_t responselen);

int getClipResponse(int slotId, android::ClientId clientId,
                   int responseType, int serial, RIL_Errno e, void *response,
                   size_t responselen);

int getDataCallListResponse(int slotId, android::ClientId clientId,
                            int responseType, int serial, RIL_Errno e,
                            void *response, size_t responseLen);

int setSuppServiceNotificationsResponse(int slotId, android::ClientId clientId,
                                       int responseType, int serial, RIL_Errno e, void *response,
                                       size_t responselen);

int writeSmsToSimResponse(int slotId, android::ClientId clientId,
                         int responseType, int serial, RIL_Errno e, void *response,
                         size_t responselen);

int deleteSmsOnSimResponse(int slotId, android::ClientId clientId,
                          int responseType, int serial, RIL_Errno e, void *response,
                          size_t responselen);

int setBandModeResponse(int slotId, android::ClientId clientId,
                       int responseType, int serial, RIL_Errno e, void *response,
                       size_t responselen);

int getAvailableBandModesResponse(int slotId, android::ClientId clientId,
                                 int responseType, int serial, RIL_Errno e, void *response,
                                 size_t responselen);

int sendEnvelopeResponse(int slotId, android::ClientId clientId,
                        int responseType, int serial, RIL_Errno e, void *response,
                        size_t responselen);

int sendTerminalResponseToSimResponse(int slotId, android::ClientId clientId,
                                     int responseType, int serial, RIL_Errno e, void *response,
                                     size_t responselen);

int handleStkCallSetupRequestFromSimResponse(int slotId, android::ClientId clientId,
                                            int responseType, int serial, RIL_Errno e,
                                            void *response, size_t responselen);

int explicitCallTransferResponse(int slotId, android::ClientId clientId,
                                int responseType, int serial, RIL_Errno e, void *response,
                                size_t responselen);

int setPreferredNetworkTypeResponse(int slotId, android::ClientId clientId,
                                   int responseType, int serial, RIL_Errno e, void *response,
                                   size_t responselen);

int getPreferredNetworkTypeResponse(int slotId, android::ClientId clientId,
                                   int responseType, int serial, RIL_Errno e, void *response,
                                   size_t responselen);

int getNeighboringCidsResponse(int slotId, android::ClientId clientId,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responselen);

int setLocationUpdatesResponse(int slotId, android::ClientId clientId,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responselen);

int setCdmaSubscriptionSourceResponse(int slotId, android::ClientId clientId,
                                     int responseType, int serial, RIL_Errno e, void *response,
                                     size_t responselen);

int setCdmaRoamingPreferenceResponse(int slotId, android::ClientId clientId,
                                    int responseType, int serial, RIL_Errno e, void *response,
                                    size_t responselen);

int getCdmaRoamingPreferenceResponse(int slotId, android::ClientId clientId,
                                    int responseType, int serial, RIL_Errno e, void *response,
                                    size_t responselen);

int setTTYModeResponse(int slotId, android::ClientId clientId,
                      int responseType, int serial, RIL_Errno e, void *response,
                      size_t responselen);

int getTTYModeResponse(int slotId, android::ClientId clientId,
                      int responseType, int serial, RIL_Errno e, void *response,
                      size_t responselen);

int setPreferredVoicePrivacyResponse(int slotId, android::ClientId clientId,
                                    int responseType, int serial, RIL_Errno e, void *response,
                                    size_t responselen);

int getPreferredVoicePrivacyResponse(int slotId, android::ClientId clientId,
                                    int responseType, int serial, RIL_Errno e, void *response,
                                    size_t responselen);

int sendCDMAFeatureCodeResponse(int slotId, android::ClientId clientId,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responselen);

int sendBurstDtmfResponse(int slotId, android::ClientId clientId,
                         int responseType, int serial, RIL_Errno e, void *response,
                         size_t responselen);

int sendCdmaSmsResponse(int slotId, android::ClientId clientId,
                       int responseType, int serial, RIL_Errno e, void *response,
                       size_t responselen);

int acknowledgeLastIncomingCdmaSmsResponse(int slotId, android::ClientId clientId,
                                          int responseType, int serial, RIL_Errno e, void *response,
                                          size_t responselen);

int getGsmBroadcastConfigResponse(int slotId, android::ClientId clientId,
                                 int responseType, int serial, RIL_Errno e, void *response,
                                 size_t responselen);

int setGsmBroadcastConfigResponse(int slotId, android::ClientId clientId,
                                 int responseType, int serial, RIL_Errno e, void *response,
                                 size_t responselen);

int setGsmBroadcastActivationResponse(int slotId, android::ClientId clientId,
                                     int responseType, int serial, RIL_Errno e, void *response,
                                     size_t responselen);

int getCdmaBroadcastConfigResponse(int slotId, android::ClientId clientId,
                                  int responseType, int serial, RIL_Errno e, void *response,
                                  size_t responselen);

int setCdmaBroadcastConfigResponse(int slotId, android::ClientId clientId,
                                  int responseType, int serial, RIL_Errno e, void *response,
                                  size_t responselen);

int setCdmaBroadcastActivationResponse(int slotId, android::ClientId clientId,
                                      int responseType, int serial, RIL_Errno e,
                                      void *response, size_t responselen);

int getCDMASubscriptionResponse(int slotId, android::ClientId clientId,
                               int responseType, int serial, RIL_Errno e, void *response,
                               size_t responselen);

int writeSmsToRuimResponse(int slotId, android::ClientId clientId,
                          int responseType, int serial, RIL_Errno e, void *response,
                          size_t responselen);

int deleteSmsOnRuimResponse(int slotId, android::ClientId clientId,
                           int responseType, int serial, RIL_Errno e, void *response,
                           size_t responselen);

int getDeviceIdentityResponse(int slotId, android::ClientId clientId,
                             int responseType, int serial, RIL_Errno e, void *response,
                             size_t responselen);

int exitEmergencyCallbackModeResponse(int slotId, android::ClientId clientId,
                                     int responseType, int serial, RIL_Errno e, void *response,
                                     size_t responselen);

int getSmscAddressResponse(int slotId, android::ClientId clientId,
                          int responseType, int serial, RIL_Errno e, void *response,
                          size_t responselen);

int setCdmaBroadcastActivationResponse(int slotId, android::ClientId clientId,
                                      int responseType, int serial, RIL_Errno e,
                                      void *response, size_t responselen);

int setSmscAddressResponse(int slotId, android::ClientId clientId,
                          int responseType, int serial, RIL_Errno e,
                          void *response, size_t responselen);

int reportSmsMemoryStatusResponse(int slotId, android::ClientId clientId,
                                 int responseType, int serial, RIL_Errno e,
                                 void *response, size_t responselen);

int reportStkServiceIsRunningResponse(int slotId, android::ClientId clientId,
                                      int responseType, int serial, RIL_Errno e,
                                      void *response, size_t responseLen);

int getCdmaSubscriptionSourceResponse(int slotId, android::ClientId clientId,
                                     int responseType, int serial, RIL_Errno e, void *response,
                                     size_t responselen);

int requestIsimAuthenticationResponse(int slotId, android::ClientId clientId,
                                     int responseType, int serial, RIL_Errno e, void *response,
                                     size_t responselen);

int acknowledgeIncomingGsmSmsWithPduResponse(int slotId, android::ClientId clientId,
                                            int responseType, int serial, RIL_Errno e,
                                            void *response, size_t responselen);

int sendEnvelopeWithStatusResponse(int slotId, android::ClientId clientId,
                                  int responseType, int serial, RIL_Errno e, void *response,
                                  size_t responselen);

int getVoiceRadioTechnologyResponse(int slotId, android::ClientId clientId,
                                   int responseType, int serial, RIL_Errno e,
                                   void *response, size_t responselen);

int getCellInfoListResponse(int slotId, android::ClientId clientId,
                            int responseType,
                            int serial, RIL_Errno e, void *response,
                            size_t responseLen);

int setCellInfoListRateResponse(int slotId, android::ClientId clientId,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responselen);

int setInitialAttachApnResponse(int slotId, android::ClientId clientId,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responselen);

int getImsRegistrationStateResponse(int slotId, android::ClientId clientId,
                                   int responseType, int serial, RIL_Errno e,
                                   void *response, size_t responselen);

int sendImsSmsResponse(int slotId, android::ClientId clientId, int responseType,
                      int serial, RIL_Errno e, void *response, size_t responselen);

int iccTransmitApduBasicChannelResponse(int slotId, android::ClientId clientId,
                                       int responseType, int serial, RIL_Errno e,
                                       void *response, size_t responselen);

int iccOpenLogicalChannelResponse(int slotId, android::ClientId clientId,
                                  int responseType, int serial, RIL_Errno e, void *response,
                                  size_t responselen);


int iccCloseLogicalChannelResponse(int slotId, android::ClientId clientId,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responselen);

int iccTransmitApduLogicalChannelResponse(int slotId, android::ClientId clientId,
                                         int responseType, int serial, RIL_Errno e,
                                         void *response, size_t responselen);

int nvReadItemResponse(int slotId, android::ClientId clientId,
                      int responseType, int serial, RIL_Errno e,
                      void *response, size_t responselen);


int nvWriteItemResponse(int slotId, android::ClientId clientId,
                       int responseType, int serial, RIL_Errno e,
                       void *response, size_t responselen);

int nvWriteCdmaPrlResponse(int slotId, android::ClientId clientId,
                          int responseType, int serial, RIL_Errno e,
                          void *response, size_t responselen);

int nvResetConfigResponse(int slotId, android::ClientId clientId,
                         int responseType, int serial, RIL_Errno e,
                         void *response, size_t responselen);

int setUiccSubscriptionResponse(int slotId, android::ClientId clientId,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responselen);

int setDataAllowedResponse(int slotId, android::ClientId clientId,
                          int responseType, int serial, RIL_Errno e,
                          void *response, size_t responselen);

int getHardwareConfigResponse(int slotId, android::ClientId clientId,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responseLen);

int requestIccSimAuthenticationResponse(int slotId, android::ClientId clientId,
                                       int responseType, int serial, RIL_Errno e,
                                       void *response, size_t responselen);

int setDataProfileResponse(int slotId, android::ClientId clientId,
                          int responseType, int serial, RIL_Errno e,
                          void *response, size_t responselen);

int requestShutdownResponse(int slotId, android::ClientId clientId,
                           int responseType, int serial, RIL_Errno e,
                           void *response, size_t responselen);

int getRadioCapabilityResponse(int slotId, android::ClientId clientId,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responseLen);

int setRadioCapabilityResponse(int slotId, android::ClientId clientId,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responseLen);

int startLceServiceResponse(int slotId, android::ClientId clientId,
                           int responseType, int serial, RIL_Errno e,
                           void *response, size_t responselen);

int stopLceServiceResponse(int slotId, android::ClientId clientId,
                          int responseType, int serial, RIL_Errno e,
                          void *response, size_t responselen);

int pullLceDataResponse(int slotId, android::ClientId clientId,
                        int responseType, int serial, RIL_Errno e,
                        void *response, size_t responseLen);

int getModemActivityInfoResponse(int slotId, android::ClientId clientId,
                                int responseType, int serial, RIL_Errno e,
                                void *response, size_t responselen);

int setAllowedCarriersResponse(int slotId, android::ClientId clientId,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responselen);

int getAllowedCarriersResponse(int slotId, android::ClientId clientId,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responselen);

int sendDeviceStateResponse(int slotId, android::ClientId clientId,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responselen);

int setIndicationFilterResponse(int slotId, android::ClientId clientId,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responselen);

int setSimCardPowerResponse(int slotId, android::ClientId clientId,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responselen);

int startKeepaliveResponse(int slotId, android::ClientId clientId,
                           int responseType, int serial, RIL_Errno e,
                           void *response, size_t responselen);

int stopKeepaliveResponse(int slotId, android::ClientId clientId,
                          int responseType, int serial, RIL_Errno e,
                          void *response, size_t responselen);

int setSignalStrengthReportingCriteriaResponse(int slotId, android::ClientId clientId,
        int indicationType, int token, RIL_Errno err, void *response, size_t responseLen);

int setLinkCapacityReportingCriteriaResponse(int slotId, android::ClientId clientId,
        int indicationType, int token, RIL_Errno err, void *response, size_t responseLen);

void acknowledgeRequest(int slotId, int serial);

int radioStateChangedInd(int slotId,
                          int indicationType, int token, RIL_Errno e, void *response,
                          size_t responseLen);

int callStateChangedInd(int slotId, int indType, int token,
                        RIL_Errno e, void *response, size_t responselen);

int networkStateChangedInd(int slotId, int indType,
                                int token, RIL_Errno e, void *response, size_t responselen);

int newSmsInd(int slotId, int indicationType,
              int token, RIL_Errno e, void *response, size_t responselen);

int newSmsStatusReportInd(int slotId, int indicationType,
                          int token, RIL_Errno e, void *response, size_t responselen);

int newSmsOnSimInd(int slotId, int indicationType,
                   int token, RIL_Errno e, void *response, size_t responselen);

int onUssdInd(int slotId, int indicationType,
              int token, RIL_Errno e, void *response, size_t responselen);

int nitzTimeReceivedInd(int slotId, int indicationType,
                        int token, RIL_Errno e, void *response, size_t responselen);

int currentSignalStrengthInd(int slotId,
                             int indicationType, int token, RIL_Errno e,
                             void *response, size_t responselen);

int dataCallListChangedInd(int slotId, int indicationType,
                           int token, RIL_Errno e, void *response, size_t responselen);

int suppSvcNotifyInd(int slotId, int indicationType,
                     int token, RIL_Errno e, void *response, size_t responselen);

int stkSessionEndInd(int slotId, int indicationType,
                     int token, RIL_Errno e, void *response, size_t responselen);

int stkProactiveCommandInd(int slotId, int indicationType,
                           int token, RIL_Errno e, void *response, size_t responselen);

int stkEventNotifyInd(int slotId, int indicationType,
                      int token, RIL_Errno e, void *response, size_t responselen);

int stkCallSetupInd(int slotId, int indicationType,
                    int token, RIL_Errno e, void *response, size_t responselen);

int simSmsStorageFullInd(int slotId, int indicationType,
                         int token, RIL_Errno e, void *response, size_t responselen);

int simRefreshInd(int slotId, int indicationType,
                  int token, RIL_Errno e, void *response, size_t responselen);

int callRingInd(int slotId, int indicationType,
                int token, RIL_Errno e, void *response, size_t responselen);

int simStatusChangedInd(int slotId, int indicationType,
                        int token, RIL_Errno e, void *response, size_t responselen);

int cdmaNewSmsInd(int slotId, int indicationType,
                  int token, RIL_Errno e, void *response, size_t responselen);

int newBroadcastSmsInd(int slotId,
                       int indicationType, int token, RIL_Errno e, void *response,
                       size_t responselen);

int cdmaRuimSmsStorageFullInd(int slotId,
                              int indicationType, int token, RIL_Errno e, void *response,
                              size_t responselen);

int restrictedStateChangedInd(int slotId,
                              int indicationType, int token, RIL_Errno e, void *response,
                              size_t responselen);

int enterEmergencyCallbackModeInd(int slotId,
                                  int indicationType, int token, RIL_Errno e, void *response,
                                  size_t responselen);

int cdmaCallWaitingInd(int slotId,
                       int indicationType, int token, RIL_Errno e, void *response,
                       size_t responselen);

int cdmaOtaProvisionStatusInd(int slotId,
                              int indicationType, int token, RIL_Errno e, void *response,
                              size_t responselen);

int cdmaInfoRecInd(int slotId,
                   int indicationType, int token, RIL_Errno e, void *response,
                   size_t responselen);

int indicateRingbackToneInd(int slotId,
                            int indicationType, int token, RIL_Errno e, void *response,
                            size_t responselen);

int resendIncallMuteInd(int slotId,
                        int indicationType, int token, RIL_Errno e, void *response,
                        size_t responselen);

int cdmaSubscriptionSourceChangedInd(int slotId,
                                     int indicationType, int token, RIL_Errno e,
                                     void *response, size_t responselen);

int cdmaPrlChangedInd(int slotId,
                      int indicationType, int token, RIL_Errno e, void *response,
                      size_t responselen);

int exitEmergencyCallbackModeInd(int slotId,
                                 int indicationType, int token, RIL_Errno e, void *response,
                                 size_t responselen);

int rilConnectedInd(int slotId,
                    int indicationType, int token, RIL_Errno e, void *response,
                    size_t responselen);

int voiceRadioTechChangedInd(int slotId,
                             int indicationType, int token, RIL_Errno e, void *response,
                             size_t responselen);

int cellInfoListInd(int slotId,
                    int indicationType, int token, RIL_Errno e, void *response,
                    size_t responselen);

int imsNetworkStateChangedInd(int slotId,
                              int indicationType, int token, RIL_Errno e, void *response,
                              size_t responselen);

int subscriptionStatusChangedInd(int slotId,
                                 int indicationType, int token, RIL_Errno e, void *response,
                                 size_t responselen);

int srvccStateNotifyInd(int slotId,
                        int indicationType, int token, RIL_Errno e, void *response,
                        size_t responselen);

int hardwareConfigChangedInd(int slotId,
                             int indicationType, int token, RIL_Errno e, void *response,
                             size_t responselen);

int radioCapabilityIndicationInd(int slotId,
                                 int indicationType, int token, RIL_Errno e, void *response,
                                 size_t responselen);

int onSupplementaryServiceIndicationInd(int slotId,
                                        int indicationType, int token, RIL_Errno e,
                                        void *response, size_t responselen);

int stkCallControlAlphaNotifyInd(int slotId,
                                 int indicationType, int token, RIL_Errno e, void *response,
                                 size_t responselen);

int lceDataInd(int slotId,
               int indicationType, int token, RIL_Errno e, void *response,
               size_t responselen);

int pcoDataInd(int slotId,
               int indicationType, int token, RIL_Errno e, void *response,
               size_t responselen);

int modemResetInd(int slotId,
                  int indicationType, int token, RIL_Errno e, void *response,
                  size_t responselen);

int networkScanResultInd(int slotId,
                         int indicationType, int token, RIL_Errno e, void *response,
                         size_t responselen);

int keepaliveStatusInd(int slotId,
                       int indicationType, int token, RIL_Errno e, void *response,
                       size_t responselen);

int sendRequestRawResponse(int slotId, android::ClientId clientId,
                           int responseType, int serial, RIL_Errno e,
                           void *response, size_t responseLen);

int sendRequestStringsResponse(int slotId, android::ClientId clientId,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responseLen);

int setCarrierInfoForImsiEncryptionResponse(int slotId, android::ClientId clientId,
                                            int responseType, int serial, RIL_Errno e,
                                            void *response, size_t responseLen);

int carrierInfoForImsiEncryption(int slotId, android::ClientId clientId,
                        int responseType, int serial, RIL_Errno e,
                        void *response, size_t responseLen);

int currentPhysicalChannelConfigs(int slotId, int indicationType,
        int token, RIL_Errno err, void *response, size_t responseLen);

int currentLinkCapacityEstimate(int slotId, int indicationType,
        int token, RIL_Errno err, void *response, size_t responseLen);

pthread_rwlock_t * getRadioServiceRwlock(int slotId);

void lockRadioServiceRlock(pthread_rwlock_t *radioServiceRwlockPtr, int slotId);
void unlockRadioServiceRlock(pthread_rwlock_t *radioServiceRwlockPtr, int slotId);
void lockRadioServiceWlock(pthread_rwlock_t *radioServiceRwlockPtr, int slotId);
void unlockRadioServiceWlock(pthread_rwlock_t *radioServiceRwlockPtr, int slotId);
int isTidInRLock(int slotId, pthread_t tid);

void setNitzTimeReceived(int slotId, long timeReceived);

int emergencyNumberListInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response,
        size_t responselen);

int enableModemResponse(int slotId, android::ClientId clientId, int responseType,
        int serial, RIL_Errno err, void *response, size_t responseLen);

int getModemStackStatusResponse(int slotId, android::ClientId clientId,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen);

int setSystemSelectionChannelsResponse(int slotId, android::ClientId clientId,
                             int responseType, int serial, RIL_Errno e, void *response,
                             size_t responselen);
}   // namespace radio


namespace mtkRadioEx {
int setNetworkSelectionModeManualWithActResponse(int slotId, android::ClientId clientId,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen);
int getAvailableNetworksWithActResponse(int slotId, android::ClientId clientId,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responseLen);

int cancelAvailableNetworksResponse(int slotId, android::ClientId clientId,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen);

int cfgA2offsetResponse(int slotId, android::ClientId clientId __unused,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen);

int cfgB1offsetResponse(int slotId, android::ClientId clientId __unused,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen);

int enableSCGfailureResponse(int slotId, android::ClientId clientId __unused,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen);

int disableNRResponse(int slotId, android::ClientId clientId __unused,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen);

int setTxPowerResponse(int slotId, android::ClientId clientId __unused,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen);

int setSearchStoredFreqInfoResponse(int slotId, android::ClientId clientId __unused,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen);

int setSearchRatResponse(int slotId, android::ClientId clientId __unused,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen);

int setBgsrchDeltaSleepTimerResponse(int slotId, android::ClientId clientId __unused,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen);

int setClipResponse(int slotId, android::ClientId clientId, int responseType, int serial,
                    RIL_Errno e, void *response, size_t responseLen);

int getColpResponse(int slotId, android::ClientId clientId, int responseType, int serial,
                    RIL_Errno e, void *response, size_t responseLen);

int getColrResponse(int slotId, android::ClientId clientId, int responseType, int serial,
                    RIL_Errno e, void *response, size_t responseLen);

int sendCnapResponse(int slotId, android::ClientId clientId, int responseType, int serial,
                    RIL_Errno e, void *response, size_t responseLen);

int setColpResponse(int slotId, android::ClientId clientId, int responseType, int serial,
                    RIL_Errno e, void *response, size_t responseLen);

int setColrResponse(int slotId, android::ClientId clientId, int responseType, int serial,
                    RIL_Errno e, void *response, size_t responseLen);

int queryCallForwardInTimeSlotStatusResponse(int slotId, android::ClientId clientId,
                    int responseType, int serial, RIL_Errno e, void *response,
                    size_t responseLen);

int setCallForwardInTimeSlotResponse(int slotId, android::ClientId clientId,
                    int responseType, int serial, RIL_Errno e, void *response,
                    size_t responseLen);

int runGbaAuthenticationResponse(int slotId, android::ClientId clientId, int responseType,
                    int serial, RIL_Errno e, void *response, size_t responseLen);

// FastDormancy
int setFdModeResponse(int slotId, android::ClientId clientId, int responseType,
        int serial, RIL_Errno e, void *response, size_t responselen);

int hangupAllResponse(int slotId, android::ClientId clientId,
                      int responseType, int serial, RIL_Errno e,
                      void *response, size_t responselen);

int hangupWithReasonResponse(int slotId, android::ClientId clientId,
                      int responseType, int serial, RIL_Errno e,
                      void *response, size_t responselen);

int setCallIndicationResponse(int slotId, android::ClientId clientId,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responselen);

/// M: CC: Emergency mode for Fusion RIL
int setEccModeResponse(int slotId, android::ClientId clientId, int responseType,
                            int serial, RIL_Errno e, void *response,
                            size_t responselen);

/// M: CC: Vzw/CTVolte ECC for Fusion RIL
int eccPreferredRatResponse(int slotId, android::ClientId clientId, int responseType,
                            int serial, RIL_Errno e, void *response,
                            size_t responselen);

int setVoicePreferStatusResponse(int slotId, android::ClientId clientId, int responseType,
                            int serial, RIL_Errno e, void *response,
                            size_t responselen);

int setEccNumResponse(int slotId, android::ClientId clientId, int responseType,
                            int serial, RIL_Errno e, void *response,
                            size_t responselen);

int getEccNumResponse(int slotId, android::ClientId clientId, int responseType,
                            int serial, RIL_Errno e, void *response,
                            size_t responselen);

int cfuStatusNotifyInd(int slotId, int indicationType,
                              int token, RIL_Errno e, void *response, size_t responseLen);

/// M: CC: CDMA call accepted indication @{
int cdmaCallAcceptedInd(int slotId, int indicationType,
                    int token, RIL_Errno e, void *response, size_t responseLen);
/// @}

int eccNumIndication(int slotId, int indicationType,
                    int token, RIL_Errno e, void *response, size_t responseLen);

/// M: CC: call control part ([IMS] common flow) @{
int incomingCallIndicationInd(int slotId, int indicationType,
                    int token, RIL_Errno e, void *response, size_t responseLen);

int cipherIndicationInd(int slotId, int indicationType,
              int token, RIL_Errno e, void *response, size_t responseLen);

int suppSvcNotifyExInd(int slotId, int indicationType,
                     int token, RIL_Errno e, void *response, size_t responselen);

int crssNotifyInd(int slotId, int indicationType,
                  int token, RIL_Errno e, void *response, size_t responseLen);

int speechCodecInfoInd(int slotId, int indicationType,
                       int token, RIL_Errno e, void *response, size_t responseLen);

int atciInd(int slotId,
            int indicationType, int token, RIL_Errno e, void *response,
            size_t responselen);

int noEmergencyCallbackModeInd(int slotId,
                                 int indicationType, int token, RIL_Errno e, void *response,
                                 size_t responselen);

// M: [VzW] Data Framework @{
int pcoDataAfterAttachedInd(int slotId,
               int indicationType, int token, RIL_Errno e, void *response,
               size_t responselen);

int networkRejectCauseInd(int slotId,
               int indicationType, int token, RIL_Errno e, void *response,
               size_t responselen);
// M: [VzW] Data Framework @}

int sendAtciResponse(int slotId, android::ClientId clientId,
                     int responseType, int serial, RIL_Errno e,
                     void *response, size_t responseLen);

/// SUBSIDYLOCK
int sendSubsidyLockResponse(int slotId, android::ClientId clientId,
                     int responseType, int serial, RIL_Errno e,
                     void *response, size_t responseLen);

/// [IMS] Response ======================================================================

int imsEmergencyDialResponse(int slotId, android::ClientId clientId,
                             int responseType, int serial, RIL_Errno e, void *response,
                             size_t responseLen);

int imsDialResponse(int slotId, android::ClientId clientId,
                    int responseType,
                    int serial, RIL_Errno e, void *response, size_t responselen);

int imsVtDialResponse(int slotId, android::ClientId clientId,
                      int responseType, int token, RIL_Errno e, void *response,
                      size_t responselen);
int videoCallAcceptResponse(int slotId, android::ClientId clientId,
                            int responseType, int token, RIL_Errno e, void *response,
                            size_t responselen);

int eccRedialApproveResponse(int slotId, android::ClientId clientId,
                            int responseType, int token, RIL_Errno e, void *response,
                            size_t responselen);

int imsEctCommandResponse(int slotId, android::ClientId clientId,
                          int responseType, int token, RIL_Errno e,
                          void *response, size_t responselen);

int controlCallResponse(int slotId, android::ClientId clientId,
                 int responseType, int token, RIL_Errno e,
                 void *response, size_t responselen);

int imsDeregNotificationResponse(int slotId, android::ClientId clientId,
                                 int responseType, int token, RIL_Errno e, void *response,
                                 size_t responselen);

int setImsEnableResponse(int slotId, android::ClientId clientId,
                         int responseType, int token, RIL_Errno e, void *response,
                         size_t responselen);

int setImsVideoEnableResponse(int slotId, android::ClientId clientId,
                           int responseType, int token, RIL_Errno e, void *response,
                           size_t responselen);

int setImscfgResponse(int slotId, android::ClientId clientId,
                      int responseType, int token, RIL_Errno e, void *response,
                      size_t responselen);

int setModemImsCfgResponse(int slotId, android::ClientId clientId,
                      int responseType, int token, RIL_Errno e, void *response,
                      size_t responselen);

int getProvisionValueResponse(int slotId, android::ClientId clientId,
                              int responseType, int token, RIL_Errno e, void *response,
                              size_t responselen);

int setProvisionValueResponse(int slotId, android::ClientId clientId,
                              int responseType, int token, RIL_Errno e, void *response,
                              size_t responselen);

// IMS Config TelephonyWare START

int setImsCfgFeatureValueResponse(int slotId, android::ClientId clientId,
                                  int responseType, int token, RIL_Errno e, void *response,
                                  size_t responselen);

int getImsCfgFeatureValueResponse(int slotId, android::ClientId clientId,
                                  int responseType, int token, RIL_Errno e, void *response,
                                  size_t responselen);

int setImsCfgProvisionValueResponse(int slotId, android::ClientId clientId,
                                    int responseType, int token, RIL_Errno e, void *response,
                                    size_t responselen);

int getImsCfgProvisionValueResponse(int slotId, android::ClientId clientId,
                                    int responseType, int token, RIL_Errno e, void *response,
                                    size_t responselen);

int getImsCfgResourceCapValueResponse(int slotId, android::ClientId clientId,
                                      int responseType, int token, RIL_Errno e, void *response,
                                      size_t responselen);
// IMS Config TelephonyWare END

int controlImsConferenceCallMemberResponse(int slotId, android::ClientId clientId,
                                int responseType, int token, RIL_Errno e, void *response,
                                size_t responselen);

int setWfcProfileResponse(int slotId, android::ClientId clientId,

                          int responseType, int token, RIL_Errno e, void *response,
                          size_t responselen);

int conferenceDialResponse(int slotId, android::ClientId clientId,
                           int responseType, int token, RIL_Errno e, void *response,
                           size_t responselen);

int vtDialWithSipUriResponse(int slotId, android::ClientId clientId,
                             int responseType, int token, RIL_Errno e, void *response,
                             size_t responselen);

int dialWithSipUriResponse(int slotId, android::ClientId clientId,
                           int responseType, int token, RIL_Errno e, void *response,
                           size_t responselen);

int sendUssiResponse(int slotId, android::ClientId clientId,
                     int responseType, int token, RIL_Errno e, void *response,
                     size_t responselen);

int cancelUssiResponse(int slotId, android::ClientId clientId,
                              int responseType, int token, RIL_Errno e, void *response,
                              size_t responselen);

int getXcapStatusResponse(int slotId, android::ClientId clientId, int responseType, int serial,
                          RIL_Errno e, void *response, size_t responseLen);

int resetSuppServResponse(int slotId, android::ClientId clientId, int responseType, int serial,
                          RIL_Errno e, void *response, size_t responseLen);

int setupXcapUserAgentStringResponse(int slotId, android::ClientId clientId, int responseType,
                          int serial, RIL_Errno e, void *response, size_t responseLen);

int forceReleaseCallResponse(int slotId, android::ClientId clientId,
                        int responseType, int token, RIL_Errno e, void *response,
                        size_t responselen);

int setImsRtpReportResponse(int slotId, android::ClientId clientId,
                            int responseType, int token, RIL_Errno e, void *response,
                            size_t responselen);

int imsBearerStateConfirmResponse(int slotId, android::ClientId clientId,
                                         int responseType, int token, RIL_Errno e,
                                         void *response, size_t responselen);

int setImsBearerNotificationResponse(int slotId, android::ClientId clientId,
                                           int responseType, int token, RIL_Errno e,
                                           void *response, size_t responselen);

int pullCallResponse(int slotId, android::ClientId clientId,
                     int responseType, int serial, RIL_Errno e,
                     void *response, size_t responselen);
int setImsRegistrationReportResponse(int slotId, android::ClientId clientId,
                                     int indicationType, int token, RIL_Errno e,
                                     void *response, size_t responseLen);

int setVendorSettingResponse(int slotId, android::ClientId clientId, int responseType, int serial,
        RIL_Errno e, void *response, size_t responseLen);

/// [IMS] Response ======================================================================
pthread_rwlock_t * getMtkRadioServiceExRwlock(int slotId);

int getSmsRuimMemoryStatusResponse(int slotId, android::ClientId clientId,
                                   int responseType, int serial, RIL_Errno e,
                                   void *response, size_t responseLen);

int responseNetworkEventInd(int slotId,
                       int indicationType, int token, RIL_Errno e, void *response,
                       size_t responseLen);

int responsePsNetworkStateChangeInd(int slotId,
                                    int indicationType, int token, RIL_Errno e,
                                    void *response, size_t responseLen);

int networkBandInfoInd(int slotId, int indicationType, int token,
                       RIL_Errno e, void *response, size_t responseLen);

int responseModulationInfoInd(int slotId,
                       int indicationType, int token, RIL_Errno e, void *response,
                       size_t responseLen);

int responseCsNetworkStateChangeInd(int slotId,
                       int indicationType, int token, RIL_Errno e, void *response,
                       size_t responseLen);

int resetAttachApnInd(int slotId, int indicationType, int token,
                      RIL_Errno e, void *response, size_t responseLen);

int responseInvalidSimInd(int slotId,
                       int indicationType, int token, RIL_Errno e, void *response,
                       size_t responseLen);

int responseLteNetworkInfo(int slotId, int indicationType,
                       int token, RIL_Errno e, void *response, size_t responseLen);

int mdChangeApnInd(int slotId, int indicationType, int token,
                   RIL_Errno e, void *response, size_t responseLen);

// MTK-START: SIM
int getATRResponse(int slotId, android::ClientId clientId,
                 int responseType, int serial, RIL_Errno e,
                 void *response, size_t responseLen);

int getIccidResponse(int slotId, android::ClientId clientId, int responseType, int serial,
        RIL_Errno e, void *response, size_t responseLen);

int setSimPowerResponse(int slotId, android::ClientId clientId,
                 int responseType, int serial, RIL_Errno e,
                 void *response, size_t responseLen);

int onVirtualSimStatusChanged(int slotId,
        int indicationType, int token, RIL_Errno e, void *response,
        size_t responseLen);

int onImeiLock(int slotId,
        int indicationType, int token, RIL_Errno e, void *response,
        size_t responseLen);

int onImsiRefreshDone(int slotId,
        int indicationType, int token, RIL_Errno e, void *response,
        size_t responseLen);

int onCardDetectedInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response,
        size_t responseLen);
// MTK-END

/// [IMS] Indication ////////////////////////////////////////////////////////////////////
int callInfoIndicationInd(int slotId,
                          int indicationType, int token, RIL_Errno e, void *response,
                          size_t responseLen);

int econfResultIndicationInd(int slotId,
                             int indicationType, int token, RIL_Errno e,
                             void *response, size_t responseLen);

int sipCallProgressIndicatorInd(int slotId,
                                int indicationType, int token, RIL_Errno e,
                                void *response, size_t responseLen);

int callmodChangeIndicatorInd(int slotId,
                              int indicationType, int token, RIL_Errno e,
                              void *response, size_t responseLen);

int videoCapabilityIndicatorInd(int slotId,
                                int indicationType, int token, RIL_Errno e,
                                void *response, size_t responseLen);

int onUssiInd(int slotId,
              int indicationType, int token, RIL_Errno e, void *response,
             size_t responseLen);

int getProvisionDoneInd(int slotId,
                        int indicationType, int token, RIL_Errno e,
                        void *response, size_t responseLen);

int imsCfgDynamicImsSwitchCompleteInd(int slotId,
                                          int indicationType, int token, RIL_Errno e,
                                          void *response, size_t responseLen);

int imsCfgFeatureChangedInd(int slotId,
                            int indicationType, int token, RIL_Errno e,
                            void *response, size_t responseLen);

int imsCfgConfigChangedInd(int slotId,
                           int indicationType, int token, RIL_Errno e,
                           void *response, size_t responseLen);

int imsCfgConfigLoadedInd(int slotId,
                           int indicationType, int token, RIL_Errno e,
                           void *response, size_t responseLen);

int imsRtpInfoInd(int slotId,
                  int indicationType, int token, RIL_Errno e, void *response,
                  size_t responseLen);

int onXuiInd(int slotId,
             int indicationType, int token, RIL_Errno e, void *response,
             size_t responseLen);

int onVolteSubscription(int slotId, int indicationType, int token,
                        RIL_Errno e, void *response, size_t responseLen);

int imsEventPackageIndicationInd(int slotId,
                                 int indicationType, int token, RIL_Errno e,
                                 void *response, size_t responseLen);

int imsRegistrationInfoInd(int slotId,
                           int indicationType, int token, RIL_Errno e,
                           void *response, size_t responseLen);

int imsEnableDoneInd(int slotId,
                     int indicationType, int token, RIL_Errno e,
                     void *response, size_t responseLen);

int imsDisableDoneInd(int slotId,
                      int indicationType, int token, RIL_Errno e,
                      void *response, size_t responseLen);

int imsEnableStartInd(int slotId,
                      int indicationType, int token, RIL_Errno e,
                      void *response, size_t responseLen);

int imsDisableStartInd(int slotId,
                       int indicationType, int token, RIL_Errno e,
                       void *response, size_t responseLen);

int ectIndicationInd(int slotId,
                     int indicationType, int token, RIL_Errno e,
                     void *response, size_t responseLen);

int volteSettingInd(int slotId,
                    int indicationType, int token, RIL_Errno e,
                    void *response, size_t responseLen);

int imsBearerStateNotifyInd(int slotId,
                           int indicationType, int token, RIL_Errno e,
                           void *response, size_t responseLen);

int imsBearerInitInd(int slotId,
                     int indicationType, int token, RIL_Errno e,
                     void *response, size_t responseLen);

int imsDataInfoNotifyInd(int slotId,
                           int indicationType, int token, RIL_Errno e,
                           void *response, size_t responseLen);

int imsDeregDoneInd(int slotId,
                    int indicationType, int token, RIL_Errno e,
                    void *response, size_t responseLen);
int incomingCallIndicationInd(int slotId, int indicationType,
                    int token, RIL_Errno e, void *response, size_t responseLen);

int callAdditionalInfoInd(int slotId, int indicationType,
                    int token, RIL_Errno e, void *response, size_t responseLen);

int cipherIndicationInd(int slotId,
                               int indicationType, int token, RIL_Errno e,
                               void *response, size_t responseLen);

int multiImsCountInd(int slotId,
          int indicationType, int token, RIL_Errno e,
          void *response, size_t responseLen);

int imsSupportEccInd(int slotId,
                     int indicationType, int token, RIL_Errno e,
                     void *response, size_t responseLen);

int redialEmergencyIndication(int slotId, int indicationType,
                      int token, RIL_Errno e, void *response, size_t responseLen);

int emergencyBearerInfoInd(int slotId, int indicationType,
                      int token, RIL_Errno e, void *response, size_t responseLen);

int imsRadioInfoChangeInd(int slotId,
                     int indicationType, int token, RIL_Errno e,
                     void *response, size_t responseLen);
/// [IMS] Indication End

// MTK-START: SIM ME LOCK
int queryNetworkLockResponse(int slotId, android::ClientId clientId,
        int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen);

int setNetworkLockResponse(int slotId, android::ClientId clientId,
        int responseType, int serial, RIL_Errno e,
        void *response, size_t responseLen);

int supplyDepersonalizationResponse(int slotId, android::ClientId clientId, int responseType,
        int serial, RIL_Errno e, void *response, size_t responselen);
// MTK-END

int confSRVCCInd(int slotId, int indicationType, int token,
                 RIL_Errno e, void *response, size_t responseLen);

// World Phone part {
int setResumeRegistrationResponse(int slotId, android::ClientId clientId,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responseLen);

int modifyModemTypeResponse(int slotId, android::ClientId clientId,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responseLen);

int plmnChangedIndication(int slotId,
                      int indicationType, int token, RIL_Errno e, void *response,
                      size_t responselen);

int registrationSuspendedIndication(int slotId,
                      int indicationType, int token, RIL_Errno e, void *response,
                      size_t responselen);

int gmssRatChangedIndication(int slotId,
                      int indicationType, int token, RIL_Errno e, void *response,
                      size_t responselen);

int worldModeChangedIndication(int slotId,
                      int indicationType, int token, RIL_Errno e, void *response,
                      size_t responselen);
// World Phone part }
int esnMeidChangeInd(int slotId,
                     int indicationType, int token, RIL_Errno e, void *response,
                     size_t responseLen);

int restartRILDResponse(int slotId, android::ClientId clientId, int responseType, int serial,
        RIL_Errno e, void *response, size_t responseLen);

// SMS-START
int getSmsParametersResponse(int slotId, android::ClientId clientId, int responseType,
        int token, RIL_Errno e, void *response, size_t responselen);
int setSmsParametersResponse(int slotId, android::ClientId clientId, int responseType,
        int token, RIL_Errno e, void *response, size_t responselen);
int getSmsMemStatusResponse(int slotId, android::ClientId clientId, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen);
int meSmsStorageFullInd(int slotId, int indicationType,
        int token, RIL_Errno e, void *response, size_t responseLen);
int smsReadyInd(int slotId, int indicationType, int token,
        RIL_Errno e, void *response, size_t responseLen);
int setGsmBroadcastLangsResponse(int slotId, android::ClientId clientId,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen);
int getGsmBroadcastLangsResponse(int slotId, android::ClientId clientId,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen);
int getGsmBroadcastActivationRsp(int slotId, android::ClientId clientId,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen);
int setEtwsResponse(int slotId, android::ClientId clientId, int responseType,
        int token, RIL_Errno e, void *response, size_t responselen);
int removeCbMsgResponse(int slotId, android::ClientId clientId, int responseType, int serial,
        RIL_Errno e, void *response, size_t responseLen);
int newEtwsInd(int slotId, int indicationType, int token, RIL_Errno e,
        void *response, size_t responselen);
int sendImsSmsExResponse(int slotId, android::ClientId clientId, int responseType,
                      int serial, RIL_Errno e, void *response, size_t responselen);
int acknowledgeLastIncomingGsmSmsExResponse(int slotId, android::ClientId clientId,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen);
int newSmsStatusReportIndEx(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responselen);
int newSmsIndEx(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responselen);
int acknowledgeLastIncomingCdmaSmsExResponse(int slotId, android::ClientId clientId,
                                          int responseType, int serial, RIL_Errno e, void *response,
                                          size_t responselen);
int cdmaNewSmsIndEx(int slotId, int indicationType,
                  int token, RIL_Errno e, void *response, size_t responselen);

// SMS-END

/// M: eMBMS feature
int sendEmbmsAtCommandResponse(int slotId, android::ClientId clientId,
                            int responseType, int serial, RIL_Errno e,
                            void *response, size_t responseLen);
int embmsSessionStatusInd(int slotId, int indicationType, int token,
                      RIL_Errno e, void *response, size_t responselen);

int embmsAtInfoInd(int slotId, int indicationType, int token,
                   RIL_Errno e, void *response, size_t responselen);
/// M: eMBMS end

// / M: BIP, only to build pass {
int bipProactiveCommandInd(int slotId, int indicationType,
                           int token, RIL_Errno e, void *response, size_t responselen);
// / M: BIP }
// / M: OTASP, only to build pass {
int triggerOtaSPInd(int slotId, int indicationType,
                           int token, RIL_Errno e, void *response, size_t responselen);
// / M: OTASP }

// / M: STK, only to build pass {
int onStkMenuResetInd(int slotId, int indicationType,
                           int token, RIL_Errno e, void *response, size_t responselen);

int handleStkCallSetupRequestFromSimWithResCodeResponse(int slotId, android::ClientId clientId,
                                            int responseType, int serial, RIL_Errno e,
                                            void *response, size_t responselen);
// / M: STK }

int syncDataSettingsToMdResponse(int slotId, android::ClientId clientId,
                            int indicationType, int token, RIL_Errno e, void *response,
                            size_t responselen);

// M: Data Framework - Data Retry enhancement @{
int resetMdDataRetryCountResponse(int slotId, android::ClientId clientId,
                            int indicationType, int token, RIL_Errno e, void *response,
                            size_t responselen);

int onMdDataRetryCountReset(int slotId, int indicationType, int token,
                            RIL_Errno e, void *response, size_t responseLen);
// M: Data Framework - Data Retry enhancement @}

// M: Data Framework - CC 33 @{
int setRemoveRestrictEutranModeResponse(int slotId, android::ClientId clientId,
                            int indicationType, int token, RIL_Errno e, void *response,
                            size_t responselen);
int onRemoveRestrictEutran(int slotId, int indicationType, int token,
                            RIL_Errno e, void *response, size_t responseLen);
// M: Data Framework - CC 33 @}

// MTK-START: SIM HOT SWAP / SIM RECOVERY
int onSimPlugIn(int slotId,
        int indicationType, int token, RIL_Errno e, void *response,
        size_t responseLen);

int onSimPlugOut(int slotId,
        int indicationType, int token, RIL_Errno e, void *response,
        size_t responseLen);

int onSimMissing(int slotId,
        int indicationType, int token, RIL_Errno e, void *response,
        size_t responseLen);

int onSimRecovery(int slotId,
        int indicationType, int token, RIL_Errno e, void *response,
        size_t responseLen);
// MTK-END
// MTK-START: SIM POWER
int onSimPowerChangedInd(int slotId, int indicationType, int token,
        RIL_Errno e, void *response, size_t responseLen);
// MTK-END
// MTK-START: SIM COMMON SLOT
int onSimTrayPlugIn(int slotId,
        int indicationType, int token, RIL_Errno e, void *response,
        size_t responseLen);

int onSimCommonSlotNoChanged(int slotId,
        int indicationType, int token, RIL_Errno e, void *response,
        size_t responseLen);
// MTK-END

// M: [LTE][Low Power][UL traffic shaping] @{
int setLteAccessStratumReportResponse(int slotId, android::ClientId clientId,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responseLen);
int setLteUplinkDataTransferResponse(int slotId, android::ClientId clientId,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responseLen);
int onLteAccessStratumStateChanged(int slotId, int indicationType,
                        int token, RIL_Errno e, void *response, size_t responseLen);
// M: [LTE][Low Power][UL traffic shaping] @}

int getFemtocellListResponse(int slotId, android::ClientId clientId, int responseType, int serial,
                         RIL_Errno e, void *response, size_t responseLen);

int abortFemtocellListResponse(int slotId, android::ClientId clientId, int responseType,
                         int serial, RIL_Errno e, void *response, size_t responseLen);

int selectFemtocellResponse(int slotId, android::ClientId clientId, int responseType, int serial,
                         RIL_Errno e, void *response, size_t responseLen);

int queryFemtoCellSystemSelectionModeResponse(int slotId, android::ClientId clientId,
                         int responseType, int serial, RIL_Errno e, void *response,
                         size_t responseLen);

int setFemtoCellSystemSelectionModeResponse(int slotId, android::ClientId clientId,
                         int responseType, int serial, RIL_Errno e, void *response,
                         size_t responseLen);

int responseFemtocellInfo(int slotId, int responseType, int serial,
                         RIL_Errno e, void *response, size_t responseLen);

int networkInfoInd(int slotId, int indicationType, int token,
        RIL_Errno e, void *response, size_t responseLen);

int setRxTestConfigResponse(int slotId, android::ClientId clientId, int responseType, int serial,
        RIL_Errno e, void *response, size_t responseLen);
int getRxTestResultResponse(int slotId, android::ClientId clientId, int responseType, int serial,
        RIL_Errno e, void *response, size_t responseLen);

int getPOLCapabilityResponse(int slotId, android::ClientId clientId, int responseType, int serial,
                         RIL_Errno e, void *response, size_t responseLen);

int getCurrentPOLListResponse(int slotId, android::ClientId clientId, int responseType, int serial,
                         RIL_Errno e, void *response, size_t responseLen);

int setPOLEntryResponse(int slotId, android::ClientId clientId, int responseType, int serial,
                         RIL_Errno e, void *response, size_t responseLen);

// PHB START
int queryPhbStorageInfoResponse(int slotId, android::ClientId clientId,
                                int responseType, int serial, RIL_Errno e,
                                void *response, size_t responseLen);

int writePhbEntryResponse(int slotId, android::ClientId clientId,
                          int responseType, int serial, RIL_Errno e,
                          void *response, size_t responseLen);

int readPhbEntryResponse(int slotId, android::ClientId clientId,
                         int responseType, int serial, RIL_Errno e,
                         void *response, size_t responseLen);

int queryUPBCapabilityResponse(int slotId, android::ClientId clientId,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responseLen);

int editUPBEntryResponse(int slotId, android::ClientId clientId,
                         int responseType, int serial, RIL_Errno e,
                         void *response, size_t responseLen);

int deleteUPBEntryResponse(int slotId, android::ClientId clientId,
                           int responseType, int serial, RIL_Errno e,
                           void *response, size_t responseLen);

int readUPBGasListResponse(int slotId, android::ClientId clientId,
                           int responseType, int serial, RIL_Errno e,
                           void *response, size_t responseLen);

int readUPBGrpEntryResponse(int slotId, android::ClientId clientId,
                            int responseType, int serial, RIL_Errno e,
                            void *response, size_t responseLen);

int writeUPBGrpEntryResponse(int slotId, android::ClientId clientId,
                             int responseType, int serial, RIL_Errno e,
                             void *response, size_t responseLen);

int getPhoneBookStringsLengthResponse(int slotId, android::ClientId clientId,
                                      int responseType, int serial, RIL_Errno e,
                                      void *response, size_t responseLen);

int getPhoneBookMemStorageResponse(int slotId, android::ClientId clientId,
                                   int responseType, int serial, RIL_Errno e,
                                   void *response, size_t responseLen);

int setPhoneBookMemStorageResponse(int slotId, android::ClientId clientId,
                                    int responseType, int serial, RIL_Errno e,
                                    void *response, size_t responseLen);

int readPhoneBookEntryExtResponse(int slotId, android::ClientId clientId,
                                  int responseType, int serial, RIL_Errno e,
                                  void *response, size_t responseLen);

int writePhoneBookEntryExtResponse(int slotId, android::ClientId clientId,
                                   int responseType, int serial, RIL_Errno e,
                                   void *response, size_t responseLen);

int queryUPBAvailableResponse(int slotId, android::ClientId clientId,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responseLen);

int readUPBEmailEntryResponse(int slotId, android::ClientId clientId,
                              int responseType, int serial, RIL_Errno e,
                              void *response, size_t responseLen);

int readUPBSneEntryResponse(int slotId, android::ClientId clientId,
                            int responseType, int serial, RIL_Errno e,
                            void *response, size_t responseLen);

int readUPBAnrEntryResponse(int slotId, android::ClientId clientId,
                            int responseType, int serial, RIL_Errno e,
                            void *response, size_t responseLen);

int readUPBAasListResponse(int slotId, android::ClientId clientId,
                           int responseType, int serial, RIL_Errno e,
                           void *response, size_t responseLen);

int phbReadyNotificationInd(int slotId,
                            int indicationType, int token, RIL_Errno e, void *response,
                            size_t responselen);

int setPhonebookReadyResponse(int slotId, android::ClientId clientId,
                               int responseType, int serial, RIL_Errno e,
                               void *response, size_t responseLen);
// PHB END

int setTrmResponse(int slotId, android::ClientId clientId, int responseType, int serial,
        RIL_Errno e, void *response, size_t responseLen);

// APC
int setApcModeResponse(int slotId, android::ClientId clientId, int indicationType, int token,
        RIL_Errno e, void *response, size_t responseLen);

int getApcInfoResponse(int slotId, android::ClientId clientId, int indicationType, int token,
        RIL_Errno e, void *response, size_t responseLen);

int onPseudoCellInfoInd(int slotId, int indicationType, int token,
        RIL_Errno e, void *response, size_t responseLen);

/// M: [Network][C2K] Sprint roaming control @{
int setRoamingEnableResponse(int slotId, android::ClientId clientId, int responseType, int serial,
        RIL_Errno e, void *response, size_t responseLen);

int getRoamingEnableResponse(int slotId, android::ClientId clientId, int responseType, int serial,
        RIL_Errno e, void *response, size_t responseLen);
/// @}

int setLteReleaseVersionResponse(int slotId, android::ClientId clientId, int responseType,
        int serial, RIL_Errno e, void *response, size_t responselen);

int getLteReleaseVersionResponse(int slotId, android::ClientId clientId, int responseType,
        int serial, RIL_Errno e, void *response, size_t responselen);

int onMccMncChangedInd(int slotId, int indicationType, int token,
        RIL_Errno err, void *response, size_t responseLen);

int dataAllowedNotificationInd(int slotId, int indicationType,
        int token, RIL_Errno e, void *response, size_t responseLen);

int triggerModeSwitchByEccResponse(int slotId, android::ClientId clientId, int responseType,
        int serial, RIL_Errno e, void *response, size_t responseLen);

int setModemPowerResponse(int slotId, android::ClientId clientId, int responseType, int serial,
        RIL_Errno e, void *response, size_t responseLen);

// External SIM [Start]
int vsimNotificationResponse(int slotId, android::ClientId clientId, int responseType, int serial,
        RIL_Errno e, void *response, size_t responseLen);

int vsimOperationResponse(int slotId, android::ClientId clientId, int responseType, int serial,
        RIL_Errno e, void *response, size_t responseLen);

int onVsimEventIndication(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responselen);

// External SIM [End]
int setVoiceDomainPreferenceResponse(int slotId, android::ClientId clientId,
                            int responseType, int token, RIL_Errno e, void *response,
                            size_t responselen);

int getVoiceDomainPreferenceResponse(int slotId, android::ClientId clientId,
                            int responseType, int token, RIL_Errno e, void *response,
                            size_t responselen);
/// Ims Data Framework {@
int dedicatedBearerActivationInd(int slotId, int indicationType,
        int serial, RIL_Errno e, void *response, size_t responseLen);
int dedicatedBearerModificationInd(int slotId, int indicationType,
        int serial, RIL_Errno e, void *response, size_t responseLen);
int dedicatedBearerDeactivationInd(int slotId, int indicationType,
        int serial, RIL_Errno e, void *response, size_t responseLen);
/// @}

int setWifiEnabledResponse(int slotId, android::ClientId clientId, int responseType, int serial,
        RIL_Errno err, void *response, size_t responseLen);

int setWifiAssociatedResponse(int slotId, android::ClientId clientId, int responseType, int serial,
        RIL_Errno err, void *response, size_t responseLen);

int setWfcConfigResponse(int slotId, android::ClientId clientId,
        int responseType, int serial, RIL_Errno err, void *response, size_t responseLen);

int setWifiSignalLevelResponse(int slotId, android::ClientId clientId, int responseType,
        int serial, RIL_Errno err, void *response, size_t responseLen);

int setWifiIpAddressResponse(int slotId, android::ClientId clientId, int responseType, int serial,
        RIL_Errno err, void *response, size_t responseLen);

int setLocationInfoResponse(int slotId, android::ClientId clientId, int responseType, int serial,
        RIL_Errno err, void *response, size_t responseLen);

int setEmergencyAddressIdResponse(int slotId, android::ClientId clientId, int responseType,
        int serial, RIL_Errno err, void *response, size_t responseLen);

int onWifiMonitoringThreshouldChanged(int slotId, int indicationType,
        int token, RIL_Errno err, void *response, size_t responseLen);

int onWifiPdnActivate(int slotId, int indicationType, int token,
        RIL_Errno err, void *response, size_t responseLen);

int onWfcPdnError(int slotId, int indicationType, int token,
        RIL_Errno err, void *response, size_t responseLen);

int onPdnHandover(int slotId, int indicationType, int token,
        RIL_Errno err, void *response, size_t responseLen);

int onWifiRoveout(int slotId, int indicationType, int token,
        RIL_Errno err, void *response, size_t responseLen);

int onLocationRequest(int slotId, int indicationType, int token,
        RIL_Errno err, void *response, size_t responseLen);

int onWfcPdnStateChanged(int slotId, int indicationType, int token,
        RIL_Errno err, void *response, size_t responseLen);

int setServiceStateToModemResponse(int slotId, android::ClientId clientId, int responseType,
        int serial, RIL_Errno err, void *response, size_t responseLen);

int setNattKeepAliveStatusResponse(int slotId, android::ClientId clientId, int responseType,
        int serial, RIL_Errno err, void *response, size_t responseLen);

int onNattKeepAliveChanged(int slotId, int indicationType, int token,
        RIL_Errno err, void *response, size_t responseLen);

int setWifiPingResultResponse(int slotId, android::ClientId clientId, int responseType, int serial,
        RIL_Errno err, void *response, size_t responseLen);

int onWifiPingRequest(int slotId, int indicationType, int token,
        RIL_Errno err, void *response, size_t responseLen);

int querySsacStatusResponse(int slotId, android::ClientId clientId, int responseType, int serial,
        RIL_Errno err, void *response, size_t responseLen);

int onSsacStatus(int slotId, int indicationType, int token,
        RIL_Errno err, void *response, size_t responseLen);

int onTxPowerIndication(int slotId, int indicationType, int token,
        RIL_Errno err, void *response, size_t responseLen);

int onTxPowerStatusIndication(int slotId, int indicationType,
        int token, RIL_Errno err, void *response, size_t responseLen);

int getCellInfoListResponse_1_2(int slotId, android::ClientId clientId, int indicationType,
        int token, RIL_Errno err, void *response, size_t responseLen);

int getIccCardStatusResponse_1_2(int slotId, android::ClientId clientId, int indicationType,
        int token, RIL_Errno err, void *response, size_t responseLen);

int getCurrentCallsResponse_1_2(int slotId, android::ClientId clientId, int indicationType,
        int token, RIL_Errno err, void *response, size_t responseLen);

int getSignalStrengthResponse_1_2(int slotId, android::ClientId clientId, int indicationType,
        int token, RIL_Errno err, void *response, size_t responseLen);

int getSignalStrengthWithWcdmaEcioResponse(int slotId, android::ClientId clientId,
        int indicationType, int token, RIL_Errno err, void *response, size_t responseLen);

int getVoiceRegistrationStateResponse_1_2(int slotId, android::ClientId clientId,
        int indicationType, int token, RIL_Errno err, void *response, size_t responseLen);

int getDataRegistrationStateResponse_1_2(int slotId, android::ClientId clientId,
        int indicationType, int token,RIL_Errno err, void *response, size_t responseLen);

int networkScanResult_1_2(int slotId, android::ClientId clientId, int indicationType, int token,
        RIL_Errno err, void *response, size_t responseLen);

int cellInfoList_1_2(int slotId, android::ClientId clientId, int indicationType, int token,
        RIL_Errno err, void *response, size_t responseLen);

int currentSignalStrength_1_2(int slotId, android::ClientId clientId, int indicationType,
        int token, RIL_Errno err, void *response, size_t responseLen);

int currentSignalStrengthWithWcdmaEcioInd(int slotId,
        int indicationType, int token, RIL_Errno err, void *response, size_t responseLen);

int onWifiPdnOOS(int slotId, int indicationType, int token,
        RIL_Errno err, void *response, size_t responseLen);

int onWifiLock(int slotId, int indicationType,
        int token, RIL_Errno err, void *response, size_t responselen);

int onImsConferenceInfoIndication(int slotId, int indicationType,
        int token, RIL_Errno err, void *response, size_t responseLen);

int onLteMessageWaitingIndication(int slotId, int indicationType,
        int token, RIL_Errno err, void *response, size_t responseLen);

///M:Dialog Event Package Info @{
int imsDialogIndicationInd(int slotId, int indicationType, int token,
        RIL_Errno e, void *response, size_t responseLen);
///@}

int dataConnectionAttachResponse(int slotId, android::ClientId clientId, int responseType,
        int serial, RIL_Errno e, void *response, size_t responselen);

int dataConnectionDetachResponse(int slotId, android::ClientId clientId, int responseType,
        int serial, RIL_Errno e, void *response, size_t responselen);

int resetAllConnectionsResponse(int slotId, android::ClientId clientId, int responseType,
        int serial, RIL_Errno e, void *response, size_t responselen);

int setTxPowerStatusResponse(int slotId, android::ClientId clientId,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen);

int setSuppServPropertyResponse(int slotId, android::ClientId clientId,
    int responseType, int token, RIL_Errno e, void *response, size_t responselen);

int onDsbpStateChanged(int slotId,
    int responseType, int token, RIL_Errno e, void *response, size_t responselen);

// MTK-START: SIM SLOT LOCK
int smlSlotLockInfoChangedInd(int slotId, int indicationType,
        int token, RIL_Errno e, void *response, size_t responseLen);
int supplyDeviceNetworkDepersonalizationResponse(int slotId, android::ClientId clientId,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen);
// MTK-END
// SIM RSU
int onRsuSimLockEvent(int slotId, int indicationType, int token, RIL_Errno e, void *response,
        size_t responseLen);

int getPlmnNameFromSE13TableResponse(int slotId, android::ClientId clientId,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responselen);

int enableCAPlusBandWidthFilterResponse(int slotId, android::ClientId clientId,
                              int responseType, int serial, RIL_Errno e, void *response,
                              size_t responselen);

///M: Notify ePDG screen state
int notifyEPDGScreenStateResponse(int slotId, android::ClientId clientId, int responseType,
        int serial, RIL_Errno err, void *response, size_t responseLen);

int oemHookRawInd(int slotId,
                  int indicationType, int token, RIL_Errno e, void *response,
                  size_t responselen);
int sendVopsIndication(int slotId, int indicationType, int token, RIL_Errno e,
        void *response, size_t responselen);
int queryVopsStatusResponse(int slotId, android::ClientId clientId,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen);

// M: RTT @{
int setRttModeResponse(int slotId, android::ClientId clientId,
        int responseType, int serial, RIL_Errno e,
        void *response, size_t responselen);

int sendRttModifyRequestResponse(int slotId, android::ClientId clientId,
        int responseType, int serial, RIL_Errno e,
        void *response, size_t responselen);

int sendRttTextResponse(int slotId, android::ClientId clientId,
        int responseType, int serial, RIL_Errno e,
        void *response, size_t responselen);

int rttModifyRequestResponseResponse(int slotId, android::ClientId clientId,
        int responseType, int serial, RIL_Errno e,
        void *response, size_t responselen);

int toggleRttAudioIndicationResponse(int slotId, android::ClientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responselen);

int rttModifyResponseInd(int slotId, int indicationType, int token, RIL_Errno e,
                         void *response, size_t responselen);

int rttTextReceiveInd(int slotId, int indicationType, int token, RIL_Errno e,
                      void *response, size_t responselen);

int rttCapabilityIndicationInd(int slotId, int indicationType, int token, RIL_Errno e,
                               void *response, size_t responselen);

int rttModifyRequestReceiveInd(int slotId, int indicationType, int token, RIL_Errno e,
                               void *response, size_t responselen);

int audioIndicationInd(int slotId, int indicationType, int token, RIL_Errno e,
                               void *response, size_t responselen);
// @}

int setSipHeaderResponse(int slotId, android::ClientId clientId,
    int responseType, int token, RIL_Errno e, void *response, size_t responselen);

int setSipHeaderReportResponse(int slotId, android::ClientId clientId,
    int responseType, int token, RIL_Errno e, void *response, size_t responselen);

int setImsCallModeResponse(int slotId, android::ClientId clientId,
    int responseType, int token, RIL_Errno e, void *response, size_t responselen);

int sipHeaderReportInd(int slotId,
    int indicationType, int token, RIL_Errno e, void *response, size_t responselen);

int callRatIndication(int slotId,
    int indicationType, int token, RIL_Errno e, void *response, size_t responselen);
// M: GWSD @{
int setGwsdModeResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen);
int setCallValidTimerResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen);
int setIgnoreSameNumberIntervalResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen);
int setKeepAliveByPDCPCtrlPDUResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen);
int setKeepAliveByIpDataResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen);
// @}

int qualifiedNetworkTypesChangedInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response,
        size_t responselen);
int sipRegInfoInd(int slotId, int indicationType, int token, RIL_Errno e,
        void *response, size_t responseLen);
int imsRegistrationStateInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen);
int eiregDataInd(int slotId,
        int indicationType, int token, RIL_Errno e, void *response, size_t responseLen);
int enableDsdaIndicationResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen);
int getDsdaStatusResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen);
int onDsdaChangedInd(int slotId,
    int responseType, int token, RIL_Errno e, void *response, size_t responselen);
int activateUiccCardRsp(int slotId, android::ClientId clientId, int responseType, int serial,
        RIL_Errno e, void *response, size_t responseLen);
int deactivateUiccCardRsp(int slotId, android::ClientId clientId, int responseType, int serial,
        RIL_Errno e, void *response, size_t responseLen);
int getCurrentUiccCardProvisioningStatusRsp(int slotId, android::ClientId clientId,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen);
int radioStateChangedToImsInd(int slotId, int indicationType, int token, RIL_Errno e,
        void *response, size_t responseLen);
// MUSE WFC requirement
int registerCellQltyReportResponse(int slotId, android::ClientId clientId, int responseType, int serial,
        RIL_Errno e, void *response, size_t responselen);

// MUSE WFC requirement
int onCellularQualityChangedInd(int slotId, int indicationType, int token,
        RIL_Errno err, void *response, size_t responseLen);

int getSuggestedPlmnListResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen);

int routeCertificateResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen);
int routeAuthMessageResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen);
int enableCapabilityResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen);
int abortCertificateResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen);
int mobileDataUsageInd(int slotId, int indicationType, int token, RIL_Errno e,
        void *response, size_t responseLen);
int onNwLimitInd(int slotId, int indicationType, int token,
        RIL_Errno err, void *response, size_t responseLen);
int sendSarIndicatorResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen);
int setCallAdditionalInfoResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen);
//RSU @{
int sendRsuRequestResponse(int slotId, android::ClientId clientId __unused,
        int responseType, int serial, RIL_Errno e, void *response, size_t responseLen);
int onRsuEvent(int slotId, int indicationType, int token, RIL_Errno e,
        void *response, size_t responseLen);
// @}
}   // namespace mtkRadioEx
#endif  // RIL_SERVICE_H
