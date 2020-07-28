/*
 * Copyright (c) 2015, Google Inc. All rights reserved
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

#include <asm.h>

.altmacro

/* only checks low 64 bits for now */
.macro init_regs vall=r0, valh=r1, reg=0
    vmov.32    d\reg, \vall, \valh
.if \reg < 31
    add     \vall, \vall, #1
    add     \valh, \valh, #1
    init_regs \vall, \valh, %(\reg+1)
.endif
.endm

FUNCTION(fptest_arch_init)
    init_regs
    bx      lr

.macro check_regs errors, vall, valh, tmpl, tmph, reg=0
    vmov    \tmpl, \tmph, d\reg
    cmp     \vall, \tmpl
    cmpeq   \valh, \tmph
    addne   \errors, \errors, #1
.if \reg < 31
    add     \vall, \vall, #1
    add     \valh, \valh, #1
    check_regs \errors, \vall, \valh, \tmpl, \tmph, %(\reg+1)
.endif
.endm

FUNCTION(fptest_arch_check_state)
    mov     r2, r1
    mov     r1, r0
    mov     r0, #0
    check_regs r0, r1, r2, r3, ip
    bx      lr
