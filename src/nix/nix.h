#ifndef H_DPISHIT_INTERNAL_NIX
#define H_DPISHIT_INTERNAL_NIX

#include <stdbool.h>

bool dpishit_env_double(
	struct dpishit* context,
	char** env,
	int count,
	double* out);

#endif
