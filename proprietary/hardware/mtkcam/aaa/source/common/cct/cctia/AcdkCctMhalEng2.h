#ifndef _MTK_ACDK_CAMERA_DEVICE_H_
#define _MTK_ACDK_CAMERA_DEVICE_H_

#include <string.h>
using std::string;
using android::sp;
#include <android/hardware/camera/device/1.0/ICameraDeviceCallback.h>
using android::hardware::camera::device::V1_0::ICameraDeviceCallback;

void create_CameDev (string instanceName, int instanceID);
void close_CameDev ();

//void open_CameDev (ICameraDeviceCallback *cam_dev_cb);
void open_CameDev (const sp<ICameraDeviceCallback>& cam_dev_cb, camera_notify_callback notify_cb, camera_data_callback data_cb, camera_data_timestamp_callback data_cb_timestamp, camera_request_memory get_memory);

void setParameters_CameDev (string param);
void getParameters_CameDev (string &param);
void enableMsgType_CameDev (uint32_t msgType);

void startPreview_CameDev ();
void stopPreview_CameDev ();
void takePicture_CameDev ();

#endif
