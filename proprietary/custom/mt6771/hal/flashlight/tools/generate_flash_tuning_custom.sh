#!/bin/sh

########################################################
# Default config
########################################################
DUTY_CURRENT="dutyCurrent.xml"
TUNING_CUSTOM="tuningCustom.xml"
CALI_AE="caliAe.xml"

########################################################
# Options
########################################################
function usage()
{
	echo "Usage: generate_flash_tuning_custom.sh"
	echo "       -s [SENSOR_TYPE]"
	echo "       -p [PART]"
	echo "       -d [DUTY_CURRENT]"
	echo "       -t [TUNING_CUSTOM]"
	echo "       -c [CALI_AE]"
	echo "Ex: generate_flash_tuning_custom.sh -s main -p 1"
	echo "Ex: generate_flash_tuning_custom.sh -s main:sub -p 1:2"
	echo "Ex: generate_flash_tuning_custom.sh -s main:sub -p 1:2 -d d.xml -t t.xml -c c.xml"
	echo "Default xml: ${DUTY_CURRENT}, ${TUNING_CUSTOM}, ${CALI_AE}."
	echo ""
}

function get_opt()
{
	while getopts "hs:p:d:t:c:" OPTION
	do
		case $OPTION in
			h)
				usage
				exit 0
				;;
			s)
				TYPE="${OPTARG}"
				;;
			p)
				PART="${OPTARG}"
				;;
			d)
				DUTY_CURRENT="${OPTARG}"
				;;
			t)
				TUNING_CUSTOM="${OPTARG}"
				;;
			c)
				CALI_AE="${OPTARG}"
				;;
			?)
				usage
				exit 0
				;;
		esac
	done
}

function verify_opt()
{
	if [ -z "${TYPE}" ] || [ -z "${PART}" ]; then
		usage
		exit 0
	fi
}

function process_opt()
{
	TYPE1=`echo ${TYPE} | awk -F ":" '{print $1}' | tr '[:upper:]' '[:lower:]'`
	TYPE2=`echo ${TYPE} | awk -F ":" '{print $2}' | tr '[:upper:]' '[:lower:]'`
	TYPE3=`echo ${TYPE} | awk -F ":" '{print $3}' | tr '[:upper:]' '[:lower:]'`
	TYPE4=`echo ${TYPE} | awk -F ":" '{print $4}' | tr '[:upper:]' '[:lower:]'`
	PART1=`echo ${PART} | awk -F ":" '{print $1}'`
	PART2=`echo ${PART} | awk -F ":" '{print $2}'`
	PART1="part${PART1}"
	if [ -n "${PART2}" ]; then
		PART2="part${PART2}"
	fi

	TYPE=`echo ${TYPE} | sed 's/:/ /g'`
	PART=`echo ${PART} | sed 's/:/ /g'`
}

function show_opt()
{
	echo "Target:"
	echo "  ${TYPE1} ${TYPE2} ${TYPE3} ${TYPE4}"
	echo "  ${PART1} ${PART2}"
	echo "Duty current: ${DUTY_CURRENT}"
	echo "Tuning custom: ${TUNING_CUSTOM}"
	echo "Cali ae: ${CALI_AE}"
	echo ""
}

get_opt $@
verify_opt
process_opt
show_opt

########################################################
# XML schema
########################################################
DUTY_CURRENT_KEYS="
dutyI_1
dutyI_2
dutyI_3
dutyI_4
dutyI_5
dutyI_6
dutyI_7
dutyI_8
dutyI_9
dutyI_10
dutyI_11
dutyI_12
dutyI_13
dutyI_14
dutyI_15
dutyI_16
dutyI_17
dutyI_18
dutyI_19
dutyI_20
dutyI_21
dutyI_22
dutyI_23
dutyI_24
dutyI_25
dutyI_26
dutyI_27
dutyI_28
dutyI_29
dutyI_30
dutyI_31
dutyI_32
dutyI_33
dutyI_34
dutyI_35
dutyI_36
dutyI_37
dutyI_38
dutyI_39
dutyI_40
"

TUNING_CUSTOM_KEYS="
maskI
torchDutyHt
torchDutyLt
dutyNumHt
dutyNumLt
coolTimeOutParaTabId_0
coolTimeOutParaTabId_1
coolTimeOutParaTabId_2
coolTimeOutParaTabId_3
coolTimeOutParaTabId_4
coolTimeOutParaCoolingTM_0
coolTimeOutParaCoolingTM_1
coolTimeOutParaCoolingTM_2
coolTimeOutParaCoolingTM_3
coolTimeOutParaCoolingTM_4
coolTimeOutParaTimOutMs_0
coolTimeOutParaTimOutMs_1
coolTimeOutParaTimOutMs_2
coolTimeOutParaTimOutMs_3
coolTimeOutParaTimOutMs_4
coolTimeOutParaLtTabId_0
coolTimeOutParaLtTabId_1
coolTimeOutParaLtTabId_2
coolTimeOutParaLtTabId_3
coolTimeOutParaLtTabId_4
coolTimeOutParaLtCoolingTM_0
coolTimeOutParaLtCoolingTM_1
coolTimeOutParaLtCoolingTM_2
coolTimeOutParaLtCoolingTM_3
coolTimeOutParaLtCoolingTM_4
coolTimeOutParaLtTimOutMs_0
coolTimeOutParaLtTimOutMs_1
coolTimeOutParaLtTimOutMs_2
coolTimeOutParaLtTimOutMs_3
coolTimeOutParaLtTimOutMs_4
maxCapExpTimeUs
"

