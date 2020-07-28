#ifndef _IMFLLIMAGEBUFFER_H__
#define _IMFLLIMAGEBUFFER_H__

#include "MfllDefs.h"
#include "MfllTypes.h"
#include "IMfllEvents.h"

#include <utils/RefBase.h> // android::RefBase

/* give a symbol here avoid nested include */
namespace mfll {
    class IMfllCore;
};

namespace mfll {
typedef enum IMfllImageBuffer_Flag {
    Flag_Undefined = 0,
    Flag_FullSize, // describe full size YUYV/YV16
    Flag_QuarterSize, // describe 1/4 size YV16/Y
    Flag_WeightingTable, // describe full size Y8 for weighting table
    Flag_Algorithm, // describe algorithm working buffer
} IMfllImageBuffer_Flag_t;

class IMfllImageBuffer : public android::RefBase {
public:
    /**
     *  Create an IMfllImageBuffer instance depends on what module you're using
     *  @param bufferName       - Buffer name for debug usage
     *  @param flag             - Describe the buffer usage
     *  @return                 - An instance of MfllImageBuffer
     */
    static IMfllImageBuffer* createInstance(
            const char *bufferName = "",
            const IMfllImageBuffer_Flag_t &flag = Flag_Undefined);

    /**
     *  Due to management of Android's strong pointer mechanism, IMfllImageBuffer
     *  doesn't provide destructor for user, instead, invoke IMfllImageBuffer::destroyInstance()
     *  to decrease strong reference of the instance.
     */
    virtual void destroyInstance(void);

public:
    /**
     *  Set image resolution
     *  @param w                - Width in pixel
     *  @param h                - Height in pixel
     *  @return                 - Returns MfllErr_Ok if ok
     */
    virtual enum MfllErr setResolution(unsigned int w, unsigned int h) = 0;

    /**
     *  Set image resolution alignment, IMfllImageBuffer will create image buffer
     *  with a continuous memory chunk therefore it's possible to set the alignments
     *  of both width and height.
     *
     *  If alignment won't be set, default it's 1
     *
     *  @param aligned_w        - Alignment of width
     *  @param aligned_h        - Alignment of height
     *  @return                 - Returns MfllErr_Ok if ok. After image buffer has
     *                            been inited, you cannot set alignment anymore
     */
    virtual enum MfllErr setAligned(unsigned int aligned_w, unsigned int aligned_h) = 0;

    /**
     *  Set image format to be inited
     *  @param f                - Image format
     *  @return                 - Returns MfllErr_Ok if ok
     */
    virtual enum MfllErr setImageFormat(enum ImageFormat f) = 0;

    /**
     *  Get resolution
     *  @param [out] w          - Width in pixel
     *  @param [out] h          - Height in pixel
     *  @return                 - Returns MfllErr_Ok if ok
     */
    virtual enum MfllErr getResolution(unsigned int &w, unsigned int &h) = 0;

    /**
     *  Get width
     *  @return                 - Returns width
     */
    virtual unsigned int getWidth(void) const = 0;

    /**
     *  Get height
     *  @return                 - Returns height
     */
    virtual unsigned int getHeight(void) const = 0;

    /**
     *  Get aligned width
     *  @return                 - Retruns aligned width
     */
    virtual unsigned int getAlignedWidth(void) const = 0;

    /**
     *  Get aligned height
     *  @return                 - Returns aligned height
     */
    virtual unsigned int getAlignedHeight(void) const = 0;

    /**
     *  Get alignment
     *  @param [out] w          - Alignment of width in pixel
     *  @param [out] h          - Alignment of height in pixel
     *  @return                 - Returns MfllErr_Ok if ok
     */
    virtual enum MfllErr getAligned(unsigned int &w, unsigned int &h) = 0;

    /**
     *  Get image format
     *  @return                 - Image format
     */
    virtual enum ImageFormat getImageFormat(void) = 0;

    /**
     *  Acquire physical memory trunk according information created.
     *
     *  @return                 - Returns MfllErr_Ok if ok.
     *  @note                   - This function should be synchronized.
     *                            This function should be thread-safe.
     *                            Duplicated call should not make errors.
     */
    virtual enum MfllErr initBuffer(void) = 0;

