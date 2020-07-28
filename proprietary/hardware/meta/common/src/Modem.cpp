#include "Modem.h"
#include "Device.h"
#include "LogDefine.h"

Modem::Modem(const char *ccci, unsigned short id)
	: CmdTarget(id), m_pDev(new CCCI(ccci))
{
}

Modem::Modem(unsigned short id)
	: CmdTarget(id), m_pDev(new CCB())
{

}
Modem::~Modem(void)
{	
	m_pDev->close();
	META_LOG("[Meta] Delete Modem");
	delete m_pDev;
}

signed int Modem::pumpAsync(IDevWatcher *p)
{
	return m_pDev->pumpAsync(p);
}

void Modem::popUpAsync()
{
	m_pDev->setExitFlag(1);
}


void Modem::exec(Frame *pFrm)
{
	CmdTarget::exec(pFrm);
	m_pDev->write(pFrm->localBuf(), pFrm->localLen());
}

void Modem::exec(const unsigned char *p, unsigned int len )
{
	m_pDev->write(p,len);
}

signed int Modem::getDevHandle()
{
	return m_pDev->getDevHandle();
}
	
int Modem::init(Frame*)
{
	return 1;
}


void Modem::deinit()
{
}

