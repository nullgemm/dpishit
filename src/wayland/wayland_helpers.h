#ifndef H_DPISHIT_INTERNAL_WAYLAND_HELPERS
#define H_DPISHIT_INTERNAL_WAYLAND_HELPERS

#include "dpishit.h"
#include "common/dpishit_error.h"
#include "wayland/wayland.h"

#include <stdint.h>
#include <wayland-client.h>

// registry handler
void dpishit_wayland_helpers_registry_handler(
	void* data,
	struct wl_registry* registry,
	uint32_t name,
	const char* interface,
	uint32_t version);

void dpishit_wayland_helpers_registry_remover(
	void* data,
	struct wl_registry* registry,
	uint32_t name);

size_t dpishit_wayland_helpers_output_index(
	struct dpishit* context,
	struct wl_output* output);

void dpishit_wayland_helpers_surface_enter(
	void* data,
	struct wl_surface* surface,
	struct wl_output* output);

void dpishit_wayland_helpers_surface_leave(
	void* data,
	struct wl_surface* surface,
	struct wl_output* output);

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
	int32_t output_transform);

void dpishit_wayland_helpers_output_mode(
	void* data,
	struct wl_output* output,
	uint32_t flags,
	int32_t width,
	int32_t height,
	int32_t refresh);

void dpishit_wayland_helpers_output_scale(
	void* data,
	struct wl_output* output,
	int32_t scale);

void dpishit_wayland_helpers_output_done(
	void* data,
	struct wl_output* output);

#endif
