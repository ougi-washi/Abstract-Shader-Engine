#include "window_core.h"

#if defined(_WIN32)
#include <windows.h>
#else
#include <linux/fb.h>
#endif

#if defined(_WIN32)

void as::test_display() {};

#else

struct fb_fix_screeninfo finfo;
struct fb_var_screeninfo vinfo;

void as::test_display()
{
	int fb_fd = open("/dev/fb0", O_RDWR);

	//Get variable screen information
	ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);

	//Get fixed screen information
	ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);
}
#endif
