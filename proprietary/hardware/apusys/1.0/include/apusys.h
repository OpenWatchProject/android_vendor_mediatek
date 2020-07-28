#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include "stdint.h"


typedef enum {
	APUSYS_DEVICE_NONE,
	APUSYS_DEVICE_SAMPLE,

	APUSYS_DEVICE_MDLA,
	APUSYS_DEVICE_VPU,
	APUSYS_DEVICE_EDMA,
	APUSYS_DEVICE_WAIT,

	APUSYS_DEVICE_MAX,
}APUSYS_DEVICE_E;

typedef enum {
	APUSYS_USER_MEM_DRAM,
	APUSYS_USER_MEM_VLM,

	APUSYS_USER_MEM_MAX,
}APUSYS_USER_MEM_E;

/* enumuratio priority for apusys cmd */
typedef enum {
    APUSYS_PRIORITY_LOW = 0,
    APUSYS_PRIORITY_NORMAL = 1,
    APUSYS_PRIORITY_HIGH = 2,

    APUSYS_PRIORITY_MAX,
}APUSYS_PRIORITY_E;

struct apusysProp {
    /* priority */
    int priority; // APUSYS_PRIORITY_E

    /* apusys cmd performance tuning */
    uint32_t targetMs; // target deadline
    uint32_t estimateMs; // estimate ms
};

class IApusysMem {
public:
    uint64_t va;
    uint64_t kva;
    uint32_t iova;

    uint32_t offset;
    uint32_t size;

    uint32_t align;

    int type;
};

class IApusysDev {
public:
    virtual ~IApusysDev(){};
};

class IApusysCmd {
public:
    virtual ~IApusysCmd(){};

    /* cmd functions */

    /*
     * @setCmdProperty: config apusys information
     *  description:
     *      config parameters of IApusysCmd which may impact schduler working
     *  arguement:
     *      <prop>: parameters user can config for IApusysCmd
     *          priority: priority
     *          vlmForce: force scheduler provide real vlm, if no vlm, blocking until vlm release and occuiped it (1/0)
     *          targetMs: deadline, command should be done in indicated time duration, if no, return false (0: no effect)
     *          estimateMs: time cost of this command on trail run (0: no effect)
     *  return:
     *      success: true
     *      fail: false
     */
    virtual bool setCmdProperty(struct apusysProp &prop) = 0;

    /*
     * @addSubCmd: add subcmd(subgraph) into apusys cmd
     *  description:
     *      add subcmd into apusys cmd
     *  arguement:
     *      <subCmdBuf>: subcmd buffer contain communicate driver structure, device driver will get this buffer at scheduler dispatching
     *      <deviceType>: device type
     *      <dependency>: idx vector array, this subcmd should be dispatched after all dependency idx subcmd finished
     *      <estimateMs>: time cost of this subcmd on trail run (0: no effect)
     *  return:
     *      success: idx(>=0)
     *      fail: -1
     */
    virtual int addSubCmd(IApusysMem * subCmdBuf, APUSYS_DEVICE_E deviceType, std::vector<int> & dependency) = 0;
    virtual int addSubCmd(IApusysMem * subCmdBuf, APUSYS_DEVICE_E deviceType, std::vector<int> & dependency, uint32_t estimateMs) = 0;
    virtual int addSubCmd(IApusysMem * subCmdBuf, APUSYS_DEVICE_E deviceType, std::vector<int> & dependency, uint32_t estimateMs, uint32_t ctxId) = 0;

    /*
     * @packSubCmd: add subcmd and pack indicate subcmd
     *  description:
     *      add subcmd and pack indicate subcmd, the dependency will follow target subcmd
     *      apusys is responsible to trigger all packed subcmd at the same time
     *      used to user multicore application
     *  arguement:
     *      <subCmdBuf>: subcmd buffer contain communicate driver structure, device driver will get this buffer at scheduler dispatching
     *      <deviceType>: device type
     *      <subCmdIdx>: target subcmd user want to packed with
     *  return:
     *      success: idx(>=0)
     *      fail: -1
     */
    virtual int packSubCmd(IApusysMem * subCmdBuf, APUSYS_DEVICE_E deviceType , int subCmdIdx) = 0;

    virtual bool unpackSubCmd(IApusysMem * subCmdBuf) = 0;

    virtual bool deleteSubCmd(uint32_t idx) = 0;

    virtual bool modifySubCmd(uint32_t idx, IApusysMem * subCmdBuf) = 0;

    /*
     * @constructCmd: construct apusys command format buffer
     *  description:
     *      construct apusys command format buffer according IApusysCmd detail information
     *      user call this function to avoid the construct overhead in first inference
     *  return:
     *      success: true
     *      fail: false
     */
    virtual bool constructCmd() = 0;
};

class IApusysEngine {
public:
    virtual ~IApusysEngine(){};

    /* inst fnuctions */

