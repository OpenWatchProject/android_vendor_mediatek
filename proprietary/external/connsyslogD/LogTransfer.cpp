#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
//#include <sys/stat.h>
//#include <sys/statfs.h>
#include <unistd.h>
#include <cutils/properties.h>
#include <dirent.h>


#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <termios.h>
#include "ConsysLog.h"
#include "LogWriter.h"
#include "Utils.h"


/////////////////////////////////////////

typedef struct {
    char file_name[64];
    unsigned int file_len;
} FILE_HEADER;

#define FULL_DIR_FILE "%s/%s"
#define LOG_BUFFER_SIZE         (1024*256)

FILE_HEADER file_header_info;
strList m_logFilelist;
strListIter mFileIterator;

char pre_fix[16]={0};
int file_count = 0;
char post_fix[16]={0};
int usb_port_fd = -1;
char m_DataLogBuf[LOG_BUFFER_SIZE]={0};
ATM_ONLINE_MODE transfer_mode = ATM_USB_MODE;

///////wifi support
bool m_bConnect = false;
int m_nSockFd = -1;
int m_nClientFd = -1;
static const int WIFI_SOCKET_PORT = 10005;
static const int BACKLOG = 32;
int log_port = -1;
//bool m_waitingConnect = false;

void closeUSBPort() {
    int err;
    LOGD("begin closeUSBPort");
    if (usb_port_fd != -1) {
        err = close(usb_port_fd);
        if (err != 0) {
            LOGW("closeUSBPort: close err=%d", err);
        }
        usb_port_fd = -1;
        LOGD(" close USB port fd.");
    }
}

bool openUSBPort() {
    closeUSBPort();
    const char *vcom_dev_name;
    vcom_dev_name = "/dev/ttyGS3";

    usb_port_fd = open(vcom_dev_name, O_RDWR | O_NOCTTY);
    LOGI("open USB dev %s", vcom_dev_name);

    if (usb_port_fd == -1) {
        LOGE("Cannot open USB fd, err=%d", errno);
        return false;
    }

    struct termios termOptions;
    fcntl(usb_port_fd, F_SETFL, 0);
    tcgetattr(usb_port_fd, &termOptions);

    // Raw mode
    termOptions.c_iflag &= ~(INLCR | ICRNL | IXON | IXOFF | IXANY);
    termOptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); /*raw input*/
    termOptions.c_oflag &= ~OPOST; /*raw output*/

    tcflush(usb_port_fd, TCIFLUSH); //clear input buffer
    termOptions.c_cc[VTIME] = 1; /* inter-character timer unused */
    termOptions.c_cc[VMIN] = 0; /* blocking read until 0 character arrives */

    tcsetattr(usb_port_fd, TCSANOW, &termOptions);
    return true;
}

bool sendLogDataToPC(const char *data, unsigned int len) {
    int bytes_left = (int) len;
    char *data_ptr = (char *) data;
    int bytes_written;

//    LOGV("=====[Send MD packet to PC tool, dataLen=%uB]=====", len);

    while (bytes_left > 0 ) {
        bytes_written = write(log_port, data_ptr, bytes_left);

        if (bytes_written == -1) {
            if (errno == EAGAIN) {
                LOGE("sendLogDataToPC: EAGAIN");
                return true;
            }
            LOGE("sendLogDataToPC: write failed, errno=%d, len=%d, left=%d", errno, len, bytes_left);
            if (errno == 5 || errno == 9) {
               if (transfer_mode == ATM_USB_MODE) {
                    openUSBPort();
            }
         }
            return false;
        }
        data_ptr += bytes_written;
        bytes_left -= bytes_written;
    }
    return true;
}

bool TransferFile(char* logPath,const char* fileName){
    ///////init file header
    memset(file_header_info.file_name,0,64);
    file_header_info.file_len = 0;
    FILE *fp = NULL;
    char full_path[256];
    memset(full_path,0,256);
    sprintf(full_path,FULL_DIR_FILE,logPath,fileName);
    LOGI("file be pulled is : %s", full_path);
    fp = fopen(full_path, "rb");
    if (fp != NULL) {
        int ret = fseek(fp, 0, SEEK_END);
        if (ret == -1) {
            LOGE("fseek end file error:%s",strerror(errno));
            fclose(fp);
            return false;
        }
        int file_size = ftell(fp);
        if (file_size == -1) {
            LOGE("ftell file error: %s",strerror(errno));
            fclose(fp);
            return false;
        }
        strncpy(file_header_info.file_name, fileName, sizeof(file_header_info.file_name) - 1);
        file_header_info.file_name[sizeof(file_header_info.file_name) - 1] = '\0';
        file_header_info.file_len = file_size;
        // send file_header data
        sendLogDataToPC((char*)(&file_header_info), sizeof(file_header_info));
        ret = fseek(fp, 0, SEEK_SET);
        if (ret == -1) {
            LOGE("fseek set file error: %s",strerror(errno));
            fclose(fp);
            return false;
        }
        while(!feof(fp)){
            memset(m_DataLogBuf,0,LOG_BUFFER_SIZE);
            ssize_t read_len = fread(m_DataLogBuf, sizeof(char),LOG_BUFFER_SIZE,fp);
            if (-1 == read_len) {
                LOGE("Failed to read data from  file, errno=%d", errno);
                fclose(fp);
                return false;
            } else if (read_len > 0) {
                sendLogDataToPC(m_DataLogBuf, read_len);
            } else {
             LOGE(" read data from  file, size = 0 .errno=%d", errno);
            }
        }
        fclose(fp);
    } else {
        LOGE("Failed to open file %s, errno=%d", full_path,errno);
        return false;
    }
    return true;
}
void socketClose();

