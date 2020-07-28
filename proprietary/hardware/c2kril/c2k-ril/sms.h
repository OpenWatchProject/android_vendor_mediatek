#include <telephony/ril_cdma_sms.h>
#include <telephony/mtk_ril.h>
/*
 *  get from VTUI2 platform
 */
#define NUM_BYTES(bitCount) \
      ((bitCount % 8) ? ((bitCount / 8) + 1):(bitCount / 8))

#define SMS_UNUSED(x)   ((void)(x))

/*
 * The max cell broadcast cateogory range number
 * supported by MD
 */
#define MAX_RANGE    10

/*
 * The max character number of a range
 * include '\0'
 * range example: "65534-65535", total 14 character,
 * allow to include some spaces
 */
#define MAX_RANGE_CHARACTER       20

/* A range include a start endpoint and a end endpoint */
#define VALID_RANGE_ENDPOINT_NUM  2

/* BitMasks for uint16 values */
static const unsigned short _valueMask[] =
{
    0x0000,
    0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
    0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};

/* BitMasks for uint8 values */
static const unsigned char _bitValueMask[] =
{
    0xff, 0x7f, 0x3f, 0x1f, 0x0f, 0x07, 0x03, 0x01
};

static const unsigned char _saveTopMask[] =
{
    0x00, 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe
};

/*
 * The structure stands for cell broadcast ranges
 * such as 4096-4096, 4098-4101
 */
typedef struct {
    int start;
    int end;
} Range;

/*
 * brief : transform RIL_CDMA_SMS_Message struct into cdma pdu format
 *
 * param SmsMsg [IN] RIL_CDMA_SMS_Message struct will be transformed
 *
 * param PduP [OUT] transformed pdu format byte array should alloced memory first
 *
 * param nlength [OUT] length of PduP byte array
 *
 * ret : NONE
 */
void RILCdmaSmsPdu (RIL_CDMA_SMS_Message *SmsMsg, unsigned char *PduP, unsigned char *nlength);

/*
 * brief : transform teleserviceId into pdu byte array
 *
 * param data [IN/OUT] pdu byte array , will add teleserviceId field into this array
 *
 * param teleserviceId [IN] teleserviceId value will be encoded into pdu teleserviceId field
 *
 * ret : pos after put teleserviceId value
 */
unsigned char MakeTeleSrvId(unsigned char *data, int teleserviceId);

/*
 * brief : transform Address into pdu byte array
 *
 * param data [IN/OUT] pdu byte array , will add Address field into this array
 *
 * param addr [IN] Address value will be encoded into pdu teleserviceId field
 *
 * param MoSmsMsg [IN] this value should always be 1
 *
 * ret : pos after put address value
 */
unsigned char MakeAddress(unsigned char *data, RIL_CDMA_SMS_Address *addr, int MoSmsMsg);

/*
 * brief : put bits value into a byte(aux function for transform pdu array)
 *
 * param data [IN/OUT] byte value
 *
 * param startBit [IN] start bit pos in a byte
 *
 * param numBits [IN] length of bits value
 *
 * param value [IN] value of bits
 *
 * ret : NONE
 */
void putuint8(unsigned char *data, unsigned short startBit, unsigned char numBits,
        unsigned char value);

/*
 * brief : put bits value into a word(aux function for transform pdu array)
 *
 * param data [IN/OUT] byte value
 *
 * param startBit [IN] start bit pos in a byte
 *
 * param numBits [IN] length of bits value
 *
 * param value [IN] value of bits
 *
 * ret : NONE
 */
void putuint16(unsigned char *data, unsigned short startBit, unsigned char numBits,
        unsigned short value);

/*
 * brief : get value frome a byte
 */
unsigned char getuint8( unsigned char *data, unsigned short startBit, unsigned char numBits );

/*
 * brief : Convert a byte value into ascii value
 *
 * param [IN] : byte value will be transformed
 *
 * ret : ascii value
 */
char ConvertChar(char value);

/*
 * brief : Convert byte value array into ascii value array
 * 
 * param pdu [IN] byte value array will be transformed into ascii value array
 *
 * param length [IN] length of pdu array
 *
 * param strPdu [OUT] array store ascii value should alloc memory first
 *
 * ret : NONE
 */
void ConvertAscii(unsigned char* pdu, int length, char strPdu[]);

/*
 * brief : Convert ascii value number into dtmf format
 *
 * param Digit [IN] acsii value will be transform into dtmf format 
 *
 * ret : dtmf value
 */
signed char RILNumCharToDTMF(signed char Digit);

/*
 * brief : transformed RIL_CDMA_SMS_Message struct into ascii format pdu array & get number from struct
 *
 * param p_message [IN] struct will be transformed
 *
 * param s_pdu [OUT] ascii format pdu array callee should free memory alloced by function
 *
 * param s_number [OUT] number get from struct callee should free memory aclloced by function
 */
int RILEncodeCdmaSmsPdu(RIL_CDMA_SMS_Message* p_message, char** s_pdu, char** s_number);

/** used for receive cdma sms */

/*
 * brief : transformed RIL_CDMA_SMS_Ack struct into ascii format pdu array & get length
 *
 * param p_message [IN] struct will be transformed
 *
 * param replySeqNo [IN] reply sequence number
 *
 * param address [IN] the destination address of this acknowlegdement message
 *
 * param s_pdu [OUT] ascii format pdu array callee should free memory alloced by function
 *
 * param p_len [OUT] length of the pdu byte array
 */
