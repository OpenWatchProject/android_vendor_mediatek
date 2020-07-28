/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/
/* MediaTek Inc. (C) 2015. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
* AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
* NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
* SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
* SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
* THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
* THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
* CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
* SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
* CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
* AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
* OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
* MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* The following software/firmware and/or related documentation ("MediaTek Software")
* have been modified by MediaTek Inc. All revisions are subject to any receiver\'s
* applicable license agreements with MediaTek Inc.
*/

#include <linux/input.h>
#include <sys/epoll.h>

#include <cutils/properties.h>
#include <utils/Log.h>
#include <utils/Timers.h>
#include <utils/threads.h>
#include <utils/Errors.h>
#include <cutils/properties.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include <math.h>
#include <cutils/properties.h>
#include <sys/time.h>

#undef LOG_TAG
#define LOG_TAG "TouchFilter"


static bool gTouchFilterLogEnable = false;//false;

#define DEBUG_ON 1
#if DEBUG_ON
#define touch_filter_debug(fmt, arg...) if (gTouchFilterLogEnable) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, fmt"", ##arg)
#else
#define touch_filter_debug(fmt, arg...) {}
#endif

#define touch_filter_err(fmt, arg...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, fmt"", ##arg)

//static const char *version = "Touchfilter-silkswipe-v2";

struct _filter_ {
    struct input_event event;
    int w_flag;
    int flag;
};

struct filter_ {
    struct _filter_ filer_xy[2];//0:x,1: y
    struct input_event orgin_event[2];//0:x, 1:y
};

struct tpd_filter_t
{
    int enable; //0: disable, 1: enable
    int pixel_density; //XXX pixel/cm
    int w_w[3][4];//filter custom setting prameters
    unsigned int VECLOCITY_THRESHOLD[3];//filter speed custom settings
};

extern "C"
{
    int drvb_f0(void);
}


#define TOUCH_IOC_MAGIC 'A'
#define TPD_GET_FILTER_PARA _IOWR(TOUCH_IOC_MAGIC,2,struct tpd_filter_t)
#define F_COUNT_MAX (4)
#define P_COUNT_MAX (4)
#define F_PHASE (3)
#define TPD_DEV "/dev/touch"

/////POTING MODIFY/////
#define WINDOW_SIZE 3
#define SMALL_NUM 0.005
#define MT_SLOT_HISTORY_MAX (2)
int window[2] = {0, 0};
int window_size[2] = {3, 3};
int window_counter[2] = {0, 0};
float window_weight[4] = {0.05, 0.05, 0.1, 0.8};
float init_speed_window[2];
float pre_window_rspeed[2];
float speed_diff[2];
float pspeed_diff[2];
float M_BOUND = 1000;
float now_window_rspeed[2] = {0, 0};
int start[2] = {true, true};
float window_size_speed_limit[2] = {30, 25};
int window_counter_limit = 10;
int pre_event_value[2] = {0,0};
float aaaaa[2] = {0, 0};
int mt_slot_cnt = 0;
int mt_slot_history[MT_SLOT_HISTORY_MAX] = {-1, -1};
int is_multi_touch = 0;

static int TOUCH_PIXEL_DENSITY =146;//146pixel/cm FHD: 146, WQHD:195
int adj_max_d_flag[2] = {0,0};

static unsigned int VECLOCITY_THRESHOLD[F_PHASE] ={10,30,40};//6cmm/s
static unsigned int VECLOCITY_THRESHOLD_default[F_PHASE] ={10,30,40};//6cmm/s

static int F_COUNT =4;

static float W_W[3][F_COUNT_MAX] = {{0.1,0.2,0.3,0.4}, {0.1,0.1,0.2,0.6}, {0,0.1,0.2,0.7}};
static float W_W_default[3][F_COUNT_MAX] = {{0.1,0.2,0.3,0.4}, {0.1,0.1,0.2,0.6}, {0,0.1,0.2,0.7}};

#define COORDINATE_DIMENSIONS 2
typedef struct exponential_moving_average {
	bool enable;
	double coefficient;
	double value;
} EMA;

// [x/y][speed/location]
// 30ms: 0.6
static EMA velocity[COORDINATE_DIMENSIONS] = {
	{false, 0.7, 0},
	{false, 0.7, 0},
};

static EMA raw_position[COORDINATE_DIMENSIONS] = {
	{false, 0.65, 0},
	{false, 0.65, 0},
};


//speed_diff EMA
static EMA velocity_window[COORDINATE_DIMENSIONS] = {
	{false, 0.7, 0},
	{false, 0.7, 0},
};
// software filter 1
// 30ms: 0.5
// 10ms: 0.6
static EMA position[COORDINATE_DIMENSIONS] = {
	{false, 0.5, 0},
	{false, 0.5, 0},
};

static EMA displacement[COORDINATE_DIMENSIONS] = {
	{false, 0.6, 0},
	{false, 0.6, 0},
};


double smooth_EMA(double value, EMA* p, double default_value)
{
	if (p->enable) {
		p->value = value * p->coefficient + (1.0 - p->coefficient) * p->value;
		return p->value;
	}
	p->enable = true;
	p->value = default_value;
	return default_value;
}

enum axis {
	X_AXIS = 0,
	Y_AXIS = 1,
};
static int enable_status = -1;
pthread_mutex_t filterara_lock = PTHREAD_MUTEX_INITIALIZER;

static struct filter_ filter_event[F_COUNT_MAX];

