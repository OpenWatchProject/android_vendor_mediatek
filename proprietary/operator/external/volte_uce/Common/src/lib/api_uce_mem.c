#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <malloc.h>
#include <time.h>

#include "api_uce_mutex.h"
#include "api_uce_mem.h"

#ifdef MEM_FREE_DEBUG
#define MAX_MEM_LOG_NUM         (512)
#define MAX_DUP_MEM_ALLOCATE    (128)
static int debug_mem_vmb_count = 0;
static void *debug_mem_vmb_mutex = 0;
static struct __ss
{
    unsigned int addr;
    char *file;
    unsigned short line;
    unsigned short size;
    int c;
} debug_mem_vmb[MAX_MEM_LOG_NUM];
#endif /* MEM_FREE_DEBUG */


/*****************************************************************************
 * FUNCTION
 *  API_dump_mem_debug
 * DESCRIPTION
 *
 * PARAMETERS
 *  void
 * RETURNS
 *  void
 *****************************************************************************/
void API_dump_mem_debug()
{
#ifdef MEM_FREE_DEBUG
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    int i;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    printf("###################################\n");
    for (i = 0; i < MAX_MEM_LOG_NUM; ++i)
    {
        if (debug_mem_vmb[i].addr)
        {
            printf(
                "## not free [%d], file : %s, line = %d, time = %d, data = (%x)\n",
                i,
                debug_mem_vmb[i].file,
                debug_mem_vmb[i].line,
                debug_mem_vmb[i].c,
                debug_mem_vmb[i].addr);
        }
    }
    printf("###################################\n");
#endif /* MEM_FREE_DEBUG */
}


/*****************************************************************************
 * FUNCTION
 *  API_get_mem
 * DESCRIPTION
 *
 * PARAMETERS
 *  size        [IN]
 *  file        [?]
 *  line        [IN]
 * RETURNS
 *  void
 *****************************************************************************/
void *API_get_mem(unsigned int size, char *file, int line)
{
    char *tempF = file;
    int tempL = line;
#ifdef MEM_FREE_DEBUG
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    void *ptr = 0;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (debug_mem_vmb_mutex == 0)
    {
        debug_mem_vmb_mutex = malloc(CMUTX_Size());
        CMUTX_Init(debug_mem_vmb_mutex, "mem mutex");
        memset(debug_mem_vmb, 0, sizeof(debug_mem_vmb));
        debug_mem_vmb_count = 0;
    }

    CMUTX_Get(debug_mem_vmb_mutex, WAIT_FOREVER);
    ptr = malloc(size);
    if (ptr)
    {
        int i = 0;
        int write = 0;

        for (i = 0; i < MAX_MEM_LOG_NUM; ++i)
        {
            if (debug_mem_vmb[i].addr)
            {
            #if 0
                if (file == debug_mem_vmb[i].file && line == debug_mem_vmb[i].line) {
                    //if (debug_mem_vmb_count > MAX_MEM_LOG_NUM*2 && debug_mem_vmb[i].c < MAX_MEM_LOG_NUM) {
                    //    dump = 1;
                    //}
                    count++;
                }
                if ((unsigned int)debug_mem_vmb[i].addr == (unsigned int)ptr) {
                    printf("ERROR %p !!!!!!!!!!!!! %s:%d\n", ptr, file, line);
                    printf("DATA  %s:%d\n", debug_mem_vmb[i].file, debug_mem_vmb[i].line);
                    fflush(stdout);
                    exit(-1);
                }
            #endif /* 0 */
            }
            else
            {
                if (write == 0)
                {
                    debug_mem_vmb[i].addr = (unsigned int)ptr;
                    debug_mem_vmb[i].size = size;
                    debug_mem_vmb[i].file = file;
                    debug_mem_vmb[i].line = line;
                    debug_mem_vmb[i].c = debug_mem_vmb_count++;
                    write = 1;
                    break;
                }
            }
        }

    #if 0
        if (write == 0) {
            printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
            printf("memory can't find the free record, file = %s, line = %d, ptr = %p\n", file, line, ptr);
            printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        }

        if (count > MAX_DUP_MEM_ALLOCATE || dump == 1) {
            printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
            printf("file : %s, line = %d, size = %d, count = %d\n", file, line, size, count);
            for (i=0 ; i<MAX_MEM_LOG_NUM ; ++i) {
                if (debug_mem_vmb[i].addr) {
                    if (file == debug_mem_vmb[i].file && line == debug_mem_vmb[i].line) {
                        //printf("not free [%d], file : %s, line = %d, time = %d, data = (%x)(%s)\n", i, debug_mem_vmb[i].file, debug_mem_vmb[i].line, debug_mem_vmb[i].c, debug_mem_vmb[i].addr, (char *)(debug_mem_vmb[i].addr));
                        printf("not free [%d], file : %s, line = %d, time = %d, data = (%x)\n", i, debug_mem_vmb[i].file, debug_mem_vmb[i].line, debug_mem_vmb[i].c, debug_mem_vmb[i].addr);
                    }
                }
            }
            printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        }
    #endif /* 0 */
    }
    CMUTX_Put(debug_mem_vmb_mutex);

    return ptr;
#else /* MEM_FREE_DEBUG */
    return malloc(size);
#endif /* MEM_FREE_DEBUG */
}


/*****************************************************************************
 * FUNCTION
 *  API_free_mem
 * DESCRIPTION
 *
 * PARAMETERS
 *  ptr         [?]
 *  file        [?]
 *  line        [IN]
 * RETURNS
 *  void
 *****************************************************************************/
void API_free_mem(void *ptr, char *file, int line)
{
    /*----------------------------------------------------------------*/
    /* Local Variables                                                */
    /*----------------------------------------------------------------*/
    char *tempF = file;
    int tempL = line;

    /*----------------------------------------------------------------*/
    /* Code Body                                                      */
    /*----------------------------------------------------------------*/
    if (!ptr)
        return;
#ifdef MEM_FREE_DEBUG
    CMUTX_Get(debug_mem_vmb_mutex, WAIT_FOREVER);
    if (ptr)
    {
        int i = 0;
        int clear = 0;

        for (i = 0; i < MAX_MEM_LOG_NUM; ++i)
        {
            if ((unsigned int)debug_mem_vmb[i].addr == (unsigned int)ptr)
            {
                debug_mem_vmb[i].addr = 0;
                debug_mem_vmb[i].size = 0;
                debug_mem_vmb[i].file = 0;
                debug_mem_vmb[i].line = 0;
                debug_mem_vmb[i].c = 0;
                clear = 1;
                break;
            }
        }
        if (clear == 0)
        {
            printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
            printf("memory can't find the record, ptr = %p, %s:%d\n", ptr, file, line);
            printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
        }
    }
    free(ptr);
    CMUTX_Put(debug_mem_vmb_mutex);
#else /* MEM_FREE_DEBUG */
    free(ptr);
#endif /* MEM_FREE_DEBUG */
}

