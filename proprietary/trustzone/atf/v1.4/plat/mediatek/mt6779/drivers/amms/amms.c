#include <debug.h>
#include <mmio.h>
#include <mtk_plat_common.h>
#include <platform.h>
#include <platform_def.h>
#include <console.h>
#include <spinlock.h>
#include <amms.h>
#include <pccif.h>
#include <string.h>
#include <mt_mpu.h>
#include <gic_v2.h>
#include <gic_v3.h>
#if defined(MTK_ENABLE_MPU_HAL_SUPPORT)
#include <mpu_ctrl/mpu_api.h>
#endif
#define DMB()	(__asm__ volatile("dmb sy" : : : "memory"))

/*#define AMMS_DEBUG*/


/*global variable */
spinlock_t amms_pending_lock;
int is_DRDI_free;
/*MPU 1: AP own, already release to AP, 0:MD own*/
int is_POS_free = AMMS_POS_OWN_BY_AP;
/*MPU 1: AP own, 0:MD own 2:AP process to MD 3:MD process to AP*/
unsigned int amms_pending_request_bitmap;
unsigned long long md_base_address,
	md_pos_addr, md_pos_length, md_view_pos_addr;
unsigned long long amms_free_addr, amms_free_length;
void amms_md_pos_mpu_change(enum AMMS_MD_POS_OWNER owner)
{
	unsigned long long low_bound, high_bound;

	high_bound = ((md_pos_addr + md_pos_length) >> 16)<<16;
	low_bound = md_pos_addr;
	if (low_bound % 0x10000)
		low_bound = ((low_bound + 0x10000) >> 16) << 16;
	if (high_bound <= low_bound) {
		/* we do not apply any MPU setting but just return success*/
		NOTICE("invalid bound\n");
		NOTICE("low_bound =0x%llx high_bound=0x%llx MPU no apply\n"
			, low_bound, high_bound);
		return;
	}

#if defined(MTK_ENABLE_MPU_HAL_SUPPORT)
	{
	uint64_t mpu_addr_s = low_bound;
	uint64_t mpu_size = (high_bound - low_bound);

	switch (owner) {
	case AMMS_POS_OWNER_AP:
		sip_feat_mpu_request(MPU_REQ_ORIGIN_ATF_ZONE_AMMS_POS,
				     MPU_FEAT_AMMS_POS_OWNER_AP,
				     mpu_addr_s, mpu_size);
		NOTICE("SET MPU owner to AP!\n");
		break;
	case AMMS_POS_OWNER_MD:
		sip_feat_mpu_request(MPU_REQ_ORIGIN_ATF_ZONE_AMMS_POS,
				     MPU_FEAT_AMMS_POS_OWNER_MD,
				     mpu_addr_s, mpu_size);
		NOTICE("SET MPU owner to MD!\n");
		break;
	default:
		NOTICE("invalid MPU owner,  MPU no apply!\n");
		return;
	}
	}
#else
	ERROR("%s %d: AMMS MPU permission is not set!\n", __func__, __LINE__);
#endif
}

void amms_cmd_AMMS_CMD_DEALLOCATE_POS_BUFFEE_handler(
	struct amms_msg_request *request)
{
	int ret;
	struct amms_msg_response reply;
	struct amms_address_range range
		= request->u_req.deallocate_pos_buf_cmd.range;
	unsigned long long low_bound, high_bound;

	memset(&reply, 0, sizeof(reply));
	/* dump info */
#ifdef AMMS_DEBUG
	console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
#endif
	NOTICE("cmd =[AMMS_CMD_DEALLOCATE_POS_BUFFER]\n");
	NOTICE("seq_id = %d start=%x length=%x is_POS_free=%d\n",
		request->seq_id, range.start_address,
		range.length, is_POS_free);
#ifdef AMMS_DEBUG
	console_uninit();
#endif

	if (md_pos_addr == 0) {
		NOTICE("AMMS: md_pos_addr is not set, panic\n");
		panic();
	}

