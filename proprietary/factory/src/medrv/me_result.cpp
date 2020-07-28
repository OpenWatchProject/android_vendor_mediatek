#include "me_result.h"

ATParamElem::ATParamElem()
{

}


ATParamElem::~ATParamElem()
{

}

ATParamLst::ATParamLst()
{
}

ATParamLst::~ATParamLst()
{
}


ATResult::ATResult()
{
  this->retType = 0;
  this->urcId = 0xff;
}


ATResult::~ATResult()
{

}

void ATResult::clear(void)
{
	resultLst.clear();
	this->retType = 0;
}

int ATResult::check_key(const char* szKey) const
{
	if (NULL == szKey)
		return 0;

	string key;
	get_string(key, 0, 0);
	return (key == szKey);
}

int ATResult::get_string(string& str, int line, int row) const
{
	if (line >= (int)resultLst.size())
		return 0;
	if (row >= (int)resultLst[line].eleLst.size())
		return 0;
	if (resultLst[line].eleLst[row].type != AT_STRING)
		return 0;

	str = resultLst[line].eleLst[row].str_value;
	return 1;
}

int ATResult::get_string(char* str, int line, int row) const
{
	if (line >= (int)resultLst.size())
		return 0;
	if (row >= (int)resultLst[line].eleLst.size())
		return 0;
	if (resultLst[line].eleLst[row].type != AT_STRING)
		return 0;

	strncpy(str, resultLst[line].eleLst[row].str_value.c_str(), strlen(resultLst[line].eleLst[row].str_value.c_str())); // to unicode
	return 1;
}

int ATResult::get_integer(int line, int row) const
{
	if (line >= (int)resultLst.size())
		return -1;
	if (row >= (int)resultLst[line].eleLst.size())
		return -1;
	if (resultLst[line].eleLst[row].type != AT_INTEGER)
		return -1;

	return resultLst[line].eleLst[row].int_value;
}
