#ifndef __MTEE_SYS_H
#define __MTEE_SYS_H
#include <sys/types.h>
#include <tz_private/mtee_sys_fd.h>
__BEGIN_CDECLS


/* Command for system */
enum MTEE_SYS_CMD {
	MTEE_SYS_GET_VERSION = 0x1,
	MTEE_RTC_RD_TIME,
	MTEE_RTC_SET_TIME,
	MTEE_CELLINFO_GET_BUF,
	MTEE_UART_SET,
	MTEE_SYS_GET_HWUID,
	MTEE_DUMP_APP_MMU, // debug only!!!!
	MTEE_DUMP_KERNEL_HEAP,
	MTEE_DUMP_RAM_USAGE,
	MTEE_ACQUIRE_MTEE_TEE_STATIC_SHM,
	MTEE_RELEASE_MTEE_TEE_STATIC_SHM,
	MTEE_DUMP_MTEE_TEE_STATIC_SHM,
};

#define MTEE_VERSION_NUM 256 // max version size in char

typedef struct mtee_info {
	char version[MTEE_VERSION_NUM];
} mtee_info_t;


/* 	MTEE_GetVersion: get MTEE version
*  	@param info 	a pointer to mtee info parameters
*	@param return 	return lk_version
*/
TZ_RESULT MTEE_GetVersion(mtee_info_t *info);

/*  MTEE_GetUUID: get hardware UUID
*   @param info     a pointer to 16 byte char
*   @param return   return uuid
*/
TZ_RESULT MTEE_GetUUID(uint8_t *uuid);

TZ_RESULT MTEE_SetUart(int control);

struct rtc_handle {
	int fd;
};


struct rtc_time {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
};


/* 	MTEE_rtc_open: get MTEE SYS FD
*  	@param h 		a pointer to rtc handle
*	@param return 	return code
*/
TZ_RESULT MTEE_rtc_open(struct rtc_handle *h);
TZ_RESULT MTEE_rtc_close(struct rtc_handle *handle);

/* 	MTEE_rtc_write: set rtc time
*  	@param h 		a pointer to rtc handle
*  	@param tv 		a pointer to rtc time parameters
*	@param return 	return code
*/
TZ_RESULT MTEE_rtc_write(struct rtc_handle *handle, struct rtc_time *tv);

/* 	MTEE_rtc_read: 	read rtc time
*  	@param h 		a pointer to rtc handle
*  	@param tv 		a pointer to rtc time parameters
*	@param return 	return code
*/
TZ_RESULT MTEE_rtc_read(struct rtc_handle *handle, struct rtc_time *tv);


typedef struct cellinfo_buffer {
	uint64_t chunk_handle;
	uint64_t mem_handle;
	int size;
} cellinfo_buffer_t;


TZ_RESULT MTEE_GetCellInfoBuf(cellinfo_buffer_t *buf_handle);

void MTEE_DumpAppHeap(void);

/* dump gz kernel heap*/
TZ_RESULT MTEE_DumpKernelHeap(void);

/* dump gz ram usage */
TZ_RESULT MTEE_DumpRamUsage(void);

/* TEE MTEE shm */
int MTEE_AcquireMteeTeeStaticShm(void **va, u32 *size, const char *name);
int MTEE_ReleaseMteeTeeStaticShm(void **va, u32 *size, const char *name);
int MTEE_DumpMteeTeeStaticShm(void);

__END_CDECLS

#endif

