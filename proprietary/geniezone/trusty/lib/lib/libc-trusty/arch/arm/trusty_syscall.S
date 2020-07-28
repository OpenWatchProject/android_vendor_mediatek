/*
 * Copyright (c) 2013 Google Inc. All rights reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* This file is auto-generated. !!! DO NOT EDIT !!! */

#include <asm.h>
#include <trusty_syscalls.h>

FUNCTION(write)
    ldr     r12, =__NR_write
    swi     #0
    bx      lr

FUNCTION(brk)
    ldr     r12, =__NR_brk
    swi     #0
    bx      lr

FUNCTION(exit_group)
    ldr     r12, =__NR_exit_group
    swi     #0
    bx      lr

FUNCTION(read)
    ldr     r12, =__NR_read
    swi     #0
    bx      lr

FUNCTION(ioctl)
    ldr     r12, =__NR_ioctl
    swi     #0
    bx      lr

FUNCTION(nanosleep)
    ldr     r12, =__NR_nanosleep
    swi     #0
    bx      lr

FUNCTION(gettime)
    ldr     r12, =__NR_gettime
    swi     #0
    bx      lr

FUNCTION(mmap)
    ldr     r12, =__NR_mmap
    swi     #0
    bx      lr

FUNCTION(munmap)
    ldr     r12, =__NR_munmap
    swi     #0
    bx      lr

FUNCTION(prepare_dma)
    ldr     r12, =__NR_prepare_dma
    swi     #0
    bx      lr

FUNCTION(finish_dma)
    ldr     r12, =__NR_finish_dma
    swi     #0
    bx      lr

FUNCTION(upanic)
    ldr     r12, =__NR_upanic
    swi     #0
    bx      lr

FUNCTION(port_create)
    ldr     r12, =__NR_port_create
    swi     #0
    bx      lr

FUNCTION(connect)
    ldr     r12, =__NR_connect
    swi     #0
    bx      lr

FUNCTION(accept)
    ldr     r12, =__NR_accept
    swi     #0
    bx      lr

FUNCTION(close)
    ldr     r12, =__NR_close
    swi     #0
    bx      lr

FUNCTION(set_cookie)
    ldr     r12, =__NR_set_cookie
    swi     #0
    bx      lr

FUNCTION(wait)
    ldr     r12, =__NR_wait
    swi     #0
    bx      lr

FUNCTION(wait_any)
    ldr     r12, =__NR_wait_any
    swi     #0
    bx      lr

FUNCTION(get_msg)
    ldr     r12, =__NR_get_msg
    swi     #0
    bx      lr

FUNCTION(read_msg)
    ldr     r12, =__NR_read_msg
    swi     #0
    bx      lr

FUNCTION(put_msg)
    ldr     r12, =__NR_put_msg
    swi     #0
    bx      lr

FUNCTION(send_msg)
    ldr     r12, =__NR_send_msg
    swi     #0
    bx      lr

FUNCTION(request_irq)
    ldr     r12, =__NR_request_irq
    swi     #0
    bx      lr

FUNCTION(release_irq)
    ldr     r12, =__NR_release_irq
    swi     #0
    bx      lr

FUNCTION(wait_irq)
    ldr     r12, =__NR_wait_irq
    swi     #0
    bx      lr

FUNCTION(user_thread_create)
    ldr     r12, =__NR_user_thread_create
    swi     #0
    bx      lr

FUNCTION(user_thread_join)
    ldr     r12, =__NR_user_thread_join
    swi     #0
    bx      lr

FUNCTION(user_thread_resume)
    ldr     r12, =__NR_user_thread_resume
    swi     #0
    bx      lr

FUNCTION(user_thread_get_handle)
    ldr     r12, =__NR_user_thread_get_handle
    swi     #0
    bx      lr

