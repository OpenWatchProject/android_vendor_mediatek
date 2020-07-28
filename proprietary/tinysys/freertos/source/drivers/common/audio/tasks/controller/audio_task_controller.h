#ifndef AUDIO_TASK_CONTROLLER_H
#define AUDIO_TASK_CONTROLLER_H

#include <audio_task_interface.h>

AudioTask *task_controller_new();
void task_controller_delete(AudioTask *task);

#endif /* end of AUDIO_TASK_CONTROLLER_H */

