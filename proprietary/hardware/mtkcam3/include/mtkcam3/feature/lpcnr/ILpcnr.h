#ifndef __ILPCNRCORE_H__
#define __ILPCNRCORE_H__

// AOSP
#include <utils/RefBase.h> // android::RefBase
// STD
#include <memory>
#include <vector>
#include <string>
#include <deque> // std::deque
//
// MTKCAM
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
// MTK utils
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/metadata/IMetadata.h>


using android::sp;
using std::vector;
using namespace NSCam;

namespace lpcnr {

enum LpcnrErr {
    LpcnrErr_Ok = 0,
    LpcnrErr_Shooted,
    LpcnrErr_AlreadyExist,
    LpcnrErr_NotInited,
    LpcnrErr_BadArgument,
};

class ILpcnr {
public:
    struct ConfigParams{
        unsigned int openId;
        MSize       buffSize;
        IMetadata *appMeta;
        IMetadata *halMeta;
        IMetadata *appDynamic; //P1 dynamic app
        IImageBuffer *inputBuff;
        IImageBuffer *outputBuff;
        ConfigParams()
            : openId(0)
            , appMeta(nullptr)
            , halMeta(nullptr)
            , appDynamic(nullptr)
            , inputBuff(nullptr)
            , outputBuff(nullptr)
            {};
    };
public:
    /**
     *  Caller should always create instance via INTERFACE::createInstance method.
     *  And caller also have responsibility to destroy it by invoking destroyInstance.
     *  If caller uses android::sp to manager life cycle of instance who doesn't need to invoke
     *  destroyInstance to destroy instance.
     *
     *  @param major        - Major version, if the value is or smaller to 0, use default.
     *  @param minor        - Minor version, if the value is or smaller to 0, use default.
     *  @param featured     - Featured version.
     *  @return             - An ILpcnr instance, caller has responsibility to manager it's lifetime.
     */
    static std::shared_ptr<ILpcnr> createInstance();
    static bool supportLpcnr();

public:
    /**
     *  Init LpcnrCore, if using MrpMode_BestPerformance, a thread will be create
     *  to allocate memory chunks, notice that, this thread will be joined while
     *  destroying LpcnrCore.
     *  @param cfg          - LpcnrConfig_t structure to describe usage
     *  @return             - Returns LpcnrErr_Ok if works
     */
    virtual enum LpcnrErr init (const ConfigParams &cfg) = 0;
    virtual enum LpcnrErr doLpcnr () = 0;
    virtual enum LpcnrErr makeDebugInfo(IMetadata* metadata) = 0;



protected:
    virtual ~ILpcnr(void) {};

}; /* class ILpcnr */
}; /* namespace lpcnr */
#endif

