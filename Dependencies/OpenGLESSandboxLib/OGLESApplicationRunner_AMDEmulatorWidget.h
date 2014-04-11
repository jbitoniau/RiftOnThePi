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
#pragma once

#ifdef _MSC_VER
	#pragma warning( push )
	#pragma warning ( disable : 4127 )
	#pragma warning ( disable : 4231 )
	#pragma warning ( disable : 4251 )
	#pragma warning ( disable : 4800 )	
#endif
#include <QtCore/QTimer>
#include <QtGui/QFrame>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QResizeEvent>
#ifdef _MSC_VER
	#pragma warning( pop )
#endif

#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include "OGLESApplication.h"

namespace OGLESSandbox
{

class QOGLESWidget : public QWidget
{
	Q_OBJECT

public:
	QOGLESWidget( Application* application, QWidget* parent, Qt::WindowFlags flags=0  );
	virtual ~QOGLESWidget();
	
	void setApplicationContext( const ApplicationContext& applicationContext );

	void setAutoUpdate( bool value );

private slots:
	virtual void			paintEvent( QPaintEvent* evt );
	virtual QPaintEngine*	paintEngine() const;
	virtual void			resizeEvent( QResizeEvent* evt );
	
protected:
	virtual void			keyPressEvent( QKeyEvent* event );
	virtual void			keyReleaseEvent( QKeyEvent* event );

private:
	Application*			mApplication;
	ApplicationContext		mApplicationContext;
	QTimer*					mUpdateTimer;
};


}