#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/stat.h>
#include <time.h>
#include <limits.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <cutils/log.h>
#include <cutils/misc.h>

#include "main.h"
#include "ioctl_priv.h"
#if defined(MT6620)
#include "mt6620_reg.h"
#elif defined(MT6628)
#include "mt6628_reg_copy.h"
#endif
#include "test_lib.h"

//#define ENABLE_PTHREAD
//#define VERBOSE_LOG


extern int init_module(void *, unsigned long, const char *);
extern int delete_module(const char *, unsigned int);

#define MAX_TX_QUEUE_NUM 		16

typedef struct _loopback_param_set {
    unsigned int packet_length;
    unsigned int packet_num;
} LOOPBACK_PARAM_SET, *P_LOOPBACK_PARAM_SET;

/* Packet info to the host driver. The first DW is consistent with the first DW of the HIF RX header. */
	// total 16 bytes anyway, this header is pasted by HIF-SW
typedef struct _NIC_HIF_RX_HEADER_T{
	uint16_t	u2PacketLen;
	uint16_t	u2PacketType;
} NIC_HIF_RX_HEADER_T, *P_NIC_HIF_RX_HEADER_T;

/* Packet info from the host driver. The first DW is consistent with the first DW of the HIF RX header. */
	// change to 4 bytes long which is used currently by F/W
	// this structure is for HIF-HW
typedef struct _NIC_HIF_TX_HEADER_T{
	uint16_t	u2TxByteCount;
	uint8_t		ucEtherTypeOffset;
	uint8_t		ucCSUMFlags;
} NIC_HIF_TX_HEADER_T, *P_NIC_HIF_TX_HEADER_T;

/* static variables */
static int running_flag = 0;
static int completed_packet_num = 0;
#if defined(ENABLE_PTHREAD)
static pthread_t thread_instance;
#endif
static ENUM_WIFI_TEST_RESULT_T curr_result = WIFI_TEST_RESULT_FAIL_NOT_STARTED;
static LOOPBACK_PARAM_SET paramSet;
int g_flag_fd = 0;


static void randomize(void) 
{
	static int i;
	// randomize
	srand(i + (unsigned int)time(NULL));
	i++;
}

static int generate_random_payload(unsigned char *buf, int len)
{
	int i;

	randomize();

	for(i = 0 ; i < len ; i++)
		buf[i] = (unsigned char)(rand() & 0xff);

	return 0;
}

static int send_packet(int portNum, int fd, unsigned char fn, unsigned char *buf, int len)
{
    int retval;

	// write to WTDR directly
	if(portNum == 0)
		retval = write_bytes(fd, fn, MCR_WTDR0, 0, buf, len);
	else if(portNum == 1)
		retval = write_bytes(fd, fn, MCR_WTDR1, 0, buf, len);
	else
		assert(0);

    if(retval < 0) {
		myerrorLog("write_types fail");
        return retval;
    }
    else {
    	return 0;
    }
}

#if 0
static int recv_packet(int portNum, int fd, unsigned char fn, unsigned char *buf, int *len)
{
	INTR_DATA_STRUCT_T irqTmp;
	unsigned int dwTmp;
	int pktLength;

	// Wait until interrupted by RX_DONE (TX_DONE may appears too)
	while(1) {
		is_irq_pending(fd, &irqTmp);

		if(irqTmp.irq == 1) {
			enable_interrupt(fd, fn);

			if(irqTmp.u4HISR & 0x1) { // TX DONE
				read_reg32(fd, fn, MCR_WTSR0);
				read_reg32(fd, fn, MCR_WTSR1);
			}
			if(irqTmp.u4HISR & (1 << (1 + portNum))) // RX# DONE
				break;
		}
	}

	// Read WRPLR for RX packet length
	dwTmp = read_reg32(fd, fn, WRPLR);

	if(portNum == 0)
		pktLength = dwTmp & 0xffff;
	else if(portNum == 1)
		pktLength = (dwTmp >> 16) & 0xffff;
	else
		assert(0);

	// RX Packet length is not including last DW indicating CS_STATUS
	pktLength = (pktLength + 0x7) & (~0x3);

	// read from WRDR
	if(portNum == 0)
		read_bytes(fd, fn, MCR_WRDR0, 0, buf, pktLength);
	else if(portNum == 1)
		read_bytes(fd, fn, MCR_WRDR1, 0, buf, pktLength);
	else
		assert(0);

	// clear status after packet is received
	is_irq_pending(fd, &irqTmp);
	enable_interrupt(fd, fn);

	*len = pktLength;

	return 0;
}
#endif