extern "C" void touch_driver_event_filter(struct input_event *event);

extern "C" void setFilterPara(void);

extern "C" int touchFilterStatus(void);

extern "C" void setM_BOUND(float m_bound);

void setTouchFilterLogEnable(bool enabled)
{
       gTouchFilterLogEnable = enabled;

       touch_filter_err("setTouchFilter LOG Enable prameter: %d", gTouchFilterLogEnable);

}

void setTouchFilterPara(int velocity[], float w_velocity[], int phase, int pointCount)
{
    int i = 0;

    pthread_mutex_lock(&filterara_lock);

    touch_filter_err("before phase:%d, pointCount:%d, velocity:%d, %d, %d", phase, pointCount, VECLOCITY_THRESHOLD[0], VECLOCITY_THRESHOLD[1], VECLOCITY_THRESHOLD[2]);
    for(i = 0; i < F_PHASE; i++)
    {
        touch_filter_err("WW[%d]: %f,%f,%f,%f", i, W_W[i][0], W_W[i][1], W_W[i][2], W_W[i][3]);
    }
    if ((phase > F_PHASE) || (pointCount > F_COUNT_MAX)) {
        touch_filter_err("filter not support:phase:%d(max: %d), pointcount:%d(max: %d)", phase, F_PHASE, pointCount, F_COUNT_MAX);
        goto FILTER_PARA_DONE;
    }
    if((phase == -1) && (pointCount == -1))
    {
        memcpy(&VECLOCITY_THRESHOLD[0], &VECLOCITY_THRESHOLD_default[0], sizeof(VECLOCITY_THRESHOLD_default));
        memcpy(W_W, W_W_default, sizeof(W_W_default));
    }
    else
    {
        if(velocity != NULL) {
            memcpy(&VECLOCITY_THRESHOLD[0], &velocity[0], (int)(sizeof(int)*phase));
        } else {
            touch_filter_err("velocity = NULL, error!\n");
        }
        memcpy(&W_W[0],&w_velocity[0], (int)(sizeof(float)*pointCount*phase));
    }

    for (i=0; i<F_PHASE; i++)
    {
        if ((fabs(W_W[i][0]+W_W[i][1]+W_W[i][2]+W_W[i][3])-1.00) > 0.00001)
        {
            touch_filter_err("fault parameters WW[%d]:%f, %f, %f, %f", i, W_W[i][0], W_W[i][1], W_W[i][2], W_W[i][3]);
            /* return default value */
            memcpy(&VECLOCITY_THRESHOLD[0], &VECLOCITY_THRESHOLD_default[0], sizeof(VECLOCITY_THRESHOLD_default));
            memcpy(W_W, W_W_default, sizeof(W_W_default));
            break;
        }
    }

    touch_filter_err("after phase:%d, pointCount:%d, velocity:%d, %d, %d", phase, pointCount, VECLOCITY_THRESHOLD[0], VECLOCITY_THRESHOLD[1], VECLOCITY_THRESHOLD[2]);
    for(i = 0; i < F_PHASE; i++)
    {
        touch_filter_err("WW[%d]: %f,%f,%f,%f", i, W_W[i][0], W_W[i][1], W_W[i][2], W_W[i][3]);
    }
FILTER_PARA_DONE:
    pthread_mutex_unlock(&filterara_lock);
}

void setFilterPara(void)
{
    int velocity[3];
    float velocity_para[12];
    char buf[PROPERTY_VALUE_MAX] = {0};

    property_get("persist.sys.input.Touchfval", buf, "true");
    if (strcmp(buf, "true")) {
        sscanf(buf, "%d %d %d %f %f %f %f %f %f %f %f %f %f %f %f",
            &velocity[0], &velocity[1], &velocity[2],
            &velocity_para[0], &velocity_para[1], &velocity_para[2], &velocity_para[3],
            &velocity_para[4], &velocity_para[5], &velocity_para[6], &velocity_para[7],
            &velocity_para[8], &velocity_para[9], &velocity_para[10], &velocity_para[11]);
       touch_filter_debug("Touch filter: %d %d %d %f %f %f %f %f %f %f %f %f %f %f %f",
            velocity[0], velocity[1], velocity[2],
            velocity_para[0], velocity_para[1], velocity_para[2], velocity_para[3],
            velocity_para[4], velocity_para[5], velocity_para[6], velocity_para[7],
            velocity_para[8], velocity_para[9], velocity_para[10], velocity_para[11]);

       setTouchFilterPara(&velocity[0], &velocity_para[0], 3, 4);
    }
}

int touchFilterStatus(void)
{
    char buf[PROPERTY_VALUE_MAX] = {0};

    property_get("sys.input.TouchFilterLogEnable", buf, "false");
    if (!strcmp(buf, "true")) {
        setTouchFilterLogEnable(true);
        touch_filter_err("Event Hub Touch Filter log is enabled");
    } else if (!strcmp(buf, "false")) {
        setTouchFilterLogEnable(false);
        touch_filter_err("Event Hub Touch Filter log is disabled");
    }

    memset(&buf[0], 0, PROPERTY_VALUE_MAX);
        property_get("sys.input.TouchFilterEnable", buf, "true");
    if (!strcmp(buf, "true")) {
        touch_filter_err("Event Hub Touch Filter is enabled");
        return true;
    } else if (!strcmp(buf, "false")) {
       touch_filter_err("Event Hub Touch Filter is disabled");
        return false;
    }
    return true;
}

