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
		// increase array size
		size_t info_count =
			context->display_info_count + 1;

		struct dpishit_display_info* info =
			realloc(
				context->display_info,
				info_count * (sizeof (struct dpishit_display_info)));

		if (info == NULL)
		{
			dpishit_error_throw(context, &error, DPISHIT_ERROR_ALLOC);
			return;
		}

		struct wayland_backend_output* outputs =
			realloc(
				backend->outputs,
				info_count * (sizeof  (struct wayland_backend_output)));

		if (outputs == NULL)
		{
			dpishit_error_throw(context, &error, DPISHIT_ERROR_ALLOC);
			return;
		}

		// save new structures and sizes
		context->display_info_count = info_count;
		context->display_info = info;
		backend->outputs = outputs;
		size_t k = info_count - 1;

		// initialize our display info slot
		context->display_info[k].x = 0;
		context->display_info[k].y = 0;
		context->display_info[k].px_width = 0;
		context->display_info[k].px_height = 0;
		context->display_info[k].mm_width = 0;
		context->display_info[k].mm_height = 0;
		context->display_info[k].dpi_logic_valid = backend->dpi_logic_valid;
		context->display_info[k].dpi_logic = backend->dpi_logic;
		context->display_info[k].dpi_scale_valid = backend->dpi_scale_valid;
		context->display_info[k].dpi_scale = backend->dpi_scale;

		// bind output / initialize our output info slot
		backend->outputs[k].done = false;
		backend->outputs[k].name = name;
		backend->outputs[k].priority = 0;

		backend->outputs[k].output =
			wl_registry_bind(
				registry,
				name,
				&wl_output_interface,
				2);

		if (backend->outputs[k].output == NULL)
		{
			dpishit_error_throw(
				context,
				&error,
				DPISHIT_ERROR_WAYLAND_REQUEST);
			return;
		}

		// add listener
		error_posix =
			wl_output_add_listener(
				backend->outputs[k].output,
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

void dpishit_wayland_helpers_registry_remover(
	void* data,
	struct wl_registry* registry,
	uint32_t name)
{
	struct dpishit* context = data;
	struct wayland_backend* backend = context->backend_data;
	struct dpishit_error_info error;
	int error_posix;

	// search for this object in the outputs
	size_t k = 0;

	while (k < context->display_info_count)
	{
		if (backend->outputs[k].name == name)
		{
			break;
		}

		++k;
	}

	if (k >= context->display_info_count)
	{
		return;
	}

	// release output
	wl_output_destroy(backend->outputs[k].output);

	// decrease array size
	context->display_info_count -= 1;

	// copy last element of the arrays in the hole
	context->display_info[k] =
		context->display_info[context->display_info_count];

	backend->outputs[k] =
		backend->outputs[context->display_info_count];

	// reallocate memory to erase last slot
	struct dpishit_display_info* info =
		realloc(
			context->display_info,
			context->display_info_count
			* (sizeof (struct dpishit_display_info)));

	if (info == NULL)
	{
		dpishit_error_throw(context, &error, DPISHIT_ERROR_ALLOC);
		return;
	}

	struct wayland_backend_output* outputs =
		realloc(
			backend->outputs,
			context->display_info_count
			* (sizeof (struct wayland_backend_output)));

	if (outputs == NULL)
	{
		dpishit_error_throw(context, &error, DPISHIT_ERROR_ALLOC);
		return;
	}

	// save new structures and sizes
	context->display_info = info;
	backend->outputs = outputs;
}

size_t dpishit_wayland_helpers_output_index(struct dpishit* context, struct wl_output* output)
{
	struct wayland_backend* backend = context->backend_data;
	size_t k = 0;

	if (output == NULL)
	{
		return SIZE_MAX;
	}

	while (k < context->display_info_count)
	{
		if (backend->outputs[k].output == output)
		{
			return k;
		}

		++k;
	}

	return SIZE_MAX;
}

void dpishit_wayland_helpers_surface_enter(
	void* data,
	struct wl_surface* surface,
	struct wl_output* output)
{
	struct dpishit* context = data;
	struct wayland_backend* backend = context->backend_data;
	struct dpishit_error_info error;

	// save the monitor we just entered as the current one
	backend->total_active += 1;
	backend->output_current = output;

	// if the current monitor was fully configured, process event
	size_t k = dpishit_wayland_helpers_output_index(context, output);

	if (k == SIZE_MAX)
	{
		dpishit_error_throw(context, &error, DPISHIT_ERROR_BOUNDS);
		return;
	}

	backend->outputs[k].priority = backend->total_active;

	if (backend->outputs[k].done == true)
	{
		backend->new_info = true;
		backend->event_callback(backend->event_callback_data, NULL);
	}
}

void dpishit_wayland_helpers_surface_leave(
	void* data,
	struct wl_surface* surface,
	struct wl_output* output)
{
	struct dpishit* context = data;
	struct wayland_backend* backend = context->backend_data;
	struct dpishit_error_info error;

	// set output priority to 0
	backend->total_active -= 1;

	size_t k = dpishit_wayland_helpers_output_index(context, output);

	if (k == SIZE_MAX)
	{
		dpishit_error_throw(context, &error, DPISHIT_ERROR_BOUNDS);
		return;
	}

	size_t priority = backend->outputs[k].priority; // save priority value

	backend->outputs[k].priority = 0;

	// also decrement all output priorities above it
	size_t max = 0;
	size_t m = 0;

	for (size_t i = 0; i < context->display_info_count; ++i)
	{
		if (backend->outputs[i].priority > priority)
		{
			backend->outputs[i].priority -= 1;
		}

		if (max < backend->outputs[i].priority) // save max priority index
		{
			max = backend->outputs[i].priority;
			m = i;
		}
	}

	// If the monitor we just leaved was the current one,
	// we update the current monitor variable.
	if (backend->output_current == output)
	{
		if (max > 0)
		{
			backend->output_current = backend->outputs[m].output;

			if (backend->outputs[m].done == true)
			{
				backend->new_info = true;
				backend->event_callback(backend->event_callback_data, NULL);
			}
		}
		else
		{
			backend->output_current = NULL;
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
	struct dpishit_error_info error;

	size_t k = dpishit_wayland_helpers_output_index(context, output);

	if (k == SIZE_MAX)
	{
		dpishit_error_throw(context, &error, DPISHIT_ERROR_BOUNDS);
		return;
	}

	context->display_info[k].x = x;
	context->display_info[k].y = y;
	context->display_info[k].mm_width = physical_width;
	context->display_info[k].mm_height = physical_height;

	if (backend->gdk_dpi_logic_valid == true)
	{
		context->display_info[k].dpi_logic_valid = true;
		context->display_info[k].dpi_logic =
			backend->gdk_dpi_logic
			* context->display_info[k].px_width
			* 25.4
			/ physical_width;
	}
	else
	{
		context->display_info[k].dpi_logic_valid = backend->dpi_logic_valid;
		context->display_info[k].dpi_logic = backend->dpi_logic;
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
	struct dpishit_error_info error;

	size_t k = dpishit_wayland_helpers_output_index(context, output);

	if (k == SIZE_MAX)
	{
		dpishit_error_throw(context, &error, DPISHIT_ERROR_BOUNDS);
		return;
	}

	context->display_info[k].px_width = width;
	context->display_info[k].px_height = height;
}

void dpishit_wayland_helpers_output_scale(
	void* data,
	struct wl_output* output,
	int32_t scale)
{
	struct dpishit* context = data;
	struct wayland_backend* backend = context->backend_data;
	struct dpishit_error_info error;

	size_t k = dpishit_wayland_helpers_output_index(context, output);

	if (k == SIZE_MAX)
	{
		dpishit_error_throw(context, &error, DPISHIT_ERROR_BOUNDS);
		return;
	}

	context->display_info[k].dpi_scale_valid = true;

	if (backend->dpi_scale_valid == true)
	{
		// keep the scale computed from environment variables if appropriate
		context->display_info[k].dpi_scale = backend->dpi_scale;
	}
	else
	{
		// otherwise use the scale supplied by the compositor
		context->display_info[k].dpi_scale = scale;
	}
}

void dpishit_wayland_helpers_output_done(
	void* data,
	struct wl_output* output)
{
	struct dpishit* context = data;
	struct wayland_backend* backend = context->backend_data;
	struct dpishit_error_info error;

	// mark monitor as fully configured
	size_t k = dpishit_wayland_helpers_output_index(context, output);

	if (k == SIZE_MAX)
	{
		dpishit_error_throw(context, &error, DPISHIT_ERROR_BOUNDS);
		return;
	}

	backend->outputs[k].done = true;

	// if the monitor we just configured is the current one, process event
	size_t m = dpishit_wayland_helpers_output_index(context, backend->output_current);

	if (k == m)
	{
		backend->new_info = true;
		backend->event_callback(backend->event_callback_data, NULL);
	}
}
