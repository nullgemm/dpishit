#include "dpishit.h"
#include "dpishit_macos.h"

#include <objc/message.h>
#include <objc/runtime.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// reproduce typedefs
struct cg_point
{
	double x;
	double y;
};

struct cg_size
{
	double width;
	double height;
};

struct cg_rect
{
	struct cg_point origin;
	struct cg_size size;
};

enum cg_error
{
	kcg_error_success = 0,
};

// fetch accessible symbols
extern size_t CGDisplayPixelsWide(
	uint32_t);

extern size_t CGDisplayPixelsHigh(
	uint32_t);

extern struct cg_size CGDisplayScreenSize(
	uint32_t);

extern enum cg_error CGGetActiveDisplayList(
	uint32_t,
	uint32_t*,
	uint32_t*);

extern enum cg_error CGGetDisplaysWithRect(
	struct cg_rect,
	uint32_t,
	uint32_t*,
	uint32_t*);

// declare remaining functions
double (*macos_msg_font)(id, SEL) =
	(double (*)(id, SEL)) objc_msgSend;

// dpishit
bool dpishit_refresh_scale(
	struct dpishit* dpishit)
{
	double font_size =
		macos_msg_font(
			(id) objc_getClass("NSFont"),
			sel_getUid("systemFontSize"));

	dpishit->display_info.scale = font_size / 13.0;
	
	return true;
}

bool dpishit_refresh_logic_density(
	struct dpishit* dpishit)
{
	return false;
}

bool dpishit_refresh_real_density(
	struct dpishit* dpishit)
{
	// count active displays
	enum cg_error ok;
	uint32_t display_active_count;

	ok =
		CGGetActiveDisplayList(
			0xFFFFFFFF,
			NULL,
			&display_active_count);

	if (ok != kcg_error_success)
	{
		return false;
	}

	// allocate as many id slots as active displays
	uint32_t* display_ids =
		malloc(
			display_active_count * sizeof (uint32_t));

	// get the position and size of the application window
	struct cg_rect rect;

	object_getInstanceVariable(
		dpishit->dpishit_macos.window_obj,
		"frame",
		(void*) &rect);

	// save the id of any display crossed by the application window
	uint32_t display_crossed_count;

	ok =
		CGGetDisplaysWithRect(
			rect,
			display_active_count,
			display_ids,
			&display_crossed_count);

	if (ok != kcg_error_success)
	{
		return false;
	}

	// get the size of the highest-density display
	double dpm_cur;
	double dpm_max = 0;
	struct cg_size display_size;
	struct dpishit_display_info* display_info = &(dpishit->display_info);

	for (uint32_t i = 0; i < display_crossed_count; ++i)
	{
		display_size = CGDisplayScreenSize(display_ids[i]);
		dpm_cur = display_size.width / CGDisplayPixelsWide(display_ids[i]);

		if (dpm_cur > dpm_max)
		{
			dpm_max = dpm_cur;
			display_info->px_width = CGDisplayPixelsWide(display_ids[i]);
			display_info->px_height = CGDisplayPixelsHigh(display_ids[i]);
			display_info->mm_width = display_size.width;
			display_info->mm_height = display_size.height;
		}
	}

	free(display_ids);
	return true;
}

void dpishit_init(
	struct dpishit* dpishit,
	void* display_system_info)
{
	dpishit->dpishit_macos = *((struct dpishit_data_macos*) display_system_info);
}

struct dpishit_display_info* dpishit_get_display_info(
	struct dpishit* dpishit)
{
	return &(dpishit->display_info);
}
