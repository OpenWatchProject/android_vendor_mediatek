#include <string.h>
#include "SerPort.h"
#include "LogDefine.h"
#include "MdRxWatcher.h"
#include "Context.h"
#include "PortInterface.h"
#include "FtModule.h"
#include "snappy-c.h"


MdRxWatcher::MdRxWatcher(int index)
{
	m_bufLen = 0;
	m_frmStat = 0;
	m_frmStat0 = 0;
	m_chkSum = 0;
	m_frmLen = 0;
	m_bL1Header = 0;
	nModemIndex = index;
}

MdRxWatcher::~MdRxWatcher(void)
{
}

signed int MdRxWatcher::onReceived(
	 unsigned char *buf,
	unsigned int len)
{	
	META_LOG("[Meta] Receive data from modem[%d], len = %d", nModemIndex, len);
	if(getMDMode()==1)//normal mode
	{
		SerPort *pPort = getSerPort();
		if(NULL == pPort || 1 == getNormalModeTestFlag())  return 0;  //Discard this packet if pPort is NULL or in ATM mode
		signed int cbWriten = 0;
		cbWriten= pPort->write(buf, len);
		META_LOG("[Meta] Write data by USB. data len = %d, write done = %d", len, cbWriten);		
		dumpDataInHexString(buf,len,16);
	}
	else
	{
		dumpDataInHexString(buf,len,16);  

		int mdType = getMDChType(nModemIndex); 
		if 	( mdType == FT_MODEM_CH_TUNNELING || 
	      	mdType == FT_MODEM_CH_TUNNELING_IGNORE_CKSM )
		{
			processTunnelData(buf, len);
		}
		else
		{
	   		process(buf, len);
		}
	}

	return 0;
}

int MdRxWatcher:: fillDataToTSTBufferReverse(unsigned char data, char **buffer_ptr)
{
	if (data == 0x5A) {
		*(*buffer_ptr)-- = 0x5A;
		*(*buffer_ptr)-- = 0x5A;
		return 2;
	} else if (data == 0xA5) {
		*(*buffer_ptr)-- = 0x01;
		*(*buffer_ptr)-- = 0x5A;
		return 2;
	}
	
	*(*buffer_ptr)-- = data;
	return 1;
}

int MdRxWatcher::fillDataToTSTBuffer(unsigned char data, char **buffer_ptr)
{
	if (data == 0x5A) {
		*(*buffer_ptr)++ = 0x5A;
		*(*buffer_ptr)++ = 0x5A;
		return 2;
	} else if (data == 0xA5) {
		*(*buffer_ptr)++ = 0x5A;
		*(*buffer_ptr)++ = 0x01;
		return 2;
	}
	
	*(*buffer_ptr)++ = data;
	return 1;
}


void MdRxWatcher::processTunnelData(void *pdata, unsigned short len)
{
	//META_LOG("[Meta] processTunnelData, len = %d", len);
	int md_rsp_type = RS232_RESPONSE_MD_DATA_TUNNEL_START;
	char *buf_begin = NULL;
	char *p = NULL;
	TST_MD_RECV_BUF recv_buf;
	memset(&recv_buf, 0, sizeof(TST_MD_RECV_BUF));
#ifdef MTK_META_COMPRESS_SUPPORT

	char compressed[(64+12)*1024] = {0};
	size_t compressed_len = (64+12)*1024;
	
	if(GetDataCompressStatus() && (len >= 50))
	{
		md_rsp_type = RS232_RESPONSE_MD_DATA_TUNNEL_COMP_START;
		
		if (true != compress((char *)pdata, len, &compressed[0], &compressed_len))
		{
			META_LOG("[Meta] error: compress failed\n");
			return;
		}
		
		META_LOG("[Meta] Dump Compressed Confirm Data\n");
		dumpDataInHexString((unsigned char *)compressed, compressed_len, 16);
		
//		pdata = compressed;
//		len = compressed_len;
		if( compressed_len >= MAX_TST_RECEIVE_BUFFER_LENGTH)
			compressed_len = MAX_TST_RECEIVE_BUFFER_LENGTH - 1;
		memcpy(recv_buf.data, compressed, compressed_len);
		recv_buf.data_len = compressed_len;
	}
	else
	{
		memcpy(recv_buf.data, pdata, len);
		recv_buf.data_len = len;
	}
#else
	memcpy(recv_buf.data, pdata, len);
	recv_buf.data_len = len;
#endif

	// create tunneling header
	// TST spec: For confirm message, the length of frame content includes data and message type (1 byte). 
	// See TST spec for detail
	unsigned int length_in_tunnel_resp = recv_buf.data_len + 1;
	// Write the TST header in RESERSE ORDER, please note that buffer space must preseve enough spacing for possibile escaping translation
	buf_begin = (char *)recv_buf.data-1;
	fillDataToTSTBufferReverse((md_rsp_type + nModemIndex), &buf_begin);
	fillDataToTSTBufferReverse((length_in_tunnel_resp & 0xff), &buf_begin);
	fillDataToTSTBufferReverse((length_in_tunnel_resp >> 8), &buf_begin);
	*(buf_begin) = 0x55;

	// calculate checksum if needed, otherwise the checksum field is 0xff
	char cksm_byte = 0xff;
	char *recv_buf_end = recv_buf.data + recv_buf.data_len;

	if (getMDChType(nModemIndex) == FT_MODEM_CH_TUNNELING) 
	{
		cksm_byte = (md_rsp_type+nModemIndex);
		cksm_byte ^=(length_in_tunnel_resp & 0xff);
		cksm_byte ^=(length_in_tunnel_resp >> 8);
		cksm_byte ^= 0x55;
		
		for(p = (char *)recv_buf.data; p < recv_buf_end; p++) {
			cksm_byte ^= *p;
		}
	}

	fillDataToTSTBuffer(cksm_byte, &recv_buf_end);

	processMDConfirm(buf_begin, recv_buf_end - buf_begin);
}


