#include "algoConfig.h"
#include <stdbool.h>
#include <string.h>

void init_data_resampling(resampling_t *resample, uint32_t input_sample_delay)
{
    memset(resample, 0, sizeof(resampling_t));
    resample->input_sample_delay   = input_sample_delay;
    resample->modify_sample_scheme = 0;
}

void sensor_input_data_resampling_preparation(resampling_t *resample, uint32_t input_time_stamp_ms, uint32_t evtType)
{
    uint32_t ref_algo_time_stamp = 0;
    // reset result and condition
    resample->input_count = 0;
    resample->no_timestamp_exception = TS_NORMAL;
    resample->do_interpolation = 0;

    //resample initialization
    if (resample->init_done) {
        resample->last_time_stamp = resample->current_time_stamp;
        if (evtType == EVT_SENSOR_ACCEL) {
            resample->lastAccData = (AccelData_t) {
                resample->currentAccData.x, resample->currentAccData.y, resample->currentAccData.z
            };
        } else if (evtType == EVT_SENSOR_BARO) {
            resample->lastBaroData = resample->currentBaroData;
        }
    } else {
        resample->last_time_stamp = input_time_stamp_ms;
        resample->algo_time_stamp = resample->last_time_stamp;
        resample->init_done = 1;
    }
    resample->current_time_stamp = input_time_stamp_ms + resample->ts_rollback_amount;
    ref_algo_time_stamp = resample->algo_time_stamp;

    // resample data with a time interval of resample->input_sample_delay
    bool has_new_algo_data = (resample->current_time_stamp >= (ref_algo_time_stamp + resample->input_sample_delay));
    bool is_rollback = (resample->current_time_stamp < resample->last_time_stamp);

    if (has_new_algo_data) {
        do {
            resample->input_count++;
            ref_algo_time_stamp += resample->input_sample_delay;

            if (resample->input_count > INTERPOLATION_UPPER_LIMIT) {
                resample->no_timestamp_exception = TS_LARGE_GAP;
                resample->input_count = 1;
                resample->algo_time_stamp = resample->current_time_stamp;
                break;
            }
        } while ((resample->current_time_stamp >= (ref_algo_time_stamp + resample->input_sample_delay)));
    } else if (is_rollback) {
        resample->no_timestamp_exception = TS_ROLL_BACK;
        resample->ts_rollback_amount += resample->last_time_stamp - resample->current_time_stamp + resample->input_sample_delay;
        resample->current_time_stamp = resample->last_time_stamp + resample->input_sample_delay;
        resample->input_count = 1;
    }

    if (resample->input_count >= INTERPOLATION_LOWER_LIMIT) {
        resample->do_interpolation = 1;
    }

}

void algo_input_data_generation(resampling_t *resample, uint32_t evtType)
{
    bool is_close_to_current = false;
    int32_t current_data_ratio = 0;
    int32_t last_data_ratio = 0;

    if (resample->no_timestamp_exception) {
        resample->algo_time_stamp += resample->input_sample_delay;
    }

    if (resample->do_interpolation) {
        current_data_ratio = ((resample->algo_time_stamp - resample->last_time_stamp) * INTERPOLATION_RATIO_SCALE) /
                             (resample->current_time_stamp - resample->last_time_stamp);
    } else {
        is_close_to_current = ((resample->algo_time_stamp - resample->last_time_stamp) >= (resample->current_time_stamp -
                               resample->algo_time_stamp));
        if (resample->modify_sample_scheme) {
            is_close_to_current = (resample->current_time_stamp == resample->algo_time_stamp) ? 1 : 0;
        }
        current_data_ratio = is_close_to_current ? INTERPOLATION_RATIO_SCALE : 0;
    }
    last_data_ratio = INTERPOLATION_RATIO_SCALE - current_data_ratio;

    if (evtType == EVT_SENSOR_ACCEL) {
        resample->algoAccData.x = (last_data_ratio * resample->lastAccData.x + current_data_ratio * resample->currentAccData.x)
                                  / INTERPOLATION_RATIO_SCALE;
        resample->algoAccData.y = (last_data_ratio * resample->lastAccData.y + current_data_ratio * resample->currentAccData.y)
                                  / INTERPOLATION_RATIO_SCALE;
        resample->algoAccData.z = (last_data_ratio * resample->lastAccData.z + current_data_ratio * resample->currentAccData.z)
                                  / INTERPOLATION_RATIO_SCALE;
    } else if (evtType == EVT_SENSOR_BARO) {
        resample->algoBaroData = (last_data_ratio * resample->lastBaroData + current_data_ratio * resample->currentBaroData) /
                                 INTERPOLATION_RATIO_SCALE;
    }
}

int32_t check_if_same_input(struct TripleAxisDataEvent *ev, uint32_t last_time_stamp, int32_t data_count)
{
    int32_t same_data = 0;
    struct TripleAxisDataPoint *sample;
    uint64_t time_stamp_ms = ev->referenceTime;
    uint32_t count = ev->samples[0].firstSample.numSamples;
    uint32_t i;
    if (count >= 1) {
        for (i = 0; i < count; i++) {
            sample = &ev->samples[i];
            if (i > 0)
                time_stamp_ms += sample->deltaTime;
        }
        if ((time_stamp_ms <= last_time_stamp + 10) && (time_stamp_ms > last_time_stamp - 10)) {
            same_data = 1;
        }
    }
    return same_data;
}
