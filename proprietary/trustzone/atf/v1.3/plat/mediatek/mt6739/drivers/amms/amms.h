#ifndef __AMMS_H__
#define __AMMS_H__

#define AMMS_VERSION 1
#define AMMS_SUPPORT_MPU_NUM 1
#define AMMS_DYNAMIC_REGION_ID1 6


enum AMMS_MD_PER {
	R_ONLY,
	R_WRITE,
	R_DISABLE,
	AMMS_MD_PER_LAST,
};


enum AMMS_STATUS {
	AMMS_STATUS_SUCCESS = 0,
	AMMS_STATUS_FAIL = -1,
	AMMS_STATUS_INVALID = -2,
};


enum AMMS_DEBUG {
	AMMS_DEBUG_DISABLE = 0,
	AMMS_DEBUG_ENABLE = 1,
};


enum AMMS_EMI_MPU {
	AMMS_EMI_MPU_0 = 0,
};


enum AMMS_CMD {
		AMMS_CMD_INIT = 1,
		AMMS_CMD_QUERY_DRDI_STATUS,
		AMMS_CMD_MPU,
		AMMS_CMD_STATIC_FREE,
		AMMS_CMD_DYNAMIC_FREE,
		AMMS_CMD_DYNAMIC_RETRIEVE,
};


struct __attribute__((__packed__)) amms_init_request_cmd {
	unsigned char version;/*md amms version info start from 1*/
};

struct __attribute__((__packed__)) amms_address_range {
	unsigned int start_address;
	unsigned int length;
};

struct __attribute__((__packed__)) amms_free_static_cmd {
	struct amms_address_range range;
};

struct __attribute__((__packed__)) amms_mpu_cmd {
	unsigned char per;
	unsigned char mpu_id;
	unsigned char padding[2];
	struct amms_address_range range;
};

struct __attribute__((__packed__)) init_cmd_reply {
	unsigned char version; /* ap amms version start from 1*/
	unsigned char debug;/*enable debug or not in MD side*/
	unsigned char support_mpu_num;/*at least support 1,could be two or more*/
	unsigned char padding;
};

struct __attribute__((__packed__)) amms_msg_request {
	unsigned char cmd;
	unsigned char seq_id;/*md control and +1 by every request*/
	unsigned char padding[2];
	union {
		struct amms_init_request_cmd init_request_cmd;
		struct amms_free_static_cmd free_static_cmd;
		struct amms_mpu_cmd mpu_cmd;
	} u_req;
};

struct __attribute__((__packed__)) amms_msg_response{
	char status;
	char error; /* reserved for debugging*/
	unsigned char seq_id; /*the same with the one we received)*/
	unsigned char padding;
	union {
		struct init_cmd_reply init_reply;
	} u_resp;
};


int handle_amms_request(struct amms_msg_request *request);
unsigned long long sip_amms_set_md_base_address(unsigned long long base_addr);
unsigned long long sip_amms_get_md_base_address(void);
unsigned long long sip_kernel_amms_get_free_addr(void);
unsigned long long sip_kernel_amms_get_free_length(void);


#endif
