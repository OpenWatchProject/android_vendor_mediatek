#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>
#include <string.h>


#include "main.h"
#include "ioctl_priv.h"
#if defined(MT6620)
#include "mt6620_reg.h"
#elif defined(MT6628)
#include "mt6628_reg_copy.h"
#endif

enum ACCESS_MODE {
	FIFO_ACCESS,
	INCR_ACCESS,
	ACCESS_MODE_NUM
};

static unsigned int blksz;
static unsigned char func;
static unsigned int addr;
static int access_mode = -1;

unsigned int getBusClock(int fd) {
	unsigned int clock;

	if(ioctl(fd, MT6620_IOC_GET_BUS_CLOCK, &clock) == 0) {
		pr_debug("SDBUS clock = %d Hz\n", clock);
		return clock;
	}
	else
		return -1;
}

unsigned char getBusWidth(int fd) {
	unsigned char bus_width;

	if(ioctl(fd, MT6620_IOC_GET_SDBUS_WIDTH, &bus_width) == 0) {
		pr_debug("SDBUS Width = %d bit\n", bus_width);
		return bus_width;
	}
	else 
		return -1;
}

unsigned int getBlockSize(int fd) {
	if(ioctl(fd, MT6620_IOC_GET_BLOCK_SIZE, &blksz) == 0) {
		pr_debug("SDBUS Block Size = %d bytes\n", blksz);
		return blksz;
	}
	else 
		return -1;
}

unsigned int setBlockSize(int fd, unsigned int new_blksz) {
	if(ioctl(fd, MT6620_IOC_SET_BLOCK_SIZE, &new_blksz) == 0) {
		if(ioctl(fd, MT6620_IOC_GET_BLOCK_SIZE, &blksz) == 0)
			pr_debug("[NEW] SDBUS Block Size = %d bytes\n", blksz);
		return blksz;
	}
	else 
		return -1;
}

unsigned char getFuncFocus(int fd) {
	if(ioctl(fd, MT6620_IOC_GET_FUNC_FOCUS, &func) == 0) {
		pr_debug("SD Function focused on %d\n", func);
		return func;
	}
	else 
		return -1;
}

unsigned char setFuncFocus(int fd, unsigned char new_fn) {
	if(ioctl(fd, MT6620_IOC_SET_FUNC_FOCUS, &new_fn) == 0) {
		if(ioctl(fd, MT6620_IOC_GET_FUNC_FOCUS, &func) == 0)
			pr_debug("[NEW] SD Function focused on %d\n", func);
		return func;
	}
	else 
		return -1;
}

unsigned int getAddr(int fd) {
	if(ioctl(fd, MT6620_IOC_GET_ADDR, &addr) == 0) {
		pr_debug("SD current address = %08X\n", addr);
		return addr;
	}
	else 
		return -1;
}

unsigned int setAddr(int fd, unsigned int new_addr) {
	if(ioctl(fd, MT6620_IOC_SET_ADDR, &new_addr) == 0) {
		pr_debug("change new SD current address = %08X\n", new_addr);
		if(ioctl(fd, MT6620_IOC_GET_ADDR, &addr) == 0)
			pr_debug("SD current address = %08X\n", addr);
		return addr;
	}
	else 
		return -1;
}

int setFifoMode(int fd) {
	if(ioctl(fd, MT6620_IOC_SET_FIFO_MODE) == 0) {
		access_mode = FIFO_ACCESS;
		pr_debug("CMD53 switched to FIFO mode\n");
		return 0;
	}
	else 
		return -1;
}

int setIncrMode(int fd) {
	if(ioctl(fd, MT6620_IOC_SET_INCR_MODE) == 0) {
		access_mode = INCR_ACCESS;
		pr_debug("CMD53 switched to INCR mode\n");
		return 0;
	}
	else 
		return -1;
}

