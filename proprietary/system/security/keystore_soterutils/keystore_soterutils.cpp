/*
 * Copyright (c) 2016 MediaTek Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "key_store_service.h"

using android::String8;
using android::KeymasterArguments;

static uint8_t *ask_blob = NULL;

std::vector<keymaster_key_param_t> acquireSoterParams(KeyStore *mKeyStore,
        const KeymasterArguments& params, const uid_t uid) {
    std::vector<keymaster_key_param_t> opParams; // Do the copy from params manually
    keymaster_key_param_t ask = {KM_TAG_INVALID, {0}};

    ALOGE("acquireSoterParams() ++ ask_blob %p", ask_blob);

    for (std::vector<const keymaster_key_param_t>::iterator param = params.params.cbegin();
            param != params.params.cend(); ++param) {
        if ((*param).tag == KM_TAG_SOTER_AUTO_SIGNED_COMMON_KEY_WHEN_GET_PUBLIC_KEY) {
            Blob keyBlob;
            String8 keyName(reinterpret_cast<const char*>((*param).blob.data), (*param).blob.data_length);

            ResponseCode rc = mKeyStore->getKeyForName(&keyBlob, keyName, uid,
                    TYPE_KEYMASTER_10);
            if (rc != ::NO_ERROR) {
                ALOGE("Reading %s failed (%d)", keyName.string(), rc);
                continue;
            }

            ask.tag = KM_TAG_SOTER_AUTO_SIGNED_COMMON_KEY_WHEN_GET_PUBLIC_KEY;
            if (ask_blob == NULL) {
                ask_blob = (uint8_t*) malloc(keyBlob.getLength());
            } else {
                ask_blob = (uint8_t*) realloc(ask_blob, keyBlob.getLength());
            }
            memset(ask_blob, 0, keyBlob.getLength());
            ask.blob.data = ask_blob;
            memcpy((void*)(ask.blob.data), keyBlob.getValue(), keyBlob.getLength());
            ask.blob.data_length = keyBlob.getLength();

            opParams.push_back(ask);
        } else {
            opParams.push_back(*param);
        }
    }

    ALOGE("acquireSoterParams() -- ask_blob %p", ask_blob);
    return opParams;
}

void releaseSoterParams(std::vector<keymaster_key_param_t>) {
    ALOGE("releaseSoterParams() ++ ask_blob %p", ask_blob);
    if (ask_blob != NULL) {
        free(ask_blob);
        ask_blob = NULL;
    }
    ALOGE("releaseSoterParams() -- ask_blob %p", ask_blob);
}
