#ifndef __MTEE_TIME_H
#define __MTEE_TIME_H

#include <stdio.h>
#include <trusty_std.h>
#include <tz_private/system.h>
#include <lib/mtee/mtee_sys.h>
#include <errno.h>

__BEGIN_CDECLS

#define TM_YEAR_BASE 1900


typedef int time_t;
typedef int clock_t;

struct tm {
	int tm_sec;         /* seconds */
	int tm_min;         /* minutes */
	int tm_hour;        /* hours */
	int tm_mday;        /* day of the month */
	int tm_mon;         /* month */
	int tm_year;        /* year */
	int tm_wday;        /* day of the week */
	int tm_yday;        /* day in the year */
	int tm_isdst;       /* daylight saving time */

	long int tm_gmtoff;     /* Seconds east of UTC.  */
	const char *tm_zone;    /* Timezone abbreviation.  */
};

time_t time (time_t* timer);
clock_t clock (void);
double difftime (time_t end, time_t beginning);
time_t mktime (struct tm * timeptr);

char* asctime (const struct tm * timeptr);
char* ctime (const time_t * timer);
struct tm * gmtime (const time_t * timer);
struct tm * localtime (const time_t * timer);
size_t strftime (char* ptr, size_t maxsize, const char* format, const struct tm* timeptr );
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Return 1 if YEAR + TM_YEAR_BASE is a leap year.  */
static int leapyear (int year)
{
	return
    ( (year & 3) == 0 && (year % 100) != 0 || ( (year % 400) == 0 && (year % 4000) != 0 ) );
}


static const unsigned short int __mon_yday[2][13] =
{
    /* Normal years.  */
    { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
    /* Leap years.  */
    { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};

static const unsigned short int __year_yday[2] = {365, 366};

inline time_t time (time_t* timer)
{
	struct rtc_handle rtc_fd;
	struct rtc_time rtc_tm;
	struct tm *time_tm;
	
	MTEE_rtc_open(&rtc_fd);
	MTEE_rtc_read(&rtc_fd, &rtc_tm);
	MTEE_rtc_close(&rtc_fd);

	int sec = rtc_tm.tm_sec;
	int min = rtc_tm.tm_min;
	int hour = rtc_tm.tm_hour;
	int mday = rtc_tm.tm_mday;
	int mon = rtc_tm.tm_mon;
	int year = rtc_tm.tm_year;

	int thisyearday = 0;
	int total_day = 0;
	time_t return_sec = 0;
	
	thisyearday = __mon_yday[leapyear(year)][mon-1] + mday-1;
	
	for(int i = year-1;i >= 1970; i--)
		total_day = total_day + __year_yday[leapyear(i)];
		
	total_day = total_day + thisyearday;
	
	
	return_sec = (((total_day)*24+hour)*60+min)*60+sec;
	
	return return_sec;

}

inline clock_t clock (void)
{
	
	return MTEE_GetSystemCnt();
	//printf("%s not support!!!\n", __func__); return -1;
}

inline double difftime (time_t end, time_t beginning)
{
	if(end > beginning && end > 0 && beginning >0)
	return end-beginning;
	else return 0;
	//printf(const char * format,...)("%s not support!!!\n", __func__); return -1;
}

inline time_t mktime (struct tm * timeptr)
{
	
	int sec = timeptr->tm_sec;
	int min = timeptr->tm_min;
	int hour = timeptr->tm_hour;
	int mday = timeptr->tm_mday;
	int mon = timeptr->tm_mon;
	int year_requested = timeptr->tm_year+1900;

	int thisyearday = 0;
	int total_day = 0;
	time_t return_sec = 0;

	thisyearday = __mon_yday[leapyear(year_requested)][mon] + mday-1;

	for(int i = year_requested-1;i >= 1970; i--)
		total_day = total_day + __year_yday[leapyear(i)];
	
	total_day = total_day + thisyearday;


	return_sec = (((total_day)*24+hour)*60+min)*60+sec;


	return return_sec;
}

inline char* asctime (const struct tm * timeptr)
{
	static const char wday_name[][4] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
  	};

  	static const char mon_name[][4] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
  	};
  	static char result[26];
	const char *	wn;
	const char *	mn;

	if (timeptr == NULL) {
		errno = EINVAL;
		return strcpy(result, "??? ??? ?? ??:??:?? ????\n");
	}

	if (timeptr->tm_wday < 0 || timeptr->tm_wday >= 7)
		wn = "???";
	else	wn = wday_name[timeptr->tm_wday];
	if (timeptr->tm_mon < 0 || timeptr->tm_mon >= 12)
		mn = "???";
	else	mn = mon_name[timeptr->tm_mon];

	
	sprintf(result, "%.3s %.3s%3d %02d:%02d:%02d %d\n",
    				wn,
   					mn,
    				timeptr->tm_mday, timeptr->tm_hour,
    				timeptr->tm_min, timeptr->tm_sec,
    				1900 + timeptr->tm_year);
  	return result;
	
//printf("%s not support!!!\n", __func__); return NULL;
}

inline char* ctime (const time_t * timer)
{printf("%s not support!!!\n", __func__); return NULL;}

inline struct tm * gmtime (const time_t * timer)
{printf("%s not support!!!\n", __func__); return NULL;}

inline struct tm * localtime (const time_t * timer)
{printf("%s not support!!!\n", __func__); return NULL;}

inline size_t strftime (char* ptr, size_t maxsize, const char* format, const struct tm* timeptr )
{printf("%s not support!!!\n", __func__); return -1;}
__END_CDECLS
#endif //__TIME_H
