/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

//#define LOG_DEBUG 0
#define LOG_TAG "Utils_MTK"

#include <utils/Log.h>
//#include <media/stagefright/MetaData.h>
//#include <media/stagefright/foundation/AMessage.h>
#include <utils/String8.h>
#include <media/stagefright/MediaCodecList.h>

#include "Utils_MTK.h"

namespace android {


AString getProcessNameByPid(int pid) {
      // get the proc name by pid, via reading /proc/<pid>/cmdline file
      char path[30];
      snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
      AString result;
      FILE* fp = fopen(path, "r");
      if (NULL != fp) {
          char proc_name[100];
          bzero(proc_name, sizeof(proc_name));
          if (NULL != fgets(proc_name, sizeof(proc_name), fp)) {
              result.append(proc_name);
              ALOGD("getProcessNameByPid [%s]->[%s]", path, proc_name);
          } else {
              ALOGW("getProcessNameByPid failed to get processName [path:%s], reason [%s]",
                      path, strerror(errno));
          }
          fclose(fp);
      } else {
          ALOGW("getProcessNameByPid : failed to open [%s] , reason [%s]",
                  path, strerror(errno));
      }
      return result;
}

bool isMtkMp3Music(uint32_t app_pid) {
    bool isMtkSupportApp = false;
    AString proName = getProcessNameByPid((int)app_pid);
    const char *processName = proName.c_str();
    if(processName != NULL &&
        ((!strncasecmp(processName, "com.android.music", 17)) ||
        (!strncasecmp(processName, "com.google.android.music", 24)))) {
        ALOGV("it is mtk music.");
        isMtkSupportApp = true;
    }
    return isMtkSupportApp;
}

bool isFindMtkMp3Codec(const AString &mime, bool encoder) {
    Vector<AString> matchingCodecs;
    MediaCodecList::findMatchingCodecs(
                mime.c_str(),
                encoder,
                0,
                &matchingCodecs);

    if (matchingCodecs.size() == 0) {
        return false;
    }
    AString componentName = matchingCodecs[0];
    if (!strcmp(componentName.c_str(), "OMX.MTK.AUDIO.DECODER.MP3")) {
        ALOGV("Find mtk mp3 codec.");
        return true;
    }
    return false;
}


}


