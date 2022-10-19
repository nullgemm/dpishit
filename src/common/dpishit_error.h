#ifndef H_DPISHIT_ERROR
#define H_DPISHIT_ERROR

#include "include/dpishit.h"

#define dpishit_error_throw(context, error, code) \
	dpishit_error_throw_extra(\
		context,\
		error,\
		code,\
		DPISHIT_ERROR_FILE,\
		DPISHIT_ERROR_LINE)
#define DPISHIT_ERROR_FILE __FILE__
#define DPISHIT_ERROR_LINE __LINE__

void dpishit_error_throw_extra(
	struct dpishit* context,
	struct dpishit_error_info* error,
	enum dpishit_error code,
	const char* file,
	unsigned line);

void dpishit_error_init(
	struct dpishit* context);

#endif
