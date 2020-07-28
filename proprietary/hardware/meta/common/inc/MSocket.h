#ifndef _METASOCKET_H_
#define _METASOCKET_H_

#include <string>
using namespace std;

typedef enum
{
	SOCKET_MDLOGGER    = 0,
	SOCKET_MOBILELOG   = 1,
	SOCKET_ATCI_CLIENT = 2,
	SOCKET_ATCI_SERVER = 3,
	SOCKET_ATM_COMM    = 4,
	SOCKET_CONNSYSLOG  = 5,
	SOCKET_END         = 6
}SOCKET_TYPE;


class MSocket
{

public:
	MSocket();
	virtual ~MSocket(void);
	int initClient(const char * socket_name, int namespaceId, int bListen=1);
	int initServer(const char * socket_name, int namespaceId, int bListen=1);
	void deinit();
	int connect();
	void disconnect();
	int getClientSocketID() const
	{
	   return m_clientID;
	}
	int getServerSocketID() const
	{
	   return m_serverID;
	}
	virtual void send_msg(const char *msg);

private:
	static void* ThreadFunc(void*);
	virtual void wait_msg() = 0;

public:
	SOCKET_TYPE m_type;
	bool m_bClientConnected;
	
protected:	
	int m_clientID;
	int m_serverID;
	int m_threadID;
	int m_stop;
	pthread_t  m_thread;
};

//////////////////////////////////////////////MATCIClientSocket////////////////////////////////////////////////////
//ATCI is socket client
class MATCIClientSocket : public MSocket
{
public:
	MATCIClientSocket();
	MATCIClientSocket(SOCKET_TYPE type);
	virtual ~MATCIClientSocket();

private:	
	virtual void wait_msg();
};

//////////////////////////////////////////////MATCIServerSocket////////////////////////////////////////////////////
//ATCI is socket server
class MATCIServerSocket : public MSocket
{
public:
	MATCIServerSocket();
	MATCIServerSocket(SOCKET_TYPE type);
	virtual ~MATCIServerSocket();

private:	
	virtual void wait_msg();
};


//////////////////////////////////////////////MLogSocket////////////////////////////////////////////////////

class MLogSocket : public MSocket
{
public:
	MLogSocket();
	MLogSocket(SOCKET_TYPE type);
	virtual ~MLogSocket();
	int recv_rsp(char *buf);
	void send_msg(const char *msg, bool ignore);
	int getLogPullingStatus(int type);
	void setLogPullingStatus(int type, int value);

private:
	string m_strCmd;
	string m_strRsp;
	int    m_mdlogpulling;
	int    m_mblogpulling;
	int    m_connsyslogpulling;
	int    m_mddbpulling;
	pthread_mutex_t m_Mutex;
	
private:	
	virtual void wait_msg();
};


//////////////////////////////////////////////MATMSocket////////////////////////////////////////////////////

class MATMSocket : public MSocket
{
public:
	MATMSocket();
	MATMSocket(SOCKET_TYPE type);
	virtual ~MATMSocket();

private:	
	virtual void wait_msg();
};

#endif


