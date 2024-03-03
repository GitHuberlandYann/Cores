#include "Display.hpp"
#include "random.hpp"
#include "callbacks.hpp"

Display::Display( void )
	: _window(NULL), _winWidth(WIN_WIDTH), _winHeight(WIN_HEIGHT), _texture(0), _current_core(0),
		_input_released(true)
{
	_gui = new Gui();
	_cores.reserve(9);
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

	_uniWinPos = glGetUniformLocation(_shaderRenderProgram, "winPos");
	_uniWinSize = glGetUniformLocation(_shaderRenderProgram, "winSize");
	_uniRMinSpeed = glGetUniformLocation(_shaderRenderProgram, "minSpeed");
	_uniRMaxSpeed = glGetUniformLocation(_shaderRenderProgram, "maxSpeed");

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

void Display::add_core( void )
{
	size_t index = _cores.size();
	if (index >= 9) {
		std::cout << "can't add new core, limit of 9 reached." << std::endl;
		return ;
	}
	t_core core;
	_cores.push_back(core);
	t_core &c = _cores.back();

	c._num_parts = NUM_PARTS;
	c._origin = {static_cast<float>(_winPos[0] + _winWidth / 2), static_cast<float>(_winPos[1] + _winHeight / 2)};
	c._mass = 6.6989; //==5000000;
	_gravity[index * 3 + 0] = c._origin[0];
	_gravity[index * 3 + 1] = c._origin[1];
	_gravity[index * 3 + 2] = c._mass; // we store power of 10, so mass of 1 is 10, 2 is 100, ...
	_polarity[index] = POLARITY::ATTRACTION;
	c._minTheta = -3.1415f;
	c._maxTheta = 3.1415f;
	c._minSpeed = 50.0f;
	c._maxSpeed = 60.0f;
	c._terminalVelocity = 300.0f;

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
			
		check_glstate("setup vao " + std::to_string(i) + " of core " + std::to_string(index), true);
	}
}

