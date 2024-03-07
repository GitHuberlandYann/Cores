#include "Display.hpp"
#include "random.hpp"
#include "callbacks.hpp"
#include "string.h" // strncmp

Display::Display( void )
	: _window(NULL), _winWidth(WIN_WIDTH), _winHeight(WIN_HEIGHT), _texture(0), _current_core(0),
		_multi_id('x'), _input_released(true), _socket(NULL)
{
	_gui = new Gui();
}

Display::~Display( void )
{
	std::cout << "Destructor of display called" << std::endl;

	if (_texture) {
		glDeleteTextures(1, &_texture);
	}

	glDeleteProgram(_shaderUpdateProgram);
	glDeleteProgram(_shaderRenderProgram);

	for (auto &c : _cores) {
		glDeleteBuffers(2, c._vbos);
		glDeleteVertexArrays(2, c._vaos);
	}

	glfwMakeContextCurrent(NULL);
    glfwTerminate();

	delete _gui;
	delete _socket;

	check_glstate("Display successfully destructed", true);
}

// ************************************************************************** //
//                                Private                                     //
// ************************************************************************** //

void Display::setup_window( void )
{
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
    	std::cerr << "glfwInit failure" << std::endl;
        exit(1);
    }

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
	// glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GL_TRUE);

	_window = glfwCreateWindow(_winWidth, _winHeight, "Cores", nullptr, nullptr);
	if (_window == NULL)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

	// activate opengl context
	glfwMakeContextCurrent(_window);

	// glew is there to use the correct version for all functions
	glewExperimental = GL_TRUE;
	glewInit();

	check_glstate("Window successfully created", true);
}

void Display::create_shaders( void )
{
	_shaderUpdateProgram = createShaderProgram("particle_update_vertex", "", "");

	glBindAttribLocation(_shaderUpdateProgram, POSATTRIB, "position");
	glBindAttribLocation(_shaderUpdateProgram, AGEATTRIB, "age");
	glBindAttribLocation(_shaderUpdateProgram, LIFEATTRIB, "life");
	glBindAttribLocation(_shaderUpdateProgram, VELATTRIB, "velocity");

	const GLchar *feedbackVaryings[] = {"Position", "Age", "Life", "Velocity"};
	glTransformFeedbackVaryings(_shaderUpdateProgram, 4, feedbackVaryings, GL_INTERLEAVED_ATTRIBS);

	glLinkProgram(_shaderUpdateProgram);
	glUseProgram(_shaderUpdateProgram);

	check_glstate("Particle update shader program successfully created", true);

	_shaderRenderProgram = createShaderProgram("particle_render_vertex", "", "particle_render_fragment");

	glBindFragDataLocation(_shaderRenderProgram, 0, "outColor");

	glBindAttribLocation(_shaderRenderProgram, POSATTRIB, "position");
	glBindAttribLocation(_shaderRenderProgram, AGEATTRIB, "age");
	glBindAttribLocation(_shaderRenderProgram, LIFEATTRIB, "life");
	glBindAttribLocation(_shaderRenderProgram, VELATTRIB, "velocity");

	glLinkProgram(_shaderRenderProgram);
	glUseProgram(_shaderRenderProgram);

	check_glstate("Particle render shader program successfully created", true);
}