int write_byte(int fd, unsigned char fn, unsigned int address, unsigned char value) {
	if(func != fn)
		setFuncFocus(fd, fn);
	if(addr != address)
		setAddr(fd, address);

	if(ioctl(fd, MT6620_IOC_WRITE_DIRECT, &value) == 0) {
		return 0;
    }
	else {
		assert(0);
        return -1;
    }
};

unsigned char read_byte(int fd, unsigned char fn, unsigned int address) {
	unsigned char value;

	if(func != fn)
		setFuncFocus(fd, fn);
	if(addr != address)
		setAddr(fd, address);

	if(ioctl(fd, MT6620_IOC_READ_DIRECT, &value) == 0) {
		return value;
    }
	else {
		assert(0);
        return -1;
    }
};

int write_bytes(int fd, unsigned char fn, unsigned int address, unsigned char incr_mode, unsigned char *buf, int length) {
	if(func != fn)
		setFuncFocus(fd, fn);

	if(addr != address)
		setAddr(fd, address);

	if(incr_mode == 0 && access_mode != FIFO_ACCESS)
		setFifoMode(fd);
	else if(incr_mode == 1 && access_mode != INCR_ACCESS)
		setIncrMode(fd);

	return (int)write(fd, buf, length);
}

int read_bytes(int fd, unsigned char fn, unsigned int address, unsigned char incr_mode, unsigned char *buf, int length) {
	if(func != fn)
		setFuncFocus(fd, fn);

	if(addr != address)
		setAddr(fd, address);

	if(incr_mode == 0 && access_mode != FIFO_ACCESS)
		setFifoMode(fd);
	else if(incr_mode == 1 && access_mode != INCR_ACCESS)
		setIncrMode(fd);

	return (int)read(fd, buf, length);
}

int write_reg32(int fd, unsigned char fn, unsigned int address, unsigned int value) {
	if(func != fn)
		setFuncFocus(fd, fn);
	if(addr != address)
		setAddr(fd, address);

	return (int)write(fd, &value, sizeof(unsigned int));
}

int read_reg32(int fd, unsigned char fn, unsigned int address) {
	unsigned int tmp;
    ssize_t ret;

	if(func != fn)
		setFuncFocus(fd, fn);
	if(addr != address)
		setAddr(fd, address);

	ret = read(fd, &tmp, sizeof(addr));

    if(ret < 0) {
		myerrorLog("read_reg32 fail");
        return (int)ret;
    }
    else {
		//pr_debug("read SD ADDR[%08X]=%u\n", address, tmp);
    	return tmp;
    }
}

int reg_write_and_show(int fd, unsigned char fn, unsigned int address, unsigned int value) { 
	if(func != fn)
		setFuncFocus(fd, fn);
	if(addr != address)
		setAddr(fd, address);

	if(access_mode != INCR_ACCESS)
		setIncrMode(fd);

	pr_debug("setting fn%d[%04X] = %08X\n", fn, address, value);

	write(fd, &value, sizeof(unsigned int));
	read_and_show(fd, fn, address, sizeof(unsigned int));

    return 0;
}

int read_and_show(int fd, unsigned char fn, unsigned int address, unsigned int length) {
	int i;
	char *buffer;

	if((buffer = (char *)malloc(sizeof(char) * length)) == NULL)
		return -1;

	if(func != fn)
		setFuncFocus(fd, fn);
	if(addr != address)
		setAddr(fd, address);
	if(access_mode != INCR_ACCESS)
		setIncrMode(fd);

	read(fd, buffer, length);

	for(i = 0 ; i < (int)length ; i+= 4) {
		if(length - i >= 4) {
			mylog("ADDR[%08X] = %02X %02X %02X %02X\n", address + i, 
							buffer[i+0] & 0xff,
							buffer[i+1] & 0xff,
							buffer[i+2] & 0xff,
							buffer[i+3] & 0xff);
		}else {
			switch(length - i) {
			case 3:
				mylog("ADDR[%08X] = %02X %02X %02X\n", address + i, 
								buffer[i+0] & 0xff,
								buffer[i+1] & 0xff,
								buffer[i+2] & 0xff);
				break;
			case 2:
				mylog("ADDR[%08X] = %02X %02X\n", address + i, 
								buffer[i+0] & 0xff,
								buffer[i+1] & 0xff);
				break;
			case 1:
				mylog("ADDR[%08X] = %02X\n", address + i, 
								buffer[i+0] & 0xff);
				break;
			default:
				assert(0);
			}
		}
	}

	free(buffer);
	return 0;
}

