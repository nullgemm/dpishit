#define _XOPEN_SOURCE 700
#include "include/dpishit.h"
#include "common/dpishit_private.h"
#include "include/dpishit_wayland.h"
#include "wayland/wayland.h"
#include "wayland/wayland_helpers.h"
#include "nix/nix.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// registry handler
void dpishit_wayland_helpers_registry_handler(
	void* data,
	struct wl_registry* registry,
	uint32_t name,
	const char* interface,
	uint32_t version)
{
	struct dpishit* context = data;
	struct wayland_backend* backend = context->backend_data;
	struct dpishit_error_info error;
	int error_posix;

	if (strcmp(interface, wl_output_interface.name) == 0)
	{
		backend->output =
			wl_registry_bind(
				registry,
				name,
				&wl_output_interface,
				2);

		if (backend->output == NULL)
		{
			dpishit_error_throw(
				context,
				&error,
				DPISHIT_ERROR_WAYLAND_REQUEST);
			return;
		}

		error_posix =
			wl_output_add_listener(
				backend->output,
				&(backend->listener_output),
				context);

		if (error_posix == -1)
		{
			dpishit_error_throw(
				context,
				&error,
				DPISHIT_ERROR_WAYLAND_LISTENER_ADD);

			return;
		}
	}
}

void dpishit_wayland_helpers_output_geometry(
	void* data,
	struct wl_output* output,
	int32_t x,
	int32_t y,
	int32_t physical_width,
	int32_t physical_height,
	int32_t subpixel,
	const char* make,
	const char* model,
	int32_t output_transform)
{
	struct dpishit* context = data;
	struct wayland_backend* backend = context->backend_data;

	context->display_info[0].x = x;
	context->display_info[0].y = y;
	context->display_info[0].mm_width = physical_width;
	context->display_info[0].mm_height = physical_height;

	if (backend->gdk_dpi_logic_valid == true)
	{
		context->display_info[0].dpi_logic_valid = true;
		context->display_info[0].dpi_logic =
			backend->gdk_dpi_logic
			* context->display_info[0].px_width
			* 25.4
			/ physical_width;
	}
	else
	{
		context->display_info[0].dpi_logic_valid = backend->dpi_logic_valid;
		context->display_info[0].dpi_logic = backend->dpi_logic;
	}
}

void dpishit_wayland_helpers_output_mode(
	void* data,
	struct wl_output* output,
	uint32_t flags,
	int32_t width,
	int32_t height,
	int32_t refresh)
{
	struct dpishit* context = data;
	struct wayland_backend* backend = context->backend_data;

	context->display_info[0].px_width = width;
	context->display_info[0].px_height = height;
}

void dpishit_wayland_helpers_output_scale(
	void* data,
	struct wl_output* output,
	int32_t scale)
{
	struct dpishit* context = data;
	struct wayland_backend* backend = context->backend_data;

	context->display_info[0].dpi_scale_valid = true;

	if (backend->dpi_scale_valid == true)
	{
		// keep the scale computed from environment variables if appropriate
		context->display_info[0].dpi_scale = backend->dpi_scale;
	}
	else
	{
		// otherwise use the scale supplied by the compositor
		context->display_info[0].dpi_scale = scale;
	}
}

void dpishit_wayland_helpers_output_done(
	void* data,
	struct wl_output* output)
{
	struct dpishit* context = data;
	struct wayland_backend* backend = context->backend_data;

	backend->new_info = true;
	backend->event_callback(backend->event_callback_data, NULL);
}
