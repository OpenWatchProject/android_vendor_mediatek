/*
 * This program is test software;
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

/**************************/
/* INCLUDES                          */
/**************************/
#include <errno.h>
#include <fcntl.h>
//#include <features.h>
#include <paths.h>
#ifdef SD_ANDROID
#include <pathconf.h>
#endif
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <dlfcn.h>

#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <dirent.h>
#include <c2kutils.h>

/**************************/
/* DEFINES                            */
/**************************/
#define CMDM_IOCTL_LOOPBACK_GET    _IOR('c', 0x01, int)
#define CMDM_IOCTL_LOOPBACK_SET	_IOW('c', 0x02, int)

#define LOOPBACK_TESTFILE_NAME "/data/vendor/loopback"
#define LOG_FILE_PATH    "/data/vendor/flashless/loopback.log"
#define MUX_CHANNEL_KEYWORDS "/dev/pts/"
#define RETRY_TIMES 50


/*Logging*/\
#ifndef LOOPBACK_ANDROID
#include <syslog.h>

#define LOGLB(lvl,f,...) do{if(lvl<=syslog_level){\
								if (logToFile){\
								  fprintf(logFile,"%d:%s(): " f "\n", __LINE__, __FUNCTION__, ##__VA_ARGS__);\
								  fflush(logFile);}\
								else\
								  fprintf(stderr,"%d:%s(): " f "\n", __LINE__, __FUNCTION__, ##__VA_ARGS__);\
								}\
							}while(0)
#else //will enable logging using android logging framework (not to file)
#define LOG_TAG "LOOPBACK"
#include <utils/Log.h> //all Android LOG macros are defined here.
#define LOGLB(lvl,f,...) do{\
	if (logToFile){\
		fprintf(logFile,"%d:%s(): " f "\n", __LINE__, __FUNCTION__, ##__VA_ARGS__);\
		fflush(logFile);\
	}else{\
		if(lvl<=syslog_level){\
		LOG_PRI(android_log_lvl_convert[lvl],LOG_TAG,"%d:%s(): " f, __LINE__, __FUNCTION__, ##__VA_ARGS__);}\
	}\
}while(0)

//just dummy defines since were not including syslog.h.
#define LOG_EMERG	0
#define LOG_ALERT	1
#define LOG_CRIT	2
#define LOG_ERR		3
#define LOG_WARNING	4
#define LOG_NOTICE	5
#define LOG_INFO	6
#define LOG_DEBUG	7

int android_log_lvl_convert[8]={ANDROID_LOG_SILENT, /*8*/
								ANDROID_LOG_SILENT, /*7*/
								ANDROID_LOG_FATAL, /*6*/
								ANDROID_LOG_ERROR,/*5*/
								ANDROID_LOG_WARN,/*4*/
								ANDROID_LOG_INFO,/*3*/
								ANDROID_LOG_DEBUG,/*2*/
								ANDROID_LOG_VERBOSE};/*1*/


#endif /*SD_ANDROID*/

#define SYSCHECK(c) do{if((c)<0){LOGLB(LOG_ERR,"system-error: '%s' (code: %d)", strerror(errno), errno);\
						return -1;}\
					}while(0)


#ifndef min
#define min(a,b) ((a < b) ? a :b)
#endif




/**************************/
/* TYPES                                */
/**************************/
typedef enum {
         STATE_LOOPBACK_SUCCESS  = 0, //操作成功
         STATE_LOOPBACK_WORK = 1,    //loopback测试中
         STATE_LOOPBACK_CLOSED = 2,  //测试模式关闭状态
         STATE_LOOPBACK_INVALID = 3,  //测试指令无效
         STATE_LOOPBACK_FAIL = 4,     //操作失败
         STATE_LOOPBACK_NUM,
}CP_LOOPBACK_STATE;


typedef struct Serial
{
	int channel;
	char *devicename;
	int fd;
	time_t start_time;
    pthread_mutex_t	write_mutex;
    pthread_mutex_t	read_mutex;
    // Condition variable used by deferred action thread
    pthread_cond_t write_cond;
    pthread_cond_t read_cond;
	unsigned int test_count;
	unsigned int err_count;
	unsigned int file_size;
	int data_count;
	int time_count;
	float download_speed;
	float upload_speed;
} SerialDev;

struct port_info{
	int	count;		/* Usage count */
	unsigned short state;		/*loopback state*/
};


/* Struct is used for passing fd, read function and read funtion arg to a device polling thread */



/**************************/
/* CONSTANTS & GLOBALS     */
/**************************/
// config stuff
static char* revision = "$Rev: 1 $";
/*pthread */
static pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t m_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t syslogdump_lock;
pthread_mutex_t main_exit_signal_lock;
pthread_attr_t thread_attr;
pthread_t m_thread_id;
pthread_t thread_id_write[VIATEL_CHANNEL_NUM];
pthread_t thread_id_read[VIATEL_CHANNEL_NUM];
pthread_mutex_t test_lock;
static int main_exit_signal[VIATEL_CHANNEL_NUM]={0};  /* 0:main() received exit signal */
static SerialDev serial[VIATEL_CHANNEL_NUM];
unsigned int onetime_size[VIATEL_CHANNEL_NUM] = {255,255,255,255,255,255,255,255};
unsigned int random_data[VIATEL_CHANNEL_NUM] = {0};
unsigned int channel = 0;
static int csd_restart = 0;//restart sd if any error ocur
static int csd_port_speed = 5; //115200 baud rate
static int syslog_level = LOG_DEBUG;
static int logToFile = 0;
static FILE * logFile = NULL;
static time_t tStart;

/*
 * The following arrays must have equal length and the values must
 * correspond. also it has to correspond to the xxxxxxx spec regarding
 * baud id of CSD the command.
 */
static int baud_rates[] = {
	0, 9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600
};
static speed_t baud_bits[] = {
	0, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B921600
};

static void set_main_exit_signal(int signal);


