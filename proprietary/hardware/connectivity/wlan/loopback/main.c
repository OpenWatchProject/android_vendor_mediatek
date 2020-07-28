#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include "test_lib.h"
#include "main.h"

#define MAX_BUFFER 1024
#define MSDC_BUFFER 128

#define REQ_CMD_ERR  (0x001)
#define REQ_DAT_ERR  (0x010)
#define REQ_STOP_ERR (0x100)

#define DATA_CONTENT_TUNE 1
#define TUNE_CMD 1
#define DATA_TUNE 1
#define DATA_READ_TUNE 1
#define DATA_WRITE_TUNE 1
// forward decalaration
int main(void) {

#if DATA_CONTENT_TUNE	
	ENUM_WIFI_TEST_RESULT_T testResult;    
	unsigned int pkt_len = 1500;
	unsigned int total_count = 320;
	unsigned int complete_count;
#endif    
	unsigned int sdio_flag = 0xff;

	char buf[MAX_BUFFER];
	char msdc_buf[MSDC_BUFFER];
	int fd,flag_fd,crc_err;
	ssize_t msdc_proc_size;
	int cmd_edge,data_edge,cmd_delay;
	int final_cmd_edge = 0;
	int final_cmd_delay = 0;
	
	int cmd_edge_pass[2]={0};
	int cmd_delay_pass[2][32];
	int data_edge_pass[2]={0};
	int data_delay_pass[2][32];
		
#if DATA_TUNE
	int final_data_edge = 0;		
	int final_read_delay = 0;
	int final_write_delay = 0;

	int read_delay = 0;
	int write_delay = 0;
#endif

	int exit_counter = 0;
	//Clearing the relative variables		
	memset(cmd_delay_pass,0,sizeof(cmd_delay_pass));
	memset(data_delay_pass,0,sizeof(data_delay_pass));

	fd = open("/proc/msdc_tune", O_RDWR);
	flag_fd = open("/proc/msdc_tune_flag", O_RDWR);
	//Enable MSDC tuning
	sprintf(msdc_buf,"%x %x %x\n",0,1,1);
	write(fd,msdc_buf,strlen(msdc_buf));		
	
	data_edge = 0;
	//Tune for Command
	printf("Starting tuning.....Please wait\n");

#if TUNE_CMD		
	for(cmd_edge=0;cmd_edge<2;cmd_edge++)
	{
		mylog("TUNE CMD test, loop=%d of 2\n", cmd_edge+1);
		for(cmd_delay=0;cmd_delay<32;cmd_delay++)
		{
			mylog("set latch edge cmd_dealy=%d\n", cmd_delay);
			//Set latch edge for cmd and data
			sprintf(msdc_buf,"%x %x %x\n",1,cmd_edge,data_edge);
			write(fd,msdc_buf,strlen(msdc_buf));
			//Enable MSDC tuning
			sprintf(msdc_buf,"%x %x %x\n",2,cmd_delay,0);
			write(fd,msdc_buf,strlen(msdc_buf));
    		
			lseek(fd,0,SEEK_SET);
			msdc_proc_size = read(fd, buf, MAX_BUFFER);	   							
			//mylog("%s\n", buf);
			/* 1. initialize */
			init_mtk_wifi_loopback(); 
			/* 4. uninitialize */
			uninit_mtk_wifi_loopback();
    		
    		
			lseek(flag_fd,0,SEEK_SET);                            
			msdc_proc_size = read(flag_fd, buf, MAX_BUFFER);     
			sscanf(buf,"%x",&sdio_flag);
			//mylog("sdio_flag = %x\n",sdio_flag);
			if (0 == (sdio_flag &(REQ_CMD_ERR|REQ_STOP_ERR)) )
			{
				cmd_edge_pass[cmd_edge]=1;
				cmd_delay_pass[cmd_edge][cmd_delay]=1;
			}
		}
	}
#endif
    mylog("list of test result[1: pass, 0: fail]\n");
	for(cmd_edge=0;cmd_edge<2;cmd_edge++)
	{		
		for(cmd_delay=0;cmd_delay<32;cmd_delay++)
		{
			printf("%d ",cmd_delay_pass[cmd_edge][cmd_delay]);
		}
		printf("\n");
	}

	if(cmd_edge_pass[0] == 1)
		final_cmd_edge = 0;
	else if(cmd_edge_pass[1] == 1)
		final_cmd_edge = 1;		
	
	mylog("final cmd edge : %d\n",final_cmd_edge);	

	for(cmd_delay=0;cmd_delay<30;cmd_delay++)
	{
		if( cmd_delay_pass[final_cmd_edge][cmd_delay]==1 && 
		    cmd_delay_pass[final_cmd_edge][cmd_delay+1]==1 && 
		    cmd_delay_pass[final_cmd_edge][cmd_delay+2]==1 ) {
				final_cmd_delay = cmd_delay;			
				break;	
			}									
	}


	for(cmd_delay=31;cmd_delay>1;cmd_delay--)
	{
		if( cmd_delay_pass[final_cmd_edge][cmd_delay]==1 && 
		    cmd_delay_pass[final_cmd_edge][cmd_delay-1]==1 && 
		    cmd_delay_pass[final_cmd_edge][cmd_delay-2]==1 ) {			    
			final_cmd_delay += cmd_delay;				
			break;
		}									
	}
	final_cmd_delay = final_cmd_delay >> 1;
	printf("final cmd delay : %d\n",final_cmd_delay);

#if DATA_READ_TUNE
	mylog("===== DATA READ TUNE TEST=====\n");
	write_delay = 0;		
	for(data_edge=0;data_edge<2;data_edge++)
	{
		for(read_delay=0;read_delay<32;read_delay++)
		{
			exit_counter = 0;
			//Set latch edge for cmd and data
			sprintf(msdc_buf,"%x %x %x\n",1,final_cmd_edge,data_edge);
			write(fd,msdc_buf,strlen(msdc_buf));
			//Enable MSDC tuning
			sprintf(msdc_buf,"%x %x %x\n",2,final_cmd_delay,write_delay);
			write(fd,msdc_buf,strlen(msdc_buf));
			//Write "Read delay"
			sprintf(msdc_buf,"%x %x %x\n",3,read_delay,read_delay);
			write(fd,msdc_buf,strlen(msdc_buf));
		
			//lseek(fd,0,SEEK_SET);
			//msdc_proc_size = read(fd, buf, MAX_BUFFER);
			//printf("%s\n", buf);	   							
    		
			/* 1. initialize */
    		init_mtk_wifi_loopback();

#if DATA_CONTENT_TUNE    			
    			/* 2. start test */
			mylog("SDIO tune setting:\n final_cmd)edge=%d, data_edge=%d\n"
				"final_cmd_dealy=%d\n, write_dealy=%d, read_delay=%d\n",
				final_cmd_edge, data_edge, final_cmd_delay,
				write_delay, read_delay);

			mylog("prepare to start data turn test ..\n");
			mtk_wifi_loopback(pkt_len, total_count);
    		
			mylog("try to retrieve result ..\n");
			/* 3. retrieve result til completion */
	
			do {
				exit_counter++;
				sleep(1);
    		
				testResult = mtk_wifi_get_result(&complete_count);
				mylog("test result:%s [%d/%d]\n", 
					(testResult == 0)? "pass":"fail", 
					complete_count, total_count);
				if(exit_counter > 3) {
					data_edge_pass[data_edge]=0;
					data_delay_pass[data_edge][read_delay]=0;
					break;
				}
    		
			} while(testResult  == WIFI_TEST_RESULT_LOOPBACK_RUNNING);
			mylog("retrieving result completed.\n");
#endif    			
			/* 4. uninitialize */
			uninit_mtk_wifi_loopback();
			lseek(flag_fd,0,SEEK_SET);                            
			msdc_proc_size = read(flag_fd, buf, MAX_BUFFER);      
			sscanf(buf,"%x",&sdio_flag);
			if (0 == sdio_flag)
			{
				data_edge_pass[data_edge]=1;
				data_delay_pass[data_edge][read_delay]=1;
			}			       		    
		}				
	}
	mylog("list of test result[1: pass, 0: fail]\n");
	for(data_edge=0;data_edge<2;data_edge++)
	{		    
		for(read_delay=0;read_delay<32;read_delay++)
		{
			printf("%d ",data_delay_pass[data_edge][read_delay]);
		}
		printf("\n"); 
	}
 
	 if(data_edge_pass[0] == 1)
		final_data_edge = 0;
	else if(data_edge_pass[1] == 1)
		final_data_edge = 1;	   
   
	printf("final data edge : %d\n",final_data_edge); 


	for(read_delay=0;read_delay<30;read_delay++)
   	{
		if( data_delay_pass[final_data_edge][read_delay]==1 && 
			data_delay_pass[final_data_edge][read_delay+1]==1 && 
			data_delay_pass[final_data_edge][read_delay+2]==1 ) {
			final_read_delay = read_delay; 		   
			break;  
		}								   
	}
   
   
	for(read_delay=31;read_delay>1;read_delay--)
   	{
		if( data_delay_pass[final_data_edge][read_delay]==1 && 
			data_delay_pass[final_data_edge][read_delay-1]==1 && 
			data_delay_pass[final_data_edge][read_delay-2]==1 ) {			   
			final_read_delay += read_delay;			   
			break;
	  	}								   
   	}

	final_read_delay = final_read_delay >> 1;
	printf("final read delay : %d\n",final_read_delay);
#endif // #if DATA_READ_TUNE

	memset(data_delay_pass,0,sizeof(data_delay_pass));

#if DATA_WRITE_TUNE
	mylog("===== DATA WRITE TUNE TEST=====\n");
	for(data_edge=0;data_edge<2;data_edge++)
	{
		for(write_delay=0;write_delay<32;write_delay++)
		{
			exit_counter = 0;
			//Set latch edge for cmd and data
			sprintf(msdc_buf,"%x %x %x\n",1,final_cmd_edge,data_edge);
			write(fd,msdc_buf,strlen(msdc_buf));
			//Enable MSDC tuning
			sprintf(msdc_buf,"%x %x %x\n",2,final_cmd_delay,write_delay);
			write(fd,msdc_buf,strlen(msdc_buf));
			//Write "Read delay"
			sprintf(msdc_buf,"%x %x %x\n",3,final_read_delay,final_read_delay);
			//sprintf(msdc_buf,"%x %x %x\n",3,0,0);
			write(fd,msdc_buf,strlen(msdc_buf));
		
			//lseek(fd,0,SEEK_SET);
			//msdc_proc_size = read(fd, buf, MAX_BUFFER);
			//printf("%s\n", buf);								
			
			/* 1. initialize */
			init_mtk_wifi_loopback();
		
#if DATA_CONTENT_TUNE
				/* 2. start test */
			mylog("SDIO tune setting:\n final_cmd)edge=%d, data_edge=%d\n"
				"final_cmd_dealy=%d\n, write_dealy=%d, read_delay=%d\n",
				final_cmd_edge, data_edge, final_cmd_delay,
				write_delay, read_delay);
			mylog("prepare to start test ..\n");
			mtk_wifi_loopback(pkt_len, total_count);
			
			printf("try to retrieve result ..\n");
			/* 3. retrieve result til completion */
		
			do {
				sleep(1);
				exit_counter++;
			
				//Holmes test
				lseek(flag_fd,0,SEEK_SET);
				read(flag_fd, buf, 20);   
				sscanf(buf,"%x",&crc_err);
				//printf("%d tune: %x\n",completed_packet_num,crc_err);
				
				/* check for termination */
				if(crc_err != 0) {
					//testResult = WIFI_TEST_RESULT_FAIL_MISMATCH_CONTENT;
					break;
				}
				//Holmes//
				testResult = mtk_wifi_get_result(&complete_count);
				mylog("test result:%s [%d/%d]\n", 
					(testResult == 0)? "pass":"fail", 
					complete_count, total_count);
				if(exit_counter > 3) {
					data_delay_pass[data_edge][write_delay]=0;
					break;
				}
			
			} while(testResult	== WIFI_TEST_RESULT_LOOPBACK_RUNNING);
			mylog("retrieving result completed.\n");
#endif
			/* 4. uninitialize */
			uninit_mtk_wifi_loopback();
			lseek(flag_fd,0,SEEK_SET);							 
			msdc_proc_size = read(flag_fd, buf, MAX_BUFFER);	  
			sscanf(buf,"%x",&sdio_flag);
			if (0 == sdio_flag )
			{
				data_delay_pass[data_edge][write_delay]=1;
			}							
		}				
	}
#if 0
	for(data_edge=0;data_edge<2;data_edge++)
	{
		printf("%d",data_edge_pass[cmd_edge]);									
	}
	printf("\n");
#endif
	for(data_edge=0;data_edge<2;data_edge++)
	{
		for(write_delay=0;write_delay<32;write_delay++)
		{
			printf("%d ",data_delay_pass[data_edge][write_delay]);
		}
		printf("\n"); 
	}
#if 0	 
	 if(data_edge_pass[0] == 1)
		final_data_edge = 0;
	else if(data_edge_pass[1] == 1)
		final_data_edge = 1;	   
	
	printf("final data edge : %d\n",final_data_edge); 
#endif	
	
	for(write_delay=0;write_delay<30;write_delay++)
	{
		if( data_delay_pass[final_data_edge][write_delay]==1 && 
			data_delay_pass[final_data_edge][write_delay+1]==1 && 
			data_delay_pass[final_data_edge][write_delay+2]==1 ) {
			final_write_delay = write_delay;		   
			break;	
		}								   
	}
	
	
	for(write_delay=31;write_delay>1;write_delay--)
	{
		if( data_delay_pass[final_data_edge][write_delay]==1 && 
			data_delay_pass[final_data_edge][write_delay-1]==1 && 
			data_delay_pass[final_data_edge][write_delay-2]==1 ) {			   
			final_write_delay += write_delay; 		   
			break;
		}								   
	}
	
	final_write_delay = final_write_delay >> 1;
	printf("final write delay : %d\n",final_write_delay);
#endif

    
	close(flag_fd);
	close(fd);
	return 0;
}
