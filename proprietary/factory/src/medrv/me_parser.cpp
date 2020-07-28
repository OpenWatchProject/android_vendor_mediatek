#include "me_comm.h"
#include "me_result.h"
#include "me_parser.h"

////////////////////////////////////////////////////////////////////////
//response parser
static void AddExpectedCmd(void *pfsm, ATParamLst &paraLst)
{
	ATParamElem elem;

	elem.type = AT_STRING;
	elem.str_value = string(((PMEFSM)pfsm)->req.expectedCmd);
	paraLst.eleLst.push_back(elem);
}

void ME_Query_FWVersion_Parser(void *pfsm, void *pObj)
{
	char *rawdata = ((PMEFSM)pfsm)->ctldata;

	char buf[128];
	ATParamElem elem;
	ATParamLst paraLst;
	ATResult *pRet = (ATResult*)((PMEFSM)pfsm)->req.context;
	if (pObj == NULL) // reserved parameter, for resolving warning
	{
        ME_Print(ME_TAG "[Medrv] The parameter is NULL......");
	}

	AddExpectedCmd(pfsm, paraLst);

	elem.type = AT_STRING;
	GetString(rawdata, buf, GetSignIdx(rawdata, ':', 1), me_strlen(rawdata));
	elem.str_value = string(buf);
	paraLst.eleLst.push_back(elem);

	pRet->resultLst.push_back(paraLst);

}

void ME_Query_SIMStatus_Parser(void *pfsm, void *pObj)
{
	int begin;
	char *rawdata = ((PMEFSM)pfsm)->ctldata;

	ATParamElem elem;
	ATParamLst paraLst;
	ATResult *pRet = (ATResult*)((PMEFSM)pfsm)->req.context;
	if (pObj == NULL) // reserved parameter, for resolving warning
	{
        ME_Print(ME_TAG "[Medrv] The parameter is NULL......");
	}

	AddExpectedCmd(pfsm, paraLst);

	begin = GetSignIdx(rawdata, ':', 1);
	elem.type = AT_INTEGER;
	elem.int_value = GetInteger(rawdata, begin, me_strlen(rawdata));
	paraLst.eleLst.push_back(elem);

	pRet->resultLst.push_back(paraLst);
}

void ME_Query_IMSI_Parser(void *pfsm, void *pObj)
{
	char *rawdata = ((PMEFSM)pfsm)->ctldata;

	ATParamElem elem;
	ATParamLst paraLst;
	ATResult *pRet = (ATResult*)((PMEFSM)pfsm)->req.context;
	if (pObj == NULL) // reserved parameter, for resolving warning
	{
        ME_Print(ME_TAG "[Medrv] The parameter is NULL......");
	}

	elem.type = AT_STRING;
	elem.str_value = string(rawdata);
	paraLst.eleLst.push_back(elem);

	pRet->resultLst.push_back(paraLst);

}

void ME_Query_Revision_Parser(void *pfsm, void *pObj)
{
	int begin, end;
	char revision[128] = {0};
	char *rawdata = ((PMEFSM)pfsm)->ctldata;

	ATParamElem elem;
	ATParamLst paraLst;
	ATResult *pRet = (ATResult*)((PMEFSM)pfsm)->req.context;
	if (pObj == NULL) // reserved parameter, for resolving warning
	{
        ME_Print(ME_TAG "[Medrv] The parameter is NULL......");
	}

	AddExpectedCmd(pfsm, paraLst);

	begin = GetSignIdx(rawdata, '\"', 1);
	end   = GetSignIdx(rawdata, '\"', 2);
	GetString(rawdata, revision, begin, end);
	elem.type = AT_STRING;
	elem.str_value = string(revision);
	paraLst.eleLst.push_back(elem);

	pRet->resultLst.push_back(paraLst);
}


