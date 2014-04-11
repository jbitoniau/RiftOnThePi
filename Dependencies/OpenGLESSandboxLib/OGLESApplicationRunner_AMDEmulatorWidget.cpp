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
#include "OGLESApplicationRunner_AMDEmulatorWidget.h"

#include <assert.h>

namespace OGLESSandbox
{

QOGLESWidget::QOGLESWidget( Application* application, QWidget* parent, Qt::WindowFlags flags )
	:	QWidget(parent, flags | Qt::MSWindowsOwnDC ),
		mApplication(application),
		mApplicationContext(),
		mUpdateTimer(NULL)
{
	setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_PaintUnclipped);

	mUpdateTimer = new QTimer(this);
	mUpdateTimer->setInterval(15);
	bool ret = connect( mUpdateTimer, SIGNAL( timeout() ), SLOT( update() ));
	assert( ret );
}

QOGLESWidget::~QOGLESWidget()
{
}

void QOGLESWidget::keyPressEvent( QKeyEvent* event )
{
	if ( event->key()==Qt::Key_Space )
	{
		if ( windowState() & Qt::WindowFullScreen )
			setWindowState(windowState() & ~Qt::WindowFullScreen);
		else
			setWindowState(windowState() | Qt::WindowFullScreen);
	}
}

void QOGLESWidget::keyReleaseEvent( QKeyEvent* /*event*/ )
{
}

void QOGLESWidget::setApplicationContext( const ApplicationContext& applicationContext )
{
	mApplicationContext = applicationContext;
	mApplicationContext.width = width();
	mApplicationContext.height = height();
}

void QOGLESWidget::setAutoUpdate( bool value )
{
	if ( value )
		mUpdateTimer->start();
	else
		mUpdateTimer->stop();
}

void QOGLESWidget::resizeEvent( QResizeEvent* evt )
{
	mApplicationContext.width = evt->size().width();
	mApplicationContext.height = evt->size().height();
}
	
QPaintEngine* QOGLESWidget:: paintEngine() const
{
	return 0;
}
	
void QOGLESWidget::paintEvent( QPaintEvent* /*evt*/ )
{
	if ( mApplication )
		mApplication->draw( mApplicationContext );
}

}