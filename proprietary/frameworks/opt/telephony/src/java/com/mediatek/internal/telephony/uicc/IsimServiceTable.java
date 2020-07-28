/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.internal.telephony.uicc;
import com.android.internal.telephony.uicc.IccServiceTable;
// MTK-START: ISIM
/**
 * Wrapper class for the USIM Service Table EF.
 * See 3GPP TS 31.102 Release 10 section 4.2.8
 */
public final class IsimServiceTable extends IccServiceTable {
    public enum IsimService {
        PCSCF_ADDRESS,
        // MTK-START: GBA
        GBA,
        // MTK-END
        HTTP_DIGEST,
        GBA_LOCAL_KEY_ESTABLISHMENT,
        PCSCF_DISCOVERY,
        SMS,
        SMSR,
        SM_OVER_IP,
        COMMUNICATION_CONTROL_BY_ISIM,
        UICC_ACCESS_IMS
    }

    public IsimServiceTable(byte[] table) {
        super(table);
    }

    public boolean isAvailable(IsimService service) {
        return super.isAvailable(service.ordinal());
    }

    @Override
    protected String getTag() {
        return "IsimServiceTable";
    }

    @Override
    protected Object[] getValues() {
        return IsimService.values();
    }
}
// MTK-END
