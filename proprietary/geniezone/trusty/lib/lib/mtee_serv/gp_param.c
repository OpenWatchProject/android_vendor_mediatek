#include <tz_cross/trustzone.h>
#include <stdio.h>
#include <string.h>

#include <gp_param.h>
#include <gz_service.h>

//#define GZ_GP_DEBUG

#if defined (GZ_GP_DEBUG)
#define GP_DEBUG(fmt, ...) printf("[gp_param]"fmt, __VA_ARGS__)
#else
#define GP_DEBUG(fmt, ...)
#endif

void rewrite_param_mem_addr(uint32_t paramTypes, MTEEC_PARAM *params, uint32_t in_buffer)
{
	uint32_t type, offset, size;
	int i;

	offset = in_buffer;
	for (i = 0; i < 4; i++) {
		type = TZ_GetParamTypes(paramTypes, i);

		if (type == TZPT_MEM_INPUT || type == TZPT_MEM_INOUT) {
			// save size first
			size = params[i].mem64.size;

			params[i].mem.buffer = offset; // use 32bit addr instead in GZ
			params[i].mem.size = size;
			GP_DEBUG("rewrite mem addr %i to 0x%x, size %u\n", i, offset, size);
			offset += params[i].mem.size;
		}
	}
}

void rewrite_inout_param_mem_addr(uint32_t paramTypes, MTEEC_PARAM *params, uint64_t in_buffer, uint64_t out_buffer)
{
	uint32_t type, size;
	uint64_t offset, out_offset;
	int i;

	offset = in_buffer;
	out_offset = out_buffer;
	GP_DEBUG("RWIOWA: head of in buffer: 0x%llx, out buffer: 0x%llx\n", (uint64_t)in_buffer, (uint64_t)out_buffer);

	for (i = 0; i < 4; i++) {
		type = TZ_GetParamTypes(paramTypes, i);

		// save size first
		size = params[i].mem64.size;

		if (type == TZPT_MEM_INPUT || type == TZPT_MEM_INOUT) {
			params[i].mem.buffer = offset; // use 32bit addr instead in GZ
			params[i].mem.size = size;
			GP_DEBUG("RWIOMA: rewrite in mem addr %i to 0x%x, size %u\n", i, params[i].mem.buffer, size);
			offset += params[i].mem.size;

			if (type == TZPT_MEM_INOUT) {
				out_offset += params[i].mem.size;
			}
		} else if(type == TZPT_MEM_OUTPUT) {
			params[i].mem.buffer = out_offset; // use 32bit addr instead in GZ
			params[i].mem.size = size;
			GP_DEBUG("RWIOMA: rewrite out mem addr %i to 0x%x, size %u\n", i, params[i].mem.buffer, size);
			out_offset += params[i].mem.size;
		}
	}
}

int copy_mem_to_buffer(uint32_t paramTypes, MTEEC_PARAM *params, void *buffer)
{
	uint32_t type, size, offset = 0;
	int i;
	void *addr;

	if (!buffer) {
		GP_DEBUG("ERR: invalid gp buffer 0x%x\n", buffer);
		return -1;
	}

	for (i = 0; i < 4; i++) {
		type = TZ_GetParamTypes(paramTypes, i);

		if (type == TZPT_MEM_OUTPUT || type == TZPT_MEM_INOUT || type == TZPT_MEM_INPUT) {

			addr = (void *)params[i].mem.buffer;
			size = params[i].mem.size;

			if ((offset + size) > GZ_MSG_DATA_MAX_LEN) {
				GP_DEBUG("ERR: parameter data to copy exceeds max len, offset=%u\n", offset);
				return -1;
			}

			if (!addr) {
				GP_DEBUG("ERR: invalid gp mem param, src addr = 0x%x\n", addr);
				continue;
			}
			/*
			if (size > GZ_MEM_MAX_LEN) {
				GP_DEBUG("ERR: invalid gp mem param, size(%u) exceed max len(%u)\n", size, GZ_MEN_MAX_LEN);
				continue;
			}
			*/

			memcpy(buffer + offset, addr, size);
			GP_DEBUG("CPTIB: copy mem %i from 0x%x to 0x%x, size=%u\n", i, (uint32_t)addr, (uint32_t)buffer + offset, size);

			offset += size;
		}
	}
	return offset;
}

int copy_inout_mem_to_buffer(uint32_t paramTypes, MTEEC_PARAM *params,
		void *buffer, int is_server)
{
	uint32_t type, size, offset = 0;
	int i;
	void *addr;

	if (!buffer) {
		GP_DEBUG("ERR: invalid gp buffer 0x%x\n", buffer);
		return -1;
	}

	GP_DEBUG("CPTB: head of out buffer: 0x%x\n", (uint32_t)buffer);
	for (i = 0; i < 4; i++) {
		type = TZ_GetParamTypes(paramTypes, i);

		if (type == TZPT_MEM_INOUT
			|| (is_server && type == TZPT_MEM_OUTPUT)
			|| (!is_server && type == TZPT_MEM_INPUT)) {

			addr = (void *)params[i].mem.buffer; // this should point to input buffer
			size = params[i].mem.size;

			if ((offset + size) > GZ_MSG_DATA_MAX_LEN) {
				GP_DEBUG("ERR: parameter data to copy exceeds max len, offset=%u\n", offset);
				return -1;
			}

			if (!addr) {
				GP_DEBUG("WARN: invalid gp mem param, src addr = 0x%x\n", addr);
				continue;
			}
			/*
			if (size > GZ_MEM_MAX_LEN) {
				GP_DEBUG("WARN: invalid gp mem param, size(%u) exceed max len(%u)\n", size, GZ_MEN_MAX_LEN);
				continue;
			}
			*/

			if (type == TZPT_MEM_INOUT) {
				memcpy(&buffer[offset], addr, size);
				GP_DEBUG("CPTB: copy mem %i from 0x%x to 0x%x, size=%u\n", i, (uint32_t)addr, (uint32_t)buffer + offset, size);
			}

			offset += size;
		}
	}
	return offset;
}

int copy_out_mem_from_buffer(uint32_t paramTypes, MTEEC_PARAM *params, void *buffer)
{
	uint32_t type, size, offset = 0;
	int i;
	void *addr;

	if (!buffer) {
		GP_DEBUG("ERR: invalid gp buffer 0x%x\n", buffer);
		return -1;
	}

	GP_DEBUG("CPFB: head of out buffer: 0x%x\n", (uint32_t)buffer);
	for (i = 0; i < 4; i++) {
		type = TZ_GetParamTypes(paramTypes, i);

		if (type == TZPT_MEM_INOUT || type == TZPT_MEM_OUTPUT) {

			addr = (void *)params[i].mem.buffer;
			size = params[i].mem.size;

			if ((offset + size) > GZ_MSG_DATA_MAX_LEN) {
				GP_DEBUG("ERR: parameter data to copy exceeds max len, offset=%u\n", offset);
				return -1;
			}

			if (!addr) {
				GP_DEBUG("WARN: invalid gp mem param, src addr = 0x%x\n", addr);
				continue;
			}
			/*
			if (size > GZ_MEM_MAX_LEN) {
				GP_DEBUG("WARN: invalid gp mem param, size(%u) exceed max len(%u)\n", size, GZ_MEN_MAX_LEN);
				continue;
			}
			*/

			memcpy(addr, &buffer[offset], size);
			GP_DEBUG("CPFB: copy mem %i from 0x%x to 0x%x, size=%u\n", i, (uint32_t)buffer + offset, (uint32_t)addr, size);

			offset += size;
		}
	}
	return offset;
}
