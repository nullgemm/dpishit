#ifndef H_DPISHIT_INFO
#define H_DPISHIT_INFO

#include <stdint.h>

struct dpishit_display_info
{
	uint16_t px_width;
	uint16_t px_height;
	uint16_t mm_width;
	uint16_t mm_height;
	double dpi_logic;
	double scale;
};

#endif