void ME_Query_Signal_Parser(void *pfsm, void *pObj)
{
	//+CSQ: <rssi>, <ber>
	int begin, end;
	char *rawdata = ((PMEFSM)pfsm)->ctldata;

	ATParamElem elem;
	ATParamLst paraLst;
	ATResult *pRet = (ATResult*)((PMEFSM)pfsm)->req.context;
	if (pObj == NULL) // reserved parameter, for resolving warning
	{
        ME_Print(ME_TAG "[Medrv] The parameter is NULL......");
	}

	AddExpectedCmd(pfsm, paraLst);

	begin = GetSignIdx(rawdata, ':', 1);
	end	  = GetSignIdx(rawdata, ',', 1);
	elem.type = AT_INTEGER;
	elem.int_value = GetInteger(rawdata, begin, end);
	paraLst.eleLst.push_back(elem);

	elem.type = AT_INTEGER;
	if(GetSignIdx(rawdata, ',', 2) < 0)
		elem.int_value = GetInteger(rawdata, end, me_strlen(rawdata));
	else
		elem.int_value = GetInteger(rawdata, end, GetSignIdx(rawdata, ',', 2));
	paraLst.eleLst.push_back(elem);

	pRet->resultLst.push_back(paraLst);

}

//response parser
////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////
//urc parser
int ME_URC_ESIMS_Parser(void *pData)
{
	//+ESIMS: <SIM_INSERTED>				  //AT response
	//+ESIMS: <SIM_INSERTED>,<LOCK_REQUIRED>  //URC

	PMEFSM pfsm = NULL;
	pfsm = (PMEFSM)pData;

	if(me_memcmp("+ESIMS", pfsm->ctldata, me_strlen("+ESIMS")) == 0)
	{
		if(GetSignCount(pfsm->ctldata, ',') != 1)
			return 0;  // it is AT response

		if(pfsm->urclist[ID_ESIMS].reply == NULL)
			return 1;  //not register, return directly


		ATResult atret;
		// in the future ,the owner need to parse
		atret.urcId = ID_ESIMS;
		pfsm->urclist[ID_ESIMS].reply((void*)&atret);


		return 1;
	}

	return 0;
}

void ME_Query_3G_Status_Parser(void *pfsm, void *pObj)
{
	int begin;
	char *rawdata = ((PMEFSM)pfsm)->ctldata;

	ATParamElem elem;
	ATParamLst paraLst;
	ATResult *pRet = (ATResult*)((PMEFSM)pfsm)->req.context;
	if (pObj == NULL) // reserved parameter, for resolving warning
	{
        ME_Print(ME_TAG "[Medrv] The parameter is NULL......");
	}

	AddExpectedCmd(pfsm, paraLst);

	begin = GetSignIdx(rawdata, ':', 1);
	elem.type = AT_INTEGER;
	elem.int_value = GetInteger(rawdata, begin, me_strlen(rawdata));
	paraLst.eleLst.push_back(elem);

	pRet->resultLst.push_back(paraLst);
}

int ME_URC_EUSIM_Parser(void *pData)
{
    //+EUSIM: <SIM_INSERTED>				  //AT response
    //+EUSIM: <SIM_INSERTED>,<LOCK_REQUIRED>  //URC

    PMEFSM pfsm = NULL;
    pfsm = (PMEFSM)pData;

    if(me_memcmp("+EUSIM", pfsm->ctldata, me_strlen("+EUSIM")) == 0)
    {
        if(GetSignCount(pfsm->ctldata, ',') != 1)
            return 0;  // it is AT response

		if(pfsm->urclist[ID_EUSIM].reply == NULL)
			return 1;  //not register, return directly


		ATResult atret;
		// in the future ,the owner need to parse
		atret.urcId = ID_EUSIM;
		pfsm->urclist[ID_EUSIM].reply((void*)&atret);

		return 1;
	}

	return 0;
}

int ME_URC_ECSQ_Parser(void *pData)
{
	// +ECSQ: <flag>		 //AT response
	// +ECSQ: <rssi>, <ber>  //URC

	int begin, end;
	PMEFSM pfsm = NULL;


	pfsm = (PMEFSM)pData;
	if(me_memcmp("+ECSQ", pfsm->ctldata, me_strlen("+ECSQ")) == 0)
	{
		if(GetSignCount(pfsm->ctldata, ',') == 0)
			return 0;  // it is AT response

		if(pfsm->urclist[ID_ECSQ].reply == NULL)
			return 1;

		ATResult atret;
		ATParamElem elem;
		ATParamLst paraLst;
		char *rawdata = ((PMEFSM)pfsm)->ctldata;

		elem.type = AT_STRING;
		elem.str_value = string("+ECSQ");
		paraLst.eleLst.push_back(elem);

		begin = GetSignIdx(rawdata, ':', 1);
		end	  = GetSignIdx(rawdata, ',', 1);
		elem.type = AT_INTEGER;
		elem.int_value = GetInteger(rawdata, begin, end);
		paraLst.eleLst.push_back(elem);

		begin = end;
		elem.type = AT_INTEGER;
		if(GetSignCount(rawdata, ',') > 1)
			end = GetSignIdx(rawdata, ',', 2);
		else
			end = me_strlen(rawdata);
		elem.int_value = GetInteger(rawdata, begin, end);
		paraLst.eleLst.push_back(elem);

		atret.resultLst.push_back(paraLst);
		atret.urcId = ID_ECSQ;
		pfsm->urclist[ID_ECSQ].reply((void*)&atret);

		return 1;
	}

	return 0;
}