int is_irq_pending(int fd, INTR_DATA_STRUCT_T *tmp) {
	if(ioctl(fd, MT6620_IOC_QUERY_IRQ_LEVEL, tmp) == 0) {
		
		//mylog("tmp irq =%d, u4HISR=%u\n", 
			//tmp->irq, tmp->u4HISR);

		// workaround for MCR_WTSR0 | MCR_WTSR1
		if((tmp->u4HISR & WHISR_TX_DONE_INT) == 0 &&
			(tmp->rTxInfo.au4WTSR[0] | tmp->rTxInfo.au4WTSR[1])) {
			tmp->u4HISR |= WHISR_TX_DONE_INT; //WHISR_TX_DONE_INT
		}

		return tmp->irq;
	}
	else {
		myerrorLog("QUERY_IRQ_LEVEL fail\n");
        return -1;
    }
}

int is_irq_pending_2(int fd, INTR_DATA_STRUCT_T *tmp) {
	if(ioctl(fd, MT6620_IOC_QUERY_IRQ_LEVEL, tmp) == 0) {
		return tmp->irq;
    }
	else {
		assert(0);
        return -1;
    }
}

int disable_interrupt(int fd, unsigned char fn) {
	return write_reg32(fd, fn, MCR_WHLPCR, 0x2);
}

int enable_interrupt(int fd, unsigned char fn) {
	return write_reg32(fd, fn, MCR_WHLPCR, 0x1);
}

void show_dump(char *buffer, int length) {
	int i;

	for(i = 0 ; i < length ; i+= 4) {
		if(length - i >= 4)
			printf("OFFSET[%08X] = %02X %02X %02X %02X\n", i, 
							buffer[i+0] & 0xff,
							buffer[i+1] & 0xff,
							buffer[i+2] & 0xff,
							buffer[i+3] & 0xff);
		else {
			switch(length - i) {
			case 3:
				printf("OFFSET[%08X] = %02X %02X %02X\n", i, 
								buffer[i+0] & 0xff,
								buffer[i+1] & 0xff,
								buffer[i+2] & 0xff);
				break;
			case 2:
				printf("OFFSET[%08X] = %02X %02X\n", i, 
								buffer[i+0] & 0xff,
								buffer[i+1] & 0xff);
				break;
			case 1:
				printf("OFFSET[%08X] = %02X\n", i, 
								buffer[i+0] & 0xff);
				break;
			default:
				assert(0);
			}
		}
	}

}

void dump(int fd, unsigned char fn, int addr, int length) {
	char buffer[64*1024];

	read_bytes(fd, fn, addr, fn, (unsigned char *)buffer, length);
	show_dump(buffer, length);
}

