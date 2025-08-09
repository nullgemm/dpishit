#ifndef H_DPISHIT_WAYLAND
#define H_DPISHIT_WAYLAND

#include "dpishit.h"

#include <stdbool.h>
#include <stdint.h>

struct dpishit_wayland_data
{
	// capabilities handler
	bool (*add_capabilities_handler)(
		void* data,
		void (*capabilities_handler)(
			void* data,
			void* seat,
			uint32_t capabilities),
		void* capabilities_handler_data);

	void* add_capabilities_handler_data;

	// registry handler
	bool (*add_registry_handler)(
		void* data,
		void (*registry_handler)(
			void* data,
			void* registry,
			uint32_t name,
			const char* interface,
			uint32_t version),
		void* registry_handler_data);

	void* add_registry_handler_data;

	// registry remover
	bool (*add_registry_remover)(
		void* data,
		void (*registry_remover)(
			void* data,
			void* registry,
			uint32_t name),
		void* registry_remover_data);

	void* add_registry_remover_data;

	// event callback
	void (*event_callback)(
		void* data,
		void* event);

	void* event_callback_data;
};

void dpishit_set_wayland_surface(
	struct dpishit* context,
	void* surface,
	struct dpishit_error_info* error);

void dpishit_prepare_init_wayland(
	struct dpishit_config_backend* config);

#endif
