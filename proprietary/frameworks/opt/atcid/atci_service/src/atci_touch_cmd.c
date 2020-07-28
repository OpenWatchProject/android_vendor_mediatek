#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/fb.h>

#include "atci_service.h"
#include "atcid_util.h"
#include "atci_touch_cmd.h"
#include <errno.h>
// ============================================================ //
//define
// ============================================================ //
#define TP_EMU_RELEASE      0
#define TP_EMU_DEPRESS      1
#define TP_EMU_SINGLETAP    2
#define TP_EMU_DOUBLETAP    3

#define INPUT_DEVICE_LIST   "/proc/bus/input/devices"
#define INPUT_DEVICE_PREFIX "/dev/input/"
#define TPD_DEVICE_DEFAULT  "/dev/input/event2"
#define TPD_DEVICE_NAME     "mtk-tpd"
#define TOUCH_CMD_LENGTH    (40)

// ============================================================ //
// Global variable
// ============================================================ //
int lcm_width;
int lcm_height;
static char tpd_dev[24];
static int tpd_dev_init = 0;
static int tpd_down = 0;
// ============================================================ //
// function prototype
// ============================================================ //


// ============================================================ //
//extern variable
// ============================================================ //

// ============================================================ //
//extern function
// ============================================================ //

static int getInputDevice(char* target, int target_size) {
    FILE *fp;
    int ret, found = 0;
    char buf[128], info[32], *dev_input;

    fp = fopen(INPUT_DEVICE_LIST, "r");
    if (fp == NULL) {
        ALOGE("cannot open %s, errno = %d\n", INPUT_DEVICE_LIST, errno);
        return -1;
    }

    while(fgets(buf, sizeof(buf), (FILE*) fp)) {
        if (strlen(buf) <= 1) {
            found = 0;
        } else if (found == 0) {
            ret = sscanf(buf, "N: Name=\"%31s\"", info);
            if (ret == 1) {
                if (!strncmp(info, target, target_size)) {
                    ALOGD("%s found\n", target);
                    found =1;
                }
            }
        } else {
            ret = sscanf(buf, "H: Handlers=%31[^\n]", info);
            if (ret == 1) {
                dev_input = strtok(info, " ");
                while (dev_input != NULL) {
                    if (dev_input != NULL && !strncmp(dev_input, "event", strlen("event"))) {
                        snprintf(tpd_dev, sizeof(tpd_dev), "%s%s", INPUT_DEVICE_PREFIX, dev_input);
                        fclose(fp);
                        return 0;
                    }
                    dev_input = strtok(NULL, " ");
                }
            }
        }
    }
    fclose(fp);
    return -1;
}


void get_tpd_inputdev(void)
{
    if (tpd_dev_init == 1)
        return;

    if (getInputDevice(TPD_DEVICE_NAME, strlen(TPD_DEVICE_NAME)) < 0) {
        strncpy(tpd_dev, TPD_DEVICE_DEFAULT, sizeof(tpd_dev));
        ALOGE("%s not found, using default device\n", TPD_DEVICE_NAME);
    }
    ALOGE("touch input device is %s", tpd_dev);

    tpd_dev_init = 1;
}

void get_fb_info(void)
{
    int fd;
    struct fb_var_screeninfo vi;

    fd = open("/dev/graphics/fb0", O_RDWR);
    if (fd < 0) {
        ALOGE("cannot open fb0");
        return;
    }

    if (ioctl(fd, FBIOGET_VSCREENINFO, &vi) < 0) {
        ALOGE("failed to get fb0 info");
        close(fd);
        return;
    }

    if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "270", 3) || 0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "90", 2)) {
        lcm_width = (unsigned int)vi.yres;
        lcm_height = (unsigned int)vi.xres;
    } else {
        lcm_width = (unsigned int)vi.xres;
        lcm_height = (unsigned int)vi.yres;
    }

    ALOGD("LCM_WIDTH=%d, LCM_HEIGHT=%d", lcm_width, lcm_height);

    close(fd);
}

void rotate_relative_to_lcm(int *x, int *y)
{
    int temp;

    if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "180", 3)) {
        // X reverse; Y reverse
        *x = lcm_width - *x - 1;
        *y = lcm_height- *y - 1;
        ALOGD("[rotate_relative_to_lcm] 180");
    } else if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "270", 3)) {
        //X,Y change; X reverse
        temp = *x;
        *x = lcm_width - *y - 1;
        *y = temp;
        ALOGD("[rotate_relative_to_lcm] 270");
    } else if(0 == strncmp(MTK_LCM_PHYSICAL_ROTATION, "90", 2)) {
        //X, Y change;  Y reverse
        temp = *x;
        *x = *y;
        *y = lcm_height - temp - 1;
        ALOGD("[rotate_relative_to_lcm] 90");
    } else {
        // no change
        ALOGD("[rotate_relative_to_lcm] 0");
    }
}

void runcmd(char* cmd)
{
    if (cmd == NULL) {
        ALOGE("Invalid command");
        return;
    }
 //   ALOGE("[atci] Run command %s", cmd);
    int ret = system(cmd);
    ALOGE("[atci] Run command %s, ret = %d", cmd, ret);

}

