#ifndef __IMFLLMFB_H__
#define __IMFLLMFB_H__

#include "MfllDefs.h"
#include "MfllTypes.h"
#include "IMfllEvents.h"
#include "IMfllImageBuffer.h"

#include <utils/RefBase.h> // android::RefBase

#include <deque> // std::deque

namespace mfll {

/**
 *  IMfllMfb is a class which is resonsibility to do some operations of Pass 2.
 *      1. blending
 *      2. mixing
 *      3. color space converting
 *      4. image cropping/scaling/rotating(90 degree)
 *  For these operations, caller has to set sensor ID first (using IMfllMfb::init),
 *  and perhaps it's necessary to set the sync private data.
 *
 *  @note   Pass 2 driver is NOT a thread-safe mechanism which means if you process
 *          the operation in different thread may cause race condition and occur
 *          unexpected error. Therefore, IMfllMfb has done the job that make all
 *          IMfllMfb instances won't process the operation at the same time to avoid
 *          race condition of Pass 2 driver.
 */
class IMfllMfb : public android::RefBase {
public:
    static IMfllMfb* createInstance(void);
    virtual void destroyInstance(void);

public:
    /**
     *  Init IMfllMfb module
     *
     *  @note               - You don't need to to unint this module it will be unit in destructor.
     */
    virtual enum MfllErr init(int sensorId) = 0;

    /**
     *  Give reference of IMfllCore
     *  @param c                - IMfllCore address
     */
    virtual void setMfllCore(IMfllCore *c) = 0;

    /**
     *  Tell MfllMfb what mode is using now
     *  @param mode             - Mode to tell, default is MfllMode_NormalMfll
     */
    virtual void setShotMode(const enum MfllMode &mode) = 0;

    /**
     *  Tell MfllMfb the post NR type is using
     *  @param nrType           - NoiseReductionType
     */
    virtual void setPostNrType(const enum NoiseReductionType &nrType) = 0;

    /**
     *  Do image blending.
     *  Before invokes this method, you could prepare weighting memory for IMfllMfb, or
     *  IMfllMfb will invoke IMfllImageBuffer::createInstance to create. And IMfllImageBuffer::initBuffer
     *  is also invoked before use it.
     *
     *  @param base         - Base frame as input image.
     *  @param ref          - Reference frame as input image.
     *  @param out          - Blended image.
     *  @param wt_in        - Weighting table input, the first time to blend, set this argument as NULL
     *  @param wt_out       - Weighting table output.
     *  @note               - This function must be thread-safe.
     */
    virtual enum MfllErr blend(IMfllImageBuffer *base, IMfllImageBuffer *ref, IMfllImageBuffer *out, IMfllImageBuffer *wt_in, IMfllImageBuffer *wt_out) = 0;

    /**
     *  Do image blending with a confidence map (since MFNR v2.0)
     *  Before invokes this method, you could prepare weighting memory for IMfllMfb, or
     *  IMfllMfb will invoke IMfllImageBuffer::createInstance to create. And IMfllImageBuffer::initBuffer
     *  is also invoked before use it.
     *
     *  @param base         - Base frame as input image.
     *  @param ref          - Reference frame as input image.
     *  @param conf         - confidence map as input image.
     *  @param out          - Blended image.
     *  @param wt_in        - Weighting table input, the first time to blend, set this argument as NULL
     *  @param wt_out       - Weighting table output.
     *  @note               - This function must be thread-safe.
     */
    virtual enum MfllErr blend(
                             IMfllImageBuffer* base,
                             IMfllImageBuffer* ref,
                             IMfllImageBuffer* conf,
                             IMfllImageBuffer* out,
                             IMfllImageBuffer* wt_in,
                             IMfllImageBuffer* wt_out
                         ) = 0;

    /**
     *  Do image mixing.
     *  Before invokes this method, you could prepare weighting memory for IMfllMfb, or
     *  IMfllMfb will invoke IMfllImageBuffer::createInstance to create. And IMfllImageBuffer::initBuffer
     *  is also invoked before use it.
     *
     *  @param base         - Base frame as input image.
     *  @param ref          - Reference frame as input image.
     *  @param out          - Mixed frame image.
     *  @param wt           - Weighting table of the last blended output
     *  @note               - This function must be thread-safe.
     */
    virtual enum MfllErr mix(IMfllImageBuffer *base, IMfllImageBuffer *ref, IMfllImageBuffer *out, IMfllImageBuffer *wt) = 0;

