/*
 * Define sensor overlay object here
 */


#define OVERLAY_SECTION_TEXT (.text* .data* .rodata* .bss*)
#define OVERLAY_ONE_OBJECT(tag, file) .tag { *file.o OVERLAY_SECTION_TEXT }
#define OVERLAY_TWO_OBJECT(tag, file1, file2) \
        .tag { *file1.o OVERLAY_SECTION_TEXT } \
        .tag { *file2.o OVERLAY_SECTION_TEXT }

#define OVERLAY_LIB_OBJECT(tag, lib, file) .tag { lib.a: file.o OVERLAY_SECTION_TEXT }
#define OVERLAY_THREE_OBJECT(tag, file1, file2, file3) \
        .tag { *file1.o OVERLAY_SECTION_TEXT } \
        .tag { *file2.o OVERLAY_SECTION_TEXT } \
        .tag { *file3.o OVERLAY_SECTION_TEXT }
#define OVERLAY_SIX_OBJECT(tag, file1, file2, file3, file4, file5, file6, file7) \
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
    OVERLAY_TWO_OBJECT(stk832x, stk832x, stk_hand_detect)

#ifdef OVERLAY_SECTION_ACCGYRO
#define OVERLAY0 OVERLAY_SECTION_ACCGYRO
#else
#define OVERLAY0
#endif  // OVERLAY_SECTION_ACCGYRO


/*****************************************************************************
* Overlay1: MAG
*****************************************************************************/
#define OVERLAY_SECTION_MAG                        \
    OVERLAY_ONE_OBJECT(akm09918, akm09918)

#ifdef OVERLAY_SECTION_MAG
#define OVERLAY1 OVERLAY_SECTION_MAG
#else
#define OVERLAY1
#endif  // OVERLAY_SECTION_MAG


/*****************************************************************************
* Overlay2: ALSPS
*****************************************************************************/
#define OVERLAY_SECTION_ALSPS                      \
    OVERLAY_ONE_OBJECT(epl_sensor, epl_sensor)

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
#define OVERLAY_SECTION_ALSPS_SECONDARY           \
    /* OVERLAY_ONE_OBJECT(XXX, XXX) */

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

