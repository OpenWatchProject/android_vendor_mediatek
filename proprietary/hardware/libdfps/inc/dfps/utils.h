#ifndef __MTK_DFPS_UTILS_H__
#define __MTK_DFPS_UTILS_H__

#include <vendor/mediatek/hardware/dfps/1.0/IFpsPolicyService.h>

namespace android {

using hardware::Return;
using vendor::mediatek::hardware::dfps::V1_0::Error;

constexpr Error kDefaultError = Error::NONE;

template<typename T, typename U>
T unwrapRet(Return<T>& ret, const U& default_val);

Error unwrapRet(Return<Error>& ret);

};
#endif
