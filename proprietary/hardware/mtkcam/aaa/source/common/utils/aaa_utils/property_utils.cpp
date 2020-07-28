#include <stdlib.h>
#include <cutils/properties.h>

#include "string_utils.h"

int getPropInt(const char *key, int *value, int defValue)
{
    char sValue[PROPERTY_VALUE_MAX] = {'\0'};

    /* verify arguments */
    if (!key || !value)
        return -1;

    *value = defValue;

    if (!property_get(key, sValue, NULL))
        return -1;

    if (!isInt(sValue))
        return -1;

    *value = atoi(sValue);

    return 0;
}

int getPropStr(const char *key, char *value, const char *defValue)
{
    /* verify arguments */
    if (!key || !value)
        return -1;

    return property_get(key, value, defValue);
}

