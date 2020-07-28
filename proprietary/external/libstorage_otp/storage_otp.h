#ifndef __STORAGE_OTP_H__
#define __STORAGE_OTP_H__

#include <limits.h>
#include <log/log.h>
//#include <utils/Log.h>


/* common define */
enum user_id {
	USER_CCCI = 0,
	USER_UNDEFINED1,
	USER_UNDEFINED2,
	USER_UNDEFINED3,
	MAX_USER_NUM
};

#undef LOG_TAG
#define LOG_TAG "STORAGE_OTP"

#define OTP_DEBUG_LOG

#ifdef OTP_DEBUG_LOG
#define OTP_LOG(...) \
    do { \
	ALOGD(__VA_ARGS__); \
    } while (0)
#else
#define OTP_LOG(...)
#endif


typedef enum {
	WP_TEMPORARY = 0,
	WP_POWER_ON,
	WP_PERMANENT,
	WP_DISABLE,
} WP_TYPE;

struct otp_region {
	unsigned int user_id;
	unsigned long long start;
	unsigned long long size;
};
struct otp;

struct otp {
	unsigned long long total_size; /* total size of storage */
	char otp_blk_path[PATH_MAX];
	unsigned long long blk_sz;
	unsigned long long wp_grp_size;
	unsigned long long otp_start; /* otp partition start address(alignment) */
	unsigned long long otp_size;  /* otp partition size */
	unsigned long long partition_start; /* otp partition real start */
	unsigned int current_user_id; /* current user id */
	struct otp_region region[MAX_USER_NUM]; /* otp region */
	unsigned int max_user_num;
	unsigned int block_sz;
	int (*block_read)(struct otp *otp_device, char *buffer,
			unsigned long long start_block, unsigned long long block_number);
	int (*block_write)(struct otp *otp_device, char *buffer,
			unsigned long long start_block, unsigned long long block_number);
	int (*read)(struct otp *otp_device, char *buffer,
			unsigned long long start, unsigned long long size);
	unsigned int (*write)(struct otp *otp_device, char *buffer,
			unsigned long long start, unsigned long long size);
	int (*lock)(struct otp *otp_device, unsigned int type, unsigned long long start_block,
			unsigned long long group_count, unsigned long long wp_grp_size);
	int (*unlock)(struct otp *otp_device, unsigned long long start_block,
			unsigned long long block_number, unsigned long long wp_grp_size);
	int (*get_status)(struct otp *otp_device, unsigned long long start_block,
			unsigned long long group_count, unsigned int *status, unsigned int *type);
};

struct otp *otp_open(unsigned int user_id);
void otp_close(struct otp *otp_device);
int otp_read(struct otp *otp_device, char *buffer, unsigned long long offset, unsigned long long size);
unsigned int otp_write(struct otp *otp_device, char *buffer, unsigned long long offset, unsigned long long size);
int otp_lock(struct otp *otp_device, unsigned int type);
int otp_unlock(struct otp *otp_device);
int otp_get_status(struct otp *otp_device, unsigned int *status, unsigned int *type);
unsigned long long otp_get_size(struct otp *otp_device);
unsigned int otp_get_libversion(void);

#include "emmc.h"
#include "ufs.h"

#endif