/*
 * @return     : 0 for correct, -1 on error
 */
static int tx_compare_packet(unsigned char *send_buf, 
				unsigned char *recv_buf,
				int len)
{
	int send_buf_start, recv_buf_start;
	size_t length_to_compare;
	
	send_buf_start = sizeof(NIC_HIF_TX_HEADER_T);
	recv_buf_start = sizeof(NIC_HIF_RX_HEADER_T);

    if(len != ((P_NIC_HIF_RX_HEADER_T)recv_buf)->u2PacketLen) {
#if defined(VERBOSE_LOG)
        printf("length mismatch!\n");
#endif
        return -1;
    }

	length_to_compare = ((P_NIC_HIF_RX_HEADER_T)recv_buf)->u2PacketLen - sizeof(NIC_HIF_RX_HEADER_T);

	// content matching check
	return my_memcmp(send_buf + send_buf_start, recv_buf + recv_buf_start, length_to_compare);
}


static int generate_frame_with_random_payload(
				unsigned char *buf,
				int len)
{
	// controls pattern (initialization included)
	char *payload = NULL;
	u_short payload_s = 0;

    // header manipulation
    NIC_HIF_TX_HEADER_T txHeader;
	char *dst;
    int offset;

	// size check
	if(len < (int)sizeof(NIC_HIF_TX_HEADER_T)) {
		assert(0);
    }
				
	// randomize();
	randomize();

	/* 1. payload generation */
	payload_s = len - sizeof(NIC_HIF_TX_HEADER_T);
	if(payload_s > (u_short)0) {
		payload = (char *)malloc(sizeof(char) * payload_s);
		generate_random_payload((unsigned char *)payload, payload_s);
	}
	
    /* 2. header manipulation */
    dst = (char*)buf;
    offset = 0;

	// attach NIC_HIF_TX_HEADER_T in the head
    txHeader.u2TxByteCount		= len;
    txHeader.ucEtherTypeOffset 	= (sizeof(NIC_HIF_TX_HEADER_T) + 12) / 2;
	/* HIF_TX_LOOPBACK = (0x2 << 6)*/
	txHeader.ucCSUMFlags		= (0x2 << 6);

    memcpy(&dst[offset], &txHeader, sizeof(NIC_HIF_TX_HEADER_T));
    offset += sizeof(NIC_HIF_TX_HEADER_T);

    // memcpy for generated frame
    memcpy(&dst[offset], payload, payload_s);

    // free allocated payload
	free(payload);

	return 0;
}


