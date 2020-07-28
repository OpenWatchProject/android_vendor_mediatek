#include <cutils/sockets.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "Context.h"
#include "LogDefine.h"
#include "MSocket.h"

#define DATA_LEN	256

MSocket::MSocket()
{
	m_serverID = -1;
	m_clientID = -1;
	m_threadID = -1;
	m_stop = 0;
	memset(&m_thread, 0, sizeof(pthread_t));
	m_type = SOCKET_END;
	m_bClientConnected = false;
	
	signal(SIGPIPE,SIG_IGN);
	
}

MSocket::~MSocket(void)
{
	deinit();
}

int MSocket::initServer(const char * socket_name, int namespaceId, int bListen)
{
	META_LOG("[META][Socket] To Create Socket Server:(%s)", socket_name);
	
	m_serverID = socket_local_server(socket_name, namespaceId, SOCK_STREAM);
	
	META_LOG("[Meta][Socket] m_serverID = %d errno = %d", m_serverID, errno);	

	listen(m_serverID,4);

	if(bListen)
	{
		m_threadID = pthread_create(&m_thread, NULL, ThreadFunc,  this);
		if(m_threadID)
		{
			META_LOG("[Meta][Socket] Failed to create socket thread!");
			return 0;
		}
	}
	
	return 1;	
		
}

int MSocket::initClient(const char * socket_name, int namespaceId, int bListen)
{
	int count = 0;
	int val = 0;
	
	META_LOG("[Meta][Socket] To connect server:(%s)", socket_name);
	while(m_clientID < 0) 
	{
		count++;
		m_clientID = socket_local_client(socket_name, namespaceId, SOCK_STREAM);
		META_LOG("[Meta][Socket] init client m_clientID = %d", m_clientID);
        META_LOG("[Meta][Socket] errno = %d, string = %s", errno, strerror(errno));
		usleep(200*1000);
		if(count == 5)
			return 0;		
	}

	META_LOG("[Meta][Socket] connect successful");
	//if bListen is true, we will create thread to read socket data.
	if(bListen)
	{
		m_threadID = pthread_create(&m_thread, NULL, ThreadFunc,  this);
		if(m_threadID)
		{
			META_LOG("[Meta][Socket] Failed to create socket thread!");
			return 0;
		}
	}
	
	if(0 == setsockopt(m_clientID, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val)))
	{
		META_LOG("[Meta][Socket] set socket to option to TCP_NODELAY!");
	}
	
	return 1;
}

void MSocket::deinit()
{
	if(m_threadID == 0)
	{
		m_stop = 1;
		pthread_join(m_thread, NULL);
	}

    if (m_clientID > 0)
    {
       	close (m_clientID);
        m_clientID = -1;
    }

	if (m_serverID > 0)
    {
       	close (m_serverID);
        m_serverID = -1;
    }
}

int MSocket::connect()
{
	if(m_bClientConnected)
		return 0;

	while((m_clientID = accept(m_serverID, NULL, NULL)) == -1)
	{
		if(errno == EAGAIN)
		{
			usleep(100*1000);
			continue;
		}
		META_LOG("[Meta][Socket] Socket accept error, errno=%d", errno);
		return -1;
	}
	META_LOG("[Meta][Socket] Accept client connection, m_clientID = %d", m_clientID);
	m_bClientConnected = true;
	
	return 0;
}


void MSocket::disconnect()
{
	if (m_bClientConnected)
	{
		if (m_clientID != -1)
		{
			META_LOG("[Meta][Socket] Disconnect client connection, m_clientID = %d", m_clientID);
			close(m_clientID);
			m_clientID = -1;
			m_bClientConnected = false;
		}
    }
}


void MSocket::send_msg(const char *msg)
{
	int nWritten = 0;

	META_LOG("[Meta][Socket] send mssage (%s) - socket id = %d", msg,  m_clientID);

	if((nWritten = write(m_clientID, msg, strlen(msg))) < 0)
	{
		META_LOG("[Meta][Socket] socket write error: %s", strerror(errno));
	}
	else
	{
		META_LOG("[Meta][Socket] write %d Bytes, total = %zd", nWritten, strlen(msg));
	}
}

void *MSocket::ThreadFunc(void *p)
{
	MSocket *pSocket = (MSocket *)p;
	if(pSocket != NULL)
		pSocket->wait_msg();
	
	return NULL;
}

//////////////////////////////////////////////MATCIClientSocket////////////////////////////////////////////////////

MATCIClientSocket::MATCIClientSocket()
{

}

MATCIClientSocket::MATCIClientSocket(SOCKET_TYPE type)
{
	m_type = type;
}

MATCIClientSocket::~MATCIClientSocket()
{

}

