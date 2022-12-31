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

//#include "opengl/opengl.h"
#include "window_core.h"


#if defined(_WIN32)
	// glfw: whenever the window size changed (by OS or user resize) this callback function executes
	// ---------------------------------------------------------------------------------------------
	void framebuffer_size_callback(GLFWwindow* window, int width, int height)
	{
		// make sure the viewport matches the new window dimensions; note that width and 
		// height will be significantly larger than specified on retina displays.
		glViewport(0, 0, width, height);
	}
	bool as::create_window(as::window& window, const u16& height, const u16& width)
	{
		// glfw: initialize and configure
		// ------------------------------
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
		// glfw window creation https://github.com/matusnovak/rpi-opengl-without-x
		// --------------------
		window.GLFW = glfwCreateWindow(width, height, "LearnOpenGL", NULL, NULL);
		if (window.GLFW == NULL)
		{
			AS_LOG(LV_ERROR, "Failed to create GLFW window");
			glfwTerminate();
			return false;
		}
		glfwMakeContextCurrent(window.GLFW);
		glfwSetFramebufferSizeCallback(window.GLFW, framebuffer_size_callback);

		// glad: load all OpenGL function pointers
		// ---------------------------------------
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			AS_LOG(LV_ERROR, "Failed to initialize GLAD");
			return false;
		}
		return true;
	}

#else

	inline u32 pixel_color(const u8& r, const u8& g, const u8& b, struct fb_var_screeninfo* vinfo)
	{
		return (r << vinfo->red.offset) | (g << vinfo->green.offset) | (b << vinfo->blue.offset);
	}

	bool as::create_window(as::window& window, const u16& height, const u16& width)
	{
		int fbfd = 0;
		struct fb_var_screeninfo vinfo;
		struct fb_fix_screeninfo finfo;
		long int screensize = 0;
		char* fbp = 0;
		int x = 0, y = 0;
		long int location = 0;

		// Open the file for reading and writing
		fbfd = open("/dev/fb0", O_RDWR);
		if (fbfd == -1) 
		{
			AS_LOG(LV_ERROR, "Cannot open framebuffer device");
			return false;
		}
		AS_LOG(LV_LOG, "The framebuffer device was opened successfully");

		// Get fixed screen information
		if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) 
		{
			AS_LOG(LV_ERROR, "Error reading fixed information");
			return false;
		}

		// Get variable screen information
		if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) 
		{
			AS_LOG(LV_ERROR, "Error reading variable information");
			return false;
		}


		AS_LOG(LV_LOG, "Screen data : " + std::to_string(vinfo.xres) + "x" + std::to_string(vinfo.yres) + ", " + std::to_string(vinfo.bits_per_pixel));

		// Figure out the size of the screen in bytes
		screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

		// Map the device to memory
		fbp = (char*)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fbfd, 0);
		if ((i64)fbp == -1) 
		{
			AS_LOG(LV_ERROR, "Failed to map framebuffer device to memory");
			return false;
		}
		AS_LOG(LV_LOG, "The framebuffer device was mapped to memory successfully");

		x = 100; y = 100;       // Where we are going to put the pixel

		// Figure out where in memory to put the pixel
		for (y = 100; y < 300; y++)
			for (x = 100; x < 300; x++) {

				location = (x + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) +
					(y + vinfo.yoffset) * finfo.line_length;

				if (vinfo.bits_per_pixel == 32) {
					*(fbp + location) = 100;        // Some blue
					*(fbp + location + 1) = 15 + (x - 100) / 2;     // A little green
					*(fbp + location + 2) = 200 - (y - 100) / 5;    // A lot of red
					*(fbp + location + 3) = 0;      // No transparency
					//location += 4;
				}
				else { //assume 16bpp
					int b = 10;
					int g = (x - 100) / 6;     // A little green
					int r = 31 - (y - 100) / 16;    // A lot of red
					unsigned short int t = r << 11 | g << 5 | b;
					*((unsigned short int*)(fbp + location)) = t;
				}

			}
		munmap(fbp, screensize);
		close(fbfd);
		return true;
	}

#endif

