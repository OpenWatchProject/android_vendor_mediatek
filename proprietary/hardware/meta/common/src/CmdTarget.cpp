#include "CmdTarget.h"
#include "LogDefine.h"

CmdTarget::CmdTarget(unsigned short id)
	: m_myId(id), m_isInited(false)
{
	META_LOG("[Meta] id = %d", m_myId);
	m_token = 0;
}

CmdTarget::~CmdTarget(void)
{
	if (m_isInited)
	{
		deinit();
		m_isInited = false;
	}
}

void CmdTarget::exec(Frame* pFrame)
{
	if (!m_isInited)
	{
		if(init(pFrame))
			m_isInited = true;
	}
}


int CmdTarget::init(Frame*)
{
	return true;
}

void CmdTarget::deinit()
{
}
