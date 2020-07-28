#ifndef __IAINRCORE_H__
#define __IAINRCORE_H__

#include "AinrDefs.h"
#include "AinrTypes.h"
#include "IAinrNvram.h"
#include <utils/RefBase.h> // android::RefBase


namespace ainr {

enum AINRCORE_VERSION {
    AINRCORE_VERSION_0_0 = 0x00000000,
    AINRCORE_VERSION_1_0 = 0x00010000,
    AINRCORE_VERSION_2_0 = 0x00020000,
};

class IAinrCore {
public:
    /**
     *  Caller should always create instance via INTERFACE::createInstance method.
     *  And caller also have responsibility to destroy it by invoking destroyInstance.
     *  If caller uses android::sp to manager life cycle of instance who doesn't need to invoke
     *  destroyInstance to destroy instance.
     *
     *  @return             - An IAinrCore instance, caller has responsibility to manager it's lifetime.
     */
    static std::shared_ptr<IAinrCore> createInstance();

public:
    /**
     *  Init AinrCore, provide basic information such as buffer resolution to allocate
     *  to allocate working buffer acyncly
     *  @param cfg          - AinrConfig_t structure to describe usage
     *  @return             - Returns AinrErr_Ok if works
     */
    virtual enum AinrErr init (const AinrConfig_t &cfg) = 0;

    virtual enum AinrErr doAinr() = 0;

    /**
     *  Tell MFLL to cancel this operation
     */
    virtual enum AinrErr doCancel() = 0;

    /**
     *  User may also set a Nvram Provider to AinrCore. If Nvram Provider hasn't been set,
     *  AinrCore will invoke IAinrNvram::createInstance to create IAinrNvram instance when need.
     *
     *  @param nvramProvider - A strong pointer contains reference of IAinrNvram instance.
     *  @return             - If ok returns AinrErr_Ok
     *  @note               - This operation is thread-safe
     */
    virtual enum AinrErr setNvramProvider(const std::shared_ptr<IAinrNvram> &nvramProvider) = 0;


    /**
     *  addd buffer & metadata to ainr core
     *  @param inputPack    - Structure of buffers and metadata
     *  @return             - If OK returns AinrErr_Ok.
     */
     virtual enum AinrErr addInput(
            const std::vector<AinrPipelinePack>&     inputPack
            ) = 0;

    /**
     *  addd outputbuffer ainr core
     *  @param output    - Structure of buffers and metadata
     *  @return             - If OK returns AinrErr_Ok.
     */
     virtual enum AinrErr addOutput(
            NSCam::IImageBuffer *outBuffer
            ) = 0;

    /**
     *  Query whether algo is supported for specific resolution
     *  @NSCam::MSize size  - Input image size
     *  @return             - If support returns true.
     */
     virtual bool queryAlgoSupport(NSCam::MSize size) = 0;

    /**
     *  Register callback function for core indicating next caputre
     *  @std::function<void(int32_t, int32_t)> cb  - function(uniqueKey, mainFrame)
     *  @return             - return void.
     */
     virtual void registerCallback(std::function<void(int32_t, int32_t)> cb) = 0;

     /**
     * Query IAinrCore version
     * @return AINRCORE_VERSION enumeration.
     */
    virtual AINRCORE_VERSION queryVersion() const = 0;

     /**
     *  Query main frame index assigned by core
     *  @return             - Main frame index.
     */
     virtual int32_t queryMainFrameIndex() const = 0;

protected:
    virtual ~IAinrCore(void) {};

};  // class IAinrCore
};  // namespace ainr
#endif