void MATCIClientSocket::wait_msg()
{
	const char *msg = "calibration";
	char data[DATA_LEN] = {0};
	int  len = 0;
	
	META_LOG("[Meta][MATCIClientSocket] wait_msg m_clientID = %d", m_clientID);

	while(m_stop == 0)
	{
		if(!m_bClientConnected)
		{
			if(-1 == connect())
				continue;
		}
		memset(data, 0, DATA_LEN);	
		len = read(m_clientID, data, DATA_LEN);
		
		if(len >0)
		{
            if(len == DATA_LEN)
				data[len-1] = 0;
			else
				data[len] = 0;
			
			META_LOG("[Meta][MATCIClientSocket] data len = %d, rawdata = (%s)", len, data);
			char *pos = strstr(data, msg);
			if(pos != NULL)
			{
				createSerPortThread();
				createAllModemThread();
				continue;
			}
		}
		else if(0 == len)
		{	
			META_LOG("[Meta][MATMSocket] Socket connection lost, need to reconnect");
			disconnect();
		}
		else
		{
			usleep(100000); // wake up every 0.1sec   
		}
	}
	return;
}

//////////////////////////////////////////////MATCIServerSocket////////////////////////////////////////////////////

MATCIServerSocket::MATCIServerSocket()
{

}

MATCIServerSocket::MATCIServerSocket(SOCKET_TYPE type)
{
	m_type = type;
}

MATCIServerSocket::~MATCIServerSocket()
{

}

void MATCIServerSocket::wait_msg()
{
	const char *msg = "EMETACFG";
	const char *msg_ok = "OK";
	char data[DATA_LEN] = {0};
	int len = 0;
	
	META_LOG("[Meta][MATCIServerSocket] wait_msg m_clientID = %d", m_clientID);

	while(m_stop == 0)
	{
		memset(data, 0, DATA_LEN);				
		len = read(m_clientID, data, DATA_LEN);

		if(len > 0)
		{
			if(len == DATA_LEN)
				data[len-1] = 0;
			else
				data[len] = 0;
						
			META_LOG("[Meta][MATCIServerSocket] data len = %d, rawdata = %s", len, data);
			
			char *pos = strstr(data, msg);
			if(pos != NULL)
			{
				pos += 10; //reply format is +EMETACFG: X, the X is modem mode
				int mdmode = atoi(pos);  //Modem side : meta=0, normal=1
				if(0 == mdmode) 
					setCurrentMdMode(2); //AP side: normal=1; meta = 2;
				else if(1 == mdmode) 
					setCurrentMdMode(1);
			
				continue;
			}

			pos = strstr(data, msg_ok);
			if(pos != NULL)
			{
				META_LOG("[Meta][MATCIServerSocket] got OK from modem");
				if(getATRespFlag()==1)
				{
					META_LOG("[Meta][MATCIServerSocket] setATRespFlag to 0");
					setATRespFlag(0);
				}
				continue;
			}
			setATRespFlag(-1);
		}
		else
		{
			usleep(100*1000); // wake up every 0.1sec
		}
	}
	
	return;
}

//////////////////////////////////////////////MLogSocket////////////////////////////////////////////////////

MLogSocket::MLogSocket()
{
	m_mdlogpulling = -1;
	m_mblogpulling = -1;
	m_connsyslogpulling = -1;
	m_mddbpulling = -1;
	m_Mutex = PTHREAD_MUTEX_INITIALIZER;
}

MLogSocket::MLogSocket(SOCKET_TYPE type)
{
	m_type = type;
	m_mdlogpulling = -1;
	m_mblogpulling = -1;
	m_connsyslogpulling = -1;
	m_mddbpulling = -1;
	m_Mutex = PTHREAD_MUTEX_INITIALIZER;
}

MLogSocket::~MLogSocket()
{

}

void MLogSocket::wait_msg()
{
	char data[DATA_LEN] = {0};
	int  len = 0;

	META_LOG("[Meta][MLogSocket] wait_msg m_clientID = %d", m_clientID);

	while(m_stop == 0)
	{
		memset(data, 0, DATA_LEN);				
		len = read(m_clientID, data, DATA_LEN);

		if(len > 0)
		{
		    if(len == DATA_LEN)
				data[len-1] = 0;
			else
				data[len] = 0;
			
			META_LOG("[Meta][MLogSocket] m_clientID = %d, data len = %d, rawdata = (%s), cmd = (%s)", m_clientID, len, data, m_strCmd.c_str());
			
			if(strstr(data, MDLOG_PULL_START) != NULL)
			{
				setLogPullingStatus(0, 1);
				META_LOG("[Meta][MLogSocket] modem log pull done");
			}
			else if(strstr(data, MBLOG_PULL_START) != NULL)
			{
				setLogPullingStatus(1, 1);
				META_LOG("[Meta][MLogSocket] mobile log pull done");
			}
			else if(strstr(data, CONNLOG_PULL_START) != NULL)
			{
				setLogPullingStatus(3, 1);
				META_LOG("[Meta][MLogSocket] connsys log pull done");
			}
			else if(strstr(data, MDDB_PULL_START) != NULL)
			{
				setLogPullingStatus(4, 1);
				META_LOG("[Meta][MLogSocket] mddb pull done");
			}
			
			if(m_strCmd.size() > 1)
			{
				if(strstr(data, m_strCmd.c_str()) != NULL)
				{
					pthread_mutex_lock(&m_Mutex);
					m_strRsp = string(data);
					m_strCmd = "";
					META_LOG("[Meta][MLogSocket] wait_msg response = (%s)", m_strRsp.c_str());
					pthread_mutex_unlock(&m_Mutex);
				}
			}
		}
		else
		{		
			usleep(100000); // wake up every 0.1sec   
		}		
	}
	return;
}

