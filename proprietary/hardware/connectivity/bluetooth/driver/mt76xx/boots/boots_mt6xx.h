#ifndef __BOOTS_MT6XX_H__
#define __BOOTS_MT6XX_H__

#include <stdbool.h>
/**
  * Type definition
  */
#ifndef FALSE
#define FALSE     0
#endif
#ifndef TRUE
#define TRUE      1
#endif
#ifndef BOOL
#define BOOL      bool
#endif

typedef unsigned char UCHAR;
typedef unsigned char UINT8;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
typedef char INT8;
typedef short INT16;
typedef int INT32;
typedef long long INT64;
typedef void VOID;
typedef unsigned char* PUCHAR;


/*
  * Structure Definitions
  */
#define HCI_CMD_PARM_LEN   256
#define MAX_EVENT_SIZE     256

typedef struct _HCI_CMD_T {
  UINT16 opCode;  /* HCI command OpCode */
  UINT8  len;     /* Length of the command parameters */
  UINT8  parms[HCI_CMD_PARM_LEN];
} HCI_CMD_T;

typedef struct _HCI_EVENT_T {
  UINT8 event;    /* HCI event type */
  UINT8 len;      /* Length of the event parameters */
  UINT8 *parms;   /* Event specific parameters */
} HCI_EVENT_T;


typedef enum _ENUM_BT_STATUS_T {
  BT_STATUS_SUCCESS = 0,
  BT_STATUS_FAILED,
  BT_STATUS_PENDING,
  BT_STATUS_BUSY,
  BT_STATUS_NO_RESOURCES,
  BT_STATUS_NOT_FOUND,
  BT_STATUS_DEVICE_NOT_FOUND,
  BT_STATUS_CONNECTION_FAILED,
  BT_STATUS_TIMEOUT,
  BT_STATUS_NO_CONNECTION,
  BT_STATUS_INVALID_PARM,
  BT_STATUS_IN_PROGRESS,
  BT_STATUS_RESTRICTED,
  BT_STATUS_INVALID_TYPE,
  BT_STATUS_HCI_INIT_ERR,
  BT_STATUS_NOT_SUPPORTED,
  BT_STATUS_IN_USE,
  BT_STATUS_SDP_CONT_STATE,
  BT_STATUS_CANCELLED,
  BT_STATUS_NOSERVICES,
  BT_STATUS_SCO_REJECT,
  BT_STATUS_CHIP_REASON,
  BT_STATUS_BLOCK_LIST,
  BT_STATUS_SCATTERNET_REJECT
} ENUM_BT_STATUS_T;

/* The record structure of bt nvram file */
typedef struct
{
    unsigned char addr[6];            /* BT address */
    unsigned char Voice[2];           /* Voice setting for SCO connection */
    unsigned char Codec[4];           /* PCM codec setting */
    unsigned char Radio[6];           /* RF configuration */
    unsigned char Sleep[7];           /* Sleep mode configuration */
    unsigned char BtFTR[2];           /* Other feature setting */
    unsigned char TxPWOffset[3];      /* TX power channel offset compensation */
    unsigned char CoexAdjust[6];      /* BT/WIFI coexistence performance adjustment */
    unsigned char Radio_ext[2];       /* RF configuration extended parameters */
    unsigned char TxPWOffset_ext[4];  /* Tx power channel offset compensation with new range */
    unsigned char Reserved1[2];       /* Reserved */
    unsigned char Reserved2[4];       /* Reserved */
    unsigned char Reserved3[8];       /* Reserved */
    unsigned char Reserved4[8];       /* Reserved */
}__attribute__((packed)) ap_nvram_btradio_struct;


typedef union {
  ap_nvram_btradio_struct fields;
  unsigned char raw[sizeof(ap_nvram_btradio_struct)];
} BT_NVRAM_DATA_T;

typedef ENUM_BT_STATUS_T (*HCI_CMD_FUNC_T)(VOID);
typedef struct {
  HCI_CMD_FUNC_T command_func;
} HCI_SEQ_T;

typedef struct {
  UINT32 chip_id;
  BT_NVRAM_DATA_T bt_nvram;
  HCI_SEQ_T *cur_script;
} BT_INIT_VAR_T;

/*
* Function Declaration
*/
#endif // __BOOTS_MT6XX_H__
