#define LOG_TAG "MfllCore/Prop"

#include <mtkcam3/feature/mfnr/MfllProperty.h>
#include <mtkcam3/feature/mfnr/MfllLog.h>
#include "MfllCore.h"

#include <cutils/properties.h> // property_get

#include <stdlib.h> // atoi

#ifndef PROPERTY_VALUE_MAX
#define PROPERTY_VALUE_MAX 32
#endif

#ifndef PROPERTY_STRING_MAX
#define PROPERTY_STRING_MAX 256
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
    char str[PROPERTY_STRING_MAX];
    ::property_get(PropertyString[(size_t)Property_ForceGmv], str, "-1");
    if (0 == strcmp(str, "-1")) {
        mfllLogD("no force gmv setting");
        return false;
    }
    else{
        // Rest force setting to default
        for (int i = 0; i < MFLL_MAX_FRAMES; i++) {
            mfllLogD("%s: original GMV[%d] (x,y) = (%d,%d)", __FUNCTION__, i, globalMv[i].x, globalMv[i].y);
            globalMv[i].x = 0;
            globalMv[i].y = 0;
        }
        // Apply GMV force setting
        int setting_count = 0;
        int frame_count = 0;

        char *delim = " ),(;";
        char * pch;
        mfllLogD("%s: Force GMV setting: \"%s\"", __FUNCTION__, str);
        pch = strtok(str, delim);
        while (pch != NULL)
        {
            setting_count++;
            int value = atoi(pch);
            if (setting_count%2 != 0) {
                mfllLogD("%s: GMV[%d].x=%d", __FUNCTION__, frame_count, value);
                globalMv[frame_count].x = value;
            }
            else {
                mfllLogD("%s: GMV[%d].y=%d", __FUNCTION__, frame_count, value);
                globalMv[frame_count].y = value;

                frame_count++;
                if (frame_count >= MFLL_MAX_FRAMES) {
                    mfllLogW("%s: out of globalMv's MFLL_MAX_FRAMES:%d", __FUNCTION__, frame_count);
                    break;
                }
            }
            pch = strtok (NULL, delim);
        }

        return true;
    }
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

bool MfllProperty::isFeed(void)
{
    std::unique_lock<std::mutex> _l(m_mutex);
    bool is_feed = false;
    for (int i = (int)Property_FeedBfbld; i < (int)Property_DumpAll; i++) {
        if (m_propValue[i] == MFLL_PROPERTY_ON) {
            is_feed = true;
            break;
        }
    }
    return is_feed;
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
