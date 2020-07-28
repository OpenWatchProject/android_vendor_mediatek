/*
 * Define sensor overlay object here
 */


#define OVERLAY_SECTION_TEXT (.text* .data* .rodata* .bss*)
#define OVERLAY_ONE_OBJECT(tag, file) .tag { *file.o OVERLAY_SECTION_TEXT }
#define OVERLAY_LIB_OBJECT(tag, lib, file) .tag { lib.a: file.o OVERLAY_SECTION_TEXT }
#define OVERLAY_TWO_OBJECT(tag, lib, file1, file2) \
        .tag { lib.a: file1.o OVERLAY_SECTION_TEXT } \
        .tag { lib.a: file2.o OVERLAY_SECTION_TEXT }
#define OVERLAY_THREE_OBJECT(tag, file1, file2, file3) \
        .tag { *file1.o OVERLAY_SECTION_TEXT } \
        .tag { *file2.o OVERLAY_SECTION_TEXT } \
        .tag { *file3.o OVERLAY_SECTION_TEXT }

#define OVERLAY_SEVEN_OBJECT(tag, file1, file2, file3, file4, file5, file6, file7) \
        .tag { *file1.o OVERLAY_SECTION_TEXT } \
        .tag { *file2.o OVERLAY_SECTION_TEXT } \
        .tag { *file3.o OVERLAY_SECTION_TEXT } \
        .tag { *file4.o OVERLAY_SECTION_TEXT } \
        .tag { *file5.o OVERLAY_SECTION_TEXT } \
        .tag { *file6.o OVERLAY_SECTION_TEXT } \
        .tag { *file7.o OVERLAY_SECTION_TEXT }


/*****************************************************************************
* Overlay0: ACCGYRO
*****************************************************************************/
#define OVERLAY_SECTION_ACCGYRO                    \
    OVERLAY_ONE_OBJECT(lsm6dsm, lsm6dsm)           \
    OVERLAY_ONE_OBJECT(lsm6ds3, lsm6ds3)           \
    OVERLAY_ONE_OBJECT(bmi160, bmi160)             \
    OVERLAY_ONE_OBJECT(lis3dh, lis3dh)             \
    OVERLAY_ONE_OBJECT(lis2hh12, lis2hh12)

#ifdef OVERLAY_SECTION_ACCGYRO
#define OVERLAY0 OVERLAY_SECTION_ACCGYRO
#else
#define OVERLAY0
#endif  // OVERLAY_SECTION_ACCGYRO


/*****************************************************************************
* Overlay1: MAG
*****************************************************************************/
#define OVERLAY_SECTION_MAG                        \
    OVERLAY_SEVEN_OBJECT(akm09915, akm09915, akm09918, akl_apis, measure, akm_wrapper, akm_apis, akl_nonos_ext)\
    OVERLAY_LIB_OBJECT(akm09915, libakm, AK*)      \
    OVERLAY_SEVEN_OBJECT(akm09918, akm09915, akm09918, akl_apis, measure, akm_wrapper, akm_apis, akl_nonos_ext)\
    OVERLAY_LIB_OBJECT(akm09918, libakm, AK*)      \
    OVERLAY_THREE_OBJECT(mmc3530, mmc3530, MemsicConfig, memsic_wrapper) \
    OVERLAY_TWO_OBJECT(mmc3530, libMemsicAlgo, Memsic*, mymath*)

#ifdef OVERLAY_SECTION_MAG
#define OVERLAY1 OVERLAY_SECTION_MAG
#else
#define OVERLAY1
#endif  // OVERLAY_SECTION_MAG


/*****************************************************************************
* Overlay2: ALSPS
*****************************************************************************/
#define OVERLAY_SECTION_ALSPS                      \
    OVERLAY_ONE_OBJECT(cm36558, cm36558)           \
    OVERLAY_ONE_OBJECT(tmd2725, tmd2725)           \
    OVERLAY_ONE_OBJECT(apds9922, apds9922)         \
    OVERLAY_ONE_OBJECT(stk3x3x, stk3x3x)

#ifdef OVERLAY_SECTION_ALSPS
#define OVERLAY2 OVERLAY_SECTION_ALSPS
#else
#define OVERLAY2
#endif  // OVERLAY_SECTION_ALSPS


/*****************************************************************************
* Overlay3: BARO
*****************************************************************************/
#define OVERLAY_SECTION_BARO                       \
    OVERLAY_ONE_OBJECT(bmp280, bmp280)

#ifdef OVERLAY_SECTION_BARO
#define OVERLAY3 OVERLAY_SECTION_BARO
#else
#define OVERLAY3
#endif  // OVERLAY_SECTION_BARO


/*****************************************************************************
* Overlay4: ALSPS_SECONDARY
*****************************************************************************/
#define OVERLAY_SECTION_ALSPS_SECONDARY

#ifdef OVERLAY_SECTION_ALSPS_SECONDARY
#define OVERLAY4 OVERLAY_SECTION_ALSPS_SECONDARY
#else
#define OVERLAY4
#endif  // OVERLAY_SECTION_ALSPS_SECONDARY


/*****************************************************************************
* Overlay5: SAR
*****************************************************************************/
#define OVERLAY_SECTION_SAR                       \
    /* OVERLAY_ONE_OBJECT(XXX, XXX) */

#ifdef OVERLAY_SECTION_SAR
#define OVERLAY5 OVERLAY_SECTION_SAR
#else
#define OVERLAY5
#endif  // OVERLAY_SECTION_SAR


/*****************************************************************************
* Overlay6: TO_BE_DETERMINED_6
*****************************************************************************/
#define OVERLAY_SECTION_TO_BE_DETERMINED_6        \
    /* OVERLAY_ONE_OBJECT(XXX, XXX) */

#ifdef OVERLAY_SECTION_TO_BE_DETERMINED_6
#define OVERLAY6 OVERLAY_SECTION_TO_BE_DETERMINED_6
#else
#define OVERLAY6
#endif  // OVERLAY_SECTION_TO_BE_DETERMINED_6


/*****************************************************************************
* Overlay7: TO_BE_DETERMINED_7
*****************************************************************************/
#define OVERLAY_SECTION_TO_BE_DETERMINED_7        \
    /* OVERLAY_ONE_OBJECT(XXX, XXX) */

#ifdef OVERLAY_SECTION_TO_BE_DETERMINED_7
#define OVERLAY7 OVERLAY_SECTION_TO_BE_DETERMINED_7
#else
#define OVERLAY7
#endif  // OVERLAY_SECTION_TO_BE_DETERMINED_7


/*****************************************************************************
* Overlay8: TO_BE_DETERMINED_8
*****************************************************************************/
#define OVERLAY_SECTION_TO_BE_DETERMINED_8        \
    /* OVERLAY_ONE_OBJECT(XXX, XXX) */

#ifdef OVERLAY_SECTION_TO_BE_DETERMINED_8
#define OVERLAY8 OVERLAY_SECTION_TO_BE_DETERMINED_8
#else
#define OVERLAY8
#endif  // OVERLAY_SECTION_TO_BE_DETERMINED_8


/*****************************************************************************
* Overlay9: TO_BE_DETERMINED_9
*****************************************************************************/
#define OVERLAY_SECTION_TO_BE_DETERMINED_9        \
    /* OVERLAY_ONE_OBJECT(XXX, XXX) */

#ifdef OVERLAY_SECTION_TO_BE_DETERMINED_9
#define OVERLAY9 OVERLAY_SECTION_TO_BE_DETERMINED_9
#else
#define OVERLAY9
#endif  // OVERLAY_SECTION_TO_BE_DETERMINED_9

