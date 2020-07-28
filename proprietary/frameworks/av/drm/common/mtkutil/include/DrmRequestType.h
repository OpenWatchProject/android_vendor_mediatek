#ifndef __DRM_REQUEST_TYPE_H__
#define __DRM_REQUEST_TYPE_H__

#include <utils/String8.h>

namespace android
{
class DrmRequestType
{
private:
    DrmRequestType(){};

public:
    static const int TYPE_SET_DRM_INFO = 2021;
    static const int TYPE_GET_DRM_INFO = 2022;

    static const String8 KEY_ACTION;
    static const String8 KEY_DATA;
    static const String8 KEY_DATA_EXTRA_1;
    static const String8 KEY_DATA_EXTRA_2;
    static const String8 KEY_DM_FD;
    static const String8 KEY_DCF_FD;

    static const String8 KEY_FILEDESCRIPTOR;
    static const String8 KEY_DATA_1;
    static const String8 KEY_DATA_1_EXTRA;
    static const String8 KEY_DATA_2;
    static const String8 KEY_DATA_2_EXTRA;

    static const String8 ACTION_INSTALL_DRM_MSG;
    static const String8 ACTION_INSTALL_DRM_TO_DEVICE;
    static const String8 ACTION_CONSUME_RIGHTS;

    static const String8 ACTION_LOAD_DEVICE_ID;
    static const String8 ACTION_SAVE_DEVICE_ID;

    static const String8 ACTION_UPDATE_OFFSET;
    static const String8 ACTION_CHECK_SECURE_TIME;
    static const String8 ACTION_SAVE_SECURE_TIME;

    static const String8 ACTION_MARK_AS_CONSUME_IN_APP_CLIENT;
    static const String8 ACTION_CHECK_CONSUME_IN_APP_CLIENT;

    static const String8 ACTION_CHECK_RIGHTS_STATUS_BY_FD;

    static const String8 ACTION_SHOW_DRM_DIALOG_IF_NEED;

    static const String8 RESULT_SUCCESS;
    static const String8 RESULT_FAILURE;

    //For CTA5 feature
    static const String8 ACTION_CTA5_ENCRYPT;
    static const String8 ACTION_CTA5_DECRYPT;
    static const String8 ACTION_CTA5_SETKEY;
    static const String8 ACTION_CTA5_CHANGEPASSWORD;
    static const String8 ACTION_CTA5_GETPROGESS;
    static const String8 ACTION_CTA5_CANCEL;
    static const String8 ACTION_CTA5_ISCTAFILE;
    static const String8 ACTION_CTA5_GETTOKEN;
    static const String8 ACTION_CTA5_CHECKTOKEN;
    static const String8 ACTION_CTA5_CLEARTOKEN;

    static const String8 KEY_CTA5_CLEAR_FD;
    static const String8 KEY_CTA5_CIPHER_FD;
    static const String8 KEY_CTA5_FD;
    static const String8 KEY_CTA5_KEY;
    static const String8 KEY_CTA5_OLDKEY;
    static const String8 KEY_CTA5_NEWKEY;
    static const String8 KEY_CTA5_RAW_MIME;
    static const String8 KEY_CTA5_TOKEN;
    static const String8 KEY_CTA5_FILEPATH;
};

}
#endif /* __DRM_INFO_TYPE_H__ */
