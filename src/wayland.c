#include "dpishit.h"
#include "dpishit_wayland.h"
#include "nix.h"

#include <pthread.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

void dpishit_wl_geometry(
	void* data,
	struct wl_output* wl_output,
	int32_t x,
	int32_t y,
	int32_t physical_width,
	int32_t physical_height,
	int32_t subpixel,
	const char* make,
	const char* model,
	int32_t output_transform)
{
	struct dpishit* dpishit = data;

	pthread_mutex_lock(&(dpishit->dpishit_wayland.info_mutex));

	dpishit->display_info.mm_width = physical_width;
	dpishit->display_info.mm_height = physical_height;

	pthread_mutex_unlock(&(dpishit->dpishit_wayland.info_mutex));
}

void dpishit_wl_mode(
	void *data,
	struct wl_output *wl_output,
	uint32_t flags,
	int32_t width,
	int32_t height,
	int32_t refresh)
{
	struct dpishit* dpishit = data;

	pthread_mutex_lock(&(dpishit->dpishit_wayland.info_mutex));

	dpishit->display_info.px_width = width;
	dpishit->display_info.px_height = height;

	pthread_mutex_unlock(&(dpishit->dpishit_wayland.info_mutex));
}

void dpishit_wl_scale(
	void* data,
	struct wl_output* wl_output,
	int32_t scale)
{
	struct dpishit* dpishit = data;

	pthread_mutex_lock(&(dpishit->dpishit_wayland.info_mutex));

	dpishit->display_info.scale = scale;

	pthread_mutex_unlock(&(dpishit->dpishit_wayland.info_mutex));
}

bool dpishit_refresh_scale(
	struct dpishit* dpishit)
{
	return true;
}

bool dpishit_refresh_logic_density(
	struct dpishit* dpishit)
{
	bool ret = false;

	char* env[2] =
	{
		"GDK_DPI_SCALE",
		"QT_FONT_DPI",
	};

	int env_scale =
		dpishit_env_double(
			dpishit,
			env,
			2,
			&(dpishit->display_info.dpi_logic));

	if (env_scale >= 0)
	{
		ret = true;
	}

	// GDK gives a scale not a dpi value, so we apply it to the physical density
	if (env_scale == 0)
	{
		dpishit->display_info.dpi_logic *= dpishit->display_info.px_width * 25.4;
		dpishit->display_info.dpi_logic /= dpishit->display_info.mm_width;
	}

	return ret;
}

bool dpishit_refresh_real_density(
	struct dpishit* dpishit)
{
	return true;
}

void dpishit_init(
	struct dpishit* dpishit,
	void* display_system_info)
{
	dpishit->dpishit_wayland =
		*((struct dpishit_data_wayland*) display_system_info);

	pthread_mutex_init(&(dpishit->dpishit_wayland.info_mutex), NULL);
	pthread_mutex_lock(&(dpishit->dpishit_wayland.info_mutex));

	*(dpishit->dpishit_wayland.output_data) = dpishit;
	dpishit->dpishit_wayland.listener->geometry = dpishit_wl_geometry;
	dpishit->dpishit_wayland.listener->mode = dpishit_wl_mode;
	dpishit->dpishit_wayland.listener->scale = dpishit_wl_scale;

	pthread_mutex_unlock(&(dpishit->dpishit_wayland.info_mutex));
}

struct dpishit_display_info* dpishit_get_display_info(
	struct dpishit* dpishit)
{
	pthread_mutex_lock(&(dpishit->dpishit_wayland.info_mutex));

	dpishit->dpishit_wayland.display_info_copy = dpishit->display_info;

	pthread_mutex_unlock(&(dpishit->dpishit_wayland.info_mutex));

	return &(dpishit->dpishit_wayland.display_info_copy);
}
