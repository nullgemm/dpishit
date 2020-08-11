#ifndef H_DPISHIT
#define H_DPISHIT

#include <stdint.h>
#include <stdbool.h>

// common structs

struct dpishit_display_info
{
	uint16_t px_width;
	uint16_t px_height;
	uint16_t mm_width;
	uint16_t mm_height;
	double dpi_logic;
	double scale;
};

// platform structs

#ifdef DPISHIT_WAYLAND
#include <wayland-client.h>
#include <pthread.h>

struct dpishit_wayland_info
{
	pthread_mutex_t wayland_info_mutex;
	struct dpishit_display_info display_info_copy;
};
#endif

#ifdef DPISHIT_X11
#include <xcb/xcb.h>

struct dpishit_x11_info
{
	xcb_connection_t* x11_conn;
	xcb_window_t x11_win;
};
#endif

#ifdef DPISHIT_WIN
#include <windows.h>

struct dpishit_win_info
{
	HDC win_hdc;
	HWND win_hwnd;
};
#endif

#ifdef DPISHIT_OSX
#include <objc/runtime.h>

struct dpishit_osx_info
{
	id osx_win;
};
#endif

// public API

struct dpishit
{
#ifdef DPISHIT_WAYLAND
	struct dpishit_wayland_info wl_info;
#endif

#ifdef DPISHIT_X11
	struct dpishit_x11_info x11_info;
#endif

#ifdef DPISHIT_WIN
	struct dpishit_win_info win_info;
#endif

#ifdef DPISHIT_OSX
	struct dpishit_osx_info osx_info;
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

#ifdef DPISHIT_WAYLAND
// callback
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
