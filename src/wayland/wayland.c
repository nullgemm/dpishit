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

	// wayland structures
	backend->outputs = NULL;
	backend->output_current = NULL;
	backend->surface = NULL;

	// surface listener
	struct wl_surface_listener listener_surface =
	{
		.enter = dpishit_wayland_helpers_surface_enter,
		.leave = dpishit_wayland_helpers_surface_leave,
	};

	backend->listener_surface = listener_surface;

	// output listener
	struct wl_output_listener listener_output =
	{
		.geometry = dpishit_wayland_helpers_output_geometry,
		.mode = dpishit_wayland_helpers_output_mode,
		.done = dpishit_wayland_helpers_output_done,
		.scale = dpishit_wayland_helpers_output_scale,
	};

	backend->listener_output = listener_output;

	// display info array
	context->display_info_count = 0;
	context->display_info = NULL;

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
	backend->total_active = 0;
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

	// register output callbacks
	window_data->add_registry_handler(
		window_data->add_registry_handler_data,
		dpishit_wayland_helpers_registry_handler,
		context);

	window_data->add_registry_remover(
		window_data->add_registry_remover_data,
		dpishit_wayland_helpers_registry_remover,
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

	size_t k = dpishit_wayland_helpers_output_index(context, backend->output_current);

	if (k == SIZE_MAX)
	{
		dpishit_error_throw(context, error, DPISHIT_ERROR_BOUNDS);
		return false;
	}

	*display_info = context->display_info[k];
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

	for (size_t i = 0; i < context->display_info_count; ++i)
	{
		wl_output_destroy(backend->outputs[i].output);
	}

	if (backend->outputs != NULL)
	{
		free(backend->outputs);
	}

	free(backend);

	dpishit_error_ok(error);
}

void dpishit_set_wayland_surface(
	struct dpishit* context,
	struct wl_surface* surface,
	struct dpishit_error_info* error)
{
	struct wayland_backend* backend = context->backend_data;
	int error_posix;

	// setup wayland surface
	backend->surface = surface;

	error_posix =
		wl_surface_add_listener(
			backend->surface,
			&(backend->listener_surface),
			context);

	if (error_posix == -1)
	{
		dpishit_error_throw(
			context,
			error,
			DPISHIT_ERROR_WAYLAND_LISTENER_ADD);

		return;
	}

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
