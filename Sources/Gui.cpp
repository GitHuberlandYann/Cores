#include "Gui.hpp"

Gui::Gui( void ) : _selection(-1), _highlighted_window(-1), _mouse_button(GLFW_RELEASE),
	_moving_window(false), _resize_window(false), _moving_slider(false), _moving_color(false),
	_closing_window(false), _cursor(NULL)
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
		win.selection = -1;
		int index = 3;
		for (auto &cont : win.content) {
			if (title_height * 1.5f * (index - 2) + title_height > win.size[1] - 10) return ;
			switch (cont.type) {
				case CONTAINER::SLIDER_INT:
				case CONTAINER::SLIDER_FLOAT:
					if (inRectangle(posX, posY, 10, title_height * 1.5f * (index - 2), win.size[0] / 2, title_height)) {
						win.selection = index;
						return ;
					}
					break ;
				case CONTAINER::ENUM:
					if (inRectangle(posX, posY, 10, title_height * 1.5f * (index - 2), win.size[0] - 20, title_height)) {
						win.selection = index;
						return ;
					}
					break ;
				case CONTAINER::COLOR:
					int div = (cont._color[3]) ? 4 : 3, width = (win.size[0] / 2 - ((div == 4) ? 30 : 20)) / div;
					if (inRectangle(posX, posY, 10, title_height * 1.5f * (index - 2), width, title_height)) {
						win.selection = index;
						cont.selection = 0;
						win.offset[0] = posX + win.pos[0];
						return ;
					} else if (inRectangle(posX, posY, 20 + width, title_height * 1.5f * (index - 2), width, title_height)) {
						win.selection = index;
						cont.selection = 1;
						win.offset[0] = posX + win.pos[0];
						return ;
					} else if (inRectangle(posX, posY, 30 + 2 * width, title_height * 1.5f * (index - 2), width, title_height)) {
						win.selection = index;
						cont.selection = 2;
						win.offset[0] = posX + win.pos[0];
						return ;
					} else if (div == 4 && inRectangle(posX, posY, 40 + 3 * width, title_height * 1.5f * (index - 2), width, title_height)) {
						win.selection = index;
						cont.selection = 3;
						win.offset[0] = posX + win.pos[0];
						return ;
					}
					break ;
			}
			++index;
		}
	}
}

