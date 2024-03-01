#include "Display.hpp"
#include "random.hpp"
#include "callbacks.hpp"

Display::Display( void )
	: _window(NULL), _winWidth(WIN_WIDTH), _winHeight(WIN_HEIGHT), _texture(0)
{
}

Display::~Display( void )
{
	std::cout << "Destructor of display called" << std::endl;

	if (_texture) {
		glDeleteTextures(1, &_texture);
	}

	glDeleteProgram(_shaderUpdateProgram);
	glDeleteProgram(_shaderRenderProgram);

	glDeleteBuffers(2, _vbos);
	glDeleteVertexArrays(4, _vaos);

	glfwMakeContextCurrent(NULL);
    glfwTerminate();

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
	_uniMinTheta = glGetUniformLocation(_shaderUpdateProgram, "minTheta");
	_uniMaxTheta = glGetUniformLocation(_shaderUpdateProgram, "maxTheta");
	_uniMinSpeed = glGetUniformLocation(_shaderUpdateProgram, "minSpeed");
	_uniMaxSpeed = glGetUniformLocation(_shaderUpdateProgram, "maxSpeed");

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

void Display::init_particles( int num_parts, float min_age, float max_age ) {

	glGenVertexArrays(4, _vaos);
	glGenBuffers(2, _vbos);
	check_glstate("VAOs and VBOs", false);

	unsigned seed = 654321;
	std::vector<t_particle> vertices;
	for (int i = 0; i < num_parts; ++i) {
		float life = min_age + Random::randomFloat(seed) * (max_age - min_age);

		vertices.push_back({{0, 0}, life + 1, life, {0, 0}});
	}
	std::cout << "v size " << vertices.size() << ", tpart " << sizeof(t_particle) << std::endl;

	for (int i = 0; i < 2; ++i) {
		glBindBuffer(GL_ARRAY_BUFFER, _vbos[i]);
		// glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(int), &(vertices[0]), GL_DYNAMIC_DRAW);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(t_particle), &(vertices[0]), GL_STREAM_DRAW);

		check_glstate("setup vbo " + std::to_string(i), true);
	}

	for (int i = 0; i < 4; ++i) {
		glBindVertexArray(_vaos[i]);
		glBindBuffer(GL_ARRAY_BUFFER, _vbos[i & 0x1]);

		glEnableVertexAttribArray(POSATTRIB);
		glVertexAttribPointer(POSATTRIB, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (void *)(0 * sizeof(GL_FLOAT)));

		glEnableVertexAttribArray(AGEATTRIB);
		glVertexAttribPointer(AGEATTRIB, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (void *)(2 * sizeof(GL_FLOAT)));

		glEnableVertexAttribArray(LIFEATTRIB);
		glVertexAttribPointer(LIFEATTRIB, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (void *)(3 * sizeof(GL_FLOAT)));

		glEnableVertexAttribArray(VELATTRIB);
		glVertexAttribPointer(VELATTRIB, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(GL_FLOAT), (void *)(4 * sizeof(GL_FLOAT)));
		
		check_glstate("setup vao " + std::to_string(i), true);
	}

	check_glstate("init_particles", true);
}

void Display::render( double deltaTime )
{
	int num_part = static_cast<int>(_state.born_parts);
	if (num_part < NUM_PARTS) {
		_state.born_parts += 1000 * deltaTime; // birth rate
		if (_state.born_parts > NUM_PARTS) {
			_state.born_parts = NUM_PARTS;
		}
	}

	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(_shaderUpdateProgram);

	glUniform1f(_uniDeltaT, deltaTime);
	glUniform2f(_uniOrigin, _origin[0], _origin[1]);
	glUniform2f(_uniGravity, _gravity_center[0], _gravity_center[1]);
	glUniform1f(_uniMinTheta, -3.1415f);
	glUniform1f(_uniMaxTheta, 3.1415f);
	// glUniform1f(_uniMinSpeed, 0.05f);
	// glUniform1f(_uniMaxSpeed, 0.1f);
	glUniform1f(_uniMinSpeed, 50.0f);
	glUniform1f(_uniMaxSpeed, 60.0f);

	glBindVertexArray(_vaos[_state.read]);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, _vbos[_state.write]);

	glEnable(GL_RASTERIZER_DISCARD); // we don't render anything

	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, num_part);
	glEndTransformFeedback();

	glDisable(GL_RASTERIZER_DISCARD);

	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, NULL); // unbind transform feedback buffer

	// now we draw
	glBindVertexArray(_vaos[_state.read + 2]);
	// glBindVertexArray(_vaos[_state.write + 2]);
	glUseProgram(_shaderRenderProgram);

	glUniform2i(_uniWinPos, _winPos[0], _winPos[1]);
	glUniform2i(_uniWinSize, _winWidth, _winHeight);
	// glUniform2i(_uniWinOffset, 0, 0);
	// glUniform1f(_uniWinZoom, 1.0f);
	glUniform1f(_uniRMinSpeed, 20.0f);
	glUniform1f(_uniRMaxSpeed, 100.0f);

	glDrawArrays(GL_POINTS, 0, num_part);

	int tmp = _state.read;
	_state.read = _state.write;
	_state.write = tmp;

	check_glstate("render", false);
}

