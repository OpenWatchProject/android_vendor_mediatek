#ifndef __IAINRNVRAM_H__
#define __IAINRNVRAM_H__

#include "AinrDefs.h"
#include "AinrTypes.h"

#include <memory>

/**
 *  AinrNvram has responsibility to read NVRAM data chunk. Different platform
 *  may have different structures to format the chunk. Here only provide a way
 *  to read the NVRAM data chunk, without format it to the certain structures.
 *
 *  AinrNVram uses cache mechanism, which means it's cheap to create the several
 *  instances, all of them shared w/ the same memory chunk (if sensor ID equals).
 */
namespace ainr {
class IAinrNvram {
public:
    enum class nvram_hint : int32_t {
        AIISP_THRE = 0,
        AIISP_LV_PRE = 1,
        AIISP_ISO_FEFM = 2,
        AIISP_ISO_SWME = 3,
        AIISP_ISO_APU_Part1 = 4,
        AIISP_ISO_APU_Part2 = 5,
        AIISP_ISO_APU_Part3 = 6,
        AIISP_ISO_POSTSW = 7,
        AIISP_ISO_DRC = 8,
    };

    struct ConfigParams {
        int             iSensorId;
        bool            bFlashOn;
        int             queryIndexAinr;
        int             queryIndexAinrTh;
        int             iso;
        AinrFeatureType featureType;
        ConfigParams()
            : iSensorId(0)
            , bFlashOn(false)
            , queryIndexAinr(-1)
            , queryIndexAinrTh(-1)
            , iso(0)
            , featureType(AINR_SINGLE)
            {};
        ConfigParams(int sensorId)
            : iSensorId(sensorId)
            , bFlashOn(false)
            , queryIndexAinr(-1)
            , queryIndexAinrTh(-1)
            , iso(0)
            , featureType(AINR_SINGLE)
            {};
    };
public:
    static std::shared_ptr<IAinrNvram> createInstance(void);

/* interfaces */
public:
    /* NVRAM data chunk is takes sensor ID to be found.
     *  @param sensorId             There're different NVRAM settings in the
     *                              different sensors.
     */
    virtual enum AinrErr init(int sensorId) = 0;

    /* New NVRAM data chunk is takes sensor ID and more params to be found.
     *  @param params               There're different NVRAM settings in the
     *                              different params.
     */
    virtual enum AinrErr init(IAinrNvram::ConfigParams& params) = 0;


    /* Returns a copied data chunk.
     *  @param [out] bufferSize     Represents buffer size. This argument can be
     *                              null.
     *  @return                     Address of the buffer, if read fail returns
     *                              null.
     */
    virtual const std::shared_ptr<char> chunk(size_t* bufferSize = nullptr) const = 0;

    /* Returns data chunk address.
     *  @param [out] bufferSize     Represents buffer size. This argument can be
     *                              null.
     *  @return                     Address of the buffer, if read fail returns
     *                              null.
     */
    virtual const char* getChunk(size_t* bufferSize = nullptr) const = 0;

    /* Returns a copied data chunk.
     *  @param [out] bufferSize     Represents buffer size. This argument can be
     *                              null.
     *  @return                     Address of the buffer, if read fail returns
     *                              null.
     */
    virtual const std::shared_ptr<char> chunkAinrTh(size_t* bufferSize = nullptr) const = 0;

    /* Returns data chunk address.
     *  @param [out] bufferSize     Represents buffer size. This argument can be
     *                              null.
     *  @return                     Address of the buffer, if read fail returns
     *                              null.
     */
    virtual const char* getChunkAinrTh(size_t* bufferSize = nullptr) const = 0;

    /* Returns data chunk address according to nvram_hint
     *  @param [out] bufferSize     Represents buffer size. This argument can be
     *                              null.
     *  @return                     Address of the buffer, if read fail returns
     *                              null.
     */
    virtual const char* getSpecificChunk(const IAinrNvram::nvram_hint hint
        , size_t* bufferSize = nullptr) const = 0;

protected:
    virtual ~IAinrNvram() { };
};
}; /* namespace ainr */

#endif//__IAINRNVRAM_H__
