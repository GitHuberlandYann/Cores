#ifndef CALLBACKS_HPP
# define CALLBACKS_HPP

void set_display_callback( Display *dis );
void window_size_callback( GLFWwindow *window, int width, int height );
void error_callback( int error, const char *msg );

#endif
