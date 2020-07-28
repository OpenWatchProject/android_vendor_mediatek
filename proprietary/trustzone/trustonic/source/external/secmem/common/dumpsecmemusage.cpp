#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "tlcsecmem.h"
#include "tlc_sec_mem.h"

#define LOG_TAG "TLC_SECMEM_USAGE"
#include "tlsecmem_log.h"

#define RET_OK	0

int main(int argc, char *argv[])
{
    unsigned int result	 = 0;
    UREE_SECUREMEM_HANDLE handle, handle1, handle2, handle3, handle4;
    mcResult_t ret = MC_DRV_OK;
    uint32_t refcount;

    printf("Secure memory usage\n");
    printf("###################\n");
    printf("SECMEM TLC called");

    ret = tlcMemOpen();
	
    if (MC_DRV_OK != ret) 
    {
        printf("open TL session failed!\n");
        return ret;
    }

    printf("Start to call the Dump Usage Secure Memory\n");
    if(RET_OK == UREE_DumpSecurememInfo())
    {
        printf("Secure memory dump success\n");	
    }
    
    tlcMemClose();	
    return ret;
}