    /**
     *  Do image mixing (and generate thumbnail in this stage).
     *  Before invokes this method, you could prepare weighting memory for IMfllMfb, or
     *  IMfllMfb will invoke IMfllImageBuffer::createInstance to create. And IMfllImageBuffer::initBuffer
     *  is also invoked before use it.
     *
     *  @param base               - Base frame as input image.
     *  @param ref                - Reference frame as input image.
     *  @param out_main           - Mixed frame image.
     *  @param out_thumbnail      - Mixed frame image.
     *  @param wt                 - Weighting table of the last blended output
     *  @param output_main_crop   - Cropping window of source image for main yuv output
     *  @param output_thumb_crop  - Cropping window of source image for tuumbnail yuv output
     *  @note                     - This function must be thread-safe.
     */
    virtual enum MfllErr mix(
            IMfllImageBuffer *base,
            IMfllImageBuffer *ref,
            IMfllImageBuffer *out_main,
            IMfllImageBuffer *out_thumbnail,
            IMfllImageBuffer *wt,
            const MfllRect_t& output_thumb_crop
            ) = 0;

    /**
     *  Encode RAW10 image to YUV serial image using pass 2 driver, like YUV2, YV16
     *  If the the resolutions are different between input and output buffers,
     *  a cropping will be applied and the ratio will be keeped automatically
     *
     *  @param input        - Input buffer
     *  @param output       - Output buffer
     *  @param s            - Different stage will relate to different ISP profile.
     *  @return             - Returns MfllErr_Ok if ok.
     *  @note               - The different ISP profile may be applied if the
     *                        NoiseReductionType is different.
     *  @sa                 setNoiseReductionType
     */
    virtual enum MfllErr encodeRawToYuv(IMfllImageBuffer *input, IMfllImageBuffer *output, const enum YuvStage &s) = 0;

    /**
     *  Encode RAW10 image to 2 YUV buffer (full size + quarter size).
     *  If the the resolutions are different between input and output buffers,
     *  a cropping will be applied and the ratio will be keeped automatically
     *  @param input        Input buffer, full size
     *  @param output       Output buffer, full size
     *  @param output_q     Output buffer, quarter size
     *  @param s            YuvStage should be YuvStage_RawToYuy2 or YuvStage_RawToYv16
     */
    virtual enum MfllErr encodeRawToYuv(
            IMfllImageBuffer *input,
            IMfllImageBuffer *output,
            IMfllImageBuffer *output_q,
            const MfllRect_t& output_crop,
            const MfllRect_t& output_q_crop,
            enum YuvStage s = YuvStage_RawToYuy2) = 0;

    /**
     *  Encode RAW image to 3 YUV buffer (a same size with source YUV, two resized
     *  possible output YUVs).
     *
     *  @param input        Input buffer, full size
     *  @param output       Output buffer, size must be the same as input
     *  @param output2      Output buffer2, resized possible
     *  @param output3      Output buffer3, resized possible
     *  @param output2_crop Cropping window of source image for output2
     *  @param output3_crop Cropping window of source image for output3
     *  @param s            YuvStage should be YuvStage_RawToYuy2 or YuvStage_RawToYv16
     */
    virtual enum MfllErr encodeRawToYuv(
            IMfllImageBuffer *input,
            IMfllImageBuffer *output,
            IMfllImageBuffer *output2,
            IMfllImageBuffer *output3,
            const MfllRect_t& output2_crop,
            const MfllRect_t& output3_crop,
            enum YuvStage s = YuvStage_RawToYuy2) = 0;


    /**
     *  To reduce P2 overhead, some YUV stage is only to change the arrangement of
     *  YUV format and which can be done by MDP.
     *  @param input        Input buffer, full size.
     *  @param output       Output buffer, the same size w/ input.
     *  @param output_q     Output buffer, the different size w/ input, can be NULL.
     *  @param output_crop  ROI for output base on input.
     *  @param output_q_crop ROI for output_q, base on input.
     *  @param s            YuvStage.
     */
    virtual enum MfllErr convertYuvFormatByMdp(
            IMfllImageBuffer* input,
            IMfllImageBuffer* output,
            IMfllImageBuffer* output_q,
            const MfllRect_t& output_crop,
            const MfllRect_t& output_q_crop,
            enum YuvStage s = YuvStage_Unknown) = 0;
    /**
     *  Set sync data for pass 1 and pass 2. This data should be get by IspSyncControl
     *
     *  @param dataset      - Private data container
     *  @return             - Returns MfllErr_Ok if ok.
     */
    virtual enum MfllErr setSyncPrivateData(const std::deque<void*>& dataset) = 0;

protected:
    virtual ~IMfllMfb(void){};
};
}; /* namespace mfll */

#endif//__IMFLLMFB_H__

