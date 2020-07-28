#ifndef __PERFORMANCE_H__
#define __PERFORMANCE_H__

#include <unistd.h>
#include <linux/hwmsensor.h>
#include <linux/types.h>
#include <log/log.h>

struct time_records {
    int64_t sum_hal_time;
    unsigned int count;
};

#define LIMIT 1000

//#define DEBUG_PERFORMANCE

#ifdef DEBUG_PERFORMANCE
static struct time_records record;
static inline void mark_timestamp(unsigned char sensor_type, int64_t current_time, int64_t event_time)
{
    record.sum_hal_time +=  current_time - event_time;
    record.count++;
    if (record.count == LIMIT) {
        ALOGE("sensor[%d] ====> hal time:%" PRId64 "\n", sensor_type,
            record.sum_hal_time / LIMIT);
        record.sum_hal_time = 0;
        record.count = 0;
    }
}
#else
#define mark_timestamp(A, B, C)
#endif

#endif
