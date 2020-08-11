#include "dpishit.h"
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

	pthread_mutex_lock(&(dpishit->wl_info.wayland_info_mutex));

	dpishit->display_info.mm_width = physical_width;
	dpishit->display_info.mm_height = physical_height;

	pthread_mutex_unlock(&(dpishit->wl_info.wayland_info_mutex));
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

	pthread_mutex_lock(&(dpishit->wl_info.wayland_info_mutex));

	dpishit->display_info.px_width = width;
	dpishit->display_info.px_height = height;

	pthread_mutex_unlock(&(dpishit->wl_info.wayland_info_mutex));
}

void dpishit_wl_scale(
	void* data,
	struct wl_output* wl_output,
	int32_t scale)
{
	struct dpishit* dpishit = data;

	pthread_mutex_lock(&(dpishit->wl_info.wayland_info_mutex));

	dpishit->display_info.scale = scale;

	pthread_mutex_unlock(&(dpishit->wl_info.wayland_info_mutex));
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

	// GDK gives a scale, not a dpi value, so we apply it to the physical density
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
	dpishit->wl_info = *((struct dpishit_wayland_info*) display_system_info);
}

struct dpishit_display_info* dpishit_get_display_info(
	struct dpishit* dpishit)
{
	pthread_mutex_lock(&(dpishit->wl_info.wayland_info_mutex));

	dpishit->wl_info.display_info_copy = dpishit->display_info;

	pthread_mutex_unlock(&(dpishit->wl_info.wayland_info_mutex));

	return &(dpishit->wl_info.display_info_copy);
}
