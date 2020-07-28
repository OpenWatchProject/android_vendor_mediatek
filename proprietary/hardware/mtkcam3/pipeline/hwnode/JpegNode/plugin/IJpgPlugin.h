#include <utils/RefBase.h>
#include <mtkcam/def/UITypes.h>
#include <mtkcam/def/Errors.h>

using namespace android;
namespace NSCam {
namespace v3 {

class IJpgPlugin
    :public virtual android::RefBase
{
public:
    virtual ~IJpgPlugin() = default;

    static sp<IJpgPlugin> createInstance();

    virtual MERROR init() = 0;

    virtual size_t process() = 0;

    virtual MERROR uninit() = 0;

};
};
};
