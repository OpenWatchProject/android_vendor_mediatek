/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <utils/StrongPointer.h>

#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <cutils/sockets.h>
#define LOG_TAG "NetdagentTest"
#include <log/log.h>

#include <vendor/mediatek/hardware/netdagent/1.0/INetdagent.h>
#include <hidl/HidlSupport.h>
#include <string>
using std::string;
using android::hardware::hidl_string;
using vendor::mediatek::hardware::netdagent::V1_0::INetdagent;


static void usage(char *progname);
#if 0
static int do_monitor(int sock, int stop_after_cmd);
static int do_cmd(int sock, int argc, char **argv);
static int do_cmd(int sock, string &cmd);
static int do_test(int sock);
#endif
static int do_test();
static int do_test(int argc, char **argv);

int main(int argc, char **argv) {

    if (argc < 2)
        usage(argv[0]);

    if (!strcmp(argv[1], "hidl"))
        exit(do_test());
  
    exit(do_test(argc, argv));

#if 0
    // try interpreting the first arg as the socket name - if it fails go back to netd
    if ((sock = socket_local_client("netdagent",
                                     ANDROID_SOCKET_NAMESPACE_ABSTRACT,
                                     SOCK_STREAM | SOCK_CLOEXEC)) < 0) {
        fprintf(stderr, "Error connecting (%s)\n", strerror(errno));
        exit(4);
    }


    if (!strcmp(argv[1], "monitor"))
        exit(do_monitor(sock, 0));

    if (!strcmp(argv[1], "sokcet"))
        exit(do_test(sock));

    exit(do_cmd(sock, argc, &(argv[0])));
#endif
}

static int do_test(int argc, char **argv) {
    hidl_string hidl_cmd;
    string temp_cmd;
    int i;

    for (i = 1; i < argc; i++) {
        temp_cmd += argv[i];
        temp_cmd += " ";
    }
    fprintf(stderr, "%s\n", temp_cmd.c_str());

    android::sp<INetdagent> gNetdagentService;
    //get Netdagent HIDL service
    gNetdagentService = INetdagent::getService();
    if (gNetdagentService == nullptr) {
        ALOGE("get %s service failed\n", INetdagent::descriptor);
        return 0;
    }

    //test netdagent
    hidl_cmd = temp_cmd;
    gNetdagentService->dispatchNetdagentCmd(hidl_cmd);
    return 0;
}