void Display::setup_communication_shaders( void )
{
	_uniDeltaT = glGetUniformLocation(_shaderUpdateProgram, "deltaTime");
	_uniOrigin = glGetUniformLocation(_shaderUpdateProgram, "origin");
	_uniGravity = glGetUniformLocation(_shaderUpdateProgram, "gravity");
	_uniPolarity = glGetUniformLocation(_shaderUpdateProgram, "polarity");
	_uniMinTheta = glGetUniformLocation(_shaderUpdateProgram, "minTheta");
	_uniMaxTheta = glGetUniformLocation(_shaderUpdateProgram, "maxTheta");
	_uniMinSpeed = glGetUniformLocation(_shaderUpdateProgram, "minSpeed");
	_uniMaxSpeed = glGetUniformLocation(_shaderUpdateProgram, "maxSpeed");
	_uniTerminalVelocity = glGetUniformLocation(_shaderUpdateProgram, "terminalVelocity");
	_uniLifeSpan = glGetUniformLocation(_shaderUpdateProgram, "lifeSpan");
	_uniLifeRange = glGetUniformLocation(_shaderUpdateProgram, "lifeRange");

	_uniWinPos = glGetUniformLocation(_shaderRenderProgram, "winPos");
	_uniWinSize = glGetUniformLocation(_shaderRenderProgram, "winSize");
	_uniRMinSpeed = glGetUniformLocation(_shaderRenderProgram, "minSpeed");
	_uniRMaxSpeed = glGetUniformLocation(_shaderRenderProgram, "maxSpeed");
	_uniBirthSize = glGetUniformLocation(_shaderRenderProgram, "birthSize");
	_uniDeathSize = glGetUniformLocation(_shaderRenderProgram, "deathSize");
	_uniBirthColor = glGetUniformLocation(_shaderRenderProgram, "birthColor");
	_uniDeathColor = glGetUniformLocation(_shaderRenderProgram, "deathColor");
	_uniSpeedColor = glGetUniformLocation(_shaderRenderProgram, "speedColor");
	_uniRLifeSpan = glGetUniformLocation(_shaderRenderProgram, "lifeSpan");
	_uniRLifeRange = glGetUniformLocation(_shaderRenderProgram, "lifeRange");

	check_glstate("\nCommunication with shader program successfully established", true);
}

void Display::load_texture( void )
{
	glUseProgram(_shaderUpdateProgram);

	glGenTextures(1, &_texture);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, _texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, 512, 512, 0, GL_RG, GL_UNSIGNED_BYTE, &(Random::randomRGData(512, 512)[0]));

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glUniform1i(glGetUniformLocation(_shaderUpdateProgram, "rgNoise"), 0);

	check_glstate("Texture rgNoise done", true);
}

void Display::add_core( int index )
{
	if (index < 0 || index >= 9) return ;
	t_core &c = _cores[index];

	if (!c._destroyed) return ;
	c._destroyed = false;
	c._visible = false;
	c._num_parts = NUM_PARTS;
	_gravity[index * 3 + 0] = _winPos[0] + _winWidth / 2;
	_gravity[index * 3 + 1] = _winPos[1] + _winHeight / 2;
	_gravity[index * 3 + 2] = 6.6989f; // we store power of 10, so mass of 1 is 10, 2 is 100, ...
	_polarity[index] = POLARITY::ATTRACTION;
	c._minTheta = -3.1415f;
	c._maxTheta = 3.1415f;
	c._minSpeed = 50.0f;
	c._maxSpeed = 60.0f;
	c._terminalVelocity = 300.0f;
	c._birthCol = {1.0f, 0.0f, 0.0f, 1.0f};
	c._deathCol = {0.0f, 0.0f, 1.0f, 0.0f};
	c._speedCol = {0.5f, 1.0f, 0.5f};
	c._birthSize = 5;
	c._deathSize = 0;
	c._lifeSpan = 5.01f;
	c._lifeRange = 5.14f;

	glGenVertexArrays(2, c._vaos);
	glGenBuffers(2, c._vbos);
	check_glstate("VAOs and VBOs", false);

	for (int i = 0; i < 2; ++i) {
		glBindVertexArray(c._vaos[i]);
		glBindBuffer(GL_ARRAY_BUFFER, c._vbos[i]);
		glBufferData(GL_ARRAY_BUFFER, _particles.size() * sizeof(t_particle), &(_particles[0]), GL_STREAM_DRAW);

		glEnableVertexAttribArray(POSATTRIB);
		glVertexAttribPointer(POSATTRIB, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (void *)(0 * sizeof(GL_FLOAT)));

		glEnableVertexAttribArray(AGEATTRIB);
		glVertexAttribPointer(AGEATTRIB, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (void *)(2 * sizeof(GL_FLOAT)));

		glEnableVertexAttribArray(LIFEATTRIB);
		glVertexAttribPointer(LIFEATTRIB, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (void *)(3 * sizeof(GL_FLOAT)));

		glEnableVertexAttribArray(VELATTRIB);
		glVertexAttribPointer(VELATTRIB, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (void *)(4 * sizeof(GL_FLOAT)));
			
		check_glstate("setup vao " + std::to_string(i) + " of core " + std::to_string(index), false);
	}
}

