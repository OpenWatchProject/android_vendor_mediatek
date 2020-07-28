/*
*  Copyright (C) 2014 MediaTek Inc.
*
*  Modification based on code covered by the below mentioned copyright
*  and/or permission notice(s).
*/

// {AT command, timeout} // mins
{"CGACT", 15},
{"CGATT", 15},
{"COPS", 15},
{"CMGS", 10}, // SMS can over CS or IMS and will re-try on different domain. The worst case will be around 8 mins
{"EFUN", 15},
{"EGACT", 15},