void Gui::renderWindow( t_window &win, int windex )
{
	// std::cout << "rendering gui window " << win.title << std::endl;
	_text->addQuads(0, win.pos[0], win.pos[1] + title_height, win.size[0], win.size[1] - title_height, RGBA::BACK_WINDOW);
	_text->addQuads(0, win.pos[0], win.pos[1], win.size[0] - title_height, title_height, (windex == _highlighted_window) ? RGBA::TITLE_SELECTED_WINDOW : RGBA::TITLE_WINDOW); // title bar
	_text->addQuads(1, win.pos[0] + win.size[0] - title_height, win.pos[1], title_height, title_height, (win.selection == 0) ? RGBA::CLOSE_WINDOW_HOVER : RGBA::CLOSE_WINDOW);
	_text->addText(win.pos[0] + 10, win.pos[1] + 4, 12, RGBA::WHITE, win.title);
	_text->addTriangle(0, {win.pos[0] + win.size[0], win.pos[1] + win.size[1] - title_height}, {win.pos[0] + win.size[0], win.pos[1] + win.size[1]},
			{win.pos[0] + win.size[0] - title_height, win.pos[1] + win.size[1]}, RGBA::MOVE_WINDOW);
	
	int index = 3;
	std::string str;
	for (auto &cont : win.content) {
		if (title_height * 1.5f * (index - 2) + title_height > win.size[1] - 10) return ;
		int posY = win.pos[1] + title_height * 1.5f * (index - 2);
		switch (cont.type) {
			case CONTAINER::TEXT:
				_text->addText(win.pos[0] + 10, posY, 12, RGBA::WHITE, cont.name, win.size[0] - 30);
				break ;
			case CONTAINER::SLIDER_INT:
				_text->addQuads(0, win.pos[0] + 10, posY, win.size[0] / 2, title_height, RGBA::BUTTON);
				_text->addQuads(0, win.pos[0] + 10 + (win.size[0] / 2 - title_height + 8) * getPercent(*cont.islider, cont.irange_start, cont.irange_end),
						posY + 2, title_height - 8, title_height - 4, (win.selection == index) ? RGBA::SLIDER_HOVER : RGBA::SLIDER);
				str = std::to_string(*cont.islider);
				_text->addText(win.pos[0] + 10 + win.size[0] / 4 - _text->textWidth(12, str) / 2, posY + 4, 12, RGBA::WHITE, str);
				_text->addText(win.pos[0] + win.size[0] / 2 + 20, posY + 4, 12, RGBA::WHITE, cont.name, win.size[0] / 2 - 30);
				break ;
			case CONTAINER::SLIDER_FLOAT:
				_text->addQuads(0, win.pos[0] + 10, posY, win.size[0] / 2, title_height, RGBA::BUTTON);
				_text->addQuads(0, win.pos[0] + 10 + (win.size[0] / 2 - title_height + 8) * getPercent(*cont.fslider, cont.frange_start, cont.frange_end),
						posY + 2, title_height - 8, title_height - 4, (win.selection == index) ? RGBA::SLIDER_HOVER : RGBA::SLIDER);
				str = to_string_with_precision(*cont.fslider, 2, false);
				_text->addText(win.pos[0] + 10 + win.size[0] / 4 - _text->textWidth(12, str) / 2, posY + 4, 12, RGBA::WHITE, str);
				_text->addText(win.pos[0] + win.size[0] / 2 + 20, posY + 4, 12, RGBA::WHITE, cont.name, win.size[0] / 2 - 30);
				break ;
			case CONTAINER::ENUM:
				_text->addQuads(0, win.pos[0] + 10, posY, win.size[0] - 20, title_height, (win.selection == index) ? RGBA::SLIDER_HOVER : RGBA::BUTTON);
				str = cont.enu_list[cont.enu_index];
				_text->addText(win.pos[0] + (win.size[0] - _text->textWidth(12, str)) / 2, posY + 4, 12, RGBA::WHITE, str);
				break ;
			case CONTAINER::COLOR:
				int div = (cont._color[3]) ? 4 : 3, width = (win.size[0] / 2 - ((div == 4) ? 30 : 20)) / div;
				_text->addQuads(0, win.pos[0] + 10, posY, width, title_height, RGBA::BUTTON);
				str = std::to_string(static_cast<int>(*cont._color[0] * 255));
				_text->addText(win.pos[0] + 10 + (width - _text->textWidth(12, str)) / 2, posY + 4, 12, RGBA::WHITE, str);
				_text->addQuads(0, win.pos[0] + 20 + width, posY, width, title_height, RGBA::BUTTON);
				str = std::to_string(static_cast<int>(*cont._color[1] * 255));
				_text->addText(win.pos[0] + 20 + width + (width - _text->textWidth(12, str)) / 2, posY + 4, 12, RGBA::WHITE, str);
				_text->addQuads(0, win.pos[0] + 30 + 2 * width, posY, width, title_height, RGBA::BUTTON);
				str = std::to_string(static_cast<int>(*cont._color[2] * 255));
				_text->addText(win.pos[0] + 30 + 2 * width + (width - _text->textWidth(12, str)) / 2, posY + 4, 12, RGBA::WHITE, str);
				if (div == 4) {
					_text->addQuads(0, win.pos[0] + 40 + 3 * width, posY, width, title_height, RGBA::BUTTON);
					str = std::to_string(static_cast<int>(*cont._color[3] * 255));
					_text->addText(win.pos[0] + 40 + 3 * width + (width - _text->textWidth(12, str)) / 2, posY + 4, 12, RGBA::WHITE, str);
				}
				_text->addQuads(0, win.pos[0] + win.size[0] / 2 + 20, posY, title_height, title_height, rgbaFromVec({*cont._color[0], *cont._color[1], *cont._color[2], (div == 4) ? *cont._color[3] : 1.0f}));
				_text->addText(win.pos[0] + win.size[0] / 2 + 30 + title_height, posY + 4, 12, RGBA::WHITE, cont.name, win.size[0] / 2 - 40 - title_height);
				break ;
		}
		++index;
	}
	// _text->addText(win.pos[0] + 10, win.pos[1] + title_height * 1.5f * (index - 2), 12, RGBA::WHITE, "selection " + std::to_string(win.selection));
}

// ************************************************************************** //
//                                Public                                      //
// ************************************************************************** //

void Gui::setWindowSize( int width, int height )
{
	for (auto &win : _content) {
		win.pos[0] *= static_cast<float>(width) / _winWidth;
		win.pos[1] *= static_cast<float>(height) / _winHeight;
	}
	_winWidth = width;
	_winHeight = height;
	_text->setWindowSize(width, height);
}

int Gui::getHighlightedWindow( int previous )
{
	if (_highlighted_window == -1) {
		return (previous);
	}
	return (_highlighted_window);
}

void Gui::setCursorPos( double posX, double posY )
{
	if (_moving_window) {
		t_window &win = _content[_selection];
		win.pos[0] = posX - win.offset[0];
		win.pos[1] = posY - win.offset[1];
		if (win.pos[0] < 0) win.pos[0] = 0;
		else if (win.pos[0] > _winWidth - title_height) win.pos[0] = _winWidth - title_height;
		if (win.pos[1] < 0) win.pos[1] = 0;
		else if (win.pos[1] > _winHeight - title_height) win.pos[1] = _winHeight - title_height;
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
		if (cont.type == CONTAINER::SLIDER_INT) {
			*cont.islider = static_cast<int>(gradient(posX, win.pos[0] + 10, win.pos[0] + 10 + win.size[0] / 2, cont.irange_start, cont.irange_end));
		} else {
			*cont.fslider = gradient(posX, win.pos[0] + 10, win.pos[0] + 10 + win.size[0] / 2, cont.frange_start, cont.frange_end);
		}
		return ;
	} else if (_moving_color) {
		t_window &win = _content[_selection];
		int diff = posX - win.offset[0];
		win.offset[0] = posX;
		t_container &cont = win.content[win.selection - 3];
		*cont._color[cont.selection] += diff / 200.0f;
		if (*cont._color[cont.selection] < 0) *cont._color[cont.selection] = 0;
		if (*cont._color[cont.selection] > 1.0f) *cont._color[cont.selection] = 1.0f;
		return ;
	}

	for (int i = _draw_order.size() - 1; i >= 0; --i) {
		t_window &win = _content[_draw_order[i]];
		if (inRectangle(posX, posY, win.pos[0], win.pos[1], win.size[0], win.size[1])) {
			setCursorPosWindow(win, posX - win.pos[0], posY - win.pos[1]);
			_selection = _draw_order[i];
			return ;
		}
	}
	_selection = -1;
}

