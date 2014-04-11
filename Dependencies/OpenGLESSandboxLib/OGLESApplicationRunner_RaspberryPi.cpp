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
#include "OGLESApplicationRunner_RaspberryPi.h"
 
#include "bcm_host.h"
#include "GLES2/gl2.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>

#define check() assert(glGetError() == 0)

namespace OGLESSandbox
{

void RaspberryPiApplicationRunner::run( Application* application, int argc, char** argv )
{
	ApplicationContext applicationContext;
	createEGLContext( applicationContext );

	std::vector< std::pair<std::string, std::string> > parameters;
	parseCommandLineParameters( argc, argv, parameters );
	applicationContext.parameters = parameters;

	const ApplicationContext& ac = applicationContext;
	printf("ctx:%d disp:%d surf:%d w:%d h:%d\n", (int)ac.context, (int)ac.display, (int)ac.surface, ac.width, ac.height );

	if ( application ) 
	{
		if ( application->initialize( applicationContext ) )
		{
			while ( true )
			{
				application->draw( applicationContext );
				usleep( 1 * 1000 );
			}
		}
	}
}

/*
	The following createEGLContext() method is based on the Raspberry Pi example code
	that can be found in /opt/vc/src/hello_pi/ which license is the following.	
*/
/*	
	Copyright (c) 2012, Broadcom Europe Ltd
	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:
		* Redistributions of source code must retain the above copyright
		  notice, this list of conditions and the following disclaimer.
		* Redistributions in binary form must reproduce the above copyright
		  notice, this list of conditions and the following disclaimer in the
		  documentation and/or other materials provided with the distribution.
		* Neither the name of the copyright holder nor the
		  names of its contributors may be used to endorse or promote products
		  derived from this software without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
	ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
	WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
	DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
	DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
	(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
	ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
void RaspberryPiApplicationRunner::createEGLContext( ApplicationContext& applicationContext )
{
	// Don't forget to this at startup otherwise bcm function will fail like graphics_get_display_size 
	bcm_host_init();

	int32_t success = 0;
	EGLBoolean result;
	EGLint num_config;

	static EGL_DISPMANX_WINDOW_T nativewindow;

	DISPMANX_ELEMENT_HANDLE_T dispman_element;
	DISPMANX_DISPLAY_HANDLE_T dispman_display;
	DISPMANX_UPDATE_HANDLE_T dispman_update;
	VC_RECT_T dst_rect;
	VC_RECT_T src_rect;

	static const EGLint attribute_list[] =
	{
		EGL_RED_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_BLUE_SIZE, 8,
		EGL_ALPHA_SIZE, 8,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_NONE
	};

	static const EGLint context_attributes[] = 
	{
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};
	EGLConfig config;

	// get an EGL display connection
	applicationContext.display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	assert(applicationContext.display!=EGL_NO_DISPLAY);
	check();

	// initialize the EGL display connection
	result = eglInitialize(applicationContext.display, NULL, NULL);
	assert(EGL_FALSE != result);
	check();

	// get an appropriate EGL frame buffer configuration
	result = eglChooseConfig(applicationContext.display, attribute_list, &config, 1, &num_config);
	assert(EGL_FALSE != result);
	check();

	// get an appropriate EGL frame buffer configuration
	result = eglBindAPI(EGL_OPENGL_ES_API);
	assert(EGL_FALSE != result);
	check();

	// create an EGL rendering context
	applicationContext.context = eglCreateContext(applicationContext.display, config, EGL_NO_CONTEXT, context_attributes);
	assert(applicationContext.context!=EGL_NO_CONTEXT);
	check();

	// create an EGL window surface
	uint32_t width = 0;
	uint32_t height = 0;
	success = graphics_get_display_size(0 /* LCD */, &width, &height );
	assert( success >= 0 );

	applicationContext.width = width;
	applicationContext.height = height;


	dst_rect.x = 0;
	dst_rect.y = 0;
	dst_rect.width = width;
	dst_rect.height = height;

	src_rect.x = 0;
	src_rect.y = 0;
	src_rect.width = width << 16;
	src_rect.height = height << 16;        

	dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
	dispman_update = vc_dispmanx_update_start( 0 );

	//dispman_element = vc_dispmanx_element_add ( dispman_update, dispman_display,
	//   0/*layer*/, &dst_rect, 0/*src*/,
	//   &src_rect, DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, 0/*transform*/);

	dispman_element = vc_dispmanx_element_add( dispman_update, dispman_display, 
		0/*layer*/, &dst_rect, 0/*src*/, 
		&src_rect, DISPMANX_PROTECTION_NONE, 0, (DISPMANX_CLAMP_T *)NULL, (DISPMANX_TRANSFORM_T)0 );

	nativewindow.element = dispman_element;
	nativewindow.width = width;
	nativewindow.height = height;
	vc_dispmanx_update_submit_sync( dispman_update );

	check();

	applicationContext.surface = eglCreateWindowSurface( applicationContext.display, config, &nativewindow, NULL );
	assert(applicationContext.surface != EGL_NO_SURFACE);
	check();

	// connect the context to the surface
	result = eglMakeCurrent(applicationContext.display, applicationContext.surface, applicationContext.surface, applicationContext.context);
	assert(EGL_FALSE != result);
	check();

	// Set background color and clear buffers
	glClearColor(0.15f, 0.25f, 0.35f, 1.0f);
	glClear( GL_COLOR_BUFFER_BIT );

	check();
}

}
