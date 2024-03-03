#include "Display.hpp"

#if __linux__
# define IS_LINUX true
#else
# define IS_LINUX false
#endif

Display *display = NULL;
Gui *gui = NULL;

void set_display_callback( Display *dis, Gui *g )
{
	display = dis;
	gui = g;
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

void cursor_pos_callback( GLFWwindow *window, double posX, double posY )
{
	(void)window;
	if (gui) {
		gui->setCursorPos(posX, posY);
	}
}

void mouse_button_callback( GLFWwindow *window, int button, int action, int mods )
{
	(void)mods;
	if (gui) {
		gui->setMouseButton(window, button, action);
	}
}

void error_callback( int error, const char *msg )
{
    std::string s;
    s = " [" + std::to_string(error) + "] " + msg + '\n';
    std::cerr << s << std::endl;
}