int MLogSocket::recv_rsp(char *buf)
{
	META_LOG("[Meta][MLogSocket] recv_rsp begin");
	int count = 0;
	while(1)
	{
		pthread_mutex_lock(&m_Mutex);
		if(m_strRsp.length() > 0)
		{
			string strRsp = m_strRsp.erase(0, m_strRsp.find_last_of(',')+1);
			if(strRsp.length() > 0)
				strncpy(buf, strRsp.c_str(), strRsp.length());
			META_LOG("[Meta][MLogSocket] recv_rsp response = (%s)", buf);
			m_strRsp = "";
			pthread_mutex_unlock(&m_Mutex);
			return true;
		}
		pthread_mutex_unlock(&m_Mutex);
		usleep(100000);
		if (++count == 45)
		{
			pthread_mutex_lock(&m_Mutex);
			META_LOG("[Meta][MLogSocket] recv_rsp end-false");
			m_strCmd = "";
			pthread_mutex_unlock(&m_Mutex);
			return false;
		}
	}

	META_LOG("[Meta][MLogSocket] recv_rsp end-true");
	return true;
}


void MLogSocket::send_msg(const char *msg, bool ignore)
{
	int nWritten = 0;

	META_LOG("[Meta][MLogSocket] send mssage (%s) - socket id = %d", msg,  m_clientID);

	if(ignore == false)
	{
		pthread_mutex_lock(&m_Mutex);
		m_strCmd = string(msg);
		pthread_mutex_unlock(&m_Mutex);
	}
	
	if((nWritten = write(m_clientID, msg, strlen(msg))) < 0)
	{
		m_strCmd = "";
		META_LOG("[Meta][MLogSocket] socket write error: %s", strerror(errno));
	}
	else
	{
		META_LOG("[Meta][MLogSocket] write %d Bytes, total = %zd", nWritten, strlen(msg));
	}
}

int MLogSocket::getLogPullingStatus(int type)
{
	int status = -1;
	switch(type)
	{
		case 0:
			status = m_mdlogpulling;
			break;
		case 1:
			status = m_mblogpulling;
			break;
		case 3:
			status = m_connsyslogpulling;
			break;
		case 4:
			status = m_mddbpulling;
			break;
		default:
			break;
	}

	return status;
}

void MLogSocket::setLogPullingStatus(int type, int value)
{	
	pthread_mutex_lock(&m_Mutex);

	switch(type)
	{
		case 0:
			m_mdlogpulling = value;
			break;
		case 1:
			m_mblogpulling = value;
			break;
		case 3:
			m_connsyslogpulling = value;
			break;
		case 4:
			m_mddbpulling = value;
			break;
		default:
			break;
	}

	pthread_mutex_unlock(&m_Mutex);
}


//////////////////////////////////////////////MATMSocket////////////////////////////////////////////////////

MATMSocket::MATMSocket()
{

}

MATMSocket::MATMSocket(SOCKET_TYPE type)
{
	m_type = type;
}

MATMSocket::~MATMSocket()
{

}

void MATMSocket::wait_msg()
{
	const char *msg = "ATM";
	char data[DATA_LEN] = {0};
	int len = 0;
	
	META_LOG("[Meta][MATMSocket] wait_msg m_clientID = %d", m_clientID);

	while(m_stop == 0)
	{
		memset(data, 0, DATA_LEN);				
		len = read(m_clientID, data, DATA_LEN);

		if(len > 0)
		{
			if(len == DATA_LEN)
				data[len-1] = 0;
			else
				data[len] = 0;
						
			META_LOG("[Meta][MATMSocket] data len = %d, rawdata = %s", len, data);
			char *pos = strstr(data, msg);
			if(pos != NULL)
			{
				HandleSocketCmd(data);
				continue;
			}
		}
		else
		{
			usleep(100*1000); // wake up every 0.1sec
		}
	}
	return;
}




