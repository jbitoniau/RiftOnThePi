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
#include "OGLESApplicationRunner_AMDEmulator.h"

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <stdio.h>

#ifdef _MSC_VER
	#pragma warning( push )
	#pragma warning ( disable : 4127 )
	#pragma warning ( disable : 4231 )
	#pragma warning ( disable : 4251 )
	#pragma warning ( disable : 4800 )
#endif
#include <QtGui/QApplication>
#ifdef _MSC_VER
	#pragma warning( pop )
#endif

#include "OGLESApplicationRunner_AMDEmulatorWidget.h"

namespace OGLESSandbox
{

void AMDEmulatorApplicationRunner::run( Application* application, int argc, char** argv )
{
	QApplication* app = NULL;
	app = new QApplication( argc, argv );

	QOGLESWidget* widget = new QOGLESWidget(application, NULL);
	widget->resize(1280, 800);
	int windowSystemId = reinterpret_cast<int>( widget->winId() );

	ApplicationContext applicationContext; 
	if ( !createEGLContext( windowSystemId, applicationContext ) )
		return;

	std::vector< std::pair<std::string, std::string> > parameters;
	parseCommandLineParameters( argc, argv, parameters );
	applicationContext.parameters = parameters;

	widget->setApplicationContext( applicationContext );
	widget->setAutoUpdate(true);

	widget->show();
	
	if ( application )
	{
		if ( application->initialize( applicationContext ) )
		{
			/*int ret = */app->exec();
		}
	}
}

bool AMDEmulatorApplicationRunner::createEGLContext( int windowSystemId, ApplicationContext& applicationContext )
{
	applicationContext = ApplicationContext();

    EGLBoolean bsuccess;

    // create native window
    EGLNativeDisplayType nativeDisplay = 0;
    /*if(!OpenNativeDisplay(&nativeDisplay))
    {
        printf("Could not get open native display\n");
        return GL_FALSE;
    }*/

    // get egl display handle
    EGLDisplay eglDisplay;
    eglDisplay = eglGetDisplay(nativeDisplay);
    if(eglDisplay == EGL_NO_DISPLAY)
    {
        printf("Could not get EGL display\n");
        //CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }
    //ctx.eglDisplay = eglDisplay;

    // Initialize the display
    EGLint major = 0;
    EGLint minor = 0;
    bsuccess = eglInitialize(eglDisplay, &major, &minor);
    if (!bsuccess)
    {
        printf("Could not initialize EGL display\n");
        //CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }
    if (major < 1 || minor < 4)
    {
        // Does not support EGL 1.4
        printf("System does not support at least EGL 1.4\n");
        //CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }

    // Obtain the first configuration with a depth buffer
    EGLint attrs[] = 
		{ 
			EGL_DEPTH_SIZE, 16,

			EGL_RED_SIZE, 8,
			EGL_GREEN_SIZE, 8,
			EGL_BLUE_SIZE, 8,
			EGL_ALPHA_SIZE, 8,
			EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
			EGL_NONE
		};
    EGLint numConfig =0;
    EGLConfig eglConfig = 0;
    bsuccess = eglChooseConfig(eglDisplay, attrs, &eglConfig, 1, &numConfig);
    if (!bsuccess)
    {
        printf("Could not find valid EGL config\n");
        //CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }

    // Get the native visual id
    int nativeVid;
    if (!eglGetConfigAttrib(eglDisplay, eglConfig, EGL_NATIVE_VISUAL_ID, &nativeVid))
    {
        printf("Could not get native visual id\n");
        //CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }

    EGLNativeWindowType nativeWin = reinterpret_cast<EGLNativeWindowType>( windowSystemId );
    /*if(!CreateNativeWin(nativeDisplay, 640, 480, nativeVid, &nativeWin))
    {
        printf("Could not create window\n");
        //CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }*/

    // Create a surface for the main window
    EGLSurface eglSurface;
    eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, nativeWin, NULL);
    if (eglSurface == EGL_NO_SURFACE)
    {
        printf("Could not create EGL surface\n");
        //DestroyNativeWin(nativeDisplay, nativeWin);
        //CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }
    //ctx.eglSurface = eglSurface;

    // Create an OpenGL ES context
    EGLContext eglContext;
    eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, NULL);
    if (eglContext == EGL_NO_CONTEXT)
    {
        printf("Could not create EGL context\n");
        //DestroyNativeWin(nativeDisplay, nativeWin);
        //CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }

    // Make the context and surface current
    bsuccess = eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
    if(!bsuccess)
    {
        printf("Could not activate EGL context\n");
        //DestroyNativeWin(nativeDisplay, nativeWin);
        //CloseNativeDisplay(nativeDisplay);
        return GL_FALSE;
    }

    //ctx.nativeDisplay = nativeDisplay;
    //ctx.nativeWin = nativeWin;
    applicationContext.context = eglContext;
	applicationContext.display = eglDisplay;
	applicationContext.surface = eglSurface;

	return GL_TRUE;
}

}