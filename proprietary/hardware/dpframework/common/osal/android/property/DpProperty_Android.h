#ifndef __DP_PROPERTY_ANDROID_H__
#define __DP_PROPERTY_ANDROID_H__

#include "DpDataType.h"

DP_STATUS_ENUM getProperty(const char *pName, int32_t *pProp);

DP_STATUS_ENUM getProperty(const char *pName, char *pProp);

#endif  // __DP_PROPERTY_ANDROID_H__