	high_bound = ((md_pos_addr + md_pos_length) >> 16)<<16;
	low_bound = md_pos_addr;
	/*we want to free memory to kernel
	 * so if not align , we can only free align part
	 * because MPU is 64K align
	 */
	if (low_bound % 0x10000)
		low_bound = ((low_bound + 0x10000) >> 16) << 16;
	if (high_bound <= low_bound) {
		/* we do not apply any MPU setting but just return success*/
		reply.status = AMMS_STATUS_INVALID;
		NOTICE("invalid low_bound/high_bound\n");
		NOTICE("low=0x%llx high=0x%llx MPU no apply\n",
			low_bound, high_bound);
		goto pos_mpu_out;
	}

	if (is_POS_free == AMMS_POS_OWN_BY_MD) {
		/*free already no need to notify kernel again
		 */
		spin_lock(&amms_pending_lock);
		amms_pending_request_bitmap |= AMMS_PENDING_POS_DEALLOC_BIT;
		is_POS_free = AMMS_POS_OWN_BY_AP;
		/* set to AP own immediately , because next
		 * MD query need it to be assigned to AP
		 */
		amms_md_pos_mpu_change(AMMS_POS_OWNER_AP);
		spin_unlock(&amms_pending_lock);
		gicd_set_ispendr(BASE_GICD_BASE, ATF_AMMS_IRQ_ID);
		NOTICE("%s:%d notify kernel to get memory\n",
			__func__, __LINE__);
	} else if (is_POS_free == AMMS_POS_OWN_BY_AP) {
		NOTICE("%s:%d already free, not notify kernel\n",
			__func__, __LINE__);
	} else {
		NOTICE("%s:%d POS request is under processing\n",
			__func__, __LINE__);
	}
	/**/
pos_mpu_out:
	reply.error = 0;
	reply.seq_id = request->seq_id;
	/* reply.seq_id = 1; */
#ifdef AMMS_DEBUG
	console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
	NOTICE("reply.status=%d\n", reply.status);
	console_uninit();
#endif

	ret = ccif_irq0_user_write((unsigned int *)&reply,
		((sizeof(reply)/4)+(((sizeof(reply)%4) == 0)?0:1)), 0);

	if (ret < 0) {
#ifdef AMMS_DEBUG
		console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
#endif
		NOTICE("ccif_irq0_user_write failed ret = %d\n", ret);
#ifdef AMMS_DEBUG
		console_uninit();
#endif
	}

	ret = ccif_irq0_user_notify_md(0);
	if (ret < 0) {
#ifdef AMMS_DEBUG
		console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
#endif
		NOTICE("ccif_irq0_user_notify_md failed ret = %d\n", ret);
#ifdef AMMS_DEBUG
		console_uninit();
#endif
	}
}

void amms_cmd_AMMS_CMD_ALLOCATE_POS_BUFFEE_handler(
	struct amms_msg_request *request)
{
	int ret;
	struct amms_msg_response reply;
	struct amms_address_range range =
		request->u_req.allocate_pos_buf_cmd.range;
	unsigned long long low_bound, high_bound;

	memset(&reply, 0, sizeof(reply));
	/* dump info */
#ifdef AMMS_DEBUG
	console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
#endif
	NOTICE("cmd =[AMMS_CMD_ALLOCATE_POS_BUFFER]\n");
	NOTICE("seq_id = %d start=%x length=%x is_POS_free=%d\n",
		request->seq_id, range.start_address,
		range.length, is_POS_free);
#ifdef AMMS_DEBUG
	console_uninit();
#endif

	if (md_pos_addr == 0) {
		NOTICE("AMMS: md_pos_addr is not set, panic\n");
		panic();
	}

	high_bound = ((md_pos_addr + md_pos_length) >> 16)<<16;
	low_bound = md_pos_addr;
	/*we want to free memory to kernel*/
	/* so if not align , we can only free align part
	 * because MPU is 64K align
	 */
	if (low_bound % 0x10000)
		low_bound = ((low_bound + 0x10000) >> 16) << 16;
	if (high_bound <= low_bound) {
		/* we do not apply any MPU setting but just return success*/
		reply.status = AMMS_STATUS_INVALID;
		NOTICE("invalid low_bound =0x%llx high_bound=0x%llx\n",
			   low_bound, high_bound);
		NOTICE("MPU no apply\n");
		goto pos_mpu_out;
	}
	/*for MPU setting , it should be in another phase instead of this one
	 */

