#ifndef SIG_HANDLER_H
#define SIG_HANDLER_H

#include "recorder.h"

#ifdef __cplusplus
extern "C" {
#endif

void install_signal();
void malloc_signal_handler(int, siginfo_t*, void*);

#ifdef __cplusplus
}
#endif
#endif //#ifndef SIG_HANDLER_H