void Display::main_loop( void )
{
	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glfwSwapInterval(1);
	glClearColor(0, 0, 0, 1.0f);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	set_display_callback(this);
	glfwSetWindowSizeCallback(_window, window_size_callback);
	glfwSetWindowPosCallback(_window, window_pos_callback);

	glfwGetWindowPos(_window, &_winPos[0], &_winPos[1]);
	_origin = {static_cast<float>(_winPos[0] + _winWidth / 2), static_cast<float>(_winPos[1] + _winHeight / 2)};

	check_glstate("setup done, entering main loop\n", true);

	double lastTime = glfwGetTime(), previousFrame = lastTime - 0.5;
	int nbFrames = 0, nbFramesLastSecond = 0;

	while (!glfwWindowShouldClose(_window)) {
		if (glfwGetKey(_window, GLFW_KEY_BACKSPACE) == GLFW_PRESS) {
			glfwSetWindowShouldClose(_window, GL_TRUE);
			continue ;
		}

		if (glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
			double mouseX, mouseY;
			glfwGetCursorPos(_window, &mouseX, &mouseY);
			// _origin = {static_cast<float>((mouseX / _winWidth) * 2 - 1), -static_cast<float>((mouseY / _winHeight) * 2 - 1)};
			_origin = {static_cast<float>(mouseX + _winPos[0]), static_cast<float>(mouseY + _winPos[1])};
		}
		if (glfwGetMouseButton(_window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
			double mouseX, mouseY;
			glfwGetCursorPos(_window, &mouseX, &mouseY);
			// _gravity_center = {static_cast<float>((mouseX / _winWidth) * 2 - 1), -static_cast<float>((mouseY / _winHeight) * 2 - 1)};
			_gravity_center = {static_cast<float>(mouseX + _winPos[0]), static_cast<float>(mouseY + _winPos[1])};
		} else {
			_gravity_center[0] = 1000000;
		}

		double currentTime = glfwGetTime();
		double deltaTime = currentTime - previousFrame;
		++nbFrames;
		if (currentTime - lastTime >= 1.0) {
			std::cout << "FPS: " << nbFrames << ", " << _state.born_parts << " parts" << std::endl;
			nbFramesLastSecond = nbFrames;
			nbFrames = 0;
			lastTime += 1.0;
		}

		render(deltaTime);
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
	// init_particles(NUM_PARTS, 1.01f, 1.15f);
	init_particles(NUM_PARTS, 5.01f, 10.15f);
	main_loop();
}
