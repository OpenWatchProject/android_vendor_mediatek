#include "DpProperty.h"
#include <cutils/properties.h>

DP_STATUS_ENUM getProperty(const char *pName, int32_t *pProp)
{
    char value[PROPERTY_VALUE_MAX];

    memset(value, 0x0, sizeof(value));
    property_get(pName, value, "0");
    *pProp = atoi(value);

    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM getProperty(const char *pName, char *pProp)
{
    char value[PROPERTY_VALUE_MAX];

    memset(value, 0x0, sizeof(value));
    property_get(pName, value, "");
    sprintf(pProp, "%s", value);

    return DP_STATUS_RETURN_SUCCESS;
}
