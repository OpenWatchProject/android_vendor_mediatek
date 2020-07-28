#ifndef _PORTHANDLE_H_
#define _PORTHANDLE_H_


#ifdef __cplusplus
extern "C" {
#endif

extern int WriteDataToPC(void *Local_buf,unsigned short Local_len,void *Peer_buf,unsigned short Peer_len);

#ifdef __cplusplus
}
#endif

#endif	// _PORTHANDLE_H_