void MdRxWatcher::processMDConfirm(void *pdata, unsigned short len)
{
	//META_LOG("[Meta] processMDConfirm, len = %d", len);
	signed int iCheckNum = 0;
	signed int dest_index=0;
	signed int cbWriten = 0;
	signed int cbTxBuffer = len;

	const unsigned char *pTempBuf = (unsigned char*)pdata;

	unsigned char pCache[FRAME_MAX_LEN]={0};
	unsigned char *pTempDstBuf = pCache;

	SerPort *pPort = getSerPort();
	if(NULL == pPort) return; //Discard this packet because it is during port switching process, and pPort is NULL

	/*
	so we use 0x77 and 0x01 indicate 0x11,
	use 0x77 and 0x03 indicate 0x77,
	use 0x77 and 0x13 indicate 0x13
	the escape is just for compatible with feature phone
	*/

	META_COM_TYPE eComType = getComType();

	if(eComType == META_USB_COM || eComType == META_SOCKET )
	{
		cbWriten = pPort->write((unsigned char *)pdata, len);
		//META_LOG("[Meta] write data by USB. data len = %d, write done = %d", len, cbWriten);		
		dumpDataInHexString((unsigned char *)pdata,len,16);
		return;
	}
	else if(eComType == META_UART_COM)
	{
		while (iCheckNum != cbTxBuffer)
		{
			++ iCheckNum;
	
			if (*pTempBuf ==0x11 )
			{
				*pTempDstBuf++ = 0x77;
				*pTempDstBuf++ = 0x01;
				++ dest_index;
			}
			else if (*pTempBuf ==0x13 )
			{
				*pTempDstBuf++ = 0x77;
				*pTempDstBuf++ = 0x02;
				++ dest_index;
			}
			else if (*pTempBuf ==0x77 )
			{
				*pTempDstBuf++ = 0x77;
				*pTempDstBuf++ = 0x03;
				++ dest_index;
			}
			else
			{
				*pTempDstBuf++ = *pTempBuf;
			}

			++ dest_index;
			++ pTempBuf;

			if (dest_index+2 > (int)sizeof(pCache))
			{
				cbWriten = pPort->write(pCache, dest_index);
				dest_index = 0;
				pTempDstBuf = pCache;
				META_LOG("[Meta] write data by UART. data len = %d, write done = %d", dest_index, cbWriten);
				dumpDataInHexString((unsigned char *)pCache,dest_index,16);
			}
		}
	}

	if (dest_index > 0)
	{
		cbWriten = pPort->write(pCache, dest_index);
		META_LOG("[Meta] write data by UART. data len = %d, write done = %d", dest_index, cbWriten);
		dumpDataInHexString((unsigned char *)pCache,dest_index,16);
	}
}

