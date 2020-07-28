#include <stdlib.h>   
#include <sys/wait.h>   
#include <sys/types.h> 
#include <cutils/log.h> 
#include <sys/capability.h>
#include <cutils/properties.h>
#ifndef LOG_TAG 
#define LOG_TAG "LaunchPPPOE"
#endif


#define LOGD(...)  __android_log_print(3,LOG_TAG,__VA_ARGS__)


int main(argc, argv)
	int argc;
	char **argv;
{
 char PPPOECMDLINE[320];
 pid_t status;
 char ExitCode[8];  
 
 if(argc <9)
 	LOGD("prameters error \n");
 	
 snprintf(PPPOECMDLINE, sizeof(PPPOECMDLINE), "/system/bin/pppd pty \"/system/bin/pppoe -p /etc/ppp/pppoe.pid -I %s -T 80 -U -m %s \" noipdefault noauth default-asyncmap hide-password nodetach usepeerdns mtu %s mru %s noaccomp nodeflate nopcomp novj novjccomp user %s password %s lcp-echo-interval %s lcp-echo-failure %s" ,argv[1],argv[2],argv[3],argv[4],argv[5],argv[6],argv[7],argv[8]);

#ifdef INIT_ENG_BUILD	
 LOGD("Launch CMD: %s\n", PPPOECMDLINE);
#endif

  status = system(PPPOECMDLINE);
  
	snprintf(ExitCode,sizeof(ExitCode),"%d",0);
	
	if (-1 == status)  
    {  
        LOGD("system error(folk error)!");
        snprintf(ExitCode,sizeof(ExitCode),"%d",-1); 
        property_set ("net.pppoe.status",ExitCode);        
        exit(-1); 
    }  
    else  
    {  
        LOGD("exit status value = [0x%x]\n", status);  
        //launch cmd successfully
        if (WIFEXITED(status))  
        {  
            if (0 == WEXITSTATUS(status))  
            {  
                 LOGD("run cmd  successfully.\n");
                 snprintf(ExitCode,sizeof(ExitCode),"%d",0); 
                 property_set ("net.pppoe.status",ExitCode); 
                 exit(0);
            }  
            else  
            {  
                LOGD("run cmd fail, script exit code: %d\n", WEXITSTATUS(status)); 
                snprintf(ExitCode,sizeof(ExitCode),"%d",WEXITSTATUS(status)); 
                 property_set ("net.pppoe.status",ExitCode); 
                exit(WEXITSTATUS(status)); 
            }  
        }  
        else  
        {  
            LOGD("launch cmd fail = [%d]\n", WEXITSTATUS(status)); 
            snprintf(ExitCode,sizeof(ExitCode),"%d",-1); 
            property_set ("net.pppoe.status",ExitCode);  
            exit(-1); 
        }  
    }  
    
	return 0;
}
