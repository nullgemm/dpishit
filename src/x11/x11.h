#ifndef H_DPISHIT_X11_PRIVATE
#define H_DPISHIT_X11_PRIVATE

#include "dpishit.h"
#include "common/dpishit_error.h"

#include <xcb/xcb.h>

struct x11_backend
{
	xcb_connection_t* conn;
	xcb_window_t window;
	double font_dpi;
};

void dpishit_x11_init(
	struct dpishit* context,
	struct dpishit_error_info* error);

void dpishit_x11_start(
	struct dpishit* context,
	void* data,
	struct dpishit_error_info* error);

struct dpishit_display_info dpishit_x11_get(
	struct dpishit* context,
	struct dpishit_error_info* error);

void dpishit_x11_stop(
	struct dpishit* context,
	struct dpishit_error_info* error);

void dpishit_x11_clean(
	struct dpishit* context,
	struct dpishit_error_info* error);

#endif
