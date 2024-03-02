#include "Gui.hpp"

Gui::Gui( void ) : _selection(-1), _mouse_button(GLFW_RELEASE), _moving_window(false), _closing_window(false)
{
	_text = new Text();
}

Gui::~Gui( void )
{
	delete _text;
}

// ************************************************************************** //
//                                Private                                     //
// ************************************************************************** //

// posX, posY are relative to t_window's position
void Gui::setCursorPosWindow( t_window &win, int posX, int posY )
{
	if (inRectangle(posX, posY, 0, 0, win.size[0], title_height)) { // title bar
		if (inRectangle(posX, posY, win.size[0] - title_height, 0, title_height, title_height)) {
			win.selection = 0; // close window
		} else {
			win.offset = {posX, posY};
			win.selection = 1; // move window
		}
	} else {
		win.selection = -1;
	}
}

void Gui::renderWindow( t_window &win )
{
	// std::cout << "rendering gui window " << win.title << std::endl;
	_text->addQuads(0, win.pos[0], win.pos[1] + title_height, win.size[0], win.size[1] - title_height, RGBA::BACK_WINDOW);
	_text->addQuads(0, win.pos[0], win.pos[1], win.size[0] - title_height, title_height, 0x4F2000FF); // title bar
	_text->addQuads(0, win.pos[0] + win.size[0] - title_height, win.pos[1], title_height, title_height, (win.selection == 0) ? RGBA::CLOSE_WINDOW_HOVER : RGBA::CLOSE_WINDOW);
	_text->addText(win.pos[0] + 10, win.pos[1] + 4, 12, RGBA::WHITE, win.title);
	_text->addText(win.pos[0] + 10, win.pos[1] + 40, 12, RGBA::WHITE, "selection " + std::to_string(win.selection));
}

// ************************************************************************** //
//                                Public                                      //
// ************************************************************************** //

void Gui::setWindowSize( int width, int height )
{
	_text->setWindowSize(width, height);
}

void Gui::setCursorPos( double posX, double posY )
{
	if (_moving_window) {
		t_window &win = _content[_selection];
		win.pos[0] += posX - win.pos[0] - win.offset[0];
		win.pos[1] += posY - win.pos[1] - win.offset[1];
		return ;
	}
	if (_selection != -1) { // first check current window
		t_window &win = _content[_selection];
		if (inRectangle(posX, posY, win.pos[0], win.pos[1], win.size[0], win.size[1])) {
			setCursorPosWindow(win, posX - win.pos[0], posY - win.pos[1]);
			return ;
		}
	}
	int index = 0;
	for (auto &win : _content) {
		if (inRectangle(posX, posY, win.pos[0], win.pos[1], win.size[0], win.size[1])) {
			setCursorPosWindow(win, posX - win.pos[0], posY - win.pos[1]);
			_selection = index;
			return ;
		}
		++index;
	}
	_selection = -1;
}

void Gui::setMouseButton( int button, int action )
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		_mouse_button = action;
		if (action == GLFW_PRESS && _selection != -1) {
			switch (_content[_selection].selection) {
				case 0: // close window
					_content.erase(_content.begin() + _selection);
					_selection = -1;
					_closing_window = true;
					break ;
				case 1: // move around
					_moving_window = true;
					break ;
			}
		} else if (action == GLFW_RELEASE) {
			_moving_window = false;
			_closing_window = false;
		}
	}
}

bool Gui::mouseControl( void )
{
	return (_selection != -1 || _closing_window);
}

void Gui::start( void )
{
	_shaderProgram = _text->start();
	check_glstate("Gui successfully started", true);
}

void Gui::render( void )
{
	// _text->addText(10, 10, 20, RGBA::WHITE, "window " + std::to_string(_selection));
	int index = 0;
	for (auto &win : _content) {
		if (_selection != index) {
			renderWindow(win);
		}
		++index;
	}
	if (_selection != -1) {
		renderWindow(_content[_selection]);
	}

	_text->toScreen();
}

void Gui::addText( int posX, int posY, int font_size, int color, std::string str )
{
	_text->addText(posX, posY, font_size, color, str);
}

void Gui::createWindow( std::string title, std::array<int, 2> pos, std::array<int, 2> size )
{
	for (auto &win : _content) {
		if (win.title == title) {
			return ;
		}
	}
	_content.push_back({title, pos, size});
}
