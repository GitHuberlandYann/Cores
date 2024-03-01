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
}				t_state;

typedef struct s_core {
	// bool _visible;
	GLuint _vaos[2], _vbos[2];
	float _born_parts = 0;
	std::array<float, 2> _origin;
	float _mass; // NONE, ATTRACTION>0, REPULSION<0
	float _minTheta, _maxTheta;
	float _minSpeed, _maxSpeed, _terminalVelocity;
}				t_core;

class Display
{
	private:
		GLFWwindow *_window;
		GLuint _shaderUpdateProgram, _shaderRenderProgram;
		GLint _uniDeltaT, _uniOrigin, _uniGravity, _uniMinTheta, _uniMaxTheta, _uniMinSpeed, _uniMaxSpeed, _uniTerminalVelocity;
		GLint _uniWinPos, _uniWinSize, _uniRMinSpeed, _uniRMaxSpeed;
		GLint _winWidth, _winHeight;
		std::vector<t_particle> _particles;
		std::array<float, 30> _gravity;
		std::array<int, 2> _winPos;
		GLuint _texture;
		t_state _state;
		int _current_core;
		bool _input_released;
		std::vector<t_core> _cores;

		void setup_window( void );
		void create_shaders( void );
		void setup_communication_shaders( void );
		void load_texture( void );
		void add_core( void );
		void init_cores( int num_parts, float min_age, float max_age );

		void handleInputs( void );
		void render( double deltaTime );
		void main_loop( void );

	public:
		Display( void );
		~Display( void );

		void setWindowSize( int width, int height );
		void setWindowPos( int posX, int posY );

		void start( void );
};

#endif
