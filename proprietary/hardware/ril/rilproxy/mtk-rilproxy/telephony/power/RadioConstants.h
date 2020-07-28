/*
 * Copyright (C) 2014 The Android Open Source Project
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


#ifndef __RADIO_CONSTANT_H
#define __RADIO_CONSTANT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RIL_CAPABILITY_NONE,
    RIL_CAPABILITY_GSM_ONLY,
    RIL_CAPABILITY_CDMA_ONLY,
    RIL_CAPABILITY_CDMA_ON_LTE
} RIL_RADIO_CAPABILITY;

typedef enum {
    SUCCESS,
    SUCCESS_WITHOUT_SETTING_RADIO,
    ERROR
} SuggestRadioResult;

#define CAPABILITY_HAS_CDMA(a) ((a == RIL_CAPABILITY_CDMA_ONLY || a == RIL_CAPABILITY_CDMA_ON_LTE) ? 1 : 0)
#define CAPABILITY_HAS_GSM(a) ((a == RIL_CAPABILITY_GSM_ONLY || a == RIL_CAPABILITY_CDMA_ON_LTE) ? 1 : 0)
#define CAPABILITY_OFF(a) ((a == RIL_CAPABILITY_NONE) ? 1 : 0)

static const char* emdstatusProperty = "vendor.ril.cdma.emdstatus.send";

#ifdef __cplusplus
}
#endif
#endif /*RADIO_CONSTANT_H*/
