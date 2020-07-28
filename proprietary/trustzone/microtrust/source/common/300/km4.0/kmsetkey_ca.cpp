/*
 * Copyright 2018 The Android Open Source Project
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

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <ut_kmsetkey.h>
#include <dlfcn.h>
#include <cutils/properties.h>
#include <log/log.h>
#include <unistd.h>

#define KB_SIZE 16384
#define KEYBOX_CHECK "hello keybox"
#define KEYBOX_CHECK_LEN strlen(KEYBOX_CHECK)
#define KEYBOX_STATUS_PROP "vendor.soter.teei.googlekey.status"

using namespace std;

kmsetkey_device* device = nullptr;
const hw_module_t* module;

int main(int argc, char *argv[])
{
	cout<<"bp_kmsetkey_ca v1.0\n";
	uint8_t kb[KB_SIZE];
	uint32_t kb_len = 0;
	bool import_keybox = false;
	int ret = 0;

	if (argv[1][0] != '-' || (argv[1][1] != 'i' && argv[1][1] != 'I' && argv[1][1] != 'c'&& argv[1][1] != 'C')) {
		cout<<"Usage: kmsetkey_ca <-i/-c> <keybox_file>\n";
		return -1;
	}

	if (argv[1][1] == 'i' || argv[1][1] == 'I') {
	    if (argc != 3) {
			cout<<"Usage: kmsetkey_ca <-i/-c> <keybox_file>\n";
		return -1;
	    }
		ifstream fin;
	    fin.open(argv[2], ios::in | ios::binary);
	    if (!fin.good()) {
		cout<<"keybox_file open failed\n";
		return -2;
	    }
	    fin.seekg(0, ios::end);
	    kb_len = fin.tellg();
	    fin.seekg(0, ios::beg);
	    cout<<"kb_len = "<<kb_len<<endl;
	    if (kb_len == 0 || kb_len > KB_SIZE) {
		cout<<"kb_len is zero or too large ( > "<<KB_SIZE<<")\n";
		return -3;
	    }

	    fin.read((char *)kb, kb_len);
	    fin.close();
	    import_keybox = true;
	} else {
	    memcpy(kb, KEYBOX_CHECK, KEYBOX_CHECK_LEN);
	    kb_len = KEYBOX_CHECK_LEN;
	}

	ret = hw_get_module_by_class(KMSETKEY_HARDWARE_MODULE_ID, NULL, &module);
	if (ret) {
		cout<<"Unable to open key_attestation HAL.\n";
		return -4;
	}

	ret = kmsetkey_open(module, &device);
	if (ret) {
		cout<<"Error to open kmsetkey device.\n";
		dlclose(module->dso);
		return -5;
	}

	if (device == nullptr) {
		cout<<"No key_attestation HAL exists.\n";
		dlclose(module->dso);
		return -6;
	}

	if (import_keybox == true) {
		cout<<"Calling attest_key_install...\n";
		ret = device->attest_key_install(kb, kb_len);
	if (ret) {
		    cout<<"attest key install failed "<<ret<<endl;
	} else {
	    property_set(KEYBOX_STATUS_PROP, "ok");
		    cout<<"attest key check success "<<ret<<endl;
	}
	} else {
		cout<<"Calling attest_key_check...\n";
		ret = device->attest_key_check(kb, kb_len);
	if (ret) {
	    property_set(KEYBOX_STATUS_PROP, "fail");
		    cout<<"attest key check failed "<<ret<<endl;
	} else {
	    property_set(KEYBOX_STATUS_PROP, "ok");
		    cout<<"attest key check success "<<ret<<endl;
	}
	}

    if (ret) {
		dlclose(module->dso);
	return -7;
    }

	ret = kmsetkey_close(device);
	if (ret)
		cout<<"Unable to close key_attestation HAL\n";

	dlclose(module->dso);
	return ret;
}
