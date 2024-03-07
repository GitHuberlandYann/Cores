#ifndef CALLBACKS_HPP
# define CALLBACKS_HPP

void set_display_callback( Display *dis, Gui *g );
void window_size_callback( GLFWwindow *window, int width, int height );
void window_pos_callback( GLFWwindow *window, int posX, int posY );
void cursor_pos_callback( GLFWwindow *window, double posX, double posY );
void mouse_button_callback( GLFWwindow *window, int button, int action, int mods );
void gui_randomize_callback( int index );
void rm_core_callback( int index );
void gui_open_multiplayer_window_callback( int index );
void host_server_callback( int index );
void join_server_callback( int index );
void close_socket_callback( int index );
void error_callback( int error, const char *msg );

#endif