int RILEncodeCdmaSmsAckPdu(RIL_CDMA_SMS_Ack* p_message,
        int replySeqNo, unsigned char *address, char** s_pdu, int* p_len);

/** used for send cdma sms delivery ack */

/*
 * brief : Convert ascii pdu array into byte value array
 *
 * param pdu [IN] ascii pdu array will be transformed into byte value array
 *
 * param  bytePdu [OUT] array store byte value should alloced memory first
 *
 * param len [IN] len of bytePdu should passed to function 
 */
void ConvertByte(unsigned char* pdu, unsigned char* bytePdu, unsigned short len);

/*
 * brief : Convert ascii value into byte value
 *
 * param s [IN] ascii value
 *
 * ret : byte value
 */
char ConvertNum(unsigned char* s);

/*
 * brief : Convert bytePdu into RIL_CDMA_SMS_Message
 *
 * param bytePdu [IN] byte value pdu array
 *
 * param length [IN] length of bytePdu array
 *
 * param p_message [OUT] struct store pdu content
 *
 * param _replySeqNo [OUT] reply sequence number
 *
 * ret : NONE
 */
void ProcessCdmaIncomingSms(unsigned char* bytePdu, unsigned short length,
        RIL_CDMA_SMS_Message* p_message, int* p_replySeqNo, unsigned char *address);

/*
 * brief : Extract Address info from pdu byte array
 *
 * param BytePdu [IN] pdu byte array
 *
 * param Address [OUT] struct stroe pdu address content
 */
unsigned char RILCdmaSmsExtractAddress(unsigned char* BytePdu, RIL_CDMA_SMS_Address* Address);

/*
 * brief : Extract SubAddress info from pdu byte array
 *
 * param BytePdu [IN] pdu byte array
 *
 * param addr [OUT] struct stroe pdu subaddress content
 */
unsigned char RILCdmaSmsExtractSubaddress(unsigned char *BytePdu, RIL_CDMA_SMS_Subaddress* addr);

/*
 * brief : resolve unsolicited new sms incoming at command string
 *
 * param SmsAtStr [IN] new sms incoming at command string
 *
 * param SmsPdu [OUT] Pdu String extracted from at command string
 *
 * param length [OUT] length of pdu string
 */
char ParseSmsAtString(char* SmsAtStr, char** SmsPdu, unsigned short* length);

/*
 * brief : Convert Dtmf format number into ascii format
 *
 * DtmfDigit [IN] dtmf digit number
 *
 * ret : ascii format number
 */
signed char RILNumDTMFToChar(signed char DtmfDigit);

/*
 * brief : Get Index data from At Command +CDSI
 *
 * s [IN] CDSI At Command
 *
 * ret : Index value in At Command
 */
int RetrieveIndex(char* s);

/*
 * brief : uesed for AT Command CMGR to read Pdu String
 *
 * s [IN] unsolicited string by AT Command CMGR 
 *
 * begin [IN] begin char of Pdu String
 *
 * end   [IN] end char of Pdu String
 *
 * pdu   [IN/OUT] PDU String from s
 */
unsigned int RetrievePDUStr(char* s, char begin, char end, char** pdu);

/*
 * brief : Retrieve string from a string begin with begin end with end
 *
 * begin [IN] begin char
 *
 * end   [IN] end char
 *
 * str   [IN/OUT] string retrieved
 */
unsigned int RetrieveStr(char* s, char begin, char end, char** str);

/*
 * brief : sort a integer array in ascending order
 *
 * a [IN] the array to be sorted
 * n [IN] the array number
 */
extern void sort(int a[], int n);

/*
 * brief : get the ranges from an ascending order sorted integer array
 *      if array is [10, 15, 16, 17, 19]
 *      the ranges is 10-10, 15-17, 19-19
 *
 * a     [IN] sorted integer array
 * n     [IN] the array number
 * range [OUT] the range array we get from the sorted array
 *
 * ret: range number
 */
extern int getRange(int a[], int n, Range *range);

/*
 * brief: split a string by a sperator
 *     if the string is "4096-4096,4098-4101", separator is ","
 *     the result is "4096-4096", "4098-4101"
 *
 * src [IN] the input string, cannot be a string literal
 * sep [IN] the separator
 * dest [OUT] the output string array
 * num  [OUT] the output string number
 */
extern void split(char *src, const char *sep, char dest[][MAX_RANGE_CHARACTER], int* num);

/*
 * brief: remove the beginning and ending spaces
 *     if input string is " 4096-4096  "
 *     the output string is "4096-4096"
 *
 * s  [IN] the input string
 *
 * ret: the handled string
 */
extern char* trim(char *s);

/*
 * brief: skip the beginning and ending quote
 *   if input string is "\"4096-4096\""
 *   the out put string is "4096-4096"
 *
 *   s [IN] the input string
 *
 *   ret: the handled string
 */
extern char* skipQuote(char *s);

/*
 * brief: get the ranges from md cateogry or language string
 *       if category string is "\"4096-4096\","\"4098-4101\""
 *       the range should be 4096-4096 and 4098-4101
 *
 *  category [IN] the category or lanuage string
 *  r        [OUT] the ranges
 *
 *  ret: the range number
 */
extern int getRangeFromModem(char *cateogry, Range *r);