/*
 * Cache linker script include
 */

#ifdef CFG_CACHE_SUPPORT
#ifndef __CACHE_LD
#define __CACHE_LD

#define CACHE_ALS_TEXT \
    ?*/middleware/contexthub/MEMS_Driver/alsps/?*.o(.text*)
#define CACHE_ALS_DATA \
    ?*/middleware/contexthub/MEMS_Driver/alsps/?*.o(.rodata* .data* .bss*)

#define CACHE_BARO_TEXT \
    ?*/middleware/contexthub/MEMS_Driver/barometer/?*.o(.text*) \

#define CACHE_BARO_DATA \
    ?*/middleware/contexthub/MEMS_Driver/barometer/?*.o(.rodata* .data* .bss*) \

#define CACHE_MAGNETOMETER_TEXT \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/lib/akl/AKM_Library/?*.o(.text*) \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/lib/akl/AKM_Library/common/?*.o(.text*) \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/lib/akl/AKM_Library/libSmartCompass/?*.a:*.o(.text*) \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/lib/akl/AKM_Library/lib9d/?*.a:*.o(.text*) \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/lib/akl/?*.o(.text*) \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/?*.o(.text*)

#define CACHE_MAGNETOMETER_DATA \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/lib/akl/AKM_Library/?*.o(.rodata* .data* .bss*) \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/lib/akl/AKM_Library/common/?*.o(.rodata* .data* .bss*) \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/lib/akl/AKM_Library/lib9d/?*.a:*.o(.rodata* .data* .bss*) \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/lib/akl/AKM_Library/libSmartCompass/?*.a:*.o(.rodata* .data* .bss*) \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/lib/akl/?*.o(.rodata* .data* .bss*) \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/?*.o(.rodata* .data* .bss*)

#define CACHE_FUSION_TEXT \
    ?*/middleware/contexthub/VIRT_Driver/sensorFusion.o(.text*) \
    ?*/middleware/contexthub/algo/fusion/?*.a:*.o(.text*)

#define CACHE_FUSION_DATA \
    ?*/middleware/contexthub/VIRT_Driver/sensorFusion.o(.rodata* .data* .bss*) \
    ?*/middleware/contexthub/algo/fusion/?*.a:*.o(.rodata* .data* .bss*)

#define CACHE_SMARTPA_TEXT \
    ?*/middleware/lib/aurisys/maxim/?*.o(.text*) \
    ?*/middleware/lib/aurisys/maxim/libDSM_aurisys.a:*.o(.text*) \
    ?*/drivers/common/audio/tasks/spkprotect/audio_task_speaker_protection.o(.text*)

#define CACHE_SMARTPA_DATA ?*/middleware/lib/aurisys/maxim/?*.o(.rodata* .data* .bss) \
    ?*/middleware/lib/aurisys/maxim/libDSM_aurisys.a:*.o(.rodata* .data* .bss*) \
    ?*/drivers/common/audio/tasks/spkprotect/audio_task_speaker_protection.o(.rodata* .data* .bss*)
#endif

#endif