void Display::init_cores( int num_parts, float min_age, float age_range )
{
	unsigned seed = 654321;
	_particles.reserve(num_parts);
	for (int i = 0; i < num_parts; ++i) {
		float life = Random::randomFloat(seed);
		// float life = min_age + Random::randomFloat(seed) * (max_age - min_age);

		_particles.push_back({{0, 0}, Random::randomFloat(seed) * (min_age + life * age_range), life, {0, 0}});
	}

	glfwGetWindowPos(_window, &_winPos[0], &_winPos[1]);

	for (int i = 0; i < 9; ++i) {
		add_core(i);
		if (i > 0) {
			_cores[i]._destroyed = true;
		}
	}
	int index = 1;
	for (; index < 9; ++index) {
		_gravity[index * 3 + 0] = 1000000;
		_gravity[index * 3 + 1] = 0;
		_gravity[index * 3 + 2] = 0;
		_polarity[index] = POLARITY::ATTRACTION;
	}
	_gravity[29] = 7.0f; // 10**7
	_polarity[9] = POLARITY::ATTRACTION;

	check_glstate("init_particles", true);
}

void Display::updateCore( int index, void *data )
{
	// std::cout << "update Core " << index << std::endl;
	// for (int i = 0; i < 90; ++i) {
	// 	std::cout << static_cast<int>(static_cast<char*>(data)[i]) << '|';
	// }
	// std::cout << std::endl;
	if (index < 0 || index >= 9) return ;

	t_core &c = _cores[index];
	memmove(&c._destroyed, &static_cast<char*>(data)[8], CORE_PACKET_SIZE - 16);
	memmove(&_gravity[index * 3], &static_cast<char*>(data)[8 + CORE_PACKET_SIZE - 16], 12); // position + mass
	memmove(&_polarity[index], &static_cast<char*>(data)[8 + CORE_PACKET_SIZE - 4], 4); // just polarity
}

void Display::updateCores( void *data )
{
	// std::cout << "update Cores" << std::endl;
	for (int index = 0; index < 9; ++index) {
		if (index == _multi_id) continue ;

		t_core &c = _cores[index];
		memmove(&c._destroyed, &static_cast<char*>(data)[7 + CORE_PACKET_SIZE * index], CORE_PACKET_SIZE - 16);
		memmove(&_gravity[index * 3], &static_cast<char*>(data)[7 + CORE_PACKET_SIZE * index + CORE_PACKET_SIZE - 16], 12); // position + mass
		memmove(&_polarity[index], &static_cast<char*>(data)[7 + CORE_PACKET_SIZE * index + CORE_PACKET_SIZE - 4], 4); // just polarity
		// std::cout << "core " << index << " destroyed " << _cores[index]._destroyed << " at pos " << _cores[index]._origin[0] << ", " << _cores[index]._origin[1] << std::endl;
	}
}

