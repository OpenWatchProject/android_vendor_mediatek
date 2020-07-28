#include "DpProfiler.h"
#include "DpTimer.h"
#include "DpLogger.h"

DpTimeValue g_profileTemp[7];
int32_t     g_profileData[7];

void dpProfilerResetInfo()
{
    memset((void*)g_profileData, 0x0, sizeof(g_profileData));
}

void dpProfilerDumpInfo()
{
    DPLOGD("-----------------------\n");
    DPLOGD("Stream total time: %d\n", g_profileData[0]);
    DPLOGD("Compose path time: %d\n", g_profileData[1]);
    DPLOGD("Config frame time: %d\n", g_profileData[2]);
    DPLOGD("      Map HW addr: %d\n", g_profileData[5]);
    DPLOGD("Config tile time:  %d\n", g_profileData[3]);
    DPLOGD("HW execution time: %d\n", g_profileData[4]);
    DPLOGD("Flush buffer time: %d\n", g_profileData[6]);
    DPLOGD("-----------------------\n");
}