void cli_interpreter(int fd) {
	char *endptr;
	char cmd_buf[1024];

#define MAX_PARAM 3
	unsigned int params[MAX_PARAM];
	int param_num = 0;
	unsigned char fn;

	fn = getFuncFocus(fd);

	while(1) {
		printf("MediaTek> ");
		fgets(cmd_buf, 1023, stdin);

#define CMD_CMP(pivot) strncmp(cmd_buf, pivot, strlen(pivot)) 

		if(CMD_CMP("port") == 0) {
			param_num = 0;

			endptr = cmd_buf + strlen("port");

			if(*endptr == '\0' || *endptr == '\n') {
				printf("port <func-no> <addr> <value> : write operaion\n");
				printf("port <func-no> <addr>         : read operaion\n");
				printf("port <addr>         : read operaion for func#1\n");
				continue;
			}

			for(param_num = 0 ; param_num < MAX_PARAM ; ) {
				params[param_num++] = strtoul(endptr, &endptr, 16);

				if(endptr == NULL || *endptr == '\n')
					break;
			}

			if(param_num == 3) {
				// WRITE: [fn] [address] [value]
				write_reg32(fd, params[0], params[1], params[2]);
				read_and_show(fd, params[0], params[1], sizeof(unsigned int));
			}
			else if(param_num == 2) {
				// READ: [fn] [address]
				read_and_show(fd, params[0], params[1], sizeof(unsigned int));
			}
			else if(param_num == 1) {
				// READ: 1 [address]
				read_and_show(fd, fn, params[0], sizeof(unsigned int));
			}
			else {
				printf("port <func-no> <addr> <value> : write operaion\n");
				printf("port <func-no> <addr>         : read operaion\n");
				printf("port <func-no> <addr>         : read operaion for func#1\n");
			}
		}
		else if(CMD_CMP("dump") == 0) {
			param_num = 0;

			endptr = cmd_buf + strlen("dump");

			if(*endptr == '\0' || *endptr == '\n') {
				printf("dump <func-no> <addr> <length> : dump operaion\n");
				continue;
			}

			for(param_num = 0 ; param_num < MAX_PARAM ; ) {
				params[param_num++] = strtoul(endptr, &endptr, 16);

				if(endptr == NULL || *endptr == '\n')
					break;
			}

			if(param_num == 3)
				dump(fd, params[0], params[1], params[2]);
			else
				printf("dump <func-no> <addr> <length> : dump operaion\n");
		}
		else if(CMD_CMP("readb") == 0) {
			param_num = 0;

			endptr = cmd_buf + strlen("readb");

			if(*endptr == '\0' || *endptr == '\n') {
				printf("readb <func-no> <addr>         : read operaion\n");
				continue;
			}

			for(param_num = 0 ; param_num < MAX_PARAM ; ) {
				params[param_num++] = strtoul(endptr, &endptr, 16);

				if(endptr == NULL || *endptr == '\n')
					break;
			}

			if(param_num == 2) {
				char tmp;

				// READ: [fn] [address]
				tmp = read_byte(fd, params[0], params[1]);
				printf("CMD52 Read ADDR[%08X] = %02X\n", params[1], tmp & 0xff);
			}
			else {
				printf("readb <func-no> <addr>         : read operaion\n");
			}
		}
		else if(CMD_CMP("writeb") == 0) {
			param_num = 0;

			endptr = cmd_buf + strlen("writeb");

			if(*endptr == '\0' || *endptr == '\n') {
				printf("writeb <func-no> <addr> <value>: write operaion\n");
				continue;
			}

			for(param_num = 0 ; param_num < MAX_PARAM ; ) {
				params[param_num++] = strtoul(endptr, &endptr, 16);

				if(endptr == NULL || *endptr == '\n')
					break;
			}

			if(param_num == 3) {
				write_byte(fd, params[0], params[1], (unsigned char)params[2]);
				printf("CMD52 Write ADDR[%08X] Completed.\n", params[1]);
			}
			else {
				printf("writeb <func-no> <addr> <value>: write operaion\n");
			}
		}
		else if(CMD_CMP("exit") == 0 || CMD_CMP("quit") == 0)
			break;
		else if(CMD_CMP("kill") == 0)
			exit(1);
	}
}

