#ifndef __IMFLLCAPTURER_H__
#define __IMFLLCAPTURER_H__

#include "MfllDefs.h"
#include "MfllTypes.h"
#include "IMfllEvents.h"
#include "IMfllImageBuffer.h"

#include <utils/RefBase.h> // android::RefBase
#include <vector>

/* give a symbol here avoid nested include */
namespace mfll {
    class IMfllCore;
};

namespace mfll {
/**
 *  MfllCapturer has responsibility to provide YUV and QYUV frames for MfllCore.
 *  Usually, capturer should accept RAW buffers from outside, so here we provide
 *  an interface: IMfllCapturer::queueSourceImage to queue full size RAW to
 *  capturer which should be thread-safe function.
 *
 *  Furthurmore, IMfllCapturer has responsibility to invoke
 *  MfllCore::doAllocRawBuffer
 *  MfllCore::doAllocQyuvBuffer
 *  while capturing in the implementation
 */
class IMfllCapturer : public android::RefBase {
public:
    enum class Type
    {
        RawCapturer,
        YuvCapturer,
    };

public:
    /**
     *  Create Instance, IMfllCapturer provides different type of capturer for
     *  different purposes.
     *
     *  @param t                    - Type of capturer to be created.
     *  @retrun                     - Instance of IMfllCapturer, if the type of
     *                                capturer is not supported, this method
     *                                returns nullptr.
     */
    static IMfllCapturer* createInstance(Type t = Type::RawCapturer);

    /**
     *  Destroy instance using android::RefBase::decStrong(void*)
     */
    virtual void destroyInstance(void);

public:
    /**
     *  Capture frames with synchronized call. The caller thread will be blocked until
     *  frame capturing has been done.
     *
     *  @param[out] raws             - A container will contain captured frames if returns MfllErr_Ok.
     *                                 Caller have to prepare IMfllImageBuffer with the same width and
     *                                 height to capture.
     *  @param[out] rrzos            - A container will contain captured resized raw frames.
     *  @param[out] gmvs             - A container will contain global motion vector information.
     *  @param[out] status           - A container contains results of every frame. If failed, the status
     *                                 will be non-zero
     *  @return                      - If succeed returns MfllErr_Ok.
     *  @note                        - After invoked this method and returns Ok, the frame number will
     *                                 be updated. Invoke getCapturedFrameNum to retrieve frame number.
     *                                 All vector size is supposed to be the same but size of parameter
     *                                 "gmvs" should be (n-1).
     */
    virtual enum MfllErr captureFrames(
        unsigned int captureFrameNum,
        std::vector< sp<IMfllImageBuffer>* >    &raws,
        std::vector< sp<IMfllImageBuffer>* >    &rrzos,
        std::vector< MfllMotionVector_t* >      &gmvs,
        std::vector<int>                        &status
    ) = 0;

    /**
     *  Capture frames with synchronized call. The caller thread will be blocked until
     *  frame capturing has been done.
     *
     *  @param[out] raws             - A container will contain captured frames if returns MfllErr_Ok.
     *                                 Caller have to prepare IMfllImageBuffer with the same width and
     *                                 height to capture.
     *  @param[out] rrzos            - A container will contain captured resized raw frames.
     *  @param[out] qyuvs            - A container will contain captured framees in QYUV format.
     *  @param[out] gmvs             - A container will contain global motion vector information.
     *  @param[out] status           - A container contains results of every frame. If failed, the status
     *                                 will be non-zero
     *  @return                      - If succeed returns MfllErr_Ok.
     *  @note                        - After invoked this method and returns Ok, the frame number will
     *                                 be updated. Invoke getCapturedFrameNum to retrieve frame number.
     *                                 All vector size is supposed to be the same but size of parameter
     *                                 "gmvs" should be (n-1).
     */
    virtual enum MfllErr captureFrames(
        unsigned int captureFrameNum,
        std::vector< sp<IMfllImageBuffer>* >    &raws,
        std::vector< sp<IMfllImageBuffer>* >    &rrzos,
        std::vector< sp<IMfllImageBuffer>* >    &qyuvs,
        std::vector< MfllMotionVector_t* >      &gmvs,
        std::vector<int>                        &status
    ) = 0;

    /**
     *  Capture frames with synchronized call. The caller thread will be blocked until
     *  frame capturing has been done.
     *  @param[out] raws
     *  @param[out] rrzos
     *  @param[out] yuvs
     *  @param[out] qyuvs
     *  @param[out] gmvs
     *  @param[out] status           - A container contains results of every frame. If failed, the status
     *                                 will be non-zero
     *  @return
     *  @note
     */
    virtual enum MfllErr captureFrames(
        unsigned int captureFrameNum,
        std::vector< sp<IMfllImageBuffer>* >    &raws,
        std::vector< sp<IMfllImageBuffer>* >    &rrzos,
        std::vector< sp<IMfllImageBuffer>* >    &yuvs,
        std::vector< sp<IMfllImageBuffer>* >    &qyuvs,
        std::vector< MfllMotionVector_t* >      &gmvs,
        std::vector<int>                        &status
    ) = 0;

