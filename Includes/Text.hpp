#ifndef TEXT_HPP
# define TEXT_HPP

# include "utils.hpp"

# include <vector>

namespace RGBA
{
	const int WHITE = 0xFFFFFFFF;
	const int BLACK = 0xFF000000;
	const int BACK_WINDOW = 0xDF202020;
	const int TITLE_WINDOW = 0xDFFF0000;
	const int CLOSE_WINDOW = 0xDFDD0000;
	const int CLOSE_WINDOW_HOVER = 0xFFFF0000;
}

namespace TEXT
{
	enum {
		SPECATTRIB,
		COLATTRIB,
		POSATTRIB
	};
}

class Text
{
	private:
        GLuint _vao, _vbo, _shaderProgram;
		GLint _uniWidth, _uniHeight;
		GLuint _texture;
		std::vector<int> _texts;

        void setup_shader( void );
		void setup_communication_shader( void );
		void load_texture( void );

	public:
		Text( void );
		~Text( void );

		GLuint start( void );
		void setWindowSize( int width, int height );
		void addQuads( int spec, int posX, int posY, int width, int height, int color );
        void addText( int posX, int posY, int font_size, int color, std::string str );
		void toScreen( void );
};

#endif
