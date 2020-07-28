/*
 * Copyright 2018 The Android Open Source Project
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

#define LOG_TAG "beanpodkeymaster"

#include <cutils/log.h>
#include <keymaster/android_keymaster_messages.h>
#include <keymaster/keymaster_configuration.h>
#include <BeanpodKeymaster.h>
#include <ipc/beanpod_keymaster_ipc.h>
#include <unistd.h>



namespace keymaster {

int BeanpodKeymaster::Initialize() {
    int err;

    ALOGI("keymaster connect start");
    err = bp_keymaster_connect();
    if (err) {
        ALOGE("Failed to connect to beanpod keymaster %d", err);
        return err;
    }

    ALOGI("keymaster connect end");

    ConfigureRequest req;
    req.os_version = GetOsVersion();
    req.os_patchlevel = GetOsPatchlevel();

    ConfigureResponse rsp;
    Configure(req, &rsp);

    if (rsp.error != KM_ERROR_OK) {
        ALOGE("Failed to configure keymaster %d", rsp.error);
        return -1;
    }

    return 0;
}

BeanpodKeymaster::BeanpodKeymaster() {}

BeanpodKeymaster::~BeanpodKeymaster() {
    bp_keymaster_disconnect();
}

static void ForwardCommand(enum keymaster_command command, const Serializable& req,
                           KeymasterResponse* rsp) {
    keymaster_error_t err;
    err = bp_keymaster_send(command, req, rsp);
    if (err != KM_ERROR_OK) {
        ALOGE("Failed to send cmd %d err: %d", command, err);
        rsp->error = err;
    }
}

static void ForwardCommandNoRequest(enum keymaster_command command, KeymasterResponse* rsp) {
    keymaster_error_t err;
    err = bp_keymaster_send_no_request(command, rsp);
    if (err != KM_ERROR_OK) {
        ALOGE("BeanpodKeymaster Failed to send cmd %d err: %d", command, err);
        rsp->error = err;
    }
}

void BeanpodKeymaster::GetVersion(const GetVersionRequest& request, GetVersionResponse* response) {
    ForwardCommand(KM_GET_VERSION, request, response);
}

void BeanpodKeymaster::SupportedAlgorithms(const SupportedAlgorithmsRequest& request,
                                          SupportedAlgorithmsResponse* response) {
    ForwardCommand(KM_GET_SUPPORTED_ALGORITHMS, request, response);
}

void BeanpodKeymaster::SupportedBlockModes(const SupportedBlockModesRequest& request,
                                          SupportedBlockModesResponse* response) {
    ForwardCommand(KM_GET_SUPPORTED_BLOCK_MODES, request, response);
}

void BeanpodKeymaster::SupportedPaddingModes(const SupportedPaddingModesRequest& request,
                                            SupportedPaddingModesResponse* response) {
    ForwardCommand(KM_GET_SUPPORTED_PADDING_MODES, request, response);
}

void BeanpodKeymaster::SupportedDigests(const SupportedDigestsRequest& request,
                                       SupportedDigestsResponse* response) {
    ForwardCommand(KM_GET_SUPPORTED_DIGESTS, request, response);
}

void BeanpodKeymaster::SupportedImportFormats(const SupportedImportFormatsRequest& request,
                                             SupportedImportFormatsResponse* response) {
    ForwardCommand(KM_GET_SUPPORTED_IMPORT_FORMATS, request, response);
}

void BeanpodKeymaster::SupportedExportFormats(const SupportedExportFormatsRequest& request,
                                             SupportedExportFormatsResponse* response) {
    ForwardCommand(KM_GET_SUPPORTED_EXPORT_FORMATS, request, response);
}

void BeanpodKeymaster::AddRngEntropy(const AddEntropyRequest& request,
                                    AddEntropyResponse* response) {
    ForwardCommand(KM_ADD_RNG_ENTROPY, request, response);
}

void BeanpodKeymaster::Configure(const ConfigureRequest& request, ConfigureResponse* response) {
    ForwardCommand(KM_CONFIGURE, request, response);
}

void BeanpodKeymaster::GenerateKey(const GenerateKeyRequest& request,
                                  GenerateKeyResponse* response) {
    ALOGI("BeanpodKeymaster begin generate key");
    GenerateKeyRequest datedRequest(request.message_version);
    datedRequest.key_description = request.key_description;

    if (!request.key_description.Contains(TAG_CREATION_DATETIME)) {
        datedRequest.key_description.push_back(TAG_CREATION_DATETIME, java_time(time(NULL)));
    }

    ForwardCommand(KM_GENERATE_KEY, datedRequest, response);
}

void BeanpodKeymaster::GetKeyCharacteristics(const GetKeyCharacteristicsRequest& request,
                                            GetKeyCharacteristicsResponse* response) {
    ForwardCommand(KM_GET_KEY_CHARACTERISTICS, request, response);
}

void BeanpodKeymaster::ImportKey(const ImportKeyRequest& request, ImportKeyResponse* response) {
    ForwardCommand(KM_IMPORT_KEY, request, response);
}

void BeanpodKeymaster::ImportWrappedKey(const ImportWrappedKeyRequest& request,
                                       ImportWrappedKeyResponse* response) {
    ForwardCommand(KM_IMPORT_WRAPPED_KEY, request, response);
}

void BeanpodKeymaster::ExportKey(const ExportKeyRequest& request, ExportKeyResponse* response) {
    ForwardCommand(KM_EXPORT_KEY, request, response);
}

void BeanpodKeymaster::AttestKey(const AttestKeyRequest& request, AttestKeyResponse* response) {
    ForwardCommand(KM_ATTEST_KEY, request, response);
}

void BeanpodKeymaster::UpgradeKey(const UpgradeKeyRequest& request, UpgradeKeyResponse* response) {
    ForwardCommand(KM_UPGRADE_KEY, request, response);
}

void BeanpodKeymaster::DeleteKey(const DeleteKeyRequest& request, DeleteKeyResponse* response) {
    ForwardCommand(KM_DELETE_KEY, request, response);
}

void BeanpodKeymaster::DeleteAllKeys(const DeleteAllKeysRequest& request,
                                    DeleteAllKeysResponse* response) {
    ForwardCommand(KM_DELETE_ALL_KEYS, request, response);
}

void BeanpodKeymaster::BeginOperation(const BeginOperationRequest& request,
                                     BeginOperationResponse* response) {
    ALOGI("Enter BeanpodKeymaster BeginOperation");
    ForwardCommand(KM_BEGIN_OPERATION, request, response);
    ALOGI("Exit BeanpodKeymaster BeginOperation");
}

void BeanpodKeymaster::UpdateOperation(const UpdateOperationRequest& request,
                                      UpdateOperationResponse* response) {
    ALOGI("Enter BeanpodKeymaster UpdateOperation");
    ForwardCommand(KM_UPDATE_OPERATION, request, response);
    ALOGI("Exit BeanpodKeymaster UpdateOperation");
}

void BeanpodKeymaster::FinishOperation(const FinishOperationRequest& request,
                                      FinishOperationResponse* response) {
    ALOGI("Enter BeanpodKeymaster FinishOperation");
    ForwardCommand(KM_FINISH_OPERATION, request, response);
    ALOGI("Exit BeanpodKeymaster FinishOperation");
}

void BeanpodKeymaster::AbortOperation(const AbortOperationRequest& request,
                                     AbortOperationResponse* response) {
    ALOGI("Enter BeanpodKeymaster AbortOperation");
    ForwardCommand(KM_ABORT_OPERATION, request, response);
    ALOGI("Exit BeanpodKeymaster AbortOperation");
}

/* Methods for Keymaster 4.0 functionality -- not yet implemented */
void BeanpodKeymaster::GetHmacSharingParameters(GetHmacSharingParametersResponse* response) {
    ForwardCommandNoRequest(KM_GET_HMAC_SHARING_PARAMETERS, response);
}

void BeanpodKeymaster::ComputeSharedHmac(
        const ComputeSharedHmacRequest& request, ComputeSharedHmacResponse* response) {
    ForwardCommand(KM_COMPUTE_SHARED_HMAC, request, response);
}

void BeanpodKeymaster::VerifyAuthorization(
        const VerifyAuthorizationRequest& request, VerifyAuthorizationResponse* response) {
    ForwardCommand(KM_VERIFY_AUTHORIZATION, request, response);
}

}  // namespace keymaster
