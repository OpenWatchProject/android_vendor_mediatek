#!/bin/sh

########################################################
# Default config
########################################################
TUNING_CUSTOM_CCT="tuningCustomCct.xml"
ENG_TAB="engTab.xml"
FWB_GAIN="fwbGain.xml"
IS_PARSING_FWB_GAIN=0

########################################################
# Options
########################################################
function usage()
{
	echo "Usage: generate_flash_tuning_custom_cct.sh"
	echo "       -s [SENSOR_TYPE]"
	echo "       -p [PART]"
	echo "       -t [DUTY_CURRENT]"
	echo "       -e [TUNING_CUSTOM]"
	echo "       -f [CALI_AE]"
	echo "       -i : is parsing fwb gain"
	echo "Ex: generate_flash_tuning_custom_cct.sh -s main -p 1"
	echo "Ex: generate_flash_tuning_custom_cct.sh -s main:main2:sub:sub2 -p 1:2"
	echo "Ex: generate_flash_tuning_custom_cct.sh -s main:main2:sub:sub2 -p 1:2 -t t.xml -e e.xml -f f.xml"
	echo "Ex: generate_flash_tuning_custom_cct.sh -s main:main2:sub:sub2 -p 1:2 -t t.xml -e e.xml -f f.xml -i"
	echo "Default xml: ${TUNING_CUSTOM_CCT}, ${ENG_TAB}, ${FWB_GAIN}."
	echo "Default is parsing fwb gain: ${IS_PARSING_FWB_GAIN}"
	echo ""
}

function get_opt()
{
	while getopts "hs:p:t:e:f:i" OPTION
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
			t)
				TUNING_CUSTOM_CCT="${OPTARG}"
				;;
			e)
				ENG_TAB="${OPTARG}"
				;;
			f)
				FWB_GAIN="${OPTARG}"
				;;
			i)
				IS_PARSING_FWB_GAIN=1
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
	echo "Tuning custom cct: ${TUNING_CUSTOM_CCT}"
	echo "Eng tab: ${ENG_TAB}"
	echo "Fwb gain: ${FWB_GAIN}"
	echo "Is parsing fwb gain: ${IS_PARSING_FWB_GAIN}"
	echo ""
}

get_opt $@
verify_opt
process_opt
show_opt

########################################################
# XML schema
########################################################
TUNING_CUSTOM_CCT_KEYS="
nvramStrobeVersion
yTarget_x
fgWIncreaseLevelbySize_x
fgWIncreaseLevelbyRef_x
ambientRefAccuracyRatio_x
flashRefAccuracyRatio_x
backlightAccuracyRatio_x
backlightUnderY_x
backlightWeakRefRatio_x
safetyExp_x
maxUsableISO_x
yTargetWeight_x
lowReflectanceThreshold_x
flashReflectanceWeight_x
bgSuppressMaxDecreaseEV_x
bgSuppressMaxOverExpRatio_x
fgEnhanceMaxIncreaseEV_x
fgEnhanceMaxOverExpRatio_x
isFollowCapPline_x
histStretchMaxFgYTarget_x
histStretchBrightestYTarget_x
fgSizeShiftRatio_x
backlitPreflashTriggerLV_x
backlitMinYTarget_x
minstameanpass_x
isFollowCapPline_0
paraIdxForceOn_00
paraIdxForceOn_01
paraIdxForceOn_02
paraIdxForceOn_03
paraIdxForceOn_04
paraIdxForceOn_05
paraIdxForceOn_06
paraIdxForceOn_07
paraIdxForceOn_08
paraIdxForceOn_09
paraIdxForceOn_10
paraIdxForceOn_11
paraIdxForceOn_12
paraIdxForceOn_13
paraIdxForceOn_14
paraIdxForceOn_15
paraIdxForceOn_16
paraIdxForceOn_17
paraIdxForceOn_18
paraIdxAuto_00
paraIdxAuto_01
paraIdxAuto_02
paraIdxAuto_03
paraIdxAuto_04
paraIdxAuto_05
paraIdxAuto_06
paraIdxAuto_07
paraIdxAuto_08
paraIdxAuto_09
paraIdxAuto_10
paraIdxAuto_11
paraIdxAuto_12
paraIdxAuto_13
paraIdxAuto_14
paraIdxAuto_15
paraIdxAuto_16
paraIdxAuto_17
paraIdxAuto_18
engLevelTorchDuty
engLevelAfDuty
engLevelPfDutyN
engLevelMfDutyMaxN
engLevelMfDutyMinN
engLeveliChangeByVBatEn
engLevelVBatL
engLevelPfDutyL
engLevelMfDutyMaxL
engLevelMfDutyMinL
engLeveliChangeByBurstEn
engLevelPfDutyB
engLevelMfDutyMaxB
engLevelMfDutyMinB
engLevelDecSysIAtHighEn
engLevelDutyH
engLevelLTTorchDuty
engLevelLTAfDuty
engLevelLTPfDutyN
engLevelLTMfDutyMaxN
engLevelLTMfDutyMinN
engLevelLTPfDutyL
engLevelLTMfDutyMaxL
engLevelLTMfDutyMinL
engLevelLTPfDutyB
engLevelLTMfDutyMaxB
engLevelLTMfDutyMinB
toleranceEV_pos
toleranceEV_neg
xyWeighting
useAwbPreferenceGain
envOffsetIndex_0
envOffsetIndex_1
envOffsetIndex_2
envOffsetIndex_3
envXrOffsetValue_0
envXrOffsetValue_1
envXrOffsetValue_2
envXrOffsetValue_3
envYrOffsetValue_0
envYrOffsetValue_1
envYrOffsetValue_2
envYrOffsetValue_3
varianceTolerance
chooseColdOrWarm
"

