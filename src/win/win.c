#include "include/dpishit.h"
#include "common/dpishit_private.h"
#include "include/dpishit_win.h"
#include "win/win.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <shellscalingapi.h>
#include <windows.h>

static void dpishit_rect(
	struct dpishit* context,
	int* x,
	int* y,
	unsigned* width,
	unsigned* height,
	struct dpishit_error_info* error)
{
	struct win_backend* backend = context->backend_data;

	// get window handle
	HWND win = GetActiveWindow();

	if (win == NULL)
	{
		// fail silently
		dpishit_error_ok(error);
		return;
	}

	// get window main monitor
	HMONITOR monitor = MonitorFromWindow(win, MONITOR_DEFAULTTONEAREST);

	if (monitor == NULL)
	{
		dpishit_error_throw(context, error, DPISHIT_ERROR_WIN_MONITOR_GET);
		*x = 0;
		*y = 0;
		return;
	}

	// get monitor info
	MONITORINFO info =
	{
		.cbSize = sizeof (MONITORINFO),
	};

	BOOL ok = GetMonitorInfoW(monitor, &info);

	if (ok == 0)
	{
		dpishit_error_throw(context, error, DPISHIT_ERROR_WIN_MONITOR_INFO_GET);
		*x = 0;
		*y = 0;
		return;
	}

	// save monitor position
	dpishit_error_ok(error);
	*x = info.rcMonitor.left;
	*y = info.rcMonitor.top;
	*width = info.rcMonitor.right - info.rcMonitor.left;
	*height = info.rcMonitor.bottom - info.rcMonitor.top;
	return;
}

static void dpishit_scale(
	struct dpishit* context,
	double* scale,
	bool* valid,
	struct dpishit_error_info* error)
{
	struct win_backend* backend = context->backend_data;

	HWND win = GetActiveWindow();

	if (win == NULL)
	{
		dpishit_error_throw(
			context,
			error,
			DPISHIT_ERROR_WIN_ACTIVE_GET);

		return;
	}

	HMONITOR monitor = MonitorFromWindow(win, MONITOR_DEFAULTTONEAREST);

	if (monitor == NULL)
	{
		dpishit_error_throw(context, error, DPISHIT_ERROR_WIN_MONITOR_GET);
		*scale = 0.00;
		*valid = false;
		return;
	}

	DEVICE_SCALE_FACTOR scale_id;
	HRESULT result = GetScaleFactorForMonitor(monitor, &scale_id);

	if (result != S_OK)
	{
		dpishit_error_throw(context, error, DPISHIT_ERROR_WIN_SCALE_GET);
		*scale = 0.00;
		*valid = false;
		return;
	}

	switch (scale_id)
	{
		case SCALE_100_PERCENT:
		{
			*scale = 1.00;
			break;
		}
		case SCALE_120_PERCENT:
		{
			*scale = 1.20;
			break;
		}
		case SCALE_125_PERCENT:
		{
			*scale = 1.25;
			break;
		}
		case SCALE_140_PERCENT:
		{
			*scale = 1.40;
			break;
		}
		case SCALE_150_PERCENT:
		{
			*scale = 1.50;
			break;
		}
		case SCALE_160_PERCENT:
		{
			*scale = 1.60;
			break;
		}
		case SCALE_175_PERCENT:
		{
			*scale = 1.75;
			break;
		}
		case SCALE_180_PERCENT:
		{
			*scale = 1.80;
			break;
		}
		case SCALE_200_PERCENT:
		{
			*scale = 2.00;
			break;
		}
		case SCALE_225_PERCENT:
		{
			*scale = 2.25;
			break;
		}
		case SCALE_250_PERCENT:
		{
			*scale = 2.50;
			break;
		}
		case SCALE_300_PERCENT:
		{
			*scale = 3.00;
			break;
		}
		case SCALE_350_PERCENT:
		{
			*scale = 3.50;
			break;
		}
		case SCALE_400_PERCENT:
		{
			*scale = 4.00;
			break;
		}
		case SCALE_450_PERCENT:
		{
			*scale = 4.50;
			break;
		}
		case SCALE_500_PERCENT:
		{
			*scale = 5.00;
			break;
		}
		case DEVICE_SCALE_FACTOR_INVALID:
		default:
		{
			*scale = 0.00;
			break;
		}
	}

	*valid = true;
	dpishit_error_ok(error);
}

void dpishit_win_init(
	struct dpishit* context,
	struct dpishit_error_info* error)
{
	struct win_backend* backend = malloc(sizeof (struct win_backend));

	if (backend == NULL)
	{
		dpishit_error_throw(context, error, DPISHIT_ERROR_ALLOC);
		return;
	}

	struct win_backend zero = {0};
	*backend = zero;

	context->backend_data = backend;

	dpishit_error_ok(error);
}

void dpishit_win_start(
	struct dpishit* context,
	void* data,
	struct dpishit_error_info* error)
{
	struct win_backend* backend = context->backend_data;
	struct dpishit_win_data* window_data = data;

	dpishit_error_ok(error);
}

bool dpishit_win_handle_event(
	struct dpishit* context,
	void* event,
	struct dpishit_display_info* display_info,
	struct dpishit_error_info* error)
{
	struct win_backend* backend = context->backend_data;

	dpishit_rect(
		context,
		&(display_info->x),
		&(display_info->y),
		&(display_info->px_width),
		&(display_info->px_height),
		error);

	if (dpishit_error_get_code(error) != DPISHIT_ERROR_OK)
	{
		return false;
	}

	HWND win = GetActiveWindow();

	if (win == NULL)
	{
		// fail silently
		dpishit_error_ok(error);
		return false;
	}

	HDC device_context = GetDC(win);

	if (device_context == NULL)
	{
		dpishit_error_throw(context, error, DPISHIT_ERROR_WIN_DEVICE_CONTEXT_GET);
		return false;
	}

	display_info->mm_width = GetDeviceCaps(device_context, HORZSIZE);
	display_info->mm_height = GetDeviceCaps(device_context, VERTSIZE);
	display_info->dpi_logic = GetDeviceCaps(device_context, LOGPIXELSX);
	display_info->dpi_logic_valid = true;

	dpishit_scale(
		context,
		&(display_info->dpi_scale),
		&(display_info->dpi_scale_valid),
		error);

	if (dpishit_error_get_code(error) != DPISHIT_ERROR_OK)
	{
		return false;
	}

	dpishit_error_ok(error);
	return true;
}

void dpishit_win_stop(
	struct dpishit* context,
	struct dpishit_error_info* error)
{
	struct win_backend* backend = context->backend_data;

	dpishit_error_ok(error);
}

void dpishit_win_clean(
	struct dpishit* context,
	struct dpishit_error_info* error)
{
	struct win_backend* backend = context->backend_data;

	if (context->display_info != NULL)
	{
		free(context->display_info);
	}

	free(backend);

	dpishit_error_ok(error);
}

void dpishit_prepare_init_win(
	struct dpishit_config_backend* config)
{
	config->data = NULL;
	config->init = dpishit_win_init;
	config->start = dpishit_win_start;
	config->handle_event = dpishit_win_handle_event;
	config->stop = dpishit_win_stop;
	config->clean = dpishit_win_clean;
}
