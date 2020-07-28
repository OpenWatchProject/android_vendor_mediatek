
#ifndef _MERESULT_H
#define _MERESULT_H

#include <vector>
#include <string>

using namespace std;

typedef enum err_code_t
{
	// +cme error:
	ER_OK			= 0,
	ER_NOALLOW		= 3,
	ER_NOSUPPORT	= 4,
	ER_NOSIM		= 10,
	ER_PINREQ		= 11,
	ER_PUKREQ		= 12,
	ER_SIMFAIL		= 13,
	ER_SIMBUSY		= 14,
	ER_SIMWRONG		= 15,
	ER_WRONGPWD		= 16,
	ER_PIN2REQ		= 17,
	ER_PUK2REQ		= 18,
	ER_BADINDEX		= 21,
	ER_LONGTEXT		= 24,
	ER_BADTEXT		= 25,
	ER_LONGNUM		= 26,
	ER_BADNUM		= 27,
	ER_NOSERVICE	= 30,
	ER_EMCALLONLY	= 32,
	ER_UNKNOWN		= 100,
	ER_TRYLATER		= 256,
	ER_CALLBARRED	= 257,
	ER_SSNOTEXE		= 261,
	ER_SIMBLOCK		= 262,
	
	// todo: +cms error:

	ER_TIMEOUT		= 0x7f00,
	ER_NOCONNECTION,
	ER_USERABORT,
	ER_CMEERROR
	
}ERR_CODE;

typedef enum AtParaTypeTag
{	
	AT_OMIT,
	AT_STRING,
	AT_INTERVAL,
	AT_INTEGER,
	AT_PARA_LIST
}
AtParaType;

class ATParamElem
{
public:
	AtParaType  type;
	string		str_value;				//string type, maybe pdu
	int			int_value;				//value type
	int int_range_begin, int_range_end;	//range type
   
	vector<ATParamElem> paramLst;		//vector type
public:
	ATParamElem();
	~ATParamElem();

};


class ATParamLst
{
public:
	vector<ATParamElem> eleLst;

public:
	ATParamLst();
	~ATParamLst();

};


class ATResult
{
public:
	int retType;
	int urcId;
	vector<ATParamLst> resultLst;  //  two-dimensional array

public:

	ATResult();
	~ATResult();

	void clear(void);
	
	int  get_integer(int line, int row) const;
	int check_key(const char* szKey) const;
	int get_string(char* str, int line, int row) const;
	int get_string(string& str, int line, int row) const;
};


#endif
