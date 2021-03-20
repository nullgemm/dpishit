#ifndef H_DPISHIT_X11
#define H_DPISHIT_X11

#include <xcb/xcb.h>

struct dpishit_data_x11
{
	xcb_connection_t* conn;
	xcb_window_t win;
};

#endif
