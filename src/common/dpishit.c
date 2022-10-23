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

	context->display_info = NULL;
	context->display_info_count = 0;

	return context;
}

void dpishit_start(
	struct dpishit* context,
	void* data,
	struct dpishit_error_info* error)
{
	context->backend_callbacks.start(context, data, error);
}

bool dpishit_handle_event(
	struct dpishit* context,
	void* event,
	struct dpishit_display_info* display_info,
	struct dpishit_error_info* error)
{
	return
		context->backend_callbacks.handle_event(
			context,
			event,
			display_info,
			error);
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
