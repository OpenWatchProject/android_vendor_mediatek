#ifndef __AMMS_H__
#define __AMMS_H__



/*DRDI only need 1 MPU
 * to support DRDI and POS concurrently
 * we need to support 2
 */
#define AMMS_SUPPORT_MPU_NUM 2
#define AMMS_DYNAMIC_REGION_DRDI 8
/*DRDI will use REGION_ID1
 * for POS it will use REGION_ID2
 */
#define AMMS_DYNAMIC_REGION_POS 9


#define AMMS_PENDING_DRDI_FREE_BIT (1<<0)
#define AMMS_PENDING_POS_DEALLOC_BIT (1<<1)
#define AMMS_PENDING_POS_ALLOC_BIT (1<<2)


enum AMMS_POS_OWN_STATUS {
	AMMS_POS_OWN_BY_MD,
	AMMS_POS_OWN_BY_AP,
	AMMS_POS_OWN_BY_AP_PROCESS_TO_MD,
	AMMS_POS_OWN_BY_MD_PROCESS_TO_AP,
};

enum AMMS_MD_POS_OWNER {
	AMMS_POS_OWNER_AP,
	AMMS_POS_OWNER_MD,
};

enum AMMS_VERSION_CAP {
	AMMS_VERSION_CAP_DRDI = 1<<0,
	AMMS_VERSION_CAP_POS = 1<<1,
};

enum AMMS_MD_PER {
	R_ONLY,
	R_WRITE,
	R_DISABLE,
	AMMS_MD_PER_LAST,
};

enum AMMS_MEM_OWNER_STATUS {
	AMMS_MEM_OWNER_AP = -1,
	AMMS_MEM_OWNER_MD = 0,
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
	/*0:for DRDI*/
	AMMS_EMI_MPU_DRDI = 0,
	/*1:for POS*/
	AMMS_EMI_MPU_POS = 1,
};


enum AMMS_CMD {
		AMMS_CMD_INIT = 1,
		AMMS_CMD_QUERY_DRDI_STATUS,
		AMMS_CMD_MPU,
		AMMS_CMD_STATIC_FREE,
		AMMS_CMD_DYNAMIC_FREE,
		AMMS_CMD_DYNAMIC_RETRIEVE,
		AMMS_CMD_DEALLOCATE_POS_BUFFER,
		AMMS_CMD_ALLOCATE_POS_BUFFER,
		AMMS_CMD_QUERY_POS_STATUS,
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

struct __attribute__((__packed__)) amms_deallocate_pos_buf_cmd {
	struct amms_address_range range;
};

struct __attribute__((__packed__)) amms_allocate_pos_buf_cmd {
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
	struct amms_address_range range;
};

struct __attribute__((__packed__)) amms_msg_request {
	unsigned char cmd;
	unsigned char seq_id;/*md control and +1 by every request*/
	unsigned char padding[2];
	union {
		struct amms_init_request_cmd init_request_cmd;
		struct amms_free_static_cmd free_static_cmd;
		struct amms_deallocate_pos_buf_cmd deallocate_pos_buf_cmd;
		struct amms_allocate_pos_buf_cmd allocate_pos_buf_cmd;
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
unsigned long long sip_amms_set_md_pos_addr(unsigned long long md_pos_addr);
unsigned long long sip_amms_set_md_pos_length(unsigned long long md_pos_length);
unsigned long long sip_amms_get_md_pos_addr(void);
unsigned long long sip_amms_get_md_pos_length(void);
unsigned int       sip_amms_get_pending(void);
void sip_amms_clear_pending(unsigned int pending);
unsigned long long sip_amms_get_md_view_pos_addr(void);
unsigned long long sip_amms_set_md_view_pos_addr(
		unsigned long long md_pos_addr);
unsigned char sip_amms_get_seq_id(void);


#endif
