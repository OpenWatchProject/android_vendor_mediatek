#ifndef __MMSYS_REG_BASE_H__
#define __MMSYS_REG_BASE_H__

#define CAM_CTRL_BASE       (0x15004000)

#define MMSYS_CONFIG_BASE   (0x14000000)
#define MDP_RDMA0_BASE      (0x14001000)
#define MDP_RSZ0_BASE       (0x14002000)
#define MDP_RSZ1_BASE       (0x14003000)
#define MDP_WDMA_BASE       (0x14004000)
#define MDP_WROT0_BASE      (0x14005000)
#define MDP_TDSHP0_BASE     (0x14006000)

#define MMSYS_MUTEX_BASE    DpDriver::getInstance()->getMMSysMutexBase()

#define DISP_OVL0_BASE      (0x14007000)
#define DISP_OVL1_BASE      (0x14008000)
#define DISP_RDMA0_BASE     (0x14009000)
#define DISP_RDMA1_BASE     (0x1400a000)
#define DISP_WDMA0_BASE     (0x1400b000)
#define DISP_COLOR_BASE     (0x1400c000)
#define DISP_CCORR_BASE     (0x1400d000)
#define DISP_AAL_BASE       (0x1400e000)
#define DISP_GAMMA_BASE     (0x1400f000)
#define DISP_DITHER_BASE    (0x14010000)
#define DISP_UFOE_BASE      (0x14011000)
#define DSI0_BASE           (0x14012000)
#define DPI0_BASE           (0x14013000)
#define DISP_PWM0_BASE      (0x14014000)

#define MMSYS_CMDQ_BASE     (0x1020a000)

#define IMGSYS_BASE         (0x15000000)
#define ISP_CAM_0_BASE      (0x15004000)

#define USE_ENGBASE_MMSYS_CONFIG	1
#define USE_ENGBASE_MDP_RDMA0		1
#define USE_ENGBASE_MDP_RSZ0		1
#define USE_ENGBASE_MDP_RSZ1		1
#define USE_ENGBASE_MDP_WDMA		1
#define USE_ENGBASE_MDP_WROT0		1
#define USE_ENGBASE_MDP_TDSHP0		1
#define USE_ENGBASE_MMSYS_MUTEX		1
#define USE_ENGBASE_IMGSYS			1
#define USE_ENGBASE_ISP_CAM_0		1
#define USE_ENGBASE_MMSYS_CMDQ		1

#define MM_REG_READ(cmd, addr, val, ...)                (cmd).read((addr), (val), ##__VA_ARGS__)
#define MM_REG_READ_BEGIN(cmd)                          //MM_REG_WAIT(cmd, DpCommand::SYNC_TOKEN_GPR_READ)        /* Must wait token before read */
#define MM_REG_READ_END(cmd)                            //MM_REG_SET_EVENT(cmd, DpCommand::SYNC_TOKEN_GPR_READ)   /* Must set token after read */

#define MM_REG_WRITE_MASK(cmd, addr, val, mask, ...)    (cmd).write((addr), (val), (mask), ##__VA_ARGS__)
#define MM_REG_WRITE(cmd, addr, val, mask, ...)         MM_REG_WRITE_MASK(cmd, addr, val, (((mask) & (addr##_MASK)) == (addr##_MASK)) ? (0xFFFFFFFF) : (mask), ##__VA_ARGS__)

#define MM_REG_WAIT(cmd, event)                         (cmd).wait(event)

#define MM_REG_WAIT_NO_CLEAR(cmd, event)                (cmd).waitNoClear(event)

#define MM_REG_CLEAR(cmd, event)                        (cmd).clear(event)

#define MM_REG_SET_EVENT(cmd, event)                    (cmd).setEvent(event)

#define MM_REG_POLL_MASK(cmd, addr, val, mask, ...)     (cmd).poll((addr), (val), (mask), ##__VA_ARGS__)
#define MM_REG_POLL(cmd, addr, val, mask, ...)          MM_REG_POLL_MASK(cmd, addr, val, (((mask) & (addr##_MASK)) == (addr##_MASK)) ? (0xFFFFFFFF) : (mask), ##__VA_ARGS__)

#define MM_REG_WRITE_FROM_REG(cmd, addr, val, mask)     (cmd).writeFromReg((addr), (val), (mask))
#define MM_REG_WRITE_FROM_REG_BEGIN(cmd)                //MM_REG_WAIT(cmd, DpCommand::SYNC_TOKEN_GPR_WRITE_FROM_REG)      /* Must wait token before write from reg */
#define MM_REG_WRITE_FROM_REG_END(cmd)                  //MM_REG_SET_EVENT(cmd, DpCommand::SYNC_TOKEN_GPR_WRITE_FROM_REG) /* Must set token after write from reg */
#define ID_ADDR(id)		(id << 12)

#define CMD_GPR_R32(id) (MMSYS_CMDQ_BASE+ (0x4*id)+ 0x80)

#endif  // __MM_REG_BASE_H__
