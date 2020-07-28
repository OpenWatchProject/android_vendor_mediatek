#ifndef __MMSYS_MUTEX_H__
#define __MMSYS_MUTEX_H__

#include "mmsys_reg_base.h"
#include "DpPlatform.h"

#define MM_MUTEX_INTEN              (MMSYS_MUTEX_BASE + 0x00)
#define MM_MUTEX_INTSTA             (MMSYS_MUTEX_BASE + 0x04)
#define MM_MUTEX_CFG                (MMSYS_MUTEX_BASE + 0x08)
//#define MM_MUTEX_TIMEOUT            (MMSYS_MUTEX_BASE + 0x08)
//#define MM_MUTEX_COMMIT             (MMSYS_MUTEX_BASE + 0x0C)
//#define MM_MUTEX_DEBUG_OUT_SEL      (MMSYS_MUTEX_BASE + 0x30C)

#define MM_MUTEX_EN                 (MMSYS_MUTEX_BASE + 0x20 + m_mutexID * 0x20)
#define MM_MUTEX_GET                (MMSYS_MUTEX_BASE + 0x24 + m_mutexID * 0x20)
#define MM_MUTEX_RST                (MMSYS_MUTEX_BASE + 0x28 + m_mutexID * 0x20)
#define MM_MUTEX_MOD                (MMSYS_MUTEX_BASE + MM_MUTEX_MOD_OFFSET + m_mutexID * 0x20)
#define MM_MUTEX_SOF                (MMSYS_MUTEX_BASE + MM_MUTEX_SOF_OFFSET + m_mutexID * 0x20)


// MASK
#define MM_MUTEX_INTEN_MASK         (0x0FFF)
#define MM_MUTEX_INTSTA_MASK        (0x0FFF)
#define MM_MUTEX_DEBUG_OUT_SEL_MASK (0x03)
#define MM_MUTEX_CFG_MASK           (0x01)

#define MM_MUTEX_EN_MASK            (0x01)
#define MM_MUTEX_GET_MASK           (0x03)
#define MM_MUTEX_RST_MASK           (0x01)
#define MM_MUTEX_MOD_MASK           (0x0FFFFFFF)
#define MM_MUTEX_SOF_MASK           (0x0F)

#endif  // __MMSYS_MUTEX_H__
