#ifndef _FRAME_H_
#define _FRAME_H_

#include <stdlib.h>
#include "MetaPub.h"

class CmdTarget;

class Frame
{
public:
	Frame(const META_RX_DATA&, CmdTarget*);
	Frame();
	~Frame(void);

public:
	void exec();

	META_FRAME_TYPE type() const
	{
		return m_frmData.eFrameType;
	}

	unsigned char *localBuf() const
	{
		return m_frmData.pData;
	}
	unsigned char *peerBuf() const
	{
		if (m_frmData.PeerLen > 8)
		{
		    return m_frmData.pData + m_frmData.LocalLen + 8; //skip the header of peer buffer
	    }
		else
		{
			return NULL;
		}
	}

	unsigned short localLen() const
	{
		return m_frmData.LocalLen;
	}
	unsigned short peerLen() const
	{
		if (m_frmData.PeerLen > 8)
		{
		    return m_frmData.PeerLen - 8;//skip the header of peer buffer
	    }
		else
		{
			return 0;
		}
	}
	unsigned char getIsValid()
	{
		return 	m_isValid;
	}

	CmdTarget * getCmdTarget() const
	{
		return m_myMod;
	}

protected:
	void decode();

private:
	CmdTarget *m_myMod;
	META_RX_DATA m_frmData;
	unsigned char m_isValid;
};

#endif	// _FRAME_H_