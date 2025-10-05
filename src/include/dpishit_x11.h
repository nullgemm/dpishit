#ifndef H_DPISHIT_X11
#define H_DPISHIT_X11

#include "dpishit.h"

#include <xcb/xcb.h>

struct dpishit_x11_data
{
	xcb_connection_t* conn;
	xcb_window_t window;
	xcb_window_t root;
};

#if !defined(DPISHIT_SHARED)
void dpishit_prepare_init_x11(
	struct dpishit_config_backend* config);
#endif

#endif
