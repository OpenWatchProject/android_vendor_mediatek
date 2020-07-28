#ifndef __MDP_REG_RSZ_H__
#define __MDP_REG_RSZ_H__

#include "mmsys_reg_base.h"

// For RSZ0
#define PRZ_ENABLE                              (ID_ADDR(m_identifier) + MDP_RSZ0_BASE + 0x0 )
#define PRZ_CONTROL                             (ID_ADDR(m_identifier) + MDP_RSZ0_BASE + 0x4 )
#define PRZ_INT_FLAG                            (ID_ADDR(m_identifier) + MDP_RSZ0_BASE + 0x8 )
#define PRZ_INPUT_IMAGE                         (ID_ADDR(m_identifier) + MDP_RSZ0_BASE + 0xc )
#define PRZ_OUTPUT_IMAGE                        (ID_ADDR(m_identifier) + MDP_RSZ0_BASE + 0x10)
#define PRZ_HORIZONTAL_COEFF_STEP               (ID_ADDR(m_identifier) + MDP_RSZ0_BASE + 0x14)
#define PRZ_VERTICAL_COEFF_STEP                 (ID_ADDR(m_identifier) + MDP_RSZ0_BASE + 0x18)
#define PRZ_LUMA_HORIZONTAL_INTEGER_OFFSET      (ID_ADDR(m_identifier) + MDP_RSZ0_BASE + 0x1c)
#define PRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET     (ID_ADDR(m_identifier) + MDP_RSZ0_BASE + 0x20)
#define PRZ_LUMA_VERTICAL_INTEGER_OFFSET        (ID_ADDR(m_identifier) + MDP_RSZ0_BASE + 0x24)
#define PRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET       (ID_ADDR(m_identifier) + MDP_RSZ0_BASE + 0x28)
#define PRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET    (ID_ADDR(m_identifier) + MDP_RSZ0_BASE + 0x2c)
#define PRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET   (ID_ADDR(m_identifier) + MDP_RSZ0_BASE + 0x30)
#define PRZ_CHROMA_VERTICAL_INTEGER_OFFSET      (ID_ADDR(m_identifier) + MDP_RSZ0_BASE + 0x34)
#define PRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET     (ID_ADDR(m_identifier) + MDP_RSZ0_BASE + 0x38)
#define PRZ_DEBUG_SEL                           (ID_ADDR(m_identifier) + MDP_RSZ0_BASE + 0x40)
#define PRZ_DEBUG                               (ID_ADDR(m_identifier) + MDP_RSZ0_BASE + 0x44)

