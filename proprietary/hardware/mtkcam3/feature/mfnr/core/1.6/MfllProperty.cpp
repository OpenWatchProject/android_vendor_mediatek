#define LOG_TAG "MfllCore/Prop"

#include <mtkcam3/feature/mfnr/MfllProperty.h>
#include <mtkcam3/feature/mfnr/MfllLog.h>
#include "MfllCore.h"

#include <cutils/properties.h> // property_get

#include <stdlib.h> // atoi

#ifndef PROPERTY_VALUE_MAX
#define PROPERTY_VALUE_MAX 32
#endif


namespace mfll_property {
    static int _dbgLevel = mfll::MfllProperty::readProperty(mfll::Property_LogLevel);
}


using namespace mfll;
using std::vector;

MfllProperty::MfllProperty(void)
{
    for (size_t i = 0; i < (size_t)Property_Size; i++) {
        int v = readProperty((Property_t)i);
        m_propValue[i] = v;

        /* If dump all is set ... */
        if (Property_DumpAll == i && v == MFLL_PROPERTY_ON) {
            for (int j = (int)Property_DumpRaw; j<(int)Property_Size; j++) {
                setProperty((Property_t)j, MFLL_PROPERTY_ON);
            }
            break;
        }
    }
}

MfllProperty::~MfllProperty(void)
{
}

int MfllProperty::readProperty(const Property_t &t)
{
    return readProperty(t, -1);
}

int MfllProperty::readProperty(const Property_t& t, int defaultVal)
{
    return ::property_get_int32(PropertyString[(size_t)t], defaultVal);
}

int MfllProperty::isForceMfll(void)
{
    return readProperty(Property_ForceMfll);
}

int MfllProperty::getFullSizeMc(void)
{
    return readProperty(Property_FullSizeMc);
}

int MfllProperty::getCaptureNum(void)
{
    return readProperty(Property_CaptureNum);
}

int MfllProperty::getBlendNum(void)
{
    return readProperty(Property_BlendNum);
}

int MfllProperty::getDropNum(void)
{
    return readProperty(Property_DropNum);
}

int MfllProperty::getExposure(void)
{
    return readProperty(Property_Exposure);
}

int MfllProperty::getIso(void)
{
    return readProperty(Property_Iso);
}

int MfllProperty::getBss(void)
{
    int r = readProperty(Property_Bss);
    if (r == -1)
        r = 1;
    return r;
}

bool MfllProperty::getForceBssOrder(vector<int> &bssOrder)
{
    /* not implemented and not supported*/
    return false;
}

int MfllProperty::getForceGmvZero(void)
{
    return readProperty(Property_ForceGmvZero);
}

bool MfllProperty::getForceGmv(MfllMotionVector_t (&globalMv)[MFLL_MAX_FRAMES])
{
    mfllLogD("not support force gmv setting");
    return false;
}

int MfllProperty::getForceMmdvfsOff(void)
{
    return readProperty(Property_ForceMmdvfsOff);
}

int MfllProperty::getDebugLevel(void)
{
    return mfll_property::_dbgLevel;
}

int MfllProperty::getMfbP2DirectLink(void)
{
    return 0;
}

bool MfllProperty::isDump(void)
{
    std::unique_lock<std::mutex> _l(m_mutex);
    bool is_dump = false;
    for (int i = (int)Property_DumpRaw; i < (int)Property_Size; i++) {
        if (m_propValue[i] == MFLL_PROPERTY_ON) {
            is_dump = true;
            break;
        }
    }
    return is_dump;
}

int MfllProperty::getProperty(const Property_t &t)
{
    std::unique_lock<std::mutex> _l(m_mutex);
    return m_propValue[(size_t)t];
}

void MfllProperty::setProperty(const Property_t &t, const int &v)
{
    std::unique_lock<std::mutex> _l(m_mutex);
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    snprintf(value, PROPERTY_VALUE_MAX, "%d", v);
    property_set(PropertyString[(size_t)t], value);
    m_propValue[(size_t)t] = v;
}
