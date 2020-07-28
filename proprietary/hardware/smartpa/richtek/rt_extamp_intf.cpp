#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#ifdef ANDROID_BUILD
#include <cutils/log.h>
#endif
#ifdef USE_TINYALSA
#include <tinyalsa/asoundlib.h>
#endif
#include <tinyxml.h>
#include <list>
#include "rt_extamp_intf.h"

#ifdef __MT6660
#include "Custom_NvRam_LID.h"
#include "libnvram.h"
#include "CFG_SMARTPA_CALIB_File.h"
#include "CFG_SMARTPA_CALIB_Default.h"
#include "libfile_op.h"
#endif /* __MT6660 */

using namespace std;
#define DEVICE_CONFIG_FILE "/system/vendor/etc/smartpa_param/rt_device.xml"

#ifdef MTK_SCENARIO
enum {
	RT_CASE_NORMAL,
	RT_CASE_RINGTONE,
	RT_CASE_PHONECALL,
	RT_CASE_VIOP,
	RT_CASE_MAX,
};

enum {
	RT_DEV_SPK,
	RT_DEV_RECV,
	RT_DEV_MAX,
};

static const char * const scenario[RT_DEV_MAX][RT_CASE_MAX] = {
	{ "speaker_on_normal", "speaker_on_ringtone", "speaker_on_phonecall", "speaker_on_voip"},
	{ NULL, NULL, "receiver_on_phonecall", "receiver_on_voip"},
};
#endif

#ifdef ANDROID_BUILD
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "rt_extamp"

