#pragma once
#ifndef __SEQUENTIAL_TESTING__
#define __SEQUENTIAL_TESTING__
#include <stdio.h>

#define LIKELIHOOD_BUFFER_DEPTH 150

typedef struct {
    int32_t m;
    int32_t b_l;
    int32_t b_r;
    int32_t offset;
} FlickerStatistics;

typedef enum { HZ50 = 0, HZ60 = 1 } EvTable;

//#define MIN_PAST_FRAMES 3
//#define MAX_PAST_FRAMES 14

typedef struct {
    int32_t vals[LIKELIHOOD_BUFFER_DEPTH];
    int32_t front;
    int32_t count;
} LlBuffer;

FlkStatus test_next_frame(const int32_t *frame_data, uint32_t dim, int8_t is_valid, uint32_t n_history);
void reset_flicker_queue();
void set_flicker_state(EvTable val);
int32_t get_slrt_score(uint32_t history_length);
int32_t get_current_llr(const int32_t *frame_data, uint32_t dim);

#ifdef FLK_PC_SIMULATION
void set_frame_idx_seq(int32_t frame_idx);
void set_file_ptr_seq(FILE *fptr);
#endif

void set_log(int32_t state);

// ========== s5k2x5 ==========
FlkStatus test_next_frame_2x5(const int32_t *frame_data, uint32_t dim, int8_t is_valid, uint32_t n_history, int32_t current_llr);

#endif