/**************************/
/* MAIN CODE                        */
/**************************/
/*
* Purpose:  Determine baud-rate index for CSD command
* Input:	    baud_rate - baud rate (eg. 460800)
* Return:    -1 if fail, i - baud rate index if success
*/
static int baud_rate_index(
			int baud_rate)
{
	unsigned int i;
	for (i = 0; i < sizeof(baud_rates) / sizeof(*baud_rates); ++i)
		if (baud_rates[i] == baud_rate)
			return i;
	return -1;
}

/*
* Purpose:  ascii/hexdump a byte buffer 
* Input:	    prefix - string to printed before hex data on every line
*                ptr - the string to be dumped
*                length - the length of the string to be dumped
* Return:    0
*/
static int syslogdump(
				const char *prefix,
				const unsigned char *ptr,
				unsigned int length)
{
if (LOG_DEBUG>syslog_level) /*No need for all frame logging if it's not to be seen */
	return 0;

	char buffer[100];
	unsigned int offset = 0l;
	int i=0;
	
	pthread_mutex_lock(&syslogdump_lock); 	//new lock
	while (offset < length)
	{
		int off;
		strcpy(buffer, prefix);
		off = strlen(buffer);
		SYSCHECK(snprintf(buffer + off, sizeof(buffer) - off, "%08x: ", offset));
		off = strlen(buffer);
		for (i = 0; i < 16; i++)
		{
			if (offset + i < length){
				SYSCHECK(snprintf(buffer + off, sizeof(buffer) - off, "%02x%c", ptr[offset + i], i == 7 ? '-' : ' '));
				}
			else{
				SYSCHECK(snprintf(buffer + off, sizeof(buffer) - off, " .%c", i == 7 ? '-' : ' '));
				}
			off = strlen(buffer);
		}
		SYSCHECK(snprintf(buffer + off, sizeof(buffer) - off, " "));
		off = strlen(buffer);
		for (i = 0; i < 16; i++)
			if (offset + i < length)
			{
				SYSCHECK(snprintf(buffer + off, sizeof(buffer) - off, "%c", (ptr[offset + i] < ' ') ? '.' : ptr[offset + i]));
				off = strlen(buffer);
			}
		offset += 16;
		LOGLB(LOG_DEBUG,"%s", buffer);
	}
	pthread_mutex_unlock(&syslogdump_lock);/*new lock*/

	return 0;
}



/* 
* Purpose:  Function responsible by all signal handlers treatment any new signal must be added here
* Input:      param - signal ID
* Return:    -
*/
void signal_treatment(
	int param)
{
	switch (param)
	{
	case SIGPIPE:
				exit(0);
				break;
	case SIGHUP:
//reread the configuration files
	break;
	case SIGINT:
	case SIGTERM:
	case SIGUSR1:
             set_main_exit_signal(0);
	case SIGKILL:
			set_main_exit_signal(0);
			break;
	default:
			exit(0);
			break;
	}
}


/* 
* Purpose:  Creates a detached thread. also checks for errors on exit.
* Input:      thread_id - pointer to pthread_t id
*                thread_function - void pointer to thread function
*                thread_function_arg - void pointer to thread function args
* Return:    0 if success, 1 if fail
*/
int create_thread(pthread_t * thread_id, 
					void * thread_function, 
					void * thread_function_arg )
{
	LOGLB(LOG_DEBUG,"Enter");
	pthread_attr_init(&thread_attr);
	pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED);

	if(pthread_create(thread_id, &thread_attr, thread_function, thread_function_arg)!=0)
	{
		switch (errno)
		{
		  case EAGAIN:
		    LOGLB(LOG_ERR,"Interrupt signal EAGAIN caught");
		    break;
		  case EINVAL:
		    LOGLB(LOG_ERR,"Interrupt signal EINVAL caught");
		    break;
		  default:
		    LOGLB(LOG_ERR,"Unknown interrupt signal caught");
		}
		LOGLB(LOG_ERR,"Could not create thread");
		set_main_exit_signal(0); //exit main function if thread couldn't be created
		return 1;
	}
	pthread_attr_destroy(&thread_attr); /* Not strictly necessary */
	return 0; //thread created successfully
}


/* 
* Purpose:  Open and initialize the serial device used.
* Input:      serial - the serial struct
* Return:    0 if port successfully opened, else 1.
*/
int loopback_open_device(SerialDev* serial)
{
	LOGLB(LOG_DEBUG, "Enter");
	unsigned int i;
	
/* open the serial port */
	SYSCHECK(serial->fd = open(serial->devicename, O_RDWR | O_NOCTTY | O_NONBLOCK));
	LOGLB(LOG_NOTICE, "Opened serial port device = %s ",serial->devicename);
	int fdflags;
	SYSCHECK(fdflags = fcntl(serial->fd, F_GETFL));
	SYSCHECK(fcntl(serial->fd, F_SETFL, fdflags & O_NONBLOCK));
	struct termios t;
	tcgetattr(serial->fd, &t);
	t.c_cflag &= ~(CSIZE | CSTOPB | PARENB | PARODD);	
	t.c_cflag |= CREAD | CLOCAL | CS8 ;
	t.c_cflag &= ~(CRTSCTS);
	t.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG);
	t.c_iflag &= ~(INPCK | IGNPAR | PARMRK | ISTRIP | IXANY | ICRNL);
	t.c_iflag &= ~(IXON | IXOFF);
	t.c_oflag &= ~(OPOST | OCRNL);
	t.c_cc[VMIN] = 1;
	t.c_cc[VTIME] = 0;

	//Android does not directly define _POSIX_VDISABLE. It can be fetched using pathconf()
	long posix_vdisable;
	char cur_path[FILENAME_MAX];
       if (!getcwd(cur_path, sizeof(cur_path))){
         LOGLB(LOG_ERR,"_getcwd returned errno %d",errno);
         return 1;
       }
    posix_vdisable = pathconf(cur_path, _PC_VDISABLE);
	t.c_cc[VINTR] = posix_vdisable;
	t.c_cc[VQUIT] = posix_vdisable;
	t.c_cc[VSTART] = posix_vdisable;
	t.c_cc[VSTOP] = posix_vdisable;
	t.c_cc[VSUSP] = posix_vdisable;

	speed_t speed = baud_bits[csd_port_speed];
	cfsetispeed(&t, speed);
	cfsetospeed(&t, speed);
	SYSCHECK(tcsetattr(serial->fd, TCSANOW, &t));
	int status = TIOCM_DTR | TIOCM_RTS;
	ioctl(serial->fd, TIOCMBIS, &status);
	LOGLB(LOG_INFO, "Configured serial device");
	time(&serial->start_time); //get the current time

	LOGLB(LOG_DEBUG, "serial->fd = %d ",serial->fd);
	return 0;
}

