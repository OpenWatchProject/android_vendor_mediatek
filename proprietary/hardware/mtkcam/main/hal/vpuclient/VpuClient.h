#ifndef VENDOR_MEDIATEK_HARDWARE_CAMERA_VPUCLIENT_V2_0_MTKCALLBACKCLIENT_H
#define VENDOR_MEDIATEK_HARDWARE_CAMERA_VPUCLIENT_V2_0_MTKCALLBACKCLIENT_H

#include <vendor/mediatek/hardware/camera/vpuclient/2.0/IMtkVpuClient.h>
#include <vendor/mediatek/hardware/camera/vpuclient/2.0/IMtkVpuBuffer.h>

#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#include <mutex>
#include <vector>
#include <mutex>
#include<string>
#include<map>
#include<list>

#include <utils/KeyedVector.h>

#include "HandleImporter.h"

#include "vpuhidl_drv.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace camera {
namespace vpuclient {
namespace V2_0 {
namespace implementation {

using namespace android;

using ::android::hardware::camera::common::V1_0::Status;
using ::android::hardware::camera::common::V1_0::helper::HandleImporter;
using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::vendor::mediatek::hardware::camera::vpuclient::V2_0::IMtkVpuClient;
using ::vendor::mediatek::hardware::camera::vpuclient::V2_0::IMtkVpuBuffer;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_handle;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct MtkVpuClient : public IMtkVpuClient {
    // Methods from ::vendor::mediatek::hardware::camera::vpuclient::V2_0::IMtkVpuClient follow.
    Return<Status> existVpuClient  () override;
    Return<Status> createVpuClient (int64_t timestamp) override;

    
    Return<Status> createinstanceVpuClient(const hidl_vec<int64_t>& vpuStreamIns, const VpuBuffer_t& vpuBuffer, const hidl_handle& dstHidlShareFd) override;
    Return<Status> createInstance(const hidl_string& callername) override;
    Return<Status> destroyInstance(const hidl_string& callername) override;
    /*
        *    Return<Status> getAlgo(const hidl_string& stringname) override;
        */
    Return<uint64_t> getAlgo(const hidl_string& stringname) override;
    /*
        *   Return<Status> acquire() override;
        */
    Return<uint64_t> acquire(uint64_t algo) override;

    /*
	  * Return<Status> addBuffer(const VpuBuffer_t& vpuBuffer, const hidl_handle& hidlShareFd) override;
	  * Return<Status> setProperty(int32_t sett, int32_t id) override;
        *  Return<Status> setPropertyMem(const hidl_memory& propertyMem) override;
	 *  Return<Status> setExtraParam(const VpuExtraParam_t& extra_param) override;
      */
    Return<Status> addBuffer(uint64_t request, const VpuBuffer_t& vpuBuffer, const hidl_handle& hidlShareFd, const hidl_handle& hidlShareFd1, const hidl_handle& hidlShareFd2) override;
    Return<Status> setProperty(uint64_t request, int32_t sett, int32_t id) override;
    Return<Status> setPropertyMem(uint64_t request, const hidl_memory& propertyMem) override;
    Return<Status> setExtraParam(uint64_t request, const VpuExtraParam_t& extra_param) override;
    
    Return<void>   getVpuStatus(getVpuStatus_cb _hidl_cb) override;
    /*
        *   Return<Status> enque(const VpuRequest_t& request, uint32_t core_index) override;
        *   Return<Status> deque() override;
       */
    Return<Status> enque(uint64_t request, uint32_t core_index) override;
    /*
        *   Return<Status> deque(uint64_t request) override;
       */
    Return<uint64_t> deque() override;
    /*
        *   Return<Status> release(const VpuRequest_t& request) override;
       */
    Return<Status> release(uint64_t request) override;

    Return<Status> setOutputSurfaces    (int64_t timestamp, const sp<vendor::mediatek::hardware::camera::vpuclient::V2_0::IMtkVpuBuffer>& bufferCB, const hidl_vec<int32_t>& msgType) override;
    Return<Status> destroyVpuClient(int64_t timestamp) override;
    // Methods from IMtkVpuClient follow.
    //Return<::android::hardware::camera::common::V1_0::Status> sendCommand(int64_t timestamp, MtkCBClientCmdType cmd, int32_t arg1, int32_t arg2) override;
    // Methods from ::android::hidl::base::V1_0::IBase follow.
};



class VpuClientRequest {
public:
    virtual ~VpuClientRequest() { };
};

typedef std::map <uint64_t, VpuClientRequest*>  vpuClientRequestMap_t;
typedef std::pair<uint64_t, VpuClientRequest*>  vpuClientRequestPair_t;

typedef struct vpu_plane_buffer {
	uint32_t		  share_fd;
	uint64_t		  ion_handle;
	uint32_t		  mva;
	uint32_t		  offset;
} vpu_plane_buffer_t;
typedef struct vpu_request_buffer {
	uint8_t 		   plane_count;
	vpu_plane_buffer_t plane[3];
} vpu_request_buffer_t;

typedef struct vpu_setting_buffer {
	uint32_t		  share_fd;
	uint64_t		  ion_handle;
	uint32_t		  mva;
	uint64_t		  va;
} vpu_setting_buffer_t;

class VpuClientRequestImp : public VpuClientRequest {
public:
    unsigned short  mBufferCount;
    vpu_setting_buffer_t mSetting;
    vpu_request_buffer_t mRequestBuffer[VPU_MAX_NUM_PORTS] ;    // for ion

public:
    VpuClientRequestImp();
    virtual ~VpuClientRequestImp();

};













extern "C" IMtkVpuClient* HIDL_FETCH_IMtkVpuClient(const char* name);


/*
* Wrapper for calling VpuClientMgr
*/
#if 0
class VpuClientWrap {

public:
    static VpuClientWrap* getInstance();