void Display::updateGameState( void )
{
	while (true) { // first we receive info, same for client and server
		Address sender;
		char buffer[PACKET_SIZE_LIMIT];

		int bytes_read = _socket->Receive(sender, buffer, sizeof(buffer));
		
		if (bytes_read <= 0) {
			break;
		}

		// std::cout << "received " << bytes_read << " bytes" << std::endl;

		// process packet
		if (!strncmp(buffer, "Core ", 5)) { // "Core 0: xxxxxx"
			updateCore(buffer[5] - '0', buffer);
		} else if (!strncmp(buffer, "Cores: ", 7)) {
			updateCores(buffer);
		} else if (!strncmp(buffer, "Connect", 7)) {
			if (_socket->GetType() == SOCKET::SERVER) {
				int id = _socket->GetId(sender);
				buffer[7] = id + '0';
				buffer[8] = '\0';
				_socket->Send(sender, buffer, 9);
				add_core(id);
			} else {
				_multi_id = buffer[7] - '0';
				add_core(_multi_id);
				std::cout << "Connect " << _multi_id << std::endl;
			}
		}
	}

	if (_socket->GetType() == SOCKET::SERVER) {
		char buffer[PACKET_SIZE_LIMIT];
		strcpy(buffer, "Cores: ");
		for (int index = 0; index < 9; ++index) {
			memmove(&buffer[7 + index * CORE_PACKET_SIZE], &_cores[index]._destroyed, CORE_PACKET_SIZE - 16);
			memmove(&buffer[7 + index * CORE_PACKET_SIZE + CORE_PACKET_SIZE - 16], &_gravity[index * 3], 12);
			memmove(&buffer[7 + index * CORE_PACKET_SIZE + CORE_PACKET_SIZE - 4], &_polarity[index], 4);
		}
		_socket->Broadcast(buffer, 7 + 9 * CORE_PACKET_SIZE);
	} else if (_multi_id == 'x') { // we ask to connect until we receive answer
		char buffer[PACKET_SIZE_LIMIT];
		strcpy(buffer, "Connect");
		if (!_socket->Send(_socket->GetServerAddress(), buffer, 8)) {
			closeSocket();
		}
	} else {
		char buffer[PACKET_SIZE_LIMIT];
		strcpy(buffer, (std::string("Core ") + static_cast<char>(_multi_id + '0') + ": ").c_str());
		memmove(&buffer[8], &_cores[_multi_id]._destroyed, CORE_PACKET_SIZE - 16);
		memmove(&buffer[8 + CORE_PACKET_SIZE - 16], &_gravity[_multi_id * 3], 12);
		memmove(&buffer[8 + CORE_PACKET_SIZE - 4], &_polarity[_multi_id], 4);
		if (!_socket->Send(_socket->GetServerAddress(), buffer, 8 + CORE_PACKET_SIZE)) {
			closeSocket();
		}
	}
}

