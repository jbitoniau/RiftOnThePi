/*
	Copyright (C) 2014 Jacques Menuet

	This content is released under the MIT License (http://opensource.org/licenses/MIT)
	
	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.
*/
#include "Common.h"

#include <stdlib.h>
#include <stdio.h>
#include <cmath>

namespace OGLESSandbox
{

GLuint Common::createAndCompileShader( GLenum type, const char *shaderSrc )
{
	// Create the shader object
	GLuint shader=0;
	shader = glCreateShader(type);
	if ( shader==0 )
		return 0;

	// Load the shader source
	glShaderSource(shader, 1, &shaderSrc, NULL);

	// Compile the shader
	glCompileShader(shader);

	// Check the compile status
	GLint compiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if ( !compiled )
	{
		GLint infoLen = 0;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
		if ( infoLen>1 )
		{
			char *infoLog = (char*)malloc ( sizeof ( char ) * infoLen );			// Use new !!!!!
			glGetShaderInfoLog( shader, infoLen, NULL, infoLog );
			printf( "Error compiling shader:\n%s\n", infoLog );
			free( infoLog );
		}
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

GLuint Common::createAndLinkProgram( GLuint vertexShader, GLuint fragmentShader )
{
	// Create the program object
	GLuint programObject = glCreateProgram();
	if ( programObject==0 )
		return 0;

	// Attach shaders to the program
	glAttachShader(programObject, vertexShader);
	glAttachShader(programObject, fragmentShader);
	
	// Link the program
	glLinkProgram(programObject);

	// Check the link status
	GLint linked = 0;
	glGetProgramiv(programObject, GL_LINK_STATUS, &linked);
	if ( !linked )
	{
		GLint infoLen = 0;
		glGetProgramiv( programObject, GL_INFO_LOG_LENGTH, &infoLen );
		if ( infoLen > 1 )
		{
			char *infoLog = (char*)malloc ( sizeof ( char ) * infoLen );		// use new!!!
			glGetProgramInfoLog ( programObject, infoLen, NULL, infoLog );
			printf( "Error linking program:\n%s\n", infoLog );
			free ( infoLog );
		}
		glDeleteProgram(programObject);
		return 0;
	}
	return programObject;
}

}