    Return<Status> createVpuClient (int64_t timestamp);
    Return<Status> setOutputSurfaces    (int64_t timestamp, const sp<IMtkVpuBuffer>& bufferCB, int32_t size);
    Return<Status> destroyVpuClient(int64_t timestamp);
    Return<Status> sendCommand          (int64_t timestamp, MtkCBClientCmdType cmd, int32_t arg1, int32_t arg2);
    bool onVpuBuffer( int64_t const   i8Timestamp,
                           uint32_t const  u4BitstreamSize,
                           uint8_t const*  puBitstreamBuf,
                           uint32_t const  u4CallbackIndex,
                           bool            fgIsFinalImage,
                           uint32_t const  msgType,
                           uint32_t const  u4BufOffset4Ts,
                           uint32_t const u4CbId);

    bool onPostviewCallback(int64_t const  i8Timestamp,
                            uint32_t const planeCount,
                            uint32_t const width,
                            uint32_t const height,
                            uint32_t const fmt,
                            uint8_t* const u4Bitstream[],
                            uint32_t const widthInBytes[],
                            uint32_t const strideInBytes[],
                            uint32_t const scanlines[],
                            uint32_t const u4BufferOffsetForTimeStamp,
                            uint32_t const u4CbId
                            );

    static bool bufferCallback( int64_t const   i8Timestamp,
                                uint32_t const  u4BitstreamSize,
                                uint8_t const*  puBitstreamBuf,
                                uint32_t const  u4CallbackIndex,
                                bool            fgIsFinalImage,
                                uint32_t const  msgType,
                                uint32_t const  u4BufOffset4Ts,
                                uint32_t const u4CbId);

    static bool postviewCallback (int64_t const  i8Timestamp,
                                  uint32_t const planeCount,
                                  uint32_t const width,
                                  uint32_t const height,
                                  uint32_t const fmt,
                                  uint8_t* const u4Bitstream[],
                                  uint32_t const widthInBytes[],
                                  uint32_t const strideInBytes[],
                                  uint32_t const scanlines[],
                                  uint32_t const u4BufferOffsetForTimeStamp,
                                  uint32_t const u4CbId
                                  );


private:
     VpuClientWrap() {};
    ~VpuClientWrap() {};

private:
    static HandleImporter                        sHandleImporter;
    std::mutex                                   mCbMutex;            // mutex for onVpuBuffer and postviewCallback
    std::mutex                                   mMutex;              // mutex for below members
    KeyedVector<int64_t, sp<IMtkVpuBuffer>> mVpuBufferMap;  //<timestamp, IMtkVpuBuffer>
    KeyedVector<int64_t, uint32_t>               mCbIdMap;            //<timestamp, callback id>
    std::list<int64_t>                           mlTimestamp;         //timestamp to setVpuClient

};// class VpuClientWrap
#endif

}  // namespace implementation
}  // namespace V2_0
}  // namespace vpuclient
}  // namespace camera
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor


#endif  // VENDOR_MEDIATEK_HARDWARE_CAMERA_CALLBACKCLIENT_V2_0_MTKCALLBACKCLIENT_H