	/*TO-DO:we can notify kernel to get back memory to use now
	 */
#if 0
	NOTICE("%s:%d md_pos_addr=0x%llx length=%llx low = 0x%llx high=0x%llx\n"
			, __func__, __LINE__, md_pos_addr
			, md_pos_length, low_bound, high_bound);
#endif
	spin_lock(&amms_pending_lock);
	amms_pending_request_bitmap |= AMMS_PENDING_POS_ALLOC_BIT;
	is_POS_free = AMMS_POS_OWN_BY_AP_PROCESS_TO_MD;
	spin_unlock(&amms_pending_lock);
	gicd_set_ispendr(BASE_GICD_BASE, ATF_AMMS_IRQ_ID);

pos_mpu_out:
	reply.error = 0;
	reply.seq_id = request->seq_id;
	/* reply.seq_id = 1; */
#ifdef AMMS_DEBUG
	console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
	NOTICE("reply.status=%d\n", reply.status);
	console_uninit();
#endif

	ret = ccif_irq0_user_write(
		(unsigned int *)&reply,
		((sizeof(reply)/4)+(((sizeof(reply)%4) == 0)?0:1)), 0);

	if (ret < 0) {
#ifdef AMMS_DEBUG
		console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
#endif
		NOTICE("ccif_irq0_user_write failed ret = %d\n", ret);
#ifdef AMMS_DEBUG
		console_uninit();
#endif
	}

	ret = ccif_irq0_user_notify_md(0);
	if (ret < 0) {
#ifdef AMMS_DEBUG
		console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
#endif
		NOTICE("ccif_irq0_user_notify_md failed ret = %d\n", ret);
#ifdef AMMS_DEBUG
		console_uninit();
#endif
	}
}


void amms_cmd_is_POS_free(struct amms_msg_request *request)
{
	int ret;
	struct amms_msg_response reply;

	memset(&reply, 0, sizeof(reply));
	/* dump info */
	NOTICE("cmd =[AMMS_CMD_QUERY_POS_STATUS]\n");
	NOTICE("seq_id = %d is_POS_free=%d md_view_pos_addr=%llx\n",
		request->seq_id, is_POS_free, md_view_pos_addr);
	/* reply now */
	/* reply.status = AMMS_STATUS_INVALID; */
	if (is_POS_free == AMMS_POS_OWN_BY_AP ||
		is_POS_free == AMMS_POS_OWN_BY_AP_PROCESS_TO_MD)
		/*when process from AP to MD , it still belong to AP*/
		reply.status = AMMS_MEM_OWNER_AP;
	else
		reply.status = AMMS_MEM_OWNER_MD;

	reply.error = 0;
	reply.seq_id = request->seq_id;
	reply.u_resp.init_reply.range.start_address = md_view_pos_addr;
	reply.u_resp.init_reply.range.length = md_pos_length;
	/* reply.seq_id = 1; */
	NOTICE("reply.status=%d\n", reply.status);
	ret = ccif_irq0_user_write((unsigned int *)&reply,
		((sizeof(reply)/4)+(((sizeof(reply)%4) == 0)?0:1)), 0);

	if (ret < 0)
		ERROR("ccif_irq0_user_write failed ret = %d\n", ret);

	ret = ccif_irq0_user_notify_md(0);
	if (ret < 0)
		ERROR("ccif_irq0_user_notify_md failed ret = %d\n", ret);
}

