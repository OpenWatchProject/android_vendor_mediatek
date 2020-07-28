#ifndef _MODEM_H_
#define _MODEM_H_

#include "CmdTarget.h"
#include "SerPort.h"

class IDevWatcher;

class Modem : public CmdTarget
{
public:
	Modem(const char*, unsigned short);
	Modem(unsigned short id);
	virtual ~Modem(void);

public:
	virtual void exec(Frame*);
	virtual void exec(const unsigned char *p, unsigned int len );

	signed int pumpAsync(IDevWatcher*);
	signed int getDevHandle();
	void popUpAsync(); 

protected:
	 int init(Frame*);
	 void deinit();

private:
	Device *m_pDev;
};

#endif	// _MODEM_H_
