
#include <getopt.h>
#include  <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <signal.h>

#include <statusd.h>
#include <errno.h>
#include <com_intf.h>
char client_type = MODULE_TYPE_DG;

typedef int (*option_do)(void);

struct rule_option {
	int count;
	char *description;
	option_do operation;
};

static int modify_statusd_loglevel(void);
static int edable_statusd_log2file(void);
static int debugger_help();
static int role_help(void);
static int start_statusd_client(void);
static int stop_statusd_client(void);


struct rule_option debugger[]={
		{1,"modify statusd print level",modify_statusd_loglevel},
		{2,"enable/disable statusd log to file",edable_statusd_log2file},
		{3,"start client",start_statusd_client},
		{4,"stop client",stop_statusd_client},
		{5,"print this help",debugger_help}
};


static int loglevel_help()
{
	printf("0~7. modify log level\n");
	printf("8. query statusd log level\n");
	printf("q.back to upper menu");
	return 0;
}

static int modify_statusd_loglevel(void)
{
	char c;
	char buf[20]={0};
	int ret=0;
	int level;

	loglevel_help();
	while(1){
		c = getchar();
		if(c=='q')
			break;
		else if((c=='?') || (c=='h')){
			loglevel_help();
		}else if ((c=='\r') || (c=='\n')){/*do nothing*/
			continue;
		}else if((c>=0x30) && (c <=0x39)){
			level = c-0x30;
			if(level ==8)
				sprintf(buf,"%s","?");
			else
				sprintf(buf,"%s%d",KEY_DEBUG_LEVEL,level);
			if(strlen(buf)!=0){
				ret = statusd_c2ssend_data(MODULE_TYPE_DG,MODULE_TYPE_SR,buf,strlen(buf));
				if(ret != strlen(buf)){
					printf("CLIENT %s::query statusd debug level failed ret=%d\n",name_inquery(client_type),ret);
				}
			}
			loglevel_help();
		}
	}
	return 0;
}

static int log2file_help()
{
	printf("0.disable statusd log to file\n");
	printf("1.enable statusd log to file\n");
	printf("q.back to upper menu");
	return 0;
}


static int edable_statusd_log2file(void)
{
	char c;
	char buf[20]={0};
	int ret=0;
	int log2file;

	log2file_help();
	while(1){
		c = getchar();
		if(c=='q')
			break;
		else if((c=='?') || (c=='h')){
			log2file_help();
		}else if ((c=='\r') || (c=='\n')){/*do nothing*/
			continue;
		}else if((c>=0x30) && (c <=0x39)){
			log2file = c-0x30;
				sprintf(buf,"%s%d",KEY_LOG2FILE,log2file);
			if(strlen(buf)!=0){
				ret = statusd_c2ssend_data(MODULE_TYPE_DG,MODULE_TYPE_SR,buf,strlen(buf));
				if(ret != strlen(buf)){
					printf("CLIENT %s::query statusd debug level failed ret=%d\n",name_inquery(client_type),ret);
				}
			}
			log2file_help();
		}
	}
	return 0;
}

static int start_statusd_client(void)
{
	char c;
	char buf[20]={0};
	int ret=0;
	int role;

	role_help();
	while(1){
		c = getchar();
		if(c=='q')
			break;
		else if((c=='?') || (c=='h')){
			role_help();
		}else if ((c=='\r') || (c=='\n')){/*do nothing*/
			continue;
		}else if((c>=0x30) && (c <=0x39)){
			role = c-0x30;
			switch(role)
			{
				case MODULE_TYPE_FLS:
					sprintf(buf,"%s",KEY_START_FLS);
					break;
				case MODULE_TYPE_MUX:
					sprintf(buf,"%s",KEY_START_MUX);
					break;
				case MODULE_TYPE_RIL:
					sprintf(buf,"%s",KEY_START_RIL);
					break;
				case MODULE_TYPE_CTC:
					sprintf(buf,"%s",KEY_START_CTC);
					break;
			}
			if(strlen(buf)!=0){
				ret = statusd_c2ssend_data(MODULE_TYPE_DG,MODULE_TYPE_SR,buf,strlen(buf));
				if(ret != strlen(buf)){
					printf("CLIENT %s::query statusd debug level failed ret=%d\n",name_inquery(client_type),ret);
				}
			}
			role_help();
		}
	}
	return 0;
}



static int stop_statusd_client(void)
{
	char c;
	char buf[20]={0};
	int ret=0;
	int role;

	role_help();
	while(1){
		c = getchar();
		if(c=='q')
			break;
		else if((c=='?') || (c=='h')){
			role_help();
		}else if ((c=='\r') || (c=='\n')){/*do nothing*/
			continue;
		}else if((c>=0x30) && (c <=0x39)){
			role = c-0x30;
			switch(role)
			{
				case MODULE_TYPE_FLS:
					sprintf(buf,"%s",KEY_STOP_FLS);
					break;
				case MODULE_TYPE_MUX:
					sprintf(buf,"%s",KEY_STOP_MUX);
					break;
				case MODULE_TYPE_RIL:
					sprintf(buf,"%s",KEY_STOP_RIL);
					break;
				case MODULE_TYPE_CTC:
					sprintf(buf,"%s",KEY_STOP_CTC);
					break;
			}
			if(strlen(buf)!=0){
				ret = statusd_c2ssend_data(MODULE_TYPE_DG,MODULE_TYPE_SR,buf,strlen(buf));
				if(ret != strlen(buf)){
					printf("CLIENT %s::query statusd debug level failed ret=%d\n",name_inquery(client_type),ret);
				}
			}
			role_help();
		}
	}
	return 0;
	return 0;
}


