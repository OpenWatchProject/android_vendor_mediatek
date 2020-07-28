#ifndef _USB_RX_WATCHER_H_
#define _USB_RX_WATCHER_H_

#include "Device.h"
#include "Frame.h"
#include "MetaPub.h"
#include "DriverInterface.h"

#define FrameMaxSize 4096*16//2048

typedef struct
{
	unsigned short     local_len;
	unsigned short     peer_len;
} PRIM_HEADER;

/* the define of buf of meta type */
typedef struct
{
	PRIM_HEADER	inject_prim;				//lenght of peer buf and local buf
	unsigned short	received_prig_header_length;	//recieved header count
	unsigned short	received_buf_para_length;		//recieved buf count
	unsigned char	*header_ptr;					//header pointer
	unsigned char	*buf_ptr;						//buf pointer
} PRIM_FRAME;


typedef struct 
{
	unsigned short	frame_len;
	unsigned char		frame_state;
	unsigned char		frame_cksm;
	unsigned char		frame_md_index;
	unsigned char		frame_buf[FrameMaxSize]; // Must be 4-byte aligned
	unsigned char*   	frame_data_ptr; // this is a frame type dependent data pointer
} TST_FRMAE_INTERNAL_STRUCT;

class UsbRxWatcher : public IDevWatcher
{
public:
	UsbRxWatcher(void);
	virtual ~UsbRxWatcher(void);

public:
	virtual signed int onReceived(
		unsigned char*, unsigned int);

private:
	Frame *decode(unsigned char*, unsigned int, unsigned short&);
	Frame *decodeMDFrame(void *pdata, unsigned int len, unsigned char frmType,unsigned short&);
	Frame *decodeAPFrame(unsigned int input_len,unsigned char * src,unsigned short&);	
	Frame *decodeLTE_C2KFrame(unsigned int input_len,unsigned char * src,unsigned char frmType,unsigned short &u16Length);
	Frame *sendFtTask();
	
	Frame * sendMdTask(void *pdata, unsigned int len,unsigned char frmType);

	unsigned char transferFrame(unsigned char * ch);
	unsigned char checkEscape(unsigned char ch);
	
	Frame * dispatchFrame(unsigned char ch, unsigned char *buf_ptr, unsigned int input_len, unsigned char *src,unsigned short& );
	
	unsigned char *reallocFrameBuf(unsigned int len);

	unsigned int flowControl(
		void *pdata, unsigned int len);
	
	unsigned char getUARTEsc(unsigned char &ch);

    bool uncompress(void *pdata, int len, char *uncompressed, int *uncompressed_len);
	unsigned int nRemainLen;	
	unsigned char szRemainBuf[FrameMaxSize*2];
	char m_uncompressed[FrameMaxSize];
private:
	unsigned char m_checksum;
	unsigned short m_uFrameLength;

	char m_cTstFrameState;
	char m_cOldTstFrameState;

	unsigned short m_frame_buf_len;
	PRIM_FRAME m_sRs232Frame;

	char m_flow_ctrl_flag;

	unsigned char m_md_index;//Only LTE modem use

	unsigned char m_frm_len_byte; //frame length take up to bytes
	unsigned int m_nStartByteLen;

};

#endif	// _USB_RX_WATCHER_H_
