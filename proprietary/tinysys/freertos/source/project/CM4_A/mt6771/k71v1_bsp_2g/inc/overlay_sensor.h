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

#define OVERLAY0                                   \
    OVERLAY_ONE_OBJECT(lsm6dsm, lsm6dsm)           \
    OVERLAY_ONE_OBJECT(lsm6ds3, lsm6ds3)           \
    OVERLAY_ONE_OBJECT(bmi160, bmi160)             \
    OVERLAY_ONE_OBJECT(lis3dh, lis3dh)             \
    OVERLAY_ONE_OBJECT(lis2hh12, lis2hh12)

#define OVERLAY1                                   \
    OVERLAY_SEVEN_OBJECT(akm09915, akm09915, akm09918, akl_apis, measure, akm_wrapper, akm_apis, akl_nonos_ext)\
    OVERLAY_LIB_OBJECT(akm09915, libakm, AK*)      \
    OVERLAY_SEVEN_OBJECT(akm09918, akm09915, akm09918, akl_apis, measure, akm_wrapper, akm_apis, akl_nonos_ext)\
    OVERLAY_LIB_OBJECT(akm09918, libakm, AK*)      \
    OVERLAY_THREE_OBJECT(mmc3530, mmc3530, MemsicConfig, memsic_wrapper) \
    OVERLAY_TWO_OBJECT(mmc3530, libMemsicAlgo, Memsic*, mymath*)

#define OVERLAY2                                   \
    OVERLAY_ONE_OBJECT(cm36558, cm36558)           \
    OVERLAY_ONE_OBJECT(tmd2725, tmd2725)           \
    OVERLAY_ONE_OBJECT(apds9922, apds9922)         \
    OVERLAY_ONE_OBJECT(stk3x3x, stk3x3x)

#define OVERLAY3                                   \
    OVERLAY_ONE_OBJECT(bmp280, bmp280)

#define OVERLAY4
