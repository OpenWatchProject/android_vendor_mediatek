#if MTK_CAM_NEW_NVRAM_SUPPORT
#define LOG_TAG "nvram_ut"

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <istream>
#include <vector>
#include <algorithm>
#include <assert.h>
#include <time.h>
#include <thread>
#include <utils/Mutex.h>
#include <cutils/properties.h>
#include <mtkcam/utils/std/Log.h>
#include "camera_custom_nvram.h"
#include <mtkcam/utils/mapping_mgr/cam_idx_mgr.h>
#include "tuning_mapping/cam_idx_struct_ext.h"
#include "tuning_mapping/cam_idx_struct_int.h"
#include <EModule_string.h>

using namespace std;
using namespace NSIspTuning;

typedef struct {
	std::vector<std::vector<unsigned short>> comb;
	unsigned short index;
} COMB_TYPE;

std::vector<COMB_TYPE>	goldenComb[EModule_NUM];

unsigned long long ut_ok_cnt = 0;
unsigned long long ut_ng_cnt = 0;

unsigned long long ut_ok_cnt_curMod[EModule_NUM] = {0};
unsigned long long ut_ng_cnt_curMod[EModule_NUM] = {0};
bool find_ambigious(unsigned short  val_golden, CAM_IDX_QRY_COMB& qry, int _CombIdx, int modIdx)
{
    bool bPreviousMatch = true;

    std::vector<COMB_TYPE>::iterator combIt;
    int CombIdx = 0;
    for (combIt = goldenComb[modIdx].begin(); combIt != goldenComb[modIdx].end();  combIt++)
    {
        if (val_golden != combIt->index && _CombIdx > CombIdx)
        {
            bPreviousMatch = true;
            for (int dim = 0; dim < EDim_NUM; dim++) //loop through all dim
            {
                unsigned short dim_in_qry =(unsigned short)qry.query[dim];
                for (int i = 0; i < combIt->comb[dim].size(); i++) //check each vector in inComb.comb[dim]
                {
                    unsigned short val = combIt->comb[dim][i];
                    int dim_size_in_comb = combIt->comb[dim].size();
                    if (val == (unsigned short)-1) //-1 , match all case
                        break;
                    if (val == dim_in_qry)
                        break;
                    if (i == (dim_size_in_comb - 1)) //last in dim, havn't break, no match, dim match fail
                        bPreviousMatch = false;
                }
                if (!bPreviousMatch) //no match in this dim, try next comb
                    break;
            }
            if (bPreviousMatch) //one comb is enough
                return true;
        }
        CombIdx++;
    }
    return false;
}
void do_DFS_userDefine(int dimIdx, COMB_TYPE& inComb, std::vector<unsigned short>& outComb, IdxMgr* pMgr, unsigned short modIdx, int CombIdx,int SensorDev)
{
	if (dimIdx < EDim_NUM)
	{
		for (int i = 0; i < inComb.comb[dimIdx].size(); i++)
		{
			unsigned short val = inComb.comb[dimIdx][i];
			if (val == (unsigned short)-1)
			{
				assert (inComb.comb[dimIdx].size() == 1);
				val = 0;
			}
			outComb.push_back(val);
			do_DFS_userDefine(dimIdx + 1, inComb, outComb, pMgr, modIdx, CombIdx, SensorDev);
			outComb.pop_back();
		}
	}
	else
	{
		unsigned short val_query;

		assert(outComb.size() == sizeof(ISP_IDX_QRY_COMB)/sizeof(int));

		CAM_IDX_QRY_COMB qry;
		int* out = (int*)(&qry);
		for (int i = 0; i < outComb.size(); i++)
		{
			out[i] = (int)outComb[i];
		}

		val_query = pMgr->query((MUINT32)SensorDev, (EModule_T)modIdx, qry, __FUNCTION__);

		unsigned short val_golden = inComb.index;


		if (val_query != val_golden && (!find_ambigious(val_golden, qry, CombIdx, modIdx)))
		{
            ut_ng_cnt_curMod[modIdx]++;

            CAM_LOGD("UT fail @ [");
            for (unsigned int i = 0; i < outComb.size(); i++)
            {
                CAM_LOGD("%d, ",outComb[i]);
            }
            CAM_LOGD(" for value (golden: %d, query :%d)\n", val_golden, val_query);
		}
		else
		{
			ut_ok_cnt_curMod[modIdx]++;
		}
	}
}

