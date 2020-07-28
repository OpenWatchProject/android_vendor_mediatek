/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2019. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#define LOG_TAG "MtkUtils"
#include "tensorflow/lite/c/c_api_internal.h"
#include "tensorflow/lite/mtk/mtk_utils.h"
#include "tensorflow/lite/mtk/mtk_minimal_logging.h"

#include <cstdlib>
#include <string>
#include <sstream>
#include <vector>

#ifdef __ANDROID__
#include <sys/system_properties.h>
#endif  // __ANDROID__

#define MAX_OEM_OP_STRING_LEN 100

namespace tflite {
namespace mtk {

static bool sOptions[26] = {false};
static bool sReadProperty = false;

static std::vector<std::string> Split(const std::string& s, char delimiter) {
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(s);

  while (std::getline(tokenStream, token, delimiter)) {
    tokens.push_back(token);
  }

  return tokens;
}

static void InitOptions(const char* key) {
  char var[PROP_VALUE_MAX];
  int length = __system_property_get(key, var);

  if (length == 0) {
    TFLITE_MTK_LOG_INFO("Empty NeuroPilot feature property");
    return;
  }

  const std::string vOptionSetting = std::string(var);
  std::vector<std::string> elements = Split(vOptionSetting, ',');

  for (const auto& elem : elements) {
    if (elem.length() != 1) {
      continue;
    }

    char c = elem.at(0);
    int index = tolower(c) - 'a';

    if (index < 0 || index > 25) {
      TFLITE_MTK_LOG_ERROR("Invalid index: %d", index);
      continue;
    }

    TFLITE_MTK_LOG_INFO("Property index: %d", index);
    sOptions[index] = true;
  }
}

static bool OverwriteOptions() {
  char var[PROP_VALUE_MAX];

  if (__system_property_get("debug.nn.mtk_nn.option", var) != 0) {
    TFLITE_MTK_LOG_INFO("For debug: feature support by debug proerty: %s", var);

    // Reset old data first.
    for (int i = 0; i <= 25; i++) {
      sOptions[i] = false;
    }

    InitOptions("debug.nn.mtk_nn.option");
    return true;
  }

  return false;
}

/*
 * The format of data will be:
 *  -------------------------------------------------------------------------------
 *  | 1 byte typeLen  | N bytes type     | 4 bytes dataLen  | N bytes data        |
 *  -------------------------------------------------------------------------------
 */
static int EncodeOperandValue(OemOperandValue* operand, uint8_t* output) {
  size_t currPos = 0;

  // 1 byte for typeLen, 4 bytes for bufferLen
  if (output == nullptr) {
    return -1;
  }

  // Set length of type
  *output = operand->typeLen;
  currPos += sizeof(uint8_t);

  // Copy type to output
  memcpy(output + currPos, operand->type, operand->typeLen);
  currPos += operand->typeLen;

  // Set the length of buffer
  uint32_t* dataLen = reinterpret_cast<uint32_t*>(&output[currPos]);
  *dataLen = operand->dataLen;
  currPos += sizeof(uint32_t);

  // Copy  operand value to output
  memcpy(&output[currPos], operand->data, operand->dataLen);

  return 0;
}

static int32_t GetAndroidSdkVersion() {
#ifdef __ANDROID__
  const char* debugSdkProp = "debug.mtk.build.version.sdk";
  const char* sdkProp = "ro.build.version.sdk";
  char sdkVersion[PROP_VALUE_MAX];
  int length = __system_property_get(debugSdkProp, sdkVersion);
  if (length != 0) {
    return 29;
  }
  length = __system_property_get(sdkProp, sdkVersion);
  if (length != 0) {
    for (int i = 0; i < length; ++i) {
      int digit = sdkVersion[i] - '0';
      if (digit < 0 || digit > 9) {
        // Non-numeric SDK version, assume it's higher then expected;
        return 0xFFFF;
      }
    }
    return atoi(sdkVersion);
  }
  TFLITE_MTK_LOG_ERROR("No %s prop", sdkProp);
  return 0;
#endif  // __ANDROID__
  return 0;
}

bool PropertyGetBool(const char *key, bool default_value) {
#ifdef __ANDROID__
  if (!key) {
    return default_value;
  }

  bool result = default_value;
  char buf[PROP_VALUE_MAX] = {'\0'};

  int len = __system_property_get(key, buf);
  if (len == 1) {
    char ch = buf[0];
    if (ch == '0' || ch == 'n') {
      result = false;
    } else if (ch == '1' || ch == 'y') {
      result = true;
    }
  } else if (len > 1) {
     if (!strcmp(buf, "no") || !strcmp(buf, "false") ||
         !strcmp(buf, "off")) {
       result = false;
     } else if (!strcmp(buf, "yes") || !strcmp(buf, "true") ||
                !strcmp(buf, "on")) {
       result = true;
     }
  }
  return result;
#else
  return false;
#endif  // __ANDROID__
}

int PropertyGetInt(const char *key, int default_value) {
#ifdef __ANDROID__
  char buf[PROP_VALUE_MAX] = {'\0'};
  int length = __system_property_get(key, buf);
  if (length != 0) {
    for (int i = 0; i < length; ++i) {
      int digit = buf[i] - '0';
      if (digit < 0 || digit > 9) {
        return default_value;
      }
    }
    return atoi(buf);
  }
#endif  // __ANDROID__
  return default_value;
}

int PropertyGet(const char* key, char** value) {
  int len = 0;
#ifdef __ANDROID__
  *value = reinterpret_cast<char *>(calloc(PROP_VALUE_MAX, sizeof(char)));
  if (*value == nullptr) {
    return len;
  }
  len = __system_property_get(key, *value);
#endif  // __ANDROID__
  return len;
}

int32_t GetAndroidSdkVersionCached() {
  static int32_t androidSdkVersion = GetAndroidSdkVersion();
  return androidSdkVersion;
}

int32_t Hash(const char* str) {
  int32_t hash = 5381;
  int c;
  size_t count = 0;
  size_t len = strlen(str);
  TFLITE_MTK_LOG_INFO("Hash string: %s", str);

  while (count < len) {
    c = *(str + count);
    hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    count++;
  }

  return hash;
}

int DecodeOperandValue(uint8_t* input, OemOperandValue* operand) {
  size_t currPos = 0;

  // Get Type length
  operand->typeLen = *input;
  currPos += sizeof(uint8_t);

  // Get Types
  operand->type = reinterpret_cast<uint8_t*>(malloc(operand->typeLen));
  if (operand->type == nullptr) {
    return -1;
  }
  memcpy(operand->type, &input[currPos], operand->typeLen);
  currPos += operand->typeLen;

  // Get Buffer length
  uint32_t* dataLen = reinterpret_cast<uint32_t*>(&input[currPos]);
  operand->dataLen = *dataLen;
  currPos += sizeof(uint32_t);

  // Get Buffer
  operand->data = reinterpret_cast<uint8_t*>(malloc(operand->dataLen));
  if (operand->data == nullptr) {
    return -1;
  }
  memcpy(operand->data, &input[currPos], operand->dataLen);

  return 0;
}

size_t PackOemScalarString(const char* str, uint8_t** out_buffer) {
  if (str == nullptr) {
    return 0;
  }
  size_t out_len = 0;
  uint8_t type[] = {'s', 't', 'r', 'i', 'n', 'g'};
  OemOperandValue operand_value;

  operand_value.typeLen = sizeof(type);
  operand_value.type = type;
  operand_value.dataLen = strlen(str);
  if (operand_value.dataLen > MAX_OEM_OP_STRING_LEN) {
    return 0;
  }
  operand_value.data =
    reinterpret_cast<uint8_t*>(malloc(operand_value.dataLen));
  if (operand_value.data == nullptr) {
    return 0;
  }
  memcpy(operand_value.data, str, operand_value.dataLen);

  out_len = operand_value.typeLen +
            operand_value.dataLen +
            (sizeof(size_t) * 2);
  *out_buffer = reinterpret_cast<uint8_t*>(calloc(out_len, sizeof(uint8_t)));
  if (*out_buffer == nullptr) {
    free(operand_value.data);
    return 0;
  }
  EncodeOperandValue(&operand_value, *out_buffer);
  free(operand_value.data);

  TFLITE_MTK_LOG_INFO("PackOemScalarString: %s, buffer size:%zu", str, out_len);
  return out_len;
}

bool IsFeatureSupported(int feature) {
  if (feature < 0 || feature > 25) {
    TFLITE_MTK_LOG_ERROR("Unknown feature: %d", feature);
    return false;
  }

  if (OverwriteOptions()) {
    return sOptions[feature];
  }

  if (!sReadProperty) {
    InitOptions("ro.vendor.mtk_nn.option");
    sReadProperty = true;
  }

  return sOptions[feature];
}
int64_t GetMtkNnApiDelegateFlag(void) {
  static bool enabled =
    tflite::mtk::PropertyGetBool("debug.mtk_tflite.allow_dynamic_tensor",
                                false);
  return (enabled ?
      kTfLiteDelegateFlagsAllowDynamicTensors : kTfLiteDelegateFlagsNone);
}

}  // namespace mtk
}  // namespace tflite
