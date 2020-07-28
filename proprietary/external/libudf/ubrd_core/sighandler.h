#ifndef SIGHANDLER_H
#define SIGHANDLER_H

#include <ubrd_config.h>

#ifdef __cplusplus
extern "C" {
#endif

void ubrd_install_signal(PUBRD pUBRD);
void dumpUBRD(PUBRD pUBRD);

#ifdef __cplusplus
}
#endif
#endif //#ifndef SIG_HANDLER_H