void Gui::setMouseButton( GLFWwindow *window, int button, int action )
{
	if (button == GLFW_MOUSE_BUTTON_LEFT) {
		_mouse_button = action;
		if (action == GLFW_PRESS && _selection != -1) {
			_highlighted_window = _selection;
			putWindowOnTop(_selection);
			t_window &win = _content[_selection];
			switch (win.selection) {
				case -1: // no select
					break ;
				case 0: // close window
					rmWindow(_selection);
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
						case CONTAINER::SLIDER_INT:
						case CONTAINER::SLIDER_FLOAT:
							_moving_slider = true;
							break ;
						case CONTAINER::COLOR:
							_moving_color = true;
							break ;
						case CONTAINER::ENUM:
							t_container &cont = win.content[win.selection - 3];
							++cont.enu_index;
							if (cont.enu_index >= static_cast<int>(cont.enu_list.size())) {
								cont.enu_index = 0;
							}
							if (cont.enu) *cont.enu = cont.enu_index;
							if (cont.foo_ptr) cont.foo_ptr(cont.enu_index);
							break ;
					}
					break ;
			}
		} else if (action == GLFW_RELEASE) {
			if (_resize_window) glfwDestroyCursor(_cursor);
			_moving_window = false;
			_resize_window = false;
			_moving_slider = false;
			_moving_color = false;
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
	for (int index : _draw_order) {
		renderWindow(_content[index], index);
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
	int index;
	bool posOk = false;
	while (!posOk) {
		posOk = true;
		index = 0;
		for (auto &win : _content) {
			if (win.title == title) {
				_highlighted_window = index;
				return (false);
			}
			if (win.pos[1] == pos[1]) {
				pos[0] += title_height;
				pos[1] += title_height;
				posOk = false;
			}
			++index;
		}
		if (posOk) {
			posOk = false;
			if (pos[0] < 0) pos[0] = 0;
			else if (pos[0] > _winWidth - title_height) pos[0] = _winWidth - title_height;
			else if (pos[1] < 0) pos[1] = 0;
			else if (pos[1] > _winHeight - title_height) pos[1] = _winHeight - title_height;
			else posOk = true;
		}
	}
	_highlighted_window = _content.size();
	_draw_order.push_back(_highlighted_window);
	_content.push_back({title, pos, size});
	return (true);
}

void Gui::rmWindow( int index )
{
	_content.erase(_content.begin() + index);
	for (int &d : _draw_order) {
		if (d >= index) --d;
	}
	_draw_order.pop_back();
	_highlighted_window = -1;
}

void Gui::putWindowOnTop( int index )
{
	size_t i = 0, doSize = _draw_order.size();
	if (!doSize) return ;
	for (; _draw_order[i] != index && i < doSize; ++i);
	if (i == doSize || i == doSize - 1) return ;
	for (; i < doSize - 1; ++i) {
		_draw_order[i] = _draw_order[i + 1];
	}
	_draw_order[i] = index;
}

void Gui::addText( std::string name )
{
	if (_content.empty()) return ;

	_content.back().content.push_back({CONTAINER::TEXT, name});
}

void Gui::addSliderInt( std::string name, int *ptr, int minRange, int maxRange )
{
	if (_content.empty()) return ;

	_content.back().content.push_back({CONTAINER::SLIDER_INT, name, NULL, ptr, minRange, maxRange});
}

void Gui::addSliderFloat( std::string name, float *ptr, float minRange, float maxRange )
{
	if (_content.empty()) return ;

	_content.back().content.push_back({CONTAINER::SLIDER_FLOAT, name, NULL, NULL, 0, 0, ptr, minRange, maxRange});
}

void Gui::addEnum( std::vector<std::string> enu_list, int *iptr, void (*foo_ptr)( int ) )
{
	if (_content.empty()) return ;

	_content.back().content.push_back({CONTAINER::ENUM, "", foo_ptr, NULL, 0, 0, NULL, 0, 0, iptr, (iptr) ? *iptr : 0, enu_list});
}

void Gui::addColor( std::string name, std::array<float*, 4> color )
{
	if (_content.empty()) return ;

	_content.back().content.push_back({CONTAINER::COLOR, name, NULL, NULL, 0, 0, NULL, 0, 0, NULL, 0, {}, -1, color});
}
