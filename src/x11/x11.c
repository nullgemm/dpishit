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

static bool dpishit_refresh_real_density(
	struct dpishit* context)
{
	struct x11_backend* backend = context->backend_data;

	// init
	xcb_connection_t* x11_conn = backend->conn;
	xcb_window_t x11_window = backend->window;
	xcb_generic_error_t* x11_error;

	// send request and get reply
	xcb_randr_get_screen_info_cookie_t x11_screen_info_cookie =
		xcb_randr_get_screen_info(
			x11_conn,
			x11_window);

	xcb_randr_get_screen_info_reply_t* x11_screen_info_reply =
		xcb_randr_get_screen_info_reply(
			x11_conn,
			x11_screen_info_cookie,
			&x11_error);

	if (x11_error != NULL)
	{
		free(x11_error);
		return false;
	}

	// process reply to get screen sizes
	xcb_randr_screen_size_t* x11_screen_sizes =
		xcb_randr_get_screen_info_sizes(
			x11_screen_info_reply);

	if (x11_screen_sizes == NULL)
	{
		free(x11_screen_info_reply);
		return false;
	}

	int x11_screen_sizes_len =
		xcb_randr_get_screen_info_sizes_length(
			x11_screen_info_reply);

	// RandR returns multiple display information sets using an array
	// we will only save the size of the highest-density display
	double dpm_cur;
	double dpm_max = 0;
	struct dpishit_display_info* display_info = &(context->display_info);
	
	for (int i = 0; i < x11_screen_sizes_len; ++i)
	{
		dpm_cur = x11_screen_sizes[i].width / x11_screen_sizes[i].mwidth;

		if (dpm_cur > dpm_max)
		{
			dpm_max = dpm_cur;
			display_info->px_width = x11_screen_sizes[i].width;
			display_info->px_height = x11_screen_sizes[i].height;
			display_info->mm_width = x11_screen_sizes[i].mwidth;
			display_info->mm_height = x11_screen_sizes[i].mheight;
		}
	}

	free(x11_screen_info_reply);
	return true;
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

	backend->conn = window_data->conn;
	backend->window = window_data->window;
	backend->gdk_dpi_logic = 0.0;
	backend->gdk_dpi_logic_valid = false;

	// get Xft's font dpi value
	context->display_info.scale_valid =
		dpishit_xresources_xft_double(
			context,
			"Xft.scale",
			&(context->display_info.scale));

	if (context->display_info.scale_valid == false)
	{
		// the Xft value could not be relied on so we try using the environment
		char* env[3] =
		{
			"GDK_SCALE",
			"ELM_SCALE",
			"QT_SCALE_FACTOR",
		};

		context->display_info.scale_valid =
			dpishit_env_double(
				context,
				env,
				3,
				&(context->display_info.scale));
	}

	// get Xft's font scale value
	context->display_info.dpi_logic_valid =
		dpishit_xresources_xft_double(
			context,
			"Xft.dpi",
			&(context->display_info.dpi_logic));

	if (context->display_info.dpi_logic_valid == false)
	{
		// the Xft value could not be relied on so we try using the GDK variable
		char* env = "GDK_DPI_SCALE";

		context->display_info.dpi_logic_valid =
			dpishit_env_double(
				context,
				&env,
				1,
				&(backend->gdk_dpi_logic));
	}

	if (context->display_info.dpi_logic_valid == false)
	{
		// the GDK environment variable is not valid, try with the Qt variable
		char* env = "QT_FONT_DPI";

		context->display_info.dpi_logic_valid =
			dpishit_env_double(
				context,
				&env,
				1,
				&(context->display_info.dpi_logic));
	}
	else
	{
		// the GDK environment variable is valid, but since it is a density scale
		// we have to compute the actual logic density value manually
		context->display_info.dpi_logic_valid = false;
		backend->gdk_dpi_logic_valid = true;
	}

	dpishit_error_ok(error);
}

struct dpishit_display_info dpishit_x11_get(
	struct dpishit* context,
	struct dpishit_error_info* error)
{
	struct x11_backend* backend = context->backend_data;

	bool error_xcb = dpishit_refresh_real_density(context);

	if (error_xcb == false)
	{
		dpishit_error_throw(context, error, DPISHIT_ERROR_XCB_DISPLAY_INFO);
	}
	else
	{
		if (backend->gdk_dpi_logic_valid == true)
		{
			context->display_info.dpi_logic_valid = true;
			context->display_info.dpi_logic =
				backend->gdk_dpi_logic
				* context->display_info.px_width
				* 25.4
				/ context->display_info.mm_width;
		}

		dpishit_error_ok(error);
	}

	return context->display_info;
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

	free(backend);
	dpishit_error_ok(error);
}

void dpishit_prepare_init_x11(
	struct dpishit_config_backend* config)
{
	config->data = NULL;
	config->init = dpishit_x11_init;
	config->start = dpishit_x11_start;
	config->get = dpishit_x11_get;
	config->stop = dpishit_x11_stop;
	config->clean = dpishit_x11_clean;
}
