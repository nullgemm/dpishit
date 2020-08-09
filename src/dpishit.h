#ifndef H_DPISHIT
#define H_DPISHIT

#include <stdint.h>
#include <stdbool.h>

// platform structs

#ifdef DPISHIT_WAYLAND
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
#endif

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

struct dpishit
{
#ifdef DPISHIT_X11
	struct dpishit_x11_info x11_info;
#endif

#ifdef DPISHIT_WIN
	struct dpishit_win_info win_info;
#endif

	struct dpishit_display_info display_info;
};

// public API

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

#endif
