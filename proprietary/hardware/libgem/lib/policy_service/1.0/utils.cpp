#include "dfps/utils.h"

namespace android {

template<typename T, typename U>
T unwrapRet(Return<T>& ret, const U& default_val)
{
    return (ret.isOk()) ? static_cast<T>(ret) :
        static_cast<T>(default_val);
}

Error unwrapRet(Return<Error>& ret)
{
    return unwrapRet(ret, kDefaultError);
}

};
