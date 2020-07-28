#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "librda_export.h"


/******************************************************************************
 *  RDA MAIN FLOW
 ******************************************************************************/
int main(void)
{
	int ret = 0;

	ret = rda_check(OPT_UPDATE_ROOT_BASED_ON_UNION, (1UL << ROOT_SYMPTOM_MAX)-1,
	                0, 0, MAX_TIMEOUT);

	return ret;
}

