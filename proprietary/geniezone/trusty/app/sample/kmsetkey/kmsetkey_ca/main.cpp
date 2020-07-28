#include <iostream>
#include <ctime>
#include <sys/types.h>
#include <kmsetkey.h>
using namespace std;

#define TEST_TIMES 1000
int main(int argc, char *argv[])
{
	uint32_t ret = 0;
	struct timespec total_t1, total_t2;
	double average_time_diff;
	int i;

	if (argc != 2 || (argv[1][0] != 'R' && argv[1][0] != 'E' && argv[1][0] != 'A' && argv[1][0] != 'S')) {
		cout<<"usage: kmsetkey <(R)SA / (E)CC / (A)ES / (S)HA>\n";
		return -1;
	}

	cout<<"call ree_benchmark with cmd = "<<argv[1]<<endl;
	clock_gettime(CLOCK_MONOTONIC, &total_t1);
	for (i = 0; i < TEST_TIMES; ++i)
		ret += ree_benchmark(argv[1], true);
	clock_gettime(CLOCK_MONOTONIC, &total_t2);
	average_time_diff = (total_t2.tv_sec - total_t1.tv_sec) * 1000000000 + total_t2.tv_nsec;
	average_time_diff -= total_t1.tv_nsec;
	average_time_diff /= 1000000;
	average_time_diff /= TEST_TIMES;
	cout<<"ree_benchmark = "<<ret<<endl<<"average time (ms) = "<<average_time_diff<<endl<<endl;
	return 0;
}
