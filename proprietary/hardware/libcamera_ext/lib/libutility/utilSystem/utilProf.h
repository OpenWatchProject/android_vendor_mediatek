#ifndef _UTIL_PROF_H_
#define _UTIL_PROF_H_

#include <time.h>
#if defined(__linux__) || defined(__ANDROID__) || defined(__QNX__) || defined(__APPLE__) || defined(__CYGWIN__)
#include <dlfcn.h>
#include <semaphore.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>
#elif defined(_WIN32) || defined(UNDER_CE)
#include <windows.h>
#endif
#define UTIL_FMT_TIME   "%9.3f"
#include "MTKUtilCommon.h"

class utilPerf
{
public:
    utilPerf operator+(const utilPerf &in);
    MUINT64 tmp;          /*!< \brief Holds the last measurement. */
    MUINT64 beg;          /*!< \brief Holds the first measurement in a set. */
    MUINT64 end;          /*!< \brief Holds the last measurement in a set. */
    MUINT64 sum;          /*!< \brief Holds the summation of durations. */
    MUINT64 avg;          /*!< \brief Holds the average of the durations. */
    MUINT64 min;          /*!< \brief Holds the minimum of the durations. */
    MUINT64 num;          /*!< \brief Holds the number of measurements. */

};

void utilInitPerf(utilPerf *perf);
void utilStartCapture(utilPerf *perf);
void utilStopCapture(utilPerf *perf);
void utilPrintPerf(utilPerf *perf, const char *name = "");


#endif /* _UTIL_PROF_H_ */

