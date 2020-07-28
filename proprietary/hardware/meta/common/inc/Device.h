#pragma once

#include <pthread.h>
#include "MetaPub.h"


#define NULL_FILE_DESCRIPTOR (-1)	// 0 or -1 ???

class IDevWatcher
{
public:
	virtual signed int onReceived(
		 unsigned char*, unsigned int) = 0;
	virtual ~IDevWatcher(void) = 0;
};

class Device
{
public:
	Device(void);
	virtual ~Device(void);

public:
	virtual signed int read(unsigned char*, unsigned int);
	virtual signed int write(const unsigned char*, unsigned int);
	virtual void update();
	virtual void close();
    void waitForThreadExit();
	void setExitFlag(unsigned int exitFlag);

	signed int pump(IDevWatcher*);
	signed int pumpAsync(IDevWatcher*);
	signed int getDevHandle() const
	{
		return m_fd;
	}
	

private:
	static void *ThreadProc(void*);
	unsigned int m_exitFlag;

protected:
	signed int m_fd;
	pthread_t m_thread;
	IDevWatcher *m_pWatcher;
	pthread_mutex_t m_wMutex;
};
