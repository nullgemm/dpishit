#include "include/dpishit.h"
#include "common/dpishit_private.h"
#include "include/dpishit_wayland.h"
#include "wayland/wayland.h"
#include "wayland/wayland_helpers.h"
#include "nix/nix.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

void dpishit_wayland_init(
	struct dpishit* context,
	struct dpishit_error_info* error)
{
	struct wayland_backend* backend = malloc(sizeof (struct wayland_backend));

	if (backend == NULL)
	{
		dpishit_error_throw(context, error, DPISHIT_ERROR_ALLOC);
		return;
	}

	struct wayland_backend zero = {0};
	*backend = zero;

	context->backend_data = backend;

	// output listener
	struct wl_output_listener listener_output =
	{
		.geometry = dpishit_wayland_helpers_output_geometry,
		.mode = dpishit_wayland_helpers_output_mode,
		.done = dpishit_wayland_helpers_output_done,
		.scale = dpishit_wayland_helpers_output_scale,
	};

	backend->listener_output = listener_output;

	dpishit_error_ok(error);
}

void dpishit_wayland_start(
	struct dpishit* context,
	void* data,
	struct dpishit_error_info* error)
{
	struct wayland_backend* backend = context->backend_data;
	struct dpishit_wayland_data* window_data = data;

	// save event callback
	backend->event_callback = window_data->event_callback;
	backend->event_callback_data = window_data->event_callback_data;

	// initialize backend
	backend->new_info = false;
	backend->gdk_dpi_logic = 0.0;
	backend->gdk_dpi_logic_valid = false;
	backend->dpi_logic = 0.0;
	backend->dpi_logic_valid = false;
	backend->dpi_scale = 0.0;
	backend->dpi_scale_valid = false;

	// get general scale from environment variables
	char* env_scale[3] =
	{
		"GDK_SCALE",
		"ELM_SCALE",
		"QT_SCALE_FACTOR",
	};

	backend->dpi_scale_valid =
		dpishit_env_double(
			context,
			env_scale,
			3,
			&(backend->dpi_scale));

	// get dpi scale using the GDK variable
	char* env_dpi_logic_gdk = "GDK_DPI_SCALE";

	backend->dpi_logic_valid =
		dpishit_env_double(
			context,
			&env_dpi_logic_gdk,
			1,
			&(backend->gdk_dpi_logic));

	if (backend->dpi_logic_valid == false)
	{
		// the GDK environment variable is not valid, try with the Qt variable
		char* env_dpi_logic_qt = "QT_FONT_DPI";

		backend->dpi_logic_valid =
			dpishit_env_double(
				context,
				&env_dpi_logic_qt,
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

	// allocate a single display info slot for wayland
	context->display_info_count = 1;

	context->display_info =
		malloc(
			context->display_info_count
			* (sizeof (struct dpishit_display_info)));

	if (context->display_info == NULL)
	{
		dpishit_error_throw(context, error, DPISHIT_ERROR_ALLOC);
		context->display_info_count = 0;
		return;
	}

	// initialize our display info slot
	context->display_info[0].x = 0;
	context->display_info[0].y = 0;
	context->display_info[0].px_width = 0;
	context->display_info[0].px_height = 0;
	context->display_info[0].mm_width = 0;
	context->display_info[0].mm_height = 0;
	context->display_info[0].dpi_logic_valid = backend->dpi_logic_valid;
	context->display_info[0].dpi_logic = backend->dpi_logic;
	context->display_info[0].dpi_scale_valid = backend->dpi_scale_valid;
	context->display_info[0].dpi_scale = backend->dpi_scale;

	// register output callbacks
	window_data->add_registry_handler(
		window_data->add_registry_handler_data,
		dpishit_wayland_helpers_registry_handler,
		context);

	// all good
	dpishit_error_ok(error);
}

bool dpishit_wayland_handle_event(
	struct dpishit* context,
	void* event,
	struct dpishit_display_info* display_info,
	struct dpishit_error_info* error)
{
	struct wayland_backend* backend = context->backend_data;

	if ((event != NULL) || (backend->new_info == false))
	{
		return false;
	}

	*display_info = *(context->display_info);
	backend->new_info = false;

	return true;
}

void dpishit_wayland_stop(
	struct dpishit* context,
	struct dpishit_error_info* error)
{
	struct wayland_backend* backend = context->backend_data;

	dpishit_error_ok(error);
}

void dpishit_wayland_clean(
	struct dpishit* context,
	struct dpishit_error_info* error)
{
	struct wayland_backend* backend = context->backend_data;

	if (context->display_info != NULL)
	{
		free(context->display_info);
	}

	if (backend->output == NULL)
	{
		wl_output_destroy(backend->output);
	}

	free(backend);

	dpishit_error_ok(error);
}

void dpishit_prepare_init_wayland(
	struct dpishit_config_backend* config)
{
	config->data = NULL;
	config->init = dpishit_wayland_init;
	config->start = dpishit_wayland_start;
	config->handle_event = dpishit_wayland_handle_event;
	config->stop = dpishit_wayland_stop;
	config->clean = dpishit_wayland_clean;
}
