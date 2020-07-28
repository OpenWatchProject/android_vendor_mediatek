/*
 * Copyright (C) 2012-2014 The Android Open Source Project
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

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <trusty_std.h>

volatile void nop(void)
{
	static int i;
	i++;
}

int main(void)
{
	int i;
	while (true) {
		printf("Hello world from timer app 1\n");
		for (i = 0; i < 100000000; i++)
			nop();
		printf("Hello world from timer app 2\n");
		for (i = 0; i < 1000; i++)
			nanosleep(0, 0, 1000 * 1000);
		printf("Hello world from timer app 3\n");
		nanosleep(0, 0, 10ULL * 1000 * 1000 * 1000);
	}
	return 0;
}