void amms_cmd_STATIC_FREE_handler(struct amms_msg_request *request)
{
	int ret;
	struct amms_msg_response reply;

	memset(&reply, 0, sizeof(reply));
	struct amms_address_range range = request->u_req.free_static_cmd.range;
	unsigned long long low_bound, high_bound;
	/* dump info */
#ifdef AMMS_DEBUG
	console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
#endif
	NOTICE("cmd =[AMMS_CMD_STATIC_FREE], seq_id = %d\n",
		   request->seq_id);
	NOTICE("start=%x length=%x is_DRDI_free=%d\n",
		range.start_address, range.length, is_DRDI_free);
#ifdef AMMS_DEBUG
	console_uninit();
#endif
	/* reply now */
	/* reply.status = AMMS_STATUS_INVALID; */
	/*validate command message*/
	if (is_DRDI_free) {
		reply.status = AMMS_STATUS_FAIL;
		goto static_mpu_out;
	}

	/*align to 64k */
	/*we only free high bound align part */
	high_bound = ((range.start_address + range.length) >> 16)<<16;
	low_bound = range.start_address;
	/*we want to free memory to kernel*/
	/* so if not align , we can only free align part */
	if (low_bound % 0x10000)
		low_bound = ((low_bound + 0x10000) >> 16) << 16;
	if (high_bound <= low_bound) {
		/* we do not apply any MPU setting but just return success*/
		reply.status = AMMS_STATUS_SUCCESS;
		NOTICE("invalid low_bound =0x%llx high_bound=0x%llx\n",
				low_bound, high_bound);
		NOTICE("MPU no apply\n");
		goto static_mpu_out;
	}


	if (md_base_address == 0) {
		NOTICE("AMMS: md_base address is not set, panic\n");
		panic();
	}
#if 1
	is_DRDI_free = 1;
#if defined(MTK_ENABLE_MPU_HAL_SUPPORT)
	{
	uint64_t mpu_addr_s = low_bound + md_base_address;
	uint64_t mpu_size = (high_bound - low_bound);

	sip_feat_mpu_request(MPU_REQ_ORIGIN_ATF_ZONE_AMMS_STATIC,
			     MPU_FEAT_AMMS_STATIC_FREE_TO_AP,
			     mpu_addr_s, mpu_size);
	}
#else
	ERROR("%s %d: AMMS MPU permission is not set!\n", __func__, __LINE__);
#endif
	reply.status  = AMMS_STATUS_SUCCESS;
#endif
	/*TO-DO:we can notify kernel to get back memory to use now*/

	amms_free_addr = low_bound + md_base_address;
	amms_free_length = high_bound - low_bound;
#if 0
	NOTICE("%s:%d md_base=0x%llx free address=%llx free length=%llx low = 0x%llx high=0x%llx\n"
			, __func__, __LINE__, md_base_address,
			amms_free_addr, amms_free_length, low_bound, high_bound);
#endif

	spin_lock(&amms_pending_lock);
	amms_pending_request_bitmap |= AMMS_PENDING_DRDI_FREE_BIT;
	spin_unlock(&amms_pending_lock);
	gicd_set_ispendr(BASE_GICD_BASE, ATF_AMMS_IRQ_ID);

	/**/
static_mpu_out:
	reply.error = 0;
	reply.seq_id = request->seq_id;
	/* reply.seq_id = 1; */
#ifdef AMMS_DEBUG
	console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
	NOTICE("reply.status=%d\n", reply.status);
	console_uninit();
#endif
	ret = ccif_irq0_user_write((unsigned int *)&reply
		, ((sizeof(reply)/4)+(((sizeof(reply)%4) == 0)?0:1)), 0);
	if (ret < 0) {
#ifdef AMMS_DEBUG
		console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
#endif
		NOTICE("ccif_irq0_user_write failed ret = %d\n", ret);
#ifdef AMMS_DEBUG
		console_uninit();
#endif
	}
	ret = ccif_irq0_user_notify_md(0);
	if (ret < 0) {
#ifdef AMMS_DEBUG
		console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
#endif
		NOTICE("ccif_irq0_user_notify_md failed ret = %d\n", ret);
#ifdef AMMS_DEBUG
		console_uninit();
#endif
	}
}
/* this command is binding with DRDI originally
 * POS will not need this command
 */
