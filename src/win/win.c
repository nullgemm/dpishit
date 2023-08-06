#include "include/dpishit.h"
#include "common/dpishit_private.h"
#include "include/dpishit_win.h"
#include "win/win.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static double abs2(double value)
{
	return value < 0.0 ? -value : value;
}

static bool overlap(
	int x1a,
	int y1a,
	int x1b,
	int y1b,
	int x2a,
	int y2a,
	int x2b,
	int y2b)
{
	// 0-area rectangle
	if (x1a == x1b || y1a == y1b || x2a == x2b || y2a == y2b)
	{
		return false;
	}

	// separated horizontally
	if (x1a > x2b || x2a > x1b)
	{
		return false;
	}

	// separated vertically
	if (y1a > y2b || y2a > y1b)
	{
		return false;
	}

	return true;
}

void dpishit_win_init(
	struct dpishit* context,
	struct dpishit_error_info* error)
{
	struct win_backend* backend = malloc(sizeof (struct win_backend));

	if (backend == NULL)
	{
		dpishit_error_throw(context, error, DPISHIT_ERROR_ALLOC);
		return;
	}

	struct win_backend zero = {0};
	*backend = zero;

	context->backend_data = backend;

	dpishit_error_ok(error);
}

void dpishit_win_start(
	struct dpishit* context,
	void* data,
	struct dpishit_error_info* error)
{
	struct win_backend* backend = context->backend_data;
	struct dpishit_win_data* window_data = data;
	xcb_generic_error_t* error_xcb;

	backend->conn = window_data->conn;
	backend->window = window_data->window;
	backend->root = window_data->root;
	backend->gdk_dpi_logic = 0.0;
	backend->gdk_dpi_logic_valid = false;
	backend->dpi_logic = 0.0;
	backend->dpi_logic_valid = false;
	backend->dpi_scale = 0.0;
	backend->dpi_scale_valid = false;

	backend->window_x = 0;
	backend->window_y = 0;
	backend->window_width = 0;
	backend->window_height = 0;
	backend->event = 0;

	// check RandR is available
	const xcb_query_extension_reply_t* extension =
		xcb_get_extension_data(
			backend->conn,
			&xcb_randr_id);

	if ((extension == NULL) || (extension->present == 0))
	{
		dpishit_error_throw(context, error, DPISHIT_ERROR_win_RANDR_MISSING);
		return;
	}

	// ask for a specific RandR version
	xcb_randr_query_version_cookie_t version_cookie =
		xcb_randr_query_version(backend->conn, 1, 4);

	xcb_randr_query_version_reply_t* version_reply =
		xcb_randr_query_version_reply(
			backend->conn,
			version_cookie,
			&error_xcb);

	if (error_xcb != NULL)
	{
		dpishit_error_throw(context, error, DPISHIT_ERROR_win_RANDR_VERSION);
		return;
	}

	free(version_reply);

	backend->event = extension->first_event + XCB_RANDR_NOTIFY;

	// register for screen update events
	xcb_void_cookie_t randr_event_cookie =
		xcb_randr_select_input(
			backend->conn,
			backend->window,
			XCB_RANDR_NOTIFY_MASK_CRTC_CHANGE
			| XCB_RANDR_NOTIFY_MASK_OUTPUT_CHANGE
			| XCB_RANDR_NOTIFY_MASK_RESOURCE_CHANGE);

	error_xcb =
		xcb_request_check(
			backend->conn,
			randr_event_cookie);

	if (error_xcb != NULL)
	{
		dpishit_error_throw(context, error, DPISHIT_ERROR_win_RANDR_EVENT);
		return;
	}

	// get Xft's font dpi value
	backend->dpi_scale_valid =
		dpishit_xresources_xft_double(
			context,
			"Xft.scale",
			&(backend->dpi_scale));

	if (backend->dpi_scale_valid == false)
	{
		// the Xft value could not be relied on so we try using the environment
		char* env[3] =
		{
			"GDK_SCALE",
			"ELM_SCALE",
			"QT_SCALE_FACTOR",
		};

		backend->dpi_scale_valid =
			dpishit_env_double(
				context,
				env,
				3,
				&(backend->dpi_scale));
	}

	// get Xft's font scale value
	backend->dpi_logic_valid =
		dpishit_xresources_xft_double(
			context,
			"Xft.dpi",
			&(backend->dpi_logic));

	if (backend->dpi_logic_valid == false)
	{
		// the Xft value could not be relied on so we try using the GDK variable
		char* env = "GDK_DPI_SCALE";

		backend->dpi_logic_valid =
			dpishit_env_double(
				context,
				&env,
				1,
				&(backend->gdk_dpi_logic));
	}