int ME_URC_Network_Parser(int id, void *pData)
{
	//+CREG: <n>,<stat>[,<lac>,<ci>[,<act>]]	//AT response
	//+CREG: <stat>[,<lac>,<ci>[,<act>]]		//URC

	//+CGREG: <n>,<stat>[,<lac>,<ci>[,<act>]]   //AT response
	//+CGREG: <stat>[,<lac>,<ci>[,<act>]]		//URC

	int count, begin, end;
	PMEFSM pfsm = NULL;

	char urc[8];

	if(id == ID_CREG)
		me_strcpy(urc, "+CREG");
	else
		me_strcpy(urc, "+CGREG");

	pfsm = (PMEFSM)pData;
	if(me_memcmp(urc, pfsm->ctldata, me_strlen(urc)) == 0)
	{
		count = GetSignCount(pfsm->ctldata, ',');
		if((count == 1)||(count == 4))  // it is AT response
			return 0;

		if(count == 3)
		{
			begin = GetSignIdx(pfsm->ctldata, ',', 3);
                        if(begin != -1)
                        {
			    count = GetSignCount(pfsm->ctldata+begin, '\"');
                        }
			if(count > 1) //<ci> is a string parameter, it has quotation marks.
				return 0; // if <ci> is fourth parameter, it is AT response.
		}

		if(pfsm->urclist[id].reply == NULL)
			return 1;

		char buf[80];
		ATResult atret;
		ATParamElem elem;
		ATParamLst paraLst;
		char *rawdata = ((PMEFSM)pfsm)->ctldata;

		elem.type = AT_STRING;
		if(id == ID_CREG)
			elem.str_value = string("+CREG");
		else
			elem.str_value = string("+CGREG");
		paraLst.eleLst.push_back(elem);

		begin = GetSignIdx(pfsm->ctldata, ':', 1);
		end	  = GetSignIdx(rawdata, ',', 1);
		if(end == -1)
			end = me_strlen(rawdata);
		elem.type = AT_INTEGER;
		elem.int_value = GetInteger(rawdata, begin, end);
		paraLst.eleLst.push_back(elem);

		begin = GetSignIdx(rawdata, '\"', 1);
		if(begin != -1)
		{
			elem.type = AT_STRING;
			GetString(rawdata, buf, begin,  GetSignIdx(rawdata, '\"', 2));
			elem.str_value = string(buf);
			paraLst.eleLst.push_back(elem);

			begin = GetSignIdx(rawdata, '\"', 3);
			if(begin != -1)
			{
				elem.type = AT_STRING;
				GetString(rawdata, buf, begin,  GetSignIdx(rawdata, '\"', 4));
				elem.str_value = string(buf);
				paraLst.eleLst.push_back(elem);
			}
		}

		begin = GetSignIdx(rawdata, ',', 3);
		if(begin != -1)
		{
			elem.type = AT_INTEGER;
			elem.int_value = GetInteger(rawdata, begin, me_strlen(rawdata));
			paraLst.eleLst.push_back(elem);
		}

		atret.resultLst.push_back(paraLst);
		atret.urcId = id;
		pfsm->urclist[id].reply((void*)&atret);

		return 1;
	}

	return 0;
}

int ME_URC_CREG_Parser(void *pData)
{
	return ME_URC_Network_Parser(ID_CREG, pData);
}

int ME_URC_CGREG_Parser(void *pData)
{
	return ME_URC_Network_Parser(ID_CGREG, pData);
}


