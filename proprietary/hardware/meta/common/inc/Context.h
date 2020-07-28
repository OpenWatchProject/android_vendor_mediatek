#ifndef _CONETXT_H_
#define _CONETXT_H_

#include "MetaPub.h"


#define MD1_INDEX 0x01
#define MD2_INDEX 0x02
#define MD3_INDEX 0x04
#define MD5_INDEX 0x10


typedef enum
{
	MODEM_AP_TST = 0,
	MODEM_DHL = 1,
	MODEM_6292 = 2,
	MODEM_6293 = 3,
	MODEM_6295 = 4,
	MODEM_6297 = 5,
	MODEM_END = 6,	
}Modem_Hw_Version;


#define BOOTMODE_PATH "/sys/class/BOOT/BOOT/boot/boot_mode"
#define UNKNOWN_BOOT -1
#define NORMAL_BOOT 0
#define META_BOOT 1

//Jade
//#define FLAG_PATH "/dev/block/platform/mtk-msdc.0/11230000.msdc0/by-name/proinfo" 
//Bianco
#define FLAG_PATH "/dev/block/platform/bootdevice/by-name/proinfo"
#define OFFSET_ATM (64+4*10+170+64+4+8)
#define OFFSET_METALOG (349)
#define KEY1 12345678
#define KEY2 23131123
#define ENCRYPT_LENTH 8

#define USBONLINE_STATUS_PATH  "/sys/class/power_supply/usb/online"

class Modem;
class CmdTarget;
class UsbRxWatcher;
class MSocket;

Modem * createModem(const char *ccci, unsigned short id);
Modem * createModem(unsigned short id);
void destroyModem(Modem *p);

Modem * getModem(unsigned short id);
CmdTarget * getModule(unsigned short id);
unsigned int getMdmType();
unsigned int getActiveMdmId();
unsigned int getMdmNumber();
signed int getModemHandle(unsigned short id); 

unsigned int dumpData(const unsigned char* con, int length);
unsigned int dumpDataInHexString(const unsigned char* con, int length, unsigned int bytesPerRow=16);
unsigned int getFileSize(int fd);
const char* makepath(unsigned char file_ID);
void createModemThread(unsigned short modemIndex,int usbUsb);
void destroyModemThread(unsigned short modemIndex);
void createSerPortThread();
void destroySerPortThread();
void createAllModemThread();
void destroyAllModemThread();

void setLogLevel(unsigned int level);
unsigned int getLogLevel();

int getModemProtocol(unsigned short modemIndex, void* modem_capa);
int getMDChType(unsigned short modemIndex);

unsigned int getMDMode(void);
void setMDMode(unsigned int modem_boot_mode);

void setActiveATModem(unsigned int activeATModemId);
unsigned int getActiveATModem();
int getIOCPort(unsigned int nModemIndex,int & bDataDevice);
int getBootMode();
void queryNormalModeTestFlag();
int getNormalModeTestFlag();
int setNormalModeTestFlag(int flag);

int setProductInfo(int type, int flag, int offset);
int getProductInfo(int type, int offset);


int getPropValue(const char *key);
int getDataChannelType();

void queryWifiPara(int argc, char** argv);
WIFI_PARA getWifiPara();

void destroyVirtualRxThread();
void createVirtualRxThread();
void setVirtualRxWatcher(UsbRxWatcher * virtualRxWatcher);

MSocket * createSocket(unsigned int type);
MSocket * getSocket(unsigned int type);
void delSocket(unsigned int type);

void destroyContext();

int getLoadType();
int getModemHwVersion(unsigned short modemIndex);

void setATRespFlag(int atFlag);
int getATRespFlag();

int notifyModemDoRFByATCI();
int ChangeModemMode(int mode);

void writeBootprof(char * str);
int writePortIndex();

int readSys_int(char const * path);
unsigned int checkMdStatus();

void SetDataCompressStatus(unsigned int enable);
unsigned int GetDataCompressStatus();

void HandleSocketCmd(char* socket_cmd);
void setCurrentMdMode(int mdMode);


#endif	// _CONETXT_H_
