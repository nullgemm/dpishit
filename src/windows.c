#include "dpishit.h"

#include <shellscalingapi.h>
#include <shtypes.h>
#include <stdbool.h>
#include <windows.h>

bool dpishit_refresh_scale(
	struct dpishit* dpishit)
{
	HMONITOR hmon = MonitorFromWindow(
		dpishit->dpishit_windows.hwnd,
		MONITOR_DEFAULTTONULL);
	
	if (hmon == NULL)
	{
		return false;
	}

	DEVICE_SCALE_FACTOR scale_id;

	HRESULT ret = GetScaleFactorForMonitor(
		hmon,
		&scale_id);

	if (ret != S_OK)
	{
		return false;
	}

	double scale_num;

	switch (scale_id)
	{
		case SCALE_100_PERCENT:
		{
			scale_num = 1.00;
			break;
		}
		case SCALE_120_PERCENT:
		{
			scale_num = 1.20;
			break;
		}
		case SCALE_125_PERCENT:
		{
			scale_num = 1.25;
			break;
		}
		case SCALE_140_PERCENT:
		{
			scale_num = 1.40;
			break;
		}
		case SCALE_150_PERCENT:
		{
			scale_num = 1.50;
			break;
		}
		case SCALE_160_PERCENT:
		{
			scale_num = 1.60;
			break;
		}
		case SCALE_175_PERCENT:
		{
			scale_num = 1.75;
			break;
		}
		case SCALE_180_PERCENT:
		{
			scale_num = 1.80;
			break;
		}
		case SCALE_200_PERCENT:
		{
			scale_num = 2.00;
			break;
		}
		case SCALE_225_PERCENT:
		{
			scale_num = 2.25;
			break;
		}
		case SCALE_250_PERCENT:
		{
			scale_num = 2.50;
			break;
		}
		case SCALE_300_PERCENT:
		{
			scale_num = 3.00;
			break;
		}
		case SCALE_350_PERCENT:
		{
			scale_num = 3.50;
			break;
		}
		case SCALE_400_PERCENT:
		{
			scale_num = 4.00;
			break;
		}
		case SCALE_450_PERCENT:
		{
			scale_num = 4.50;
			break;
		}
		case SCALE_500_PERCENT:
		{
			scale_num = 5.00;
			break;
		}
		case DEVICE_SCALE_FACTOR_INVALID:
		default:
		{
			return false;
		}
	}

	dpishit->display_info.scale = scale_num;

	return true;
}

bool dpishit_refresh_logic_density(
	struct dpishit* dpishit)
{
	dpishit->display_info.dpi_logic =
		GetDeviceCaps(
			dpishit->dpishit_windows.hdc,
			LOGPIXELSX);

	return true;
}

bool dpishit_refresh_real_density(
	struct dpishit* dpishit)
{
	dpishit->display_info.mm_width =
		GetDeviceCaps(
			dpishit->dpishit_windows.hdc,
			HORZSIZE);

	dpishit->display_info.mm_height =
		GetDeviceCaps(
			dpishit->dpishit_windows.hdc,
			VERTSIZE);

	dpishit->display_info.px_width =
		GetDeviceCaps(
			dpishit->dpishit_windows.hdc,
			HORZRES);

	dpishit->display_info.px_height =
		GetDeviceCaps(
			dpishit->dpishit_windows.hdc,
			VERTRES);

	return true;
}

void dpishit_init(
	struct dpishit* dpishit,
	void* display_system_info)
{
	dpishit->dpishit_windows = *((struct dpishit_data_windows*) display_system_info);
}

struct dpishit_display_info* dpishit_get_display_info(
	struct dpishit* dpishit)
{
	return &(dpishit->display_info);
}
