/*
 * Cache linker script include
 */

#ifdef CFG_CACHE_SUPPORT
#ifndef __CACHE_LD
#define __CACHE_LD

/******************************************************************************
* Macros for common setions
* Please note that the definitions below cannot be enclosed in parenthesis.
******************************************************************************/
#define DRAM_TEXT_SECTIONS		.text*
#define DRAM_DATA_SECTIONS		.rodata* .data* .bss* *COMMON


#define CACHE_ALS_TEXT \
    ?*/middleware/contexthub/MEMS_Driver/alsps/?*.o(DRAM_TEXT_SECTIONS)
#define CACHE_ALS_DATA \
    ?*/middleware/contexthub/MEMS_Driver/alsps/?*.o(DRAM_DATA_SECTIONS)

#define CACHE_BARO_TEXT \
    ?*/middleware/contexthub/MEMS_Driver/barometer/?*.o(DRAM_TEXT_SECTIONS) \

#define CACHE_BARO_DATA \
    ?*/middleware/contexthub/MEMS_Driver/barometer/?*.o(DRAM_DATA_SECTIONS) \

#define CACHE_MAGNETOMETER_TEXT \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/lib/akl/AKM_Library/?*.o(DRAM_TEXT_SECTIONS) \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/lib/akl/AKM_Library/common/?*.o(DRAM_TEXT_SECTIONS) \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/lib/akl/AKM_Library/libSmartCompass/?*.a:*.o(DRAM_TEXT_SECTIONS) \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/lib/akl/AKM_Library/lib9d/?*.a:*.o(DRAM_TEXT_SECTIONS) \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/lib/akl/?*.o(DRAM_TEXT_SECTIONS) \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/?*.o(DRAM_TEXT_SECTIONS)

#define CACHE_MAGNETOMETER_DATA \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/lib/akl/AKM_Library/?*.o(DRAM_DATA_SECTIONS) \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/lib/akl/AKM_Library/common/?*.o(DRAM_DATA_SECTIONS) \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/lib/akl/AKM_Library/lib9d/?*.a:*.o(DRAM_DATA_SECTIONS) \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/lib/akl/AKM_Library/libSmartCompass/?*.a:*.o(DRAM_DATA_SECTIONS) \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/lib/akl/?*.o(DRAM_DATA_SECTIONS) \
    ?*/middleware/contexthub/MEMS_Driver/magnetometer/?*.o(DRAM_DATA_SECTIONS)

#define CACHE_FUSION_TEXT \
    ?*/middleware/contexthub/VIRT_Driver/sensorFusion.o(DRAM_TEXT_SECTIONS) \
    ?*/middleware/contexthub/algo/fusion/?*.a:*.o(DRAM_TEXT_SECTIONS)

#define CACHE_FUSION_DATA \
    ?*/middleware/contexthub/VIRT_Driver/sensorFusion.o(DRAM_DATA_SECTIONS) \
    ?*/middleware/contexthub/algo/fusion/?*.a:*.o(DRAM_DATA_SECTIONS)

#define CACHE_SMARTPA_TEXT \
    ?*/middleware/lib/aurisys/maxim/?*.o(DRAM_TEXT_SECTIONS) \
    ?*/middleware/lib/aurisys/maxim/libDSM_aurisys.a:*.o(DRAM_TEXT_SECTIONS) \
    ?*/drivers/common/audio/tasks/spkprotect/audio_task_speaker_protection.o(DRAM_TEXT_SECTIONS)

#define CACHE_SMARTPA_DATA \
    ?*/middleware/lib/aurisys/maxim/?*.o(DRAM_DATA_SECTIONS) \
    ?*/middleware/lib/aurisys/maxim/libDSM_aurisys.a:*.o(DRAM_DATA_SECTIONS) \
    ?*/drivers/common/audio/tasks/spkprotect/audio_task_speaker_protection.o(DRAM_DATA_SECTIONS)
#endif

#endif

