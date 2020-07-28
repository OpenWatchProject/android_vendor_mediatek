#include "DpSync.h"
#include "DpLogger.h"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sw_sync.h>
#include <sync.h>
#include <errno.h>
#include <string.h>

struct mdp_sync_create_fence_data {
  __u32 value;
  char name[32];
  __s32 fence; /* fd of new fence */
};

#define MDP_SYNC_IOC_MAGIC 'W'
#define MDP_SYNC_IOC_CREATE_FENCE _IOWR(MDP_SYNC_IOC_MAGIC, 0, struct mdp_sync_create_fence_data)
#define MDP_SYNC_IOC_INC _IOW(MDP_SYNC_IOC_MAGIC, 1, __u32)

DpSync::DpSync()
    : m_timelineFd(-1)
{
    createTimeline();
}


DpSync::~DpSync()
{
    if (-1 != m_timelineFd)
    {
        close(m_timelineFd);
    }
}


bool DpSync::createTimeline()
{
    if (-1 == m_timelineFd)
    {
        m_timelineFd = open("/dev/mdp_sync", O_RDONLY);
        if (m_timelineFd < 0)
        {

            DPLOGE("DpSync: can't open /dev/mdp_sync %d %s\n", errno, strerror(errno));
            m_timelineFd = open("/dev/sw_sync", O_RDONLY);
            if (m_timelineFd < 0)
            {
                m_timelineFd = open("/sys/kernel/debug/sync/sw_sync", O_RDONLY);
                if (m_timelineFd < 0)
                {
                    DPLOGE("DpSync: can't create mdp_sync_timeline %d\n", m_timelineFd);
                    sprintf(m_syncName, "BlitSync-%08x%c", m_timelineFd, '\0');
                    return false;
                }
            }
        }
        sprintf(m_syncName, "BlitSync-%08x%c", m_timelineFd, '\0');
    }
    else
    {
        DPLOGE("DpSync: mdp_sync_timeline has been created\n");
        return false;
    }

    return true;
}


void DpSync::createFence(int32_t &fd, uint32_t val)
{
    struct mdp_sync_create_fence_data data;

    int err;

    data.value = val;
    strlcpy(data.name, m_syncName, sizeof(data.name));

    err = ioctl(m_timelineFd, MDP_SYNC_IOC_CREATE_FENCE, &data);
    if (err < 0)
    {
        DPLOGE("DpSync: createFence error %d, %d, %s\n", err, errno, strerror(errno));
        fd = err;
    }
    else
    {
        fd = data.fence;
    }
}

void DpSync::wait(int32_t fd, int32_t timeout)
{
    int32_t err;

    err = sync_wait(fd, timeout);
    if (err < 0)
    {
        DPLOGE("DpSync: wait %d failed: %s\n", fd, strerror(errno));
    }
}


void DpSync::wakeup(uint32_t inc)
{
    int32_t err;
    uint32_t arg = inc;

    err = ioctl(m_timelineFd, MDP_SYNC_IOC_INC, &arg);
    if (err < 0)
    {
        DPLOGE("DpSync::wakeup can't increment sync obj\n");
    }
}
