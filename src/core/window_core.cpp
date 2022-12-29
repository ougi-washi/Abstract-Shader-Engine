#if defined(_WIN32)
#include <windows.h>
#else
#include <linux/fb.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
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
	bool as::start_window(as::window& window, const u16& height, const u16& width)
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
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return false;
		}
		glfwMakeContextCurrent(window.GLFW);
		glfwSetFramebufferSizeCallback(window.GLFW, framebuffer_size_callback);

		// glad: load all OpenGL function pointers
		// ---------------------------------------
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
			return false;
		}
		return true;
	}

#else

	struct fb_fix_screeninfo finfo;
	struct fb_var_screeninfo vinfo;

	bool as::start_window(as::window& window, const u16& height, const u16& width)
	{
		i32 fb_fd = open("/dev/fb0", O_RDWR);

		//Get variable screen information
		ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);

		//Get fixed screen information
		ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);

		return false;
	}

#endif

