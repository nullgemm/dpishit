#ifndef H_DPISHIT_WIN
#define H_DPISHIT_WIN

#include "dpishit.h"

#include <windows.h>

struct dpishit_win_data
{
	HWND win;
	HDC device_context;
};

void dpishit_prepare_init_win(
	struct dpishit_config_backend* config);

#endif
