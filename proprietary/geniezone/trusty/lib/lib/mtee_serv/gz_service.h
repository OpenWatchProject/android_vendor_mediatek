#define GZ_MAX_DATA_LEN 256

// system service command
#define GZ_CMD_OPEN_SESSION 0

// system service message format
typedef unsigned int GZ_SESSION_HANDLE;

#define GZ_MEM_MAX_LEN 256
#define GZ_MSG_DATA_MAX_LEN 1024 // 256 * 4

// system service message format
typedef struct _gz_sys_msg {
    unsigned int command;      // command to access gz system service
    unsigned int session;
    int data_type;
    int data_size;
    char data[GZ_MSG_DATA_MAX_LEN+2]; // last 2 bytes is '\0', fix me!!!
} gz_sys_msg_t;

typedef struct _GZ_servicecall_cmd_param {
    int handle;
    int command;
    int ree_service;
    int payload_size;
    int paramTypes;
    int dummy[4];
    MTEEC_PARAM param[4];
    char data[GZ_MSG_DATA_MAX_LEN];
} GZ_servicecall_cmd_param_t;


