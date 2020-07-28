#ifndef __DP_PLATFORM_H__
#define __DP_PLATFORM_H__

#define RDMA_USE_SIMPLE_MODE   (true)
#define RDMA_SUPPORT_10BIT     (1) // used in DpEngine_RDMA.cpp
#define DMA_SUPPORT_ALPHA_ROT  (1) // used in DpEngine_RDMA.cpp and DpEngine_WROT.cpp

#define MOUT_MAP_SIZE    (6) // used in DpPathConnection.h
#define SEL_IN_SIZE      (6) // used in DpPathConnection.h
#define SOUT_MAP_SIZE    (1) // used in DpPathConnection.h
#define ReadRegister   (0) // used in DpIspStream.cpp
#define MM_MUTEX_MOD_OFFSET (0x30) // used in mmsys_mutex.h
#define MM_MUTEX_SOF_OFFSET (0x2c) // used in mmsys_mutex.h

#define ESL_SETTING     (1)

#define PMQOS_SETTING   (1) // used in DpDriver_Android.cpp
#define WROT_FILTER_CONSTRAINT (1) // used in DpEngine_WROT.cpp

#define CONFIG_FOR_FLUSH_RANGE (1)

#endif  // __DP_PLATFORM_H__

