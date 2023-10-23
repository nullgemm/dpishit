#include "include/dpishit.h"
#include "common/dpishit_private.h"
#include "common/dpishit_error.h"

#include <stdbool.h>

#if defined(DPISHIT_ERROR_LOG_MANUAL) || defined(DPISHIT_ERROR_LOG_THROW)
	#include <stdio.h>
#endif

#ifdef DPISHIT_ERROR_ABORT
	#include <stdlib.h>
#endif

void dpishit_error_init(
	struct dpishit* context)
{
#ifndef DPISHIT_ERROR_SKIP
	char** log = context->error_messages;

	log[DPISHIT_ERROR_OK] =
		"out-of-bound error message";
	log[DPISHIT_ERROR_NULL] =
		"null pointer";
	log[DPISHIT_ERROR_ALLOC] =
		"failed malloc";
	log[DPISHIT_ERROR_BOUNDS] =
		"out-of-bounds index";
	log[DPISHIT_ERROR_DOMAIN] =
		"invalid domain";
	log[DPISHIT_ERROR_FD] =
		"invalid file descriptor";

	log[DPISHIT_ERROR_X11_SCREEN_GET] =
		"could not get X11 screen resources info";
	log[DPISHIT_ERROR_X11_OUTPUT_GET] =
		"could not get X11 output info";
	log[DPISHIT_ERROR_X11_TRANSLATE] =
		"could not translate X11 window coordinates";
	log[DPISHIT_ERROR_X11_RANDR_EVENT] =
		"could not request X11 RandR configuration event";
	log[DPISHIT_ERROR_X11_RANDR_MISSING] =
		"could not find X11 RandR extension";
	log[DPISHIT_ERROR_X11_RANDR_VERSION] =
		"could not select X11 RandR extension version";

	log[DPISHIT_ERROR_WIN_MONITOR_GET] =
		"could not get win32 window monitor";
	log[DPISHIT_ERROR_WIN_MONITOR_INFO_GET] =
		"could not get win32 window monitor info";
	log[DPISHIT_ERROR_WIN_SCALE_GET] =
		"could not get win32 window monitor scale factor";
	log[DPISHIT_ERROR_WIN_ACTIVE_GET] =
		"couldn't get a win32 active window handle";
	log[DPISHIT_ERROR_WIN_DEVICE_CONTEXT_GET] =
		"couldn't get a win32 active window device context";

	log[DPISHIT_ERROR_WAYLAND_REQUEST] =
		"could not perform Wayland request";
	log[DPISHIT_ERROR_WAYLAND_LISTENER_ADD] =
		"could not add Wayland listener";
#endif
}

void dpishit_error_log(
	struct dpishit* context,
	struct dpishit_error_info* error)
{
#ifndef DPISHIT_ERROR_SKIP
	#ifdef DPISHIT_ERROR_LOG_MANUAL
		#ifdef DPISHIT_ERROR_LOG_DEBUG
			fprintf(
				stderr,
				"error in %s line %u: ",
				error->file,
				error->line);
		#endif

		if (error->code < DPISHIT_ERROR_COUNT)
		{
			fprintf(stderr, "%s\n", context->error_messages[error->code]);
		}
		else
		{
			fprintf(stderr, "%s\n", context->error_messages[0]);
		}
	#endif
#endif
}

const char* dpishit_error_get_msg(
	struct dpishit* context,
	struct dpishit_error_info* error)
{
	if (error->code < DPISHIT_ERROR_COUNT)
	{
		return context->error_messages[error->code];
	}
	else
	{
		return context->error_messages[0];
	}
}

enum dpishit_error dpishit_error_get_code(
	struct dpishit_error_info* error)
{
	return error->code;
}

const char* dpishit_error_get_file(
	struct dpishit_error_info* error)
{
	return error->file;
}

unsigned dpishit_error_get_line(
	struct dpishit_error_info* error)
{
	return error->line;
}

void dpishit_error_ok(
	struct dpishit_error_info* error)
{
	error->code = DPISHIT_ERROR_OK;
	error->file = "";
	error->line = 0;
}

void dpishit_error_throw_extra(
	struct dpishit* context,
	struct dpishit_error_info* error,
	enum dpishit_error code,
	const char* file,
	unsigned line)
{
#ifndef DPISHIT_ERROR_SKIP
	error->code = code;
	error->file = file;
	error->line = line;

	#ifdef DPISHIT_ERROR_LOG_THROW
		#ifdef DPISHIT_ERROR_LOG_DEBUG
			fprintf(
				stderr,
				"error in %s line %u: ",
				file,
				line);
		#endif

		if (error->code < DPISHIT_ERROR_COUNT)
		{
			fprintf(stderr, "%s\n", context->error_messages[error->code]);
		}
		else
		{
			fprintf(stderr, "%s\n", context->error_messages[0]);
		}
	#endif

	#ifdef DPISHIT_ERROR_ABORT
		abort();
	#endif
#endif
}
