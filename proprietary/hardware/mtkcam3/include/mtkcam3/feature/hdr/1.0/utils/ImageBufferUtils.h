#ifndef _IMAGEBUFFERUTILS_H_
#define _IMAGEBUFFERUTILS_H_

#include <utils/Singleton.h>
#include <mtkcam/BuiltinTypes.h>
#include <mtkcam/IImageBuffer.h>

using namespace android;

// ---------------------------------------------------------------------------

namespace NSCam {

// ---------------------------------------------------------------------------

class ImageBufferUtils : public Singleton<ImageBufferUtils>
{
public:
    MBOOL allocBuffer(IImageBuffer** ppBuf, MUINT32 w, MUINT32 h, MUINT32 fmt);
    void deallocBuffer(IImageBuffer* pBuf);
};

// ---------------------------------------------------------------------------

}; // namespace NSCam

#endif
