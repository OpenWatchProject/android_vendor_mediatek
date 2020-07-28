#ifndef __MTEE_SYS_FD_H
#define __MTEE_SYS_FD_H

#ifdef __NEBULA_HEE__
#define SYS_MTEE_SYS_FD   16
#define SYS_USER_MM_FD    17
#define SYS_USER_MTEE_FD  18
#define SYS_MEM_SRV_FD    19
#else
#define SYS_MTEE_SYS_FD   6
#define SYS_USER_MM_FD    7
#define SYS_USER_MTEE_FD  8
#define SYS_MEM_SRV_FD    9
#define SYS_MTEE_SAPU_FD  10
#endif

#endif
