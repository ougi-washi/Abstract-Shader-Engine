#if defined(_WIN32)
#include <windows.h>
#else
#include <linux/fb.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
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
		struct fb_fix_screeninfo finfo;
		struct fb_var_screeninfo vinfo;

		i32 fb_fd = open("/dev/fb0", O_RDWR);

		// set grayscale 0 and bitperpixel to 32 (default 8)
		{
			//Get variable screen information
			ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
			vinfo.grayscale = 0;
			vinfo.bits_per_pixel = 32;
			ioctl(fb_fd, FBIOPUT_VSCREENINFO, &vinfo);
			ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
			ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);
			AS_LOG(LV_LOG, "Changed bit-per-pixel to 32");
		}

		// screen
		{

			// screen size
			i64 screensize = vinfo.yres_virtual * finfo.line_length;

			if (screensize > 0)
			{
				AS_LOG(LV_LOG, "Screen size is " + std::to_string(screensize));
			}
			else
			{

				AS_LOG(LV_WARNING, "Screen size is " + std::to_string(screensize) + " cannot create window");
				return false;	
			}
			// map pixels to memory buffer
			u8* fbp = (u8*)(mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, (off_t)0));
			if (fbp)
			{
				AS_LOG(LV_LOG, "Mapped pixels to memory buffer");
				i64 x, y; //location we want to draw the pixel
				u32 pixel = 0x505050; //The pixel we want to draw at that location

				//Make sure you set x,y and pixel correctly
				i64 location = (x + vinfo.xoffset) * (vinfo.bits_per_pixel / 8) + (y + vinfo.yoffset) * finfo.line_length;
				
				if ((fbp + location) != nullptr)
				{
					AS_LOG(LV_LOG, "Setting pixel data");
					*(fbp + location) = 0;
					//*((u32*)(fbp + location)) = pixel;
					
				}
				

				struct timespec rqtp, rmtp = { 3, 500 };
				nanosleep(&rqtp, &rmtp);
			}
			else
			{
				AS_LOG(LV_ERROR, "Failed to map the pixels to the frame buffer");
			}
			
		}

		return false;
	}

#endif

