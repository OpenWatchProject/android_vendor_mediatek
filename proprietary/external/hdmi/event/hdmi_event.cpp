
#define LOG_TAG "HdmiUEventThread"

#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>
#include <linux/netlink.h>

#include <cutils/properties.h>
#include <utils/threads.h>
#include <utils/Log.h>

#include "hdmi_event.h"
#include "MtkHdmiService.h"
using namespace vendor::mediatek::hardware::hdmi::V1_0::implementation;
using android::AutoMutex;

// ---------------------------------------------------------------------------
#define CEC_BUFFER_SIZE 2048

HdmiUEventThread::HdmiUEventThread()
{
    ALOGI("HdmiUEventThread\n");
    m_socket = -1;
    init();
}

HdmiUEventThread::~HdmiUEventThread()
{
    if (m_socket > 0)  {
        close(m_socket);
    }
    ALOGE("~HdmiUEventThread");
}

void HdmiUEventThread::init()
{
    struct sockaddr_nl addr_sock;
    int optval = 64 * 1024;

    m_socket = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (m_socket < 0)
    {
        ALOGE("Unable to create uevent socket:%s", strerror(errno));
        return;
    }
    if(setsockopt(m_socket, SOL_SOCKET, SO_RCVBUFFORCE, &optval, sizeof(optval)) <0)
    {
        ALOGE("Unable to set uevent socket SO_RCVBUF option:%s(%d)", strerror(errno),errno);
       // close(m_socket);
       // return;
    }

    memset(&addr_sock, 0, sizeof(addr_sock));
    addr_sock.nl_family = AF_NETLINK;
    //addr_sock.nl_pad = 0xcec;
    addr_sock.nl_pid = getpid()<<3;
    addr_sock.nl_groups = 0xffffffff;

    ALOGI("Start to initialize, nl_pid(%d)", addr_sock.nl_pid);
    if (bind(m_socket, (struct sockaddr *)&addr_sock, sizeof(addr_sock)) < 0)
    {
        ALOGE("Failed to bind socket:%s(%d)",strerror(errno), errno);
        close(m_socket);
        return;
    }
}

void HdmiUEventThread::handleHdmiUEvents(const char *buff, int len)
{
    const char *s = buff;
    int change_hdmi = !strcmp(s, "change@/devices/virtual/switch/hdmi");
    //int change_hdmi_res = !strcmp(s, "change@/devices/virtual/switch/res_hdmi");
    //ALOGD("HdmiUEventThread handle hdmi uevents: s=%s, len=%d", s, len);
    //ALOGD("change_hdmi=%d, change_hdmi_res=%d", change_hdmi, change_hdmi_res);
    if (!change_hdmi)
    {
        return;
    }else
    {
        int state = 0;
        s += strlen(s) + 1;

        while (*s)
        {
            if (!strncmp(s, "SWITCH_STATE=", strlen("SWITCH_STATE=")))
            {
                state = atoi(s + strlen("SWITCH_STATE="));
                ALOGD("uevents: SWITCH_STATE=%d", state);
            }

            ALOGD("uevents: s=%p, %s", s, s);

            s += strlen(s) + 1;
            if (s - buff >= len)
                break;
        }

        if (state == 0x1)
        {
            ALOGD("uevents: hdmi connecting...");
            MtkHdmiService::refreshEdid();
        } else {
            ALOGD("uevents: hdmi disconnecting...");
        }
    }
}

#define UEVENT_BUFFER_SIZE 2048
bool HdmiUEventThread::threadLoop()
{
    //ALOGD("[MtkHdmiService]enter threadLoop()");
    AutoMutex l(m_lock);
    struct pollfd fds;
    static char uevent_desc[UEVENT_BUFFER_SIZE * 2];

    fds.fd = m_socket;
    fds.events = POLLIN;
    fds.revents = 0;
    int ret = poll(&fds, 1, -1);

    if (ret > 0 && (fds.revents & POLLIN))
    {
        /* keep last 2 zeroes to ensure double 0 termination */
        int count = recv(m_socket, uevent_desc, sizeof(uevent_desc) - 2, 0);
        //ALOGD("[MtkHdmiService]count = %d ", count);
        if (count > 0) handleHdmiUEvents(uevent_desc, count);
    }
    return true;
}