/* 
* Purpose:  Close all devices, send sd termination frames
* Input:      -
* Return:    0
*/
static int loopback_close_devices(SerialDev* serial)
{
	LOGLB(LOG_DEBUG, "Enter");
	int i=0,try_number=0;


	LOGLB(LOG_INFO, "serial.fd = %x",serial->fd);
	if (serial->fd >= 0)
	{
		LOGLB(LOG_INFO, "close device = %s",serial->devicename);
		SYSCHECK(close(serial->fd));
		serial->fd = -1;
	}
	LOGLB(LOG_DEBUG, "Leave");
	return 0;
}
static int loopback_data_checkout(char *srcfile,char* dstfile)
{
    int fd_src = -1,fd_dst=-1;
	unsigned int rsize=0, fsize = 0,sum=0;
    struct stat info_src,info_dst;
	char buf_src[1024]={0};
	char buf_dst[1024]={0};
	int count_src=0,count_dst=0;
	fd_src = open(srcfile, O_RDONLY);
	if(fd_src < 0){
		LOGLB(LOG_ERR, "Fail to open %s\n", srcfile);
		fsize = 0;
		return 0;
	}
	/*get test file information*/
	if(stat(srcfile, &info_src) < 0){
		LOGLB(LOG_ERR, "Fail to stat %s\n", srcfile);
		fsize = 0;
	}else{
		fsize = info_src.st_size;
		LOGLB(LOG_DEBUG, "%d fsize=%d",__LINE__,fsize);
	}

	fd_dst = open(dstfile, O_RDONLY);
	if(fd_dst < 0){
		LOGLB(LOG_ERR, "Fail to open %s\n", dstfile);
		fsize = 0;
		close(fd_src);
		return 0;
	}
	LOGLB(LOG_DEBUG, "%d fd_dst=%d",__LINE__,fd_dst);
	if(stat(dstfile, &info_dst) < 0){
		LOGLB(LOG_ERR, "Fail to stat %s\n", dstfile);
		fsize = 0;
	}else{
		fsize = info_dst.st_size;
		LOGLB(LOG_DEBUG, "%d fsize=%d",__LINE__,fsize);
	}

	if(info_src.st_size != info_dst.st_size){
		LOGLB(LOG_ERR, "%s %s file size diff",srcfile,dstfile);
		close(fd_src);
		close(fd_dst);
		return 0;
	}

	while(sum <fsize){
		memset(buf_src,0,sizeof(buf_src));
		count_src = read(fd_src,buf_src,min(sizeof(buf_src),fsize-sum));
		memset(buf_dst,0,sizeof(buf_dst));
		count_dst = read(fd_dst,buf_dst,min(sizeof(buf_dst),fsize-sum));
		if(count_src != count_dst)
			break;
		if(memcmp(buf_src,buf_dst,count_src)!=0){
			LOGLB(LOG_DEBUG, "diff found");
			break;
		}
		sum += count_src;
	}

	close(fd_src);
	close(fd_dst);
	if(sum != fsize)
		return 0;
	return 1;
}

static char *replace(char *source,char *sub,char *rep)
{
	char *result;
	char *pc1, *pc2;
	int isource;/*源字符串的长度*/
	int isub;	/*对比字符串的长度*/
	int irep;	/*替换字符串的长度*/
	int pos=0;

	if((NULL == sub)|| (NULL == source) || (NULL == rep) )
		return NULL;
	isource = strlen(source);
	isub = strlen(sub);
	irep = strlen(rep);
	/*申请结果串需要的空间*/
	result = malloc((isource-isub+irep)*sizeof(char));
	memset(result,0,(isource-isub+irep)*sizeof(char));
	pc1 = strstr(source,sub);
	if(pc1 !=NULL){
		memcpy(result,source,pc1-source);
		pc2=source + isub-1;
		pos += pc1-source;
		memcpy(result+pos,rep,irep-1);
		pos = pos + irep-1;
		strcpy(result+pos,pc2);
	}
	return result;
}

