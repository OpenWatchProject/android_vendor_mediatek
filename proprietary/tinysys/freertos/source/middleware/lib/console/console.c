#include "main.h"
/*   Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include <debug.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <console.h>
#include <scp_testsuite.h>


#define LONG_IS_INT 1

static int hexval(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    return 0;
}

int atoi(const char *num)
{
#if !LONG_IS_INT
    // XXX fail
#else
    return atol(num);
#endif
}

unsigned int atoui(const char *num)
{
#if !LONG_IS_INT
    // XXX fail
#else
    return atoul(num);
#endif
}
long atol(const char *num)
{
    long value = 0;
    int neg = 0;

    if (num[0] == '0' && num[1] == 'x') {
        // hex
        num += 2;
        while (*num && isxdigit(*num))
            value = value * 16 + hexval(*num++);
    } else {
        // decimal
        if (num[0] == '-') {
            neg = 1;
            num++;
        }
        while (*num && isdigit(*num))
            value = value * 10 + *num++  - '0';
    }

    if (neg)
        value = -value;

    return value;
}

unsigned long atoul(const char *num)
{
    unsigned long value = 0;
    if (num[0] == '0' && num[1] == 'x') {
        // hex
        num += 2;
        while (*num && isxdigit(*num))
            value = value * 16 + hexval(*num++);
    } else {
        // decimal
        while (*num && isdigit(*num))
            value = value * 10 + *num++  - '0';
    }

    return value;
}

extern char ReadDebugByte(void);
extern void WriteDebugByte(char c);
static int read_line(char *buffer, int len)
{
    int pos = 0;
    char c;
    while ((c = ReadDebugByte()) != '\r') {
        if (c == 0xff || c == 0x0) {
            continue;
        }
        //printf("%c\n\r", c);
        WriteDebugByte(c);

        buffer[pos++] = c;
        if (c == '\r' || c == '\n') {
            goto done;
        }
        /* end of line. */
        if (pos == (len - 1)) {
            puts("\nerror: line too long\n");
            pos = 0;
            goto done;
        }
    }
done:
    //printf("returning pos %d\n\r", pos);

    buffer[pos] = 0;
    return pos;
}
extern void platformTest_init(void);
extern void plat_sample_init(void);
extern void sample_init(void);
extern void ts_dvfs_init(void);
#ifdef CFG_VOW_DVT_SUPPORT
extern void ts_vow_init(void);
#endif
//eint test
extern void enit_sample_init(void);
#ifdef CFG_CCCI_DVT_SUPPORT
extern void ts_ccif_init(void);
#endif
#ifdef CFG_I2C_SUPPORT
extern void ts_i2c_init(void);
#endif
SCP_TEST_SUITE_T test_suite_list[20];
int num_test_suite = 0;
void scp_register_testsuite(SCP_TEST_SUITE_T *test_suite)
{
    test_suite_list[num_test_suite++] = *test_suite;
}
static int show_module_list(void)
{
//collect all module though build time
//print out module name
    int ix = 0;
    PRINTF_E("Please select one module from the list\n\r");
    for (ix = 0; ix < num_test_suite; ix++) {
        PRINTF_E("%d) %s \n\r", ix, test_suite_list[ix].pzDescription);
    }
    PRINTF_E("\n\r");
    return 0;
}

void show_test_case(SCP_TEST_CASE_T scp_test_cases[])
{
    int ix;
    PRINTF_E("0) %s \n\r", "back to main menu");
    for (ix = 0; ; ix++) {
        if (scp_test_cases[ix].pfTestFunc == NULL)
            break;
        else {
            PRINTF_E("%d) %s \n\r", (ix + 1), scp_test_cases[ix].pzDescription);
        }
    }
    PRINTF_E("\n\r");
}

static void scp_run_test(int test_module)
{
    char buffer[256];
    int test_case;
    int result = SCP_FAIL;
    for (;;) {
        PRINTF_E("[%s]\n\r", test_suite_list[test_module].pzDescription);
        show_test_case(test_suite_list[test_module].psTestCase);
        int len = read_line(buffer, sizeof(buffer));
        if (len == 0) {
            continue;
        }
        test_case = atoi(buffer);
        if (test_case == 0) {
            return;
        } else {
            PRINTF_E("\n\r================= You select %d\n\r", test_case);
            result = (*test_suite_list[test_module].psTestCase[test_case - 1].pfTestFunc)(
                         test_suite_list[test_module].psTestCase[test_case - 1].psDefaultInput);
            PRINTF_E("\n\r================= %s %s\n\r\n\r\n\r",
                     test_suite_list[test_module].psTestCase[test_case - 1].pzDescription,
                     (result == SCP_SUCCESS) ? "PASS" : "FAIL");
        }
    }
}
static void console_loop(void *pvParameters)
{
    char buffer[256];
    int test_module;

    PRINTF_E("[Entering SCP test main menu]\n\r");
    for (;;) {
        show_module_list();

        int len = read_line(buffer, sizeof(buffer));
        if (len == 0)
            continue;
        test_module = atoi(buffer);
        //printf("len=%d\n\r",len);

        //printf("%d\n\r", test_module);
        PRINTF_E("You select %d\n\r", test_module);
        if (test_module >= num_test_suite) {
            PRINTF_E("Wrong number, please select again.\n\r");
            continue;
        }
        scp_run_test(test_module);
        continue;
        while (1);
    }
}

void testsuite_init(void)
{
    sample_init();
    platformTest_init();
    //eint
#ifdef CFG_EINT_SUPPORT
    enit_sample_init();
#endif
    plat_sample_init();
#ifdef CFG_VCORE_DVFS_SUPPORT
    ts_dvfs_init();
#endif
#ifdef CFG_CCCI_DVT_SUPPORT
    ts_ccif_init();
#endif
#ifdef CFG_I2C_SUPPORT
    ts_i2c_init();
#endif
#ifdef CFG_VOW_DVT_SUPPORT
    ts_vow_init();
#endif
}

void console_start(void)
{
    testsuite_init();
    xTaskCreate(console_loop, "console", 500, (void*)0, PRI_CONSOLE,
                NULL);   /* set console task as the highest priority, then the console can show on the top*/
}
