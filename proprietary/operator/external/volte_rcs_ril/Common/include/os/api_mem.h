/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#ifndef __COMMON_VOIP_MEMORY_H__
#define __COMMON_VOIP_MEMORY_H__

// #include "common/voip_config.h"

/**
 * @brief get memory
 * @param size [IN] the memory size
 * @return pointer of the memory
 */
void *voip_get_mem(unsigned int size);

/**
 * @brief free memory
 * @param p [IN] the memory pointer
 */
void voip_free_mem(void *p);

/**
 * @brief get memory. this function will call voip_get_mem() eventually.
 * @param size [IN] the memory size
 * @param file [IN] the file number to call this function
 * @param line [IN] the line number to call this function
 * @return pointer of the memory
 */
void *SIP_get_mem(unsigned int size, char *file, int line);

/**
 * @brief free memory. this function will call voip_free_mem() eventually.
 * @param ptr [IN] the memory pointer
 * @param file [IN] the file number to call this function
 * @param line [IN] the line number to call this function
 */
void  SIP_free_mem(void *ptr, char *file, int line);

void SIP_test_free_mem(void *ptr, char *file, int line);

/**
 * @brief enable memory debug to check who get and free memory
 */
//#define ENABLED_MEMORY_DEBUG_INFO
#ifdef ENABLED_MEMORY_DEBUG_INFO   // enable memory debug
    #define VOIP_MEM_FREE_DEBUG
    #define sip_get_mem(s)          SIP_get_mem(s, __FILE__, __LINE__)
    #define sip_free_mem(p)         SIP_free_mem(p, __FILE__, __LINE__)
    #define sip_test_free_mem(p)    SIP_test_free_mem(p, __FILE__, __LINE__)
#else   // disable memory debug
    #define sip_get_mem(s)          voip_get_mem(s)
    #define sip_free_mem(p)         voip_free_mem(p)
    #define sip_test_free_mem(p)
#endif


/**
 * @brief this function to check if the memory is get and free normally.
 * @return the memory utilization
 */
int sip_mem_get_free(void);

void sip_mem_status(char *str);

int sip_mem_init();

#endif /* __COMMON_VOIP_MEMORY_H__ */
