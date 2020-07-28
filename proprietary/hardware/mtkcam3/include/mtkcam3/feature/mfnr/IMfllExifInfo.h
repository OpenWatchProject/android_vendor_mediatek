#ifndef __IMFLLEXIFINFO_H__
#define __IMFLLEXIFINFO_H__

#include "MfllDefs.h"
#include "MfllTypes.h"
#include "IMfllNvram.h"

#include <utils/RefBase.h> // android::RefBase
#include <memory>
#include <map>
#include <string>
#include <deque>

using android::sp;

namespace mfll {

class IMfllExifInfo : public android::RefBase {
public:
    static IMfllExifInfo* createInstance(void);
    virtual void destroyInstance(void);

/* interfaces */
public:
    virtual enum MfllErr init() = 0;

    virtual enum MfllErr updateInfo(const MfllCoreDbgInfo_t &dbgInfo) = 0;

    virtual enum MfllErr updateInfo(IMfllNvram *pNvram) = 0;

    virtual enum MfllErr updateInfo(unsigned int key, uint32_t value) = 0;

    virtual uint32_t getInfo(unsigned int key) = 0;

    virtual const std::map<unsigned int, uint32_t>& getInfoMap() = 0;

    virtual enum MfllErr sendCommand(
            const std::string& cmd,
            const std::deque<void*>& dataset) = 0;

    /**
     *  To check MF exif version
     */
    virtual unsigned int getVersion() = 0;

protected:
    virtual ~IMfllExifInfo() {};
};
}; /* namespace mfll */

#endif //__IMFLLEXIFINFO_H__