#ifdef WIN32
/* Define this for DpDriver_Win32.cpp */
#define PRZ0_ENABLE                              (MDP_RSZ0_BASE + 0x0 )           
#define PRZ0_CONTROL							 (MDP_RSZ0_BASE + 0x4 )           
#define PRZ0_INPUT_IMAGE						 (MDP_RSZ0_BASE + 0xc )           
#define PRZ0_OUTPUT_IMAGE						 (MDP_RSZ0_BASE + 0x10)           
#define PRZ0_HORIZONTAL_COEFF_STEP				 (MDP_RSZ0_BASE + 0x14)           
#define PRZ0_VERTICAL_COEFF_STEP				 (MDP_RSZ0_BASE + 0x18)           
#define PRZ0_LUMA_HORIZONTAL_INTEGER_OFFSET      (MDP_RSZ0_BASE + 0x1c)           
#define PRZ0_LUMA_HORIZONTAL_SUBPIXEL_OFFSET     (MDP_RSZ0_BASE + 0x20)           
#define PRZ0_LUMA_VERTICAL_INTEGER_OFFSET		 (MDP_RSZ0_BASE + 0x24)           
#define PRZ0_LUMA_VERTICAL_SUBPIXEL_OFFSET		 (MDP_RSZ0_BASE + 0x28)           
#define PRZ0_CHROMA_HORIZONTAL_INTEGER_OFFSET	 (MDP_RSZ0_BASE + 0x2c)           
#define PRZ0_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET   (MDP_RSZ0_BASE + 0x30)           
#define PRZ0_CHROMA_VERTICAL_INTEGER_OFFSET		 (MDP_RSZ0_BASE + 0x34)           
#define PRZ0_CHROMA_VERTICAL_SUBPIXEL_OFFSET	 (MDP_RSZ0_BASE + 0x38)           
#define PRZ0_DEBUG_SEL							 (MDP_RSZ0_BASE + 0x40)           
#define PRZ0_DEBUG								 (MDP_RSZ0_BASE + 0x44)                              
                   
                   
#define PRZ1_ENABLE                              (MDP_RSZ1_BASE + 0x0 )           
#define PRZ1_CONTROL							 (MDP_RSZ1_BASE + 0x4 )           
#define PRZ1_INPUT_IMAGE						 (MDP_RSZ1_BASE + 0xc )           
#define PRZ1_OUTPUT_IMAGE						 (MDP_RSZ1_BASE + 0x10)           
#define PRZ1_HORIZONTAL_COEFF_STEP				 (MDP_RSZ1_BASE + 0x14)           
#define PRZ1_VERTICAL_COEFF_STEP				 (MDP_RSZ1_BASE + 0x18)           
#define PRZ1_LUMA_HORIZONTAL_INTEGER_OFFSET      (MDP_RSZ1_BASE + 0x1c)           
#define PRZ1_LUMA_HORIZONTAL_SUBPIXEL_OFFSET     (MDP_RSZ1_BASE + 0x20)           
#define PRZ1_LUMA_VERTICAL_INTEGER_OFFSET		 (MDP_RSZ1_BASE + 0x24)           
#define PRZ1_LUMA_VERTICAL_SUBPIXEL_OFFSET		 (MDP_RSZ1_BASE + 0x28)           
#define PRZ1_CHROMA_HORIZONTAL_INTEGER_OFFSET	 (MDP_RSZ1_BASE + 0x2c)           
#define PRZ1_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET   (MDP_RSZ1_BASE + 0x30)           
#define PRZ1_CHROMA_VERTICAL_INTEGER_OFFSET		 (MDP_RSZ1_BASE + 0x34)           
#define PRZ1_CHROMA_VERTICAL_SUBPIXEL_OFFSET	 (MDP_RSZ1_BASE + 0x38)           
#define PRZ1_DEBUG_SEL							 (MDP_RSZ1_BASE + 0x40)           
#define PRZ1_DEBUG								 (MDP_RSZ1_BASE + 0x44)                              
#endif // WIN32        


// MASK
#define PRZ_ENABLE_MASK                              (0x010001)
#define PRZ_CONTROL_MASK                             (0xF7FFFFF3)
#define PRZ_INT_FLAG_MASK                            (0x033)
#define PRZ_INPUT_IMAGE_MASK                         (0xDFFF1FFF)
#define PRZ_OUTPUT_IMAGE_MASK                        (0x9FFF1FFF)
#define PRZ_HORIZONTAL_COEFF_STEP_MASK               (0x07FFFFF)
#define PRZ_VERTICAL_COEFF_STEP_MASK                 (0x07FFFFF)
#define PRZ_LUMA_HORIZONTAL_INTEGER_OFFSET_MASK      (0x01FFF)
#define PRZ_LUMA_HORIZONTAL_SUBPIXEL_OFFSET_MASK     (0x01FFFFF)
#define PRZ_LUMA_VERTICAL_INTEGER_OFFSET_MASK        (0x01FFF)
#define PRZ_LUMA_VERTICAL_SUBPIXEL_OFFSET_MASK       (0x01FFFFF)
#define PRZ_CHROMA_HORIZONTAL_INTEGER_OFFSET_MASK    (0x01FFF)
#define PRZ_CHROMA_HORIZONTAL_SUBPIXEL_OFFSET_MASK   (0x01FFFFF)
#define PRZ_CHROMA_VERTICAL_INTEGER_OFFSET_MASK      (0x01FFF)
#define PRZ_CHROMA_VERTICAL_SUBPIXEL_OFFSET_MASK     (0x01FFFFF)
#define PRZ_DEBUG_SEL_MASK                           (0x0F)
#define PRZ_DEBUG_MASK                               (0xFFFFFFFF)

#endif // __MDP_REG_RSZ_H__