    /*
     * @createInstance: apusys engine init function
     *  description:
     *      init apusys engine class for processing
     *      user should call this function to get IApusysEngine * first
     *  arguement:
     *      <userName>: name which user provided, can be used to identify user
     *  return:
     *      success: IApusysEngine * (libapusys entry)
     *      fail: nullptr
     */
    static IApusysEngine * createInstance(const char * userName);

    /*
     * @deleteInstance: apusys engine delete function
     *  description:
     *      delete apusys engine class which get by IApusysEngine::createInstance()
     *      shouldn't delete IApusysEngine * directly byself
     *  arguement:
     *      <engine>: apusys engine inst which init by IApusysEngine::createInstance()
     *  return:
     *      success: true
     *      fail: false
     */
    static bool deleteInstance(IApusysEngine * engine);

    /* cmd functions */

    /*
     * @initCmd: apusys cmd init function
     *  description:
     *      init apusys cmd class to user, and user can construct this cmd by class member functions (refer to IApusysCmd)
     *      all apusys cmd should be init by this function first
     *  return:
     *      success: IApusysCmd * (handle for user)
     *      fail: nullptr
     */
    virtual IApusysCmd * initCmd() = 0;

    /*
     * @destroyCmd: apusys cmd destroy function
     *  description:
     *      destroy apusys cmd which inited by IApusysEngine::initCmd() safely
     *      shouldn't delete IApusysCmd * directly byself
     *  arguement:
     *      <cmd>: the apusys cmd which get by IApusysEngine::initCmd()
     *  return:
     *      success: true
     *      fail: false
     */
    virtual bool destroyCmd(IApusysCmd * cmd) = 0;

    /*
     * @runCmd: run apusys cmd synchronous
     *  description:
     *      execute apusys cmd synchronous, blocking until cmd completed
     *  arguement:
     *      <cmd>: the apusys cmd which get by IApusysEngine::initCmd()
     *  return:
     *      success: true
     *      fail: false
     */
    virtual bool runCmd(IApusysCmd * cmd) = 0;

    /*
     * @runCmdAsync: run apusys cmd asynchronous
     *  description:
     *      execute apusys cmd asynchronous, no blocking and return immediately
     *      should call IApusysEngine::waitCmd() to check cmd completed
     *  arguement:
     *      <cmd>: the apusys cmd which get by IApusysEngine::initCmd()
     *  return:
     *      success: true
     *      fail: false
     */
    virtual bool runCmdAsync(IApusysCmd * cmd) = 0;

    /*
     * @waitCmd: wait indicate cmd which was executed by IApusysEngine::runCmdAsync()
     *  description:
     *      wait indicate cmd which was executed by IApusysEngine::runCmdAsync()
     *      block user thread until cmd completed
     *  arguement:
     *      <cmd>: the apusys cmd which get by IApusysEngine::initCmd()
     *  return:
     *      success: true
     *      fail: false
     */
    virtual bool waitCmd(IApusysCmd * cmd) = 0;

    /*
     * @runCmd: run apusys cmd synchronous
     *  description:
     *      execute apusys cmd synchronous with command buffer directly, blocking until cmd completed
     *      ***should be called by superuser who can construct apusys command format byself
     *  arguement:
     *      <cmdBuf>: memory buffer contain apusys command format
     *  return:
     *      success: true
     *      fail: false
     */
    virtual bool runCmd(IApusysMem * cmdBuf) = 0;

    /*
     * @runCmdAsync: run apusys cmd asynchronous
     *  description:
     *      execute apusys cmd asynchronous, no blocking and return immediately
     *      should call IApusysEngine::waitCmd() to check cmd completed
     *      ***should be called by superuser who can construct apusys command format byself
     *  arguement:
     *      <cmd>: the apusys cmd which get by IApusysEngine::initCmd()
     *  return:
     *      success: true
     *      fail: false
     */
    virtual bool runCmdAsync(IApusysMem * cmdBuf) = 0;

    /*
     * @waitCmd: wait indicate cmd which was executed by IApusysEngine::runCmdAsync()
     *  description:
     *      wait indicate cmd which was executed by IApusysEngine::runCmdAsync()
     *      block user thread until cmd completed
     *      ***should be called by superuser who can construct apusys command format byself
     *  arguement:
     *      <cmd>: the apusys cmd which get by IApusysEngine::initCmd()
     *  return:
     *      success: true
     *      fail: false
     */
    virtual bool waitCmd(IApusysMem * cmdBuf) = 0;

    /*
     * @checkCmdValid: check command buffer is valid with apusys command format or not
     *  description:
     *      check command buffer is valid for apusys command format
     *      used to check command version to avoid version mismatch
     *      ***should be called by superuser who can construct apusys command format byself
     *  arguement:
     *      <cmd>: the apusys cmd which get by IApusysEngine::initCmd()
     *  return:
     *      success: true
     *      fail: false
     */
    virtual bool checkCmdValid(IApusysMem * cmdBuf) = 0;

    /* memory functions*/

