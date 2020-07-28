#pragma GCC system_header
#ifndef ANDROID_MTK_GRAPHIC_BUFFER_UTIL_H
#define ANDROID_MTK_GRAPHIC_BUFFER_UTIL_H
#include <utils/Singleton.h>
#include <utils/StrongPointer.h>

#include <ui/PixelFormat.h>
#include <ui/Rect.h>

#include <ui/gralloc_extra.h>
#include <system/window.h>

namespace android {
// ---------------------------------------------------------------------------
class GraphicBuffer;

struct DownSampleConfig
{
    int mDownSampleX;
    int mDownSampleY;
    Rect mCrop;
};

class BufferInfo
{
private:
    int mErr;
    status_t updateVideoInfo(const buffer_handle_t& handle);

public:
    int mWidth;
    int mHeight;
    int mStride;
    int mVStride;
    int mFormat;
    size_t mSize;
    buffer_handle_t mHandle;
    gralloc_extra_ion_sf_info_t mExtInfo;

    BufferInfo() :
        mErr(NO_ERROR),
        mWidth(0),
        mHeight(0),
        mStride(0),
        mVStride(0),
        mFormat(0),
        mSize(0),
        mHandle(NULL)
    {
        memset(&mExtInfo, 0, sizeof(mExtInfo));
    }

    status_t getInfo(const buffer_handle_t& handle);
    status_t getInfo(const sp<GraphicBuffer>& gb);
};

class GraphicBufferUtil : public Singleton<GraphicBufferUtil>
{
public:
    static inline GraphicBufferUtil& get() { return getInstance(); }

    /** @brief dump the GraphicBuffer
     *
     * @param prefix the prefix of file name
     * @param dir the stored directory, default is "/data/"
     */
    void dump(const buffer_handle_t& handle, const char* prefix = "", const char* dir = "/data/");
    void dump(const sp<GraphicBuffer>& gb, const char* prefix = "", const char* dir = "/data/");
    void dump(const BufferInfo &handle, const char* prefix = "", const char* dir = "/data/");

    uint32_t getBitsPerPixel(int format);
    uint32_t getBitsPerPixel(buffer_handle_t handle);
    int getRealFormat(buffer_handle_t handle, PixelFormat* format);

    int drawLine(const buffer_handle_t& handle, uint8_t val, int ptn_w, int ptn_h, int pos);
    int drawLine(const sp<GraphicBuffer>& gb, uint8_t val, int ptn_w, int ptn_h, int pos);
    int drawLine(const BufferInfo &handle, uint8_t val, int ptn_w, int ptn_h, int pos);
    DownSampleConfig downSampleCopy(const DownSampleConfig& config,
                                    const sp<GraphicBuffer>& srcBuf,
                                    sp<GraphicBuffer>& dstBuf);

private:
    friend class Singleton<GraphicBufferUtil>;

    GraphicBufferUtil();
    ~GraphicBufferUtil();
};

inline GraphicBufferUtil& getGraphicBufferUtil() {
    return GraphicBufferUtil::get();
}
// ---------------------------------------------------------------------------
}; // namespace android

#endif // ANDROID_MTK_GRAPHIC_BUFFER_UTIL_H
