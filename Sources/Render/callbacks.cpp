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

void gui_randomize_callback( int index )
{
	if (gui) {
		gui->randomizeWindowAt(index);
	}
}

void rm_core_callback( int index )
{
	if (gui) {
		gui->rmWindow(index);
	}
	if (display) {
		display->rmCore(index);
	}
}

void host_server_callback( int index )
{
	(void)index;
	if (display) {
		display->hostServer();
	}
}

void join_server_callback( int index )
{
	(void)index;
	if (display) {
		display->joinServer();
	}
}

void gui_open_multiplayer_window_callback( int index )
{
	(void)index;
	if (gui) {
		if (gui->createWindow(-2, "Multiplayer", {20, 20}, {200, 75})) {
			gui->addButton("Host server", host_server_callback);
			gui->addButton("Join server", join_server_callback);
		}
	}
}

void error_callback( int error, const char *msg )
{
    std::string s;
    s = " [" + std::to_string(error) + "] " + msg + '\n';
    std::cerr << s << std::endl;
}