static int debugger_help()
{
	int count = sizeof(debugger)/sizeof(debugger[0]);
	int i;
	for(i=0;i<count;i++)
	{
		printf("%d.%s\t",i,debugger[i].description);
	}
	printf("\n");
	return 0;
}

static int debugger_handler()
{
	int operation=0;
	char c;
	int size = sizeof(debugger)/sizeof(debugger[0]);

	debugger_help();
	while(1){
		c = getchar();

		if(c=='q')
			break;
		else if ((c=='\r') || (c=='\n')){/*do nothing*/
			continue;
		}else if((c>=0x30) && (c <=0x39)){
			operation = c - 0x30;
			if(operation>size){
				printf("unknown selection\n");
				debugger_help();
			}
			else{
				printf("you select :%s\n",debugger[operation].description);
				debugger[operation].operation();
				debugger_help();
			}
		}else{
			printf("unknown selection\n");
			role_help();
		}
	}
	return 0;
}

static int role_help(void)
{
	int i=0;
	for(i=0;i<MODULE_TYPE_COUNT;i++)
	{
		printf("%d.%s\t",i,name_inquery(i));
	}
	printf("%d.help\t",i);
	printf("\n");

	return 0;
}

static int my_callback(char *src_module,char *dst_module,char *data_type,int *data_len,unsigned char *buffer)
{
	int i = 0;

	printf("CLIENT %s::src_module=%d dst_module=%d data_type=%d data_len=%d\n", 
		name_inquery(*src_module),*src_module,*dst_module,*data_type,*data_len);	
	printf("data:");
	for(i=0;i<*data_len;i++){
		printf("[%d]",buffer[i]);
		if(i %10 ==0 && (i!=0))
			printf("\n");
	}
	printf("\n");

	return 0;
}

/* 
* Purpose:  Function responsible by all signal handlers treatment any new signal must be added here
* Input:    param - signal ID
* Return:   -
*/
void stools_signal_treatment(int param)
{
	switch (param)
	{
	    case SIGTERM:
	    case SIGKILL:
			if((client_type>0) &&(client_type<MODULE_TYPE_COUNT)){
				statusd_deregister_cominf(client_type);
				//exit(0);
				break;
				default:
				break;
			}
	}
}

int main(int argc,char *argv[])
{
	int i=0;
	char c;
	int ret=0;
	int com_reg = 0;

	role_help();
	signal(SIGKILL, stools_signal_treatment);
	signal(SIGTERM, stools_signal_treatment);

	while(1){
		c = getchar();
		if(c=='q')
			break;
		else if ((c=='\r') || (c=='\n')){/*do nothing*/
					continue;
		}else if((c>=0x30) && (c <=0x39)){
			client_type = c - 0x30;
			if(client_type>MODULE_TYPE_COUNT){
				printf("unknown selection\n");
				role_help();
			}else if(client_type==MODULE_TYPE_COUNT){
				role_help();
				continue;
			}
			else
				printf("you select :%s\n",name_inquery(client_type));
		}else{
			printf("unknown selection\n");
			role_help();
		}
		if(com_reg ==0 ){
			ret = statusd_register_cominf(client_type,(statusd_data_callback)my_callback);
			if(ret){
				printf("CLIENT %s :: register failed\n",name_inquery(client_type));	
			}
			ret = statusd_c2ssend_cmd(client_type,MODULE_TYPE_SR,CMD_CLIENT_INIT);
			if(ret < 1){
				printf("CLIENT %s::send CMD_CLIENT_INIT cmd failed ret=%d\n",name_inquery(client_type),ret);
			}
			ret = statusd_c2ssend_cmd(client_type,MODULE_TYPE_SR,CMD_CLIENT_READY);
			if(ret < 1){
				printf("CLIENT %s::send CMD_CLIENT_READY cmd failed ret=%d\n",name_inquery(client_type),ret);
			}
			com_reg = 1;
		}

		switch(client_type)
		{
			case MODULE_TYPE_FLS:
				break;
			case MODULE_TYPE_MUX:
				break;
			case MODULE_TYPE_RIL:
				break;
			case MODULE_TYPE_CTC:
				break;
			case MODULE_TYPE_SR:
				break;
			case MODULE_TYPE_DG:
				debugger_handler();
				break;
			default:
				printf("unknown operation\n");
				break;
		}
		role_help();
	}
	
	if(com_reg !=0 ){
		statusd_deregister_cominf(client_type);
	}
	return 0;
}	


