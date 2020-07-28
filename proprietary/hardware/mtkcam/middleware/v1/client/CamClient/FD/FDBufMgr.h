#ifndef FD_BUFF_MGR_H
#define FD_BUFF_MGR_H

/******************************************************************************
 *
 ******************************************************************************/
#include <utils/threads.h>
#include <utils/RefBase.h>
#include <mtkcam/def/common.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
//
#include <vector>
using namespace std;
using namespace NSCam;
//
#if PADDING_SUPPORTED
#define FD_IMG_PADDING (64)
#else
#define FD_IMG_PADDING (0)
#endif
/******************************************************************************
 *
 ******************************************************************************/
class FDBuffer : public IImgBuf
{
/******************************************************************************
*   Inheritance from IMemBuf.
*******************************************************************************/
public:
    virtual int64_t             getTimestamp() const                    { return mTimestamp; }
    virtual void                setTimestamp(int64_t const timestamp)   { mTimestamp = timestamp; }
    virtual size_t              getBufSize() const                      { return mSize; }
    virtual void*               getVirAddr() const                      { return (void*)mVirtAddr; }
    virtual void*               getPhyAddr() const                      { return (void*)mPhyAddr; }
    virtual const char*         getBufName() const                      { return mName; }
    virtual int                 getIonFd() const                        { return mMemID; }


/******************************************************************************
*   Inheritance from IImgBuf.
*******************************************************************************/
public:
    virtual String8 const&      getImgFormat()      const               { return mformat; }
    virtual uint32_t            getImgWidthStride(
                                    uint_t const uPlaneIndex = 0
                                )   const
                                {
                                    return NSCam::Utils::Format::queryPlaneWidthInPixels(mImgFormat, uPlaneIndex, mStride);
                                }

    virtual uint32_t            getImgWidth()       const               { return mWidth;  }
    virtual uint32_t            getImgHeight()      const               { return mHeight; }
    virtual uint32_t            getBitsPerPixel()   const               { return mBpp;    }
    virtual bool                syncCache( eCacheCtrl const ctrl = eCACHECTRL_FLUSH) {return mpImg->syncCache(ctrl);}


/******************************************************************************
*   Initialization.
*******************************************************************************/
public:
    FDBuffer( uint32_t _w = 0,
              uint32_t _h = 0,
              uint32_t _bpp = 0,
              uint32_t _bufsize = 0,
              int32_t  _imgformat = 0,
              String8  _format = String8(""),
              const char* _name = "")
        : IImgBuf()
        , mWidth(_w)
        , mHeight(_h)
        , mBpp(_bpp)
        , mbufSize(_bufsize)
        , mTimestamp(0)
        , mImgFormat(_imgformat)
        , mformat(_format)
        , mName(_name)
    {
        createBuffer();
    }

    virtual                     ~FDBuffer() {destroyBuffer();};
    bool                        operator!() const   { return mVirtAddr != 0; }

/******************************************************************************
*   Initialization.
*******************************************************************************/
private:
    virtual void                createBuffer();
    virtual void                destroyBuffer();

private:
    uint32_t mWidth;
    uint32_t mHeight;
    uint32_t mStride;
    uint32_t mBpp;
    uint32_t mbufSize;
    int64_t  mTimestamp;
    int32_t  mImgFormat;
    String8  mformat;
    const char* mName;
    MUINT32  mSize;
    MUINTPTR mVirtAddr;
    MUINTPTR mPhyAddr;
    MINT32   mMemID;
    IImageBufferAllocator*          mAllocator;
    IImageBuffer*                   mpImg;
};

#endif