void Display::handleInputs( void )
{
	if (!_gui->mouseControl()) {
		if (glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && !_cores[_current_core]._destroyed) {
			double mouseX, mouseY;
			glfwGetCursorPos(_window, &mouseX, &mouseY);
			_gravity[_current_core * 3] = mouseX + _winPos[0];
			_gravity[_current_core * 3 + 1] = mouseY + _winPos[1];
		}
		if (glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
			double mouseX, mouseY;
			glfwGetCursorPos(_window, &mouseX, &mouseY);
			_gravity[27] = mouseX + _winPos[0];
			_gravity[28] = mouseY + _winPos[1];
		} else {
			_gravity[27] = 1000000;
		}
	}

	if (glfwGetKey(_window, GLFW_KEY_1) == GLFW_RELEASE && glfwGetKey(_window, GLFW_KEY_2) == GLFW_RELEASE
		&& glfwGetKey(_window, GLFW_KEY_3) == GLFW_RELEASE && glfwGetKey(_window, GLFW_KEY_4) == GLFW_RELEASE
		&& glfwGetKey(_window, GLFW_KEY_5) == GLFW_RELEASE && glfwGetKey(_window, GLFW_KEY_6) == GLFW_RELEASE
		&& glfwGetKey(_window, GLFW_KEY_7) == GLFW_RELEASE && glfwGetKey(_window, GLFW_KEY_8) == GLFW_RELEASE
		&& glfwGetKey(_window, GLFW_KEY_9) == GLFW_RELEASE && glfwGetKey(_window, GLFW_KEY_F3) == GLFW_RELEASE) {
		_input_released = true;
	} else if (_input_released) {
		_input_released = false;
		size_t core_loc = 12;
		if (glfwGetKey(_window, GLFW_KEY_1) == GLFW_PRESS) {
			core_loc = 0;
		} else if (glfwGetKey(_window, GLFW_KEY_2) == GLFW_PRESS) {
			core_loc = 1;
		} else if (glfwGetKey(_window, GLFW_KEY_3) == GLFW_PRESS) {
			core_loc = 2;
		} else if (glfwGetKey(_window, GLFW_KEY_4) == GLFW_PRESS) {
			core_loc = 3;
		} else if (glfwGetKey(_window, GLFW_KEY_5) == GLFW_PRESS) {
			core_loc = 4;
		} else if (glfwGetKey(_window, GLFW_KEY_6) == GLFW_PRESS) {
			core_loc = 5;
		} else if (glfwGetKey(_window, GLFW_KEY_7) == GLFW_PRESS) {
			core_loc = 6;
		} else if (glfwGetKey(_window, GLFW_KEY_8) == GLFW_PRESS) {
			core_loc = 7;
		} else if (glfwGetKey(_window, GLFW_KEY_9) == GLFW_PRESS) {
			core_loc = 8;
		} else if (glfwGetKey(_window, GLFW_KEY_F3) == GLFW_PRESS) {
			if (_gui->createWindow(-1, "Debug window", {20, 20}, {270, 150})) {
				_gui->addVarFloat(&_deltaTime, "ms this frame");
				_gui->addVarInt(&_fps, "FPS");
				_gui->addVarInt(&_tps, "TPS");
				_gui->addButton("MULTIPLAYER", gui_open_multiplayer_window_callback);
				_gui->addVarFloat(&_nb_parts, "particles");
				_gui->addVarInt(&_current_core, "current core is", false);
				_gui->addVarInt(&_nb_cores, "active cores");
				_gui->addColor("background",  {&_backCol[0], &_backCol[1], &_backCol[2], NULL});
				_gui->addText("Cursor:");
				_gui->addSliderFloat("Mass", &_gravity[29], 1, 10);
				_gui->addEnum({"ATTRACTION", "REPULSION"}, &_polarity[9]);
			}
			return ;
		}
		add_core(core_loc);
		_current_core = core_loc;
		if (_gui->createWindow(core_loc, "Core " + std::to_string(core_loc + 1))) {
			t_core &c = _cores[core_loc];
			_gui->addSliderFloat("Mass", &_gravity[core_loc * 3 + 2], 1, 10);
			_gui->addEnum({"ATTRACTION", "REPULSION"}, &_polarity[core_loc]);
			_gui->addSliderInt("Particles", &c._num_parts, 0, NUM_PARTS);
			_gui->addSliderFloat("Min Speed", &c._minSpeed, 1, 300);
			_gui->addSliderFloat("Max Speed", &c._maxSpeed, 10, 300);
			_gui->addSliderFloat("Terminal Speed", &c._terminalVelocity, 10, 1500);
			_gui->addSliderInt("birth size", &c._birthSize, 0, 10);
			_gui->addSliderInt("death size", &c._deathSize, 0, 10);
			_gui->addSliderFloat("life span", &c._lifeSpan, 0, 20);
			_gui->addSliderFloat("life range", &c._lifeRange, 0, 20);
			_gui->addSliderFloat("Min Theta", &c._minTheta, -3.14159, 3.14159);
			_gui->addSliderFloat("Max Theta", &c._maxTheta, -3.14159, 3.14159);
			_gui->addColor("birth color", {&c._birthCol[0], &c._birthCol[1], &c._birthCol[2], &c._birthCol[3]});
			_gui->addColor("death color", {&c._deathCol[0], &c._deathCol[1], &c._deathCol[2], &c._deathCol[3]});
			_gui->addColor("speed color", {&c._speedCol[0], &c._speedCol[1], &c._speedCol[2], NULL});
			_gui->addButton("RANDOMIZE", gui_randomize_callback, NULL, core_loc);
			_gui->addBool("visible", &c._visible);
			_gui->addButton("DESTROY", rm_core_callback, NULL, core_loc);
		}
	}
}

