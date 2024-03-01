#ifndef CALLBACKS_HPP
# define CALLBACKS_HPP

void set_display_callback( Display *dis );
void window_size_callback( GLFWwindow *window, int width, int height );
void window_pos_callback( GLFWwindow *window, int posX, int posY );
void error_callback( int error, const char *msg );

#endif
