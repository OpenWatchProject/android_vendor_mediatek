#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <kmsetkey.h>
using namespace std;

int main(int argc, char *argv[])
{
	uint8_t kb[16384];
	uint32_t kb_len = 0, ret = 0;
	ifstream fin;

	if (argc == 2) {
		fin.open(argv[1], ios::in | ios::binary);
		if (fin.good()) {
			fin.seekg (0, ios::end);
			kb_len = fin.tellg();
			fin.seekg (0, ios::beg);
			fin.read ((char *)kb, kb_len);
			fin.close();
		}
	}

	ret = ree_import_attest_keybox(kb, kb_len, true);
	cout<<"kb_len = "<<kb_len<<endl;
	cout<<"ree_import_attest_keybox = "<<ret<<endl;
	return 0;
}
