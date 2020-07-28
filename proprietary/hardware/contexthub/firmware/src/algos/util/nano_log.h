#ifndef LOCATION_LBS_CONTEXTHUB_NANOAPPS_UTIL_NANO_LOG_H_
#define LOCATION_LBS_CONTEXTHUB_NANOAPPS_UTIL_NANO_LOG_H_

#ifdef GOOGLE3
#include <stdio.h>

// ignore log level argument
#define LOG_FUNC(level, ...) printf(__VA_ARGS__)
#endif

#include "third_party/contexthub/nanoapps/util/log/log.h"

#endif  // LOCATION_LBS_CONTEXTHUB_NANOAPPS_UTIL_NANO_LOG_H_
