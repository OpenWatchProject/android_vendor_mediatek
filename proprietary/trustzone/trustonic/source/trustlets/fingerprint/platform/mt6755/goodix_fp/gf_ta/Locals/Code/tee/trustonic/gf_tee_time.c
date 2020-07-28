#include <taStd.h>
#include <tee_internal_api.h>
#include "gf_tee_internal_api.h"
#include "gf_dr.h"

#define LOG_TAG "[gf_tee_time]"

void gf_sleep(uint32_t milliseconds) {
    gf_dr_sleep(milliseconds);
}

gf_error_t gf_get_timestamp(uint64_t *timestamp) {
    return  gf_dr_get_timestamp(timestamp);
}
