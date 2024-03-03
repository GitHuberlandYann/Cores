#ifndef GUI_HPP
# define GUI_HPP

# include "Text.hpp"

namespace CONTAINER
{
	enum {
		TEXT,			// quite explicit
		BUTTON,			// calls ptr_function when pressed
		SLIDER_FLOAT,	// mod ptr to float linearly within given range
		ENUM,			// mot ptr to int on each press
		COLOR
	};
}

typedef struct s_container {
	int type = CONTAINER::TEXT;
	std::string name = "";
	void (*foo_ptr)( void ) = NULL;
	float *value = NULL;
	float range_start = 0.0f, range_end = 1.0f;
	int *enu = NULL;
	std::vector<std::string> enu_list = {};
}				t_container;

const int title_height = 20;
const int min_win_width = 120;
const int min_win_height = 40;
typedef struct s_window {
	std::string title;
	std::array<int, 2> pos, size, offset = {0, 0};
	int selection = -1;
	std::vector<t_container> content = {};
}			t_window;

class Gui
{
	private:
		GLuint _shaderProgram;
		int _selection, _mouse_button;
		bool _moving_window, _resize_window, _moving_slider, _closing_window;
		std::vector<t_window> _content;
		GLFWcursor *_cursor;
		Text *_text;

		void setCursorPosWindow( t_window &win, int posX, int posY );
		void renderWindow( t_window &win );

	public:
		Gui( void );
		~Gui( void );

		void setWindowSize( int width, int height );
		void setCursorPos( double posX, double posY );
		void setMouseButton( GLFWwindow *window, int button, int action );
		bool mouseControl( void );

		void start( void );
		void render( void );

		void writeText( int posX, int posY, int font_size, int color, std::string str );
		bool createWindow( std::string title, std::array<int, 2> pos = {20, 20}, std::array<int, 2> size = {200, 300} );
		void addSliderFloat( std::string name, float *ptr, float minRange = 0.0f, float maxRange = 1.0f );
};

#endif
