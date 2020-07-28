/***********************************************************************
*   This software/firmware and related documentation ("MediaTek Software")
*   are protected under relevant copyright laws. The information contained
*   herein is confidential and proprietary to MediaTek Inc. and/or its licensors.
*
*   Without the prior written permission of MediaTek Inc. and/or its licensors,
*   any reproduction, modification, use or disclosure of MediaTek Software, and
*   information contained herein, in whole or in part, shall be strictly prohibited.
*
*   MediaTek Inc. (C) [2008]. All rights reserved.
*
*************************************************************************/
//*****************************************************************************
// [File] MTK_Type.h
// [Version] v1.0
// [Revision Date] 2008-03-31
// [Author] YC Chien, yc.chien@mediatek.com, 21558
// [Description]
//*****************************************************************************

#ifndef MTK_TYPE_H
#define MTK_TYPE_H


typedef unsigned char           MTK_UINT8;
typedef signed char             MTK_INT8;

typedef unsigned short int      MTK_UINT16;
typedef signed short int        MTK_INT16;

typedef unsigned int            MTK_UINT32;
typedef signed int              MTK_INT32;

typedef unsigned long           MTK_UINT64;
typedef signed long             MTK_INT64;


typedef enum
{
    MTK_FALSE = 0,
    MTK_TRUE = 1
}   MTK_BOOL;

typedef unsigned int MTK_FILE;

typedef enum
{
    MTK_FS_READ = 0,     // open file for reading (r)
    MTK_FS_WRITE,        // create file for writing, discard previous contents if any (w)
    MTK_FS_APPEND,       // open or create file for writing at end of file (a)
    MTK_FS_RW,           // open file for reading and writing (r+)
    MTK_FS_RW_DISCARD,   // create file for reading and writing, discard previous contents if any (w+)
    MTK_FS_RW_APPEND     // open or create file for reading and writing at end of file (a+)
}   MTK_FMODE;

typedef enum
{
    MTK_FS_SEEK_SET = 0, // seek from beginning of file
    MTK_FS_SEEK_CUR,     // seek from current position
    MTK_FS_SEEK_END      // seek from end of file
}   MTK_FSEEK;

typedef struct _MTK_TIME
{
    MTK_UINT16  Year;
    MTK_UINT16  Month;
    MTK_UINT16  Day;
    MTK_UINT16  Hour;
    MTK_UINT16  Min;
    MTK_UINT16  Sec;
    MTK_UINT16  Msec;
}   MTK_TIME;

typedef enum
{
  HSDBGT_INIT = 0,
  HSDBGT_LIB,
  HSDBGT_KER,
  HSDBGT_CAL,
  HSDBGT_SYS,
  HSDBGT_COMM,
  HSDBGT_MSG,
  HSDBGT_STR,
  HSDBGT_MEM,
  HSDBGT_ALL,
  HSDBGT_END
} MTK_DEBUG_TYPE;

typedef enum
{
  HSDBGL_NONE = 0,
  HSDBGL_ERR,
  HSDBGL_WRN,
  HSDBGL_INFO1,
  HSDBGL_END
} MTK_DEBUG_LEVEL;

#define MTK_BEE_VOID void
#define MTK_BEE_INT int

#endif /* MTK_TYPE_H */
