#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <string>
#include <fstream>

#include "schedule.h"
#include "common_log.h"

int main(int argc, char **argv)
{
	Schedule schedule;

	if ( 2 != argc )
	{
		LOG_INFO("usage:%s [bt file]", argv[0]);
		return -1;
	}

	schedule.startTask(argv[1]);

	return 0;
}
