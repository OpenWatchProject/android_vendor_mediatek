#ifndef HWC_TCON_H_
#define HWC_TCON_H_

#include "epaper_post_processing.h"

#include <hwc_tcon.h>

class TconInterface : public ProxyInterface
{
public:
    TconInterface();
    virtual ~TconInterface();

    // requestBufSize() use to know that the size of required buffer
    void requestBufSize(uint32_t src_width, uint32_t src_height, nsecs_t src_rate, uint32_t* dst_width, uint32_t* dst_height);

    // sw tcon may fill some initial data in our buffer
    void init(struct ProxyBuffer* buf, uint32_t num);

    // sw tcon can do destruct job in here
    void release(struct ProxyBuffer* buf, uint32_t num);

    // provide sw tcon with the pointer of new source buffer
    unsigned char setSourceBuffer(int32_t mode, void *ptr, size_t size);

    // require that sw tcon fill new frame to specific buffer
    unsigned char getCookedBuffer(void *ptr, size_t size);

    // reset the state of sw tcon
    void reset(void);

private:
    struct tcon_module *mTcon;
};

#endif