void Display::handleMultiInputs( void )
{
	if (!_gui->mouseControl() && _multi_id != 'x') {
		if (glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			double mouseX, mouseY;
			glfwGetCursorPos(_window, &mouseX, &mouseY);
			_gravity[_multi_id * 3] = mouseX + _winPos[0];
			_gravity[_multi_id * 3 + 1] = mouseY + _winPos[1];
		}
	}

	if (glfwGetKey(_window, GLFW_KEY_1) == GLFW_RELEASE && glfwGetKey(_window, GLFW_KEY_F3) == GLFW_RELEASE) {
		_input_released = true;
	} else if (_input_released) {
		_input_released = false;
		size_t core_loc = 12;
		if (glfwGetKey(_window, GLFW_KEY_1) == GLFW_PRESS) {
			if (_multi_id == 'x') return ;
			core_loc = _multi_id;
		} else if (glfwGetKey(_window, GLFW_KEY_F3) == GLFW_PRESS) {
			if (_gui->createWindow(-1, "Debug window", {20, 20}, {270, 150})) {
				_gui->addVarFloat(&_deltaTime, "ms this frame");
				_gui->addVarInt(&_fps, "FPS");
				_gui->addVarInt(&_tps, "TPS");
				_gui->addButton("MULTIPLAYER", gui_open_multiplayer_window_callback);
				_gui->addVarFloat(&_nb_parts, "particles");
				_gui->addVarInt(&_current_core, "current core is", false);
				_gui->addVarInt(&_nb_cores, "active cores");
				_gui->addColor("background",  {&_backCol[0], &_backCol[1], &_backCol[2], NULL});
			}
			return ;
		}
		add_core(core_loc);
		_current_core = core_loc;
		if (_gui->createWindow(core_loc, "Core " + std::to_string(core_loc + 1))) {
			t_core &c = _cores[core_loc];
			_gui->addSliderFloat("Mass", &_gravity[core_loc * 3 + 2], 1, 10);
			_gui->addEnum({"ATTRACTION", "REPULSION"}, &_polarity[core_loc]);
			_gui->addSliderInt("Particles", &c._num_parts, 0, NUM_PARTS);
			_gui->addSliderFloat("Min Speed", &c._minSpeed, 1, 300);
			_gui->addSliderFloat("Max Speed", &c._maxSpeed, 10, 300);
			_gui->addSliderFloat("Terminal Speed", &c._terminalVelocity, 10, 1500);
			_gui->addSliderInt("birth size", &c._birthSize, 0, 10);
			_gui->addSliderInt("death size", &c._deathSize, 0, 10);
			_gui->addSliderFloat("life span", &c._lifeSpan, 0, 20);
			_gui->addSliderFloat("life range", &c._lifeRange, 0, 20);
			_gui->addSliderFloat("Min Theta", &c._minTheta, -3.14159, 3.14159);
			_gui->addSliderFloat("Max Theta", &c._maxTheta, -3.14159, 3.14159);
			_gui->addColor("birth color", {&c._birthCol[0], &c._birthCol[1], &c._birthCol[2], &c._birthCol[3]});
			_gui->addColor("death color", {&c._deathCol[0], &c._deathCol[1], &c._deathCol[2], &c._deathCol[3]});
			_gui->addColor("speed color", {&c._speedCol[0], &c._speedCol[1], &c._speedCol[2], NULL});
			_gui->addButton("RANDOMIZE", gui_randomize_callback, NULL, core_loc);
			_gui->addBool("visible", &c._visible);
			_gui->addButton("DESTROY", rm_core_callback, NULL, core_loc);
		}
	}
}

