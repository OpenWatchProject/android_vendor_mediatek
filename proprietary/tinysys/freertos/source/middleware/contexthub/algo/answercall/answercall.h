#ifndef _ANSWERCALL_H_
#define _ANSWERCALL_H_

#define A_C_30_GRAV            490000
#define A_C_45_GRAV            692964
#define A_C_SMALL_MOTION_T     25      // calculate 0.5 s to assure the motion
#define A_C_PREV_HOLD_TIME     1000    //answer call
#define A_C_NEXT_HOLD_TIME     2000
#define A_C_TWO_TRIGGER_TS     2000    // minimum interval between two answer calls
#define A_C_TS_OVERFLOW        5000
#define A_C_MIN_X_STD          20000   // minimum x std to trigger answer call
#define A_C_MIN_Y_STD          50000   // minimum y std to trigger answer call

// close_ear: proximity 0->1
typedef struct answer_call_adaptor {
    uint32_t last_set_time;
    uint32_t last_pick_up_time;
    uint32_t last_approach_ear_time; // proximity 0~1

    uint32_t dist_of_prox;
    uint32_t pick_up_state;
    int32_t acc_x, acc_y, acc_z;
    int32_t acc_x_std, acc_y_std;
} answer_call_adaptor_t;

int *get_answercall_result_ptr(uint8_t);
void answercall_set_ts(uint32_t);
void answercall_set_prox(uint32_t input);
void answercall_alg_enter_point();

#endif