void MdRxWatcher::process(const unsigned char *buf, unsigned int len)
{


	unsigned char	ch=0;	
	unsigned short	u16Length=0;
	const unsigned char	*src=buf;

	while (u16Length < len)
	{
		ch = *src;
		m_frmBuf[m_bufLen] = ch;

		++ u16Length;
		++ m_bufLen;

		if ((ch == STX_OCTET || ch == STX_L1HEADER ) &&
			(m_frmStat != RS232_FRAME_MD_CONFIRM_DATA) &&
			(m_frmStat != RS232_FRAME_LENHI) &&
			(m_frmStat != RS232_FRAME_LENLO) )
		{
			if(ch == STX_L1HEADER)
				m_bL1Header = true;
			else
				m_bL1Header = false;
			
			if ( m_frmStat != RS232_FRAME_CHECKSUM)
			{
				m_frmStat = RS232_FRAME_LENHI;
				META_LOG("[Meta] Flag change to RS232_FRAME_LENHI");
				++ src;
				m_chkSum = ch;
				ch = *src;
				m_frmLen = 0;
				continue;
			}		
		}
		else
		{
			if ((!m_bL1Header)   &&
				(*src == MUX_KEY_WORD ) &&
				(m_frmStat != RS232_FRAME_KEYWORD))
			{ // enter MUX state(0x5A) and save the old

				m_frmStat0 = m_frmStat;
				m_frmStat = RS232_FRAME_KEYWORD;

				++ src;

				continue;
			}
			else if(m_frmStat == RS232_FRAME_KEYWORD)
			{
				if (*src== MUX_KEY_WORD)
					ch = MUX_KEY_WORD;
				else if (*src == 0x01)
					ch=STX_L1HEADER; //0xA5 escaping

				//leave MUX state and restore the state
				m_frmStat = m_frmStat0;
				++ m_frmLen;				
			}
		}

		switch (m_frmStat)
		{
			/*the state is RS232_FRAME_LENHI*/
		case RS232_FRAME_LENHI:
			if(m_bL1Header)
				m_frmLen = ch;
			else
				m_frmLen = ch << 8;
			m_frmStat = RS232_FRAME_LENLO;
			break;

			/*the state is RS232_FRAME_LENLO*/
		case RS232_FRAME_LENLO:
			if(m_bL1Header)
				m_frmLen += (ch << 8);
			else
				m_frmLen += ch;
			if ((m_frmLen +4) > FRAME_MAX_LEN)
			{
				m_frmStat = RS232_FRAME_STX;
				META_LOG("[Meta] frame too long: %d+4 > %d.",
					m_frmLen, FRAME_MAX_LEN);
				return;
			}
			else
			{
				m_frmStat = RS232_FRAME_MD_CONFIRM_DATA;
			}
			break;

		case RS232_FRAME_MD_CONFIRM_DATA:
			if (m_bufLen == m_frmLen+3)
			{
				m_frmStat = RS232_FRAME_CHECKSUM;
			}

			break;


		case  RS232_FRAME_CHECKSUM:
			m_frmStat = RS232_FRAME_STX;

			if (m_chkSum == ch)
			{
				processMDConfirm(m_frmBuf, m_frmLen+4);

				buf = src;
				m_bufLen = 0;
				m_frmLen = 0;
				m_chkSum = 0;
			}
			else
			{
				META_LOG("[Meta] CheckSum error: %d != %d",
					(signed int)m_chkSum, (signed int)ch);
			}
			break;
		case RS232_FRAME_STX:
			m_bufLen = 0;
			m_frmLen = 0;
			m_chkSum = ch;
			break;
		default:
			/* exception of g_cTstFrameState */
			break;

		}
		m_chkSum ^= ch;
		++ src;
	}


}
bool MdRxWatcher::compress(void *pdata, unsigned short len, char *compressed, size_t *compressed_len)
{
    META_LOG("[Meta] META Confirm. compress starts\n");

    char *pTempBuf = (char*) pdata;
    // Ask for the max size of the compressed object.
    size_t max_compressed_len = snappy_max_compressed_length(len);
    if (*compressed_len < max_compressed_len)
    {
        META_LOG("[Meta] error: snappy compression buffer is not enough. buffer size: %d, max compression len: %d\n", *compressed_len, max_compressed_len);
        return false;
    }

    META_LOG("[Meta] snappy compress starts. len: %d\n", len);
    snappy_status status = snappy_compress(pTempBuf, len, compressed, (size_t *)compressed_len);
    if (status != SNAPPY_OK)
    {
        META_LOG("[Meta] Compression failed. status: %d\n", status);
        return false;
    }
    META_LOG("[Meta] snappy compress done. comp_len: %d\n", *compressed_len);

    return true;
}
