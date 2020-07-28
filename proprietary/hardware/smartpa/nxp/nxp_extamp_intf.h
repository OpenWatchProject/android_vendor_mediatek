#ifndef __NXP_EXTAMP_INTF_H
#define __NXP_EXTAMP_INTF_H

#include <AudioSmartPaController.h>

#ifdef __cplusplus
extern "C" {
#endif /* #ifdef __cplusplus */

/* Function:
 *     mtk_smartpa_init
 * Description:
 *     3rd party SmartPA need to define the interface for MTK platform.
 * Parameter:
 *     struct SmartPa includes struct SmartPaOps and struct SmartPaRuntime.
 *     struct SmartPaOps provides interface:
 *         int (*init)(struct SmartPa *smartPa);
 *         int (*speakerOn)(struct SmartPaRuntime *runtime);
 *         int (*speakerOff)();
 *         int (*deinit)();
 *     struct SmartPaRuntime provides parameter:
 *         unsigned int sampleRate;
 *         int echoReferenceConfig;
 *         int mode; --> normal = 0, ringtone = 1, phone call = 2, VOIP = 3
 *         int device;  --> speaker = 0, receiver = 1
 */

int mtk_smartpa_init(struct SmartPa *smart_pa);
#ifdef __cplusplus
}
#endif /* #ifdef __cplusplus */
#endif /* #ifndef __NXP_EXTAMP_INTF_H */
