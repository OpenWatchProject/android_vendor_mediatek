#ifndef __SOUNGTRIGGER_H__
#define __SOUNGTRIGGER_H__

//#include <linux/compat.h>
//#include <linux/fs.h>
//#include <linux/uaccess.h>
#include <linux/vow.h>

/***********************************************************************************
** Phase2.5 definition
************************************************************************************/
//#define VOW_DATA_READ_PCMFILE
//#define VOW_RECOG_PCMFILE
#define MTK_VOW_ENABLE_CPU_BOOST
#define HAL_SOUND_TRIGGER_VER "1.0.5"

enum {
    RECOG_CONTINUOUS = 1,
    RECOG_PASS,
    RECOG_FAIL,
    RECOG_BAD
};

enum {
    VENDOR_ID_MTK = 77,     //'M'
    VENDOR_ID_AMAZON = 65,  //'A'
    VENDOR_ID_OTHERS = 71,
    VENDOR_ID_NONE = 0
};

#endif //__SOUNGTRIGGER_H__
