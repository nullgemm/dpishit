#include "dpishit.h"
#include "nix.h"
#include <stdlib.h>

#if !defined(DPISHIT_NO_HACK_LOGIC_DPI_ENV) || !defined(DPISHIT_NO_HACK_SCALE_ENV)
int dpishit_env_double(
	struct dpishit* dpishit,
	char** env,
	int count,
	double* out)
{
	char* scale_str;
	int i = 0;

	// try all given environment variables until one is valid
	do
	{
		scale_str = getenv(env[i]);

		if (scale_str != NULL)
		{
			break;
		}

		++i;
	}
	while (i < count);
	
	// no environment variable was found
	if (scale_str == NULL)
	{
		return -1;
	}

	// parse environment variable
	char* endptr;
	double scale_num = strtod(scale_str, &endptr);

	if ((*endptr != '\0') || (endptr == scale_str))
	{
		return -1;
	}

	*out = scale_num;

	return i;
}
#endif
