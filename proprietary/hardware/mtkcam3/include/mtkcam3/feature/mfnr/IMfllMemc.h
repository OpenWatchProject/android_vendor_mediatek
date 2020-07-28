#ifndef __IMFLLMEMC_H__
#define __IMFLLMEMC_H__

#include "MfllDefs.h"
#include "MfllTypes.h"
#include "IMfllImageBuffer.h"
#include "IMfllNvram.h"
#include "IMfllCore.h"

#include <utils/RefBase.h> // android::RefBase

using android::sp;

namespace mfll {
class IMfllMemc : public android::RefBase {
public:
    static IMfllMemc* createInstance(void);
    virtual void destroyInstance(void);

/* interfaces */
public:
    /**
     *  Init IMfllMemc instance, the resolution should be set before invoked
     *  IMfllMemc::init
     *  @param nvramProvider    Memc needs a NVRAM provider
     *  @return                 If ok returns MfllErr_Ok
     */
    virtual enum MfllErr init(sp<IMfllNvram> &nvramProvider) = 0;

    /**
     *  To query the working buffer size of algorthim usage. This function only
     *  works after IMfllMemc::init invoked
     *  @param [out] size   Size in bytes
     *  @return             If ok returns MfllErr_Ok, or size won't be updated
     */
    virtual enum MfllErr getAlgorithmWorkBufferSize(size_t *size) = 0;

    /**
     *  Caller (MfllCore) will invoke this method to set itemself
     *  @param c            The pointer of MfllCore
     *  @return             Should returns OK if works well
     */
    virtual enum MfllErr setMfllCore(IMfllCore *c) = 0;

    /**
     *  Provides motion vector value to MEMC
     *  @param x            Horizontal vector
     *  @param y            Vertical vector
     */
    virtual void setMotionVector(int x, int y) = 0;

    /**
     *  Set the resolution of Motion Estimation
     *  @param w            Width, in pixel
     *  @param h            Height, in pixel
     */
    virtual void setMotionEstimationResolution(const int &w, const int &h) = 0;

    /**
     *  Set the resolution of Motion Compensation
     *  @param w            Width, in pixel
     *  @param h            Height, in pixel
     */
    virtual void setMotionCompensationResolution(const int &w, const int &h) = 0;

    /**
     *  Set the working buffer for algorthim. The buffer size should be queried
     *  by getAlgorithmWorkBufferSize.
     *  @param buffer       A continuous memory buffer with the correct size
     *  @note               The buffer will be release (using android::sp::decStrong)
     *                      after Motion Compensation has been finished.
     */
    virtual void setAlgorithmWorkingBuffer(sp<IMfllImageBuffer> &buffer) = 0;

    /**
     *  Set the base image for ME
     *  @param img          Base image buffer, it will be released (using
     *                      android::sp::decStrong) after finished
     */
    virtual void setMeBaseImage(sp<IMfllImageBuffer> &img) = 0;

    /**
     *  Set the reference image for ME
     *  @param img          Reference image buffer, it will be release (using
     *                      android::sp::decStrong) after this function done
     */
    virtual void setMeRefImage(sp<IMfllImageBuffer> &img) = 0;

    /**
     *  Set the image to be compensated
     *  @param img          An image to be compensated, it will be release (using
     *                      android::sp::decStrong) after this function done
     */
    virtual void setMcRefImage(sp<IMfllImageBuffer> &img) = 0;

    /**
     *  Set the output image to contain the compensated image
     *  @param img          An image to receive the compensated image, this image
     *                      will be released (using android::sp::decStrong) after
     *                      this function done
     */
    virtual void setMcDstImage(sp<IMfllImageBuffer> &img) = 0;

    /**
     *  Set the thread number that to run Motion Compensation if algorthim supported
     *  @param thread_num   Thread number
     *  @note               This method only can be invoked before init or it won't
     *                      effect
     */
    virtual void setMcThreadNum(const int &thread_num) = 0;

    /**
     *  Do Motion Estimation, and check if the motion is really bad. If yes, the
     *  motionCompensation will do nothing and returns immediately. You can invoke
     *  isIgnoredMotionCompensation to get the information after this function done
     *  @return             Returns MfllErr_Ok if ok.
     *  @note               Image buffers should be ready before invoking
     *  @sa                 isIgnoredMotionCompensation
     */
    virtual enum MfllErr motionEstimation() = 0;

    /**
     *  Do Motion Compensation, if the Motion Estimation reports it should not
     *  to apply compensation, this function will return immediately.
     *  @return             Returns MfllErr_Ok if ok.
     *  @note               Image buffers should be ready before invoking
     *  @sa                 isIgnoredMotionCompensation
     */
    virtual enum MfllErr motionCompensation() = 0;

    /**
     *  To check if the motion compensation has been ignored or not.
     *  @return             If the MC has been ignored or not
     *  @retval 0           Not
     *  @retval 1           ignored
     */
    virtual int isIgnoredMotionCompensation() = 0;

    /**
     *  If an unexpected error occurs in motion estimation, use this function
     *  to give up (ignore) motion compensation
     */
    virtual void giveupMotionCompensation() = 0;

    /**
     *  Since MEMC v1.1, confidence map buffer is necessary for MEMC algorthim.
     *  @param map          Image buffer for MEMC.
     *  @note               This API is supported since MEMC v1.1
     */
    virtual void setConfidenceMapImage(sp<IMfllImageBuffer>& map) = 0;

    /**
     *  Since MEMC v1.3, motion compensation mv buffer is necessary for MEMC algorthim.
     *  @param map          Image buffer for MEMC.
     *  @note               This API is supported since MEMC v1.3
     */
    virtual void setMotionCompensationMvImage(sp<IMfllImageBuffer>& mv) = 0;

    /**
     *  Since MEMC v1.1, blend type can be set to MEMC algorthim. For some situation,
     *  MEMC mode is necessary to be changed.
     *  @param type         Type of blend mode for MEMC algorthim.
     *  @note               This API is supported since MEMC v1.1
     */
    virtual void setBlendType(int type) = 0;

    /**
     *  Since MEMC v1.1, the current ISO info is necessary to MEMC algorthim.
     *  @param iso          The real ISO value.
     *  @note               This API is supported since MEMC v1.1
     */
    virtual void setCurrentIso(int iso) = 0;

     /**
     *  Since MEMC v1.3, the memc index info is necessary to MEMC bit true.
     *  @param p            The pipeline uniquekey for dump.
     *  @param r            The pipeline request number for dump.
     *  @param f            The pipeline frame number for dump.
     *  @note               This API is supported since MEMC v1.3
     */
    virtual void setMemcDump(int p, int r, int f) = 0;

    /**
     *  Since MEMC v1.2, To get the denominator value for calcualte Me size.
     *  @return             return the denominator value for Me size.
     *  @note               This API is supported since MEMC v1.2
     */
    virtual int32_t getMeDnRatio() = 0;

    /**
     *  Since MEMC v1.3, To get the frame level confidence from ME.
     *  @return             return the frame level confidence from ME.
     *  @note               This API is supported since MEMC v1.3
     */
    virtual int32_t getMeFrameLevelConfidence() = 0;


protected:
    virtual ~IMfllMemc() { };
};
}; /* namespace mfll */

#endif//__IMFLLMEMC_H__

