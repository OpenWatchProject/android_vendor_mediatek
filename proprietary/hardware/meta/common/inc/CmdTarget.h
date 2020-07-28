#ifndef _CMD_TARGET_H_
#define _CMD_TARGET_H_

#include "Frame.h"

class CmdTarget
{
public:
	CmdTarget(unsigned short);
	virtual ~CmdTarget(void);

public:
	virtual void exec(Frame*);
	
	unsigned short getId() const
	{
		return m_myId;
	}

	unsigned short getToken() const
	{
		return m_token;
	}

	void setToken(unsigned short token) 
	{
		m_token = token;
	}

	int getInitState() const
	{
		return m_isInited;
	}
protected:
	virtual int init(Frame*);
	virtual void deinit();

private:
	unsigned short	m_myId;
	int	m_isInited;
	unsigned short  m_token;
};
/*
template <typename _Tx>
_Tx *getInstance()
{
	static _Tx _inst;
	return &_inst;
}
*/
#endif	// _CMD_TARGET_H_