void amms_cmd_MPU_handler(struct amms_msg_request *request)
{
	int ret;
	struct amms_msg_response reply;

	memset(&reply, 0, sizeof(reply));
	uint mpu_id = request->u_req.mpu_cmd.mpu_id;
	uint per = request->u_req.mpu_cmd.per;
	struct amms_address_range range = request->u_req.mpu_cmd.range;
	unsigned long long low_bound, high_bound;
	/* dump info */
#ifdef AMMS_DEBUG
	console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
#endif
	NOTICE("cmd =[AMMS_CMD_MPU], seq_id = %d\n", request->seq_id);
	NOTICE("per=%d mpu_id=%d start=0x%x length=0x%x\n",
			per, mpu_id, range.start_address, range.length);
#ifdef AMMS_DEBUG
	console_uninit();
#endif
	/* reply now */
	/* reply.status = AMMS_STATUS_INVALID; */
	/*validate command message*/
	/* if is_DRDI_free =1  , we only support 1 EMI MPU so we must reject the*/
	/* MPU command request since this MPU is occupied*/
	if (mpu_id > AMMS_SUPPORT_MPU_NUM ||
		per >= ((unsigned char)AMMS_MD_PER_LAST)) {
		reply.status = AMMS_STATUS_INVALID;
		goto mpu_out;
	}

	/* we need to perform range validation first*/
	/* since MPU must be 64k align, we need*/
	/* to make low bound to 64K and higher bound to 64K as well*/
	high_bound = range.start_address + range.length;
	if (high_bound % 0x10000)
		high_bound = ((high_bound + 0x10000) >> 16) << 16;

	low_bound = ((range.start_address)>>16)<<16;
	if (high_bound <= low_bound) {
		reply.status = AMMS_STATUS_INVALID;
		NOTICE("invalid low_bound =0x%llx high_bound=0x%llx\n", low_bound, high_bound);
		goto mpu_out;
	}

#if defined(MTK_ENABLE_MPU_HAL_SUPPORT)
	{
	uint64_t mpu_addr_s = low_bound + md_base_address;
	uint64_t mpu_size = (high_bound - low_bound);

	switch (per) {
	case R_ONLY:
		sip_feat_mpu_request(MPU_REQ_ORIGIN_ATF_ZONE_AMMS_STATIC,
				     MPU_FEAT_AMMS_STATIC_MD_READ_ONLY,
				     mpu_addr_s, mpu_size);
		break;
	case R_WRITE:
		sip_feat_mpu_request(MPU_REQ_ORIGIN_ATF_ZONE_AMMS_STATIC,
				     MPU_FEAT_AMMS_STATIC_MD_READ_WRITE,
				     mpu_addr_s, mpu_size);
		break;
	case R_DISABLE:
		sip_feat_mpu_request(MPU_REQ_ORIGIN_ATF_ZONE_AMMS_STATIC,
				     MPU_FEAT_AMMS_STATIC_MD_DISABLE,
				     mpu_addr_s, mpu_size);
		break;
	default:
		ERROR("%s: invalid request per=%u!\n", __func__, per);
		break;
	}
	}
#else
	ERROR("%s %d: AMMS MPU permission is not set!\n", __func__, __LINE__);
#endif

	reply.status  = AMMS_STATUS_SUCCESS;
mpu_out:
	reply.error = 0;
	reply.seq_id = request->seq_id;
	/* reply.seq_id = 1; */
#ifdef AMMS_DEBUG
	console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
	NOTICE("reply.status=%d\n", reply.status);
	console_uninit();
#endif
	ret = ccif_irq0_user_write(
		(unsigned int *)&reply,
		((sizeof(reply)/4)+
		(((sizeof(reply)%4) == 0)?0:1)), 0);
	if (ret < 0) {
#ifdef AMMS_DEBUG
		console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
#endif
		ERROR("ccif_irq0_user_write failed ret = %d\n", ret);
#ifdef AMMS_DEBUG
		console_uninit();
#endif
	}
	ret = ccif_irq0_user_notify_md(0);
	if (ret < 0) {
#ifdef AMMS_DEBUG
		console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
#endif
		ERROR("ccif_irq0_user_notify_md failed ret = %d\n", ret);
#ifdef AMMS_DEBUG
		console_uninit();
#endif
	}
}

void amms_cmd_DRDI_free(struct amms_msg_request *request)
{
	int ret;
	struct amms_msg_response reply;

	memset(&reply, 0, sizeof(reply));
	/* dump info */
	NOTICE("cmd =[AMMS_CMD_QUERY_DRDI_STATUS]\n");
	NOTICE("seq_id = %d is_DRDI_free=%d\n",
		request->seq_id, is_DRDI_free);
	/* reply now */
	/* reply.status = AMMS_STATUS_INVALID; */
	if (is_DRDI_free == 1)
		reply.status = AMMS_MEM_OWNER_AP;
	else
		reply.status = AMMS_MEM_OWNER_MD;

	reply.error = 0;
	reply.seq_id = request->seq_id;
	/* reply.seq_id = 1; */
	NOTICE("reply.status=%d\n", reply.status);
	ret = ccif_irq0_user_write((unsigned int *)&reply, ((sizeof(reply)/4)+(((sizeof(reply)%4) == 0)?0:1)), 0);

	if (ret < 0)
		ERROR("ccif_irq0_user_write failed ret = %d\n", ret);

	ret = ccif_irq0_user_notify_md(0);
	if (ret < 0)
		ERROR("ccif_irq0_user_notify_md failed ret = %d\n", ret);
}