static int 
proc_mtk_wifi_loopback(int fd, unsigned int packet_length, unsigned int packet_num) 
{
	struct int_enhance_arg_t enhanced;
    unsigned char *in_buf;
	int64_t pktSentCnt, pktRecvCnt;
	int64_t received_bytes;
	int TQ_count = MAX_TX_QUEUE_NUM;
	int RX_count;
	int i;
	INTR_DATA_STRUCT_T irqTmp, irqTmp2;
	unsigned char fn;
	unsigned int crc_err;
	char buf[20];




    /* parameter check */
    if(packet_length > 1500) {
        curr_result = WIFI_TEST_RESULT_FAIL_INVALID_PARAMS;
        return -1;
    }
    else {
        packet_length = ALIGN_4(packet_length);
    }

    /* allocate memory */
	in_buf = (unsigned char *)malloc(sizeof(unsigned char) * 4*1024*MAX_TX_QUEUE_NUM);

	fn = getFuncFocus(fd);

	// enable RX ennhanced mode
	if(write_reg32(fd, fn, MCR_WHCR, read_reg32(fd, fn, MCR_WHCR) | 0x00010000) < 0) {
        curr_result = WIFI_TEST_RESULT_FAIL_UNEXPECTED_STOP;
        goto out;
    }

	// switch on INT Response mode
	enhanced.rxNum = 16;
	enhanced.totalBytes = 84;
	if(ioctl(fd, MT6620_IOC_SET_INT_ENHANCED, &enhanced) != 0) {
        curr_result = WIFI_TEST_RESULT_FAIL_UNEXPECTED_STOP;
        goto out;
    }

#if defined(VERBOSE_LOG)
	printf(">> Test: WIFI HIFSYS Loopback test \n");
#endif

	if(enable_interrupt(fd, fn) < 0) {
        curr_result = WIFI_TEST_RESULT_FAIL_UNEXPECTED_STOP;
        goto out;
    }

	received_bytes = 0;
	pktSentCnt = pktRecvCnt = 0;

	while(1) {
		uint16_t rx0Num, rx0TotalLength;
		uint16_t rx1Num, rx1TotalLength;
		unsigned char buffer[1024*1024];
		uint16_t rxOffset;
		uint16_t ignoreCount = 0;

		// create an aggregated packet
		generate_frame_with_random_payload(in_buf, ALIGN_4(packet_length));
		for(i = 1 ; i < MAX_TX_QUEUE_NUM ; i++)
			memcpy(&(in_buf[ALIGN_4(packet_length)*i]), &(in_buf[0]), ALIGN_4(packet_length));

		// TX affairs
		if(TQ_count > 0) {
			if(send_packet(0, fd, fn, in_buf, ALIGN_4(packet_length) * TQ_count) < 0) {
                curr_result = WIFI_TEST_RESULT_FAIL_UNEXPECTED_STOP;
                goto out;
            }

			pktSentCnt += TQ_count;

#if defined(VERBOSE_LOG)
			printf("send %d packets (%d)\n", (int)TQ_count, (int)pktSentCnt);
#endif
			TQ_count = 0;
		}

		RX_count = 0;

		// wait for TX/RX_DONE interrupt and updating TQ0 count by enhanced response
		while(1) {
			rx0TotalLength = rx1TotalLength = 0;

			if(is_irq_pending(fd, &irqTmp) < 0) {
                curr_result = WIFI_TEST_RESULT_FAIL_UNEXPECTED_STOP;
                goto out;
            }
		#if defined(DEBUG)
			if (irqTmp.u4HISR & WHISR_TX_DONE_INT) {
				mylog("tmp irq =%d, u4HISR=%u\n"
					" txInfo.au4WTSR[0]=%u au4WTSR[1]=%u\n", 
					irqTmp.irq, irqTmp.u4HISR, 
					irqTmp.rTxInfo.au4WTSR[0], 
					irqTmp.rTxInfo.au4WTSR[1]);
			} else if ((irqTmp.u4HISR & WHISR_RX0_DONE_INT) ||
				(irqTmp.u4HISR & WHISR_RX1_DONE_INT)) {
				mylog("tmp irq =%d, u4HISR=%u\n"
					" rxInfo.au4WTSR[0]=%u au4WTSR[1]=%u\n", 
					irqTmp.irq, irqTmp.u4HISR, 
					irqTmp.rRxInfo.u.u2NumValidRx0Len, 
					irqTmp.rRxInfo.u.u2NumValidRx1Len);
			} else {
				mylog("ingore u4HISR=%u\n", irqTmp.u4HISR);
			}
		#endif
			//Holmes test
			lseek(g_flag_fd,0,SEEK_SET);
			read(g_flag_fd, buf, 20);   
			sscanf(buf,"%x",&crc_err);
			
			//printf("%d tune: %x\n",completed_packet_num,crc_err);
			
			/* check for termination */
			if(crc_err != 0) {
				curr_result = WIFI_TEST_RESULT_FAIL_MISMATCH_CONTENT;
				goto out;
			}
			//Holmes//


			if(irqTmp.irq == 1) {
				if(irqTmp.u4HISR & 0x1) {
					TQ_count += (irqTmp.rTxInfo.u.ucTQ0Cnt & 0xff) +
							(irqTmp.rTxInfo.u.ucTQ1Cnt & 0xff) +
							(irqTmp.rTxInfo.u.ucTQ2Cnt & 0xff) +
							(irqTmp.rTxInfo.u.ucTQ3Cnt & 0xff) +
							(irqTmp.rTxInfo.u.ucTQ4Cnt & 0xff) +
							(irqTmp.rTxInfo.u.ucTQ5Cnt & 0xff);
				}
				// if RX0_DONE_INT || RX1_DONE_INT
				if(irqTmp.u4HISR & 0x6) {
					uint16_t pktLength;

					// retrive RX0 status
					rx0Num = irqTmp.rRxInfo.u.u2NumValidRx0Len;

					for(i = 0 ; i < rx0Num ; i++) {
						pktLength = irqTmp.rRxInfo.u.au2Rx0Len[i];
						rx0TotalLength += ALIGN_4(pktLength + 4);
						received_bytes += pktLength;
					}

					// retrive RX1 status
					rx1Num = irqTmp.rRxInfo.u.u2NumValidRx1Len;

					for(i = 0 ; i < rx1Num ; i++) {
						pktLength = irqTmp.rRxInfo.u.au2Rx1Len[i];
						rx1TotalLength += ALIGN_4(pktLength + 4);
						received_bytes += pktLength;
					}

					if(rx0TotalLength > 0) {
						if(read_bytes(fd, fn, MCR_WRDR0, 0, buffer, rx0TotalLength + enhanced.totalBytes) < 0) {
                            curr_result = WIFI_TEST_RESULT_FAIL_UNEXPECTED_STOP;
                            goto out;
                        }

						memcpy(&irqTmp2, &buffer[rx0TotalLength], enhanced.totalBytes);

						if(irqTmp2.rTxInfo.u.ucTQ0Cnt > 0) {
							TQ_count += (irqTmp2.rTxInfo.u.ucTQ0Cnt & 0xff) +
									(irqTmp2.rTxInfo.u.ucTQ1Cnt & 0xff) +
									(irqTmp2.rTxInfo.u.ucTQ2Cnt & 0xff) +
									(irqTmp2.rTxInfo.u.ucTQ3Cnt & 0xff) +
									(irqTmp2.rTxInfo.u.ucTQ4Cnt & 0xff) +
									(irqTmp2.rTxInfo.u.ucTQ5Cnt & 0xff);
#if defined(VERBOSE_LOG)
							printf("K");
#endif
						}

						rx0TotalLength = 0;

#if 1
						/* correctness check - based on packet-by-packet basis */
						rxOffset = 0;
						ignoreCount = 0;
						for(i = 0 ; i < rx0Num ; i++) {
							/* identify SLEEPY_NOTIFY packet */
							switch(((P_NIC_HIF_RX_HEADER_T)(&(buffer[rxOffset])))->u2PacketType) {
							case 0: /* HIF_RX_PKT_TYPE_DATA */
								assert(0);
							case 1: /* HIF_RX_PKT_TYPE_EVENT */
#if defined(VERBOSE_LOG)
								printf("ignoring event packet ..\n");
#endif
								ignoreCount++;
								break;
							case 2: /* HIF_RX_PKT_TYPE_TX_LOOPBACK */
								if(tx_compare_packet(in_buf, &buffer[rxOffset], packet_length) != 0) {
                                    curr_result = WIFI_TEST_RESULT_FAIL_MISMATCH_CONTENT;
                                    free(in_buf);
                                    return -1;
								}
								else {
									RX_count++;
									pktRecvCnt++;

                                    /* increase completed packet count */
                                    completed_packet_num++;
								}
								break;
							case 3: /* HIF_RX_PKT_TYPE_MANAGEMENT */
#if defined(VERBOSE_LOG)
								printf("ignoring MMPDU ..\n");
#endif
								ignoreCount++;
								break;
							}
							
							rxOffset += ALIGN_4(irqTmp.rRxInfo.u.au2Rx0Len[i] + 4);
						}
#endif
					}

					if(rx1TotalLength > 0) {
						/* FIXME: not expecting any data coming from port #1 */
						assert(0);

						if(read_bytes(fd, fn, MCR_WRDR1, 0, buffer, rx1TotalLength + enhanced.totalBytes) < 0) {
                            curr_result = WIFI_TEST_RESULT_FAIL_UNEXPECTED_STOP;
                            goto out;
                        }
						memcpy(&irqTmp2, &buffer[rx1TotalLength], enhanced.totalBytes);

						if(irqTmp2.rTxInfo.u.ucTQ0Cnt > 0) {
							TQ_count += irqTmp2.rTxInfo.u.ucTQ0Cnt & 0xff;
#if defined(VERBOSE_LOG)
							printf("K");
#endif
						}

						rx1TotalLength = 0;
					}

#if defined(VERBOSE_LOG)
					printf("received (%d/%d) packets (%d)\n", (int)(rx0Num - ignoreCount), (int)rx1Num, (int)pktRecvCnt);
					printf("TQ count: %d / RQ count: %d\n", TQ_count, RX_count);
#endif
				}

				if(TQ_count == MAX_TX_QUEUE_NUM && RX_count == MAX_TX_QUEUE_NUM) {
					break;
				}
			}
			else {
                if(enable_interrupt(fd, fn) < 0) {
                    curr_result = WIFI_TEST_RESULT_FAIL_UNEXPECTED_STOP;
                    goto out;
                }

				usleep(100*1000);
			}
		}

		if(enable_interrupt(fd, fn) < 0) {
            curr_result = WIFI_TEST_RESULT_FAIL_UNEXPECTED_STOP;
            goto out;
        }

		//Holmes test
		lseek(g_flag_fd,0,SEEK_SET);
		read(g_flag_fd, buf, 20);	  
		sscanf(buf,"%x",&crc_err);
		//printf("tune: %x\n",crc_err);
        /* check for termination */
        if(crc_err != 0) {
            curr_result = WIFI_TEST_RESULT_FAIL_MISMATCH_CONTENT;
            break;
        }
		//Holmes//


        /* check for termination */
        if(pktSentCnt >= packet_num) {
            curr_result = WIFI_TEST_RESULT_PASS;
            break;
        }
	}

out:
    free(in_buf);
	return 0;
}

