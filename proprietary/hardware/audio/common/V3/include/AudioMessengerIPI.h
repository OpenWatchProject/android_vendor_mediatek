#ifndef ANDROID_MESSENGER_IPI_H
#define ANDROID_MESSENGER_IPI_H

#include <stdint.h>
#include <stdbool.h>

#include <AudioLock.h>
#include <AudioType.h>

#include <audio_messenger_ipi.h>

unsigned int getDspFeatureID(const uint16_t flag);


namespace android {

class AudioMessengerIPI {
public:
    virtual ~AudioMessengerIPI();
    static AudioMessengerIPI *getInstance();


    virtual void loadTaskScene(const uint8_t task_scene);


    virtual status_t sendIpiMsg(
        struct ipi_msg_t *p_ipi_msg,
        uint8_t task_scene, /* task_scene_t */
        uint8_t target_layer, /* audio_ipi_msg_target_layer_t */
        uint8_t data_type, /* audio_ipi_msg_data_t */
        uint8_t ack_type, /* audio_ipi_msg_ack_t */
        uint16_t msg_id,
        uint32_t param1, /* data_size for payload & dma */
        uint32_t param2,
        void    *data_buffer); /* buffer for payload & dma */


    virtual void registerDmaCbk(
        const uint8_t task_scene,
        const uint32_t a2dSize,
        const uint32_t d2aSize,
        audio_ipi_dma_cbk_t cbk,
        void *arg);
    virtual void deregisterDmaCbk(const uint8_t task_scene);


    virtual void registerAdspFeature(const uint16_t feature_id);
    virtual void deregisterAdspFeature(const uint16_t feature_id);



protected:
    AudioMessengerIPI();

    AudioLock mLock;



private:
    /**
     * singleton pattern
     */
    static AudioMessengerIPI *mAudioMessengerIPI;
};

} // end namespace android

#endif // end of ANDROID_MESSENGER_IPI_H
