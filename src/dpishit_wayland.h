#ifndef H_DPISHIT_WAYLAND
#define H_DPISHIT_WAYLAND

#include "dpishit_info.h"

#include <pthread.h>
#include <wayland-client.h>

struct dpishit_data_wayland
{
	pthread_mutex_t info_mutex;
	struct dpishit_display_info display_info_copy;
	struct wl_output_listener* listener;
	void** output_data;
};

#endif
