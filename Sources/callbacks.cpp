#include "Display.hpp"

#if __linux__
# define IS_LINUX true
#else
# define IS_LINUX false
#endif

Display *display = NULL;

void set_display_callback( Display *dis )
{
	display = dis;
}

void window_size_callback( GLFWwindow *window, int width, int height )
{
	(void)window;
	// std::cout << "window resized to " << width << ", " << height << std::endl;
	if (display) {
		display->setWindowSize(width, height);
		if (IS_LINUX) {
			glViewport(0, 0, width, height);
		}
	}
}


void window_pos_callback( GLFWwindow *window, int posX, int posY )
{
	(void)window;
	// std::cout << "window pos set to " << posX << ", " << posY << std::endl;
	if (display) {
		display->setWindowPos(posX, posY);
	}
}

void error_callback( int error, const char *msg )
{
    std::string s;
    s = " [" + std::to_string(error) + "] " + msg + '\n';
    std::cerr << s << std::endl;
}
