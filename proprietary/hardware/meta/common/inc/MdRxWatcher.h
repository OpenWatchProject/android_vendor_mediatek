#ifndef _MD_RX_WATCHER_H_
#define _MD_RX_WATCHER_H_

#include "Frame.h"
#include "Device.h"
#include "MetaPub.h"
#include "DriverInterface.h"



class MdRxWatcher : public IDevWatcher
{
public:
	MdRxWatcher(int index);
	virtual ~MdRxWatcher(void);

public:
	virtual signed int onReceived(
		unsigned char*, unsigned int);

private:
	void process(
		const unsigned char *buf,
		unsigned int len);

	void processMDConfirm(
		void *pdata, unsigned short len);

	
	int fillDataToTSTBufferReverse(unsigned char data, char **buffer_ptr);	
	int fillDataToTSTBuffer(unsigned char data, char **buffer_ptr);	
	void processTunnelData(void *pdata, unsigned short len);
        bool compress(void *pdata, unsigned short len, char *compressed, size_t *compressed_len);

private:
	unsigned short m_bufLen;
	unsigned short m_frmLen;
	unsigned char  m_frmBuf[FRAME_MAX_LEN];
	unsigned char  m_frmStat;
	unsigned char  m_frmStat0;
	unsigned char  m_chkSum;
	int	   m_bL1Header;  //0:PS dta 1:L1 Data
	
	int nModemIndex;
};

typedef struct 
{
	unsigned int	data_len;
	char			preserve_head_buf[MD_FRAME_HREADER_LENGTH*2]; // Double the buffer space to preserve extension for escape translation
	char			data[MAX_TST_RECEIVE_BUFFER_LENGTH];
	char			preserve_tail_buf[TST_CHECKSUM_SIZE*2];// Double the buffer space to preserve extension for escape translation
} TST_MD_RECV_BUF;



#endif	// _MD_RX_WATCHER_H_