//void disconnect();
void WaitConnect();
bool sendSourceFile(char* folder_path,ATM_ONLINE_MODE mode){
    if (mode == ATM_WIFI_MODE) {
        WaitConnect();
    }
    LOGD("sendSourceFile log folder: %s", folder_path);
    DIR* dp = NULL;
    m_logFilelist.clear();
    transfer_mode = mode;
    struct dirent* dirp = NULL;
    dp = opendir(folder_path);
    if (dp == NULL) {
        LOGE("can not open log folder: %s", folder_path);
        property_set("vendor.pullfwlog","2");
     if (mode == 0) {
        socketClose();
     }
        return false;
    }
    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, "..") == 0 || strcmp(dirp->d_name, ".") == 0) {
             continue;
        }
        if (strstr(dirp->d_name,".clog") != NULL) {
            m_logFilelist.push_back(dirp->d_name);
        }
        if (strstr(dirp->d_name,".clog.curf") != NULL) {
            m_logFilelist.push_back(dirp->d_name);
        }
    }
    closedir(dp);
    ///////begin transfer files
    if (mode == ATM_USB_MODE){
        openUSBPort();
        log_port = usb_port_fd;
    }else {
        WaitConnect();
        log_port = m_nClientFd;
    }
    LOGD("mode = %d,log_port = %d", mode,log_port);

    LOGI("total files be pulled is : %u", (unsigned int)m_logFilelist.size());
    memset(pre_fix,0,16);
    strncpy(pre_fix, "FWlog_start", sizeof(pre_fix) - 1);
    pre_fix[sizeof(pre_fix) - 1] = '\0';
    file_count = m_logFilelist.size();

   /////send prefix
   sendLogDataToPC(pre_fix,sizeof(pre_fix));
   sendLogDataToPC((char*)&file_count,sizeof(file_count));
   for (mFileIterator = m_logFilelist.begin();mFileIterator != m_logFilelist.end(); mFileIterator++) {
        string ws = *mFileIterator;
        const char* fileName = ws.c_str();
       TransferFile(folder_path,fileName);
    }
    memset(post_fix,0,16);
    strncpy(post_fix, "FWlog_end", sizeof(post_fix) - 1);
    post_fix[sizeof(post_fix) - 1] = '\0';
    sendLogDataToPC(post_fix,sizeof(post_fix));
     if (mode == ATM_USB_MODE)
        closeUSBPort();
    else
        socketClose(); // close cocket
    property_set("vendor.pullfwlog","1");
    //////end transfer
    return true;
}
//////////////////////////WIFI code begin

