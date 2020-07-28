#include "me_comm.h"

int GetHeader(char *header, char *src)
{
	char * cp = header;

	if(src[0] != '+')
		return 0;

	while(*src != ':')
	{
		*cp++ = *src++;
	}

	*cp = '\0';

	return 1;
}

int GetSignIdx(char *src, char ch, int seq)
{
	int count = 0, idx = -1;

	while(count != seq)
	{
		if(*src == '\0')
		{
			idx = -1;
			break;
		}
		if(*src++ == ch)
			count++;

		idx++;
	}

	return idx;
}

int GetInteger(char *src, int star, int end)
{
	char buf[16];
	char *dst = buf;

	star++;

	while(src[star] == ' ') //skip space
		star++;

	while(src[star] != 0 && star != end)
	{
		*dst++ = src[star++];
	}

	*dst = '\0';

	return me_atoi(buf);
}

void GetString(char *src, char *dst, int star, int end)
{
	star++;

	while(src[star] == ' ') //skip space
		star++;

	while(src[star] != 0 && star != end)
	{
		*dst++ = src[star++];
	}

	*dst = '\0';
}

void trim(char ch, char *src, char *dst)
{
	while(*src != '\0')
	{
		if(*src != ch)
			*dst++ = *src++;
		else
			src++;
	}

	*dst = '\0';
}

void trimright(char *src)
{
	int len = me_strlen(src);
	while(len > 0)
	{
		if(src[--len] == ' ')
			src[len] = '\0';
		else
			break;
	}
}

int GetSignCount(char *src, char ch)
{
	int count = 0;
	while(*src != '\0')
	{
		if(*src++ == ch)
			count ++;
	}

	return count;
}

char GetCrByte(void)
{
	return 0x0d;
}

char GetLfByte(void)
{
	return 0x0a;
}

char GetCharFromRawData(void *pData)
{
	PMEFSM pfsm = (PMEFSM)pData;
	unsigned char ch = 0xff;

	if (pfsm->nHead == RAW_DATA_LEN)
		pfsm->nHead = 0;

	if (pfsm->nHead != pfsm->nTail)
	{
		ch = pfsm->rawdata[pfsm->nHead++];
	}

	return ch;
}

BOOLEAN CheckIfGetGtandSpace(char a,  void *pData)
{
	unsigned char ch;

	if(a == '>')
	{
		while((ch=GetCharFromRawData(pData)) != 0xFF)
		{
			if( ch == ' ')
				return OSAL_TRUE;
			else
				return OSAL_FALSE;
		}		
	}
	else
	{
		return OSAL_FALSE;
	}

	return CheckIfGetGtandSpace(a, pData);	
}

void FillBufferByState(unsigned char ch, void *pData)
{
	PMEFSM pfsm = (PMEFSM)pData;

	switch(pfsm->state)
	{
	case 0:
		// Get the first tag of CR
		if(ch == GetCrByte())
		{
			pfsm->state = 1;
		}
		break;

	case 1:
		// Get the first tag of Lf
		if(ch == GetLfByte())
		{
			pfsm->state = 2;
			pfsm->nWriteIdx = 0;
		}
		else if(pfsm->bReadPdu)
		{
			if(ch != GetCrByte())
			{
				if(pfsm->nWriteIdx<CTL_DATA_LEN)
					pfsm->ctldata[pfsm->nWriteIdx++]=ch;
			}
			else
			{
				if(pfsm->nWriteIdx>1)
				{
					pfsm->state = 3;
				}
				else
				{
					pfsm->state = 1;
					pfsm->nWriteIdx = 0;
				}
			}
		}
		break;

	case 2:
		if(pfsm->bWaitPdu)
		{
			if(CheckIfGetGtandSpace(ch, pData))
			{
				pfsm->bWaitPdu = OSAL_FALSE;				
				pfsm->state = 1;
				pfsm->ctldata[0] = '>';

				if(pfsm->req.atproc != NULL)
					pfsm->req.atproc(pfsm);
			}
		}
		else
		{
			if(ch != GetCrByte())
			{
				if(pfsm->nWriteIdx<CTL_DATA_LEN)
					pfsm->ctldata[pfsm->nWriteIdx++]=ch;
			}
			else
			{
				if(pfsm->nWriteIdx>1)
				{
					// respDataBuf not an empty string
					pfsm->state = 3;
				}
				else
				{
					// respDataBuf an empty string
					// It must get into the condition of
					//	0x0d 0x0a 0x0d 0x0A
					pfsm->state = 1;
					pfsm->nWriteIdx = 0;
				}
			}
		}
		break;

	case 3:
		// Get the 2nd Lf
		if(ch == GetLfByte())
		{
			pfsm->ctldata[pfsm->nWriteIdx]='\0';
			
			if(!pfsm->urcproc(pfsm))
			{
				ME_Print(ME_TAG "[Medrv] Got a new AT response line\n");
				if(pfsm->req.atproc != NULL)
					pfsm->req.atproc(pfsm);
			}

			pfsm->state = 1;
			pfsm->nWriteIdx = 0;
			pfsm->ctldata[0] = 0;
		}
		break;

	default:
		break;
	}
}

int GetOneLine(void *pData)
{
	PMEFSM pfsm = (PMEFSM)pData;
	unsigned char ch;

	while((ch=GetCharFromRawData(pfsm)) != 0xff)
	{
		FillBufferByState(ch, pfsm);
	}
	return 0;

}

int DataCallBack(NTSTATUS status, const PUCHAR pTxt, ME_ULONG nLen, void *pData)
{
	PMEFSM	pfsm;
    ME_ULONG tmpHead, tmpTail;
    ME_ULONG overflow, overplus;

	if ((nLen == 0) || (pData == 0))
		return 0;
	if (status == 0)//reserved parameter, for resolve warning
	{
	    ME_Print(ME_TAG "[Medrv] Warning: status == 0\n");
	}
	pfsm = (PMEFSM)pData;

	if(pfsm->req.event == AT_CANCEL || pfsm->req.event == AT_TIMEOUT)
		return 0;

	tmpHead = pfsm->nHead;
	tmpTail = pfsm->nTail;

	if (tmpTail >= tmpHead)
		overplus = RAW_DATA_LEN - tmpTail + tmpHead;
	else
		overplus = tmpHead - tmpTail;

	if (overplus < nLen)
	{   //shouldn't give a warning? 
		return 0;
	}

	//copy data to circular buffer
	if(tmpTail+nLen >= RAW_DATA_LEN)
	{
		overflow = tmpTail + nLen - RAW_DATA_LEN;
		me_memcpy(pfsm->rawdata+tmpTail, (void*)pTxt, (int)(nLen-overflow));
		me_memcpy(pfsm->rawdata, (void*)(pTxt+nLen-overflow), (int)overflow);
		tmpTail = overflow;
	}
	else
	{
		me_memcpy(pfsm->rawdata+tmpTail, pTxt, (int)nLen);
		tmpTail += nLen;
	}

	pfsm->nTail = (int)tmpTail;


	GetOneLine(pData);
	return (int)nLen;
}

