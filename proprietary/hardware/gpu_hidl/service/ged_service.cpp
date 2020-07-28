#define LOG_TAG "GED Service"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <utils/Trace.h>
#include <utils/SystemClock.h>
#include <cutils/properties.h>

#include <log/log.h>

//#include <binder/IServiceManager.h>

#include <ged/ged.h>
#include <ged/ged_type.h>
#include <ged/ged_dvfs.h>
#include <ged/ged_log.h>

#include "GraphicExt.h"

namespace vendor {
namespace mediatek {
namespace hardware {
namespace gpu {
namespace V1_0 {
namespace implementation {

using namespace android;
using ::android::status_t;

static bool g_bStay = true;
static GED_HANDLE gm_hGed;

static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static pthread_t tid_offset_shifter = 0;

static uint64_t getVSyncOffset()
{
	uint64_t nsOffset = 0;

	ged_query_info(gm_hGed, GED_VSYNC_OFFSET, sizeof(nsOffset), &nsOffset);
	return nsOffset;
}


static void sig_suicide_handler(int /* signo */)
{
	g_bStay = false;
}

void sig_vsync_offset_handler(int /* signo */)
{
	pthread_mutex_lock(&mutex);
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&mutex);
}

static void sig_boost_host_handler(int /* signo */)
{
}

#define PHYSICAL_FPS            60
#define LOW_POWER_MODE_FPS      30
#define PHYSICAL_FPS_LITERAL    "60"
#define FPS_UPPER_BOUND_FILE    "/d/ged/hal/fps_upper_bound"

void sig_fps_change_handler(int /* signo */)
{
	int count = 0;
	char buff[3] = {0, 0, 0};
	int fd = open(FPS_UPPER_BOUND_FILE, O_RDONLY);

	if (-1 == fd)
	{
		ALOGE("Error on opening FPS upper bound file! err=%s", strerror(errno));
		return;
	}

	count = read(fd, buff, 2);
	close(fd);

	if (-1 == count)
	{
		ALOGE("Error on reading FPS upper bound file! err=%s", strerror(errno));
		return;
	}

	int fps = strtoul(buff, NULL, 10);

	ALOGD("buff=%s, fps=%d", buff, fps);
}

/*
 * The signal handler reads /d/ged/hal/event_notify for corresponding bit of
 * below flags while invoked:
 *
 *   GED_GAS_SIGNAL_EVENT
 *   GED_MHL4K_VID_SIGNAL_EVENT
 *   GED_VILTE_VID_SIGNAL_EVENT
 *
 * and then set FPS according the mode.
 */
static void sig_event_notify_handler(int signo)
{
	unsigned int val, fps = PHYSICAL_FPS;
	GED_ERROR err = ged_query_info(gm_hGed, GED_EVENT_STATUS, sizeof(val), &val);

	if (err != GED_OK)
	{
		ALOGE("Error on query GED info, err=%d", err);
		return;
	}

	switch(signo) {
	case GED_MHL4K_VID_SIGNAL_EVENT:
	  fps = (val & GED_EVENT_MHL4K_VID) ? LOW_POWER_MODE_FPS : PHYSICAL_FPS;
	  break;

	case GED_VILTE_VID_SIGNAL_EVENT:
	  fps = (val & GED_EVENT_VILTE_VID) ? LOW_POWER_MODE_FPS : PHYSICAL_FPS;
	  break;

	default:
	  break;
	}
}

status_t GraphicExtService::updateVsyncOffset(int ns)
{
	status_t err = NO_ERROR;
	android::hardware::Return<void> ret;

	if (mGraphicExtCallback) {
		ret = mGraphicExtCallback->sendVsyncOffsetFunc(DYNAMIC_VSYNC_OFFSET_PROTOCOL , ns); // app/sf 0ms
		if (!ret.isOk()) {
			ALOGW("Failed to send vsync offset, HIDL fail.");
		}
	}

	return err;
}

status_t GraphicExtService::disableHWC(int disable)
{
	status_t err = NO_ERROR;
	android::hardware::Return<void> ret;

	if (mGraphicExtCallback) {
		ret = mGraphicExtCallback->sendVsyncOffsetFunc(1008 , disable);
		if (!ret.isOk()) {
			ALOGW("Failed to send vsync offset, HIDL fail.");
		}
	}

	return err;
}

bool ShiftVsyncOffset(bool bInitialize)
{
	static uint64_t nsPreOffset;
	uint64_t nsVsyncOffset;
	unsigned int ui32StatusVector;
	unsigned int ui32DebugVector;
	bool bValidDisplayEventReceiver = true;
	static int HWCstate;

	if(bInitialize)
	{
		nsPreOffset = 0;
		HWCstate = 0; /* imply force enabled */
	}

	ged_query_info(gm_hGed, GED_EVENT_STATUS, sizeof(ui32StatusVector),&ui32StatusVector);
	ged_query_info(gm_hGed, GED_EVENT_DEBUG_STATUS, sizeof(ui32DebugVector),&ui32DebugVector);

	atrace_int(ATRACE_TAG, "Vsync-Offset Event Vector", ui32StatusVector);
	atrace_int(ATRACE_TAG, "Vsync-Offset Debug Vector", ui32DebugVector);

    // for debug only
    //GraphicExtService::updateVsyncOffset((int)0);
    // =========================

	// check debug vector first
	if (ui32DebugVector & GED_EVENT_FORCE_ON)
	{
		nsVsyncOffset = VSYNC_EVENT_PHASE_OFFSET_NS;
	}
	else if (ui32DebugVector & GED_EVENT_FORCE_OFF)
	{
		nsVsyncOffset = 0;
	}
	else if (ui32StatusVector & GED_EVENT_LOW_LATENCY_MODE)
	{
		nsVsyncOffset = VSYNC_EVENT_PHASE_OFFSET_NS;
	}
	else if (ui32StatusVector & (GED_EVENT_WFD | GED_EVENT_VR | GED_EVENT_MHL | GED_EVENT_GAS))
	{
		nsVsyncOffset = 0;
	}
	else
	{
		nsVsyncOffset = VSYNC_EVENT_PHASE_OFFSET_NS;
	}

	status_t err = NO_ERROR;

	if (ui32StatusVector & GED_EVENT_DHWC) /* Disable HWC */
	{
		if (HWCstate != 0) /* Already Disable */
			err = GraphicExtService::disableHWC(1);
		HWCstate = 0;
	}
	else /* Enable HWC */
	{
		if (HWCstate != 1) /* Already enabled */
			err = GraphicExtService::disableHWC(0);
		HWCstate = 1;
	}

	if (nsVsyncOffset != nsPreOffset || ui32DebugVector & GED_EVENT_NOT_SYNC)
	{
		// call SurfaceFlinger Binder here
		ged_dvfs_set_vsync_offset(nsVsyncOffset);
		if (nsVsyncOffset != 0)
		{
			err = GraphicExtService::updateVsyncOffset((int)nsVsyncOffset);
		}
		else
		{
			/*
			 *  keep vsync-offset on for at least 3 sec
			 *  before release
			 */
			if (ui32StatusVector&GED_EVENT_VR)
			{
				err = GraphicExtService::updateVsyncOffset((int)nsVsyncOffset);
			}
			else
			{
				usleep(3000000);
				nsVsyncOffset = getVSyncOffset();
				if (nsVsyncOffset == 0) // confirm to release vsync-offset
				{
					err = GraphicExtService::updateVsyncOffset((int)nsVsyncOffset);
				}
			}
		}

		if (bValidDisplayEventReceiver)
		{
			nsPreOffset = nsVsyncOffset;
		}
	}

	return bValidDisplayEventReceiver;
}

void *vsync_offset_worker(void* /* unused */)
{
	static bool bNotSync = false;
	ShiftVsyncOffset(true); // VSync-offset initialize

	struct timespec to;
	to.tv_nsec = 0;

	while (1)
	{
		pthread_mutex_lock(&mutex);
		if(bNotSync)
		{
			to.tv_sec = time(NULL) + 1;
			pthread_cond_timedwait(&cond, &mutex, &to);
		}
		else
		{
			pthread_cond_wait(&cond, &mutex);
		}
		pthread_mutex_unlock(&mutex);

		if(!g_bStay)
			break;

		if (false == ShiftVsyncOffset(false))
		{
			bNotSync = true;
			ged_dvfs_probe(gm_hGed, GED_VSYNC_OFFSET_NOT_SYNC); // set unsync flag
		}
		else
		{
			bNotSync = false;
			ged_dvfs_probe(gm_hGed, GED_VSYNC_OFFSET_SYNC); // erase unsync flag
		}

	}
	return NULL;
}

int GraphicExtService::init_ged_service()
{
	pthread_attr_t attr1;

	pthread_attr_init(&attr1);

	signal(GED_DVFS_VSYNC_OFFSET_SIGNAL_EVENT, sig_vsync_offset_handler);
	signal(GED_FPS_CHANGE_SIGNAL_EVENT, sig_fps_change_handler);
	signal(GED_SRV_SUICIDE_EVENT, sig_suicide_handler);
	signal(GED_MHL4K_VID_SIGNAL_EVENT, sig_event_notify_handler);
	signal(GED_GAS_SIGNAL_EVENT, sig_event_notify_handler);
	signal(GED_SIGNAL_BOOST_HOST_EVENT, sig_boost_host_handler);
	signal(GED_VILTE_VID_SIGNAL_EVENT, sig_event_notify_handler);

    gm_hGed = ::ged_create();
    pthread_create(&tid_offset_shifter, &attr1, vsync_offset_worker, NULL);

    ::GED_ERROR err;
	ged_dvfs_set_vsync_offset(VSYNC_EVENT_PHASE_OFFSET_NS);
	err = ged_dvfs_probe(gm_hGed, getpid());

	return 0;
}

int GraphicExtService::destroy_ged_service()
{
    ged_dvfs_probe(gm_hGed, -1);

    // trigger tid_offset_shifter worker die
    pthread_mutex_lock(&mutex);
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);

    pthread_join(tid_offset_shifter, NULL);

    ::ged_destroy(gm_hGed);

    return 0;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace gpu
}  // namespace hardware
}  // namespace mediatek
}  // namespace vendor
