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

#include "window_core.h"

bool as::create_display_handle(const bool& is_raw_display, const u16& width, const u16& height, as::display_handle& display_handle)
{
	display_handle.is_raw_display = is_raw_display;
	display_handle.width = width;
	display_handle.height = height;

	if (display_handle.is_raw_display)
	{	
		bool creation_result = as::create_raw_display(display_handle.raw_display);
		AS_ASSERT(creation_result, "Could not create raw display handle");
		return creation_result;
	}
	else
	{
		bool creation_result = create_window(display_handle.GLFW, width, height);
		AS_ASSERT(creation_result, "Could not create window handle");
		return creation_result;
	}
	return false;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, i32 width, i32 height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

bool as::create_window(GLFWwindow*& window, const u16& width, const u16& height)
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	// glfw window creation https://github.com/matusnovak/rpi-opengl-without-x
	// --------------------
	window = glfwCreateWindow(width, height, "Abstract Shader Engine", NULL, NULL);
	if (window == NULL)
	{
		AS_LOG(LV_ERROR, "Failed to create GLFW window");
		glfwTerminate();
		return false;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		AS_LOG(LV_ERROR, "Failed to initialize GLAD");
		return false;
	}
	return true;
}

#if defined(_WIN32)
bool as::create_raw_display(as::raw_display& raw_display) 
{ 
	AS_LOG(LV_ERROR, "Failed to create raw display, windows does not support this functionality");
	return false;
};

#else
bool as::create_raw_display(as::raw_display& raw_display)
{
	i32 x = 0, y = 0;
	i64 location = 0;

	// Open the file for reading and writing
	raw_display.framebuffer_device = open("/dev/fb0", O_RDWR);
	if (raw_display.framebuffer_device == -1)
	{
		AS_LOG(LV_ERROR, "Cannot open framebuffer device");
		return false;
	}
	AS_LOG(LV_LOG, "The framebuffer device was opened successfully");

	// Get fixed screen information
	if (ioctl(raw_display.framebuffer_device, FBIOGET_FSCREENINFO, &raw_display.finfo) == -1)
	{
		AS_LOG(LV_ERROR, "Error reading fixed information");
		return false;
	}

	// Get variable screen information
	if (ioctl(raw_display.framebuffer_device, FBIOGET_VSCREENINFO, &raw_display.vinfo) == -1)
	{
		AS_LOG(LV_ERROR, "Error reading variable information");
		return false;
	}

	AS_LOG(LV_LOG, "Screen data : " + std::to_string(raw_display.vinfo.xres) + "x" + std::to_string(raw_display.vinfo.yres) + ", " + std::to_string(raw_display.vinfo.bits_per_pixel));

	// Figure out the size of the screen in bytes
	raw_display.screen_size = raw_display.vinfo.xres * raw_display.vinfo.yres * raw_display.vinfo.bits_per_pixel / 8;

	// Map the device to memory
	raw_display.framebuffer_ptr = (char*)mmap(0, raw_display.screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, raw_display.framebuffer_device, 0);
	if ((i64)raw_display.framebuffer_ptr == -1)
	{
		AS_LOG(LV_ERROR, "Failed to map framebuffer device to memory");
		return false;
	}
	AS_LOG(LV_LOG, "The framebuffer device was mapped to memory successfully");

	
	munmap(raw_display.framebuffer_ptr, raw_display.screen_size);
	close(raw_display.framebuffer_device);
	return true;
}

#endif

bool as::should_display_loop(const as::display_handle& display_handle)
{
	if (display_handle.is_raw_display)
	{
		return display_handle.raw_display;
	}
	else
	{
		return display_handle.GLFW && !glfwWindowShouldClose(display_handle.GLFW);
	}
	return false;
}

bool as::process_input(const as::display_handle& display_handle)
{
	if (display_handle.GLFW)
	{
		if (glfwGetKey(display_handle.GLFW, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(display_handle.GLFW, true);
		}
		return true;
	}
	else
	{
		
	}
	return false;
}

bool as::draw(as::display_handle& display_handle)
{
	if (display_handle.is_raw_display)
	{
		// Create a PBO
		GLuint pbo;
		glGenBuffers(1, &pbo);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, pbo);

		// Allocate storage for the PBO
		glBufferData(GL_PIXEL_PACK_BUFFER, display_handle.width * display_handle.height * 4, NULL, GL_STREAM_READ);

		// Read the pixels from the framebuffer into the PBO
		glReadPixels(0, 0, display_handle.width, display_handle.height, GL_RGBA, GL_UNSIGNED_BYTE, 0);

		// Map the PBO to a pointer so we can access the pixel data
		u8* pixels = (u8*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
		draw_on_raw_display(display_handle.raw_display, pixels);

		// Unmap the PBO
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER);

		// Clean up
		glDeleteBuffers(1, &pbo);
		return true;
	}
	else
	{
		return true;
	}
}

bool as::draw_on_raw_display(as::raw_display& raw_display, const u8* pixels)
{

	//const int width = 640;
	//const int height = 480;



	//// Do something with the pixel data...
	//const int width = 640;
	//const int height = 480;

	//for (int y = 0; y < height; y++) {
	//	for (int x = 0; x < width; x++) {
	//		// Calculate the index of the current pixel
	//		int index = (y * width + x) * 4;

	//		// Get the red, green, and blue values of the pixel
	//		GLubyte red = pixels[index];
	//		GLubyte green = pixels[index + 1];
	//		GLubyte blue = pixels[index + 2];

	//		// Print the values

	//	}
	//}


	u16 x = 100; 
	u16 y = 100;       // Where we are going to put the pixel
	u16 location;
	// Figure out where in memory to put the pixel
	for (y = 100; y < 300; y++)
		for (x = 100; x < 300; x++)
		{
#ifdef __linux__

			location = (x + raw_display.vinfo.xoffset) * (raw_display.vinfo.bits_per_pixel / 8) +
				(y + raw_display.vinfo.yoffset) * raw_display.finfo.line_length;

			if (raw_display.vinfo.bits_per_pixel == 32) {
				*(raw_display.framebuffer_ptr + location) = 100;        // Some blue
				*(raw_display.framebuffer_ptr + location + 1) = 15 + (x - 100) / 2;     // A little green
				*(raw_display.framebuffer_ptr + location + 2) = 200 - (y - 100) / 5;    // A lot of red
				*(raw_display.framebuffer_ptr + location + 3) = 0;      // No transparency
				//location += 4;
			}
			else { //assume 16bpp
				i32 b = 10;
				i32 g = (x - 100) / 6;     // A little green
				i32 r = 31 - (y - 100) / 16;    // A lot of red
				u16 t = r << 11 | g << 5 | b;
				*((u16*)(raw_display.framebuffer_ptr + location)) = t;
			}
#endif
		}

	return true;
}