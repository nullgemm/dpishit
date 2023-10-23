#ifndef H_DPISHIT_WAYLAND
#define H_DPISHIT_WAYLAND

#include "dpishit.h"

#include <stdbool.h>
#include <stdint.h>
#include <wayland-client.h>

struct dpishit_wayland_data
{
	bool (*add_capabilities_handler)(
		void* data,
		void (*capabilities_handler)(
			void* data,
			struct wl_seat* seat,
			uint32_t capabilities),
		void* capabilities_handler_data);

	void* add_capabilities_handler_data;

	bool (*add_registry_handler)(
		void* data,
		void (*registry_handler)(
			void* data,
			struct wl_registry* registry,
			uint32_t name,
			const char* interface,
			uint32_t version),
		void* registry_handler_data);

	void* add_registry_handler_data;

	void (*event_callback)(
		void* data,
		void* event);

	void* event_callback_data;
};

void dpishit_prepare_init_wayland(
	struct dpishit_config_backend* config);

#endif
