/*
 * Copyright (c) 2015-2016 MICROTRUST Incorporated
 * All rights reserved
 *
 * This file and software is confidential and proprietary to MICROTRUST Inc.
 * Unauthorized copying of this file and software is strictly prohibited.
 * You MUST NOT disclose this file and software unless you get a license
 * agreement from MICROTRUST Incorporated.
 */
#include <UtClientApiDaemonProxy.h>
#include <hwbinder/Parcel.h>
#include <vendor/microtrust/hardware/capi/2.0/IClientApiDevice.h>

namespace vendor
{
namespace microtrust
{
namespace hardware
{
namespace capi
{
namespace V2_0
{
namespace implementation
{

using ::vendor::microtrust::hardware::capi::V2_0::ErrorCode;
using ::vendor::microtrust::hardware::capi::V2_0::IClientApiDevice;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_memory;
using ::android::sp;
using ::android::hardware::Parcel;
using ::android::status_t;
using ::android::OK;

class ClientApiDevice : public IClientApiDevice
{
  private:
    android::sp<android::UtClientApiDaemonProxy> proxy;
    status_t onTransact(uint32_t code, const Parcel &data, Parcel *reply, uint32_t flags);

  public:
    ClientApiDevice(android::sp<android::UtClientApiDaemonProxy> _proxy);
    ~ClientApiDevice();
    Return<ErrorCode> capi_transact(uint32_t msgid, const hidl_memory &data) override;
    Return<void> createMmapBuffer(int32_t fd, uint64_t memory_size, createMmapBuffer_cb _hidl_cb) override;
    Return<void> registerSharedFileDescriptor(int32_t fd, int32_t reg_fd, registerSharedFileDescriptor_cb _hidl_cb) override;
};

} // namespace implementation
} // namespace V2_0
} // namespace capi
} // namespace hardware
} // namespace microtrust
} // namespace vendor