static void *
entry_mtk_wifi_loopback(void *arg) {
    int fd;
    unsigned int packet_length;
    unsigned int packet_num;
    P_LOOPBACK_PARAM_SET pParamSet = (P_LOOPBACK_PARAM_SET)arg;

    packet_length   = pParamSet->packet_length;
    packet_num      = pParamSet->packet_num;

	g_flag_fd = open("/proc/msdc_tune_flag", O_RDONLY);

    /* initial sequence */
	if((fd = open(MT6620_CHRDEV, O_RDWR)) == -1) {
		myerrorLog(MT6620_CHRDEV"fail, Reason");
	}
    else {
	    if(wifi_hifsys_init(fd)) {
			mylog("wifi_hifsys_init fail\n");
			return NULL;
	    }

    	/* flush TX*/
#if defined(VERBOSE_LOG)
        printf("Flush TX -- start\n");
#endif
    	flush_tx_count(fd);
#if defined(VERBOSE_LOG)
        printf("Flush TX -- completed\n");
#endif

        /* invoke proc */
        proc_mtk_wifi_loopback(fd, packet_length, packet_num);

        /* set flag */
        running_flag = 0;

	    close(fd);
    }
	close(g_flag_fd);

#if defined(ENABLE_PTHREAD)
    pthread_exit(NULL);
#endif
    return NULL;
}