ENG_TAB_KEYS="
dutyHt_1
dutyHt_2
dutyHt_3
dutyHt_4
dutyHt_5
dutyHt_6
dutyHt_7
dutyHt_8
dutyHt_9
dutyHt_10
dutyHt_11
dutyHt_12
dutyHt_13
dutyHt_14
dutyHt_15
dutyHt_16
dutyHt_17
dutyHt_18
dutyHt_19
dutyHt_20
dutyHt_21
dutyHt_22
dutyHt_23
dutyHt_24
dutyHt_25
dutyHt_26
dutyHt_27
dutyHt_28
dutyHt_29
dutyHt_30
dutyHt_31
dutyHt_32
dutyHt_33
dutyHt_34
dutyHt_35
dutyHt_36
dutyHt_37
dutyHt_38
dutyHt_39
dutyHt_40
"

FWB_GAIN_KEYS="
rGain
gGain
bGain
"

function parse_tuning_custom_cct()
{
	local type=$1
	local part=$2
	local file=$3
	local tuning_custom_cct=$4
	local t
	local p

	xmllint --xpath "//tuningCustomCct[sensorDev=\"${type}\" and part=\"${part}\"]" ${tuning_custom_cct} &> /dev/null
	if [ $? -eq '0' ]; then
		echo "Parsing: ${tuning_custom_cct} ${type} part${part}"
		t=${type}
		p=${part}
	else
		echo "Parsing: ${tuning_custom_cct} default"
		t="default"
		p=""
	fi

	for key in ${TUNING_CUSTOM_CCT_KEYS}
	do
		val=`xmllint --xpath "//tuningCustomCct[sensorDev=\"${t}\" and part=\"${p}\"]/${key}/text()" ${tuning_custom_cct}`
		#echo "  ${key}: ${val}"
		sed -i "s/FIXME_${key}/${val}/g" ${file}
	done

}

