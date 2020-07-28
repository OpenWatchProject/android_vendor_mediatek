#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <kmattest.h>
#include <dlfcn.h>

#define KB_SIZE 16384

using namespace std;

kmsetkey_device* device = nullptr;
const hw_module_t* module;

int main(int argc, char *argv[])
{
	uint8_t kb[KB_SIZE];
	uint32_t kb_len = 0;
	int ret = 0;
	ifstream fin;

	if (argc != 3 || argv[1][0] != '-' || (argv[1][1] != 'i' && argv[1][1] != 'I' && argv[1][1] != 'c'&& argv[1][1] != 'C')) {
		cout<<"Usage: kmsetkey_ca <-i/-c> <keybox_file>\n";
		return -1;
	}

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

	if (argv[1][1] == 'i' || argv[1][1] == 'I') {
		cout<<"Calling attest_key_install...\n";
		ret = device->attest_key_install(kb, kb_len);
		cout<<"attest_key_install = "<<ret<<endl;
	} else {
		cout<<"Calling attest_key_check...\n";
		ret = device->attest_key_check(kb, kb_len);
		cout<<"attest_key_check = "<<ret<<endl;
	}

	ret = kmsetkey_close(device);
	if (ret)
		cout<<"Unable to close key_attestation HAL\n";

	dlclose(module->dso);
	return 0;
}