static int do_test() {
    hidl_string hidl_cmd;
    android::sp<INetdagent> gNetdagentService;

    //get Netdagent HIDL service
    gNetdagentService = INetdagent::getService();
    if (gNetdagentService == nullptr) {
        ALOGE("get %s service failed\n", INetdagent::descriptor);
        return 0;
    }

    //test netdagent
    hidl_cmd = "netdagent firewall get_usb_client rndis";
    gNetdagentService->dispatchNetdagentCmd(hidl_cmd);

    hidl_cmd = "netdagent firewall set_udp_forwarding rndis0 wlan0 192.168.42.28";
    gNetdagentService->dispatchNetdagentCmd(hidl_cmd);

    hidl_cmd = "netdagent firewall clear_udp_forwarding rndis0 wlan0";
    gNetdagentService->dispatchNetdagentCmd(hidl_cmd);

    hidl_cmd = "netdagent firewall set_nsiot_firewall";
    gNetdagentService->dispatchNetdagentCmd(hidl_cmd);

    hidl_cmd = "netdagent firewall clear_nsiot_firewall";
    gNetdagentService->dispatchNetdagentCmd(hidl_cmd);

    hidl_cmd = "netdagent firewall set_volte_nsiot_firewall lo";
    gNetdagentService->dispatchNetdagentCmd(hidl_cmd);

    hidl_cmd = "netdagent firewall clear_volte_nsiot_firewall lo";
    gNetdagentService->dispatchNetdagentCmd(hidl_cmd);

    hidl_cmd = "netdagent firewall set_plmn_iface_rule ccmni 7 allow";
    gNetdagentService->dispatchNetdagentCmd(hidl_cmd);

    hidl_cmd = "netdagent firewall set_interface_for_chain_rule ccmni0 powersave allow";
    gNetdagentService->dispatchNetdagentCmd(hidl_cmd);

    hidl_cmd = "netdagent firewall set_interface_for_chain_rule ccmni0 dozable allow";
    gNetdagentService->dispatchNetdagentCmd(hidl_cmd);

    hidl_cmd = "netdagent throttle cat ccmni0 rx";
    gNetdagentService->dispatchNetdagentCmd(hidl_cmd);

    hidl_cmd = "netdagent firewall priority_set_uid 10068";
    gNetdagentService->dispatchNetdagentCmd(hidl_cmd);

    hidl_cmd = "netdagent firewall priority_clear_uid 10068";
    gNetdagentService->dispatchNetdagentCmd(hidl_cmd);

    hidl_cmd = "netdagent firewall priority_clear_uid_all";
    gNetdagentService->dispatchNetdagentCmd(hidl_cmd);

    hidl_cmd = "netdagent firewall priority_set_toup 192.168.1.100 80 192.168.1.101 80 TCP";
    gNetdagentService->dispatchNetdagentCmd(hidl_cmd);

    hidl_cmd = "netdagent firewall priority_clear_toup 192.168.1.100 80 192.168.1.101 80 TCP";
    gNetdagentService->dispatchNetdagentCmd(hidl_cmd);

    hidl_cmd = "netdagent firewall priority_clear_toup_all";
    gNetdagentService->dispatchNetdagentCmd(hidl_cmd);

    //hidl_cmd = "netdagent throttle cat modem rx";
    //gNetdagentService->dispatchNetdagentCmd(hidl_cmd);

    hidl_cmd = "netdagent throttle set ccmni0 1024 -1";
    gNetdagentService->dispatchNetdagentCmd(hidl_cmd);

    //hidl_cmd = "netdagent throttle set modem 1024 -1";
    //gNetdagentService->dispatchNetdagentCmd(hidl_cmd);

    //hidl_cmd = "netdagent network forward rndis0 wlan0 192.168.42.28/32 2 enable";
    //gNetdagentService->dispatchNetdagentCmd(hidl_cmd);

    return 0;
}

#if 0
static int do_test(int sock) {
    string cmd;

    //test netdagent
    cmd = "netdagent firewall get_usb_client rndis";
    do_cmd(sock, cmd);

    cmd = "netdagent firewall set_udp_forwarding rndis0 wlan0 192.168.42.28";
    do_cmd(sock, cmd);

    cmd = "netdagent firewall clear_udp_forwarding rndis0 wlan0";
    do_cmd(sock, cmd);

    cmd = "netdagent firewall set_nsiot_firewall";
    do_cmd(sock, cmd);

    cmd = "netdagent firewall clear_nsiot_firewall";
    do_cmd(sock, cmd);

    cmd = "netdagent firewall set_volte_nsiot_firewall lo";
    do_cmd(sock, cmd);

    cmd = "netdagent firewall clear_volte_nsiot_firewall lo";
    do_cmd(sock, cmd);

    cmd = "netdagent firewall set_plmn_iface_rule ccmni 7 allow";
    do_cmd(sock, cmd);

    cmd = "netdagent firewall set_interface_for_chain_rule ccmni0 powersave allow";
    do_cmd(sock, cmd);

    cmd = "netdagent firewall set_interface_for_chain_rule ccmni0 dozable allow";
    do_cmd(sock, cmd);

    cmd = "netdagent throttle cat ccmni0 rx";
    do_cmd(sock, cmd);

    cmd = "netdagent throttle cat modem rx";
    do_cmd(sock, cmd);

    cmd = "netdagent throttle set ccmni0 1024 -1";
    do_cmd(sock, cmd);

    cmd = "netdagent throttle set modem 1024 -1";
    do_cmd(sock, cmd);

    cmd = "netdagent network forward rndis0 wlan0 192.168.42.28/32 2 enable";
    do_cmd(sock, cmd);

    //test libnetdagent_client
    android::netdagent::ifc_set_nsiot_firewall(1);

    return 0;
}

