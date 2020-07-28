#ifndef _DATA_BUFFER_H_
#define _DATA_BUFFER_H_

#include "feature_struct.h"
#include "context_setting_private.h"
#include "gesture_setting_private.h"

/* Init aggregation and signal buffer size */
AccSignals_t *get_acc_buffer_ptr();
BaroSignals_t *get_baro_buffer_ptr();
Feature_t *get_context_feature_buffer_ptr();
Feature_t *get_gesture_feature_buffer_ptr();
Feature_t *get_fall_feature_buffer_ptr();

#endif
