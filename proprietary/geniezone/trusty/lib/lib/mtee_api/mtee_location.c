/*
 * Copyright (C) 2012-2014 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>
#include <trusty_std.h>
#include "tz_private/sys_mem.h"
#include <tz_private/system.h>
#include <tz_private/log.h>
#include <lib/mtee/mtee_sys.h>

#include <lib/mtee/mtee_location.h>
#include <tz_private/ta_sys_mem.h>



#define LOG_TAG "MTEE Location API"

// static char *test_ = "+ECELL: 2,7,\"0339490D\",\"2DB4\",466,92,488,44,18,-388,-44,2,,,,1750,7,\"0FFFFFFF\",\"FFFF\",,,183,44,17,-388,-46,,,,,1750";

void resetCellInfoList(MTEE_CellInfoList *cell)
{
    cell->cell1.cellInfoType = MTEE_CELL_INFO_TYPE_UNKNOWN;
    cell->cell2.cellInfoType = MTEE_CELL_INFO_TYPE_UNKNOWN;
}

MTEE_CellInfo_Handle_t *cellInfo_handle;

extern TZ_RESULT MTEE_DumpAppMMU(void);

TZ_RESULT read_ecell(char **s) {
    SHM_Info *shm;
	int i, j;
	uint64_t *buffer;
	cellinfo_buffer_t cellinfoBuf;

	MTEE_GetCellInfoBuf(&cellinfoBuf);
	DBG_LOG("cellinfo buffer: handle = 0x%llx, size = 0x%llx\n", cellinfoBuf.mem_handle, cellinfoBuf.size);

	cellInfo_handle = MTEE_OpenCellInfo(&cellinfoBuf);
	if (cellInfo_handle == NULL) {
		DBG_LOG("cell info open fail!\n");
		return TZ_RESULT_ERROR_GENERIC;
	}
	DBG_LOG("cellinfo buffer: VA = 0x%llx, size = 0x%lx\n",
		(uint64_t)cellInfo_handle->info.buffer, cellInfo_handle->info.size);

	MTEE_CleanDcacheAll();

	MTEE_DumpAppMMU();

    // cast the buffer to the SHM_Info
    shm = (SHM_Info*)cellInfo_handle->info.buffer;

	buffer = shm;

#if 0 // test only, FIXME!!!
	printf("===>0x%llx\n", (uint64_t)buffer);
	for(i = 0; i < 128; i ++) {
		for (j = 0; j < 64; j ++)
			printf("%llx", *buffer++);
		printf("\n");
	}
#endif

    // check this SHM is valid.
    DBG_LOG("read_ecell - get magic number %d\n", shm->magic_numbe);
    if (shm->magic_numbe != 23023) return TZ_RESULT_ERROR_GENERIC;

    uint32_t last = shm->last_index;
    DBG_LOG("read_ecell - get last index %d\n", last);
    if (!((last >= 0) && (last <= 9))) {
        return TZ_RESULT_ERROR_GENERIC;
    }
    *s = shm->string[last];
    DBG_LOG("read_ecell - get last string %s\n", (*s));

    return TZ_RESULT_SUCCESS;
}

TZ_RESULT skipPrefix(char **s) {
    char *sub_s = NULL;
    sub_s = strstr(*s, "ECELL");
    if (sub_s == NULL) {
        DBG_LOG("skipPrefix can not find ECELL in %s\n", *s);
        return TZ_RESULT_ERROR_GENERIC;
    } else {
        *s = sub_s + sizeof(char)*strlen("ECELL"); //point to the next char right after the ECELL
        if (**s == ':') {
            *s = *s + 1;
        }
    }
    return TZ_RESULT_SUCCESS;
}

int checkAlphaNumber(char s)
{
    // DBG_LOG("checkAlphaNumber %c\n", s);
    if ((s >= 'a' && s <= 'z') ||
        (s >= 'A' && s <= 'Z') ||
        (s >= '0' && s <= '9') ||
        (s == '-')) {
        return 1;
    } else return 0;
}

TZ_RESULT getNext(char **s, char* tmp, int* l) {
    // DBG_LOG("getNext tmp point to %x\n", tmp);
    char * m_s = NULL;
    int m_l = 0;
    int has_quotation = 0;
    // skip white space
    while (**s == ' ') *s = *s + 1;

    // check quotation
    if (**s == '"') {
        // DBG_LOG("getNext - found quotation\n");
        has_quotation = 1;
        *s = *s + 1;
    }
    // get content
    while (checkAlphaNumber(**s) == 1) {
        if (m_l == 0) m_s = *s;
        m_l = m_l + 1;
        *s = *s + 1;
    }
    // check the paired quotation
    if (has_quotation == 1) {
        if (**s == '"') {
            // DBG_LOG("getNext - get paired quotation\n");
            *s = *s + 1;
        } else {
            DBG_LOG("getNext - format error, can not find quotation\n");
            return TZ_RESULT_ERROR_GENERIC;
        }
    }

    // go to the next one
    if (**s == ',') *s = *s + 1;
    else if (**s == NULL || **s == '\0') DBG_LOG("getNext - no more\n");

    if (m_l == 0) {
        tmp[0] = '\0';
        *l = 0;
        return TZ_RESULT_SUCCESS;
    } else {
        strncpy(tmp, m_s, m_l); // provide the string head
        tmp[m_l] = '\0';
        *l = m_l; // provide the string length
        return TZ_RESULT_SUCCESS;
    }
}

TZ_RESULT UTEE_AppendSecureMultichunkmem(MTEE_SESSION_HANDLE session,
					MTEE_SHAREDMEM_HANDLE *cm_handle,
					uint64_t pa, uint32_t size);

TZ_RESULT UTEE_AllocSecureMultichunkmem(
				MTEE_SESSION_HANDLE session,
				MTEE_SHAREDMEM_HANDLE chm_handle,
				MTEE_SECUREMEM_HANDLE *mem_handle,
				uint32_t alignment, uint32_t size);



static int cellinfo_open_once = 0;
static MTEE_CellInfo_Handle_t MTEE_CellInfo_handle; // one cell info instance

#ifndef __NEBULA_HEE__
#define MEM_SRV "com.mediatek.geniezone.srv.mem"
#else
#define MEM_SRV "nebula.com.mediatek.geniezone.srv.mem"
#endif

MTEE_CellInfo_Handle_t *MTEE_OpenCellInfo(cellinfo_buffer_t *cellinfo_buffer)
{
	int ret;
	MTEE_MEM_PARAM mem_param;
	uint64_t ta_key = 0;

	ret = TA_Mem_QueryChunkmem (cellinfo_buffer->mem_handle, &mem_param, ta_key);
	if (ret != TZ_RESULT_SUCCESS) {
		DBG_LOG("TA_Mem_QueryChunkmem Fail. ret=0x%x\n", ret);
		return NULL;
	}
	DBG_LOG("cellinfo va = 0x%llx, size = 0x%lx\n", (uint64_t)mem_param.buffer, mem_param.size);

	MTEE_CellInfo_handle.info.buffer = mem_param.buffer;
	MTEE_CellInfo_handle.info.size = mem_param.size;

	return &MTEE_CellInfo_handle;
}

TZ_RESULT MTEE_CloseCellInfo(MTEE_CellInfo_Handle_t *handle)
{
	int ret;

	WARN_LOG("%s not supported!!!\n", __func__);

	return TZ_RESULT_SUCCESS;
}

TZ_RESULT MTEE_GetCellInfo(MTEE_CellInfoList *cell)
{

    char *ecell_string;
    int num = 0;
    int len = 0;
    char tmp[64];
    int INVALID = 0x7FFFFFFF;
    int act = 0;
    // GSM
    int cid = INVALID;
    int lacTac = INVALID;
    int mcc = INVALID;
    int mnc = INVALID;
    int pscPci = INVALID;
    int sig1 = INVALID;
    int sig2 = INVALID;
    int rsrp = INVALID;
    int rsrq = INVALID;
    int timingAdvance = INVALID;
    int rssnr = INVALID;
    int cqi = INVALID;
    int bsic = 0xFF;
    int arfcn = INVALID;
    // C2K
    int nid = INVALID;
    int sid = INVALID;
    int base_station_id = INVALID;
    int base_station_longitude = INVALID;
    int base_station_latitude = INVALID;
    int cdma_dbm = INVALID;
    int cdma_ecio = INVALID;
    int evdo_dbm = INVALID;
    int evdo_ecio = INVALID;
    int snr = INVALID;
    int evdo_snr = INVALID;

    // DBG_LOG("tmp start at %x\n", &(tmp[0]));
    DBG_LOG("MTEE_GetCellInfo -- START!!!\n");
    if (cell == NULL || sizeof(*cell) != sizeof(MTEE_CellInfoList)) goto error;

    // initial
    resetCellInfoList(cell);

    // get ecell string
    if(read_ecell(&ecell_string) != TZ_RESULT_SUCCESS) {
        DBG_LOG("MTEE_GetCellInfo -- read_ecell fail\n");
        return TZ_RESULT_ERROR_GENERIC;
    }
    DBG_LOG("After read_ecell get %s\n", ecell_string);
    // skip prefix
    if(skipPrefix(&ecell_string) != TZ_RESULT_SUCCESS) {
        DBG_LOG("MTEE_GetCellInfo -- skipPrefix fail\n");
        return TZ_RESULT_ERROR_GENERIC;
    }
    // DBG_LOG("After skipPrefix get %s\n", ecell_string);

    // get num of cell list
    if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
    // DBG_LOG("After getNext get %s\n", ecell_string);

    if (len != 0) {
        // DBG_LOG("get next = %s, len =%d\n", tmp, len);
        num = atoi(tmp);
    }
    // DBG_LOG("After getNext for num = %d\n", num);

    int i;
    for (i = 0; i < num; i = i+1) {
        // get act
        if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
        // DBG_LOG("After getNext get %s\n", ecell_string);
        if (len != 0) {
            // DBG_LOG("get next = %s, len =%d\n", tmp, len);
            act = atoi(tmp);
        }
        // DBG_LOG("After getNext for act = %d\n", act);

        if (act == 0 || act == 2 || act == 7) {
            if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
            // DBG_LOG("After getNext get %s\n", ecell_string);
            if (len != 0) {
                // DBG_LOG("get next = %s, len =%d\n", tmp, len);
                cid = (int)strtoul(tmp, &tmp[len], 16); // convert Hex to long
            }
            // DBG_LOG("After getNext for cid = %d\n", cid);

            if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
            // DBG_LOG("After getNext get %s\n", ecell_string);
            if (len != 0) {
                // DBG_LOG("get next = %s, len =%d\n", tmp, len);
                lacTac = strtoul(tmp, &tmp[len], 16); // convert Hex to long
            }
            // DBG_LOG("After getNext for lacTac = %d\n", lacTac);

            if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
            // DBG_LOG("After getNext get %s\n", ecell_string);
            if (len != 0) {
                // DBG_LOG("get next = %s, len =%d\n", tmp, len);
                mcc = atoi(tmp);
            }
            // DBG_LOG("After getNext for mcc = %d\n", mcc);

            if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
            // DBG_LOG("After getNext get %s\n", ecell_string);
            if (len != 0) {
                // DBG_LOG("get next = %s, len =%d\n", tmp, len);
                mnc = atoi(tmp);
            }
            // DBG_LOG("After getNext for mnc = %d\n", mnc);

            if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
            // DBG_LOG("After getNext get %s\n", ecell_string);
            if (len != 0) {
                // DBG_LOG("get next = %s, len =%d\n", tmp, len);
                pscPci = atoi(tmp);
            }
            // DBG_LOG("After getNext for pscPci = %d\n", pscPci);

            if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
            // DBG_LOG("After getNext get %s\n", ecell_string);
            if (len != 0) {
                // DBG_LOG("get next = %s, len =%d\n", tmp, len);
                sig1 = atoi(tmp);
            }
            // DBG_LOG("After getNext for sig1 = %d\n", sig1);

            if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
            // DBG_LOG("After getNext get %s\n", ecell_string);
            if (len != 0) {
                // DBG_LOG("get next = %s, len =%d\n", tmp, len);
                sig2 = atoi(tmp);
            }
            // DBG_LOG("After getNext for sig2 = %d\n", sig2);

            if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
            // DBG_LOG("After getNext get %s\n", ecell_string);
            if (len != 0) {
                // DBG_LOG("get next = %s, len =%d\n", tmp, len);
                rsrp = atoi(tmp);
            }
            // DBG_LOG("After getNext for rsrp = %d\n", rsrp);

            if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
            // DBG_LOG("After getNext get %s\n", ecell_string);
            if (len != 0) {
                // DBG_LOG("get next = %s, len =%d\n", tmp, len);
                rsrq = atoi(tmp);
            }
            // DBG_LOG("After getNext for rsrq = %d\n", rsrq);

            if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
            // DBG_LOG("After getNext get %s\n", ecell_string);
            if (len != 0) {
                // DBG_LOG("get next = %s, len =%d\n", tmp, len);
                timingAdvance = atoi(tmp);
            }
            // DBG_LOG("After getNext for timingAdvance = %d\n", timingAdvance);

            if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
            // DBG_LOG("After getNext get %s\n", ecell_string);
            if (len != 0) {
                // DBG_LOG("get next = %s, len =%d\n", tmp, len);
                rssnr = atoi(tmp);
            }
            // DBG_LOG("After getNext for rssnr = %d\n", rssnr);

            if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
            // DBG_LOG("After getNext get %s\n", ecell_string);
            if (len != 0) {
                // DBG_LOG("get next = %s, len =%d\n", tmp, len);
                cqi = atoi(tmp);
            }
            // DBG_LOG("After getNext for cqi = %d\n", cqi);

            if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
            // DBG_LOG("After getNext get %s\n", ecell_string);
            if (len != 0) {
                // DBG_LOG("get next = %s, len =%d\n", tmp, len);
                bsic = atoi(tmp);
            }
            // DBG_LOG("After getNext for bsic = %d\n", bsic);

            if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
            // DBG_LOG("After getNext get %s\n", ecell_string);
            if (len != 0) {
                // DBG_LOG("get next = %s, len =%d\n", tmp, len);
                arfcn = atoi(tmp);
            }
            // DBG_LOG("After getNext for arfcn = %d\n", arfcn);

            DBG_LOG("act=%d,cid=%d,mcc=%d,mnc=%d,lacTac=%d,pscPci=%d,sig1=%d,sig2=%d,"
                    "sig1_dbm=%d,sig1_dbm=%d,ta=%d,rssnr=%d,cqi=%d,bsic=%d,arfcn=%d\n",
                    act, cid, mcc, mnc, lacTac, pscPci, sig1, sig2, rsrp, rsrq,
                    timingAdvance, rssnr, cqi, bsic, arfcn);
        } else if (act == 256) {
            if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
            if (len != 0) {
                nid = atoi(tmp);
            }

            if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
            if (len != 0) {
                sid = atoi(tmp);
            }

            if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
            if (len != 0) {
                base_station_id = atoi(tmp);
            }

            if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
            if (len != 0) {
                base_station_longitude = atoi(tmp);
            }

            if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
            if (len != 0) {
                base_station_latitude = atoi(tmp);
            }

            if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
            if (len != 0) {
                cdma_dbm = atoi(tmp);
                if (cdma_dbm < 0  || cdma_dbm > 31) {
                    cdma_dbm = INVALID;
                }
            }

            if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
            if (len != 0) {
                cdma_ecio = atoi(tmp);
                if (cdma_ecio < -128 || cdma_ecio > 0) {
                    cdma_ecio = INVALID;
                }
            }

            if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
            if (len != 0) {
                evdo_dbm = atoi(tmp);
                if (evdo_dbm < 0  || evdo_dbm > 31) {
                    evdo_dbm = INVALID;
                }
            }

            if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
            if (len != 0) {
                evdo_ecio = atoi(tmp);
                if (evdo_ecio < -512 || evdo_ecio > 0) {
                    evdo_ecio = INVALID;
                }
            }

            if (getNext(&ecell_string, tmp, &len) != TZ_RESULT_SUCCESS) goto error;
            if (len != 0) {
                snr = atoi(tmp);
            }

            DBG_LOG("nid=%d,sid=%d,base_station_id=%d,"
                    "base_station_longitude=%d,base_station_latitude=%d,"
                    "cdma_dbm=%d,cdma_ecio=%d,evdo_dbm=%d,evdo_ecio=%d,"
                    "snr=%d,evdo_snr=%d\n",
                    nid, sid, base_station_id, base_station_longitude,
                    base_station_latitude, cdma_dbm, cdma_ecio, evdo_dbm,
                    evdo_ecio, snr, evdo_snr);
        }
        /* <Act>  0: GSM , 2: UMTS , 7: LTE, 256: 1x */
        if (act == 0 || act == 2 || act == 7) {
            // if we already has a GSM cell info, skip others.
            if (cell->cell1.cellInfoType != MTEE_CELL_INFO_TYPE_UNKNOWN)
                continue;
            if(act == 7) {
                cell->cell1.cellInfoType = MTEE_CELL_INFO_TYPE_LTE;
                cell->cell1.CellInfo.lte.ci = cid;
                cell->cell1.CellInfo.lte.mcc = mcc;
                cell->cell1.CellInfo.lte.mnc = mnc;
                cell->cell1.CellInfo.lte.tac = lacTac;
                cell->cell1.CellInfo.lte.pci = pscPci;
                cell->cell1.CellInfo.lte.earfcn = arfcn;
            } else if(act == 2) {
                cell->cell1.cellInfoType = MTEE_CELL_INFO_TYPE_WCDMA;
                cell->cell1.CellInfo.wcdma.cid = cid;
                cell->cell1.CellInfo.wcdma.mcc = mcc;
                cell->cell1.CellInfo.wcdma.mnc = mnc;
                cell->cell1.CellInfo.wcdma.lac = lacTac;
                cell->cell1.CellInfo.wcdma.psc = pscPci;
                cell->cell1.CellInfo.wcdma.uarfcn = arfcn;
            } else if (act == 0) {
                cell->cell1.cellInfoType = MTEE_CELL_INFO_TYPE_GSM;
                cell->cell1.CellInfo.gsm.cid = cid;
                cell->cell1.CellInfo.gsm.mcc = mcc;
                cell->cell1.CellInfo.gsm.mnc = mnc;
                cell->cell1.CellInfo.gsm.lac = lacTac;
                cell->cell1.CellInfo.gsm.arfcn = arfcn;
                cell->cell1.CellInfo.gsm.bsic = bsic;
            }
        } else if (act == 256) {
            // if we already has a CDMA cell info, skip others.
            if (cell->cell2.cellInfoType != MTEE_CELL_INFO_TYPE_UNKNOWN)

            cell->cell2.cellInfoType = MTEE_CELL_INFO_TYPE_CDMA;
            cell->cell2.CellInfo.cdma.networkId = nid;
            cell->cell2.CellInfo.cdma.systemId = sid;
            cell->cell2.CellInfo.cdma.basestationId = base_station_id;
            cell->cell2.CellInfo.cdma.longitude = base_station_longitude;
            cell->cell2.CellInfo.cdma.latitude = base_station_latitude;
        } else {
            DBG_LOG("RIL_CELL_INFO_TYPE invalid act=%d\n", act);
        }
    }
    DBG_LOG("MTEE_GetCellInfo -- END!!!\n");
    return TZ_RESULT_SUCCESS;
error:
    DBG_LOG("MTEE_GetCellInfo -- ERROR!!!\n");
    return TZ_RESULT_ERROR_GENERIC;
}
