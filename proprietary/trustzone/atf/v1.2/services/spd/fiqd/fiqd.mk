#
# Copyright (c) 2013-2014, ARM Limited and Contributors. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.
#
# Neither the name of ARM nor the names of its contributors may be used
# to endorse or promote products derived from this software without specific
# prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#

TSPD_DIR		:=	services/spd/fiqd
SPD_INCLUDES		:=	-Iinclude/bl32/tsp

SPD_SOURCES		:=	services/spd/fiqd/fiqd_main.c \
				services/spd/fiqd/fiqd_common.c

# Flag used to enable routing of non-secure interrupts to EL3 when they are
# generated while the code is executing in S-EL1/0.
TSP_NS_INTR_ASYNC_PREEMPT	:=	0

# If TSPD_ROUTE_IRQ_TO_EL3 build flag is defined, use it to define value for
# TSP_NS_INTR_ASYNC_PREEMPT for backward compatibility.
ifdef TSPD_ROUTE_IRQ_TO_EL3
ifeq (${ERROR_DEPRECATED},1)
$(error "TSPD_ROUTE_IRQ_TO_EL3 is deprecated. Please use the new build flag TSP_NS_INTR_ASYNC_PREEMPT")
endif
$(warning "TSPD_ROUTE_IRQ_TO_EL3 is deprecated. Please use the new build flag TSP_NS_INTR_ASYNC_PREEMPT")
TSP_NS_INTR_ASYNC_PREEMPT	:= ${TSPD_ROUTE_IRQ_TO_EL3}
endif

$(eval $(call assert_boolean,TSP_NS_INTR_ASYNC_PREEMPT))
$(eval $(call add_define,TSP_NS_INTR_ASYNC_PREEMPT))
