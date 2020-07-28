#ifndef HWC_TCON_IMPL_H_
#define HWC_TCON_IMPL_H_

#include <utils/Timers.h>
#include <utils/Log.h>

class dummySwTcon {
public:
    dummySwTcon(void);
    ~dummySwTcon(void) {};

    // requestBufSize() return the requirement of buffer size
    void requestBufSize(uint32_t src_width, uint32_t src_height, nsecs_t src_rate, uint32_t* dst_width, uint32_t* dst_height);

    // init() used to initiate the content of buffer
    void init(struct ProxyBuffer* buf, uint32_t num);

    // release() used to clean buffer
    void release(struct ProxyBuffer* buf, uint32_t num);

    // setSourceBuffer() set the source buffer to tcon
    unsigned char setSourceBuffer(int32_t mode, void *ptr, size_t size);

    // getCookedBuffer() used to fill processed data to buffer
    unsigned char getCookedBuffer(void *ptr, size_t size);

    // reset() used to reset the state of tcon
    void reset(void);

private:

    // m_width stored the width of source buffer
    unsigned int m_width;

    // m_height stored the height of source buffer
    unsigned int m_height;

    // m_rate used to stored the refresh rate of display
    unsigned int m_rate;

    // m_src_ptr used to save the pointer of source buffer
    void* m_src_ptr;

    // m_src_size is the data size of source buffer
    size_t m_src_size;

    // m_count control the repeat times of this frame
    int m_count;

    // m_line_size is the size of white block
    unsigned int m_line_size;
};

#endif
