#if defined(_WIN32)
#include <windows.h>
#else
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#endif

#include "khronos/display_egl.h"
#include "glad/egl.h"