CALI_AE_KEYS="
quickCaliExp
quickCaliAfe
quickCaliIsp
"

function parse_duty_current()
{
	local type=$1
	local part=$2
	local file=$3
	local duty_current=$4
	local color_temps="HT LT"
	local t
	local p

	xmllint --xpath "//dutyCurrent[sensorDev=\"${type}\" and part=\"${part}\"]" ${duty_current} &> /dev/null
	if [ $? -eq '0' ]; then
		echo "Parsing: ${duty_current} ${type} part${part}"
		t=${type}
		p=${part}
	else
		echo "Parsing: ${duty_current} default"
		t="default"
		p=""
	fi

	for ct in ${color_temps}
	do
		local duty_i=""

		for key in ${DUTY_CURRENT_KEYS}
		do
			val=`xmllint --xpath "//dutyCurrent[sensorDev=\"${t}\" and part=\"${p}\" and colorTemperature=\"${ct}\"]/${key}/text()" ${duty_current} 2> /dev/null`
			if [ -z "${val}" ]; then
				continue
			fi
			#echo "  ${key}: ${val}"
			duty_i="${duty_i}, ${val}"
		done
		duty_i=`echo ${duty_i} | sed 's/^..//'`
		sed -i "s/FIXME_duty${ct}I/${duty_i}/g" ${file}
	done
}

function parse_tuning_custom()
{
	local type=$1
	local part=$2
	local file=$3
	local tuning_custom=$4
	local t
	local p

	xmllint --xpath "//tuningCustom[sensorDev=\"${type}\" and part=\"${part}\"]" ${tuning_custom} &> /dev/null
	if [ $? -eq '0' ]; then
		echo "Parsing: ${tuning_custom} ${type} part${part}"
		t=${type}
		p=${part}
	else
		echo "Parsing: ${tuning_custom} default"
		t="default"
		p=""
	fi

	for key in ${TUNING_CUSTOM_KEYS}
	do
		val=`xmllint --xpath "//tuningCustom[sensorDev=\"${t}\" and part=\"${p}\"]/${key}/text()" ${tuning_custom}`
		#echo "  ${key}: ${val}"
		sed -i "s/FIXME_${key}/${val}/g" ${file}
	done
}

function parse_cali_ae()
{
	local type=$1
	local part=$2
	local file=$3
	local cali_ae=$4
	local t
	local p

	xmllint --xpath "//caliAe[sensorDev=\"${type}\" and part=\"${part}\"]" ${cali_ae} &> /dev/null
	if [ $? -eq '0' ]; then
		echo "Parsing: ${cali_ae} ${type} part${part}"
		t=${type}
		p=${part}
	else
		echo "Parsing: ${cali_at} default"
		t="default"
		p=""
	fi

	for key in ${CALI_AE_KEYS}
	do
		val=`xmllint --xpath "//caliAe[sensorDev=\"${t}\" and part=\"${p}\"]/${key}/text()" ${cali_ae}`
		#echo "  ${key}: ${val}"
		sed -i "s/FIXME_${key}/${val}/g" ${file}
	done
}


########################################################
# Main function
########################################################
TEMPLATE="flash_tuning_custom_template.cpp"

for t in ${TYPE}
do
	for p in ${PART}
	do
		file="`echo ${TEMPLATE} | sed "s/_template/_${t}_part${p}/g" | sed 's/_part1//g'`"
		echo "Create file: ${file}"
		cp -a ${TEMPLATE} ${file}
		sed -i "s/_template/_${t}_part${p}/g" ${file}
		sed -i 's/_part1//g' ${file}

		# verify xml
		echo "Verify XML: ${DUTY_CURRENT} ${TUNING_CUSTOM} ${CALI_AE}"
		xmllint --noout ${DUTY_CURRENT} ${TUNING_CUSTOM} ${CALI_AE}
		if [ $? -ne '0' ]; then
			echo "Error: invalid XML."
			continue
		fi

		# parse xml
		parse_tuning_custom ${t} ${p} ${file} ${TUNING_CUSTOM}
		parse_duty_current ${t} ${p} ${file} ${DUTY_CURRENT}
		parse_cali_ae ${t} ${p} ${file} ${CALI_AE}
	done
done

