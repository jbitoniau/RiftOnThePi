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
#include "OGLESApplicationRunner.h"

#ifdef _WIN32
	#include "OGLESApplicationRunner_AMDEmulator.h"
#else
	#include "OGLESApplicationRunner_RaspberryPi.h"
#endif

#include <sstream>

namespace OGLESSandbox
{

ApplicationRunner::ApplicationRunner()
	: mApplicationContext()
{
}

void ApplicationRunner::splitString( const std::string& text, char delim, std::vector<std::string>& tokens ) 
{
	std::stringstream stream(text);
	std::string token;
	while ( std::getline(stream, token, delim) ) 
		tokens.push_back(token);
}

void ApplicationRunner::parseCommandLineParameters( int argc, char** argv, std::vector< std::pair<std::string, std::string> >& parameters )
{
	parameters.clear();
	for ( int i=0; i<argc; ++i )
	{
		std::vector<std::string> tokens;
		splitString( argv[i], '=', tokens );
		if ( tokens.size()==1 )
			parameters.push_back( std::make_pair( tokens[0], "" ) );
		else if ( tokens.size()>=2 )
			parameters.push_back( std::make_pair( tokens[0], tokens[1] ) );
	}
}

ApplicationRunner* ApplicationRunner::create()
{
	ApplicationRunner* runner = 0;
#ifdef _WIN32
	runner = new AMDEmulatorApplicationRunner();
#else
	runner = new RaspberryPiApplicationRunner();
#endif

	return runner;
};

}