#ifndef H_DPISHIT
#define H_DPISHIT

#include "dpishit_info.h"

#if defined(DPISHIT_X11)
	#include "dpishit_x11.h"
#elif defined(DPISHIT_WAYLAND)
	#include "dpishit_wayland.h"
#elif defined(DPISHIT_WINDOWS)
	#include "dpishit_windows.h"
#elif defined(DPISHIT_MACOS)
	#include "dpishit_macos.h"
#endif

#include <stdint.h>
#include <stdbool.h>

struct dpishit
{
#if defined(DPISHIT_X11)
	struct dpishit_data_x11 dpishit_x11;
#elif defined(DPISHIT_WAYLAND)
	struct dpishit_data_wayland dpishit_wayland;
#elif defined(DPISHIT_WINDOWS)
	struct dpishit_data_windows dpishit_windows;
#elif defined(DPISHIT_MACOS)
	struct dpishit_data_macos dpishit_macos;
#endif

	struct dpishit_display_info display_info;
};

bool dpishit_refresh_scale(
	struct dpishit* dpishit);

bool dpishit_refresh_logic_density(
	struct dpishit* dpishit);

bool dpishit_refresh_real_density(
	struct dpishit* dpishit);

void dpishit_init(
	struct dpishit* dpishit,
	void* display_system_info);

struct dpishit_display_info* dpishit_get_display_info(
	struct dpishit* dpishit);

#if defined(DPISHIT_WAYLAND)
void dpishit_wl_geometry(
	void*,
	struct wl_output*,
	int32_t,
	int32_t,
	int32_t,
	int32_t,
	int32_t,
	const char*,
	const char*,
	int32_t);

void dpishit_wl_mode(
	void*,
	struct wl_output*,
	uint32_t,
	int32_t,
	int32_t,
	int32_t);

void dpishit_wl_scale(
	void*,
	struct wl_output*,
	int32_t);
#endif

#endif