#define CMD_ARG_COUNT 26
static int do_cmd(int sock, string &cmd){
    char *temp = strdup(cmd.c_str());
    char *token = nullptr;
    int cmd_argc = 0;
    char *cmd_argv[CMD_ARG_COUNT];

    //parse command
    const char *delim = " ";
    token = strtok(temp, delim);
    while (token != NULL) {
        if (cmd_argc >= CMD_ARG_COUNT) {
            ALOGE("Command contains too many parameters\n");
            goto _release;
        }
        cmd_argv[cmd_argc++] = strdup(token);
        token = strtok(NULL, delim);
    }
    do_cmd(sock, cmd_argc, cmd_argv);

_release:
    int i;
    for (i = 0; i < cmd_argc; i++)
        free(cmd_argv[i]);
    free(temp);
    return 0;
}

static int do_cmd(int sock, int argc, char **argv) {
    char *final_cmd;
    char *conv_ptr;
    int i;

    /* Check if 1st arg is cmd sequence number */ 
    strtol(argv[1], &conv_ptr, 10);
    if (conv_ptr == argv[1]) {
        final_cmd = strdup("0 ");
    } else {
        final_cmd = strdup("");
    }
    if (final_cmd == NULL) {
        int res = errno;
        perror("strdup failed");
        return res;
    }

    for (i = 1; i < argc; i++) {
        char *tmp_final_cmd;

        if (asprintf(&tmp_final_cmd, "%s%s%s", final_cmd, argv[i],
                     (i == (argc - 1)) ? "" : " ") < 0) {
            int res = errno;
            perror("failed asprintf");
            free(final_cmd);
            return res;
        }
        free(final_cmd);
        final_cmd = tmp_final_cmd;
    }
    fprintf(stderr, "%s\n", final_cmd);

    if (write(sock, final_cmd, strlen(final_cmd) + 1) < 0) {
        int res = errno;
        perror("write");
        free(final_cmd);
        return res;
    }
    free(final_cmd);

    return do_monitor(sock, 1);
}

static int do_monitor(int sock, int stop_after_cmd) {
    char *buffer = (char *)malloc(4096);

    if (!stop_after_cmd)
        printf("[Connected to Netdagent]\n");

    while(1) {
        fd_set read_fds;
        struct timeval to;
        int rc = 0;

        to.tv_sec = 10;
        to.tv_usec = 0;

        FD_ZERO(&read_fds);
        FD_SET(sock, &read_fds);

        rc = TEMP_FAILURE_RETRY(select(sock +1, &read_fds, NULL, NULL, &to));
        if (rc < 0) {
            int res = errno;
            fprintf(stderr, "Error in select (%s)\n", strerror(res));
            free(buffer);
            return res;
        }
        if (rc == 0) {
            continue;
        }
        if (!FD_ISSET(sock, &read_fds)) {
            continue;
        }

        memset(buffer, 0, 4096);
        if ((rc = read(sock, buffer, 4096)) <= 0) {
            int res = errno;
            if (rc == 0)
                fprintf(stderr, "Lost connection to Netd - did it crash?\n");
            else
                fprintf(stderr, "Error reading data (%s)\n", strerror(res));
            free(buffer);
            if (rc == 0)
                return ECONNRESET;
            return res;
        }

        int offset = 0;
        int i = 0;

        for (i = 0; i < rc; i++) {
            if (buffer[i] == '\0') {
                int code;
                char tmp[4];

                strncpy(tmp, buffer + offset, 3);
                tmp[3] = '\0';
                code = atoi(tmp);

                printf("%s\n", buffer + offset);
                if (stop_after_cmd) {
                    if (code >= 200 && code < 600)
                        return 0;
                }
                offset = i + 1;
            }
        }
    }
    free(buffer);
    return 0;
}
#endif

static void usage(char *progname) {
    fprintf(stderr, "Usage: %s [monitor|test] <cmd> [arg ...]\n", progname);
    exit(1);
}
