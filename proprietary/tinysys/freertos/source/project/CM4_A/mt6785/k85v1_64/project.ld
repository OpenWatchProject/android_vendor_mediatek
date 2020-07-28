/*SCP sram size*/
_SCP_SRAM_SIZE = 0xc0000;
_SCP_RTOS_START = 0x800;

/* reserved memory areas*/
_SCP_IPC_SHARE_BUFFER_ADDR = _SCP_RTOS_START - 0x240;
_CNN_TO_SCP_SHARE_BUFFER_ADDR = _SCP_IPC_SHARE_BUFFER_ADDR - @@CFG_CNN_TO_SCP_BUF_SIZE@@;
_SCP_TO_CNN_SHARE_BUFFER_ADDR = _CNN_TO_SCP_SHARE_BUFFER_ADDR - @@CFG_SCP_TO_CNN_BUF_SIZE@@;

/* Specify the memory areas */
MEMORY {
        LOADER(rwx)     : ORIGIN = 0x00000000, LENGTH =    2K   /*loader for recovery use*/
#ifdef CFG_CACHE_SUPPORT
        RTOS(rwx)       : ORIGIN = 0x00000800, LENGTH =  750K   /* 768k - loader(2K) - cache(8K+8K) */
                                                                /* shift overlay section if enlarge cache region is required */
        DRAM(rwx)       : ORIGIN = @@CFG_L1C_DRAM_ADDR@@, LENGTH = @@CFG_L1C_DRAM_SIZE@@
#else
        RTOS(rwx)       : ORIGIN = 0x00000800, LENGTH =  766K   /* 768k - loader(2K) */
#endif
}