function parse_eng_tab()
{
	local type=$1
	local part=$2
	local file=$3
	local eng_tab=$4
	local t
	local p

	xmllint --xpath "//engTab[sensorDev=\"${type}\" and part=\"${part}\"]" ${eng_tab} &> /dev/null
	if [ $? -eq '0' ]; then
		echo -n "Parsing: ${eng_tab} ${type} part${part} "
		t=${type}
		p=${part}
	else
		echo -n "Parsing: ${eng_tab} default "
		t="default"
		p=""
	fi

	local engs=""
	local duty_lts=`xmllint --xpath "count(//engTab[sensorDev=\"${t}\" and part=\"${p}\"])" ${eng_tab}`
	for duty_lt in `seq 1 1 ${duty_lts}`
	do
		local eng=""

		for key in ${ENG_TAB_KEYS}
		do
			val=`xmllint --xpath "//engTab[sensorDev=\"${t}\" and part=\"${p}\" and dutyLt=\"${duty_lt}\"]/${key}/text()" ${eng_tab} 2> /dev/null`
			if [ -z "${val}" ]; then
				continue
			fi
			#echo "  ${key}: ${val}"
			eng="${eng}${val}, "
		done

		if [ -z "${eng}" ]; then
			continue
		fi
		eng=`echo ${eng} | sed 's/ $//'`
		#echo ${eng}
		engs="${engs}${eng}\n    "
		#echo -e "${engs}"
		echo -n "."
	done
	echo ""
	engs=`echo "${engs}" | sed 's/.......$//'`
	#echo -e "${engs}"

	sed -i "s/FIXME_engTab/${engs}/" ${file}
}

function parse_fwb_gain()
{
	local type=$1
	local part=$2
	local file=$3
	local fwb_gain=$4
	local t
	local p

	# if not parsing fwb gain
	if [ "${IS_PARSING_FWB_GAIN}" -ne 1 ]; then
		sed -i "s/FIXME_flashWBGain//" ${file}
		return
	fi

	xmllint --xpath "//fwbGain[sensorDev=\"${type}\" and part=\"${part}\"]" ${fwb_gain} &> /dev/null
	if [ $? -eq '0' ]; then
		echo -n "Parsing: ${fwb_gain} ${type} part${part} "
		t=${type}
		p=${part}
	else
		echo -n "Parsing: ${fwb_gain} default "
		t="default"
		p=""
	fi

	local gains=""
	local duty_hts=`xmllint --xpath "count(//fwbGain[sensorDev=\"${t}\" and part=\"${p}\" and dutyLt=\"1\"])" ${fwb_gain}`
	local duty_lts=`xmllint --xpath "count(//fwbGain[sensorDev=\"${t}\" and part=\"${p}\" and dutyHt=\"1\"])" ${fwb_gain}`
	for duty_lt in `seq 1 1 ${duty_lts}`
	do
		for duty_ht in `seq 1 1 ${duty_hts}`
		do
			local gain=""

			for key in ${FWB_GAIN_KEYS}
			do
				val=`xmllint --xpath "//fwbGain[sensorDev=\"${t}\" and part=\"${p}\" and dutyHt=\"${duty_ht}\" and dutyLt=\"${duty_lt}\"]/${key}/text()" ${fwb_gain} 2> /dev/null`
				if [ -z "${val}" ]; then
					continue
				fi
				#echo "  ${key}: ${val}"
				gain="${gain}${val}, "
			done

			if [ -z "${gain}" ]; then
				continue
			fi
			gain=`echo ${gain} | sed 's/.$//'`
			gain="{${gain}},"
			#echo ${gain}
			gains="${gains}${gain}\n    "
			#echo -e "${gains}"
		done
		echo -n "."
	done
	echo ""
	gains=`echo "${gains}" | sed 's/.......$//'`
	#echo -e "${gains}"

	sed -i "s/FIXME_flashWBGain/${gains}/" ${file}
}


########################################################
# Main function
########################################################
TEMPLATE="flash_tuning_custom_cct_template.cpp"

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
		echo "Verify XML: ${TUNING_CUSTOM_CCT} ${ENG_TAB} ${FWB_GAIN}"
		xmllint --noout ${TUNING_CUSTOM_CCT} ${ENG_TAB} ${FWB_GAIN}
		if [ $? -ne '0' ]; then
			echo "Error: invalid XML."
			continue
		fi

		# parse xml
		parse_tuning_custom_cct ${t} ${p} ${file} ${TUNING_CUSTOM_CCT}
		parse_eng_tab ${t} ${p} ${file} ${ENG_TAB}
		parse_fwb_gain ${t} ${p} ${file} ${FWB_GAIN}
	done
done

