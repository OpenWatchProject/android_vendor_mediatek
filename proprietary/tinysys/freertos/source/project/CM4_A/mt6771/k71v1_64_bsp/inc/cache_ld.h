/*
 * Cache linker script include
 */

#ifdef CFG_CACHE_SUPPORT
#ifndef __CACHE_LD
#define __CACHE_LD

#define CACHE_ALS_TEXT \
    ?*/middleware/contexthub/MEMS_Driver/alsps/?*.o(.text*)
#define CACHE_ALS_DATA \
    ?*/middleware/contexthub/MEMS_Driver/alsps/?*.o(.rodata* .data* .bss* *COMMON)

#define CACHE_BARO_TEXT \
    ?*/middleware/contexthub/MEMS_Driver/barometer/?*.o(.text*) \

#define CACHE_BARO_DATA \
    ?*/middleware/contexthub/MEMS_Driver/barometer/?*.o(.rodata* .data* .bss* *COMMON) \

#define CACHE_MAGNETOMETER_TEXT \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/lib/akl_allinone/Platform/?*.o(.text*) \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/lib/akl_allinone/AKM_Library/?*.a:*.o(.text*) \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/?*.o(.text*)

#define CACHE_MAGNETOMETER_DATA \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/lib/akl_allinone/Platform/?*.o(.rodata* .data* .bss* *COMMON) \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/lib/akl_allinone/AKM_Library/?*.a:*.o(.rodata* .data* .bss* *COMMON)

#define CACHE_FUSION_TEXT \
    ?*/middleware/contexthub/VIRT_Driver/sensorFusion.o(.text*) \
    ?*/middleware/contexthub/algo/fusion/?*.a:*.o(.text*)

#define CACHE_FUSION_DATA \
    ?*/middleware/contexthub/VIRT_Driver/sensorFusion.o(.rodata* .data* .bss* *COMMON) \
    ?*/middleware/contexthub/algo/fusion/?*.a:*.o(.rodata* .data* .bss* *COMMON)

#define CACHE_VOW_TEXT \
    ?*/middleware/lib/audio/cm4/vow/libaec.a:*.o(.text*) \
    ?*/middleware/lib/audio/cm4/vow/mt6771/libblisrc_accurate_vow.a:*.o(.text*)

#define CACHE_VOW_DATA \
    ?*/middleware/lib/audio/cm4/vow/libaec.a:*.o(.rodata* .data* .bss*) \
    ?*/middleware/lib/audio/cm4/vow/mt6771/libblisrc_accurate_vow.a:*.o(.rodata* .data* .bss*)

#if defined(CFG_MTK_SPEAKER_PROTECTION_SUPPORT)
#define CACHE_SMARTPA_TEXT \
    ?*/middleware/lib/aurisys/dummylib/?*.o(.text*) \
    ?*/middleware/lib/aurisys/dummylib/libdummylib.a:*.o(.text*) \
    ?*/middleware/lib/aurisys/mtk/libMT6660_LIB.a:*.o(.text*) \
    ?*/drivers/common/audio/tasks/spkprotect/audio_task_speaker_protection.o(.text*) \
    ?*/drivers/common/audio/framework/?*.o(.text*) \
    ?*/middleware/lib/audio_utility/?*.o(.text*)
#define CACHE_SMARTPA_DATA \
    ?*/middleware/lib/aurisys/dummylib/?*.o(.rodata* .data* .bss*) \
    ?*/middleware/lib/aurisys/dummylib/libdummylib.a:*.o(.rodata* .data* .bss*) \
    ?*/middleware/lib/aurisys/mtk/libMT6660_LIB.a:*.o(.rodata* .data* .bss*) \
    ?*/drivers/common/audio/tasks/spkprotect/audio_task_speaker_protection.o(.rodata* .data* .bss*) \
    ?*/drivers/common/audio/framework/?*.o(.rodata* .data* .bss*) \
    ?*/middleware/lib/audio_utility/?*.o(.rodata* .data* .bss*)
#endif

#endif
#endif
