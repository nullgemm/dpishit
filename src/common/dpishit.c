#include "include/dpishit.h"
#include "common/dpishit_private.h"

#include <stdlib.h>

struct dpishit* dpishit_init(
	struct dpishit_config_backend* config,
	struct dpishit_error_info* error)
{
	struct dpishit* context = malloc(sizeof (struct dpishit));

	if (context == NULL)
	{
		return NULL;
	}

	struct dpishit zero = {0};
	*context = zero;

	dpishit_error_init(context);

	context->backend_data = NULL;
	context->backend_callbacks = *config;
	context->backend_callbacks.init(context, error);

	context->display_info.px_width = 0;
	context->display_info.px_height = 0;
	context->display_info.mm_width = 0;
	context->display_info.mm_height = 0;
	context->display_info.dpi_logic = 0.0;
	context->display_info.scale = 0.0;
	context->display_info.dpi_logic_valid = false;
	context->display_info.scale_valid = false;

	return context;
}

void dpishit_start(
	struct dpishit* context,
	void* data,
	struct dpishit_error_info* error)
{
	context->backend_callbacks.start(context, data, error);
}

struct dpishit_display_info dpishit_get(
	struct dpishit* context,
	struct dpishit_error_info* error)
{
	return context->backend_callbacks.get(context, error);
}

void dpishit_stop(
	struct dpishit* context,
	struct dpishit_error_info* error)
{
	context->backend_callbacks.stop(context, error);
}

void dpishit_clean(
	struct dpishit* context,
	struct dpishit_error_info* error)
{
	context->backend_callbacks.clean(context, error);
	free(context);
}
