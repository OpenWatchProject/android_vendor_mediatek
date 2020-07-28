#ifndef _GESTURE_FREEFALL_PREDICTOR_H_
#define _GESTURE_FREEFALL_PREDICTOR_H_

#include "rf_function.h"
#include "gesture_private.h"

void freefall_predictor();
void gesture_get_freefall_result_ptr(gesture_notify_result_t *gesture_results);

// Get ptr of gesture random forest parameter function
#ifndef GESTURE_TREE_NON
Tree *get_gesture_freefall_rf_para_ptr();
#else
Tree_None_Perfect *get_gesture_freefall_rf_para_ptr();
#endif

RandomForestSetting *get_gesture_freefall_rf_setting_ptr();

#endif
