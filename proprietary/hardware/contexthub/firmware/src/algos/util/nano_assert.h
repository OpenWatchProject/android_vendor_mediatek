#ifndef LOCATION_LBS_CONTEXTHUB_NANOAPPS_UTIL_NANO_ASSERT_H_
#define LOCATION_LBS_CONTEXTHUB_NANOAPPS_UTIL_NANO_ASSERT_H_

#define ASSERT(x) configASSERT(x)

// Use NULL when compiling for C and nullptr for C++.
#ifdef __cplusplus
#define ASSERT_NOT_NULL(ptr) ASSERT((ptr) != nullptr)
#else
#define ASSERT_NOT_NULL(ptr) ASSERT((ptr) != NULL)
#endif

//#define UNUSED(x) ((void)(sizeof(x)))

#endif  // LOCATION_LBS_CONTEXTHUB_NANOAPPS_UTIL_NANO_ASSERT_H_
