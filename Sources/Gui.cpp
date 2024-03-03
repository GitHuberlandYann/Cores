#include "Gui.hpp"

Gui::Gui( void ) : _selection(-1), _mouse_button(GLFW_RELEASE),
	_moving_window(false), _resize_window(false), _moving_slider(false), _closing_window(false),
	_cursor(NULL)
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
	} else if (inRectangle(posX, posY, win.size[0] - title_height / 2, win.size[1] - title_height / 2, title_height / 2, title_height / 2)) {
		win.selection = 2; // resize window
	} else {
		int index = 3;
		for (auto &cont : win.content) {
			switch (cont.type) {
				case CONTAINER::SLIDER_FLOAT:
					if (inRectangle(posX, posY, 10, title_height * (index - 1.5f), win.size[0] / 2, title_height)) {
						win.selection = index;
						return ;
					}
					break ;
			}
			++index;
		}
		win.selection = -1;
	}
}

void Gui::renderWindow( t_window &win )
{
	// std::cout << "rendering gui window " << win.title << std::endl;
	_text->addQuads(0, win.pos[0], win.pos[1] + title_height, win.size[0], win.size[1] - title_height, RGBA::BACK_WINDOW);
	_text->addQuads(0, win.pos[0], win.pos[1], win.size[0] - title_height, title_height, RGBA::TITLE_WINDOW); // title bar
	_text->addQuads(1, win.pos[0] + win.size[0] - title_height, win.pos[1], title_height, title_height, (win.selection == 0) ? RGBA::CLOSE_WINDOW_HOVER : RGBA::CLOSE_WINDOW);
	_text->addText(win.pos[0] + 10, win.pos[1] + 4, 12, RGBA::WHITE, win.title);
	// _text->addText(win.pos[0] + 10, win.pos[1] + 120, 12, RGBA::WHITE, "selection " + std::to_string(win.selection));
	_text->addTriangle(0, {win.pos[0] + win.size[0], win.pos[1] + win.size[1] - title_height}, {win.pos[0] + win.size[0], win.pos[1] + win.size[1]},
			{win.pos[0] + win.size[0] - title_height, win.pos[1] + win.size[1]}, RGBA::MOVE_WINDOW);
	
	int index = 3;
	for (auto &cont : win.content) {
		switch (cont.type) {
			case CONTAINER::SLIDER_FLOAT:
				_text->addQuads(0, win.pos[0] + 10, win.pos[1] + title_height * (index - 1.5f), win.size[0] / 2, title_height, RGBA::BUTTON);
				_text->addQuads(0, win.pos[0] + 10 + (win.size[0] / 2 - title_height + 8) * getPercent(*cont.value, cont.range_start, cont.range_end),
						win.pos[1] + title_height * (index - 1.5f) + 2, title_height - 8, title_height - 4, (win.selection == index) ? RGBA::SLIDER_HOVER : RGBA::SLIDER);
				std::string str = to_string_with_precision(*cont.value, 2, false);
				_text->addText(win.pos[0] + 10 + win.size[0] / 4 - _text->textWidth(12, str) / 2, win.pos[1] + title_height * (index - 1.5f) + 4, 12, RGBA::WHITE, str);
				_text->addText(win.pos[0] + win.size[0] / 2 + 20, win.pos[1] + title_height * (index - 1.5f) + 4, 12, RGBA::WHITE, cont.name);
				break ;
		}
		++index;
	}
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
		win.pos[0] = posX - win.offset[0];
		win.pos[1] = posY - win.offset[1];
		return ;
	} else if (_resize_window) {
		t_window &win = _content[_selection];
		win.size[0] = posX - win.pos[0];
		win.size[1] = posY - win.pos[1];
		if (win.size[0] < min_win_width) win.size[0] = min_win_width;
		if (win.size[1] < min_win_height) win.size[1] = min_win_height;
		return ;
	} else if (_moving_slider) {
		t_window &win = _content[_selection];
		t_container &cont = win.content[win.selection - 3];
		*cont.value = gradient(posX, win.pos[0] + 10, win.pos[0] + 10 + win.size[0] / 2, cont.range_start, cont.range_end);
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

void Gui::setMouseButton( GLFWwindow *window, int button, int action )
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		_mouse_button = action;
		if (action == GLFW_PRESS && _selection != -1) {
			t_window &win = _content[_selection];
			switch (win.selection) {
				case 0: // close window
					_content.erase(_content.begin() + _selection);
					_selection = -1;
					_closing_window = true;
					break ;
				case 1: // move around
					_moving_window = true;
					break ;
				case 2: // resize window
					_resize_window = true;
					_cursor = glfwCreateStandardCursor(GLFW_RESIZE_ALL_CURSOR);
					glfwSetCursor(window, _cursor);
					break ;
				default: // on container
					switch (win.content[win.selection - 3].type) {
						case CONTAINER::SLIDER_FLOAT:
							_moving_slider = true;
							break ;
					}
					break ;
			}
		} else if (action == GLFW_RELEASE) {
			if (_resize_window) glfwDestroyCursor(_cursor);
			_moving_window = false;
			_resize_window = false;
			_moving_slider = false;
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

void Gui::writeText( int posX, int posY, int font_size, int color, std::string str )
{
	_text->addText(posX, posY, font_size, color, str);
}

//
// > sub windows <
//

bool Gui::createWindow( std::string title, std::array<int, 2> pos, std::array<int, 2> size )
{
	for (auto &win : _content) {
		if (win.title == title) {
			return (false);
		}
	}
	_content.push_back({title, pos, size});
	return (true);
}

void Gui::addSliderFloat( std::string name, float *ptr, float minRange, float maxRange )
{
	if (_content.empty()) return ;

	_content.back().content.push_back({CONTAINER::SLIDER_FLOAT, name, NULL, ptr, minRange, maxRange});
}