    /*
     * @memAlloc: memory allocate
     *  description:
     *      allocate dram memory with size, and va(va), kernel va(kva) and device va(iova) are mapped already
     *  arguement:
     *      <size>: the type of device user want to allocate
     *  return:
     *      success: IApusysMem * (handle for user)
     *      fail: nullptr
     */
    virtual IApusysMem * memAlloc(size_t size) = 0;

    /*
     * @memAlloc: memory allocate
     *  description:
     *      allocate dram memory with size and align, and va(va), kernel va(kva) and device va(iova) are mapped already
     *  arguement:
     *      <size>: the type of device user want to allocate
     *      <align>: memory alignment (should be factors of 4K)
     *  return:
     *      success: IApusysMem * (handle for user)
     *      fail: nullptr
     */
    virtual IApusysMem * memAlloc(size_t size, uint32_t align) = 0;

    /*
     * @memAlloc: memory allocate
     *  description:
     *      allocate memory with size, align and type, and va(va), kernel va(kva) and device va(iova) are mapped already
     *      if type = APUSYS_USER_MEM_VLM, always return start address(iova only, no va and kva)
     *  arguement:
     *      <size>: the type of device user want to allocate
     *      <align>: memory alignment (should be factors of 4K)
     *      <type>: type of memory
     *          APUSYS_USER_MEM_DRAM: dram
     *          APUSYS_USER_MEM_VLM: fast memory, if this type set, only return start address(iova) only to user
     *  return:
     *      success: IApusysMem * (handle for user)
     *      fail: nullptr
     */
    virtual IApusysMem * memAlloc(size_t size, uint32_t align, uint32_t cache) = 0;
    virtual IApusysMem * memAlloc(size_t size, uint32_t align, uint32_t cache, APUSYS_USER_MEM_E type) = 0;
    /*
     * @memFree: memory free
     *  description:
     *      free memory which allocated by IApusysEngine::memAlloc()
     *  arguement:
     *      <mem>: the handle return from IApusysEngine::memAlloc()
     *  return:
     *      success: true
     *      fail: false
     */
    virtual bool memFree(IApusysMem * mem) = 0;

    /*
     * @memSync: memory sync(flush)
     *  description:
     *      sync memory which allocated by IApusysEngine::memAlloc()
     *  arguement:
     *      <mem>: the handle return from IApusysEngine::memAlloc()
     *  return:
     *      success: true
     *      fail: false
     */
    virtual bool memSync(IApusysMem * mem) = 0;

    /*
     * @memInvalidate: memory invalidate
     *  description:
     *      invalidate memory which allocated by IApusysEngine::memAlloc()
     *  arguement:
     *      <mem>: the handle return from IApusysEngine::memAlloc()
     *  return:
     *      success: true
     *      fail: false
     */
    virtual bool memInvalidate(IApusysMem * mem) = 0;

    /*
     * @memImport: memory import
     *  description:
     *      import memory by ion fd, and get apusys memory handle
     *      NOT SUPPORT YET!!!!!!!!!!!!!!!
     *  arguement:
     *      <fd>: ion shared fd which allocate by ion function
     *  return:
     *      success: IApusysMem * (handle for user)
     *      fail: nullptr
     */
    virtual IApusysMem * memImport(int shareFd, uint32_t size) = 0;
    virtual bool memUnImport(IApusysMem * mem) = 0;

    /* device functions */

    /*
     * @getDeviceStatus: query # type device support
     *  description:
     *      query how many core the device type support this platform
     *  arguement:
     *      <deviceType>: the type of device user want to allocate
     *  return:
     *      success: > 0 (# type device exist this platform)
     *      fail: 0 (no type device support)
     */
    virtual int getDeviceNum(APUSYS_DEVICE_E type) = 0;

    /*
     * @devAlloc: allocate indicate type device
     *  description:
     *      user can allocate type of device for specific using
     *      apusys doesn't schedule any cmd to this device until user process free or crash
     *  arguement:
     *      <deviceType>: the type of device user want to allocate
     *  return:
     *      success: IApusysDev * (handle for user)
     *      fail: nullptr
     */
    virtual IApusysDev * devAlloc(APUSYS_DEVICE_E deviceType) = 0;

    /*
     * @devAlloc: free indicate device
     *  description:
     *      free indicate device which allocated from IApusysEngine::devAlloc()
     *      user has responsibility for free device
     *  arguement:
     *      <idev>: device handle which return from IApusysEngine::devAlloc()
     *  return:
     *      success: true
     *      fail: false
     */
    virtual bool devFree(IApusysDev * idev) = 0;

    /* power functions */

    /*
     * @setPower: power on indicate device for hiding overhead
     *  description:
     *      power on device of indicate type, user can hiding power on latency byself
     *  arguement:
     *      <deviceType>: the type of device user want to allocate
     *      <boostVal>: linear value of opp step, max:100, min:0 [0,100]
     *          ex. boostVal=100, bootup device by its highest clk
     *  return:
     *    success: true
     *    fail: false
     */
    virtual bool setPower(APUSYS_DEVICE_E deviceType, uint8_t boostVal) = 0;
};