void touch_down(int x, int y)
{
    char touch_cmd[TOUCH_CMD_LENGTH];

    if (tpd_down == 0) {
        snprintf(touch_cmd, TOUCH_CMD_LENGTH, "sendevent %s 1 330 1", tpd_dev); //BTN_TOUCH DOWN
        runcmd(touch_cmd);
        tpd_down = 1;
    }
    snprintf(touch_cmd, TOUCH_CMD_LENGTH, "sendevent %s 3 48 40", tpd_dev); //ABS_MT_TOUCH_MAJOR
    runcmd(touch_cmd);
    snprintf(touch_cmd, TOUCH_CMD_LENGTH, "sendevent %s 3 57 0", tpd_dev);  //ABS_MT_TRACKING_ID
    runcmd(touch_cmd);
    snprintf(touch_cmd, TOUCH_CMD_LENGTH, "sendevent %s 3 53 %d", tpd_dev, x); //ABS_MT_POSITION_X
    runcmd(touch_cmd);
    snprintf(touch_cmd, TOUCH_CMD_LENGTH, "sendevent %s 3 54 %d", tpd_dev, y); //ABS_MT_POSITION_Y
    runcmd(touch_cmd);
    snprintf(touch_cmd, TOUCH_CMD_LENGTH, "sendevent %s 0 2 0", tpd_dev); //SYN_MT_REPORT
    runcmd(touch_cmd);
    snprintf(touch_cmd, TOUCH_CMD_LENGTH, "sendevent %s 0 0 0", tpd_dev); //SYN_REPORT
    runcmd(touch_cmd);
}

void touch_up()
{
    char touch_cmd[TOUCH_CMD_LENGTH];

    if (tpd_down == 0) {
        ALOGE("Ignore touch up due to it is not pressed");
        return;
    }
    tpd_down = 0;
    snprintf(touch_cmd, TOUCH_CMD_LENGTH, "sendevent %s 1 330 0", tpd_dev); //BTN_TOUCH UP
    runcmd(touch_cmd);
    snprintf(touch_cmd, TOUCH_CMD_LENGTH, "sendevent %s 0 2 0", tpd_dev); //SYN_MT_REPORT
    runcmd(touch_cmd);
    snprintf(touch_cmd, TOUCH_CMD_LENGTH, "sendevent %s 0 0 0", tpd_dev); //SYN_REPORT
    runcmd(touch_cmd);
}

void touch_tap(int x, int y)
{
    touch_down(x, y);
    touch_up();
}

void touch_emu_cmd(TP_POINT points[], int point_num)
{
    int i;

    //ALOGD("[touch_emu_cmd]:num=%d", point_num);

    for (i=0; i<point_num; i++)
    {
        //ALOGD("[touch_emu_cmd]act=%d,x=%d,y=%d", points[i].action, points[i].x, points[i].y);

        if (points[i].action == TP_EMU_DEPRESS) //Down
        {
            ALOGD("[DOWN %d]:(%d, %d)", i, points[i].x, points[i].y);
            touch_down(points[i].x, points[i].y);
        }
        else if(points[i].action == TP_EMU_RELEASE) //Up
        {
            ALOGD("[UP %d]:(%d, %d)", i, points[i].x, points[i].y);
            touch_up();
            break;
        }
        else if(points[i].action == TP_EMU_SINGLETAP) //Single tap
        {
            ALOGD("[SingleTap %d]:(%d, %d)", i, points[i].x, points[i].y);
            touch_tap(points[i].x, points[i].y);
            break;
        }
        else if(points[i].action == TP_EMU_DOUBLETAP) //Double tap
        {
            ALOGD("[DoubleTap %d]:(%d, %d)", i, points[i].x, points[i].y);
            touch_tap(points[i].x, points[i].y);
            touch_tap(points[i].x, points[i].y);
            break;
        }

    }
}

int touch_cmd_parser(char *cmdline, TP_POINT points[], int *point_num)
{
    int ret = 0;
    int num = 0;
    int valid_para = 0;
    char legency_cmd[200] = {0};
    char *pcmd = cmdline;

    get_fb_info();
    get_tpd_inputdev();

    while(1)
    {
        if(num == 0)
            valid_para = sscanf(pcmd, "%d,%d,%d%199s", &points[num].action, &points[num].x, &points[num].y, legency_cmd);
        else
            valid_para = sscanf(legency_cmd, "%*[^=]=%d,%d,%d%199s", &points[num].action, &points[num].x, &points[num].y, legency_cmd);


        ALOGD("[Point%d]:%d,%d,%d,(%s); para_num=%d\n", num, points[num].action, points[num].x, points[num].y, legency_cmd, valid_para);

        // (0,0) is fixed at left-up
        rotate_relative_to_lcm(&points[num].x, &points[num].y);
        num++;

        if (valid_para < 3) // parameter is wrong
        {
            ret = -1;
            break;
        }
        else if (valid_para == 3) // parser end
        {
            break;
        }
        else if (valid_para > 3) // parser continue
        {
            continue;
        }
    }
    *point_num = num;

    return ret;

}


int touch_cmd_handler(char* cmdline, ATOP_t at_op, char* response)
{
    int ret = 0;
    int point_num = 0;
    TP_POINT points[10];

    ALOGD("touch cmdline=%s, at_op=%d, \n", cmdline, at_op);

    switch(at_op){
        case AT_ACTION_OP:
        case AT_READ_OP:
        case AT_TEST_OP:
            sprintf(response, "\r\n+CTSA: (0,1,2,3)\r\n");
            break;
        case AT_SET_OP:
            ret = touch_cmd_parser(cmdline, points, &point_num);
            if (ret < 0)
            {
                sprintf(response,"\r\n+CME ERROR: 50\r\n");
                break;
            }
            touch_emu_cmd(points, point_num);
            sprintf(response,"\r\nOK\r\n");
            break;
    default:
        break;
    }

    return 0;
}

