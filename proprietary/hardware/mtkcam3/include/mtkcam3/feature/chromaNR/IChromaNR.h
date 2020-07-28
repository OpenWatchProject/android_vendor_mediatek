#ifndef __IChroma_H__
#define __IChroma_H__

// STD
#include <memory>
#include <vector>
#include <string>
#include <deque> // std::deque
//

// AOSP
#include <cutils/native_handle.h>

namespace chromaNR {

enum ChromaNRErr {
    ChromaNR_Err_Ok = 0,
    ChromaNR_Err_Shooted,
    ChromaNR_Err_AlreadyExist,
    ChromaNR_Err_NotInited,
    ChromaNR_Err_BadArgument,
};

struct ChromaNRParam {
    /* open id */
    int32_t open_id;
    /* current iso */
    int32_t current_iso;
    /* src width */
    int32_t src_width;
    /* src height */
    int32_t src_height;
    /* src yPitch */
    int32_t src_yPitch;
    /* src uvPitch */
    int32_t src_uvPitch;
    /* src color format */
    int32_t src_format;
    /* src size list*/
    uint32_t src_sizeList[3];
    /* src plane number*/
    int32_t src_planeNumber;
    /* handle to the input image */
    buffer_handle_t inputHandle;
    /* dst width */
    int32_t dst_width;
    /* dst height */
    int32_t dst_height;
    /* dst yPitch */
    int32_t dst_yPitch;
    /* dst uvPitch */
    int32_t dst_uvPitch;
    /* dst format */
    int32_t dst_format;
    /* dst size list */
    uint32_t dst_sizeList[3];
    /* dst plane number */
    int32_t dst_planeNumber;
    /* handle to the output image */
    buffer_handle_t outputHandle;
};

class IChromaNR {
public:
    /**
     *  Caller should always create instance via INTERFACE::createInstance method.
     *  And caller also have responsibility to destroy it by invoking destroyInstance.
     *  If caller uses android::sp to manager life cycle of instance who doesn't need to invoke
     *  destroyInstance to destroy instance.
     *
     *  @return             - An IChromaNR instance, caller has responsibility to manager it's lifetime.
     */
    static std::shared_ptr<IChromaNR> createInstance();

public:
    /**
     *  do ChromaCore, which would do chroma NR processing
     *  @param cfg          - LpcnrConfig_t structure to describe usage
     *  @return             - Returns LpcnrErr_Ok if works
     */
    virtual enum ChromaNRErr doChromaNR(const ChromaNRParam& param) = 0;

protected:
    virtual ~IChromaNR(void) {};

}; /* class IChromaNR */
}; /* namespace IChromaNR */
#endif
