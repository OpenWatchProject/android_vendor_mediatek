/*
 * Copyright (C) 2013-2015 The Android Open Source Project
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

#include <printf.h>
#include <stdio.h>
#include <string.h>
#include <trusty_std.h>
#include <err.h>
#include <tz_private/sys_ipc.h>
#include <assert.h>

#define LINE_BUFFER_SIZE 128

#if CFG_LOG_WORKAROUND
MTEE_MUTEX *stdio_mutex = NULL;

static void __attribute__((constructor))
stdio_init (void)
{
	printf("constructor: %s \n", __func__);

	stdio_mutex = MTEE_CreateMutex();
	assert(stdio_mutex!=NULL);
}

static __attribute__((destructor))
stdio_exit (void)
{
	int rc;
	printf("destructor: %s \n", __func__);
	rc = MTEE_DestoryMutex(stdio_mutex);
	assert(rc==0);
}
#endif

struct file_buffer {
	char data[LINE_BUFFER_SIZE];
	size_t pos;
};

struct file_context {
	int fd;
	struct file_buffer *buffer;
};

static int buffered_put(struct file_buffer *buffer, int fd, char c)
{
	int result = 0;

	buffer->data[buffer->pos++] = c;
	if (buffer->pos == sizeof(buffer->data) || c == '\n') {
		result = write(fd, buffer->data, buffer->pos);
		buffer->pos = 0;
	}
	return result;
}

static int buffered_write(struct file_context *ctx, const char *str, size_t sz)
{
	unsigned i;

	if (!ctx->buffer) {
		return ERR_INVALID_ARGS;
	}

#if CFG_LOG_WORKAROUND
	if (stdio_mutex!=NULL) MTEE_LockMutex(stdio_mutex);
#endif

	for (i = 0; i < sz; i++) {
		int result = buffered_put(ctx->buffer, ctx->fd, str[i]);
		if (result < 0) {

#if CFG_LOG_WORKAROUND
			if (stdio_mutex!=NULL) MTEE_UnlockMutex(stdio_mutex);
#endif

			return result;
		}
	}

#if CFG_LOG_WORKAROUND
	if (stdio_mutex!=NULL) MTEE_UnlockMutex(stdio_mutex);
#endif

	return sz;
}

static int _stdio_fputc(void *ctx, int c)
{
	struct file_context *fctx = (struct file_context*)ctx;

	buffered_put(fctx->buffer, fctx->fd, (char)c);
	return INT_MAX;
}

static int _stdio_fputs(void *ctx, const char *s)
{
	struct file_context *fctx = (struct file_context*)ctx;

	return buffered_write(fctx, s, strlen(s));
}

static int _stdio_fgetc(void *ctx)
{
	return (unsigned char)0xff;
}

static int _output_func(const char *str, size_t len, void *state)
{
	struct file_context *ctx = (struct file_context*)state;

	return buffered_write(ctx, str, strnlen(str, len));
}

static int _stdio_vfprintf(void *ctx, const char *fmt, va_list ap)
{
	return _printf_engine(_output_func, ctx, fmt, ap);
}

struct file_buffer stdout_buffer = {.pos = 0};
struct file_buffer stderr_buffer = {.pos = 0};
struct file_context fctx[3] = {
	{.fd = 0, .buffer = NULL},
	{.fd = 1, .buffer = &stdout_buffer },
	{.fd = 2, .buffer = &stderr_buffer }
};

#define DEFINE_STDIO_DESC(fctx)					\
	{							\
		.ctx		= (void *)(fctx),		\
		.fputc		= _stdio_fputc,			\
		.fputs		= _stdio_fputs,			\
		.fgetc		= _stdio_fgetc,			\
		.vfprintf	= _stdio_vfprintf,		\
	}

FILE __stdio_FILEs[3] = {
	DEFINE_STDIO_DESC(&fctx[0]), /* stdin */
	DEFINE_STDIO_DESC(&fctx[1]), /* stdout */
	DEFINE_STDIO_DESC(&fctx[2]), /* stderr */
};
#undef DEFINE_STDIO_DESC


