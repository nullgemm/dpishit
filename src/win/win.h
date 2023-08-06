#ifndef H_DPISHIT_INTERNAL_WIN
#define H_DPISHIT_INTERNAL_WIN

#include "dpishit.h"
#include "common/dpishit_error.h"

#include <stdbool.h>
#include <windows.h>

struct win_backend
{
	HWND win;
	HDC device_context;
};

void dpishit_win_init(
	struct dpishit* context,
	struct dpishit_error_info* error);

void dpishit_win_start(
	struct dpishit* context,
	void* data,
	struct dpishit_error_info* error);

bool dpishit_win_handle_event(
	struct dpishit* context,
	void* event,
	struct dpishit_display_info* display_info,
	struct dpishit_error_info* error);

void dpishit_win_stop(
	struct dpishit* context,
	struct dpishit_error_info* error);

void dpishit_win_clean(
	struct dpishit* context,
	struct dpishit_error_info* error);

#endif