void amms_cmd_default_handler(struct amms_msg_request *request)
{
	int ret;
	struct amms_msg_response reply;

	memset(&reply, 0, sizeof(reply));
	/* dump info */
	NOTICE("%s:cmd =[unknown command], seq_id = %d\n"
		, __func__, request->seq_id);
	/* reply now */
	reply.status = AMMS_STATUS_INVALID;
	/* reply.status = AMMS_STATUS_SUCCESS; */
	reply.error = 0;
	reply.seq_id = request->seq_id;
	/* reply.seq_id = 1; */
	NOTICE("reply=%d\n", reply.status);
	ret = ccif_irq0_user_write((unsigned int *)&reply
		, ((sizeof(reply)/4)+(((sizeof(reply)%4) == 0)?0:1)), 0);

	if (ret < 0)
		NOTICE("ccif_irq0_user_write failed ret = %d\n", ret);

	ret = ccif_irq0_user_notify_md(0);

	if (ret < 0)
		NOTICE("ccif_irq0_user_notify_md failed ret = %d\n", ret);
}

void amms_cmd_init_handler(struct amms_msg_request *request)
{

	int ret;
	struct amms_msg_response reply;

	memset(&reply, 0, sizeof(reply));

	reply.status = AMMS_STATUS_SUCCESS;
	reply.error = 0;
	reply.seq_id = request->seq_id;
	/*we reply 2 as we support POS feature*/
	reply.u_resp.init_reply.version =
		AMMS_VERSION_CAP_DRDI|AMMS_VERSION_CAP_POS;
	reply.u_resp.init_reply.debug = 1;
	reply.u_resp.init_reply.support_mpu_num = AMMS_SUPPORT_MPU_NUM;

/*	NOTICE("reply.status=%d\n", reply.status);*/

	ret = ccif_irq0_user_write((unsigned int *)&reply,
		((sizeof(reply)/4)+(((sizeof(reply)%4) == 0)?0:1)), 0);
	if (ret < 0)
		NOTICE("ccif_irq0_user_write failed ret = %d\n", ret);
	ret = ccif_irq0_user_notify_md(0);
	if (ret < 0)
		NOTICE("ccif_irq0_user_notify_md failed ret = %d\n", ret);
}

int handle_amms_request(struct amms_msg_request *request)
{
	int ret;
	unsigned char cmd;
#ifdef AMMS_DEBUG
	console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
#if 0
	NOTICE("dump 20 bytes 0x%x 0x%x 0x%x 0x%x 0x%x\n",
		*((unsigned int *)request), *((unsigned int *)request+1),
		*((unsigned int *)request+2), *((unsigned int *)request+3),
		*((unsigned int *)request+4));
	console_uninit();
#endif
#endif
	cmd = request->cmd;
	switch (cmd) {

	case AMMS_CMD_INIT:
		amms_cmd_init_handler(request);
		ret = 0;
		break;
	case AMMS_CMD_MPU:
		amms_cmd_MPU_handler(request);
		ret = 0;
		break;
	case AMMS_CMD_QUERY_DRDI_STATUS:
		amms_cmd_DRDI_free(request);
		ret = 0;
		break;
	case AMMS_CMD_STATIC_FREE:
		amms_cmd_STATIC_FREE_handler(request);
		ret = 0;
		break;
	case AMMS_CMD_DEALLOCATE_POS_BUFFER:
		amms_cmd_AMMS_CMD_DEALLOCATE_POS_BUFFEE_handler(request);
		ret = 0;
		break;
	case AMMS_CMD_ALLOCATE_POS_BUFFER:
		amms_cmd_AMMS_CMD_ALLOCATE_POS_BUFFEE_handler(request);
		ret = 0;
		break;
	case AMMS_CMD_QUERY_POS_STATUS:
		amms_cmd_is_POS_free(request);
		ret = 0;
		break;

	default:
		amms_cmd_default_handler(request);
		ret = -1;
		break;
	}

	return ret;
}