static int generate_test_data(SerialDev *dev_info,unsigned int file_size)
{
	unsigned int i=0,j=0;
	unsigned int div = 0;
	unsigned int last = 0;
	char buf[1024];
	char src_path[255];
	int ret = 0;
	int fd=0;

	snprintf(src_path, sizeof(src_path), "%s%d",
			LOOPBACK_TESTFILE_NAME,dev_info->channel);
	unlink(src_path);
	fd = open(src_path, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if(fd < 0){/*no,create file failed too,let finish the task*/
		LOGLB(LOG_ERR, "Fail to create %s: %s (code: %d)\n", src_path, strerror(errno), errno);
		ret = -1;
		goto terminate;
	}
	div = file_size / sizeof(buf);/*generate random data for test,sizeof(buf) bytes one time,and div times*/
	last = file_size % sizeof(buf);/*if file_size can't be divided by sizeof(buf) exactly*/
	LOGLB(LOG_DEBUG, "div=%d last=%d",div,last);
	if(random_data[dev_info->channel]){
		srand((unsigned int )time(NULL));
		for(j=0;j<div;j++){
			memset(buf,0,sizeof(buf));
			for(i=0;i<sizeof(buf);i++){
				buf[i] = rand()%255; /*generate one byte random data*/
			}
			ret =write(fd,buf,sizeof(buf));/*store to file*/
			LOGLB(LOG_DEBUG, "ret=%d",ret);
			if(ret != sizeof(buf)){
				LOGLB(LOG_ERR, "Fail to write all data to %s\n", src_path);
				ret = -1;
				goto terminate;
			}
		}
		memset(buf,0,sizeof(buf));
		if(last!=0){
			for(i=0;i<last;i++){
				buf[i] = rand()%255; 
			}
			ret =write(fd,buf,last);
			if((unsigned int)ret != last){
				LOGLB(LOG_ERR, "Fail to write last data to %s\n", src_path);
				ret = -1;
				goto terminate;
			}
		}
	}else{
		for(j=0;j<div;j++){
			buf[0] = dev_info->test_count & 0xff;
			buf[1] = (dev_info->test_count & 0xff00)>>8;
			buf[2] = (dev_info->test_count & 0xff0000)>>16;
			buf[3] = (dev_info->test_count & 0xff000000)>>24;
			memset(buf+4,j,sizeof(buf)-4);
			ret =write(fd,buf,sizeof(buf));/*store to file*/
			LOGLB(LOG_DEBUG, "ret=%d",ret);
			if(ret != sizeof(buf)){
				LOGLB(LOG_ERR, "Fail to write all data to %s\n", src_path);
				ret = -1;
				goto terminate;
			}
			
		}
		if(last!=0){
			buf[0] = dev_info->test_count & 0xff;
			buf[1] = (dev_info->test_count & 0xff00)>>8;
			buf[2] = (dev_info->test_count & 0xff0000)>>16;
			buf[3] = (dev_info->test_count & 0xff000000)>>24;
			memset(buf+4,++j,sizeof(buf)-4);
			ret =write(fd,buf,last);
			if((unsigned int)ret != last){
				LOGLB(LOG_ERR, "Fail to write last data to %s\n", src_path);
				ret = -1;
				goto terminate;
			}
		}
	}	
	close(fd);
	return 0;
terminate:
	close(fd);
	return ret;
}



void* data_poll_thread(void *vargp) 
{
	SerialDev * serial= (SerialDev *)vargp;
	int channel=serial->channel;
	unsigned int packets_len = 0;
	struct timeval  begin_time,end_time;
	struct timeval timeout;
	unsigned int sum = 0;
	char src_path[255];
    struct timeval tv;
	char *dst_path=NULL;
	FILE *fp = NULL;
	float speed=0.0;
	char buf[1024];
	int count = 0;
	int ret = 0;
	fd_set fdr;
	int time ;

	LOGLB(LOG_DEBUG,"Enter");
	while (main_exit_signal[channel]!=0)
	{
		LOGLB(LOG_NOTICE, "wait read_mutex");
        pthread_mutex_lock(&serial->read_mutex);
		pthread_cond_wait(&serial->read_cond,&serial->read_mutex);
        pthread_mutex_unlock(&serial->read_mutex);
		LOGLB(LOG_NOTICE, "get read_mutex");
		sum = 0;
		dst_path = replace(serial->devicename,"/dev/","/data/vendor/");
		if(dst_path == NULL){
			LOGLB(LOG_ERR,"invalid device name %s",dst_path);
			break;
		}
		snprintf(src_path, sizeof(src_path), "%s%d",
				LOOPBACK_TESTFILE_NAME,serial->channel);
		fp = fopen(dst_path,  "w");
		if(fp == NULL){
			LOGLB(LOG_ERR, "Fail to open %s\n", dst_path);
			break;
		}
		LOGLB(LOG_DEBUG, "serial->file_size=%d sum=%d\n", serial->file_size,sum);
		gettimeofday(&begin_time, (struct timezone *) NULL);
		//time(&begin_time); //get the current time
		while(serial->file_size > sum){
			LOGLB(LOG_NOTICE, "serial->file_size=%d sum=%d", serial->file_size,sum);
			timeout.tv_sec = 0;
			timeout.tv_usec = 1000*1000*5;//500ms
			FD_ZERO(&fdr);
			FD_SET(serial->fd,&fdr);

			ret =select((1+serial->fd),&fdr,NULL,NULL,&timeout);
			//LOGLB(LOG_DEBUG, "ret=%d serial->fd=%d",ret,serial->fd);
			if (ret >0) 
			{
				if (FD_ISSET(serial->fd,&fdr))
				{
					memset(buf,0,sizeof(buf));
					count = read(serial->fd, buf,min(sizeof(buf), serial->file_size - sum));
					//LOGLB(LOG_DEBUG, "read %d bytes",count);
					if(count <= 0){ /*Call reading function*/
						LOGLB(LOG_ERR, "Device read function returned error count=%d",count);
						break;
					}
				}
				sum += count;
				//LOGLB(LOG_DEBUG, "count=%d sum=%d serial->fd=%d",count,sum,serial->fd);
				ret = fwrite(buf,count,1,fp);
				fflush(fp);
				//LOGLB(LOG_DEBUG, "write %d bytes",ret);
				if(ret <= 0){
					LOGLB(LOG_ERR, "write to file %s failed ret=%d",dst_path,ret);
					break;
				}
			}
			else if(ret == 0)
			{
			     LOGLB(LOG_DEBUG, "Device read function wake up ");
			}
			else
			{ //No need to evaluate retval=0 case, since no use of timeout in select()
			  switch (errno)
			  {
			    case EINTR:
			      LOGLB(LOG_ERR,"Interrupt signal EINTR caught");
			      break;
			    case EAGAIN:
			      LOGLB(LOG_ERR,"Interrupt signal EAGAIN caught");
			      break;
			    case EBADF:
			      LOGLB(LOG_ERR,"Interrupt signal EBADF caught");
			      break;
			    case EINVAL:
			      LOGLB(LOG_ERR,"Interrupt signal EINVAL caught");
			      break;
			    default:
			      LOGLB(LOG_ERR,"Unknown interrupt signal caught\n");
			  }
			  break;
			}
		}
		if(serial->file_size != sum){
			goto terminate;
		}
		gettimeofday(&end_time, (struct timezone *) NULL);
		time = (end_time.tv_sec- begin_time.tv_sec)*1000+(end_time.tv_usec-begin_time.tv_usec)/1000;
		//time(&end_time); //get the current time
		fclose(fp);
		LOGLB(LOG_NOTICE, "serial->file_size=%d sum=%d\n", serial->file_size,sum);
		if(loopback_data_checkout(src_path,dst_path)){
			LOGLB(LOG_NOTICE,"data checkout ok\n");
			speed = serial->file_size / (time?time:1);
			serial->upload_speed = (serial->upload_speed*serial->test_count +speed)/(serial->test_count +1);
			serial->test_count ++;
		}else{
			serial->err_count ++;
			serial->test_count ++;
			goto terminate;
		}
		if(dst_path != NULL)
			unlink(dst_path);
		usleep(1000*50);
		pthread_cond_signal(&(serial->write_cond));
		LOGLB(LOG_NOTICE,"signal write_cond\n");
	}

terminate:
	LOGLB(LOG_NOTICE, "%s terminate tyserial->file_size=%d sum=%d\n", __FUNCTION__,serial->file_size,sum);
	fclose(fp);
	set_main_exit_signal(0); //error happen,exit
    return NULL;
}



void* write_data_thread(void *vargp) 
{
    int fd = -1;
	unsigned int rsize=0, fsize = 0,sum=0;
	SerialDev * serial= (SerialDev *)vargp;
	int channel=serial->channel;
	struct timeval begin_time,end_time;
	struct timeval timeout;
	int retry_times = 0;
    struct stat info;
	char src_path[255];
	char buf[1024];
	fd_set fdr,fdw;
	float speed=0.0;
	int count=0;
	int ret = 0;
	int time ;

	LOGLB(LOG_DEBUG,"Enter");

	memset(src_path,0,sizeof(src_path));
	snprintf(src_path, sizeof(src_path), "%s%d",
			LOOPBACK_TESTFILE_NAME,serial->channel);
	LOGLB(LOG_ERR, "src_path=%s\n", src_path);
	while(main_exit_signal[channel]!=0){
		LOGLB(LOG_NOTICE, "wait write_cond");
        pthread_mutex_lock(&serial->write_mutex);
		pthread_cond_wait(&serial->write_cond,&serial->write_mutex);
        pthread_mutex_unlock(&serial->write_mutex);
		LOGLB(LOG_NOTICE, "get write_cond");

		retry_times = 0;
		ret = generate_test_data(serial,onetime_size[channel]);
		if(ret != 0){
			LOGLB(LOG_ERR, "generate test data failed ret=%d",ret);
		}
		fd = open(src_path, O_RDONLY);
		LOGLB(LOG_ERR, "fd=%d\n", fd);
		if(fd < 0){
			LOGLB(LOG_ERR, "Fail to open %s\n", src_path);
			fsize = 0;
		}
		if(fd >=0){
			/*get test file information*/
			LOGLB(LOG_ERR, "line=%d\n", __LINE__);
			if(stat(src_path, &info) < 0){
	            LOGLB(LOG_ERR, "Fail to stat %s\n", src_path);
	            fsize = 0;
			}else{
				LOGLB(LOG_ERR, "line=%d\n", __LINE__);
	            fsize = info.st_size;
				serial->file_size = fsize;
				LOGLB(LOG_DEBUG,"serial->file_size=%d",serial->file_size);
	        }
		}
		sum = 0;
		rsize = 0;
		usleep(1000*50);
		gettimeofday(&begin_time, (struct timezone *) NULL);
		pthread_cond_signal(&(serial->read_cond));
		LOGLB(LOG_NOTICE,"signal read_cond\n");
		while (fsize > rsize){
			LOGLB(LOG_DEBUG,"fsize=%d rsize=%d retry_times=%d" ,fsize,rsize,retry_times);
			if(retry_times >= RETRY_TIMES){
				LOGLB(LOG_ERR, "retry overrun in one round of loopback test");
				goto terminate;
			}
	        if((rsize < fsize)){
				memset(buf,0,sizeof(buf));
	            ret = read(fd, buf, min(sizeof(buf), fsize - rsize));
				//LOGLB(LOG_DEBUG,"read %d bytes",ret);
	            if(ret < 0){
	                LOGLB(LOG_ERR, "Fail to read image error: %s (code: %d)\n", strerror(errno), errno);
	                ret = -1;
	                goto terminate;
	            }
	            rsize += ret;
	        }
			do{
				timeout.tv_sec = 0;
				timeout.tv_usec = 1000*100*5;//500ms
				FD_ZERO(&fdw);
				FD_SET(serial->fd,&fdw);
				ret =select((1 +serial->fd),NULL,&fdw,NULL,&timeout);
				//LOGLB(LOG_DEBUG,"ret=%d",ret);
				if(ret == 0){
					 retry_times ++;
					 LOGLB(LOG_NOTICE, "select writeable timeout,try again%d",retry_times);
					 continue;
				}else if (ret >0) {
					if (FD_ISSET(serial->fd,&fdw)){
						count = write(serial->fd, buf, min(sizeof(buf), (rsize-sum)));
						//LOGLB(LOG_DEBUG,"write %d bytes",count);
						if(count <= 0){
							LOGLB(LOG_ERR, "fail to write to cbp device count=%d (error:%s, errorno:%d)\n",
								count,strerror(errno), errno);
							serial->err_count ++;
							break;
						}
						sum += count;
					}
				}else{
					LOGLB(LOG_DEBUG,"select %s error ret=%d",serial->devicename,ret);
					perror("select");
					break;
				}
			}while(sum < rsize);
			if(sum <rsize)
				goto terminate;
		}
		LOGLB(LOG_NOTICE,"fsize=%d rsize=%d retry_times=%d\n" ,fsize,rsize,retry_times);
		gettimeofday(&end_time, (struct timezone *) NULL);
		time = (end_time.tv_sec- begin_time.tv_sec)*1000+(end_time.tv_usec-begin_time.tv_usec)/1000;
		ret = close(fd);
		speed = serial->file_size / (time?time:1);
		serial->download_speed = (serial->download_speed*serial->test_count +speed)/(serial->test_count +1);
	}
terminate:
	LOGLB(LOG_NOTICE,"%s terminate fsize=%d rsize=%d retry_times=%d\n" ,__FUNCTION__,fsize,rsize,retry_times);
	return NULL;
}


static int channel_is_muxed(char *path,int *muxed,int *mux_num)
{
	char *ptr;
	
	if(path ==NULL)
		return -1;

	ptr = strstr(path,MUX_CHANNEL_KEYWORDS);
	if(ptr != NULL){
		*muxed =1;
		*mux_num = atoi(ptr+sizeof(MUX_CHANNEL_KEYWORDS));
		LOGLB(LOG_DEBUG,"*muxed=%d *mux_num=%d " ,*muxed,*mux_num);
		if(*mux_num > 0x0f){
			*muxed = 0;
			*mux_num =0;
			return -1;
		}
	}

	return 0;
}


static int enable_cp_loopback_mode(SerialDev *dev_info,int op)
{
	struct port_info info;
	int muxif=0,muxnum=0;
	char *mux_base_channel;
	int fd =0;
#if 0
	if(channel_is_muxed(dev_info->devicename,&muxif,&muxnum) !=0){
		LOGLB(LOG_DEBUG,"query if the channel is muxed failed");
		return -1;
	}
#endif
	if(muxif){
		mux_base_channel = viatelAdjustDevicePathFromProperty(VIATEL_CHANNEL_MUX);
		if(mux_base_channel !=NULL){
			fd = open(mux_base_channel, O_RDWR | O_NOCTTY | O_NONBLOCK);
			if(fd >=0){
				memset(&info,0,sizeof(info));
				info.state |= muxnum;
				if (ioctl(fd, CMDM_IOCTL_LOOPBACK_GET, &info)) {
					LOGLB(LOG_ERR,"CMDM_IOCTL_LOOPBACK_GET failed");
					perror("CMDM_IOCTL_LOOPBACK_GET");
					return -1;
				}
				LOGLB(LOG_DEBUG,"use count=%d state=%d",info.count,info.state);
				if(info.state != STATE_LOOPBACK_WORK){
					memset(&info,0,sizeof(info));
					if(op)
						info.state |= 0x8000;/*highest bit 1 : enable loopback mode ; 0 : disable loopback mode */
					else
						info.state &= ~0x7fff;
					info.state |= muxnum;
					if (ioctl(fd, CMDM_IOCTL_LOOPBACK_SET, &info)) {
						LOGLB(LOG_ERR,"CMDM_IOCTL_LOOPBACK_SET failed");
						perror("CMDM_IOCTL_LOOPBACK_SET");
						return -1;
					}
					memset(&info,0,sizeof(info));
					sleep(1);
					if (ioctl(fd, CMDM_IOCTL_LOOPBACK_GET, &info)) {
						LOGLB(LOG_ERR,"CMDM_IOCTL_LOOPBACK_GET failed");
						perror("CMDM_IOCTL_LOOPBACK_GET");
						return -1;
					}
					if(info.state != STATE_LOOPBACK_WORK){
						LOGLB(LOG_ERR,"enable device %s mux channel %d loopback mode failed state[%d]",
									mux_base_channel,muxnum,info.state);
						return -1;
					}
				}else{
					LOGLB(LOG_NOTICE,"device %s mux channel %d already in loopback mode",
						mux_base_channel,muxnum);
				}
			}else{
				LOGLB(LOG_DEBUG,"open device %s failed",mux_base_channel);
			}
			close(fd);
		}else{
			LOGLB(LOG_DEBUG,"mux base device NULL");
		}
		
	}else{

#if 0
		memset(&info,0,sizeof(info));
		if (ioctl(dev_info->fd, CMDM_IOCTL_LOOPBACK_GET, &info)) {
			LOGLB(LOG_ERR,"CMDM_IOCTL_LOOPBACK_GET failed");
			perror("CMDM_IOCTL_LOOPBACK_GET");
			return -1;
		}
		LOGLB(LOG_DEBUG,"use count=%d state=%d",info.count,info.state);
		if(info.count >1){
			LOGLB(LOG_ERR,"device %s busy,other APP is using it",dev_info->devicename);
			return -1;
		}
		if(info.state >=STATE_LOOPBACK_NUM){
			LOGLB(LOG_ERR,"invalid cp loopback state %d",info.state);
		}
		if(info.state != STATE_LOOPBACK_WORK){/*if not working,enable it */
			memset(&info,0,sizeof(info));
			if(op)
				info.state |= 0x8000;/*highest bit 1 : enable loopback mode ; 0 : disable loopback mode */
			else
				info.state &= ~0x7fff;
			if (ioctl(dev_info->fd, CMDM_IOCTL_LOOPBACK_SET, &info)) {
				LOGLB(LOG_ERR,"CMDM_IOCTL_LOOPBACK_SET failed");
				perror("CMDM_IOCTL_LOOPBACK_SET");
				return -1;
			}
			sleep(1);
			if (ioctl(dev_info->fd, CMDM_IOCTL_LOOPBACK_GET, &info)) {
				LOGLB(LOG_ERR,"CMDM_IOCTL_LOOPBACK_GET failed");
				perror("CMDM_IOCTL_LOOPBACK_GET");
				return -1;
			}
			if(info.state != STATE_LOOPBACK_WORK){/*if not working,enable it */
				LOGLB(LOG_ERR,"enable device %s loopback mode failed,state[%d]",
							dev_info->devicename,info.state);
				return -1;
			}
		}else{
			LOGLB(LOG_NOTICE,"device %s already in loopback mode",
				dev_info->devicename);
		}
#else
	int loopback_opt_fd=-1;
	int ret=-1;
	loopback_opt_fd = open("/sys/viatel/modem_sdio/loop_back_mod", O_RDWR | O_NOCTTY | O_NONBLOCK);
	if(loopback_opt_fd >=0){
		if(op){
			LOGLB(LOG_NOTICE,"open loopback");
			ret=write(loopback_opt_fd,"1",1);
		}
		else{
			LOGLB(LOG_NOTICE,"close loopback");
			ret=write(loopback_opt_fd,"2",1);
		}
		if(ret !=1){
			LOGLB(LOG_NOTICE,"loopback ctrl device write failed ret=%d [%s]",errno,strerror(errno));
		}
		
	}else{
		LOGLB(LOG_NOTICE,"loopback ctrl device open failed");
	}

#endif
	}

	return 0;
}

static int disable_cp_loopback_mode(SerialDev *dev_info)
{
	struct port_info info;
	int muxif=0,muxnum=0;

	if (ioctl(dev_info->fd, CMDM_IOCTL_LOOPBACK_GET, &info)) {
		perror("CMDM_IOCTL_LOOPBACK_GET");
		return -1;
	}
	if(info.state >=STATE_LOOPBACK_NUM){
		LOGLB(LOG_DEBUG,"invalid cp loopback state %d",info.state);
	}
	if(info.state != STATE_LOOPBACK_CLOSED){/*if not working,enable it */
		info.state = 0;/*highest bit 1 : enable loopback mode ; 0 : disable loopback mode */
		if(channel_is_muxed(dev_info->devicename,&muxif,&muxnum) !=0){
			LOGLB(LOG_DEBUG,"query if the channel is muxed failed");
			return -1;
		}
		if(muxif){
			info.state |= muxnum;
		}
		if (ioctl(dev_info->fd, CMDM_IOCTL_LOOPBACK_SET, &info)) {
			perror("CMDM_IOCTL_LOOPBACK_SET");
			return -1;
		}
	}
	return 0;
}

static int loopback_test_init(int channel)
{
	SerialDev *dev_info = &serial[channel];
	unsigned int i=0,j=0;
	unsigned int div = 0;
	unsigned int last = 0;
	char buf[1024];
	int ret = 0;
    int err=-1;
	int fd=0;
	char src_path[255];

	LOGLB(LOG_DEBUG, "ENTER");
/*try to open test file which is used for storing test data*/
	memset(dev_info,0,sizeof(SerialDev));
	dev_info->channel = channel;
    err = pthread_mutex_init (&dev_info->write_mutex, (const pthread_mutexattr_t *)NULL);
	if(err !=0){
		LOGLB(LOG_ERR, "write_mutex init failed");
	}
    err = pthread_mutex_init (&dev_info->read_mutex, (const pthread_mutexattr_t *)NULL);
	if(err !=0){
		LOGLB(LOG_ERR, "read_mutex init failed");
	}
	
    err = pthread_cond_init  (&dev_info->write_cond, (const pthread_condattr_t *)NULL);
	if(err !=0){
		LOGLB(LOG_ERR, "write_cond init failed");
	}
    err = pthread_cond_init  (&dev_info->read_cond, (const pthread_condattr_t *)NULL);
	if(err !=0){
		LOGLB(LOG_ERR, "read_cond init failed");
	}
	dev_info->devicename = viatelAdjustDevicePathFromProperty(channel);
	LOGLB(LOG_DEBUG, "%d dev_info->devicename=%s VIATEL_CHANNEL_MUX=%d",__LINE__,dev_info->devicename,VIATEL_CHANNEL_MUX);
	if(dev_info->devicename == NULL){
		LOGLB(LOG_ERR,"get channel logical channel name failed");
		ret = -1;
		goto terminate;
	}
	dev_info->test_count = 0;
	dev_info->err_count = 0;
	ret = loopback_open_device(dev_info);
	if(enable_cp_loopback_mode(dev_info,1) != 0){
		LOGLB(LOG_ERR,"enable cp loopback mode failed");
		ret = -1;
		goto terminate;
	}
	main_exit_signal[channel] = 1;
	if(create_thread(&thread_id_write[channel], write_data_thread,(void*)dev_info)!=0)
	{ //create thread for writing test data to desired channel
		LOGLB(LOG_ERR,"Could not create thread for write data thread");
		ret = -1;
		goto terminate;
  	}
 	if(create_thread(&thread_id_read[channel], data_poll_thread,(void*)&serial[channel])!=0)
	{ //create thread for reading data back  from desired channel,and checkout them
		LOGLB(LOG_ERR,"Could not create thread for listening");
		ret = -1;
		goto terminate;
  	}
	sleep(1);
	pthread_cond_signal(&dev_info->write_cond);
 terminate:
	close(fd);
	LOGLB(LOG_DEBUG, "LEAVE");
	return ret;
}


static int loopback_test_exit()
{
	int i=0;
	int ret =0;

	//unlink(LOOPBACK_TESTFILE_NAME);
	for(i=0;i<VIATEL_CHANNEL_NUM;i++){
		enable_cp_loopback_mode(&serial[i],0);
		loopback_close_devices(&serial[i]);
		pthread_join(thread_id_write[i], NULL);
		pthread_join(thread_id_read[i], NULL);
	}
	return 0;
}
/* 
* Purpose:  function to set global flag main_exit_signal
* Input:      signal - ID number of signal
* Return:    -
*/
static void set_main_exit_signal(int signal){
	//new lock
	int i=0;

	for(i=0;i<VIATEL_CHANNEL_NUM;i++)
		main_exit_signal[i] = signal;
}


static int usage(char *_name)
{
	fprintf(stderr, "\tUsage: %s [options]\n", _name);
	fprintf(stderr, "Options:\n");
	fprintf(stderr, "\t-v: verbose logging level: [%d]. 0(Silent) - 7(Debug)\n",syslog_level);
    fprintf(stderr, "\t-c: test channel: [%x]\n", channel);
    fprintf(stderr, "\t-s: one time test data size: [%d]\n", onetime_size[0]);
	fprintf(stderr, "\t-l: Log to file %s: [%s]\n", LOG_FILE_PATH, logToFile?"yes":"no");
	//help
	fprintf(stderr, "\t-h: Show this help message and show current settings.\n");
	return -1;
}

#ifdef RIL_SHLIB
/* 
* Purpose:  start one channel's loopback test mode
* Input:      channel :channel num
*                size:data size of oneshot testing
*			random:if use random data for testing
*				0: use random data
				1:use certain data
* Return:    0
*/
int loopback_start(int channel,int size,int random)
{
	int i=0;
	int ret=0;

	if(serial[channel]->fd !=-1){
		LOGLB(LOG_NOTICE,"start channel[%d]loopback mode[%d]/[%d]",channel,size,random);
		random_data= random;
		onetime_size[channel]= size;
		ret = loopback_test_init(i)
	}else{
		LOGLB(LOG_ERR,"start loopback mode failed");
		ret = -1;
	}

	return ret;
}

/* 
* Purpose:  start one channel's loopback test mode
* Input:      channel :channel num
* Return:    0:success
*		     other:failed
*/

int loopback_stop(int channel)
{
	int i=0;
	int ret=0;
	ret=enable_cp_loopback_mode(&serial[channel],0);
	if(ret !=0)
	{
		LOGLB(LOG_ERR,"stop loopback mode failed");
		ret = -1;
	}
	ret =loopback_close_devices(&serial[channel]);
	if(ret !=0)
	{
		LOGLB(LOG_ERR,"close loopback device failed");
		ret = -1;
	}
	random_data= 0;
	onetime_size[channel]= 0;
	pthread_join(thread_id_write[i], NULL);
	pthread_join(thread_id_read[i], NULL);

	return ret;
}


/* 
* Purpose:  start one channel's loopback test mode
* Input:      channel :				channel num
*                download_speed:		download speed
*			upload_speed:		upload speed
*			test_count:			test times count
			err_count :			error times count
* Return:    0
*/
int loopback_query(int channel,
						 float *download_speed,
						 float *upload_speed,
						 int *test_count,
						 int *err_count)
{
	*download_speed=serial[channel].download_speed;
	*upload_speed=serial[channel].upload_speed,
	*test_count=serial[channel].test_count;
	*err_count=serial[channel].err_count);

	LOGLB(LOG_NOTICE,"device[%s] v[%f]/^[%f] T[%d]/E[%d]\n",
		serial[channel].devicename,serial[channel].download_speed, serial[channel].upload_speed,
		serial[channel].test_count, serial[channel].err_count);
	return 0;
}

