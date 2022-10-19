#ifndef H_DPISHIT_PRIVATE
#define H_DPISHIT_PRIVATE

#include "include/dpishit.h"
#include "common/dpishit_error.h"

struct dpishit
{
	char* error_messages[DPISHIT_ERROR_COUNT];
	void* backend_data;
	struct dpishit_config_backend backend_callbacks;
	struct dpishit_display_info display_info;
};

#endif
