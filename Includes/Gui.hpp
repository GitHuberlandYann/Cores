#ifndef GUI_HPP
# define GUI_HPP

# include "Text.hpp"

namespace CONTAINER
{
	enum {
		TEXT,			// quite explicit
		BUTTON,			// calls ptr_function when pressed
		SLIDER_INT,		// mod ptr to int linearly within given range
		SLIDER_FLOAT,	// mod ptr to float linearly within given range
		ENUM,			// (call fct | mod ptr to int) on each press
		COLOR, 			// mod RBBA ptrs in float range [0-1], but display them in int range [0-255]
	};
}

typedef struct s_container {
	int type = CONTAINER::TEXT;
	std::string name = "";
	void (*foo_ptr)( int ) = NULL;
	int *islider = NULL;
	int irange_start = 0, irange_end = 10;
	float *fslider = NULL;
	float frange_start = 0.0f, frange_end = 1.0f;
	int *enu = NULL, enu_index = 0;
	std::vector<std::string> enu_list = {};
	int selection = -1;
	std::array<float*, 4> _color = {NULL, NULL, NULL, NULL};
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
		int _selection, _highlighted_window, _mouse_button, _winWidth, _winHeight;
		bool _moving_window, _resize_window, _moving_slider, _moving_color, _closing_window;
		std::vector<t_window> _content;
		std::vector<int> _draw_order;
		GLFWcursor *_cursor;
		Text *_text;

		void setCursorPosWindow( t_window &win, int posX, int posY );
		void renderWindow( t_window &win, int windex );

	public:
		Gui( void );
		~Gui( void );

		void setWindowSize( int width, int height );
		int getHighlightedWindow( int previous );
		void setCursorPos( double posX, double posY );
		void setMouseButton( GLFWwindow *window, int button, int action );
		bool mouseControl( void );

		void start( void );
		void render( void );

		void writeText( int posX, int posY, int font_size, int color, std::string str );
		bool createWindow( std::string title, std::array<int, 2> pos = {20, 20}, std::array<int, 2> size = {200, 300} );
		void rmWindow( int index );
		void putWindowOnTop( int index );
		void addText( std::string name );
		void addSliderInt( std::string name, int *ptr, int minRange = 0, int maxRange = 10 );
		void addSliderFloat( std::string name, float *ptr, float minRange = 0.0f, float maxRange = 1.0f );
		void addEnum( std::vector<std::string> enu_list, int *iptr, void (*foo_ptr)( int ) = NULL );
		void addColor( std::string name, std::array<float*, 4> color );
};

#endif