void Display::render( void )
{
	size_t index = -1;
	for (auto &c : _cores) {
		++index;
		if (c._destroyed) continue ;
		if (c._visible) {
			_gui->writeText(_gravity[index * 3] - _winPos[0] + 10, _gravity[index * 3 + 1] - _winPos[1], 12, RGBA::WHITE, "Core " + std::to_string(index + 1));
		}
		int num_part = static_cast<int>(c._born_parts);
		if (num_part < c._num_parts) {
			c._born_parts += 4 * _deltaTime; // birth rate
			if (c._born_parts > c._num_parts) {
				c._born_parts = c._num_parts;
			}
		} else if (num_part > c._num_parts) {
			c._born_parts -= 4 * _deltaTime; // death rate
			if (c._born_parts < 0) {
				c._born_parts = 0;
			}
		}

		glUseProgram(_shaderUpdateProgram);

		glUniform1f(_uniDeltaT, _deltaTime / 1000);
		glUniform2f(_uniOrigin, _gravity[index * 3], _gravity[index * 3 + 1]);
		float gSave = _gravity[index * 3];
		_gravity[index * 3] = 1000000; // disable own gravity
		glUniform3fv(_uniGravity, 10, &_gravity[0]);
		_gravity[index * 3] = gSave; // enable own gravity back for next cores
		glUniform1iv(_uniPolarity, 10, &_polarity[0]);
		glUniform1f(_uniMinTheta, c._minTheta);
		glUniform1f(_uniMaxTheta, c._maxTheta);
		glUniform1f(_uniMinSpeed, c._minSpeed);
		glUniform1f(_uniMaxSpeed, c._maxSpeed);
		glUniform1f(_uniTerminalVelocity, c._terminalVelocity);
		glUniform1f(_uniLifeSpan, c._lifeSpan);
		glUniform1f(_uniLifeRange, c._lifeRange);

		glBindVertexArray(c._vaos[_state.read]);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, c._vbos[_state.write]);

		glEnable(GL_RASTERIZER_DISCARD); // we don't render anything

		glBeginTransformFeedback(GL_POINTS);
		glDrawArrays(GL_POINTS, 0, NUM_PARTS); // we always update 10000 particles, but only draw the amount we want
		glEndTransformFeedback();

		glDisable(GL_RASTERIZER_DISCARD);

		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, NULL); // unbind transform feedback buffer

		// now we draw
		glBindVertexArray(c._vaos[_state.read]);
		glUseProgram(_shaderRenderProgram);

		glUniform2i(_uniWinPos, _winPos[0], _winPos[1]);
		glUniform2i(_uniWinSize, _winWidth, _winHeight);
		// glUniform2i(_uniWinOffset, 0, 0);
		// glUniform1f(_uniWinZoom, 1.0f);
		glUniform1f(_uniRMinSpeed, c._minSpeed / 2);
		glUniform1f(_uniRMaxSpeed, c._terminalVelocity);
		glUniform1f(_uniBirthSize, static_cast<float>(c._birthSize));
		glUniform1f(_uniDeathSize, static_cast<float>(c._deathSize));
		glUniform4fv(_uniBirthColor, 1, &c._birthCol[0]);
		glUniform4fv(_uniDeathColor, 1, &c._deathCol[0]);
		glUniform3fv(_uniSpeedColor, 1, &c._speedCol[0]);
		glUniform1f(_uniRLifeSpan, c._lifeSpan);
		glUniform1f(_uniRLifeRange, c._lifeRange);

		glDrawArrays(GL_POINTS, 0, num_part);
	}

	int tmp = _state.read;
	_state.read = _state.write;
	_state.write = tmp;

	check_glstate("render", false);
}

