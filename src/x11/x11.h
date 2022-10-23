#ifndef H_DPISHIT_X11_PRIVATE
#define H_DPISHIT_X11_PRIVATE

#include "dpishit.h"
#include "common/dpishit_error.h"

#include <stdbool.h>
#include <xcb/xcb.h>

struct x11_backend
{
	xcb_connection_t* conn;
	xcb_window_t window;
	xcb_window_t root;
	double gdk_dpi_logic;
	bool gdk_dpi_logic_valid;
	double dpi_logic;
	bool dpi_logic_valid;
	double dpi_scale;
	bool dpi_scale_valid;

	int window_x;
	int window_y;
	unsigned window_width;
	unsigned window_height;
};

void dpishit_x11_init(
	struct dpishit* context,
	struct dpishit_error_info* error);

void dpishit_x11_start(
	struct dpishit* context,
	void* data,
	struct dpishit_error_info* error);

bool dpishit_x11_handle_event(
	struct dpishit* context,
	void* event,
	struct dpishit_display_info* display_info,
	struct dpishit_error_info* error);

void dpishit_x11_stop(
	struct dpishit* context,
	struct dpishit_error_info* error);

void dpishit_x11_clean(
	struct dpishit* context,
	struct dpishit_error_info* error);

#endif
