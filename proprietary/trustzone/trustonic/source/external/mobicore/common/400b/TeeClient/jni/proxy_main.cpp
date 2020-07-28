/*
 * Copyright (c) 2015-2017 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <mutex>
#include <condition_variable>

#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mc_user.h"
#include "dynamic_log.h"
#include "proxy_server.h"

class Lock {
    std::mutex mutex_;
    std::condition_variable cond_;
    bool running_ = true;
public:
    void wait() {
        std::unique_lock<std::mutex> lock(mutex_);
        while (running_) {
            cond_.wait(lock);
        }
    }
    void signal() {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
        cond_.notify_one();
    }
};

static Lock lock;

static void terminate(int) {
    lock.signal();
}

int main(int argc, char* args[]) {
    bool daemonize = false;

    int opt;
    extern int optopt;
    while ((opt = ::getopt(argc, args, "b")) != -1) {
        switch (opt) {
            case 'b':
                daemonize = true;
                LOG_I("daemonize");
                break;
            default:
                LOG_E("Unknown option '%c', only -a or -b are acceptable",
                      optopt);
                return EXIT_FAILURE;
        }
    }

    // We wait 100 times 1s for the device file to appear
    int counter = 100;
    do {
        struct stat st;

        int ret = ::stat("/dev/" MC_USER_DEVNODE, &st);
        if (!ret) {
            break;
        }

        /* Device driver not ready yet: give some time */
        sleep(1);
    } while (counter--);

    if (counter < 0) {
        LOG_E("timed out while waiting for user device to appear");
        return EXIT_FAILURE;
    }

    // Process termination action
    struct sigaction sigaction;
    ::memset(&sigaction, 0, sizeof(sigaction));
    sigaction.sa_handler = terminate;
    ::sigemptyset(&sigaction.sa_mask);
    sigaction.sa_flags = 0;
    if (::sigaction(SIGINT, &sigaction, NULL)) {
        LOG_ERRNO("sigaction");
        return EXIT_FAILURE;
    }
    if (::sigaction(SIGTERM, &sigaction, NULL)) {
        LOG_ERRNO("sigaction");
        return EXIT_FAILURE;
    }

    if (daemonize) {
        // Become a daemon
        signal(SIGHUP, SIG_IGN);
        if (daemon(0, 0) < 0) {
            LOG_ERRNO("daemon");
            return EXIT_FAILURE;
        }
        // ignore tty signals
        signal(SIGTSTP, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
    }

    ProxyServer server(false);
    // Sometimes the permissions will change _after_ the node is created
    while (server.open()) {
        // EADDRINUSE -> another instance is already running
        // EHOSTDOWN  -> version mismatch between driver and proxy
        if ((errno == EADDRINUSE) || (errno == EHOSTDOWN)) {
            LOG_ERRNO("proxy server open");
            return EXIT_FAILURE;
        }
        LOG_W("proxy server open failed: %s, retry in 1s", strerror(errno));
        sleep(1);
    }
    LOG_I("proxy server open");
    lock.wait();
    server.close();
    LOG_I("proxy server closed");
    return EXIT_SUCCESS;
}