/* module insertion/removal control */
static int insmod(const char *filename, const char *args)
{
    void *module;
    unsigned int size;
    int ret;

    module = load_file(filename, &size);
    if (!module)
        return -1;

    ret = init_module(module, size, args);

    free(module);

    return ret;
}

static int rmmod(const char *modname)
{
    int ret = -1;
    int maxtry = 10;

    while (maxtry-- > 0) {
        ret = delete_module(modname, O_NONBLOCK | O_EXCL);
        if (ret < 0 && errno == EAGAIN)
            usleep(500000);
        else
            break;
    }

    if (ret != 0)
        printf("Unable to unload driver module \"%s\": %s\n",
             modname, strerror(errno));
    return ret;
}

int 
mtk_wifi_loopback(
    unsigned int packet_length,
    unsigned int packet_num
    )
{
#if defined(ENABLE_PTHREAD)
    int rc;
#endif

    /* 1. set proper flags */
    if(running_flag == 0) {
        running_flag = 1;
        completed_packet_num = 0;
        curr_result = WIFI_TEST_RESULT_LOOPBACK_RUNNING;
    }
    else {
		mylog("%s is running already\n", __func__);
        return -1;
    }

    /* 2. create working thread for execution */
    paramSet.packet_length  = packet_length;
    paramSet.packet_num     = packet_num;

#if defined(ENABLE_PTHREAD)
    rc = pthread_create(&thread_instance, NULL, entry_mtk_wifi_loopback, (void *)(&paramSet));
    if(rc) {
    #if defined(VERBOSE_LOG)
        printf("ERROR: pthread_create() returned %d\n", rc);
    #endif
        return -1;
    }
#else
    entry_mtk_wifi_loopback(&paramSet);
#endif

    return 0;
}

