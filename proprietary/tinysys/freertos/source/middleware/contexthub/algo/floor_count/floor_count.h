#ifndef FLOOR_COUNT
#define FLOOR_COUNT

#include <math.h>

#define HEIGHT_PER_FLOOR     3.0
#define CHECK_FLAT_RANGE     0.75
#define CHECK_FLAT_STEP      15
#define MIN_STEP_PER_FLOOR   5
#define SIZE_WIN             10
#define SIZE_FLAT            30
#define BARO_WIN_PERIOD      (long long int) 3000000000ul  // 3 sec
#define STILL_THRESHOLD      (long long int) 180000000000ul  // 180 sec

typedef struct baro_struct {
    long long int time_stamp;
    unsigned int step_count;
    double data;
} baro_struct;

typedef struct floor_count_struct {
    baro_struct win_queue[SIZE_WIN];  // for moving avg
    int win_front;
    int win_rear;
    int win_len;

    baro_struct flat_queue[SIZE_FLAT]; // for flat check
    int flat_front;
    int flat_rear;
    int flat_len;

    double base_pres;
    unsigned int base_step_count;
    long long int still_time;
    unsigned int floor_count;
} floor_count_struct;

void init_floor_counting();
void run_floor_counting(long long int time_stamp, int baro_input, unsigned int accumulated_step_count);
unsigned int get_floor_counting_result();
double baro_moving_avg();
int baro_flat_check();
void baro_enqueue(baro_struct *queue, int *front, int *rear, int *len, int size,
                  long long int time_stamp, double baro, unsigned int step_count);
void baro_dequeue(int *front, int *rear, int *len, int size);
double p2dh(double p1, double p2);

#endif