static int init_filter_param(void)
{
    int fd = -1, i,j = 0;
    static int init_status = 0;
    struct tpd_filter_t filter;
	float w_w_temp[3][F_COUNT_MAX];
	char propbuf[PROPERTY_VALUE_MAX];
    if(init_status == 1)
    {
        if(enable_status == 1)
        {
			//touch_filter_debug(":have been init");
            return 0;
        }
        else
        {
            touch_filter_debug(":filter is disable or not support:%d", enable_status);
            return -1;
        }
    }
    init_status = 1;
    if((fd = open(TPD_DEV, O_RDWR)) < 0)
    {
        touch_filter_err("Couldn't open '%s' (%s)", TPD_DEV, strerror(errno));
        return -1;
    }
    if(ioctl(fd, TPD_GET_FILTER_PARA, &filter) < 0)
    {
        touch_filter_err("ioctl error '%s' (%s)", TPD_DEV, strerror(errno));
        close(fd);
        return -1;
    }
    if(filter.enable != 1) // hw layer disable
    {
        touch_filter_err("disable");
        init_status = 1; //workaround
        close(fd);
        return -1;
    }
    else
    {
        TOUCH_PIXEL_DENSITY = filter.pixel_density;
        enable_status = filter.enable;
		for(i = 0;i<3;i++){
			touch_filter_err(" param:filter.w_w[%d]:%d,%d,%d,%d",i,filter.w_w[i][0],filter.w_w[i][1],filter.w_w[i][2],filter.w_w[i][3]);
			touch_filter_err(" param:filter.VECLOCITY_THRESHOLD[%d]:%d",i,filter.VECLOCITY_THRESHOLD[i]);
			if((filter.w_w[i][0]+filter.w_w[i][1]+filter.w_w[i][2]+filter.w_w[i][3]) != 100)
				touch_filter_err(" param:filter.w_w is not fitable:");
			else{
				for(j = 0; j < 4; j++){
					w_w_temp[i][j] = ((float)filter.w_w[i][j])/100.0;
					touch_filter_err(" param:filter.w_w[%d][%d]:%f",i,j, w_w_temp[i][j]);
					memcpy(&W_W_default[i][j],&w_w_temp[i][j],sizeof(float));
					memcpy(&W_W[i][j],&w_w_temp[i][j],sizeof(float));
				}
			}

			if(filter.VECLOCITY_THRESHOLD[0] <filter.VECLOCITY_THRESHOLD[1]
				&& filter.VECLOCITY_THRESHOLD[1] <filter.VECLOCITY_THRESHOLD[2]){
				memcpy(&VECLOCITY_THRESHOLD_default[i],&filter.VECLOCITY_THRESHOLD[i],sizeof(int));
				memcpy(&VECLOCITY_THRESHOLD[i],&filter.VECLOCITY_THRESHOLD[i],sizeof(int));
			}else{
				touch_filter_err(" param:filter.VECLOCITY_THRESHOLD is not fitable:");
			}
		}
        init_status = 1; //workaround
        touch_filter_err(" param: TOUCH_PIXEL_DENSITY:%d, enable status:%d", filter.pixel_density, filter.enable);
    }
    close(fd);
   // drvb_f0();
    property_get("ro.hardware", propbuf, "");
    if ((strcmp(propbuf,"mt6797") == 0)
		|| (strcmp(propbuf,"mt6799") == 0))
	{
		touch_filter_err("plat:%s", propbuf);
		memset(propbuf, 0, sizeof(propbuf));
	}
	return 0;
}

static float d_tail[2] = {0.0};

/*
  add flag for X/Y to set 1
  s_x_flag: set to 1 when do X filter
  s_y_flag: set to 1 when do Y filter
  origin_x_event: save every X event
  origin_y_event: save every Y event
  event_align: which really set in touch_filter_function
  modiy by Peng.Zhou
*/
static int s_x_flag = 0;
static int s_y_flag = 0;
static input_event origin_x_event,origin_y_event,event_align;