ENUM_WIFI_TEST_RESULT_T
mtk_wifi_get_result(
    unsigned int *complete_packet_num
    )
{
    if(complete_packet_num) {
        *complete_packet_num = completed_packet_num;
    }

    return curr_result;
}

int
init_mtk_wifi_loopback(
    void
    )
{
#if _ANDROID_
	int ret = 0;
    int major = -1, minor = 0;
    FILE *fp;
    char buffer[256], buffer2[256], *pivot;

	pr_debug("init wifi loop back step1\n");
    /* 1. force turning-off by rfkill */
    wifi_set_power(0);
	
    /* 2. remove wlan.ko */
#if defined(MT6620)
    rmmod("wlan_mt6620");
#elif defined(MT6628)
	rmmod("wlan_mt6628");
#else
	mylog("bad chip id\n");
	return -22;
#endif
	pr_debug("init wifi loop back step2\n");
    /* 3. install mtk_wifi_loopback.ko */
    insmod("/system/lib/modules/mtk_wifi_loopback.ko", "");
    /* 4. traverse /proc/devices to retrieve major number */
    fp = fopen("/proc/devices", "r");
    if(fp == NULL) {
		mylog("open /jproc/devices fail\n");
        return -22; /*EINVAL*/
    }
    else {
        while(feof(fp) == 0) {
            fgets(buffer, 255, fp);

            pivot = strstr(buffer, "mtk-wifi-loopback");
            if(pivot == NULL) {
                continue;
            }
            else {
                strncpy(buffer2, buffer, pivot - buffer);
                major = atoi(buffer2);
#if defined(VERBOSE_LOG)
                pr_debug("Found mtk-wifi-loopback major number = %d\n", major);
#endif
                break;
            }
        }

        fclose(fp);
    }
    if(major < 0) {
		mylog("major is invalid\n");
        return -22; /*EINVAL*/
    }

    /* 5. create /dev/mt6620 */
    ret = mknod(MT6620_CHRDEV, S_IFCHR | S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH, makedev(major, minor));
	if (ret < 0) {
		mylog("mknode %s fail\n", MT6620_CHRDEV);
		myerrorLog("");
	}
	
    /* 6. force turning-on by rfkill */
    wifi_set_power(1);
	pr_debug("init wifi loop back done\n");
#endif
    sleep(1);

    return 0;
}

int 
uninit_mtk_wifi_loopback(
    void
    )
{
#if _ANDROID_
	pr_debug("uninit wifi loop back step1\n");
    /* 1. force tuning-off by rfkill */
    wifi_set_power(0);
	pr_debug("uninit wifi loop back step2\n");
    /* 2. remove mtk_wifi_loopback.ko */
    rmmod("mtk_wifi_loopback");

    /* 3. install wlan.ko */
    insmod("/system/lib/modules/wlan.ko", "");

    /* 4. remove /dev/mt6620 */
    unlink(MT6620_CHRDEV);
	pr_debug("uninit wifi loop back done\n");
#endif

    return 0;
}


