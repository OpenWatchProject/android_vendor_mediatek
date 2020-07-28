
#ifndef __MEOW_MISC_H__
#define __MEOW_MISC_H__

#ifdef __cplusplus
extern "C" {
#endif

#undef NELEM
#define NELEM(x) (sizeof(x)/sizeof(*(x)))

#define EXPORT_API __attribute__ ((visibility ("default")))
#define HIDDEN_API __attribute__ ((visibility ("hidden")))

#define likely(x)       __builtin_expect((x), 1)
#define unlikely(x)     __builtin_expect((x), 0)

#ifdef __cplusplus
}
#endif

#endif