    /**
     *  To check if the IMfllImageBuffer has been inited.
     *
     *  @retrun                 - True for inited.
     */
    virtual bool isInited(void) = 0;

    /**
     *  This method is NOT really to convert color space but only re-align image
     *  format header for hardware processing. It means that you can use CPU to
     *  convert color space and by this method, marks as the new format for HW processing.
     *
     *  Notice that, the total size of new image format is must be smaller or equal to
     *  the original one
     *  @param f                - New image format
     *  @return                 - Returns MffErr_Ok if ok.
     *  @note                   - Only buffer created by IMfllImageBuffer::initBuffer supports
     *                            this function.
     */
    virtual enum MfllErr convertImageFormat(const enum ImageFormat &f) = 0;

    /**
     *  Set image buffer to IMfllImageBuffer rather than using dynamically create.
     *
     *  @param lpBuffer         - Address of the buffer
     *  @return                 - Returns MfllErr_Ok if ok.
     */
    virtual enum MfllErr setImageBuffer(void *lpBuffer) = 0;

    /**
     *  Get image buffer from IMfllImageBuffer
     *  @return                 - Returns address of image buffer
     *  @note                   - Caller MUST to know the buffer type you get
     */
    virtual void* getImageBuffer(void) = 0;

    /**
     *  You can set buffer name for debug usage
     *  @param name             - Name will be displayed.
     */
    virtual void setName(const char *name) = 0;

    /**
     *  Give reference of IMfllCore
     *  @param c                - IMfllCore address
     */
    virtual enum MfllErr setMfllCore(IMfllCore *c) = 0;

    /**
     *  Get virtual address of image buffer
     *
     *  @return                 - If failed or not inited returns NULL.
     *  @note                   - This function should be thread-safe.
     */
    virtual void* getVa(void) = 0;

    /**
     *  Get the real buffer size of the MfllImageBuffer, which means the real
     *  physical buffer size, included alignments.
     *
     *  @return                 - If the buffer hasn't been allocated, this function will return 0.
     *  @note                   - This function should be thread-safe.
     */
    virtual size_t getRealBufferSize(void) = 0;

    /**
     * Get the buffer Strides in bytes of a given plane.
     *
     *  @return                 - If the buffer hasn't been allocated, this function will return 0.
     *  @note                   - This function should be thread-safe.
     */
    virtual size_t getBufStridesInBytes(size_t index) = 0;

    /**
     *  Release buffer memory directly rather than destroy this object.
     *  The object will be destroyed (which means invoking destructor) when the
     *  reference count is counted to 0.
     *
     *  @return                 - Returns MfllErr_Ok if ok.
     *  @note                   - This function should be synchronized.
     *                            This function should be thread-safe.
     *                            Duplicated call should not make errors.
     *                            This function should be invoked while instance is being destroyed.
     *                            We do not suggest caller to invoke this function to release buffer,
     *                            it's better to release buffer by destructor.
     */
    virtual enum MfllErr releaseBuffer(void) = 0;

    /**
     *  The physical buffer has a cache machanism for CPU, if the image buffer is updated
     *  by CPU, caller has to invoke syncCache to makes all changes are synced
     *  to physical memory chunks
     */
    virtual enum MfllErr syncCache(void) = 0;

    /**
     *  Register event dispatcher.
     *
     *  @param e                - A strong pointer of IMfllEvents.
     *  @return                 - Returns MfllErr_Ok if ok.
     */
    virtual enum MfllErr registerEventDispatcher(const sp<IMfllEvents> &e) = 0;

    /**
     *  Save image to file.
     *
     *  @param *name            - File path to save.
     *  @return                 - Returns MfllErr_Ok if ok.
     */
    virtual enum MfllErr saveFile(const char *name) = 0;

    /**
     *  Load image from file.
     *
     *  @param *name            - File path to load.
     *  @return                 - Returns MfllErr_Ok if ok.
     */
    virtual enum MfllErr loadFile(const char *name) = 0;

protected:
    virtual ~IMfllImageBuffer(){};
};
}; /* namespace mfll */
#endif//_IMFLLIMAGEBUFFER_H__

