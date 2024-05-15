#include "tgw_module.h"
#include "tsim/tsim.h"
#include <stdio.h>
#include <stdlib.h>

void tgw_module_run(int argc, char *argv[])
{
	// TODO: parse system input args and parse etc file
	MUGGLE_UNUSED(argc);
	MUGGLE_UNUSED(argv);

	if (!tsim_log_init(LOG_LEVEL_INFO, -1, NULL, false, NULL)) {
		fprintf(stderr, "failed init log");
		exit(EXIT_FAILURE);
	}

	tsim_log_init_thread_ctx();

	LOG_INFO("hello trader simulator");

	tsim_log_cleanup_thread_ctx();
}
