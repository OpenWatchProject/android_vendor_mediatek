#ifndef __PMIC_WRAP_REGS_HMAC__
#define __PMIC_WRAP_REGS_HMAC__

#define GET_WACS2_EN(x)              ((x>>0)  & 0x00000001)
#define GET_INIT_DONE2(x)            ((x>>0)  & 0x00000001)
#define GET_WACS2_WDATA(x)           ((x>>0)  & 0x0000ffff)
#define GET_WACS2_ADR(x)             ((x>>16) & 0x00007fff)
#define GET_WACS2_WRITE(x)           ((x>>31) & 0x00000001)
#define GET_WACS2_RDATA(x)           ((x>>0)  & 0x0000ffff)
#define GET_WACS2_FSM(x)             ((x>>16) & 0x00000007)
#define GET_WACS2_REQ(x)             ((x>>19) & 0x00000001)
#define GET_WACS2_SYNC_IDLE2(x)      ((x>>20) & 0x00000001)
#define GET_WACS2_INIT_DONE2(x)      ((x>>21) & 0x00000001)
#define GET_SYS_IDLE2(x)             ((x>>22) & 0x00000001)

#endif //__PMIC_WRAP_REGS_HMAC__