static void touch_filter_function(struct input_event *event, int *flag_x, int *flag_y)
{

    int i, f_flag = 0, f_type = -1, use_d_d = 0;
    float new_value=0.0,new_d = 0.0, err_gain = 0;
    unsigned int t_t[F_COUNT_MAX]={0};
    int d_d[F_COUNT_MAX]={0}, orgin_d_d[F_COUNT_MAX] = {0};
    double speed[F_COUNT_MAX] = {0}, new_speed = 0;
    int *count = NULL;
    int p_dir = 0,n_dir = 0, orgin_p_dir = 0, orgin_n_dir = 0;
    struct input_event orgin_event;

    if(event->code == ABS_MT_POSITION_X)
    {
        f_type = 0; //X
        count = flag_x;
		/*
		  set flag for note that X has changed in Protocal B
		  if this flag is 0 that means X event same as previous point
		*/
		s_x_flag = 1;
    }
    else if(event->code == ABS_MT_POSITION_Y)
    {
        f_type = 1;//Y
        count= flag_y;
		/*
		  set flag for note that Y has changed in Protocal B
		  if this flag is 0 that means Y event same as previous point
		*/
		s_y_flag = 1;
    }

    if(f_type != -1)
    {
        if(f_type == 0)
        {
            touch_filter_debug("-X:before:type:%d, code:%d, value:%d, time:%ld.%06ld", event->type, event->code, event->value, event->time.tv_sec, event->time.tv_usec);
            //touch_filter_debug("[Touch_Filer]-X:*flag_x:%d", *count);
        }
        else if(f_type == 1)
        {
            touch_filter_debug("-Y:before:type:%d, code:%d, value:%d, time:%ld.%06ld", event->type, event->code, event->value, event->time.tv_sec, event->time.tv_usec);
            //touch_filter_debug("[Touch_Filer]-Y:*flag_y:%d", *count);
        }

        /*Save orgin data*/
        memcpy(&orgin_event, event, sizeof(struct input_event));

		/*
		  backup the event of X/Y, this backup event will
		  be set to filter function when X or Y doesn't changed in protocal B,
		  (force let protocal B same with protocal A in our filter)
		  because cannot miss any X/Y event in filter function.
		*/
		if(event->code == ABS_MT_POSITION_X) {
			memcpy(&origin_x_event, event, sizeof(struct input_event));
		}else if(event->code == ABS_MT_POSITION_Y) {
			memcpy(&origin_y_event, event, sizeof(struct input_event));
		}

        for(i = 0; i < F_COUNT; i++)
        {
            if(filter_event[i].filer_xy[f_type].flag == 1)
                f_flag += (1<<i);
        }

        touch_filter_debug(": f_flag:0x%x", f_flag);
        if (f_flag ==((1<<F_COUNT)-1))
        {
            struct timeval d_t;
            for(i = 0; i < F_COUNT-1; i++)
            {
                d_d[i] = filter_event[i+1].filer_xy[f_type].event.value -  filter_event[i].filer_xy[f_type].event.value;
                timersub(&filter_event[i+1].filer_xy[f_type].event.time, &filter_event[i].filer_xy[f_type].event.time,&d_t);
                t_t[i] = d_t.tv_sec*1000 + d_t.tv_usec/1000;
				orgin_d_d[i] = filter_event[i+1].orgin_event[f_type].value -  filter_event[i].orgin_event[f_type].value;
				speed[i] = (double)orgin_d_d[i]/(double)t_t[i];
            }
            d_d[i] = event->value - filter_event[i].filer_xy[f_type].event.value;
            orgin_d_d[i] = event->value -  filter_event[i].orgin_event[f_type].value;
			timersub(&event->time, &filter_event[i].filer_xy[f_type].event.time,&d_t);
            t_t[i] = d_t.tv_sec*1000 + d_t.tv_usec/1000;
            speed[i] = (double)orgin_d_d[i]/(double)t_t[i];
            //calc full post(+) or nav(-)
            for(i = 0; i < F_COUNT; i++)
            {
                if(d_d[i] >= 0)
                {
                    p_dir +=(1<<i);
                }
                if(d_d[i] <= 0)
                {
                    n_dir +=(1<<i);
                }
                //the orientation shift only orgin_d_d != 0
                if(orgin_d_d[i] > 0)
                {
                    orgin_p_dir +=(1<<i);
                }
                if(orgin_d_d[i] < 0)
                {
                    orgin_n_dir +=(1<<i);
                }
				if (t_t[i] <7 || t_t[i] > 30) {
					use_d_d = 1;
					touch_filter_debug("this time use d_d to jisuan new_d");
				}
            }

			//new_accel = orgin_d_d[3]-orgin_d_d[2];

			/*
			* speacil deal with slowly move case, when speed < 4, two pixel changed every point
			* we changed Weights 0.0/0.1/0.1/0.8 to adjust balance
			* need improved by Peng.Zhou (ToDo)
			*/
            if((((abs(orgin_d_d[3])*1000/TOUCH_PIXEL_DENSITY/t_t[3])) < 4)
                &&(((abs(orgin_d_d[1])*1000/TOUCH_PIXEL_DENSITY/t_t[1])) < 4))
            {
                touch_filter_debug(": veloc:<4 p_dir:0x%x, n_dir:0x%x, orgin_p_dir:0x%x, orgin_n_dir:0x%x",
                     p_dir, n_dir, orgin_p_dir, orgin_n_dir);
                //if((orgin_p_dir == ((1<<F_COUNT) - 1)) || (orgin_n_dir == ((1<<F_COUNT) - 1)))
                {
                if (use_d_d) {
                    for(i = 0; i < F_COUNT; i++)
                    {
                    	new_d = d_d[0] * 0.0 + d_d[1] * 0.1 + d_d[2] * 0.1 + d_d[3] * 0.8;
                       // new_d += d_d[i]*W_W[0][i];
                    }
                } else {
                    for(i = 0; i < F_COUNT; i++)
                    {
                    	new_speed = speed[0] * 0.0 + speed[1] * 0.1 + speed[2] * 0.1 + speed[3] * 0.8;
                    }
                    new_d = new_speed * t_t[3];
                }

				d_tail[f_type] = (new_d - (int)new_d);

				if(d_tail[f_type] > 0){
					if(fabs(d_tail[f_type]) >=0.5)
						new_d += 1.0;
				}

				if(d_tail[f_type] < 0){
					if(fabs(d_tail[f_type]) >=0.5)
						new_d -= 1.0;
				}

				new_value = filter_event[F_COUNT - 1].filer_xy[f_type].event.value + ((int)new_d);
                event->value = (int)new_value;
                touch_filter_debug(":new_d:%f, d_detail:%f, new_value:%f, high threshold:%d, err_gain:%f",
                        new_d, d_tail[f_type], new_value, (VECLOCITY_THRESHOLD[0]*TOUCH_PIXEL_DENSITY*t_t[3]/2000), err_gain);
                }
            } else if((((abs(orgin_d_d[3])*1000/TOUCH_PIXEL_DENSITY/t_t[3])) < (VECLOCITY_THRESHOLD[0]))
                &&(((abs(orgin_d_d[1])*1000/TOUCH_PIXEL_DENSITY/t_t[1])) < (VECLOCITY_THRESHOLD[0])))
            {
                touch_filter_debug(": veloc:%d, p_dir:0x%x, n_dir:0x%x, orgin_p_dir:0x%x, orgin_n_dir:0x%x",
                    VECLOCITY_THRESHOLD[0], p_dir, n_dir, orgin_p_dir, orgin_n_dir);
                //if((orgin_p_dir == ((1<<F_COUNT) - 1)) || (orgin_n_dir == ((1<<F_COUNT) - 1)))
                {
                if (use_d_d) {
                    for(i = 0; i < F_COUNT; i++)
                    {
                        new_d += d_d[i]*W_W[0][i];
                    }
                } else {
                    for(i = 0; i < F_COUNT; i++)
                    {
                        new_speed += speed[i]*W_W[0][i];
                    }
                    new_d = new_speed * t_t[3];
                }
                    //err_gain =  (filter_event[F_COUNT - 1].orgin_event[f_type].value - filter_event[F_COUNT - 1].filer_xy[f_type].event.value)*0.3;
                   // new_d = new_d + err_gain;

		    /*
                    d_tail[f_type] += (new_d - (int)new_d);
                    if((int)abs(d_tail[f_type])>=1)
                    {
                        new_d += ((int)d_tail[f_type]);
                        d_tail[f_type] = (d_tail[f_type] - (int)d_tail[f_type]);
                    }
		    */
                    d_tail[f_type] = (new_d - (int)new_d);
		    if(d_tail[f_type] > 0){
		    	if(fabs(d_tail[f_type]) >=0.5)
                    	    new_d += 1.0;
		    }
                    if(d_tail[f_type] < 0){
		    	if(fabs(d_tail[f_type]) >=0.5)
                    	    new_d -= 1.0;
		    }
                    new_value = filter_event[F_COUNT - 1].filer_xy[f_type].event.value + ((int)new_d);
                   	event->value = (int)new_value;
                    touch_filter_debug(":new_d:%f, d_detail:%f, new_value:%f, high threshold:%d, err_gain:%f",
                        new_d, d_tail[f_type], new_value, (VECLOCITY_THRESHOLD[0]*TOUCH_PIXEL_DENSITY*t_t[3]/2000), err_gain);
                }
            }
            else if((((abs(orgin_d_d[3])*1000/TOUCH_PIXEL_DENSITY/t_t[3])) < (VECLOCITY_THRESHOLD[1]))
                &&(((abs(orgin_d_d[2])*1000/TOUCH_PIXEL_DENSITY/t_t[2])) < (VECLOCITY_THRESHOLD[1])))
            {
                touch_filter_debug(": veloc:%d, p_dir:0x%x, n_dir:0x%x", VECLOCITY_THRESHOLD[1], p_dir, n_dir);
                //if((orgin_p_dir == ((1<<F_COUNT) - 1)) || (orgin_n_dir == ((1<<F_COUNT) - 1)))
                {
				if (use_d_d) {
						for(i = 0; i < F_COUNT; i++)
						{
							new_d += d_d[i]*W_W[1][i];
						}
				} else {
                    for(i = 0; i < F_COUNT; i++)
                    {
                        new_speed += speed[i]*W_W[1][i];
                    }
                    new_d = new_speed * t_t[3];
				}
                    //err_gain =  (filter_event[F_COUNT - 1].orgin_event[f_type].value - filter_event[F_COUNT - 1].filer_xy[f_type].event.value)*0.3;
                    //new_d = new_d + err_gain;

		    /*
                    d_tail[f_type] += (new_d - (int)new_d);
                    if((int)abs(d_tail[f_type])>=1)
                    {
                        new_d += ((int)d_tail[f_type]);
                        d_tail[f_type] = (d_tail[f_type] - (int)d_tail[f_type]);
                    }
		    */
                    d_tail[f_type] = (new_d - (int)new_d);
		    if(d_tail[f_type] > 0){
		    	if(fabs(d_tail[f_type]) >=0.5)
                    	    new_d+=1.0;
		    }
                    if(d_tail[f_type] < 0){
		    	if(fabs(d_tail[f_type]) >=0.5)
                    	    new_d-=1.0;
		    }
                    new_value = filter_event[F_COUNT - 1].filer_xy[f_type].event.value + ((int)new_d);


                    event->value = (int)new_value;
                    touch_filter_debug(":new_d:%f, d_detail:%f, new_value:%f, high threshold:%d, err_gain:%f",
                        new_d, d_tail[f_type], new_value, (VECLOCITY_THRESHOLD[1]*TOUCH_PIXEL_DENSITY*t_t[3]/2000), err_gain);
                }
            }
            else if((((abs(orgin_d_d[3])*1000/TOUCH_PIXEL_DENSITY/t_t[3])) < (VECLOCITY_THRESHOLD[2]))
                  &&(((abs(orgin_d_d[2])*1000/TOUCH_PIXEL_DENSITY/t_t[2])) < (VECLOCITY_THRESHOLD[2])))
            {
                touch_filter_debug(": veloc:%d, p_dir:0x%x, n_dir:0x%x", VECLOCITY_THRESHOLD[2],p_dir, n_dir);
                //if((orgin_p_dir == ((1<<F_COUNT) - 1)) || (orgin_n_dir == ((1<<F_COUNT) - 1)))
                {
				if (use_d_d) {
						for(i = 0; i < F_COUNT; i++)
						{
							new_d += d_d[i]*W_W[2][i];
						}
				} else {
                    for(i = 0; i < F_COUNT; i++)
                    {
                        new_speed += speed[i]*W_W[2][i];
                    }
                    new_d = new_speed * t_t[3];
				}
                    //err_gain =  (filter_event[F_COUNT - 1].orgin_event[f_type].value - filter_event[F_COUNT - 1].filer_xy[f_type].event.value)*0.3;
                    //new_d = new_d + err_gain;

		    /*
                    d_tail[f_type] += (new_d - (int)new_d);
                    if((int)abs(d_tail[f_type])>=1)
                    {
                        new_d += ((int)d_tail[f_type]);
                        d_tail[f_type] = (d_tail[f_type] - (int)d_tail[f_type]);
                    }
		    */
                    d_tail[f_type] = (new_d - (int)new_d);
		    if(d_tail[f_type] > 0){
		    	if(fabs(d_tail[f_type]) >=0.5)
                    	    new_d += 1.0;
		    }
                    if(d_tail[f_type] < 0){
		    	if(fabs(d_tail[f_type]) >=0.5)
                    	    new_d -= 1.0;
		    }
                    new_value = filter_event[F_COUNT - 1].filer_xy[f_type].event.value + ((int)new_d);

                    event->value = (int)new_value;
                    touch_filter_debug(":new_d:%f, d_detail:%f, new_value:%f, high threshold:%d, err_gain:%f",
                        new_d, d_tail[f_type], new_value, (VECLOCITY_THRESHOLD[2]*TOUCH_PIXEL_DENSITY*t_t[3]/2000), err_gain);

                }

            } else {
			    touch_filter_debug(": veloc:%d up, p_dir:0x%x, n_dir:0x%x, orgin_p_dir:0x%x, orgin_n_dir:0x%x",
					VECLOCITY_THRESHOLD[2], p_dir, n_dir, orgin_p_dir, orgin_n_dir);
			if (use_d_d) {
				new_d = d_d[0] * 0.0 + d_d[1] * 0.1 + d_d[2] * 0.1 + d_d[3] * 0.8;
			} else {
				new_speed = speed[0] * 0.0 + speed[1] * 0.1 + speed[2] * 0.1 + speed[3] * 0.8;
				new_d = new_speed * t_t[3];
			}
				d_tail[f_type] = (new_d - (int)new_d);
			    if (d_tail[f_type] > 0) {
				    if(fabs(d_tail[f_type]) >= 0.5)
				        new_d += 1.0;
			    }
				if (d_tail[f_type] < 0) {
				    if(fabs(d_tail[f_type]) >= 0.5)
				        new_d -= 1.0;
			    }
				new_value = filter_event[F_COUNT - 1].filer_xy[f_type].event.value + ((int)new_d);
				event->value = (int)new_value;
				touch_filter_debug(":new_d:%f, d_detail:%f, new_value:%f, high threshold:%d up, err_gain:%f",
						new_d, d_tail[f_type], new_value, (VECLOCITY_THRESHOLD[2]*TOUCH_PIXEL_DENSITY*t_t[3]/2000), err_gain);
			}
#if 0
            /* handle false turning point */
            if(((p_dir != ((1<<F_COUNT) - 1))&& (orgin_p_dir == ((1<<F_COUNT) - 1)))
                ||((n_dir != ((1<<F_COUNT) - 1))&&(orgin_n_dir == ((1<<F_COUNT) - 1))))
            {
                err_gain =  (filter_event[F_COUNT - 1].orgin_event[f_type].value - filter_event[F_COUNT - 1].filer_xy[f_type].event.value)*0.3;
                event->value = event->value + err_gain;
                touch_filter_debug(":handle false turning point new_value:%d, err_gain:%f", event->value, err_gain);
            }
#endif
			if((orgin_d_d[0]==0 && orgin_d_d[1]==0) || (orgin_d_d[1]==0 && orgin_d_d[2]==0) || (orgin_d_d[2]==0 && orgin_d_d[3]==0)){
				touch_filter_debug("3D touch");
				event->value = orgin_d_d[3] + filter_event[F_COUNT - 1].filer_xy[f_type].event.value;
			} else {
				if(((orgin_event.value >= filter_event[F_COUNT - 1].orgin_event[f_type].value)&&(event->value <= filter_event[F_COUNT - 1].filer_xy[f_type].event.value))
					||((orgin_event.value <= filter_event[F_COUNT - 1].orgin_event[f_type].value)&&(event->value >= filter_event[F_COUNT - 1].filer_xy[f_type].event.value)))
				{
					touch_filter_debug("fanxianjiaozheng, before event->value = %d",  event->value);
					event->value = filter_event[F_COUNT - 1].filer_xy[f_type].event.value;
				}
				else
				{   //only process orientation really shift, do not process fast stop, now orientation shift maybe mistake of finger move
					if(orgin_p_dir * orgin_n_dir != 0 && t_t[3] <= 50 && t_t[2] <= 20 && abs(orgin_event.value - filter_event[F_COUNT - 1].orgin_event[f_type].value) > 2) {
						touch_filter_debug("fanxianjiaozheng_guaiwan before event->value = %d",  event->value);
						event->value = (orgin_event.value + filter_event[F_COUNT - 1].filer_xy[f_type].event.value)/2;
					}
				}
			}
			if(orgin_p_dir * orgin_n_dir != 0 && abs(orgin_event.value - filter_event[F_COUNT - 1].orgin_event[f_type].value) <= 2) {
					touch_filter_debug("fanxianjiaozheng_doudongle, before event->value = %d",  event->value);
					event->value = filter_event[F_COUNT - 1].filer_xy[f_type].event.value + (orgin_event.value - filter_event[F_COUNT - 1].orgin_event[f_type].value);
			}
            for(i= 0; i < F_COUNT; i++)
            {
                touch_filter_debug(":orgin_d_d[%d]:%d, d_d[%d]:%d, t_t[%d]:%d", i, orgin_d_d[i], i, d_d[i], i, t_t[i]);
            }
            //touch_filter_debug("-X: old_value:%d, new_value:%d", tmp,new_value);
            for(i = 0; i < F_COUNT - 1; i++)
            {
                memcpy(&filter_event[i].filer_xy[f_type].event, &filter_event[i+1].filer_xy[f_type].event, sizeof(struct input_event));
                /*Save orgin data*/
                memcpy(&filter_event[i].orgin_event[f_type], &filter_event[i+1].orgin_event[f_type], sizeof(struct input_event));
            }
            memcpy(&filter_event[i].filer_xy[f_type].event, event, sizeof(struct input_event));
            /*Save orgin data*/
            memcpy(&filter_event[i].orgin_event[f_type], &orgin_event, sizeof(struct input_event));

            filter_event[F_COUNT - 1].filer_xy[f_type].flag = 1;

        }
        else
        {
            memcpy(&filter_event[*count].filer_xy[f_type].event,event, sizeof(struct input_event));
            filter_event[*count].filer_xy[f_type].flag = 1;
            memcpy(&filter_event[*count].orgin_event[f_type], &orgin_event, sizeof(struct input_event));
            (*count)++;
        }


        if(f_type == 0)
        {
            touch_filter_debug("-X:after:type:%d, code:%d, value:%d, time:%ld.%06ld", event->type, event->code, event->value, event->time.tv_sec, event->time.tv_usec);
        }
        else if(f_type == 1)
        {
            touch_filter_debug("-Y:after:type:%d, code:%d, value:%d, time:%ld.%06ld", event->type, event->code, event->value, event->time.tv_sec, event->time.tv_usec);
        }

    }
}