extern int g_flag_fd;
int wifi_hifsys_init(int fd) {
	unsigned char fn;
	unsigned int crc_err;
	char buf[20];

	getBlockSize(fd);
	fn = getFuncFocus(fd);
	getAddr(fd);

	//mylog("1. Chip ID/Revision ID\n");
	read_and_show(fd, fn, MCR_WCIR, sizeof(unsigned int));

	//mylog("2. Request FW-Own back\n");
	while(!(read_reg32(fd, fn, MCR_WHLPCR) & 0x00000100)) {
		write_reg32(fd, fn, MCR_WHLPCR, 0x00000200);

		//Holmes test
		lseek(g_flag_fd,0,SEEK_SET);
		read(g_flag_fd, buf, 20);	  
		sscanf(buf,"%x",&crc_err);
		//printf("tune: %x\n",crc_err);
        /* check for termination */
        if(crc_err != 0) {
            //curr_result = WIFI_TEST_RESULT_FAIL_MISMATCH_CONTENT;
            return 1;
        }
		//Holmes//


		usleep(100*1000);
	}

	//mylog("3. Enabling all host interrupt except abnormal ones\n");
	write_reg32(fd, fn, MCR_WHIER, 0xffffff17);
	enable_interrupt(fd, getFuncFocus(fd));
	return 0;
}


int my_memcmp(const void *s1, const void *s2, size_t n) {
	int i;
	int ret = 0;

	for(i = 0 ; i < (int)n ; i+=4) {
		if(n - i >= 4) {
			if(*(int32_t *)((int)s1+i) != *(int32_t *)((int)s2+i)) {
				printf("SRC1[%08X] = %02X %02X %02X %02X\n", i, 
								(*(int8_t *)((int)s1 + i + 0)) & 0xff,
								(*(int8_t *)((int)s1 + i + 1)) & 0xff,
								(*(int8_t *)((int)s1 + i + 2)) & 0xff,
								(*(int8_t *)((int)s1 + i + 3)) & 0xff);
				printf("SRC2[%08X] = %02X %02X %02X %02X\n", i, 
								(*(int8_t *)((int)s2 + i + 0)) & 0xff,
								(*(int8_t *)((int)s2 + i + 1)) & 0xff,
								(*(int8_t *)((int)s2 + i + 2)) & 0xff,
								(*(int8_t *)((int)s2 + i + 3)) & 0xff);
				ret = -1;
			}
		}
		else {
			switch(n - i) {
			case 3:
				if(*(int16_t *)((int)s1+i) != *(int16_t *)((int)s2+i) ||
								*(int8_t *)((int)s1+i+2) != *(int8_t *)((int)s2+i+2)) {
					printf("SRC1[%08X] = %02X %02X %02X\n", i, 
									(*(int8_t *)((int)s1 + i + 0)) & 0xff,
									(*(int8_t *)((int)s1 + i + 1)) & 0xff,
									(*(int8_t *)((int)s1 + i + 2)) & 0xff);
					printf("SRC2[%08X] = %02X %02X %02X\n", i, 
									(*(int8_t *)((int)s2 + i + 0)) & 0xff,
									(*(int8_t *)((int)s2 + i + 1)) & 0xff,
									(*(int8_t *)((int)s2 + i + 2)) & 0xff);

					ret = -1;
				}
				break;
			case 2:
				if(*(int16_t *)((int)s1+i) != *(int16_t *)((int)s2+i)) {
					printf("SRC1[%08X] = %02X %02X\n", i, 
									(*(int8_t *)((int)s1 + i + 0)) & 0xff,
									(*(int8_t *)((int)s1 + i + 1)) & 0xff);
					printf("SRC2[%08X] = %02X %02X\n", i, 
									(*(int8_t *)((int)s2 + i + 0)) & 0xff,
									(*(int8_t *)((int)s2 + i + 1)) & 0xff);

					ret = -1;
				}
				break;
			case 1:
				if(*(int8_t *)((int)s1+i) != *(int8_t *)((int)s2+i)) {
					printf("SRC1[%08X] = %02X\n", i, 
									(*(int8_t *)((int)s1 + i + 0)) & 0xff);
					printf("SRC2[%08X] = %02X\n", i, 
									(*(int8_t *)((int)s2 + i + 0)) & 0xff);

					ret = -1;
				}
				break;
			default:
				assert(0);
			}
		}

		if(ret != 0)
			return ret;
	}

	return ret;
}

