/**
 *  @file   voip_mem.h
 *  @brief  general memory functions
 **/

#ifndef __LIB_UCE_API_MEMORY_H__
#define __LIB_UCE_API_MEMORY_H__

#include <stdlib.h>

/**
 * @brief get memory.
 * @param size [IN] the memory size
 * @param file [IN] the file number to call this function
 * @param line [IN] the line number to call this function
 * @return pointer of the memory
 */
void *API_get_mem(unsigned int size, char *file, int line);

/**
 * @brief free memory.
 * @param ptr [IN] the memory pointer
 * @param file [IN] the file number to call this function
 * @param line [IN] the line number to call this function
 */
void API_free_mem(void *ptr, char *file, int line);

/**
 * @brief enable memory debug to check who get and free memory
 */
#ifdef MEM_FREE_DEBUG
#define MEM_Allocate(s)         API_get_mem(s, __FILE__, __LINE__)
#define MEM_Free(p)             API_free_mem(p, __FILE__, __LINE__)
#else /* MEM_FREE_DEBUG */ /* disable memory debug */
#define MEM_Allocate(s)         malloc(s)
#define MEM_Free(p)             free(p)
#endif /* MEM_FREE_DEBUG */

void API_dump_mem_debug();

#endif /* __LIB_UCE_API_MEMORY_H__ */

