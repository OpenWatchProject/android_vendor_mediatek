#include <debug.h>
#include <mmio.h>
#include <mtk_plat_common.h>
#include <platform.h>
#include <platform_def.h>
#include <console.h>
#include <amms.h>
#include <pccif.h>
#include <string.h>
#include <emi_drv.h>
#include <gic_v2.h>
#include <gic_v3.h>
#include <plat_dramc.h>
#define DMB()	(__asm__ volatile("dmb sy" : : : "memory"))

/* #define AMMS_DEBUG 1 */

/*global variable */
int is_DRDI_free;
unsigned long long md_base_address;
unsigned long long amms_free_addr, amms_free_length;
void amms_cmd_STATIC_FREE_handler(struct amms_msg_request *request)
{
	int ret;
	struct amms_msg_response reply;

	memset(&reply, 0, sizeof(reply));
	unsigned long long mpu_per;
	struct amms_address_range range = request->u_req.free_static_cmd.range;
	unsigned long long low_bound, high_bound;
	/* dump info */
#ifdef AMMS_DEBUG
	console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
#endif
	NOTICE("%s\n", __func__);
	NOTICE("cmd = %d, seq_id = %d start=%x length=%x\n",
			request->cmd, request->seq_id, range.start_address, range.length);
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
	is_DRDI_free = 1;
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
		NOTICE("invalid low_bound =0x%llx high_bound=0x%llx MPU no apply\n", low_bound, high_bound);
		goto static_mpu_out;
	}

	mpu_per = SET_ACCESS_PERMISSON(UNLOCK,
	SEC_R_NSEC_R, SEC_R_NSEC_RW, FORBIDDEN, SEC_R_NSEC_RW,
	FORBIDDEN, FORBIDDEN, SEC_R_NSEC_R, SEC_R_NSEC_RW);


	if (md_base_address == 0) {
		NOTICE("AMMS: md_base address is not set, panic\n");
		panic();
	}
#if 1
	emi_mpu_set_region_protection((low_bound + md_base_address), (high_bound + md_base_address) - 1
		, AMMS_DYNAMIC_REGION_ID1, mpu_per);
	reply.status  = AMMS_STATUS_SUCCESS;
#endif
	/*TO-DO:we can notify kernel to get back memory to use now*/

	amms_free_addr = low_bound + md_base_address;
	amms_free_length = high_bound - low_bound;
	NOTICE("%s:%d md_base=0x%llx free address=%llx free length=%llx low = 0x%llx high=0x%llx\n"
			, __func__, __LINE__, md_base_address,
			amms_free_addr, amms_free_length, low_bound, high_bound);
	gicd_set_ispendr(BASE_GICD_BASE, ATF_AMMS_IRQ_ID);

	/**/
static_mpu_out:
	reply.error = 0;
	reply.seq_id = request->seq_id;
	/* reply.seq_id = 1; */
#ifdef AMMS_DEBUG
	console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
	NOTICE("reply=0x%x 0x%x\n", *(unsigned int *)&reply, *((unsigned int *)&reply+1));
	console_uninit();
#endif
	ret = ccif_irq0_user_write((unsigned int *)&reply, ((sizeof(reply)/4)+(((sizeof(reply)%4) == 0)?0:1)), 0);
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