#else
/* 
* Purpose:  The main program loop
* Input:      argc - number of input arguments
*                argv - array of strings (input arguments)
* Return:    0
*/

int main(int argc,char *argv[])
{
	LOGLB(LOG_DEBUG, "Enter");

	int running_channel = 0;
	int opt,ret =0 ;
	int oneshot_size=0;
	int i=0;

	LOGLB(LOG_DEBUG, "%s %s starting", *argv, revision);
	while ((opt = getopt(argc, argv, "v:lrc:s:h?")) > 0)
	{
		switch (opt)
		{
		case 'v':
			syslog_level = atoi(optarg);
			if ((syslog_level>LOG_DEBUG) || (syslog_level < 0)){
				LOGLB(LOG_DEBUG, "%d",__LINE__);
			    usage(argv[0]);
			  exit(0);
			}
			break;
		case 'c':
			channel = atoi(optarg);
			break;
		case 's':
			oneshot_size=atoi(optarg);
			for(i=0;i<VIATEL_CHANNEL_NUM;i++)
				onetime_size[i]=oneshot_size;
			break;
	    case 'l':
		    logToFile = 1;
		    if ((logFile = fopen(LOG_FILE_PATH, "a+")) == NULL){
			    fprintf(stderr, "Error: %s.\n", strerror(errno));
			    exit(0);
			}else{
                fprintf(logFile, "\n\n################### loopback log %s\n", ctime(&tStart));
            }
		    break;
		case 'r':
			for(i=0;i<VIATEL_CHANNEL_NUM;i++)
				random_data[i]=1;
			break;
		default:
		case '?':
		case 'h':
			usage(argv[0]);
			exit(0);
			break;
		}
	}
	usage(argv[0]);
	signal(SIGKILL, signal_treatment);
	signal(SIGTERM, signal_treatment);
//Initialize modem and virtual ports
	for(i=0;i<(int)VIATEL_CHANNEL_NUM;i++){
		LOGLB(LOG_DEBUG, "%d",__LINE__);
		if((channel & (1<<i))  != 0){
			LOGLB(LOG_DEBUG, "%d i=%d",__LINE__,i);
			if(loopback_test_init(i) ==0){
				running_channel |= i;
			}
		}
	}
	while(/*(main_exit_signal==1) &&*/ (running_channel!=0))
	{
		LOGLB(LOG_DEBUG, "%d running_channel=%d",__LINE__,running_channel);
		for(i=0;i<VIATEL_CHANNEL_NUM;i++)
			if(serial[i].fd !=0){
				LOGLB(LOG_NOTICE,"device[%s] v[%f]/^[%f] T[%d]/E[%d]\n",
					serial[i].devicename,serial[i].download_speed, serial[i].upload_speed,
					serial[i].test_count, serial[i].err_count);
			}
		sleep(1);
	}


//finalize everything
	loopback_test_exit();
	LOGLB(LOG_DEBUG, "%d",__LINE__);
	if (logToFile != 0)
		fclose(logFile);	
	LOGLB(LOG_DEBUG, "LEAVE");
	return 0;
}
#endif

