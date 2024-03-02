#ifndef CALLBACKS_HPP
# define CALLBACKS_HPP

void set_display_callback( Display *dis, Gui *g );
void window_size_callback( GLFWwindow *window, int width, int height );
void window_pos_callback( GLFWwindow *window, int posX, int posY );
void cursor_pos_callback( GLFWwindow *window, double posX, double posY );
void mouse_button_callback( GLFWwindow *window, int button, int action, int mods );
void error_callback( int error, const char *msg );

#endif
