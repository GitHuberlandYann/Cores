#ifndef DISPLAY_HPP
# define DISPLAY_HPP

# include "Gui.hpp"
# include "Socket.hpp"

# define GOLDEN_RATIO 1.6180339887
# define WIN_HEIGHT 600
# define WIN_WIDTH WIN_HEIGHT * GOLDEN_RATIO
# define NUM_PARTS 10000
# define TICK 0.05

namespace POLARITY
{
	enum {
		ATTRACTION,
		REPULSION
	};
}

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
	GLuint _vaos[2], _vbos[2];
	std::string _core_name = "";
	bool _destroyed = true, _visible = false, _freezed = false;
	float _born_parts = 0;
	int _num_parts;
	float _minTheta, _maxTheta;
	float _minSpeed, _maxSpeed, _terminalVelocity;
	std::array<float, 4> _birthCol = {1.0f, 0.0f, 0.0f, 1.0f}, _deathCol = {0.0f, 0.0f, 1.0f, 0.0f};
	std::array<float, 3> _speedCol = {0.5f, 1.0f, 0.5f};
	int _birthSize = 5, _deathSize = 0;
	float _lifeSpan = 5.01f, _lifeRange = 5.14f;
}				t_core;
// used for UDP packets
// we don't send _vaos and _vbos so we substract 16 bytes
// we send pos[2], mass, and polarity so we add 16 bytes
// we don't send _core_name so we substract sizeof(std::string) (should be 24)
// we send core name as a 10 bytes char * so we add 10
const int CORE_PACKET_SIZE = sizeof(t_core) - sizeof(std::string) + 10;

class Display
{
	private:
		GLFWwindow *_window;
		GLuint _shaderUpdateProgram, _shaderRenderProgram;
		GLint _uniDeltaT, _uniOrigin, _uniGravity, _uniPolarity, _uniMinTheta, _uniMaxTheta, _uniMinSpeed, _uniMaxSpeed, _uniTerminalVelocity, _uniLifeSpan, _uniLifeRange;
		GLint _uniWinPos, _uniWinSize, _uniRMinSpeed, _uniRMaxSpeed, _uniBirthSize, _uniDeathSize, _uniBirthColor, _uniDeathColor, _uniSpeedColor, _uniRLifeSpan, _uniRLifeRange;
		GLint _winWidth, _winHeight;
		std::vector<t_particle> _particles;
		std::array<float, 30> _gravity;
		std::array<int, 10> _polarity;
		std::array<float, 3> _backCol = {0.0f, 0.0f, 0.0f};
		std::array<int, 2> _winPos;
		GLuint _texture;
		t_state _state;
		int _current_core, _nb_cores, _multi_id, _fps, _tps;
		float _deltaTime, _nb_parts;
		bool _input_released;
		std::array<t_core, 9> _cores;
		Gui *_gui;
		std::string _server_ip = "localhost", _server_port = std::to_string(DEFAULT_PORT);
		Socket *_socket;

		void setup_window( void );
		void create_shaders( void );
		void setup_communication_shaders( void );
		void load_texture( void );
		void add_core( int index );
		void init_cores( int num_parts, float min_age, float age_range );

		void updateCore( int index, void *data );
		void updateCores( void *data );
		void updateGameState( void );

		void handleInputs( void );
		void handleMultiInputs( void );
		void render( void );
		void main_loop( void );

	public:
		Display( void );
		~Display( void );

		void setWindowSize( int width, int height );
		void setWindowPos( int posX, int posY );
		void rmCore( int index );

		void openMultiplayerWindow( void );
		void hostServer( void );
		void joinServer( void );
		void closeSocket( void );

		void start( void );
};

#endif
