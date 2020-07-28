/*
 * Copyright (C) 2006 The Android Open Source Project
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

#define LOG_NDEBUG 0
#define LOG_TAG "DrmDialog"
#include <utils/Log.h>

#include <unistd.h>
#include <binder/IBinder.h>
#include <binder/IServiceManager.h>
#include <binder/Parcel.h>
#include <utils/String8.h>

#include "ActivityManager.h"

namespace android {

const uint32_t START_ACTIVITY_TRANSACTION = IBinder::FIRST_CALL_TRANSACTION + 5;
const uint32_t BROADCAST_INTENT_TRANSACTION = IBinder::FIRST_CALL_TRANSACTION + 10;
const uint32_t START_SERVICE_TRANSACTION = IBinder::FIRST_CALL_TRANSACTION + 29;

#define ACTION_DRM_DIALOG   "mediatek.intent.action.DRM_DIALOG"
#define ACTION_DRM_RESCAN   "mediatek.intent.action.DRM_RESCAN"

// send broadcast to DrmDialogReceiver to show refer dialog
bool sendBroadcastMessage(String16 cid, String16 filePath)
{
    ALOGD("sendBroadcastMessage: cid = %s", String8(cid).string());
    sp<IServiceManager> sm = defaultServiceManager();
    sp<IBinder> am = sm->getService(String16("activity"));
    if (am != NULL) {
        Parcel data, reply;
        data.writeInterfaceToken(String16("android.app.IActivityManager"));
        data.writeStrongBinder(NULL);
        data.writeInt32(1);
        // intent begin
        data.writeString16(String16(ACTION_DRM_RESCAN)); // action
        data.writeInt32(0); // Uri data type(0, 1, 2): 0 is string uri
        data.writeString16(NULL, 0); // type, it's mimetype, most is null
        data.writeInt32(0); // flags
        data.writeString16(NULL, 0); // package name
        data.writeString16(NULL, 0); // component name
        data.writeInt32(0); // source bound - size
        data.writeInt32(0); // categories - size
        data.writeInt32(0); // selector - size
        data.writeInt32(0); // clipData - size
        data.writeInt32(0); // contentUserHint: 0 -> UserHandle.USER_OWNER
        // extras start(bundle)
        int lengthPos = data.dataPosition();
        data.writeInt32(-1); // dummy, will hold length
        data.writeInt32(0x4C444E42); // BUNDLE_MAGIC: 'B' 'N' 'D' 'L';
        int startPos = data.dataPosition();
        data.writeInt32(1); // map size and write map data closely
        data.writeString16(String16("drm_cid")); // drm_cid key
        data.writeInt32(0); // value type: 0 -> VAL_STRING
        data.writeString16(cid); // drm_cid value
        // Add more extra above this line and update map size above
        int endPos = data.dataPosition();
        // Backpatch length
        data.setDataPosition(lengthPos);
        int length = endPos - startPos;
        data.writeInt32(length);
        data.setDataPosition(endPos);
        // extras end(bundle)
        // intent end
        data.writeString16(NULL, 0); // resolvedType
        data.writeStrongBinder(NULL); // resultTo
        data.writeInt32(0); // resultCode
        data.writeString16(NULL, 0); // resultData
        data.writeInt32(-1); // resultExtras
        data.writeString16(NULL, 0); // permissions
        data.writeInt32(0); // appOp
        data.writeInt32(-1); //option : -1 if unused
        data.writeInt32(0); // serialized: != 0 -> ordered
        data.writeInt32(0); // sticky
        data.writeInt32(0); // userId: 0-> UserHandle.USER_OWNER

        // BROADCAST_INTENT_TRANSACTION
        status_t ret = am->transact(BROADCAST_INTENT_TRANSACTION, data, &reply);
        ALOGD("sendBroadcastMessage: ret = %d", ret);
        if (ret == NO_ERROR) {
            int exceptionCode = reply.readExceptionCode();
            if (exceptionCode) {
                ALOGE("sendBroadcastMessage(%s) caught exception %d", cid.string(), exceptionCode);
                return false;
            }
        } else {
            return false;
        }
    } else {
        ALOGE("sendBroadcastMessage: couldn't find activity service!");
        return false;
    }
    return true;
}

bool startDrmDialogService(String16 drm_cid, String16 rightsIssuer,
        String16 drmMethod, int rightsStatus, String16 filePath)
{
    ALOGD("startDrmDialogService: drm_cid = %s, rightsIssuer = %s, drmMethod = %s, rightsStatus = %d, path=%s",
            String8(drm_cid).string(), String8(rightsIssuer).string(), String8(drmMethod).string(),
            rightsStatus, filePath.string());
    sp<IServiceManager> sm = defaultServiceManager();
    ALOGD("startDrmDialogService: sm.get():%p", sm.get());
    sp<IBinder> am = sm->getService(String16("activity"));
    if (am != NULL) {
        Parcel data, reply;
        data.writeInterfaceToken(String16("android.app.IActivityManager"));
        data.writeStrongBinder(NULL);
                data.writeInt32(1);
        // intent begin
        data.writeString16(String16(ACTION_DRM_DIALOG)); // action
        data.writeInt32(0); // Uri data type(0, 1, 2): 0 -> null uri, no need data
        //data.writeString16(NULL, 0); // Uri data
        data.writeString16(NULL, 0); // type, it's mimetype, most is null
        data.writeInt32(0x14000000); // flags
        data.writeString16(NULL, 0); // package name
        data.writeString16(NULL, 0); // component name
        data.writeInt32(0); // source bound - size
        data.writeInt32(0); // categories - size
        data.writeInt32(0); // selector - size
        data.writeInt32(0); // clipData - size
        data.writeInt32(0); // contentUserHint: 0 -> UserHandle.USER_OWNER

        // extras start(bundle)
        int lengthPos = data.dataPosition();
        data.writeInt32(-1); // dummy, will hold length
        data.writeInt32(0x4C444E42); // BUNDLE_MAGIC: 'B' 'N' 'D' 'L';
        int startPos = data.dataPosition();
        data.writeInt32(5);  // map size and write map data closely
        data.writeString16(String16("drm_cid")); // drm_cid key
        data.writeInt32(0); // value type: 0 -> VAL_STRING
        data.writeString16(drm_cid); // drm_cid value
        data.writeString16(String16("drm_rights_issuer")); // rights issuer key
        data.writeInt32(0); // value type: 0 -> VAL_STRING
        data.writeString16(rightsIssuer); // rights issuer value
        data.writeString16(String16("drm_method")); // drm method key
        data.writeInt32(0); // value type: 0 -> VAL_STRING
        data.writeString16(drmMethod); // drm method value
        data.writeString16(String16("drm_rights_status")); // rights status key
        data.writeInt32(1); // value type: 1 -> VAL_INTEGER
        data.writeInt32(rightsStatus); // rights status value
        data.writeString16(String16("path"));  // file path
        data.writeInt32(0);  // value type: 0 -> VAL_STRING
        data.writeString16(filePath);
        // Can add more extra values above this line only //Update map size above
        int endPos = data.dataPosition();
        // Backpatch length
        data.setDataPosition(lengthPos);
        int length = endPos - startPos;
        data.writeInt32(length);
        data.setDataPosition(endPos);
        // extras end(bundle)
        // intent end
        data.writeString16(NULL, 0); // resolvedType
        data.writeString16(String16("com.mediatek.providers.drm"), 0); //calling package name here
        data.writeInt32(0); //service userid user_all

        // START_ACTIVITY_TRANSACTION
        status_t ret = am->transact(START_SERVICE_TRANSACTION, data, &reply);
        ALOGD("startDrmDialogService: ret = %d", ret);
        if (ret == NO_ERROR) {
            int exceptionCode = reply.readExceptionCode();
            if (exceptionCode) {
                ALOGE("startDrmDialogService(%s) caught exception %d\n",
                        drm_cid.string(), exceptionCode);
                return false;
            }
        } else {
            return false;
        }
    } else {
        ALOGE("startDrmDialogService: couldn't find activity service!\n");
        return false;
    }
    return true;
}

} /* namespace android */