#define RT_LOGD(format, args...) \
do {\
	printf(format, ##args);\
	ALOGD(format, ##args);\
} while (0);

#define RT_LOGE(format, args...) \
do {\
	printf(format, ##args);\
	ALOGE(format, ##args);\
} while (0);
#else
#define RT_LOGD(format, args...) \
do {\
	printf(format, ##args);\
} while (0);

#define RT_LOGE(format, args...) \
do {\
	printf(format, ##args);\
} while (0);
#endif

struct ctl_item {
	const char *name;
	const char *strval;
};

class conf_item {
public:
	const char *path;
	const char *file;
	const char *check;
	list<struct ctl_item *> ctl_list;
	~conf_item() {
		struct ctl_item *item;
		while (!ctl_list.empty()) {
			item = ctl_list.front();
			ctl_list.pop_front();
			free(item);
		}
	}
};

class scenario_item {
public:
	const char *name;
	list<struct ctl_item *> ctl_list;
	conf_item *scene_conf;
	~scenario_item() {
		struct ctl_item *item;
		while (!ctl_list.empty()) {
			item = ctl_list.front();
			ctl_list.pop_front();
			free(item);
		}
		if (scene_conf)
			delete scene_conf;
	}
};

class amp_control_inst {
private:
	int apply_amp_ctl(list<struct ctl_item *> &);
	int apply_amp_param(const conf_item *);
	int amp_ctl_check(const conf_item *);
	int show_ctl_list(list<struct ctl_item *> &);
public:
#ifdef USE_TINYALSA
	struct mixer *mixer;
#endif
	list<conf_item *> conf_list;
	list<scenario_item *> scenario_list;

	amp_control_inst(struct mixer *audio_mixer) {
#ifdef USE_TINYALSA
		mixer = audio_mixer;
#endif
	}
	~amp_control_inst() {
		conf_item *conf_item;
		scenario_item *scenario_item;

		while (!conf_list.empty()) {
			conf_item = conf_list.front();
			conf_list.pop_front();
			delete conf_item;
		}
		while (!scenario_list.empty()) {
			scenario_item = scenario_list.front();
			scenario_list.pop_front();
			delete scenario_item;
		}
	}
	int apply_amp_conf();
	int apply_amp_scenario(const char *name);
	int show_all_item();
};

static amp_control_inst *amp_ctl_inst = NULL;

int amp_control_inst::apply_amp_ctl(list<struct ctl_item *> &ctl_list)
{
	list<struct ctl_item *>::iterator it;
#ifdef USE_TINYALSA
	struct mixer_ctl *mixer_ctl;
	int i, ret = 0;
	const char *tmp;
#endif

	if (ctl_list.empty())
		return 0;
	for (it = ctl_list.begin(); it != ctl_list.end(); ++it) {
#ifdef USE_TINYALSA
		mixer_ctl = mixer_get_ctl_by_name(mixer, (*it)->name);
		if (!mixer_ctl) {
			RT_LOGE("%s: %s not invalid ctl name\n", __func__, (*it)->name);
			continue;
		}
		switch (mixer_ctl_get_type(mixer_ctl)) {
		case MIXER_CTL_TYPE_BOOL:
		case MIXER_CTL_TYPE_INT:
		case MIXER_CTL_TYPE_BYTE:
			for (i = 0; i < (int)mixer_ctl_get_num_values(mixer_ctl); i++) {
				ret = mixer_ctl_set_value(mixer_ctl, i, atoi((*it)->strval));
				if (ret < 0)
					RT_LOGE("%s: %s set value fail\n", __func__, (*it)->name);
			}
			break;
		case MIXER_CTL_TYPE_ENUM:
			tmp = mixer_ctl_get_enum_string(mixer_ctl, atoi((*it)->strval));
			if (!tmp) {
				RT_LOGE("%s %s: not valid value\n", __func__, (*it)->name);
				continue;
			}
			printf("%s\n", tmp);
			ret = mixer_ctl_set_enum_by_string(mixer_ctl, tmp);
			if (ret < 0)
				RT_LOGE("%s: %s set enum fail\n", __func__, (*it)->name);
			break;
		default:
			RT_LOGE("%s: %s not supported type\n", __func__, (*it)->name);
			break;
		}
#endif
	}
	return 0;
}

int amp_control_inst::apply_amp_param(const conf_item *p_item)
{
	char data[4096] = {0};
	int size;
	int fd;

	RT_LOGD("%s: begin\n", __func__);
	if (!p_item)
		return -EINVAL;
	RT_LOGD("%s: item_path %s\n", __func__, p_item->path);
	fd = open(p_item->file, O_RDONLY);
	if (fd < 0) {
		RT_LOGE("%s: %s cannot open file\n", __func__, p_item->file);
		goto out_amp_param;
	}
	size = read(fd, data, 4096);
	close(fd);
	if (size < 0)
		goto out_amp_param;
	fd = open(p_item->path, O_WRONLY);
	if (fd < 0) {
		RT_LOGE("%s: %s cannot open path\n", __func__, p_item->path);
		goto out_amp_param;
	}
	if (write(fd, data, size) < 0)
		RT_LOGE("%s: %s cannot write path\n", __func__, p_item->path);
	close(fd);
	return 0;
out_amp_param:
	return -EBADF;
}

int amp_control_inst::amp_ctl_check(const conf_item *p_item)
{
	char data[4096] = {0};
	int size;
	int val, fd;

	RT_LOGD("%s: begin\n", __func__);
	if (!p_item)
		return -EINVAL;
	fd = open(p_item->check, O_RDONLY);
	if (fd < 0) {
		RT_LOGE("%s: %s cannot check file\n", __func__, p_item->check);
		goto out_amp_check;
	}
	size = read(fd, data, 4096);
	close(fd);
	if (size < 0)
		goto out_amp_check;

	if (sscanf(data, "%d\n", &val) != 1)
		goto out_amp_check;

	return val;
out_amp_check:
	return -EACCES;
}

int amp_control_inst::apply_amp_conf()
{
	list<conf_item *>::iterator it;
	int ret = 0;

	if (conf_list.empty())
		return 0;
	for (it = conf_list.begin(); it != conf_list.end(); ++it) {
		ret = apply_amp_param(*it);
		if (ret < 0)
			continue;
		ret = amp_ctl_check(*it);
		if (ret == 0) {
			ret = apply_amp_ctl((*it)->ctl_list);
			if (ret < 0)
				RT_LOGE("%s: %s apply amp ctl fail\n", __func__, (*it)->path);
		} else if (ret < 0)
			RT_LOGE("%s: %s check amp control fail\n", __func__, (*it)->path);
	}
	return 0;
}

int amp_control_inst::apply_amp_scenario(const char *name)
{
	list<scenario_item *>::iterator it;
	int ret = 0;

	if (!name)
		return -EINVAL;
	if (scenario_list.empty())
		return 0;
	for (it = scenario_list.begin(); it != scenario_list.end(); ++it) {
		if (!strcmp(name, (*it)->name)) {
			if ((*it)->scene_conf) {
				ret = apply_amp_param((*it)->scene_conf);
				if (ret < 0)
					RT_LOGE("%s: %s apply amp param fail\n", __func__, (*it)->name);
			} else
				RT_LOGE("%s: no amp scene conf\n", __func__);
			ret = apply_amp_ctl((*it)->ctl_list);
			if (ret < 0)
				RT_LOGE("%s: %s apply amp control fail\n", __func__, (*it)->name);
			break;
		}
	}
	return ret;
}

int amp_control_inst::show_ctl_list(list<struct ctl_item *> &ctl_list)
{
	for (list<struct ctl_item *>::iterator it = ctl_list.begin(); it != ctl_list.end(); ++it)
		printf("%s -> %s\n", (*it)->name, (*it)->strval);
	return 0;
}

int amp_control_inst::show_all_item()
{
	/* conf_item show */
	printf("conf_list start\n");
	for (list<conf_item *>::iterator it = conf_list.begin(); it != conf_list.end(); ++it) {
		printf("check %s\n", (*it)->check);
		printf("path %s\n", (*it)->path);
		printf("file %s\n", (*it)->file);
		show_ctl_list((*it)->ctl_list);
	}
	printf("conf_list end\n");
	/* scenario_item show */
	printf("scenario_list start\n");
	for (list<scenario_item *>::iterator it = scenario_list.begin(); it != scenario_list.end(); ++it) {
		printf("name %s\n", (*it)->name);
		show_ctl_list((*it)->ctl_list);
	}
	printf("scenario_list start\n");
	return 0;
}

static struct ctl_item * get_xml_ctl_item(TiXmlNode *np)
{
	TiXmlElement *ele = np->ToElement();
	struct ctl_item *p_item;

	if (!np)
		return NULL;
	p_item = (struct ctl_item *)malloc(sizeof(*p_item));
	if (!p_item)
		return NULL;
	p_item->name = ele->Attribute("name");
	p_item->strval = ele->Attribute("val");
	if (!p_item->strval)
		goto out_ctl_param;
	return p_item;
out_ctl_param:
	free(p_item);
	return NULL;
}

static int parse_xml_ctl_element(TiXmlNode *root,
	list<struct ctl_item *> &ctl_list)
{
	TiXmlNode *pChild;
	struct ctl_item *ctl_item;

	if (!root)
		return -EINVAL;
	for (pChild = root->FirstChild("ctl"); pChild; pChild = pChild->NextSibling("ctl")) {
		ctl_item = get_xml_ctl_item(pChild);
		if (!ctl_item)
			continue;
		ctl_list.push_back(ctl_item);
	}
	return 0;
}

static conf_item * get_xml_conf_item(TiXmlNode *np)
{
	TiXmlElement *ele = np->ToElement();
	conf_item *p_item;

	if (!np)
		return NULL;
	p_item = new conf_item();
	if (!p_item)
		return NULL;
	p_item->check = ele->Attribute("check_path");
	p_item->path = ele->Attribute("param_path");
	p_item->file = ele->Attribute("param_file");
	if (!p_item->check || !p_item->path || !p_item->file) {
		printf("empty attribute\n");
		goto out_conf_param;
	}
	return p_item;
out_conf_param:
	delete(p_item);
	return NULL;
}

static int parse_xml_conf_element(TiXmlNode *root,
	list<conf_item *> &conf_list)
{
	TiXmlNode *pChild;
	conf_item *p_item;

	if (!root)
		return -EINVAL;
	for (pChild = root->FirstChild("conf"); pChild; pChild = pChild->NextSibling("conf")) {
		p_item = get_xml_conf_item(pChild);
		if (!p_item)
			continue;
		if (parse_xml_ctl_element(pChild, p_item->ctl_list) < 0)
			printf("%s: parse control fail\n", __func__);
		conf_list.push_back(p_item);
	}
	return 0;
}

static scenario_item * get_xml_scenario_item(TiXmlNode *np)
{
	TiXmlElement *ele = np->ToElement();
	scenario_item *p_item;

	if (!np)
		return NULL;
	p_item = new scenario_item();
	if (!p_item)
		return NULL;
	p_item->name = ele->Attribute("name");
	if (!p_item->name)
		goto out_scenario_param;
	return p_item;
out_scenario_param:
	delete p_item;
	return NULL;
}

static int parse_xml_scenario_element(TiXmlNode *root,
	list<scenario_item *> &scenario_list)
{
	TiXmlNode *pChild;
	scenario_item *p_item;

	if (!root)
		return -EINVAL;
	for (pChild = root->FirstChild("scenario"); pChild; pChild = pChild->NextSibling("scenario")) {
		p_item = get_xml_scenario_item(pChild);
		if (!p_item)
			continue;
		p_item->scene_conf = get_xml_conf_item(pChild->FirstChild("conf"));
		if (parse_xml_ctl_element(pChild, p_item->ctl_list) < 0)
			printf("%s: parse control fail\n", __func__);
		scenario_list.push_back(p_item);
	}
	return 0;
}

static int parse_xml_document(const char *file,
	amp_control_inst *ctl_inst)
{
	TiXmlDocument *doc;
	TiXmlNode *root;
	int ret = 0;

	if (!file || !ctl_inst)
		return -EINVAL;
	doc = new TiXmlDocument(file);
	if (!doc)
		return -EINVAL;
	if (!doc->LoadFile()) {
		ret = -EACCES;
		goto out_parse;
	}
	root = doc->FirstChild("config");
	if (!root) {
		ret = -EINVAL;
		goto out_parse;
	}
	ret = parse_xml_conf_element(root, ctl_inst->conf_list);
	if (ret < 0)
		goto out_parse;
	ret = parse_xml_scenario_element(root, ctl_inst->scenario_list);
	if (ret < 0)
		goto out_parse;
	return 0;
out_parse:
	delete doc;
	return ret;
}

static int rt_extamp_init(__unused struct SmartPa *smart_pa)
{
	amp_control_inst *ctl_inst;
	int ret = 0;

	RT_LOGD("%s: begin, mixer %p\n", __func__, smart_pa->attribute.mMixer);
	ctl_inst = new amp_control_inst(smart_pa->attribute.mMixer);
	if (!ctl_inst) {
		RT_LOGE("%s: alloc amp control instance fail\n", __func__);
		return -ENOMEM;
	}
	ret = parse_xml_document(DEVICE_CONFIG_FILE, ctl_inst);
	if (ret < 0) {
		RT_LOGE("%s: parse xml fail\n", __func__);
		goto out_xml_parse;
	}
	ret = ctl_inst->apply_amp_conf();
	if (ret < 0)
		RT_LOGE("%s: apply amp conf fail\n", __func__);
	ret = ctl_inst->apply_amp_scenario("init");
	if (ret < 0)
		RT_LOGE("%s: apply amp init scenario fail\n", __func__);
	amp_ctl_inst = ctl_inst;
	return 0;
out_xml_parse:
	delete ctl_inst;
	return ret;
}

static int rt_extamp_deinit()
{
	int ret;

	RT_LOGD("%s: begin\n", __func__);
	if (amp_ctl_inst) {
		ret = amp_ctl_inst->apply_amp_scenario("deinit");
		if (ret < 0) {
			RT_LOGE("%s: apply amp on fail\n", __func__);
		}
		delete amp_ctl_inst;
		amp_ctl_inst = NULL;
	}
	return 0;
}

static int rt_extamp_speakerOn(struct SmartPaRuntime *runtime)
{
	int ret = 0;

	RT_LOGD("%s: begin\n", __func__);
	if (amp_ctl_inst) {
#ifdef MTK_SCENARIO
		RT_LOGD("%s: device = %d, mode = %d, set %s\n",
			__func__, runtime->device, runtime->mode, scenario[runtime->device][runtime->mode]);
		if (runtime->device >= RT_DEV_MAX || runtime->mode >= RT_CASE_MAX)
			return -EINVAL;
		ret = amp_ctl_inst->apply_amp_scenario(scenario[runtime->device][runtime->mode]);
		if (ret < 0)
			RT_LOGE("%s: apply amp on fail\n", __func__);
#endif
	}
	return ret;
}

static int rt_extamp_speakerOff()
{
	int ret = 0;

	RT_LOGD("%s: begin\n", __func__);
	if (amp_ctl_inst) {
		ret = amp_ctl_inst->apply_amp_scenario("amp_off");
		if (ret < 0)
			RT_LOGE("%s: apply amp off fail\n", __func__);
	}
	return ret;
}

static int rt_extamp_speakerCalibrate(int state)
{
	int result = -1;
	bool is_rt5509 = false;
#ifdef __MT6660
	int num, size;
	char calib_str[] = "calib_value: ";
#endif /* __MT6660 */
	static int volume = 231;
	char newline[128], calib_value[64];
	struct mixer *mMixer = amp_ctl_inst->mixer;
	FILE *fd;

	// only rt5509 has kcontrol "Calib_Start"
	if (mixer_get_ctl_by_name(mMixer, "Calib_Start") != NULL) {
		is_rt5509 = true;
	}
	ALOGD("%s(), is_rt5509: %d", __func__, is_rt5509);

	memset(newline, '\0', 128);
	memset(calib_value, '\0', 64);
	switch (state) {
	case SPK_CALIB_STAGE_UNKNOWN:
		break;
	case SPK_CALIB_STAGE_INIT:
		if (!is_rt5509) {
			if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "CS_Comp_Disable"), 0, 1)) {
				RT_LOGE("%s: set CS_Comp_Disable fail\n", __func__);
				return -1;
			}
			volume = mixer_ctl_get_value(mixer_get_ctl_by_name(mMixer, "Volume_Ctrl"), 0);
			if (volume < 0) {
				RT_LOGE("%s: get Volume_Ctrl value fail\n", __func__);
				return -1;
			}
			if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "Volume_Ctrl"), 0, 231)) {
				RT_LOGE("%s: set Volume_Ctrl fail\n", __func__);
				return -1;
			}
		} else {
			if (mixer_ctl_get_value(mixer_get_ctl_by_name(mMixer, "Calib_Start"), 0) == 1) {
				ALOGD("%s(), already calibrated", __FUNCTION__);
				return -1;
			} else {
				return 0;
			}
		}
		break;
	case SPK_CALIB_STAGE_CALCULATE_AND_SAVE:
		if (is_rt5509) {
			if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "Calib_Start"), 0, 5526799)) {
				ALOGE("RT5509 trigger Calibration failed");
			} else {
				ALOGD("RT5509 trigger Calibration successfully");
			}
		} else {
#ifdef __MT6660
			ap_nvram_smartpa_calib_config_struct val;
			F_ID smartpa_calib_fd;
			int file_lid = AP_CFG_CUSTOM_FILE_SMARTPA_CALIB_LID;
			fd = popen("mt6660_calibration -m 5526789 -2 /data/vendor/audiohal/audio_dump/*_ivdump.pcm", "r");
			while((fgets(newline, 128, fd)) != NULL) {
				ALOGD("%s", newline);
				if (strncmp(newline, calib_str, strlen(calib_str)) == 0) {
					snprintf(calib_value, sizeof(calib_value), "%s", newline + strlen(calib_str));
					val.calib_dcr_val = atoi(calib_value);
					ALOGD("calib_value:%d", val.calib_dcr_val);
					smartpa_calib_fd = NVM_GetFileDesc(file_lid, &size, &num, ISWRITE);
					result = write(smartpa_calib_fd.iFileDesc, &val, size*num);
					if (result != size*num) {
						ALOGE("%s NVRAM write fail\n", __func__);
						pclose(fd);
						return -1;
					}
					NVM_CloseFileDesc(smartpa_calib_fd);
					result = FileOp_BackupToBinRegion_All() ? 1 : 0;
					if (result) {
						ALOGE("SCP SmartPA calibration pass");
						result = 0;
					} else {
						ALOGD("SCP SmartPA calibration error");
						result = -1;
					}
				}
			}
			pclose(fd);
#endif /* __MT6660 */
		}

		break;
	case SPK_CALIB_STAGE_DEINIT:
		if (!is_rt5509) {
			if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "CS_Comp_Disable"), 0, 0)) {
				ALOGW("%s(), CS_Comp_Disable fail", __func__);
				return -1;
			}
			if (mixer_ctl_set_value(mixer_get_ctl_by_name(mMixer, "Volume_Ctrl"), 0, volume)) {
				RT_LOGE("%s: set Volume_Ctrl fail\n", __func__);
				return -1;
			}
		}
		break;
	}
    return result;
}

int mtk_smartpa_init(struct SmartPa *smart_pa)
{
	smart_pa->ops.init = rt_extamp_init;
	smart_pa->ops.deinit = rt_extamp_deinit;
	smart_pa->ops.speakerOn = rt_extamp_speakerOn;
	smart_pa->ops.speakerOff = rt_extamp_speakerOff;
	smart_pa->ops.speakerCalibrate = rt_extamp_speakerCalibrate;
	return 0;
}