int ME_URC_EIND_Parser(void *pData)
{
	//+EIND: <flag>

	int begin, end;
	PMEFSM pfsm = NULL;

	pfsm = (PMEFSM)pData;

	if(me_memcmp("+EIND", pfsm->ctldata, me_strlen("+EIND")) == 0)
	{
		if(pfsm->req.expectedCmd[0] != 0)
		{
			if(!me_memcmp(pfsm->req.expectedCmd, pfsm->ctldata, me_strlen(pfsm->req.expectedCmd)))
				return 0; //maybe it's AT+EIND? response.
		}

		if(pfsm->urclist[ID_EIND].reply == NULL)
			return 1;


		ATResult atret;
		ATParamElem elem;
		ATParamLst paraLst;
		char *rawdata = ((PMEFSM)pfsm)->ctldata;

		elem.type = AT_STRING;
		elem.str_value = string("+EIND");
		paraLst.eleLst.push_back(elem);

		begin = GetSignIdx(rawdata, ':', 1);
		elem.type = AT_INTEGER;
		if(GetSignCount(rawdata, ',') > 0)
			end = GetSignIdx(rawdata, ',', 1);
		else
			end = me_strlen(rawdata);
		elem.int_value = GetInteger(rawdata, begin, end);
		paraLst.eleLst.push_back(elem);

		atret.resultLst.push_back(paraLst);
		atret.urcId = ID_EIND;
		pfsm->urclist[ID_EIND].reply((void*)&atret);

		return 1;
	}

	return 0;
}

int ME_URC_RING_Parser(void *pData)
{
	PMEFSM pfsm = (PMEFSM)pData;

	if(me_memcmp("RING", pfsm->ctldata, me_strlen("RING")) == 0)
	{
		if(pfsm->urclist[ID_RING].reply == NULL)
			return 1;

		ATResult atret;
		ATParamElem elem;
		ATParamLst paraLst;

		elem.type = AT_STRING;
		elem.str_value = string("RING");
		paraLst.eleLst.push_back(elem);

		atret.resultLst.push_back(paraLst);
		atret.urcId = ID_RING;
		pfsm->urclist[ID_RING].reply((void*)&atret);

		return 1;
	}

	return 0;
}

int ME_URC_PSBEARER_Parser(void* pData)
{
	//+PSBEARER: <cell_data_speed_support>, <max_data_bearer_capability>
	int begin, end;
	PMEFSM pfsm = (PMEFSM)pData;
	char *rawdata = pfsm->ctldata;

	if(me_memcmp("+PSBEARER", pfsm->ctldata, me_strlen("+PSBEARER")) == 0)
	{
		if(pfsm->urclist[ID_PSBEARER].reply == NULL)
			return 1;

		ATResult atret;
		ATParamElem elem;
		ATParamLst paraLst;

		elem.type = AT_STRING;
		elem.str_value = string("+PSBEARER");
		paraLst.eleLst.push_back(elem);

		begin = GetSignIdx(rawdata, ':', 1);
		end   = GetSignIdx(rawdata, ',', 1);
		elem.type = AT_INTEGER;
		elem.int_value = GetInteger(rawdata, begin, end);
		paraLst.eleLst.push_back(elem);

		elem.type = AT_INTEGER;
		elem.int_value = GetInteger(rawdata, end, me_strlen(rawdata));
		paraLst.eleLst.push_back(elem);

		atret.resultLst.push_back(paraLst);
		atret.urcId = ID_PSBEARER;
		pfsm->urclist[ID_PSBEARER].reply((void*)&atret);

		return 1;
	}

	return 0;
}

int ME_URC_ECPI_Parser(void *pData)
{
	//+ECPI:<call_id>, <msg_type>, <isibt>
	int begin, end;
	PMEFSM pfsm = (PMEFSM)pData;
	char *rawdata = pfsm->ctldata;

	if(me_memcmp("+ECPI", pfsm->ctldata, me_strlen("+ECPI")) == 0)
	{
		if(pfsm->urclist[ID_ECPI].reply == NULL)
			return 1;

		ATResult atret;
		ATParamElem elem;
		ATParamLst paraLst;

		elem.type = AT_STRING;
		elem.str_value = string("+ECPI");
		paraLst.eleLst.push_back(elem);

		begin = GetSignIdx(rawdata, ':', 1);
		end   = GetSignIdx(rawdata, ',', 1);
		elem.type = AT_INTEGER;
		elem.int_value = GetInteger(rawdata, begin, end);
		paraLst.eleLst.push_back(elem);

		begin = end;
		end = GetSignIdx(rawdata, ',', 2);
		elem.type = AT_INTEGER;
		elem.int_value = GetInteger(rawdata, begin, end);
		paraLst.eleLst.push_back(elem);

		atret.resultLst.push_back(paraLst);
		atret.urcId = ID_ECPI;
		pfsm->urclist[ID_ECPI].reply((void*)&atret);

		return 1;
	}

	return 0;
}

