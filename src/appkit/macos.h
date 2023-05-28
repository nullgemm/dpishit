#ifndef H_DPISHIT_INTERNAL_APPKIT
#define H_DPISHIT_INTERNAL_APPKIT

#include "dpishit.h"
#include "common/dpishit_error.h"

#include <stdbool.h>

struct appkit_backend
{
	void* data;
};

void dpishit_appkit_init(
	struct dpishit* context,
	struct dpishit_error_info* error);

void dpishit_appkit_start(
	struct dpishit* context,
	void* data,
	struct dpishit_error_info* error);

bool dpishit_appkit_handle_event(
	struct dpishit* context,
	void* event,
	struct dpishit_display_info* display_info,
	struct dpishit_error_info* error);

void dpishit_appkit_stop(
	struct dpishit* context,
	struct dpishit_error_info* error);

void dpishit_appkit_clean(
	struct dpishit* context,
	struct dpishit_error_info* error);

#endif
