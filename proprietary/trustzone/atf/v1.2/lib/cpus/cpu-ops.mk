#
# Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
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

# Cortex A57 specific optimisation to skip L1 cache flush when
# cluster is powered down.
SKIP_A57_L1_FLUSH_PWR_DWN	?=0

WORKAROUND_CVE_2018_3639	?=0
# Process SKIP_A57_L1_FLUSH_PWR_DWN flag
$(eval $(call assert_boolean,SKIP_A57_L1_FLUSH_PWR_DWN))
$(eval $(call add_define,SKIP_A57_L1_FLUSH_PWR_DWN))

# Process WORKAROUND_CVE_2018_3639 flag
$(eval $(call assert_boolean,WORKAROUND_CVE_2018_3639))
$(eval $(call add_define,WORKAROUND_CVE_2018_3639))

# CPU Errata Build flags. These should be enabled by the
# platform if the errata needs to be applied.

# Flag to apply errata 855024 during reset. This errata applies only to
# revision <= r0p0 of the Cortex A35 cpu.
ERRATA_A35_855024	?=0

# Flag to apply errata 855472 during reset. This errata applies only to
# revision <= r0p0 of the Cortex A35 cpu.
ERRATA_A35_855472	?=0

# Flag to apply errata 855873 during reset. This errata applies only to
# revision >= r0p0 of the Cortex A53 cpu.
ERRATA_A53_855873	?=0

# Flag to apply errata 826319 during reset. This errata applies only to
# revision <= r0p2 of the Cortex A53 cpu.
ERRATA_A53_826319	?=0

# Flag to apply errata 836870 during reset. This errata applies only to
# revision <= r0p3 of the Cortex A53 cpu. From r0p4 and onwards, this
# errata is enabled by default.
ERRATA_A53_836870	?=0

# Flag to apply errata 806969 during reset. This errata applies only to
# revision r0p0 of the Cortex A57 cpu.
ERRATA_A57_806969	?=0

# Flag to apply errata 813420 during reset. This errata applies only to
# revision r0p0 of the Cortex A57 cpu.
ERRATA_A57_813420	?=0

# Flag to apply errata 854221 during reset. This errata applies only to
# revision <= r0p1 of the ARTEMIS cpu.
ERRATA_ARTEMIS_854221	?=0

# Flag to apply errata 855423 during reset. This errata applies only to
# revision <= r0p1 of the ARTEMIS cpu.
ERRATA_ARTEMIS_855423	?=0

# Process ERRATA_A35_855024 flag
$(eval $(call assert_boolean,ERRATA_A35_855024))
$(eval $(call add_define,ERRATA_A35_855024))

# Process ERRATA_A35_855472 flag
$(eval $(call assert_boolean,ERRATA_A35_855472))
$(eval $(call add_define,ERRATA_A35_855472))

# Process ERRATA_A53_855873 flag
$(eval $(call assert_boolean,ERRATA_A53_855873))
$(eval $(call add_define,ERRATA_A53_855873))

# Process ERRATA_A53_826319 flag
$(eval $(call assert_boolean,ERRATA_A53_826319))
$(eval $(call add_define,ERRATA_A53_826319))

# Process ERRATA_A53_836870 flag
$(eval $(call assert_boolean,ERRATA_A53_836870))
$(eval $(call add_define,ERRATA_A53_836870))

# Process ERRATA_A57_806969 flag
$(eval $(call assert_boolean,ERRATA_A57_806969))
$(eval $(call add_define,ERRATA_A57_806969))

# Process ERRATA_A57_813420 flag
$(eval $(call assert_boolean,ERRATA_A57_813420))
$(eval $(call add_define,ERRATA_A57_813420))

# Process ERRATA_ARTEMIS_854221 flag
$(eval $(call assert_boolean,ERRATA_ARTEMIS_854221))
$(eval $(call add_define,ERRATA_ARTEMIS_854221))

# Process ERRATA_ARTEMIS_855423 flag
$(eval $(call assert_boolean,ERRATA_ARTEMIS_855423))
$(eval $(call add_define,ERRATA_ARTEMIS_855423))
