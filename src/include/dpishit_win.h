#ifndef H_DPISHIT_WIN
#define H_DPISHIT_WIN

#include "dpishit.h"

struct dpishit_win_data
{
	xcb_connection_t* conn;
	xcb_window_t window;
	xcb_window_t root;
};

void dpishit_prepare_init_win(
	struct dpishit_config_backend* config);

#endif
