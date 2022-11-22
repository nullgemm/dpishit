#include "include/dpishit.h"
#include "common/dpishit_private.h"
#include "include/dpishit_x11.h"
#include "x11/x11.h"
#include "nix/nix.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <xcb/xcb.h>
#include <xcb/randr.h>
#include <xcb/xcb_xrm.h>

static bool dpishit_xresources_xft_double(
	struct dpishit* context,
	const char* prop,
	double* out)
{
	struct x11_backend* backend = context->backend_data;

	xcb_xrm_database_t* x11_xrm_db =
		xcb_xrm_database_from_default(
			backend->conn);

	if (x11_xrm_db == NULL)
	{
		return false;
	}

	char* double_str = NULL;

	// find Xft.dpi/Xft.scale, the font dpi/scale for Keith Packard's Xft...
	xcb_xrm_resource_get_string(
		x11_xrm_db,
		prop,
		NULL,
		&double_str);

	if (double_str == NULL)
	{
		xcb_xrm_database_free(x11_xrm_db);
		return false;
	}

	// according to the documentation, Xft.dpi and Xft.scale are `double`s...
	char* endptr;
	double double_num = strtod(double_str, &endptr);

	if ((*endptr != '\0') || (endptr == double_str))
	{
		xcb_xrm_database_free(x11_xrm_db);
		free(double_str);
		return false;
	}

	// let's not throw theoretical precision away...
	*out = double_num;

	xcb_xrm_database_free(x11_xrm_db);
	free(double_str);
	return true;
}

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

static void dpishit_refresh_display_list(
	struct dpishit* context,
	struct dpishit_error_info* error)
{
	struct x11_backend* backend = context->backend_data;
	xcb_generic_error_t* error_xcb;

	// get screen resources
	xcb_randr_get_screen_resources_current_cookie_t screen_res_cookie =
		xcb_randr_get_screen_resources_current(
			backend->conn,
			backend->window);

	xcb_randr_get_screen_resources_current_reply_t* screen_res_reply =
		xcb_randr_get_screen_resources_current_reply(
			backend->conn,
			screen_res_cookie,
			&error_xcb);

	if (error_xcb != NULL)
	{
		dpishit_error_throw(context, error, DPISHIT_ERROR_X11_SCREEN_GET);
		free(error_xcb);
		return;
	}

	// get outputs list
	int outputs_count =
		xcb_randr_get_screen_resources_current_outputs_length(
			screen_res_reply);
	
	xcb_randr_output_t* outputs_list =
		xcb_randr_get_screen_resources_current_outputs(
			screen_res_reply);

	// loop over the list
	xcb_randr_get_output_info_cookie_t output_info_cookie;
	xcb_randr_get_output_info_reply_t* output_info_reply;
	xcb_randr_get_crtc_info_cookie_t crtc_info_cookie;
	xcb_randr_get_crtc_info_reply_t* crtc_info_reply;
	xcb_randr_crtc_t* crtcs_list;
	int crtcs_count;
	int i = 0;
	int k = 0;

	if (context->display_info != NULL)
	{
		free(context->display_info);
	}

	context->display_info =
		malloc(
			outputs_count
			* (sizeof (struct dpishit_display_info)));

	if (context->display_info == NULL)
	{
		dpishit_error_throw(context, error, DPISHIT_ERROR_ALLOC);
		context->display_info_count = 0;
		free(screen_res_reply);
		return;
	}

