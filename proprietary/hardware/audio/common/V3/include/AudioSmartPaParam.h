#ifndef ANDROID_AUDIO_SMART_PA_PARAM_H
#define ANDROID_AUDIO_SMART_PA_PARAM_H

#include "AudioLock.h"
#include "AudioALSADriverUtility.h"
#if defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
#include <arsi_type.h>
#endif

#define SMARTPA_STR_LENGTH (256)

/* string should comapre with prefix + vendor + suffix*/
const char smartpa_aurisys_set_param_prefix[] = "AURISYS_SET_PARAM,DSP,PLAYBACK,SMARTPA";
const char smartpa_aurisys_get_param_prefix[] = "AURISYS_GET_PARAM,DSP,PLAYBACK,SMARTPA";

#if defined(MTK_AURISYS_FRAMEWORK_SUPPORT)
struct SmartPAParamOps {
    int (*loadParam)(const string_buf_t *product_name,
                     const string_buf_t *param_file_path,
                     const debug_log_fp_t debug_log_fp);
    int (*queryParamSize)(const arsi_task_config_t *p_arsi_task_config,
                          const arsi_lib_config_t *p_arsi_lib_config,
                          const string_buf_t *product_name,
                          const string_buf_t *param_file_path,
                          const string_buf_t *custom_info,
                          uint32_t *p_param_buf_size,
                          const debug_log_fp_t debug_log_fp);
    int (*parsingParamFile)(const arsi_task_config_t *p_arsi_task_config,
                            const arsi_lib_config_t *p_arsi_lib_config,
                            const string_buf_t *product_name,
                            const string_buf_t *param_file_path,
                            const string_buf_t *custom_info,
                            data_buf_t *p_param_buf,
                            const debug_log_fp_t debug_log_fp);
};
#else
struct SmartPAParamOps;
#endif

struct ipi_msg_t;

namespace android {

class AudioMessengerIPI;

class AudioSmartPaParam {

public:
    ~AudioSmartPaParam();

    int setParameter(const char *keyValuePair);
    char *getParameter(const char *key);


    /**
     * get instance's pointer
     */
    static AudioSmartPaParam *getInstance(void);

    int setArsiTaskConfig(const arsi_task_config_t * ArsiTaskConfig);
    int setArsiLibConfig(const arsi_lib_config_t * mArsiLibConfig);
    int setSmartpaParam();
    int setParamFilePath(const char *str);

protected:

    AudioSmartPaParam();
    static void processSmartPaDmaMsg(ipi_msg_t *msg, void *buf, uint32_t size, void *arg);

private:
    static AudioSmartPaParam *mAudioSmartPaParam;
    AudioMessengerIPI *mAudioMessengerIPI;
    void parseSetParameterStr(const char *inputStr, char **outputStr,
                              const int paramindex);
    bool checkParameter(int &paramindex, int &direction, const char *keyValuePair);
    int getsetParameterPrefixlength(int paramindex);
    int getgetParameterPrefixlength(int paramindex);
    int setProductName(const char *str);
    char *getParamFilePath(void);
    char *getProductName(void);
    int getDefalutParamFilePath(void);
    int getDefaultProductName(void);


    void initArsiTaskConfig(void);
    void initArsiLibconfig(void);

    char mSmartParamFilePath[SMARTPA_STR_LENGTH];
    char mPhoneProductName[SMARTPA_STR_LENGTH];

    arsi_task_config_t *mArsiTaskConfig;
    arsi_lib_config_t *mArsiLibConfig;
    data_buf_t      *mParamBuf;

    bool mEnableLibLogHAL;

    void *mLibHandle;
    struct SmartPAParamOps mSmartPAParam;
    int (*mtk_smartpa_param_init)(struct SmartPAParamOps *mSmartPAParam);

};

}
#endif