	if (backend->dpi_logic_valid == false)
	{
		// the GDK environment variable is not valid, try with the Qt variable
		char* env = "QT_FONT_DPI";

		backend->dpi_logic_valid =
			dpishit_env_double(
				context,
				&env,
				1,
				&(backend->dpi_logic));
	}
	else
	{
		// the GDK environment variable is valid, but since it is a density scale
		// we have to compute the actual logic density value manually
		backend->dpi_logic_valid = false;
		backend->gdk_dpi_logic_valid = true;
	}

	dpishit_refresh_display_list(context, error);

	if (dpishit_error_get_code(error) != DPISHIT_ERROR_OK)
	{
		return;
	}

	dpishit_error_ok(error);
}

bool dpishit_win_handle_event(
	struct dpishit* context,
	void* event,
	struct dpishit_display_info* display_info,
	struct dpishit_error_info* error)
{
	struct win_backend* backend = context->backend_data;

	// hande configure notify and screen update events
	xcb_generic_event_t* xcb_event = event;

	// only lock the main mutex when making changes to the context
	int code = xcb_event->response_type & ~0x80;

	if (code == XCB_CONFIGURE_NOTIFY)
	{
		xcb_configure_notify_event_t* configure =
			(xcb_configure_notify_event_t*) xcb_event;

		// translate position in screen coordinates
		xcb_generic_error_t* error_xcb;

		xcb_translate_coordinates_cookie_t cookie_translate =
			xcb_translate_coordinates(
				backend->conn,
				backend->window,
				backend->root,
				0,
				0);

		xcb_translate_coordinates_reply_t* reply_translate =
			xcb_translate_coordinates_reply(
				backend->conn,
				cookie_translate,
				&error_xcb);

		if (error_xcb != NULL)
		{
			dpishit_error_throw(context, error, DPISHIT_ERROR_win_TRANSLATE);
			return false;
		}

		backend->window_x = reply_translate->dst_x;
		backend->window_y = reply_translate->dst_y;
		backend->window_width = configure->width;
		backend->window_height = configure->height;

		free(reply_translate);
	}
	else if (code == backend->event)
	{
		dpishit_refresh_display_list(context, error);

		if (dpishit_error_get_code(error) != DPISHIT_ERROR_OK)
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	int x1a = backend->window_x;
	int y1a = backend->window_y;
	int x1b = x1a + backend->window_width;
	int y1b = y1a + backend->window_height;
	int x2a = 0;
	int y2a = 0;
	int x2b = 0;
	int y2b = 0;
	double area = 0.0;
	double width = 0.0;
	double height = 0.0;
	bool intersection = false;
	bool valid = false;

	for (size_t i = 0; i < context->display_info_count; ++i)
	{
		x2a = context->display_info[i].x;
		y2a = context->display_info[i].y;
		x2b = x2a + context->display_info[i].px_width;
		y2b = y2a + context->display_info[i].px_height;

		intersection = overlap(x1a, y1a, x1b, y1b, x2a, y2a, x2b, y2b);

		// fails if the window is not on this output (because no intersection)
		// fails if window sizes not initialized (because all values are zero)
		if (intersection == true)
		{
			// reduce the second rectangle to the intersection of both
			x2a = (x1a > x2a) ? x1a : x2a;
			y2a = (y1a > y2a) ? y1a : y2a;
			x2b = (x1b < x2b) ? x1b : x2b;
			y2b = (y1b < y2b) ? y1b : y2b;

			// compute width and height of the intersection rectangle
			width = x2b - x2a;
			height = y2b - y2a;

			// convert width and height to millimeters
			width *= context->display_info[i].mm_width;
			height *= context->display_info[i].mm_height;

			width /= context->display_info[i].px_width;
			height /= context->display_info[i].px_height;

			// select the output displaying the biggest part of the window
			// (comparing the window area in square millimeters on each screen)
			if (abs2(width * height) > area)
			{
				valid = true;
				*display_info = context->display_info[i];
				area = abs2(width * height);
			}
		}
	}

	return valid;
}

void dpishit_win_stop(
	struct dpishit* context,
	struct dpishit_error_info* error)
{
	struct win_backend* backend = context->backend_data;

	dpishit_error_ok(error);
}

void dpishit_win_clean(
	struct dpishit* context,
	struct dpishit_error_info* error)
{
	struct win_backend* backend = context->backend_data;

	if (context->display_info != NULL)
	{
		free(context->display_info);
	}

	free(backend);

	dpishit_error_ok(error);
}

void dpishit_prepare_init_win(
	struct dpishit_config_backend* config)
{
	config->data = NULL;
	config->init = dpishit_win_init;
	config->start = dpishit_win_start;
	config->handle_event = dpishit_win_handle_event;
	config->stop = dpishit_win_stop;
	config->clean = dpishit_win_clean;
}