int ME_URC_ESPEECH_Parser(void* pData)
{
	//ESPEECH
	PMEFSM pfsm = (PMEFSM)pData;

	if(me_memcmp("ESPEECH", pfsm->ctldata, me_strlen("ESPEECH")) == 0)
	{
		if(pfsm->urclist[ID_ESPEECH].reply == NULL)
			return 1;

		ATResult atret;
		ATParamElem elem;
		ATParamLst paraLst;

		elem.type = AT_STRING;
		elem.str_value = string("ESPEECH");
		paraLst.eleLst.push_back(elem);

		atret.resultLst.push_back(paraLst);
		atret.urcId = ID_ESPEECH;
		pfsm->urclist[ID_ESPEECH].reply((void*)&atret);

		return 1;
	}

	return 0;
}

int ME_URC_CONN_Parser(void* pData)
{
	//^CONN
	PMEFSM pfsm = (PMEFSM)pData;

	if(me_memcmp("^CONN", pfsm->ctldata, me_strlen("^CONN")) == 0)
	{
		if(pfsm->urclist[ID_CONN].reply == NULL)
			return 1;

		ATResult atret;
		ATParamElem elem;
		ATParamLst paraLst;

		elem.type = AT_STRING;
		elem.str_value = string("^CONN");
		paraLst.eleLst.push_back(elem);

		atret.resultLst.push_back(paraLst);
		atret.urcId = ID_CONN;
		pfsm->urclist[ID_CONN].reply((void*)&atret);

		return 1;
	}

	return 0;
}

int ME_URC_VPON_Parser(void* pData)
{
	//^VPON
	PMEFSM pfsm = (PMEFSM)pData;

	if(me_memcmp("+VPON:1", pfsm->ctldata, me_strlen("VPON:1")) == 0)
	{
		if(pfsm->urclist[ID_VPON].reply == NULL)
			return 1;

		ATResult atret;
		ATParamElem elem;
		ATParamLst paraLst;

		elem.type = AT_STRING;
		elem.str_value = string("VPON:1");
		paraLst.eleLst.push_back(elem);

		atret.resultLst.push_back(paraLst);
		atret.urcId = ID_VPON;
		pfsm->urclist[ID_VPON].reply((void*)&atret);

		return 1;
	}

	return 0;
}

int ME_URC_VPUP_Parser(void* pData)
{
	//+VPUP
	PMEFSM pfsm = (PMEFSM)pData;

	if(me_memcmp("+VPUP", pfsm->ctldata, me_strlen("+VPUP")) == 0)
	{
		if(pfsm->urclist[ID_CONN].reply == NULL)
			return 1;

		ATResult atret;
		ATParamElem elem;
		ATParamLst paraLst;

		elem.type = AT_STRING;
		elem.str_value = string("+VPUP");
		paraLst.eleLst.push_back(elem);

		atret.resultLst.push_back(paraLst);
		atret.urcId = ID_VPUP;
		pfsm->urclist[ID_VPUP].reply((void*)&atret);

		return 1;
	}

	return 0;
}

int ME_URC_UIMST_Parser(void* pData)
{
	//UIMST:
	PMEFSM pfsm = (PMEFSM)pData;

	if(me_memcmp("UIMST:", pfsm->ctldata, me_strlen("UIMST:")) == 0)
	{
		if(pfsm->urclist[ID_CONN].reply == NULL)
			return 1;

		ATResult atret;
		ATParamElem elem;
		ATParamLst paraLst;

		elem.type = AT_STRING;
		elem.str_value = string("UIMST:");
		paraLst.eleLst.push_back(elem);

		atret.resultLst.push_back(paraLst);
		atret.urcId = ID_UIMST;
		pfsm->urclist[ID_VPUP].reply((void*)&atret);

		return 1;
	}

	return 0;
}
//urc parser
////////////////////////////////////////////////////////////////////////

