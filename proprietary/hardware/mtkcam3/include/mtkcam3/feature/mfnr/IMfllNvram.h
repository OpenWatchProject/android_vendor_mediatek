#ifndef __IMFLLNVRAM_H__
#define __IMFLLNVRAM_H__

#include "MfllDefs.h"
#include "MfllTypes.h"

#include <utils/RefBase.h> // android::RefBase
#include <memory>

using android::sp;

/**
 *  MfllNvram has responsibility to read NVRAM data chunk. Different platform
 *  may have different structures to format the chunk. Here only provide a way
 *  to read the NVRAM data chunk, without format it to the certain structures.
 *
 *  MfllNVram uses cache mechanism, which means it's cheap to create the several
 *  instances, all of them shared w/ the same memory chunk (if sensor ID equals).
 */
namespace mfll {
class IMfllNvram : public android::RefBase {
public:

    struct ConfigParams
    {
        // [in]
        int     iSensorId;
        bool    bFlashOn;
        // [out]
        int     iQueryIndex_Bss;
        int     iQueryIndex_Mfnr;
        int     iQueryIndex_MfnrTh;
        ConfigParams()
            : iSensorId(0)
            , bFlashOn(false)
            , iQueryIndex_Bss(-1)
            , iQueryIndex_Mfnr(-1)
            , iQueryIndex_MfnrTh(-1)
        {};
        ConfigParams(int sensorId)
            : iSensorId(sensorId)
            , bFlashOn(false)
            , iQueryIndex_Bss(-1)
            , iQueryIndex_Mfnr(-1)
            , iQueryIndex_MfnrTh(-1)
        {};
    };

public:
    static IMfllNvram* createInstance(void);
    virtual void destroyInstance(void);

/* interfaces */
public:
    /* NVRAM data chunk is takes sensor ID to be found.
     *  @param sensorId             There're different NVRAM settings in the
     *                              different sensors.
     */
    virtual enum MfllErr init(int sensorId) = 0;

    /* New NVRAM data chunk is takes sensor ID and more params to be found.
     *  @param params               There're different NVRAM settings in the
     *                              different params.
     */
    virtual enum MfllErr init(IMfllNvram::ConfigParams& params) = 0;


    /* Returns a copied data chunk.
     *  @param [out] bufferSize     Represents buffer size. This argument can be
     *                              null.
     *  @return                     Address of the buffer, if read fail returns
     *                              null.
     */
    virtual std::shared_ptr<char> chunk(size_t* bufferSize = nullptr) = 0;

    /* Returns a copied data chunk of MFNR, new after ISP6.0, it should same as getChunk.
     *  @param [out] bufferSize     Represents buffer size. This argument can be
     *                              null.
     *  @return                     Address of the buffer, if read fail returns
     *                              null.
     */
    virtual std::shared_ptr<char> chunkMfnr(size_t* bufferSize = nullptr) = 0;

    /* Returns a copied data chunk of MFNR_TH, new after ISP6.0
     *  @param [out] bufferSize     Represents buffer size. This argument can be
     *                              null.
     *  @return                     Address of the buffer, if read fail returns
     *                              null.
     */
    virtual std::shared_ptr<char> chunkMfnrTh(size_t* bufferSize = nullptr) = 0;

    /* Returns data chunk address.
     *  @param [out] bufferSize     Represents buffer size. This argument can be
     *                              null.
     *  @return                     Address of the buffer, if read fail returns
     *                              null.
     */
    virtual const char* getChunk(size_t* bufferSize = nullptr) = 0;

    /* Returns data chunk address of MFNR, new after ISP6.0, it should same as getChunk.
     *  @param [out] bufferSize     Represents buffer size. This argument can be
     *                              null.
     *  @return                     Address of the buffer, if read fail returns
     *                              null.
     */
    virtual const char* getChunkMfnr(size_t* bufferSize = nullptr) = 0;

    /* Returns data chunk address of MFNR_TH, new after ISP6.0
     *  @param [out] bufferSize     Represents buffer size. This argument can be
     *                              null.
     *  @return                     Address of the buffer, if read fail returns
     *                              null.
     */
    virtual const char* getChunkMfnrTh(size_t* bufferSize = nullptr) = 0;


protected:
    virtual ~IMfllNvram() { };
};
}; /* namespace mfll */

#endif//__IMFLLNVRAM_H__
