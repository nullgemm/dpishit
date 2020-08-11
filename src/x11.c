#include "dpishit.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <xcb/randr.h>

#if !defined(DPISHIT_NO_HACK_X11_SCALE_XFT) || !defined(DPISHIT_NO_HACK_X11_LOGIC_DPI_XFT)
	#include <xcb/xcb_xrm.h>

	static bool dpishit_xresources_xft_double(
		struct dpishit* dpishit,
		const char* prop,
		double* out)
	{
		xcb_xrm_database_t* x11_xrm_db =
			xcb_xrm_database_from_default(
				dpishit->x11_info.x11_conn);

		if (x11_xrm_db == NULL)
		{
			return false;
		}

		char* double_str = NULL;

		// find Xft.dpi, the font dpi setting for Keith Packard's Xft...
		xcb_xrm_resource_get_string(
			x11_xrm_db,
			prop,
			NULL,
			&double_str);

		if (double_str == NULL)
		{
			xcb_xrm_database_free(x11_xrm_db);
			return false;
		}

		// according to the documentation, Xft.dpi and Xft.scale are `double`s...
		char* endptr;
		double double_num = strtod(double_str, &endptr);

		if ((*endptr != '\0') || (endptr == double_str))
		{
			xcb_xrm_database_free(x11_xrm_db);
			free(double_str);
			return false;
		}

		// let's not throw theoretical precision away...
		*out = double_num;

		xcb_xrm_database_free(x11_xrm_db);
		free(double_str);
		return true;
	}
#endif

#if !defined(DPISHIT_NO_HACK_X11_LOGIC_DPI_ENV) || !defined(DPISHIT_NO_HACK_X11_SCALE_ENV)
	static int dpishit_env_double(
		struct dpishit* dpishit,
		char** env,
		int count,
		double* out)
	{
		char* scale_str;
		int i = 0;

		// try all given environment variables until one is valid
		do
		{
			scale_str = getenv(env[i]);

			if (scale_str != NULL)
			{
				break;
			}

			++i;
		}
		while (i < count);
		
		// no environment variable was found
		if (scale_str == NULL)
		{
			return -1;
		}

		// parse environment variable
		char* endptr;
		double scale_num = strtod(scale_str, &endptr);

		if ((*endptr != '\0') || (endptr == scale_str))
		{
			return -1;
		}

		*out = scale_num;

		return i;
	}
#endif

bool dpishit_refresh_scale(
	struct dpishit* dpishit)
{
	bool ret = true;

#ifndef DPISHIT_NO_HACK_X11_SCALE_XFT
	ret =
		dpishit_xresources_xft_double(
			dpishit,
			"Xft.scale",
			&(dpishit->display_info.scale));

	if (ret == true)
	{
		return true;
	}
#endif

#ifndef DPISHIT_NO_HACK_X11_SCALE_ENV
	char* env[3] =
	{
		"GDK_SCALE",
		"ELM_SCALE",
		"QT_SCALE_FACTOR",
	};

	ret =
		dpishit_env_double(
			dpishit,
			env,
			3,
			&(dpishit->display_info.scale)) >= 0;
#endif

	return ret;
}

bool dpishit_refresh_logic_density(
	struct dpishit* dpishit)
{
	bool ret = true;

#ifndef DPISHIT_NO_HACK_X11_LOGIC_DPI_XFT
	ret =
		dpishit_xresources_xft_double(
			dpishit,
			"Xft.dpi",
			&(dpishit->display_info.dpi_logic));

	if (ret == true)
	{
		return true;
	}
#endif

#ifndef DPISHIT_NO_HACK_X11_LOGIC_DPI_ENV
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
#endif

	return ret;
}

bool dpishit_refresh_real_density(
	struct dpishit* dpishit)
{
	// init
	xcb_connection_t* x11_conn = dpishit->x11_info.x11_conn;
	xcb_window_t x11_win = dpishit->x11_info.x11_win;
	xcb_generic_error_t* x11_error;

	// send request and get reply
	xcb_randr_get_screen_info_cookie_t x11_screen_info_cookie =
		xcb_randr_get_screen_info(
			x11_conn,
			x11_win);

	xcb_randr_get_screen_info_reply_t* x11_screen_info_reply =
		xcb_randr_get_screen_info_reply(
			x11_conn,
			x11_screen_info_cookie,
			&x11_error);

	if (x11_error != NULL)
	{
		free(x11_error);
		return false;
	}

	// process reply to get screen sizes
	xcb_randr_screen_size_t* x11_screen_sizes =
		xcb_randr_get_screen_info_sizes(
			x11_screen_info_reply);

	if (x11_screen_sizes == NULL)
	{
		free(x11_screen_info_reply);
		return false;
	}

	int x11_screen_sizes_len =
		xcb_randr_get_screen_info_sizes_length(
			x11_screen_info_reply);

	// RandR returns multiple display information sets using an array
	// we will only save the size of the highest-density display
	double dpm_cur;
	double dpm_max = 0;
	struct dpishit_display_info* display_info = &(dpishit->display_info);
	
	for (int i = 0; i < x11_screen_sizes_len; ++i)
	{
		dpm_cur = x11_screen_sizes[i].width / x11_screen_sizes[i].mwidth;

		if (dpm_cur > dpm_max)
		{
			dpm_max = dpm_cur;
			display_info->px_width = x11_screen_sizes[i].width;
			display_info->px_height = x11_screen_sizes[i].height;
			display_info->mm_width = x11_screen_sizes[i].mwidth;
			display_info->mm_height = x11_screen_sizes[i].mheight;
		}
	}

	free(x11_screen_info_reply);
	return true;
}

void dpishit_init(
	struct dpishit* dpishit,
	void* display_system_info)
{
	dpishit->x11_info = *((struct dpishit_x11_info*) display_system_info);
}

struct dpishit_display_info* dpishit_get_display_info(
	struct dpishit* dpishit)
{
	return &(dpishit->display_info);
}
