#include <DrmRequestType.h>

using namespace android;

const String8 DrmRequestType::KEY_ACTION("action");
const String8 DrmRequestType::KEY_DATA("data");
const String8 DrmRequestType::KEY_DATA_EXTRA_1("data_extra_1");
const String8 DrmRequestType::KEY_DATA_EXTRA_2("data_extra_2");
const String8 DrmRequestType::KEY_DM_FD("dmFd");
const String8 DrmRequestType::KEY_DCF_FD("dcfFd");

const String8 DrmRequestType::KEY_FILEDESCRIPTOR("FileDescriptorKey");
const String8 DrmRequestType::KEY_DATA_1("data_1");
const String8 DrmRequestType::KEY_DATA_1_EXTRA("data_1_extra");
const String8 DrmRequestType::KEY_DATA_2("data_2");
const String8 DrmRequestType::KEY_DATA_2_EXTRA("data_2_extra");

const String8 DrmRequestType::ACTION_INSTALL_DRM_MSG("installDrmMsg");
const String8 DrmRequestType::ACTION_INSTALL_DRM_TO_DEVICE("installDrmToDevice");
const String8 DrmRequestType::ACTION_CONSUME_RIGHTS("consumeRights");

const String8 DrmRequestType::ACTION_UPDATE_OFFSET("updateOffset");
const String8 DrmRequestType::ACTION_CHECK_SECURE_TIME("checkSecureTime");
const String8 DrmRequestType::ACTION_SAVE_SECURE_TIME("saveSecureTime");
const String8 DrmRequestType::ACTION_LOAD_DEVICE_ID("loadDeviceId");
const String8 DrmRequestType::ACTION_SAVE_DEVICE_ID("saveDeviceId");

const String8 DrmRequestType::ACTION_MARK_AS_CONSUME_IN_APP_CLIENT("markAsConsumeInAppClient");
const String8 DrmRequestType::ACTION_CHECK_CONSUME_IN_APP_CLIENT("checkConsumeInAppClient");

const String8 DrmRequestType::ACTION_SHOW_DRM_DIALOG_IF_NEED("showDrmDialogIfNeed");

const String8 DrmRequestType::ACTION_CHECK_RIGHTS_STATUS_BY_FD("checkRightsStatusByFd");

const String8 DrmRequestType::RESULT_SUCCESS("success");
const String8 DrmRequestType::RESULT_FAILURE("failure");

const String8 DrmRequestType::ACTION_CTA5_ENCRYPT("CTA5Encrypt");
const String8 DrmRequestType::ACTION_CTA5_DECRYPT("CTA5Decrypt");
const String8 DrmRequestType::ACTION_CTA5_SETKEY("CTA5SetKey");
const String8 DrmRequestType::ACTION_CTA5_CHANGEPASSWORD("CTA5ChangePassword");
const String8 DrmRequestType::ACTION_CTA5_GETPROGESS("CTA5GetProgress");
const String8 DrmRequestType::ACTION_CTA5_CANCEL("CTA5Cancel");
const String8 DrmRequestType::ACTION_CTA5_ISCTAFILE("CTA5IsCtaFile");
const String8 DrmRequestType::ACTION_CTA5_GETTOKEN("CTA5Gettoken");
const String8 DrmRequestType::ACTION_CTA5_CHECKTOKEN("CTA5Checktoken");
const String8 DrmRequestType::ACTION_CTA5_CLEARTOKEN("CTA5Cleartoken");

const String8 DrmRequestType::KEY_CTA5_CLEAR_FD("CTA5clearFd");
const String8 DrmRequestType::KEY_CTA5_CIPHER_FD("CTA5cipherFd");
const String8 DrmRequestType::KEY_CTA5_FD("CTA5Fd");
const String8 DrmRequestType::KEY_CTA5_KEY("CTA5key");
const String8 DrmRequestType::KEY_CTA5_OLDKEY("CTA5oldKey");
const String8 DrmRequestType::KEY_CTA5_NEWKEY("CTA5newKey");
const String8 DrmRequestType::KEY_CTA5_RAW_MIME("CTA5rawMime");
const String8 DrmRequestType::KEY_CTA5_TOKEN("CTA5Token");
const String8 DrmRequestType::KEY_CTA5_FILEPATH("CTA5FilePath");
