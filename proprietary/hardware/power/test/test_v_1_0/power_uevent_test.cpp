#include <vector>
#include <dlfcn.h>
#include <log/log.h>
#include <cutils/uevent.h>
#include <stdio.h>
#include <string>
#include <inttypes.h>

#define UEVENT_MSG_LEN  1024
using namespace std;


#define PREFIX "change@/devices/virtual/misc/cpu_loading"


int main(void)
{

    int device_fd;
    char msg[UEVENT_MSG_LEN + 2];
    int n, i;

    device_fd = uevent_open_socket(64*1024, true);
    if(device_fd < 0)
        return -1;

    while ((n = uevent_kernel_multicast_recv(device_fd, msg, UEVENT_MSG_LEN)) > 0) {
        msg[n] = '\0';
        msg[n+1] = '\0';

        for (i = 0; i < n; i++)
            if (msg[i] == '\0')
                msg[i] = ' ';

        printf("%s\n", msg);
        //parseEvent(msg);
    }

    return 0;
}


