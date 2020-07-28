#ifndef __FBCONFIG_H__
#define __FBCONFIG_H__
/* IOCTL commands. */
#define FBCONFIG_IOW(num, dtype)     _IOW('X', num, dtype)
#define FBCONFIG_IOR(num, dtype)     _IOR('X', num, dtype)
#define FBCONFIG_IOWR(num, dtype)    _IOWR('X', num, dtype)
#define FBCONFIG_IO(num)             _IO('X', num)

#define GET_DSI_ID     FBCONFIG_IOW(43, unsigned int)
#define SET_DSI_ID     FBCONFIG_IOW(44, unsigned int)
#define LCM_GET_ID     FBCONFIG_IOR(45, unsigned int)
#define LCM_GET_ESD    FBCONFIG_IOWR(46, unsigned int)
#define DRIVER_IC_CONFIG    FBCONFIG_IOR(47, unsigned int)
#define DRIVER_IC_CONFIG_DONE    FBCONFIG_IO(0)
#define DRIVER_IC_RESET    FBCONFIG_IOR(48, unsigned int)


#define MIPI_SET_CLK     FBCONFIG_IOW(51, unsigned int)
#define MIPI_SET_LANE    FBCONFIG_IOW(52, unsigned int)
#define MIPI_SET_TIMING  FBCONFIG_IOW(53, unsigned int)
#define MIPI_SET_VM      FBCONFIG_IOW(54, unsigned int) /* mipi video mode timing setting */
#define MIPI_SET_CC      FBCONFIG_IOW(55, unsigned int) /* mipi non-continuous clock */
#define MIPI_SET_SSC     FBCONFIG_IOW(56, unsigned int) /* spread frequency */
#define MIPI_SET_CLK_V2  FBCONFIG_IOW(57, unsigned int) /* For div1,div2,fbk_div case */


#define TE_SET_ENABLE  FBCONFIG_IOW(61, unsigned int)
#define FB_LAYER_DUMP  FBCONFIG_IOW(62, unsigned int)
#define FB_LAYER_GET_INFO FBCONFIG_IOW(63, unsigned int)
#define FB_LAYER_GET_EN FBCONFIG_IOW(64, unsigned int)
#define LCM_GET_ESD_RET    FBCONFIG_IOR(65, unsigned int)

#define LCM_GET_DSI_CONTINU    FBCONFIG_IOR(71, unsigned int)
#define LCM_GET_DSI_CLK    FBCONFIG_IOR(72, unsigned int)
#define LCM_GET_DSI_TIMING   FBCONFIG_IOR(73, unsigned int)
#define LCM_GET_DSI_LANE_NUM    FBCONFIG_IOR(74, unsigned int)
#define LCM_GET_DSI_TE    FBCONFIG_IOR(75, unsigned int)
#define LCM_GET_DSI_SSC    FBCONFIG_IOR(76, unsigned int)
#define LCM_GET_DSI_CLK_V2    FBCONFIG_IOR(77, unsigned int)
#define LCM_TEST_DSI_CLK    FBCONFIG_IOR(78, unsigned int)
#define FB_GET_MISC FBCONFIG_IOR(80, unsigned int)

#define MAX_INSTRUCTION 50
/* currently max 12 layer through all platform. maybe more in the future. */
#define MAX_LAYER_NUM 24

typedef enum {
        RECORD_CMD = 0,
        RECORD_MS = 1,
        RECORD_PIN_SET  = 2,
} RECORD_TYPE;

typedef enum {
        PM_DSI0 = 0,
        PM_DSI1 = 1,
        PM_DSI_DUAL = 2,
        PM_DSI_MAX = 0XFF,
} DSI_INDEX;

/* "TIMCON0_REG:" "HS_PRPR" "HS_ZERO" "HS_TRAIL\n" */
/*    "TIMCON1_REG:" "TA_GO" "TA_SURE" "TA_GET" "DA_HS_EXIT\n" */
/*    "TIMCON2_REG:" "CLK_ZERO" "CLK_TRAIL" "CONT_DET\n" */
/*    "TIMCON3_REG:" "CLK_HS_PRPR" "CLK_HS_POST" "CLK_HS_EXIT\n" */
/* "VDO MODE :" "HPW" "HFP" "HBP" "VPW" "VFP" "VBP" */

typedef enum {
        HS_PRPR = 0,
        HS_ZERO = 1,
        HS_TRAIL = 2,
        TA_GO = 3,
        TA_SURE = 4,
        TA_GET = 5,
        DA_HS_EXIT = 6,
        CLK_ZERO = 7,
        CLK_TRAIL = 8,
        CONT_DET = 9,
        CLK_HS_PRPR = 10,
        CLK_HS_POST = 11,
        CLK_HS_EXIT = 12,
        HPW = 13,
        HFP = 14,
        HBP = 15,
        VPW = 16,
        VFP = 17,
        VBP = 18,
        LPX = 19,
        SSC_EN = 0xFE,
        MAX = 0XFF,
} MIPI_SETTING_TYPE;

typedef struct CONFIG_RECORD {
        RECORD_TYPE type;  /* msleep;cmd;resetpin. */
        int ins_num;
        int ins_array[MAX_INSTRUCTION];
} CONFIG_RECORD;

typedef struct MIPI_TIMING {
        MIPI_SETTING_TYPE type;
        unsigned int value;
} MIPI_TIMING;

typedef struct SETTING_VALUE {
        DSI_INDEX dsi_index;
        unsigned int value;
} SETTING_VALUE;

typedef struct PM_LAYER_EN {
        int layer_en[MAX_LAYER_NUM];
} PM_LAYER_EN;

typedef struct ESD_PARA {
        int addr;
        int type;
        int para_num;
        char * esd_ret_buffer;
} ESD_PARA;

typedef struct {
        unsigned short type;  /* must be BMP here */
        unsigned int fsize;  /* raw data size +54; */
        unsigned short res1;  /* 2bytes */
        unsigned short res2;  /* 2bytes */
        unsigned int offset;  /* should be 54bytes */
        /* below 40bytes is for BMF info header */
        unsigned int this_struct_size;  /* the size of BITMAP INFO Header should be 40 ==0x28 */
        unsigned int width;  /* the bmp  width */
        unsigned int height; /* the bmp height */
        unsigned short planes; /* must be 0x01; */
        unsigned short bpp; /* bits per piexl; */
        unsigned int compression; /* should be 0 ,no compression */
        unsigned int raw_size; /* raw data size; */
        unsigned int x_per_meter; /* pilexs per meter in x direction; */
        unsigned int y_per_meter; /* pilexs per meter in y direction; */
        unsigned int color_used; /* 0 here */
        unsigned int color_important; /* importance; */
} __attribute__((packed,aligned(2))) BMF_HEADER;

typedef struct PM_LAYER_INFO {
        unsigned int index;
        unsigned int height;
        unsigned int width;
        unsigned int fmt;
        unsigned int layer_size;
} PM_LAYER_INFO;

typedef struct MIPI_CLK_V2 {
        unsigned char div1;
        unsigned char div2;
        unsigned short fbk_div;
} MIPI_CLK_V2;

typedef struct LCM_TYPE_FB{
        unsigned int clock;
        unsigned int lcm_type;
} LCM_TYPE_FB;

struct misc_property {
        unsigned int dual_port : 1;
        unsigned int overall_layer_num : 5;
        unsigned int reserved : 26;
};

#endif /* __FBCONFIG_H__ */