signed int createSocket()
{
 //   int ret = -1;
    int sock_opt = 1;
    int enable = 1;
    if (m_nSockFd != -1) {
        LOGE("connsyslogger Wifi Socket already created.");
        return m_nSockFd;
    }
    //Create socket
    if((m_nSockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        LOGE("connsyslogger Wifi Socket created fail. errno=%d", errno);
        return -1;
    }

    LOGI("connsyslogger Wifi Socket created success m_nSockFd:%d",m_nSockFd);

    // SET SOCKET REUSE Address
    if(setsockopt(m_nSockFd, SOL_SOCKET, SO_REUSEADDR, (void*)&sock_opt, sizeof(sock_opt)) < 0)
    {
        LOGE("connsyslogger Wifi Socket setsockopt failed. errno=%d", errno);
        close(m_nSockFd);
        return -1;
    }

    //SET TCP_NODELAY
    if(setsockopt(m_nSockFd, IPPROTO_TCP, TCP_NODELAY, (void*)&enable, sizeof(enable)) < 0)
    {
        LOGE("connsyslogger Wifi Socket setsockopt TCP_NODELAY failed. errno=%d", errno);
    }
    //Prepare the sockaddr_in structure
    struct sockaddr_in* serverAddr = new struct sockaddr_in;
    if(serverAddr == NULL)
    {
        LOGE("connsyslogger Wifi Socket new server addr failed. errno=%d", errno);
        close(m_nSockFd);
        m_nSockFd =  -1;
        return -1;
    }

    serverAddr->sin_family = AF_INET;
    serverAddr->sin_port = htons(WIFI_SOCKET_PORT);
    char strIpAddr[128] = {0};
    property_get(PROP_WIFI_ADDR,strIpAddr,"0.0.0.0");
    LOGD("connsyslogger Wifi Socket IP Addr:%s", strIpAddr);
    serverAddr->sin_addr.s_addr = inet_addr(strIpAddr);
    bzero(&(serverAddr->sin_zero),8);
    //Bind
    if(::bind(m_nSockFd,(struct sockaddr*)serverAddr, sizeof(struct sockaddr)) < 0)
    {
        LOGE("connsyslogger Wifi Socket bind failed. errno=%d", errno);
        goto errout;
    }
    LOGI("connsyslogger Wifi Socket bind done");

    //Listen
    if (listen(m_nSockFd, BACKLOG) == -1)
    {
        LOGE("connsyslogger Wifi Socket Failed to listen Socket port, errno=%d", errno);
        goto errout;
    }
    LOGI("connsyslogger Wifi Socket listen done");
    delete serverAddr;
    serverAddr = NULL;
    return m_nSockFd;

errout:
    delete serverAddr;
    serverAddr = NULL;
    close(m_nSockFd);
    m_nSockFd = -1;
    return -1;
}

void disconnect()
{
    if (m_bConnect)
    {
        if (m_nClientFd != -1)
        {
            m_bConnect = false;
            close(m_nClientFd);
            m_nClientFd = -1;
            LOGD("connsyslogger wifi Socket disconnect client");
        }
    } else{ 
     LOGD("connsyslogger wifi Socket already disconnect client");
    }
}

int connect()
{
    if (-1 == m_nSockFd)
    {
        LOGE("connsyslogger wifi Socket server not create");
        return -1;
    }

    if (m_bConnect)
    {
        LOGD("connsyslogger wifi Socket already conencted");
        return 0;
    }

    struct sockaddr_in* clientAddr = new struct sockaddr_in;
    if (clientAddr == NULL)
    {
        LOGE("connsyslogger wifi Socket new client addr failed. errno=%d", errno);
        m_bConnect = false;
        return -1;
    }
    memset(clientAddr,0,sizeof(struct sockaddr_in));
    socklen_t alen = sizeof(struct sockaddr);

    while(1)
    {

        LOGI("connsyslogger wifi Socket connect, accept the connection........");
        if ((m_nClientFd = accept(m_nSockFd, (struct sockaddr*)clientAddr, &alen)) == -1)
        {
            LOGE("connsyslogger Socket accept error, errno=%d,m_nSockFd = %d", errno,m_nSockFd);

            if(errno == EAGAIN)
            {
                usleep(200*1000);
                continue;
            }
            m_bConnect = false;
            delete clientAddr;
            clientAddr = NULL;
            return -1;
        }
        else
        {
            m_bConnect = true;
            LOGI("connsyslogger Socket connect, Received a connection from %s, m_nClientFd = %d",
                (char*)inet_ntoa(clientAddr->sin_addr), m_nClientFd);
            delete clientAddr;
            clientAddr = NULL;
            return 0;
        }
    }
}

void socketClose()
{
    disconnect();
    if (m_nSockFd != -1)
    {
        close(m_nSockFd);
        m_nSockFd = -1;
    }
}

void WaitConnect(){
    if (m_bConnect) {
        LOGE("connsyslogger WaitConnect, already connected");
        return;
    }
    fd_set rfds;
    while (1) {
        FD_ZERO(&rfds);
        if(m_nSockFd > 0) {
            FD_SET(m_nSockFd, &rfds);
         }
        int maxfd = 0;
        int rc = 0;
        maxfd = m_nSockFd+1;
        if( (rc = select(maxfd, &rfds, NULL, NULL, NULL)) < 0) {
            LOGE("WaitConnect select failed (%s), errno = %d", strerror(errno), errno);
            sleep(1);
            continue;
        }
        if(FD_ISSET(m_nSockFd, &rfds)){
            LOGD("begin to accept client connection socketFd=%d",m_nSockFd);
            if (connect() == -1){
                LOGD("accept client connection failed.");
                sleep(1);
                continue;
            }
            LOGD("accept client connection done.");
            break;
        }
   }
    return ;
}
    
/*
return:
 0: WIFI
 1: USB
*/
//////////////////////////WIFI code end

ATM_ONLINE_MODE checkConnectType() {
 char connectType[128] = {0};
 property_get(PROP_META_CONN_TYPE,connectType,"");
 LOGD("connectType:%s",connectType);
 if (!strncmp("wifi", connectType, strlen("wifi"))) {
    return ATM_WIFI_MODE;
 }
 return ATM_USB_MODE;
}

