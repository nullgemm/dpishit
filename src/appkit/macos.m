#define _XOPEN_SOURCE 700

#include "include/dpishit.h"
#include "common/dpishit_private.h"
#include "include/dpishit_appkit.h"
#include "appkit/macos.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#import <AppKit/AppKit.h>

void dpishit_appkit_init(
	struct dpishit* context,
	struct dpishit_error_info* error)
{
	struct appkit_backend* backend = malloc(sizeof (struct appkit_backend));

	if (backend == NULL)
	{
		dpishit_error_throw(context, error, DPISHIT_ERROR_ALLOC);
		return;
	}

	struct appkit_backend zero = {0};
	*backend = zero;

	context->backend_data = backend;

	dpishit_error_ok(error);
}

void dpishit_appkit_start(
	struct dpishit* context,
	void* data,
	struct dpishit_error_info* error)
{
	dpishit_error_ok(error);
}

bool dpishit_appkit_handle_event(
	struct dpishit* context,
	void* event,
	struct dpishit_display_info* display_info,
	struct dpishit_error_info* error)
{
	struct appkit_backend* backend = context->backend_data;

	NSEvent* nsevent = (NSEvent*) event;
	NSEventType type = [nsevent type];

	if (type != NSEventTypeAppKitDefined)
	{
		return false;
	}

	// check if the window was moved
	NSEventSubtype subtype = [nsevent subtype];

	if ((subtype != NSEventSubtypeScreenChanged)
	&& (subtype != NSEventSubtypeWindowMoved))
	{
		return false;
	}

	NSWindow* win = [nsevent window];
	NSScreen* screen = [win screen];
	NSDictionary* dictionary = [screen deviceDescription];
	NSNumber* display_id_number = dictionary[@"NSScreenNumber"];
	CGDirectDisplayID display_id = [display_id_number unsignedIntValue];

	CGSize display_size = CGDisplayScreenSize(display_id);
	NSRect frame = [screen frame];

	display_info->x = NSMinX(frame);
	display_info->y = NSMinY(frame);
	display_info->px_width = CGDisplayPixelsWide(display_id);
	display_info->px_height = CGDisplayPixelsHigh(display_id);
	display_info->mm_width = display_size.width;
	display_info->mm_height = display_size.height;
	display_info->dpi_logic = 0.0;
	display_info->dpi_logic_valid = false;
	display_info->dpi_scale = [NSFont systemFontSize] / 13.0;
	display_info->dpi_scale_valid = true;

	return true;
}

void dpishit_appkit_stop(
	struct dpishit* context,
	struct dpishit_error_info* error)
{
	struct appkit_backend* backend = context->backend_data;

	dpishit_error_ok(error);
}

void dpishit_appkit_clean(
	struct dpishit* context,
	struct dpishit_error_info* error)
{
	struct appkit_backend* backend = context->backend_data;

	if (context->display_info != NULL)
	{
		free(context->display_info);
	}

	free(backend);

	dpishit_error_ok(error);
}

void dpishit_prepare_init_appkit(
	struct dpishit_config_backend* config)
{
	config->data = NULL;
	config->init = dpishit_appkit_init;
	config->start = dpishit_appkit_start;
	config->handle_event = dpishit_appkit_handle_event;
	config->stop = dpishit_appkit_stop;
	config->clean = dpishit_appkit_clean;
}
