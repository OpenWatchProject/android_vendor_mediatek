#ifndef MTK_HWC_TCON_H_
#define MTK_HWC_TCON_H_

#include <utils/StrongPointer.h>
#include <utils/Timers.h>
#include <ui/GraphicBuffer.h>

#define HWC_TCON_SYM HWCTCON
#define HWC_TCON_SYM_AS_STR "HWCTCON"

#define MAKE_VERSION(major, minor) \
    ((((major) & 0xff) << 8) | (minor & 0xff))

#define TCON_VERSION_1_0 MAKE_VERSION(1, 0)

using namespace android;

struct ProxyBuffer
{
public:
    sp<GraphicBuffer> gb;
    size_t size;
    void *ptr;
};

enum {
    HWC_TCON_MODE_MIRROR = 0,
    HWC_TCON_MODE_EXTENSTION,
};

struct tcon_module {
    void *handle;
    uint16_t (*get_version)(void);

    status_t (*create)(void);
    status_t (*destroy)(void);

    void (*request_buf_size)(uint32_t src_width, uint32_t src_height, nsecs_t src_rate, uint32_t* dst_width, uint32_t* dst_height);
    void (*init)(struct ProxyBuffer* buf, uint32_t num);
    void (*release)(struct ProxyBuffer* buf, uint32_t num);
    unsigned char (*setSourceBuffer)(int32_t mode, void *ptr, size_t size);
    unsigned char (*getCookedBuffer)(void *ptr, size_t size);
    void (*reset)(void);
};

#endif