	while (i < outputs_count)
	{
		// get output info
		output_info_cookie =
			xcb_randr_get_output_info(
				backend->conn,
				outputs_list[i],
				XCB_CURRENT_TIME);

		output_info_reply =
			xcb_randr_get_output_info_reply(
				backend->conn,
				output_info_cookie,
				&error_xcb);

		if (error_xcb != NULL)
		{
			dpishit_error_throw(context, error, DPISHIT_ERROR_X11_OUTPUT_GET);
			free(screen_res_reply);
			free(error_xcb);
			return;
		}

		if (output_info_reply->connection != XCB_RANDR_CONNECTION_CONNECTED)
		{
			free(output_info_reply);
			++i;
			continue;
		}

		crtc_info_cookie =
			xcb_randr_get_crtc_info(
				backend->conn,
				output_info_reply->crtc,
				XCB_CURRENT_TIME);

		crtc_info_reply =
			xcb_randr_get_crtc_info_reply(
				backend->conn,
				crtc_info_cookie,
				&error_xcb);

		if (error_xcb != NULL)
		{
			// It is possible for an output to have invalid CRTCs
			// without it being an error, so we can just move on.
			free(output_info_reply);
			++i;
			continue;
		}

		if (crtc_info_reply->num_outputs == 0)
		{
			free(crtc_info_reply);
			free(output_info_reply);
			++i;
			continue;
		}

		context->display_info[k].x = crtc_info_reply->x;
		context->display_info[k].y = crtc_info_reply->y;
		context->display_info[k].px_width = crtc_info_reply->width;
		context->display_info[k].px_height = crtc_info_reply->height;
		context->display_info[k].mm_width = output_info_reply->mm_width;
		context->display_info[k].mm_height = output_info_reply->mm_height;

		if (backend->gdk_dpi_logic_valid == true)
		{
			context->display_info[k].dpi_logic_valid = true;
			context->display_info[k].dpi_logic =
				backend->gdk_dpi_logic
				* crtc_info_reply->width
				* 25.4
				/ output_info_reply->mm_width;
		}
		else
		{
			context->display_info[k].dpi_logic_valid = backend->dpi_logic_valid;
			context->display_info[k].dpi_logic = backend->dpi_logic;
		}

		context->display_info[k].dpi_scale_valid = backend->dpi_scale_valid;
		context->display_info[k].dpi_scale = backend->dpi_scale;
		++k;

		free(crtc_info_reply);
		free(output_info_reply);
		++i;
	}

	free(screen_res_reply);
	context->display_info_count = k;
	dpishit_error_ok(error);
	return;
}

void dpishit_x11_init(
	struct dpishit* context,
	struct dpishit_error_info* error)
{
	struct x11_backend* backend = malloc(sizeof (struct x11_backend));

	if (backend == NULL)
	{
		dpishit_error_throw(context, error, DPISHIT_ERROR_ALLOC);
		return;
	}

	struct x11_backend zero = {0};
	*backend = zero;

	context->backend_data = backend;

	dpishit_error_ok(error);
}

void dpishit_x11_start(
	struct dpishit* context,
	void* data,
	struct dpishit_error_info* error)
{
	struct x11_backend* backend = context->backend_data;
	struct dpishit_x11_data* window_data = data;
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
		dpishit_error_throw(context, error, DPISHIT_ERROR_X11_RANDR_MISSING);
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
		dpishit_error_throw(context, error, DPISHIT_ERROR_X11_RANDR_VERSION);
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
		dpishit_error_throw(context, error, DPISHIT_ERROR_X11_RANDR_EVENT);
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

bool dpishit_x11_handle_event(
	struct dpishit* context,
	void* event,
	struct dpishit_display_info* display_info,
	struct dpishit_error_info* error)
{
	struct x11_backend* backend = context->backend_data;

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
			dpishit_error_throw(context, error, DPISHIT_ERROR_X11_TRANSLATE);
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

void dpishit_x11_stop(
	struct dpishit* context,
	struct dpishit_error_info* error)
{
	struct x11_backend* backend = context->backend_data;

	dpishit_error_ok(error);
}

void dpishit_x11_clean(
	struct dpishit* context,
	struct dpishit_error_info* error)
{
	struct x11_backend* backend = context->backend_data;

	if (context->display_info != NULL)
	{
		free(context->display_info);
	}

	free(backend);

	dpishit_error_ok(error);
}

void dpishit_prepare_init_x11(
	struct dpishit_config_backend* config)
{
	config->data = NULL;
	config->init = dpishit_x11_init;
	config->start = dpishit_x11_start;
	config->handle_event = dpishit_x11_handle_event;
	config->stop = dpishit_x11_stop;
	config->clean = dpishit_x11_clean;
}