void touch_driver_event_filter(struct input_event *event)
{
	static int flag_x = -1, flag_y = -1;
	static int flag_x_2 = -1, flag_y_2 = -1;
	static int flag_x_3 = -1, flag_y_3 = -1;
	int i;

#if 1
	if(init_filter_param() <0)
	{
		touch_filter_debug(" not  touch filter");
		return;
	}
#endif
	if(event->type == EV_KEY)
	{
		if(event->code == BTN_TOUCH && event->value == 1)//down
		{
			velocity[0].enable = false;
			velocity[1].enable = false;
			velocity_window[0].enable = false;
			velocity_window[1].enable = false;

			window[0] = 0;
			init_speed_window[0] = 0;
			speed_diff[0] = 0;
			pspeed_diff[0] = 0;
			window[1] = 0;
			init_speed_window[1] = 0;
			speed_diff[1] = 0;
			pspeed_diff[1] = 0;
			window_size[0] = 3;
			window_counter[0] = 0;
			window_size[1] = 3;
			window_counter[1] = 0;
			start[0] = true;
			start[1] = true;
			adj_max_d_flag[0] = 0;
			adj_max_d_flag[1] = 0;
			//smooth_velocity[0].enable = false;
			//smooth_velocity[1].enable = false;
			// acceleration[0].enable = false;
			// acceleration[1].enable = false;
			position[0].enable = false; position[1].enable = false;
			raw_position[0].enable = false; raw_position[1].enable = false;
			displacement[0].enable = false;
			displacement[1].enable = false;
			aaaaa[0] = 0;
			aaaaa[1] = 0;

			flag_x = flag_y = 0;
			flag_x_2 = flag_y_2 = 0;
			flag_x_3 = flag_y_3 = 0;
			// reset mt_slot variables which are used for multi-touch decision
			mt_slot_cnt = 0;
			is_multi_touch = 0;
			for (i = 0; i < MT_SLOT_HISTORY_MAX; i++)
				mt_slot_history[i] = -1;

			touch_filter_debug(" Down");
		}
		else if((event->code == BTN_TOUCH && event->value == 0) || ((event->code == ABS_MT_TRACKING_ID) && (event->value > 0)))//up or multitouch disable
		{
			velocity[0].enable = false;
			velocity[1].enable = false;
			velocity_window[0].enable = false;
			velocity_window[1].enable = false;
			// smooth_velocity[0].enable = false;
			// smooth_velocity[1].enable = false;
			// acceleration[0].enable = false;
			// acceleration[1].enable = false;
			position[0].enable = false; position[1].enable = false;
			raw_position[0].enable = false; raw_position[1].enable = false;
			displacement[0].enable = false;
			displacement[1].enable = false;
			window_size[0] = 3;
			window_counter[0] = 0;
			window_size[1] = 3;
			window_counter[1] = 0;
			start[0] = true;
			start[1] = true;


			flag_x = flag_y = -1;
			flag_x_2 = flag_y_2 = -1;
			flag_x_3 = flag_y_3 = -1;
			memset(&filter_event[0], 0, sizeof(struct filter_)*F_COUNT);
			d_tail[0] = d_tail[1] = 0;
			touch_filter_debug(":type:%d, code:%d, value:%d, time:%ld.%ld", event->type, event->code, event->value, event->time.tv_sec, event->time.tv_usec);
			window[0] = 0;
			window[1] = 0;
			init_speed_window[0] = 0;
			init_speed_window[1] = 0;
			speed_diff[0] = 0;
			speed_diff[1] = 0;
			pspeed_diff[0] = 0;
			adj_max_d_flag[0] = 0;
			adj_max_d_flag[1] = 0;

			aaaaa[0] = 0;
			aaaaa[1] = 0;
			// reset mt_slot variables which are used for multi-touch decision
			mt_slot_cnt = 0;
			is_multi_touch = 0;
			for (i = 0; i < MT_SLOT_HISTORY_MAX; i++)
				mt_slot_history[i] = -1;
			touch_filter_debug(" Up");
		}

	}

	if(event->type == EV_ABS)
	{
		// record the number of slots are received
		if ((event->code == ABS_MT_SLOT) && (mt_slot_cnt < MT_SLOT_HISTORY_MAX)) {
			for (i = 0; i < mt_slot_cnt; i++) {
				if (mt_slot_history[i] == event->value)
					break;
			}
			if (i == mt_slot_cnt)
				mt_slot_history[mt_slot_cnt++] = event->value;
		}
		if ((is_multi_touch == 0)
			&& (((event->code == ABS_MT_TRACKING_ID) && (event->value > 0)) || (mt_slot_cnt > 1))) // multi-touch
		{
			velocity[0].enable = false;
			velocity[1].enable = false;
			velocity_window[0].enable = false;
			velocity_window[1].enable = false;
			start[0] = true;
			start[1] = true;
			//smooth_velocity[0].enable = false;
			//smooth_velocity[1].enable = false;
			// acceleration[0].enable = false;
			// acceleration[1].enable = false;
			position[0].enable = false; position[1].enable = false;
			raw_position[0].enable = false; raw_position[1].enable = false;
			displacement[0].enable = false;
			displacement[1].enable = false;

			window[0] = 0;
			init_speed_window[0] = 0;
			speed_diff[0] = 0;
			pspeed_diff[0] = 0;
			window[1] = 0;
			init_speed_window[1] = 0;
			speed_diff[1] = 0;
			pspeed_diff[1] = 0;
			window_size[0] = 3;
			window_counter[0] = 0;
			window_size[1] = 3;
			window_counter[1] = 0;

			flag_x = flag_y = -1;
			flag_x_2 = flag_y_2 = -1;
			flag_x_3 = flag_y_3 = -1;
			memset(&filter_event[0], 0, sizeof(struct filter_)*F_COUNT);
			d_tail[0] = d_tail[1] = 0;
			is_multi_touch = 1;
			touch_filter_debug(":type:%d, code:%d, value:%d, time:%ld.%ld", event->type, event->code, event->value, event->time.tv_sec, event->time.tv_usec);
			touch_filter_debug(" multi-touch disable filter");
		}
	}


	if((flag_x >=0) && (flag_y >= 0)){
		touch_filter_function(event, &flag_x, &flag_y);
		/*
		  in protocal B, used SYN_REPORT to distinguish one point report,
		  if this report has only one X or Y, force set the X or Y which same
		  with previous X or Y to touch_filter_function().
		  The filter need every X/Y but miss anyone, so we compensate the
		  same X or Y here.
		  modify by Peng.Zhou
		*/
		if(event->type == EV_SYN) {
			if(event->code == SYN_REPORT) {
				/* only in X/Y changed do compensate point */
				if((s_x_flag == 1) || (s_y_flag == 1)) {
					if(s_x_flag == 0) {
					/*
					* update time of  compensate point
					*/
						origin_x_event.time.tv_sec = event->time.tv_sec;
						origin_x_event.time.tv_usec = event->time.tv_usec;
						memcpy(&event_align, &origin_x_event, sizeof(struct input_event));
						touch_filter_function(&event_align, &flag_x, &flag_y);
						s_x_flag = 0;
						touch_filter_debug("do type_B same X\n");
					}else {
						s_x_flag = 0;
						touch_filter_debug("do type_B diff X++\n");
					}

					if(s_y_flag == 0) {
					/*
					* update time of  compensate point
					*/
						origin_y_event.time.tv_sec = event->time.tv_sec;
						origin_y_event.time.tv_usec = event->time.tv_usec;
						memcpy(&event_align, &origin_y_event, sizeof(struct input_event));
						touch_filter_function(&event_align, &flag_x, &flag_y);
						s_y_flag = 0;
						touch_filter_debug("do type_B same Y\n");
					}else {
						s_y_flag = 0;
						touch_filter_debug("do type_B diff Y++\n");
					}
				}
			}
		}
	}

	if(event->code == ABS_MT_POSITION_X)
	{
		touch_filter_debug("-X: output:type:%d, code:%d, value:%d, time:%ld.%06ld", event->type, event->code, event->value, event->time.tv_sec, event->time.tv_usec);
	}
	else if(event->code == ABS_MT_POSITION_Y)
	{
		touch_filter_debug("-Y: output:type:%d, code:%d, value:%d, time:%ld.%06ld", event->type, event->code, event->value, event->time.tv_sec, event->time.tv_usec);
	}

}