extern int g_flag_fd;
void flush_tx_count(int fd) {
	INTR_DATA_STRUCT_T irqTmp;
	unsigned char fn;
	unsigned int crc_err;
	char buf[20];

	fn = getFuncFocus(fd);

	enable_interrupt(fd, fn);

	// wait for TX/RX_DONE interrupt and updating TQ0 count by enhanced response
	while(1) {
		is_irq_pending(fd, &irqTmp);


		//Holmes test
		lseek(g_flag_fd,0,SEEK_SET);
		read(g_flag_fd, buf, 20);	  
		sscanf(buf,"%x",&crc_err);
		//printf("tune: %x\n",crc_err);
        /* check for termination */
        if(crc_err != 0) {
            //curr_result = WIFI_TEST_RESULT_FAIL_MISMATCH_CONTENT;
            break;
        }
		//Holmes//


		if(irqTmp.irq == 1) {
			//mylog("WHISR: 0x%08X\n", irqTmp.u4HISR);


			if(irqTmp.u4HISR & 0x1) {
				read_reg32(fd, fn, MCR_WTSR0);
				read_reg32(fd, fn, MCR_WTSR1);

				enable_interrupt(fd, fn);
			}
			else {
				break;
			}
		}
		else {
			//enable_interrupt(fd, fn);
			break;
		}
	}

	disable_interrupt(fd, fn);
}


void flush_rx_queue(int fd) {
	struct int_enhance_arg_t enhanced;
	uint16_t rx0Num, rx0TotalLength;
	uint16_t rx1Num, rx1TotalLength;
	unsigned char buffer[1024*1024];
	int i;
	INTR_DATA_STRUCT_T irqTmp, irqTmp2;
	unsigned char fn;
	uint16_t pktLength;

	fn = getFuncFocus(fd);

	// enable RX ennhanced mode
	write_reg32(fd, fn, MCR_WHCR, read_reg32(fd, fn, MCR_WHCR) | 0x00010000);

	// switch on INT Response mode
	enhanced.rxNum = 16;
	enhanced.totalBytes = 84;
	if(ioctl(fd, MT6620_IOC_SET_INT_ENHANCED, &enhanced) != 0)
		assert(0);

	enable_interrupt(fd, fn);

	// wait for TX/RX_DONE interrupt and updating TQ0 count by enhanced response
	while(1) {
		rx0TotalLength = rx1TotalLength = 0;

		is_irq_pending(fd, &irqTmp);

		if(irqTmp.irq == 1) {
			if(irqTmp.u4HISR & 0x6) {
				// retrive RX0 status
				rx0Num = irqTmp.rRxInfo.u.u2NumValidRx0Len;

				for(i = 0 ; i < rx0Num ; i++) {
					pktLength = irqTmp.rRxInfo.u.au2Rx0Len[i];
					rx0TotalLength += ALIGN_4(pktLength + 4);
				}

				// retrive RX1 status
				rx1Num = irqTmp.rRxInfo.u.u2NumValidRx1Len;

				for(i = 0 ; i < rx1Num ; i++) {
					pktLength = irqTmp.rRxInfo.u.au2Rx1Len[i];
					rx1TotalLength += ALIGN_4(pktLength + 4);
				}

				if(rx0TotalLength > 0) {
					read_bytes(fd, fn, MCR_WRDR0, 0, buffer, rx0TotalLength + enhanced.totalBytes);
					memcpy(&irqTmp2, &buffer[rx0TotalLength], enhanced.totalBytes);

					rx0TotalLength = 0;
				}

				if(rx1TotalLength > 0) {
					read_bytes(fd, fn, MCR_WRDR1, 0, buffer, rx1TotalLength + enhanced.totalBytes);
					memcpy(&irqTmp2, &buffer[rx1TotalLength], enhanced.totalBytes);

					rx1TotalLength = 0;
				}

				printf("flushed (%d/%d) packets\n", rx0Num, rx1Num);
			}
			else {
				break;
			}

			enable_interrupt(fd, fn);
		}
	}

	disable_interrupt(fd, fn);
}


