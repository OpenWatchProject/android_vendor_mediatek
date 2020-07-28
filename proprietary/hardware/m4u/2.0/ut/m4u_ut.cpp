/* 
 * data path: virtual memory -> m4u -> LCDC_R
 * LCD_R read BufAddr through M4U, then LCD_W write the data to PMEM PA 
 * test APP dump PMEM_VA image to verify
 */

#include "stdio.h"
#include "errno.h"
#include "fcntl.h"
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <cutils/log.h>
#include "m4u_lib.h"
#include <ion/ion.h>
#include <libion_mtk/ion.h>

#undef LOG_TAG
#define LOG_TAG "[m4u_ut]"

#define MTKM4UDEBUG
#ifdef MTKM4UDEBUG
  #define M4UDBG(string, args...) printf("[M4U_N]"string,##args)
#else
  #define M4UDBG(string, args...)
#endif

#define TEST_START() M4UDBG("--------------test start: %s ---------------- \n", __FUNCTION__)
#define TEST_END() M4UDBG("--------------test end: %s ---------------- \n", __FUNCTION__)



extern unsigned char const rgb565_390x210[];

//#define M4U_MEM_USE_NEW
#define M4U_MEM_USE_PMEM



int vAllocate_Deallocate_basic()
{
    unsigned int i;
    int ret;
    unsigned int BufSize = 1024*1024;
    unsigned char* BufAddr = new unsigned char[BufSize];
    unsigned int BufMVA;
    MTKM4UDrv CM4u;
    
    M4U_PORT_STRUCT port;
    port.ePortID = M4U_PORT_DISP_OVL0;
    port.Direction = 0;
    port.Distance = 1;
    port.domain = 3;
    port.Security = 0;
    port.Virtuality = 1;
    CM4u.m4u_config_port(&port);


    for(i=0; i<BufSize; i++)
        BufAddr[i] = 0x55;

    ret = CM4u.m4u_alloc_mva(M4U_PORT_DISP_OVL0,     
                       (unsigned long)BufAddr, BufSize,             
                       M4U_PROT_READ|M4U_PROT_WRITE,
                       M4U_FLAGS_SEQ_ACCESS,
                       &BufMVA);             
    if(ret)
    {
        printf("allocate mva fail. ret=0x%x\n", ret);
        return ret;
    }
                       
    ret = CM4u.m4u_cache_sync(M4U_PORT_DISP_OVL0, M4U_CACHE_FLUSH_BY_RANGE, 
                    (unsigned long)BufAddr, BufSize, BufMVA);
    if(ret)
    {
        printf("cache flush fail. ret=%d,va=0x%lx,size=0x%x\n", ret,(unsigned long)BufAddr,BufSize);
        return ret;
    }

    ret = CM4u.m4u_cache_sync(M4U_PORT_DISP_OVL0, M4U_CACHE_INVALID_BY_RANGE, 
                    (unsigned long)BufAddr, BufSize, BufMVA);
    if(ret)
    {
        printf("cache invalid fail. ret=%d,va=0x%lx,size=0x%x\n", ret,(unsigned long)BufAddr,BufSize);
        return ret;
    }

    ret = CM4u.m4u_cache_sync(M4U_PORT_DISP_OVL0, M4U_CACHE_CLEAN_BY_RANGE, 
                    (unsigned long)BufAddr, BufSize, BufMVA);
    if(ret)
    {
        printf("cache invalid fail. ret=%d,va=0x%lx,size=0x%x\n", ret,(unsigned long)BufAddr,BufSize);
        return ret;
    }

    /*== OVL0 use mva here ==*/

    ret = CM4u.m4u_dealloc_mva(M4U_PORT_DISP_OVL0, (unsigned long)BufAddr, BufSize, BufMVA);
    if(ret)
    {
        printf("m4u_dealloc_mva fail. ret=%d, mva=0x%x\n", ret, BufMVA);
    }

    return 0;
}

int ion_m4u_misc_using()
{
    int i;
    int ion_fd;
    int ion_test_fd;
    ion_user_handle_t handle;
    int share_fd;
    volatile char* pBuf;
    pid_t pid;
    unsigned int bufsize = 1*1024*1024;

    ion_fd = ion_open();
    if (ion_fd < 0)
    {
        printf("Cannot open ion device.\n");
        return 0;
    }
    if (ion_alloc_mm(ion_fd, bufsize, 4, 0, &handle))
    {
        printf("IOCTL[ION_IOC_ALLOC] failed!\n");
        return 0;
    }

    if (ion_share(ion_fd, handle, &share_fd))
    {
        printf("IOCTL[ION_IOC_SHARE] failed!\n");
        return 0;
    }

    pBuf = (char*)ion_mmap(ion_fd, NULL, bufsize, PROT_READ|PROT_WRITE, MAP_SHARED, share_fd, 0);
    printf("ion_map: pBuf = 0x%lx\n", (unsigned long)pBuf);
    if (!pBuf)
    {
        printf("Cannot map ion buffer.\n");
        return 0;
    }


    MTKM4UDrv CM4u;
    unsigned int BufMVA;
    int ret;
    ret = CM4u.m4u_alloc_mva(0,     
                       (unsigned long)pBuf, bufsize,             
                       M4U_PROT_READ|M4U_PROT_WRITE,
                       M4U_FLAGS_SEQ_ACCESS,
                       &BufMVA);             
    if(ret)
    {
        printf("allocate mva fail. ret=0x%x\n", ret);
        return ret;
    }
    printf("mva=0x%x\n", BufMVA);
                       
    ret = CM4u.m4u_cache_sync(0, M4U_CACHE_FLUSH_BY_RANGE, 
                    (unsigned long)pBuf,bufsize, BufMVA);
    if(ret)
    {
        printf("cache flush fail. ret=%d,va=0x%lx,size=0x%x\n", ret,(unsigned long)pBuf,bufsize);
        return ret;
    }
    ret = CM4u.m4u_dealloc_mva(0, (unsigned long)pBuf,bufsize, BufMVA);
    if(ret)
    {
        printf("m4u_dealloc_mva fail. ret=%d, mva=0x%x\n", ret, BufMVA);
    }

    return 0;

}


int main (int argc, char *argv[])
{
    vAllocate_Deallocate_basic();
    ion_m4u_misc_using();

    return 0;
}


