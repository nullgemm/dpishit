#ifndef H_DPISHIT
#define H_DPISHIT

#include <stdbool.h>

struct dpishit;

enum dpishit_error
{
	DPISHIT_ERROR_OK = 0,
	DPISHIT_ERROR_NULL,
	DPISHIT_ERROR_ALLOC,
	DPISHIT_ERROR_BOUNDS,
	DPISHIT_ERROR_DOMAIN,
	DPISHIT_ERROR_FD,

	DPISHIT_ERROR_X11_SCREEN_GET,
	DPISHIT_ERROR_X11_OUTPUT_GET,
	DPISHIT_ERROR_X11_CRTC_GET,

	DPISHIT_ERROR_COUNT,
};

struct dpishit_error_info
{
	enum dpishit_error code;
	const char* file;
	unsigned line;
};

struct dpishit_display_info
{
	int x;
	int y;
	unsigned px_width;
	unsigned px_height;
	unsigned mm_width;
	unsigned mm_height;
	double dpi_logic;
	bool dpi_logic_valid;
	double dpi_scale;
	bool dpi_scale_valid;
};

struct dpishit_config_backend
{
	void* data;

	void (*init)(
		struct dpishit* context,
		struct dpishit_error_info* error);

	void (*start)(
		struct dpishit* context,
		void* data,
		struct dpishit_error_info* error);

	bool (*handle_event)(
		struct dpishit* context,
		void* event,
		struct dpishit_display_info* display_info,
		struct dpishit_error_info* error);

	void (*stop)(
		struct dpishit* context,
		struct dpishit_error_info* error);

	void (*clean)(
		struct dpishit* context,
		struct dpishit_error_info* error);
};

struct dpishit* dpishit_init(
	struct dpishit_config_backend* config,
	struct dpishit_error_info* error);

void dpishit_start(
	struct dpishit* context,
	void* data,
	struct dpishit_error_info* error);

bool dpishit_handle_event(
	struct dpishit* context,
	void* event,
	struct dpishit_display_info* display_info,
	struct dpishit_error_info* error);

void dpishit_stop(
	struct dpishit* context,
	struct dpishit_error_info* error);

void dpishit_clean(
	struct dpishit* context,
	struct dpishit_error_info* error);

void dpishit_error_log(
	struct dpishit* context,
	struct dpishit_error_info* error);

const char* dpishit_error_get_msg(
	struct dpishit* context,
	struct dpishit_error_info* error);

enum dpishit_error dpishit_error_get_code(
	struct dpishit_error_info* error);

const char* dpishit_error_get_file(
	struct dpishit_error_info* error);

unsigned dpishit_error_get_line(
	struct dpishit_error_info* error);

void dpishit_error_ok(
	struct dpishit_error_info* error);

#endif