void amms_cmd_MPU_handler(struct amms_msg_request *request)
{
	int ret;
	struct amms_msg_response reply;

	memset(&reply, 0, sizeof(reply));
	uint mpu_id = request->u_req.mpu_cmd.mpu_id;
	uint per = request->u_req.mpu_cmd.per;
	unsigned long long mpu_per, mpu_per_dummy;
	struct amms_address_range range = request->u_req.mpu_cmd.range;
	unsigned long long low_bound, high_bound;
	/* dump info */
#ifdef AMMS_DEBUG
	console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
#endif
	NOTICE("%s\n", __func__);
	NOTICE("cmd = %d, seq_id = %d,per=%d mpu_id=%d start=0x%x length=0x%x\n",
			request->cmd, request->seq_id, per, mpu_id, range.start_address, range.length);
#ifdef AMMS_DEBUG
	console_uninit();
#endif
	/* reply now */
	/* reply.status = AMMS_STATUS_INVALID; */
	/*validate command message*/
	/* if is_DRDI_free =1  , we only support 1 EMI MPU so we must reject the*/
	/* MPU command request since this MPU is occupied*/
	if (mpu_id > AMMS_SUPPORT_MPU_NUM || per >= ((unsigned char)AMMS_MD_PER_LAST) || is_DRDI_free) {
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

	if (per == R_ONLY) {
		mpu_per = SET_ACCESS_PERMISSON(UNLOCK,
		SEC_R_NSEC_R, FORBIDDEN, FORBIDDEN, FORBIDDEN,
		FORBIDDEN, FORBIDDEN, SEC_R_NSEC_R, FORBIDDEN);
	} else if (per == R_WRITE) {
		mpu_per = SET_ACCESS_PERMISSON(UNLOCK,
		SEC_R_NSEC_RW, FORBIDDEN, FORBIDDEN, FORBIDDEN,
		FORBIDDEN, FORBIDDEN, SEC_R_NSEC_RW, FORBIDDEN);
	} else if (per == R_DISABLE) {
		mpu_per = SET_ACCESS_PERMISSON(UNLOCK,
		NO_PROTECTION, NO_PROTECTION, NO_PROTECTION, NO_PROTECTION,
		NO_PROTECTION, NO_PROTECTION, NO_PROTECTION, NO_PROTECTION);
	} else {
		NOTICE("%s: ilegal permission should not happened\n", __func__);
	}


	mpu_per_dummy = SET_ACCESS_PERMISSON(UNLOCK,
		NO_PROTECTION, NO_PROTECTION, NO_PROTECTION, NO_PROTECTION,
		NO_PROTECTION, NO_PROTECTION, NO_PROTECTION, NO_PROTECTION);

	if (per == R_DISABLE) {
	/*set permission no protection equal to disable*/
		emi_mpu_set_region_protection((low_bound + md_base_address), (high_bound + md_base_address) - 1
			, AMMS_DYNAMIC_REGION_ID1, mpu_per_dummy);
	} else {
		/*set correct address with permission*/
		emi_mpu_set_region_protection((low_bound + md_base_address), (high_bound + md_base_address) - 1
	, AMMS_DYNAMIC_REGION_ID1, mpu_per);
	}

	reply.status  = AMMS_STATUS_SUCCESS;
mpu_out:
	reply.error = 0;
	reply.seq_id = request->seq_id;
	/* reply.seq_id = 1; */
#ifdef AMMS_DEBUG
	console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);
	NOTICE("reply=0x%x 0x%x\n", *(unsigned int *)&reply, *((unsigned int *)&reply+1));
	console_uninit();
#endif
	ret = ccif_irq0_user_write((unsigned int *)&reply, ((sizeof(reply)/4)+(((sizeof(reply)%4) == 0)?0:1)), 0);
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
	NOTICE("%s\n", __func__);
	NOTICE("cmd = %d, seq_id = %d\n", request->cmd, request->seq_id);
	/* reply now */
	/* reply.status = AMMS_STATUS_INVALID; */
	if (is_DRDI_free == 0)
		reply.status = AMMS_STATUS_SUCCESS;
	else
		reply.status = AMMS_STATUS_FAIL;

	reply.error = 0;
	reply.seq_id = request->seq_id;
	/* reply.seq_id = 1; */
	NOTICE("reply=0x%x 0x%x\n", *(unsigned int *)&reply, *((unsigned int *)&reply+1));
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
	NOTICE("%s:cmd = %d, seq_id = %d\n", __func__, request->cmd, request->seq_id);
	/* reply now */
	reply.status = AMMS_STATUS_INVALID;
	/* reply.status = AMMS_STATUS_SUCCESS; */
	reply.error = 0;
	reply.seq_id = request->seq_id;
	/* reply.seq_id = 1; */
	NOTICE("reply=0x%x 0x%x\n", *(unsigned int *)&reply, *((unsigned int *)&reply+1));
	ret = ccif_irq0_user_write((unsigned int *)&reply, ((sizeof(reply)/4)+(((sizeof(reply)%4) == 0)?0:1)), 0);

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
	/* dump info */
	NOTICE("%s\n", __func__);
	NOTICE("cmd = %d, seq_id = %d version=%d\n"
			, request->cmd, request->seq_id,
			request->u_req.init_request_cmd.version);
	/* reply now */
	reply.status = AMMS_STATUS_SUCCESS;
	reply.error = 0;
	reply.seq_id = request->seq_id;
	reply.u_resp.init_reply.version = 1;
	reply.u_resp.init_reply.debug = 1;
	reply.u_resp.init_reply.support_mpu_num = AMMS_SUPPORT_MPU_NUM;

	NOTICE("reply=0x%x 0x%x\n", *(unsigned int *)&reply, *((unsigned int *)&reply+1));

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
	NOTICE("dump 20 bytes 0x%x 0x%x 0x%x 0x%x 0x%x\n",
		*((unsigned int *)request), *((unsigned int *)request+1),
		*((unsigned int *)request+2), *((unsigned int *)request+3),
		*((unsigned int *)request+4));
	console_uninit();
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
	default:
		amms_cmd_default_handler(request);
		ret = -1;
		break;
	}

	return ret;
}

unsigned long long sip_amms_set_md_base_address(unsigned long long base_addr)
{
	NOTICE("%s:%d md_base_address = %llx\n", __func__, __LINE__, base_addr);
	if (!md_base_address)
		md_base_address = base_addr;
	return 0;
}

unsigned long long sip_amms_get_md_base_address(void)
{
	NOTICE("%s:%d md_base_address = %llx\n", __func__, __LINE__, md_base_address);
	return md_base_address;
}

unsigned long long sip_kernel_amms_get_free_addr(void)
{
	NOTICE("%s:%d free address = %llx\n", __func__, __LINE__, amms_free_addr);
	return amms_free_addr;
}

unsigned long long sip_kernel_amms_get_free_length(void)
{
	NOTICE("%s:%d length = %llx\n", __func__, __LINE__, amms_free_length);
	return amms_free_length;
}
