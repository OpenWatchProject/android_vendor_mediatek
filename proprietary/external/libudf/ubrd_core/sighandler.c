#include <ubrd_config.h>
#include "sighandler.h"
#include "../include/recorder.h"


static PUBRD pDUMPUBRD = NULL;

//
// Catches signal SIGUSR2 for debug 15
//
static void ubrd_signal_handler(int n, siginfo_t* info, void* unused) {
    (void)n;
    (void)info;
    (void)unused;
    ubrd_info_log("ubrd_signal_handler\n");
    if (pDUMPUBRD)
        dumpUBRD(pDUMPUBRD);
}

void ubrd_install_signal(PUBRD pUBRD) {
    struct sigaction act;
    pDUMPUBRD = pUBRD;
    memset(&act, 0, sizeof(act));
    act.sa_sigaction = ubrd_signal_handler;
    act.sa_flags = SA_RESTART | SA_SIGINFO;
    sigemptyset(&act.sa_mask);

    sigaction(SIGUSR2, &act, NULL);
    ubrd_info_log("install sigusr2\n");
}