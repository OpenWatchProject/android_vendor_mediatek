package com.mediatek.security.service;

import java.util.List;

import com.mediatek.security.datamanager.CheckedPermRecord;

interface INetworkDataControllerService {
    boolean modifyNetworkDateRecord(in CheckedPermRecord checkedCellular);

    CheckedPermRecord getNetworkDataRecord(in int uid);

    List<CheckedPermRecord> getNetworkDataRecordList();
}
