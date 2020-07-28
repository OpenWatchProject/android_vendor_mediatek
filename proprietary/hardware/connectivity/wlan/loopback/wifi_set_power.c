#include <stdio.h>
#include <stdint.h>
#include <sched.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cutils/log.h>
#include <cutils/misc.h> /* load_file() */
#include <errno.h>

#if defined(_RFKILL_SUPPORT)
/*******************************************************************************
 *                           P R I V A T E   D A T A
 *******************************************************************************
 */
static int wifi_rfkill_id = -1;
static char *wifi_rfkill_state_path = NULL;

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is called to initial rfkill variables
 * 
 * @param
 * 
 * @return
 */
/*----------------------------------------------------------------------------*/
static int
wifi_init_rfkill(
    void
    )
{
    char path[64];
    char buf[16];
    int fd;
    int sz;
    int id;

    for (id = 0; ; id++) {
        snprintf(path, sizeof(path), "/sys/class/rfkill/rfkill%d/type", id);
        fd = open(path, O_RDONLY);
        if (fd < 0) {
            printf("[%s] open(%s) failed: %s (%d)\n", __func__, path, strerror(errno), errno);
            return -1;
        }
        sz = read(fd, &buf, sizeof(buf));
        close(fd);
        if (sz >= 4 && memcmp(buf, "wlan", 4) == 0) {
            wifi_rfkill_id = id;
            break;
        }
    }

    asprintf(&wifi_rfkill_state_path, "/sys/class/rfkill/rfkill%d/state",
            wifi_rfkill_id);
    return 0;
}
#endif
#define WIFI_POWER_PATH                 "/dev/wmtWifi"

/*----------------------------------------------------------------------------*/
/*!
 * @brief This function is called to turn on/off Wi-Fi interface via rfkill
 * 
 * @param
 * 
 * @return
 */
/*----------------------------------------------------------------------------*/
#if defined(_RFKILL_SUPPORT)
int
wifi_set_power(
    int on
    )
{
    int sz;
    int fd = -1;
    int ret = -1;
    const char buffer = (on ? '1' : '0');

    printf("[%s] %d", __func__, on);

    if (wifi_rfkill_id == -1) {
        if (wifi_init_rfkill()) {
            goto out;
        }
    }

    fd = open(wifi_rfkill_state_path, O_WRONLY);
    printf("[%s] %s", __func__, wifi_rfkill_state_path);
    if (fd < 0) {
        printf("open(%s) for write failed: %s (%d)",
                wifi_rfkill_state_path,
                strerror(errno),
                errno);
        goto out;
    }
    sz = write(fd, &buffer, 1);
    if (sz < 0) {
        printf("write(%s) failed: %s (%d)",
                wifi_rfkill_state_path,
                strerror(errno),
                errno);
        goto out;
    }
    ret = 0;
	sleep(3);

out:
    if (fd >= 0) {
        close(fd);
    }
    return ret;
}
#else
int
wifi_set_power(int on)
{
	int sz = 0;
	int fd = -1;
	const char buffer = (on ? '1' : '0');

	fd = open(WIFI_POWER_PATH, O_WRONLY);
	if (fd < 0) {
		printf("Open \"%s\" failed. %s(%d)\n", 
			WIFI_POWER_PATH, strerror(errno), errno);
		goto out;
	}
	sz = write(fd, &buffer, 1);
	if (sz < 0) {
		//printf("Set \"%s\" [%c] failed  %s(%d)\n", 
			//WIFI_POWER_PATH, buffer, strerror(errno), errno);
		goto out;
	}

out:
	if (fd >= 0) close(fd);
	//printf("wifi set power %d done\n", on);
	return sz;
}
#endif

