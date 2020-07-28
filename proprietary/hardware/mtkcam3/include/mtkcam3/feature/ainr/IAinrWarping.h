#ifndef __IAINRWPE_H__
#define __IAINRWPE_H__

#include "AinrDefs.h"
#include "AinrTypes.h"

// STL
#include <memory>
//
/**
 *  IAinrWarping has responsibility handle two input buffers (Base frame and referenced frame)
 *  matched data. Ususally used by algorithm to do homography postprocessing. IFeFm output
 *  data FMO would indicate matching point of two frames. Be aware of that FMO is confidential
 *  format we should parsing result by algorithm before using it.
 */
using namespace NSCam;

namespace ainr {
class IAinrWarping {
public:
    struct ConfigParams
    {
        int     openId;
        MSize   bufferSize;

        ConfigParams()
            : openId(0)
            , bufferSize(MSize())
        {};
        ConfigParams(int sensorId)
            : openId(sensorId)
            , bufferSize(MSize())
        {};
    };
    //
    struct WarpingPackage
    {
        IImageBuffer *buffer;
        IImageBuffer *gridX;
        IImageBuffer *gridY;
        IImageBuffer *outRg;
        IImageBuffer *outGb;


        WarpingPackage()
            : buffer(nullptr)
            , gridX(nullptr)
            , gridY(nullptr)
            , outRg(nullptr)
            , outGb(nullptr)
            {}
    };

public:
    static std::shared_ptr<IAinrWarping> createInstance(void);

/* interfaces */
public:

    /* Feed config data into warping.
     *  @param params               There're different NVRAM settings in the
     *                              different params.
     */
    virtual enum AinrErr init(IAinrWarping::ConfigParams const& params) = 0;


    /* Returns work succed or not.
     *  @param [out] BufferPackage     Represents buffer size. This argument can be
     *                              null.
     *  @return                     Address of the buffer, if read fail returns
     *                              null.
     */
    virtual enum AinrErr doWarping(IAinrWarping::WarpingPackage &infoPack) = 0;
protected:
    virtual ~IAinrWarping() { };
};
}; /* namespace ainr */

#endif//__IAINRWPE_H__