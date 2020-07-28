
#include <cstdio>

#include <new>

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cfloat>
#include <cstddef>
#include <vector>
#include <functional>
#include <arm_neon.h>




#define PI 3.14159265


extern "C" void test_CXX_main (void);

template <class T>
void swap(T &a1, T &a2)
{
                T tmp = a1;
                a1 = a2;
                a2 = tmp;
}

struct Compare : public std::binary_function<int,int,bool> {
  bool operator() (int a, int b) {return (a==b);}
};


#define TEST_ARRAY_SIZE 32

void test_CXX_main (void)
{
	char *string, *string2;
	int *array, *array1;	
	int i;
	int a1 = 3;
  int a2 = 8;
  float a3 = 1.1;
  float a4 = 8.8;
  
  printf("========>before swap, a1=%d, a2=%d\n",a1 ,a2);
  swap(a1,a2);
  printf("========>after swap, a1=%d, a2=%d\n",a1 ,a2);

  int c = pow(a1,a2);
  printf("a1^(a2) = %d\n",c);
  
  printf("========>before swap, a3=%f, a4=%f\n",a3 ,a4);
  swap(a3,a4);
  printf("========>after swap, a3=%f, a4=%f\n",a3 ,a4);

	
	string = new char[256];
	string2 = new char[256];
	array = new int[TEST_ARRAY_SIZE];
	
	if (string == NULL)
		printf ("can not new string!!!\n");
		
	if (array == NULL)
		printf ("can not new array!!!\n");
	
	strcpy(string, "test C++!!!");
	strncpy(string2, "test C++!!!",1);
	memset(array, 0xab, TEST_ARRAY_SIZE);
	
	printf("===> string (0x%x) = %s\n", string, string);
	printf("===> string2 (0x%x) = %s\n", string2, string2);
	printf("===> array (0x%x)\n", array);

	for (i = 0; i < TEST_ARRAY_SIZE/sizeof(int); i ++)
		printf("%x", array[i]);
	printf("\n");
	
	delete string;
	delete array;
	/////////////////////////////cmath test///////////////////////////////////
	printf("=================>This is <math.h> test..............\n");
  	double param, result;
  	param = 60.0;
  	result = cos ( param * PI / 180.0 );
 	printf ("The cosine of %f degrees is %f.\n", param, result );

	double param2, result_2;
	 param2 = 5.5;
	 result_2 = log (param2);
	 printf ("log(%f) = %f\n", param2, result_2 );

	double param_atan, result_atan;
  	param_atan = 1.0;
  	result_atan = atan (param_atan) * 180 / PI;
  	printf ("The arc tangent of %f is %f degrees\n", param_atan, result_atan);

	double x, y, result_atan2;
  	x = -10.0;
  	y = 10.0;
  	result_atan2 = atan2 (y,x) * 180 / PI;
  	printf ("The arc tangent for (x=%f, y=%f) is %f degrees\n", x, y, result_atan2 );

	printf ( "ceil of 2.3 is %.1f\n", ceil(2.3) );
  	printf ( "ceil of 3.8 is %.1f\n", ceil(3.8) );
  	printf ( "ceil of -2.3 is %.1f\n", ceil(-2.3) );
  	printf ( "ceil of -3.8 is %.1f\n", ceil(-3.8) );


	double param_exp, result_exp;
  	param_exp = 5.0;
  	result_exp = exp (param_exp);
  	printf ("The exponential value of %f is %f.\n", param_exp, result_exp );

	const char * format = "%.1f \t%.1f \t%.1f \t%.1f \t%.1f\n";
  	printf ("value\tround\tfloor\tceil\ttrunc\n");
  	printf ("-----\t-----\t-----\t----\t-----\n");
  	printf (format, 2.3,round( 2.3),floor( 2.3),ceil( 2.3),trunc( 2.3));
  	printf (format, 3.8,round( 3.8),floor( 3.8),ceil( 3.8),trunc( 3.8));
  	printf (format, 5.5,round( 5.5),floor( 5.5),ceil( 5.5),trunc( 5.5));
  	printf (format,-2.3,round(-2.3),floor(-2.3),ceil(-2.3),trunc(-2.3));
  	printf (format,-3.8,round(-3.8),floor(-3.8),ceil(-3.8),trunc(-3.8));
  	printf (format,-5.5,round(-5.5),floor(-5.5),ceil(-5.5),trunc(-5.5));

	double param_tanh, result_tanh;
  	param_tanh = log(2.0);
  	result_tanh = tanh (param_tanh);
  	printf ("The hyperbolic tangent of %f is %f.\n", param_tanh, result_tanh);
	double angle = 0.40, result_tanh2;
    result_tanh2 = tanh(angle);
    printf("Tangent hyperbolic of %.2lf (in radians) = %.2lf \n", angle, result_tanh2);

	printf("=================>This is <math.h> test, end..............\n");

	////////////////////////////////////////////////////////////////////////	

	double atof_tmp;
	char atof_test[] = "3.14159";

	atof_tmp = atof(atof_test);
	

	printf("=====================>this is atof test............string:%s , after atof ==>%lf\n", atof_test, atof_tmp);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	int first[] = {-555,10,-15,20,25};
  	int second[] = {50,40,30,20,10};
	std::vector<int> v(10);

	printf("first[] = ");
	int index=0;
	while(index<5)
			{
				printf("%5d",first[index]); 
				index++;
			}
	printf("\n");

	index=0;
	printf("second[] = ");
	while(index<5)
			{
				printf("%5d",second[index]); 
				index++;
			}
	printf("\n");

	int cx = std::count_if (first, first+5, std::bind2nd(std::greater_equal<int>(),0));
  	printf("There are %d non-negative elements in first[].\n",cx);

	std::merge (first,first+5,second,second+5,v.begin());
	std::sort(v.begin(),v.end());
	printf("The result after merge first[] & second[] and sort:");
	for (std::vector<int>::iterator it=v.begin(); it!=v.end(); ++it) printf("%5d",*it);
	printf("\n");

	
	std::vector<int> myvector;
	
	  // set some values (from 1 to 10)
	printf("my vector = ");
	for (int i=1; i<=10; i++){

		myvector.push_back(i); 
		printf("%5d",myvector[i-1]);
	}
	printf("\n");
	
	  // erase the 6th element
	myvector.erase (myvector.begin()+5);
	
	  // erase the first 3 elements:
	myvector.erase (myvector.begin(),myvector.begin()+3);
	
	printf("After erase the 6th element and the first 3 elements, myvector contains:");
	for (unsigned i=0; i<myvector.size(); ++i)
			printf(" %d" ,myvector[i]);
	printf("\n");
	  ////////////////////////////////////////////////////////////////////////////////
	Compare Compare_object;
  	Compare::first_argument_type input1;
  	Compare::second_argument_type input2;
  	Compare::result_type result2;

  
  	input1 = 2;
  
 	input2 = 33;

  	result2 = Compare_object (input1,input2);

  	printf("Numbers %d and %d",input1 ,input2);
 	if (result2)
	 	printf(" are equal.\n");
  	else
	 	printf(" are not equal.\n");
  ////////////////////////////////////////////////////////////////////////////////////
  bool foo[] = {true,false,true,false};
  bool bar[] = {true,true,false,false};
  bool result1[4];
  std::transform (foo, foo+4, bar, result1, std::logical_and<bool>());
  printf("Logical AND:\n");
  for (int i=0; i<4; i++)
  printf("%d AND %d = %d \n",foo[i], bar[i],result1[i]);
	
	/////////////////////////////////neon test//////////////////////////////////////////////
	__asm__ volatile("mov r0, r0");

	
	register int16x8_t a;
	register int16x8_t b = { 1, 2, 3, 4, 5, 6, 7, 8};
	register int16x8_t d = { 11, 12, 13, 14, 15, 16, 17, 18};
	register int16x8_t e, f;
	int16_t tmp[8];
	
   	__asm__ volatile(
      "vadd.i32 %q0, %q1, %q2 \n\t"
      : "=w" (a)
      : "w" (b), "w" (d)
      );

   vst1q_s16(tmp, a);
	for(int kk = 0;kk <8;kk++)
   		printf("inline neon asm test = %2d\n",tmp[kk]);

	e = a + b;
	vst1q_s16(tmp, e);
	
	for(int kk = 0;kk <8;kk++)
   		printf("inline neon asm test = %2d\n",tmp[kk]);

	f = vaddq_s16(e, b);

	vst1q_s16(tmp, f);

	for(int kk = 0;kk <8;kk++)
   		printf("inline neon asm test = %2d\n",tmp[kk]);
	/////////////////////////////////////////////////////////////////
	printf("========================>This is <time.h> test ........\n");
	time_t t4;
	struct tm tm4;    
	tm4.tm_hour = 12;    
	tm4.tm_min = 47;    
	tm4.tm_sec = 5;    
	tm4.tm_year = 2018-1900;    
	tm4.tm_mon = 3-1;    
	tm4.tm_mday = 26;    
	tm4.tm_wday = 1;
	t4 = mktime(&tm4);
	printf("from 19700101 00:00:00 to 20180326 12:47:05 , total second = %d\n", t4);
	printf("%s\n",asctime(&tm4));

	time_t t5;   
	struct tm tm5;    
	tm5.tm_hour = 0;    
	tm5.tm_min = 0;    
	tm5.tm_sec = 0;    
	tm5.tm_year = 2020-1900;    
	tm5.tm_mon = 7-1;    
	tm5.tm_mday = 15;    
	t5 = mktime(&tm5);
	printf("from 19700101 00:00:00 to 20200715 00:00:00 , total second = %d\n", t5);

	printf("20200715 00:00:00 - 20180326 12:47:05 = %f\n",difftime(t5,t4));

	time_t timer;    
	printf("current time =%d\n",time(&timer));

	printf("clock = %d\n", clock());
	printf("========================>This is <time.h> test, end ........\n");
}

