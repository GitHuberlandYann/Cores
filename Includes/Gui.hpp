#ifndef GUI_HPP
# define GUI_HPP

# include "Text.hpp"
# include <array>

namespace CONTAINER
{
	enum {
		TEXT,
		BUTTON,
		SLIDER,
		ENUM,
		COLOR
	};
}

typedef struct s_container {
	int type = CONTAINER::TEXT;
	std::array<int, 2> pos = {5, 5};
	float *value = NULL;
	float range_start = 0.0f, range_end = 1.0f;
	void (*foo_ptr)( void ) = NULL;
	int *enu = NULL;
	std::vector<std::string> enu_list;
}				t_container;

const int title_height = 20;
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
		bool _moving_window, _closing_window;
		std::vector<t_window> _content;
		Text *_text;

		void setCursorPosWindow( t_window &win, int posX, int posY );
		void renderWindow( t_window &win );

	public:
		Gui( void );
		~Gui( void );

		void setWindowSize( int width, int height );
		void setCursorPos( double posX, double posY );
		void setMouseButton( int button, int action );
		bool mouseControl( void );

		void start( void );
		void render( void );

		void addText( int posX, int posY, int font_size, int color, std::string str );
		void createWindow( std::string title, std::array<int, 2> pos = {20, 20}, std::array<int, 2> size = {200, 300} );
};

#endif
