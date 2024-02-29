#ifndef DISPLAY_HPP
# define DISPLAY_HPP

# include "utils.hpp"

# define GOLDEN_RATIO 1.6180339887
# define WIN_HEIGHT 600
# define WIN_WIDTH WIN_HEIGHT * GOLDEN_RATIO

class Display
{
	private:
		GLFWwindow *_window;
		GLuint _vao, _vbo, _shaderProgram;
		GLuint _texture;

		void setup_window( void );
		void create_shaders( void );
		void setup_communication_shaders( void );
		void load_texture( void );

		void main_loop( void );

	public:
		Display( void );
		~Display( void );

		void start( void );
};

#endif