    /**
     *  Capture frames with synchronized call. The caller thread will be blocked until
     *  frame capturing has been done.
     *  @param[in]  requestNo
     *  @param[out] yuvs
     *  @param[out] qyuvs
     *  @param[out] gmvs
     *  @param[out] status           - A container contains results of every frame. If failed, the status
     *                                 will be non-zero
     *  @return
     *  @note
     */
    virtual enum MfllErr captureFrames(
        unsigned int requestNo,
        sp<IMfllImageBuffer>*                   yuvs,
        sp<IMfllImageBuffer>*                   qyuvs,
        MfllMotionVector_t*                     gmvs,
        int&                                    status
    )
    {
        /*default not supported*/
        return MfllErr_NotSupported;
    };

    /**
     *  Retrieve captured frame number.
     *
     *  @return                 - This method will be avaiable when frames are captured, or retruns
     *                            0 as initial state.
     */
    virtual unsigned int getCapturedFrameNum(void) = 0;

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
     *  MfllCapturer will dispatch events therefore the IMfllEvents is necessary to be set.
     *  Without setting event dispatcher, the related events won't be sent.
     *
     *  @param e                 - A strong pointer of IMfllEvents reference.
     *  @return                  - If succeed returns MfllErr_Ok.
     */
    virtual enum MfllErr registerEventDispatcher(const sp<IMfllEvents> &e) = 0;

    /**
     *  To queue the bayer source image to capturer if the source image buffer
     *  is provided from other module.
     *
     *  @param img              - IMfllImageBuffer contains bayer pattern source image.
     *  @return                 - If succeed returns MfllErr_Ok.
     */
    virtual enum MfllErr queueSourceImage(sp<IMfllImageBuffer> &img) = 0;

    /**
     *  To queue the mono source image to capturer if the bayer and mono image
     *  buffers are provided from other module.
     *
     *  @param img              - IMfllImageBuffer contains mono pattern source image.
     *  @return                 - If succeed returns MfllErr_Ok;
     */
    virtual enum MfllErr queueSourceMonoImage(sp<IMfllImageBuffer>& img) = 0;

    /**
     *  To queue GMV (Global Motion Vector) to capturer if the GMV is provided
     *  from other module.
     *  @param mv               - Gloval motion vector
     *  @return                 - If succeed returns MfllErr_Ok.
     */
    virtual enum MfllErr queueGmv(const MfllMotionVector_t &mv) = 0;


    /**
     *  To queue LCSO buffer to capturer for dump. This is not a necessary API,
     *  just for who want to dump LCSO. LCSO buffer will be assigned by the
     *  argument *param2 of the method IMfllEventListener::doneEvent with
     *  EventType_CaptureRaw.
     *
     *  For example:
     *      void MyEventListener::doneEvent(
     *          EventType           e,
     *          MfllEventStatus_t&  status,
     *          void*               pMfllCore,
     *          void*               param1,
     *          void*               param2
     *          )
     *      {
     *          switch (e) {
     *          case EventType_CaptureRaw:
     *              if (param2 != nullptr) {
     *                  IMfllImageBuffer* pLcsoRaw = static_cast<IMfllImageBuffer*>(param2);
     *                  // save LCSO raw
     *                  pLcsoRaw->saveFile(...);
     *              }
     *              else {
     *                  LOGD("LCSO doesn't exist, won't save it");
     *              }
     *          }
     *      }
     *
     *  @param img              - IMfllImageBuffer contains LCSO buffer.
     *  @return                 - If succeed returns MfllErr_Ok.
     *  @note                   - Please invoke this method before queueSourceImage.
     */
    virtual enum MfllErr queueSourceLcsoImage(sp<IMfllImageBuffer>& img) = 0;

    /**
     *  To queue RRZO buffer to capturer for raw domain bss and dump.
     *  This is not a necessary API,
     *  just for who want to dump RRZO. RRZO buffer will be assigned by the
     *  argument *param2 of the method IMfllEventListener::doneEvent with
     *  EventType_CaptureRaw.
     *
     *  For example:
     *      void MyEventListener::doneEvent(
     *          EventType           e,
     *          MfllEventStatus_t&  status,
     *          void*               pMfllCore,
     *          void*               param1,
     *          void*               param2
     *          )
     *      {
     *          switch (e) {
     *          case EventType_CaptureRaw:
     *              if (param2 != nullptr) {
     *                  IMfllImageBuffer* pRrzoRaw = static_cast<IMfllImageBuffer*>(param2);
     *                  // save RRZO raw
     *                  pRrzoRaw->saveFile(...);
     *              }
     *              else {
     *                  LOGD("RRZO doesn't exist, won't save it");
     *              }
     *          }
     *      }
     *
     *  @param img              - IMfllImageBuffer contains RRZO buffer.
     *  @return                 - If succeed returns MfllErr_Ok.
     *  @note                   - Please invoke this method before queueSourceImage.
     */
    virtual enum MfllErr queueSourceRrzoImage(sp<IMfllImageBuffer>& /*img*/) {/*not necessary API*/ return MfllErr_NotImplemented; };

protected:
    virtual ~IMfllCapturer(void){};
};
}; /* namespace mfll */
#endif//__IMFLLCAPTURER_H__

