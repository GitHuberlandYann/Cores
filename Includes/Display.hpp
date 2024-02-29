#ifndef DISPLAY_HPP
# define DISPLAY_HPP

# include "utils.hpp"
# include <array>
# include <vector>

# define GOLDEN_RATIO 1.6180339887
# define WIN_HEIGHT 600
# define WIN_WIDTH WIN_HEIGHT * GOLDEN_RATIO
# define NUM_PARTS 10000

typedef struct s_particle {
	std::array<float, 2> position;
	float age;
	float life;
	std::array<float, 2> velocity;
}			t_particle;

typedef struct s_state {
	int read = 0;
	int write = 1;
	float born_parts = 0;
}				t_state;

class Display
{
	private:
		GLFWwindow *_window;
		GLuint _vaos[4], _vbos[2], _shaderUpdateProgram, _shaderRenderProgram;
		GLint _uniDeltaT, _uniOrigin, _uniMinTheta, _uniMaxTheta, _uniMinSpeed, _uniMaxSpeed;
		GLint _winWidth, _winHeight;
		GLuint _texture;
		std::array<float, 2> _origin;
		t_state _state;

		void setup_window( void );
		void create_shaders( void );
		void setup_communication_shaders( void );
		void load_texture( void );
		void init_particles( int num_parts, float min_age, float max_age );

		void render( double deltaTime );
		void main_loop( void );

	public:
		Display( void );
		~Display( void );

		void setWindowSize( int width, int height );

		void start( void );
};

#endif