void Display::main_loop( void )
{
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glfwSwapInterval(1);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	set_display_callback(this, _gui);
	glfwSetWindowSizeCallback(_window, window_size_callback);
	glfwSetWindowPosCallback(_window, window_pos_callback);
	glfwSetCursorPosCallback(_window, cursor_pos_callback);
	glfwSetMouseButtonCallback(_window, mouse_button_callback);
	_gui->setWindowSize(_winWidth, _winHeight);

	check_glstate("setup done, entering main loop\n", true);

	double lastTime = glfwGetTime(), previousFrame = lastTime - 0.5, lastGameTick = lastTime;
	int nbFrames = 0, nbTicks = 0;
	_fps = 0;

	while (!glfwWindowShouldClose(_window)) {
		if (glfwGetKey(_window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(_window, GL_TRUE);
			continue ;
		}

		if (!_socket) {
			handleInputs();
		} else {
			handleMultiInputs();
		}

		double currentTime = glfwGetTime();
		_deltaTime = (currentTime - previousFrame) * 1000;
		++nbFrames;
		if (currentTime - lastTime >= 1.0) {
			_nb_parts = 0;
			_nb_cores = 0;
			for (size_t i = 0; i < 9; ++i) 
				if (!_cores[i]._destroyed) {
					_nb_parts += _cores[i]._born_parts;
					++_nb_cores;
				}
			// std::cout << "FPS: " << nbFrames << ", " << _nb_parts << " parts. current_core " << _current_core << "/" << _cores.size() << std::endl;
			_fps = nbFrames;
			nbFrames = 0;
			_tps = nbTicks;
			nbTicks = 0;
			lastTime += 1.0;
		}
		if (currentTime - lastGameTick >= TICK) {
			++nbTicks;
			lastGameTick += TICK;
			if (_socket) {
				updateGameState();
			}
		}

		glClearColor(_backCol[0], _backCol[1], _backCol[2], 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		render();
		_gui->render();
		_current_core = _gui->getHighlightedWindow(_current_core);
		glfwSwapBuffers(_window);
		glfwPollEvents();
		previousFrame = currentTime;
	}
}

// ************************************************************************** //
//                                Public                                      //
// ************************************************************************** //

void Display::setWindowSize( int width, int height )
{
	_winWidth = width;
	_winHeight = height;
	_gui->setWindowSize(width, height);
}

void Display::setWindowPos( int posX, int posY )
{
	_winPos = {posX, posY};
}

void Display::rmCore( int index )
{
	if (index < 0 || index >= 9) return ;

	_cores[index]._destroyed = true;
	_gravity[index * 3] = 1000000;
}

void Display::openMultiplayerWindow( void )
{
	if (!_socket) {
		if (_gui->createWindow(-2, "Multiplayer", {20, 20}, {200, 75})) {
			_gui->addInputText("port", &_server_port);
			_gui->addButton("Host server", host_server_callback);
			_gui->addInputText("server ip", &_server_ip);
			_gui->addButton("Join server", join_server_callback);
		}
	} else if (_socket->GetType() == SOCKET::SERVER) {
		if (_gui->createWindow(-2, "Host", {20, 20}, {200, 75})) {
			_gui->addText("Hosting server on " + getEth0() + ':' + std::to_string(DEFAULT_PORT));
			_gui->addButton("Close server", close_socket_callback);
		}
	} else {
		if (_gui->createWindow(-2, "Client", {20, 20}, {200, 75})) {
			_gui->addText("Joined server on " + getEth0() + ':' + std::to_string(DEFAULT_PORT)); // TODO replace this by ip of server, not my own
			_gui->addVarInt(static_cast<int*>(_socket->GetPacketLostPtr()), "packets lost");
			_gui->addVarInt(static_cast<int*>(_socket->GetPacketSentPtr()), "packets sent");
			_gui->addVarInt(static_cast<int*>(_socket->GetPingPtr()), "ping:", false);
			_gui->addButton("Quit server", close_socket_callback);
		}
	}
}

void Display::hostServer( void )
{
	if (_socket) return ;

	_socket = new Socket(SOCKET::SERVER);
	_socket->Open();
	_multi_id = 0; // host is id 0

	for (int i = 1; i < 9; ++i) {
		rm_core_callback(i);
	}

	_gui->resetWindow(-2, "Multiplayer", "Host");
	_gui->addText("Hosting server on " + getEth0() + ':' + std::to_string(DEFAULT_PORT));
	_gui->addButton("Close server", close_socket_callback);
}

void Display::joinServer( void )
{
	if (_socket) return ;

	_socket = new Socket(SOCKET::CLIENT);
	_socket->Open(0);

	for (int i = 0; i < 9; ++i) {
		rm_core_callback(i);
	}

	_gui->resetWindow(-2, "Multiplayer", "Client");
	_gui->addText("Joined server on " + getEth0() + ':' + std::to_string(DEFAULT_PORT)); // TODO replace this by ip of server, not my own
	_gui->addVarInt(static_cast<int*>(_socket->GetPacketLostPtr()), "packets lost");
	_gui->addVarInt(static_cast<int*>(_socket->GetPacketSentPtr()), "packets sent");
	_gui->addVarInt(static_cast<int*>(_socket->GetPingPtr()), "ping:", false);
	_gui->addButton("Quit server", close_socket_callback);
}

void Display::closeSocket( void )
{
	// _gui->rmWindow(-1); // F3 window

	if (!_socket) {
		_gui->rmWindow(-2); // multiplayer window
		return ;
	}

	if (_socket->GetType() == SOCKET::CLIENT) {
		_gui->resetWindow(-2, "Client", "Disconnected");
		_gui->addText("Connection with server lost");
	} else {
		_gui->resetWindow(-2, "Host", "Offline");
		_gui->addText("Server successfully closed");
	}

	delete _socket;
	_socket = NULL;
}

void Display::start( void )
{
	setup_window();
	create_shaders();
	setup_communication_shaders();
	load_texture();
	init_cores(NUM_PARTS, 5.01f, 5.14f);
	_gui->start();
	main_loop();
}
