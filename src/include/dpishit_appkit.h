#ifndef H_DPISHIT_APPKIT
#define H_DPISHIT_APPKIT

#include "dpishit.h"

struct dpishit_appkit_data
{
	void* data;
};

void dpishit_prepare_init_appkit(
	struct dpishit_config_backend* config);

#endif
