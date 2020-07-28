//MTK_SWIP_PROJECT_START
#ifndef _MTK_TSF_TYPE_H
#define _MTK_TSF_TYPE_H

#include <stdint.h>

#define MTKTSFTYPE_REVISION 6161001

#define SHADING_VER 5 // 2, 3, 4, 5

#if SHADING_VER == 4 || SHADING_VER == 5
#define __AWB_STAT_2B__
#endif

#ifdef __AWB_STAT_2B__
#define TSFSTAT MUINT16
#else
#define TSFSTAT MUINT8
#endif

#ifndef    true
#define    true    (bool) 1
#endif

#ifndef    false
#define    false    (bool) 0
#endif

#ifndef    NULL
#define    NULL    0
#endif

typedef unsigned int        MUINT32;
typedef unsigned short      MUINT16;
typedef unsigned char       MUINT8;

typedef signed int          MINT32;
typedef signed short        MINT16;
typedef signed char         MINT8;

typedef int                 MBOOL;
typedef float               MFLOAT;

typedef uintptr_t           MUINTPTR;
typedef intptr_t            MINTPTR;
#endif
//MTK_SWIP_PROJECT_END