#ifndef _INIT_LEARNING_H_
#define _INIT_LEARNING_H_
#include <stdint.h>

typedef struct gesture_initializer {
    int enable_shake;      // 0: off, 1: on
    int enable_flip;       // 0: off, 1: on
    int enable_pick_up;    // 0: off, 1: on
    int enable_snapshot;   // 0: off, 1: on
    int enable_answercall; // 0: off, 1: on
    int enable_tilt;       // 0: off, 1: on
} gesture_initializer_t;

void activity_algorithm_init(uint32_t init_time);
void gesture_algorithm_init(uint32_t init_time);

#endif
