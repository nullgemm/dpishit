#ifndef H_DPISHIT_INTERNAL_WAYLAND
#define H_DPISHIT_INTERNAL_WAYLAND

#include "dpishit.h"
#include "common/dpishit_error.h"

#include <stdbool.h>
#include <stdint.h>
#include <wayland-client.h>

struct wayland_backend_output
{
	bool done;
	uint32_t name;
	size_t priority;
	struct wl_output* output;
};

struct wayland_backend
{
	// saved info
	bool new_info;
	size_t total_active;
	double gdk_dpi_logic;
	bool gdk_dpi_logic_valid;
	double dpi_logic;
	bool dpi_logic_valid;
	double dpi_scale;
	bool dpi_scale_valid;

	// wayland data
	struct wayland_backend_output* outputs;
	struct wl_output* output_current;
	struct wl_surface* surface;

	// listeners
	struct wl_surface_listener listener_surface;
	struct wl_output_listener listener_output;

	// event callback
	void (*event_callback)(
		void* data,
		void* event);

	void* event_callback_data;
};

void dpishit_wayland_init(
	struct dpishit* context,
	struct dpishit_error_info* error);

void dpishit_wayland_start(
	struct dpishit* context,
	void* data,
	struct dpishit_error_info* error);

bool dpishit_wayland_handle_event(
	struct dpishit* context,
	void* event,
	struct dpishit_display_info* display_info,
	struct dpishit_error_info* error);

void dpishit_wayland_stop(
	struct dpishit* context,
	struct dpishit_error_info* error);

void dpishit_wayland_clean(
	struct dpishit* context,
	struct dpishit_error_info* error);

#endif