unsigned long long sip_amms_set_md_base_address(unsigned long long base_addr)
{
	NOTICE("%s:%d [ap cmd] md_base_address = %llx\n"
		, __func__, __LINE__, base_addr);
	if (!md_base_address)
		md_base_address = base_addr;
	return 0;
}

unsigned long long sip_amms_get_md_base_address(void)
{
	NOTICE("%s:%d [ap cmd]md_base_address = %llx\n"
		, __func__, __LINE__, md_base_address);
	return md_base_address;
}

unsigned long long sip_kernel_amms_get_free_addr(void)
{
	NOTICE("%s:%d [ap cmd]free address = %llx\n"
		, __func__, __LINE__, amms_free_addr);
	return amms_free_addr;
}

unsigned long long sip_kernel_amms_get_free_length(void)
{
	NOTICE("%s:%d [ap cmd]length = %llx\n",
		__func__, __LINE__, amms_free_length);
	return amms_free_length;
}

unsigned long long sip_amms_set_md_pos_addr(
		unsigned long long base_addr)
{
	NOTICE("%s:%d [ap cmd]md_pos_addr = %llx\n"
		, __func__, __LINE__, base_addr);
	if (!md_pos_addr)
		md_pos_addr = base_addr;
	return 0;
}

unsigned long long sip_amms_set_md_pos_length(unsigned long long length)
{
	NOTICE("%s:%d [ap cmd]md_pos_length = %llx\n"
		, __func__, __LINE__, length);
	if (!md_pos_length)
		md_pos_length = length;
	return 0;
}

unsigned long long sip_amms_get_md_pos_addr(void)
{
	NOTICE("%s:%d [ap cmd]md_pos_addr = %llx\n"
		, __func__, __LINE__, md_pos_addr);
	return md_pos_addr;
}

unsigned long long sip_amms_set_md_view_pos_addr(unsigned long long base_addr)
{
	NOTICE("%s:%d [ap cmd]md_view_pos_addr = %llx\n"
		, __func__, __LINE__, base_addr);
	if (!md_view_pos_addr)
		md_view_pos_addr = base_addr;
	return 0;
}

unsigned long long sip_amms_get_md_view_pos_addr(void)
{
	NOTICE("%s:%d [ap cmd]md_view_pos_addr = %llx\n",
		__func__, __LINE__, md_view_pos_addr);
	return md_view_pos_addr;
}

unsigned long long sip_amms_get_md_pos_length(void)
{
	NOTICE("%s:%d [ap cmd]md_pos_length = %llx\n",
		__func__, __LINE__, md_pos_length);
	return md_pos_length;
}

unsigned int sip_amms_get_pending(void)
{
	unsigned int pending;

	NOTICE("%s:%d [ap cmd]amms_pending = %x\n",
		__func__, __LINE__, amms_pending_request_bitmap);
	spin_lock(&amms_pending_lock);
	pending = amms_pending_request_bitmap;
	spin_unlock(&amms_pending_lock);
	return pending;
}

void sip_amms_clear_pending(unsigned int ack_pending)
{
	spin_lock(&amms_pending_lock);
	NOTICE("%s:%d [ap cmd]ack_pending = %x\n",
		__func__, __LINE__, ack_pending);
	NOTICE("amms_pending_request_bitmap=0x%x\n",
	amms_pending_request_bitmap);
	if (ack_pending & AMMS_PENDING_POS_DEALLOC_BIT)
		/*MPU setting should be applied first
		 * , or kernel can not use this memory
		 *so we do not need to apply MPU setting here
		 */
		is_POS_free = AMMS_POS_OWN_BY_AP;

	if (ack_pending & AMMS_PENDING_POS_ALLOC_BIT) {
		is_POS_free = AMMS_POS_OWN_BY_MD;
		/*kernel just ack memory is return to MD */
		/*then we should apply MPU setting here*/
		amms_md_pos_mpu_change(AMMS_POS_OWNER_MD);
	}

	if (ack_pending & AMMS_PENDING_DRDI_FREE_BIT)
		is_DRDI_free = 1;

	amms_pending_request_bitmap &= ~ack_pending;
	spin_unlock(&amms_pending_lock);
}

