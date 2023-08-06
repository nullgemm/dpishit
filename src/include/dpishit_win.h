#ifndef H_DPISHIT_WIN
#define H_DPISHIT_WIN

#include "dpishit.h"

#include <windows.h>

struct dpishit_win_data
{
	void* data;
};

void dpishit_prepare_init_win(
	struct dpishit_config_backend* config);

#endif
