package com.mediatek.mms.appcallback;

import com.mediatek.mms.appcallback.SmsEntry;

interface IMmsCallbackService {
    void sendMultipartTextMessageWithEncodingType(in SmsEntry param);
}