void Display::init_cores( int num_parts, float min_age, float max_age )
{
	unsigned seed = 654321;
	_particles.reserve(num_parts);
	for (int i = 0; i < num_parts; ++i) {
		float life = min_age + Random::randomFloat(seed) * (max_age - min_age);

		_particles.push_back({{0, 0}, life + 1, life, {0, 0}});
	}

	glfwGetWindowPos(_window, &_winPos[0], &_winPos[1]);

	int index = 0;
	for (; index < 1; ++index) {
		add_core();
	}
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

void Display::handleInputs( void )
{
	if (!_gui->mouseControl()) {
		if (glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			double mouseX, mouseY;
			glfwGetCursorPos(_window, &mouseX, &mouseY);
			// _origin = {static_cast<float>((mouseX / _winWidth) * 2 - 1), -static_cast<float>((mouseY / _winHeight) * 2 - 1)};
			_cores[_current_core]._origin = {static_cast<float>(mouseX + _winPos[0]), static_cast<float>(mouseY + _winPos[1])};
			_gravity[_current_core * 3] = mouseX + _winPos[0];
			_gravity[_current_core * 3 + 1] = mouseY + _winPos[1];
		}
		if (glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
			double mouseX, mouseY;
			glfwGetCursorPos(_window, &mouseX, &mouseY);
			// _gravity_center = {static_cast<float>((mouseX / _winWidth) * 2 - 1), -static_cast<float>((mouseY / _winHeight) * 2 - 1)};
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
		&& glfwGetKey(_window, GLFW_KEY_9) == GLFW_RELEASE) {
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
		}
		if (core_loc == _cores.size()) {
			add_core();
			_current_core = core_loc;
		} else if (core_loc < _cores.size()) _current_core = core_loc;
		else return ;
		if (_gui->createWindow("Core " + std::to_string(core_loc + 1), {_winWidth - 220, 20})) {
			t_core &c = _cores[core_loc];
			_gui->addSliderFloat("Mass", &_gravity[core_loc * 3 + 2], 1, 10);
			_gui->addEnum({"ATTRACTION", "REPULSION"}, &_polarity[core_loc]);
			_gui->addSliderInt("Particles", &c._num_parts, 0, NUM_PARTS);
			_gui->addSliderFloat("Min Speed", &c._minSpeed, 1, 300);
			_gui->addSliderFloat("Max Speed", &c._maxSpeed, 10, 300);
			_gui->addSliderFloat("Terminal Speed", &c._terminalVelocity, 10, 1500);
			_gui->addSliderFloat("Min Theta", &c._minTheta, -3.14159, 3.14159);
			_gui->addSliderFloat("Max Theta", &c._maxTheta, -3.14159, 3.14159);
		}
	}
}

void Display::render( double deltaTime )
{
	size_t index = 0;
	for (auto &c : _cores) {
		// _gui->addText(c._origin[0] - _winPos[0], c._origin[1] - _winPos[1], 24, RGBA::WHITE, "Debug");
		int num_part = static_cast<int>(c._born_parts);
		if (num_part < c._num_parts) {
			c._born_parts += 1000 * deltaTime; // birth rate
			if (c._born_parts > c._num_parts) {
				c._born_parts = c._num_parts;
			}
		} else if (num_part > c._num_parts) {
			c._born_parts -= 1000 * deltaTime; // death rate
			if (c._born_parts < 0) {
				c._born_parts = 0;
			}
		}

		glUseProgram(_shaderUpdateProgram);

		glUniform1f(_uniDeltaT, deltaTime);
		glUniform2f(_uniOrigin, c._origin[0], c._origin[1]);
		_gravity[index * 3] = 1000000; // disable own gravity
		glUniform3fv(_uniGravity, 10, &_gravity[0]);
		_gravity[index * 3] = c._origin[0]; // enable own gravity back for next cores
		++index;
		glUniform1iv(_uniPolarity, 10, &_polarity[0]);
		glUniform1f(_uniMinTheta, c._minTheta);
		glUniform1f(_uniMaxTheta, c._maxTheta);
		glUniform1f(_uniMinSpeed, c._minSpeed);
		glUniform1f(_uniMaxSpeed, c._maxSpeed);
		glUniform1f(_uniTerminalVelocity, c._terminalVelocity);

		glBindVertexArray(c._vaos[_state.read]);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, c._vbos[_state.write]);

		glEnable(GL_RASTERIZER_DISCARD); // we don't render anything

		glBeginTransformFeedback(GL_POINTS);
		glDrawArrays(GL_POINTS, 0, num_part);
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
	glClearColor(0, 0, 0, 1.0f);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	set_display_callback(this, _gui);
	glfwSetWindowSizeCallback(_window, window_size_callback);
	glfwSetWindowPosCallback(_window, window_pos_callback);
	glfwSetCursorPosCallback(_window, cursor_pos_callback);
	glfwSetMouseButtonCallback(_window, mouse_button_callback);
	_gui->setWindowSize(_winWidth, _winHeight);

	check_glstate("setup done, entering main loop\n", true);

	double lastTime = glfwGetTime(), previousFrame = lastTime - 0.5;
	int nbFrames = 0, nbFramesLastSecond = 0;

	while (!glfwWindowShouldClose(_window)) {
		if (glfwGetKey(_window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(_window, GL_TRUE);
			continue ;
		}

		handleInputs();

		double currentTime = glfwGetTime();
		double deltaTime = currentTime - previousFrame;
		++nbFrames;
		if (currentTime - lastTime >= 1.0) {
			float nb_parts = 0;
			for (size_t i = 0; i < _cores.size(); ++i) nb_parts += _cores[i]._born_parts;
			std::cout << "FPS: " << nbFrames << ", " << nb_parts << " parts. current_core " << _current_core << "/" << _cores.size() << std::endl;
			nbFramesLastSecond = nbFrames;
			nbFrames = 0;
			lastTime += 1.0;
		}
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		render(deltaTime);
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

void Display::start( void )
{
	setup_window();
	create_shaders();
	setup_communication_shaders();
	load_texture();
	init_cores(NUM_PARTS, 5.01f, 10.15f);
	_gui->start();
	main_loop();
}
