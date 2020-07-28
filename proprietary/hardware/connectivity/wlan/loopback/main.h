#ifndef __MAIN_H__
#define __MAIN_H__

// for pattern generation
#include "ioctl_priv.h"
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "time.h"

//#define DEBUG

#if defined(DEBUG)
	#define pr_debug(fmt, arg...)  do {\
		time_t t = time(NULL); \
		int tod = t%86400; \
		int h = (tod/3600); \
		int m = (tod%3600)/60; \
		int s = (tod%60); \
		printf("%02d:%02d:%02d ", h, m, s); \
		printf(fmt, ##arg); \
	} while(0)
#else
	#define pr_debug(fmt, arg...) \
		({ if (0) printf(fmt, ##arg); 0; })
#endif

#define mylog(fmt, arg...)  do {\
		time_t t = time(NULL); \
		int tod = t%86400; \
		int h = (tod/3600); \
		int m = (tod%3600)/60; \
		int s = (tod%60); \
		printf("%02d:%02d:%02d ", h, m, s); \
		printf(fmt, ##arg); \
	} while(0)

#define myerrorLog(tag)  do {\
		time_t t = time(NULL); \
		int tod = t%86400; \
		int h = (tod/3600); \
		int m = (tod%3600)/60; \
		int s = (tod%60); \
		printf("%02d:%02d:%02d ", h, m, s); \
		printf("%s", tag); \
		printf("%s(%d)", strerror(errno), errno); \
	} while(0)

#define MT6620_CHRDEV "/dev/mt6620"

#define ALIGN_4(_value)            (((_value) + 3) & ~0x3)

// utility functions (commonly used)
unsigned int getBusClock(int fd);
unsigned char getBusWidth(int fd);

unsigned int getBlockSize(int fd);
unsigned int setBlockSize(int fd, unsigned int blksz);

unsigned char getFuncFocus(int fd);
unsigned char setFuncFocus(int fd, unsigned char fn);

unsigned int getAddr(int fd);
unsigned int setAddr(int fd, unsigned int addr);

int setFifoMode(int fd);
int setIncrMode(int fd);

// CMD52 operations
int write_byte(int fd, unsigned char fn, unsigned int address, unsigned char value);
unsigned char read_byte(int fd, unsigned char fn, unsigned int address);

// CMD53 operations
int write_bytes(int fd, unsigned char fn, unsigned int addr, unsigned char incr_mode, unsigned char *buf, int length);
int read_bytes(int fd, unsigned char fn, unsigned int addr, unsigned char incr_mode, unsigned char *buf, int length);

// CMD53 operations
int write_reg32(int fd, unsigned char fn, unsigned int addr, unsigned int value);
int read_reg32(int fd, unsigned char fn, unsigned int addr);

// IRQ
int is_irq_pending(int fd, INTR_DATA_STRUCT_T* desc);
int is_irq_pending_2(int fd, INTR_DATA_STRUCT_T* desc);
int disable_interrupt(int fd, unsigned char fn);
int enable_interrupt(int fd, unsigned char fn);

// for debugging 
int reg_write_and_show(int fd, unsigned char fn, unsigned int address, unsigned int value);
int read_and_show(int fd, unsigned char fn, unsigned int address, unsigned int length);

void dump(int fd, unsigned char fn, int addr, int length);

// cli
void cli_interpreter(int fd);

// test suite 
int start_tests(int fd, int testNo);

int wifi_hifsys_init(int fd);
void common_hifsys_init(int fd);

int my_memcmp(const void *s1, const void *s2, size_t n);

void flush_rx_queue(int fd);
void flush_tx_count(int fd);


#endif
