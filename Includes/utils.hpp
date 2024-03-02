#ifndef UTILS_HPP
# define UTILS_HPP

# define GLEW_STATIC
# include <GL/glew.h> // must be before glfw
# include "GLFW/glfw3.h"

# include <iostream>
# include <string>

enum {
	POSATTRIB,
	AGEATTRIB,
	LIFEATTRIB,
	VELATTRIB
};

std::string get_file_content( std::string file_name );
bool inRectangle( int posX, int posY, int rx, int ry, int width, int height );

// shaders
GLuint createShaderProgram( std::string vertex, std::string geometry, std::string fragment );
void check_glstate( std::string str, bool displayDebug );

// textures
// void loadSubTextureArray( int layer, std::string texture_file );
void loadTextureShader( int index, GLuint texture, std::string texture_file );

#endif
