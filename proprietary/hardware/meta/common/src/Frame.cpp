#include <stddef.h>
#include <string.h>
#include "Frame.h"
#include "CmdTarget.h"
#include "LogDefine.h"

Frame::Frame(const META_RX_DATA &data, CmdTarget *mod)
	: m_myMod(mod), m_frmData(data),m_isValid(1)
{
}
Frame::Frame()
	: m_isValid(0)
{
    m_myMod = NULL;
	memset(&m_frmData, 0, sizeof(META_RX_DATA));
}


Frame::~Frame(void)
{
}

void Frame::exec()
{
	if (m_myMod != NULL)
	{
		m_myMod->exec(this);
	}
	else
	{
		META_LOG("[Meta] No module assigned; data discarded.");
	}
}
