#!/bin/bash

usage() {
	cat <<-EOF
This script builds all TinySys ADSP default images; that is, all projects
named {PLATFORM}-default-project under each project/HIFI3_* directory. It
must be run in the root directory of TinySys source repo.

Usage: $(basename ${0}) [-h]

	EOF

	exit 1
}

err() {
	echo "[ERROR] $*" >&2
	exit 1
}

[ "${1}" = '-h' ] && usage

[ -d 'kernel/FreeRTOS/Source' ] || \
	err 'this script must be run in the root directory of TinySys source'

/bin/ls project/HIFI3_* &> /dev/null || exit 0

PROJECTS=$(find project/HIFI3_* -maxdepth 3 -type d -name '*-default-project*')
[ -z "${PROJECTS}" ] && exit 0

for i in ${PROJECTS}; do
	rm -rf tinysys-scp_out/freertos && PROJECT="$(basename ${i})" BUILD_WITH_XTENSA=yes make adsp -j24 || exit 1
done

exit 0
