#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <log/log.h>
#include "MetaPub.h"
#include "meta_keypadbk_para.h"

#define KPD_BKL_PATH		"/sys/class/leds/button-backlight/brightness"

struct kpd_ledctl
{
	__u8 onoff;
	__u8 div;
	__u8 duty;
};

#define SET_KPD_BACKLIGHT	_IOW('k', 29, struct kpd_ledctl)

static int kpd_fd = -1;

BOOL Meta_KeypadBK_Init(void)
{
	kpd_fd = open(KPD_BKL_PATH, O_WRONLY);
	if (kpd_fd < 0) {
		ALOGD("kpd: open %s failed\n", KPD_BKL_PATH);
		return false;
	}

	return true;
}

KeypadBK_CNF Meta_KeypadBK_OP(KeypadBK_REQ req)
{
	KeypadBK_CNF cnf = { .status = false, };

	if (kpd_fd < 0) {
		ALOGD("kpd: uninitialized kpd_fd\n");
		return cnf;
	}

	if (req.onoff)
	{
		if (write(kpd_fd,"1",1) == -1)
			ALOGE("Can't write %d\n", req.onoff);
	} else if (write(kpd_fd, "0",1) == -1) {
			ALOGE("Can't write %d\n", req.onoff);
	}

	cnf.status = true;
	return cnf;
}

BOOL Meta_KeypadBK_Deinit(void)
{
	if (kpd_fd < 0) {
		ALOGD("kpd: uninitialized kpd_fd\n");
		return false;
	}

	close(kpd_fd);
	return true;
}