bool readGolden(std::string filename, unsigned int modIdx)
{
    char value_log[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.mapping_mgr.ut_log", value_log, "0");
    MBOOL log_enable = atoi(value_log);

	FILE* myfile = fopen(filename.c_str(), "rb");
	if (myfile == NULL)
	{
		CAM_LOGD("Error open golden file: %s\n",  filename.c_str());
	}
	else
	{
		CAM_LOGD("OK open golden file: %s\n",  filename.c_str());
		fclose(myfile);
	}

	std::ifstream	goldenBin;
	goldenBin.open(filename, std::ios::in | std::ios::binary);
	if (goldenBin.is_open())
	{
                CAM_LOGD_IF(log_enable,"readGolden 000");
		unsigned int modNs, modOfst, modSize;
		// modNs
		if (!goldenBin.read(reinterpret_cast<char*>(&modNs), sizeof(unsigned int)))
		{
		    CAM_LOGD_IF(log_enable,"readGolden 001");
			goto ERROR_RETURN;
		}

		if (modIdx >= modNs)
		{
		    CAM_LOGD("readGolden 002");
			CAM_LOGD("Request to read modIdx %d, but only %d modules in golden.bin\n", modIdx, modNs);
			goldenBin.close();
			return false;
		}

        CAM_LOGD_IF(log_enable,"readGolden 003 modIdx %d, modNs %d", modIdx, modNs);

		goldenBin.seekg(4 + modIdx * 8, std::ios_base::beg);
		// modOfst
		if (!goldenBin.read(reinterpret_cast<char*>(&modOfst), sizeof(unsigned int)))
		{
		        CAM_LOGD("readGolden 004");
			goto ERROR_RETURN;
		}

         CAM_LOGD_IF(log_enable,"readGolden 005 modOfst %d", modOfst);

		// modSize
		if (!goldenBin.read(reinterpret_cast<char*>(&modSize), sizeof(unsigned int)))
		{
		        CAM_LOGD("readGolden 006");
			goto ERROR_RETURN;
			return false;
		}

         CAM_LOGD_IF(log_enable,"readGolden 007 modSize %d", modSize);

		goldenBin.seekg(modOfst, std::ios_base::beg);

		unsigned short dimNs, combNs;

		// dimensionNs
		if (!goldenBin.read(reinterpret_cast<char*>(&dimNs), sizeof(unsigned short)))
		{
		        CAM_LOGD("readGolden 008");
			goto ERROR_RETURN;
		}

		// combinationNs
		if (!goldenBin.read(reinterpret_cast<char*>(&combNs), sizeof(unsigned short)))
		{
		        CAM_LOGD("readGolden 009");
			goto ERROR_RETURN;
		}

        CAM_LOGD_IF(log_enable,"readGolden 010 dimNs %d, combNs %d", dimNs, combNs);

		for (int i = 0; i < combNs; i++)
		{
		    CAM_LOGD_IF(log_enable,"readGolden 011 i = %d", i);
			COMB_TYPE tmpComb;

			for (int j = 0; j < dimNs; j++)
			{
			    CAM_LOGD_IF(log_enable,"readGolden 012 j = %d", j);
				unsigned short factorNs;
				std::vector<unsigned short> tmpFactor;

				// factorNs
				if (!goldenBin.read(reinterpret_cast<char*>(&factorNs), sizeof(unsigned short)))
				{
					goto ERROR_RETURN;
				}

                 CAM_LOGD_IF(log_enable,"readGolden 012 factorNs %d", factorNs);

				for (int k = 0; k < factorNs; k++)
				{
					unsigned short factor;
					// factor
					if (!goldenBin.read(reinterpret_cast<char*>(&factor), sizeof(unsigned short)))
					{
						goto ERROR_RETURN;
					}

					tmpFactor.push_back(factor);
				}

				//std::sort(tmpFactor.begin(), tmpFactor.end());
				tmpComb.comb.push_back(tmpFactor);
			}

             CAM_LOGD_IF(log_enable,"readGolden 013");
			// index
			unsigned short index;
			if (!goldenBin.read(reinterpret_cast<char*>(&index), sizeof(unsigned short)))
			{
			    CAM_LOGD("readGolden 014");
				goto ERROR_RETURN;
			}
			tmpComb.index = index;
            CAM_LOGD_IF(log_enable,"readGolden 015 index %d", index);
			goldenComb[modIdx].push_back(tmpComb);
		}

		goldenBin.close();
	}
	else
	{
		CAM_LOGD("Error open golden file: %s\n",  filename.c_str());
		return false;
	}

	return true;

ERROR_RETURN:
	CAM_LOGD("golden.bin read fail for modIdx %d\n", modIdx);
	goldenBin.close();
	return false;
}

bool ut_by_module(IdxMgr* pMgr, int modIdx, string goldenPath, int SensorDev)
{
	int rtn_code = 0;
    clock_t start_t, end_t;

    CAM_LOGD("***** Process Module [%s] *****\n", strEModule[modIdx]);
    start_t = clock();

    goldenComb[modIdx].clear();

    if (readGolden(goldenPath + "golden.bin", modIdx) && (pMgr != NULL))
    {
        std::vector<unsigned short> factor;

        std::vector<COMB_TYPE>::iterator combIt;
        int CombIdx = 0;
        for (combIt = goldenComb[modIdx].begin(); combIt != goldenComb[modIdx].end(); combIt++)
        {
            do_DFS_userDefine(0, *combIt, factor, pMgr, modIdx, CombIdx, SensorDev);
            CombIdx++;
        }
    }
    else
    {
        CAM_LOGD("***** [Open golden fail] %s not found! *****\n", strEModule[modIdx]);
        return false;
    }
    end_t = clock();
    CAM_LOGD("***** Module [%d: %s] costs %f seconds *****\n", modIdx, strEModule[modIdx], (double)(end_t - start_t)/CLOCKS_PER_SEC);
    CAM_LOGD("***** Module [%d: %s] (OK, NG) = (%llu, %llu) *****\n", modIdx, strEModule[modIdx], ut_ok_cnt_curMod[modIdx], ut_ng_cnt_curMod[modIdx]);

    return true;

}
int nvram_ut_func(int SensorDev)
{
    std::string goldenPath;
    switch (SensorDev)
    {
        case ESensorDev_Main:
            goldenPath = "/sdcard/camera_dump/ESensorDev_Main_";
            break;
        case ESensorDev_Sub:
            goldenPath = "/sdcard/camera_dump/ESensorDev_Sub_";
            break;
   	    case ESensorDev_MainSecond:
            goldenPath = "/sdcard/camera_dump/ESensorDev_MainSecond_";
            break;
        case ESensorDev_Main3D:
            goldenPath = "/sdcard/camera_dump/ESensorDev_Main3D_";
            break;
        case ESensorDev_SubSecond:
            goldenPath = "/sdcard/camera_dump/ESensorDev_SubSecond_";
            break;
        default:
            goldenPath = "/sdcard/camera_dump/";
            break;
    }

//	int targetModIdx = -1;

	int rtn_code = 0;
    IdxMgr* pMgr = NULL;

    if (SensorDev & ESensorDev_Main){
        pMgr = IdxMgr::createInstance(ESensorDev_Main);
    }
    if (SensorDev & ESensorDev_Sub){
        pMgr = IdxMgr::createInstance(ESensorDev_Sub);
    }
    if (SensorDev & ESensorDev_MainSecond){
        pMgr = IdxMgr::createInstance(ESensorDev_MainSecond);
    }
    if (SensorDev & ESensorDev_Main3D){
        pMgr = IdxMgr::createInstance(ESensorDev_Main3D);
    }
    if (SensorDev & ESensorDev_SubSecond){
        pMgr = IdxMgr::createInstance(ESensorDev_SubSecond);
    }

    thread nvram_mod[EModule_NUM];

	for (int i = 0; i < EModule_NUM; i++)
	{
        CAM_LOGD("create thread begin for %s", strEModule[i]);
        nvram_mod[i] = thread(ut_by_module, pMgr, i, goldenPath, SensorDev);
	}

	for (int i = 0; i < EModule_NUM; i++)
	{
        nvram_mod[i].join();
        CAM_LOGD("create thread end for %s",strEModule[i]);
        ut_ng_cnt += ut_ng_cnt_curMod[i];
        ut_ok_cnt += ut_ok_cnt_curMod[i];
	}

	if (ut_ng_cnt == 0)
	{
		CAM_LOGD ("***** [UT PASS] *****\n");
	}
	else
	{
		CAM_LOGD ("***** [UT Fail] *****\n");
       	rtn_code = 1;
	}

	CAM_LOGD ("***** [OK cnt %llu, NG cnt %llu] *****\n", ut_ok_cnt, ut_ng_cnt);

MY_EXIT:
	return rtn_code;
}

void nvram_ut(int SensorDev)
{
    static Mutex _lock;
    Mutex::Autolock  _l(_lock);

    static MBOOL create_thd = 0;

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.mapping_mgr.ut", value, "0");

    MBOOL nvram_ut_enable = atoi(value);

    if (nvram_ut_enable && !create_thd)
    {
        CAM_LOGD("create thread begin");
        create_thd = 1;
        thread nvram_thd = thread(nvram_ut_func, SensorDev);
        nvram_thd.join();
        CAM_LOGD("create thread end");
    }
}
#endif
