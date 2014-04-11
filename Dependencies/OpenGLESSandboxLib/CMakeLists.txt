CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

SET( PROJECT_NAME_TEMP "OpenGLESSandboxLib" )

IF( CMAKE_SYSTEM_NAME MATCHES "Windows" )

	FIND_PACKAGE( Qt4 COMPONENTS QtMain QtCore QtGui )

	IF( QT4_FOUND )
		
		PROJECT( ${PROJECT_NAME_TEMP} )
		
		IF( MSVC )
			INCLUDE( VisualStudioSettings.cmake )
		ENDIF()
					
		INCLUDE_DIRECTORIES( "${CMAKE_CURRENT_LIST_DIR}/../gles_sdk/include" )

		SET(	SOURCES
				OGLESApplicationContext.h
				OGLESApplicationContext.cpp
				OGLESApplication.h
				OGLESApplication.cpp
				OGLESApplicationRunner.h
				OGLESApplicationRunner.cpp
				OGLESApplicationRunner_AMDEmulator.h
				OGLESApplicationRunner_AMDEmulator.cpp
				OGLESApplicationRunner_AMDEmulatorWidget.h
				OGLESApplicationRunner_AMDEmulatorWidget.cpp
			)

		SOURCE_GROUP("" FILES ${SOURCES} )		# Avoid "Header Files" and "Source Files" virtual folders in VisualStudio

		QT4_WRAP_CPP(	MOC_SOURCES 
						OGLESApplicationRunner_AMDEmulatorWidget.h
					 )

		SOURCE_GROUP("Moc files" FILES ${MOC_SOURCES} )	# Put moc files in "Moc files" virtual folders in VisualStudio
							
		INCLUDE( ${QT_USE_FILE} ) 
		
		ADD_DEFINITIONS( ${QT_DEFINITIONS} )

		ADD_LIBRARY( ${PROJECT_NAME} STATIC ${SOURCES} ${MOC_SOURCES})

		TARGET_LINK_LIBRARIES( ${PROJECT_NAME} "${CMAKE_CURRENT_LIST_DIR}/../gles_sdk/x86/libEGL.lib" )
		TARGET_LINK_LIBRARIES( ${PROJECT_NAME} "${CMAKE_CURRENT_LIST_DIR}/../gles_sdk/x86/libGLESv2.lib" )
		TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${QT_LIBRARIES}) 
	
	ELSE()
		MESSAGE("Project ${PROJECT_NAME_TEMP}: QT not found. Project won't be compiled")
	ENDIF()

ELSEIF( CMAKE_SYSTEM_NAME MATCHES "Linux" )
	
	PROJECT( ${PROJECT_NAME_TEMP} )
		
	# CFLAGS+=-DSTANDALONE -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS -DTARGET_POSIX -D_LINUX -fPIC -DPIC -D_REENTRANT -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64 -U_FORTIFY_SOURCE -Wall -g -DHAVE_LIBOPENMAX=2 -DOMX -DOMX_SKIP64BIT -ftree-vectorize -pipe -DUSE_EXTERNAL_OMX -DHAVE_LIBBCM_HOST -DUSE_EXTERNAL_LIBBCM_HOST -DUSE_VCHIQ_ARM -Wno-psabi
	ADD_DEFINITIONS( 
		-DSTANDALONE 
		-D__STDC_CONSTANT_MACROS 
		-D__STDC_LIMIT_MACROS 
		-DTARGET_POSIX 
		-D_LINUX 
		-fPIC 
		-DPIC 
		-D_REENTRANT 
		-D_LARGEFILE64_SOURCE 
		-D_FILE_OFFSET_BITS=64 
		-U_FORTIFY_SOURCE 
		-Wall 
		-g 
		-DHAVE_LIBOPENMAX=2 
		-DOMX 
		-DOMX_SKIP64BIT 
		-ftree-vectorize 
		-pipe 
		-DUSE_EXTERNAL_OMX 
		-DHAVE_LIBBCM_HOST 
		-DUSE_EXTERNAL_LIBBCM_HOST 
		-DUSE_VCHIQ_ARM 
		-Wno-psabi )

	# INCLUDES+=-I$(SDKSTAGE)/opt/vc/include/ -I$(SDKSTAGE)/opt/vc/include/interface/vcos/pthreads -I$(SDKSTAGE)/opt/vc/include/interface/vmcs_host/linux -I./ -I../libs/ilclient -I../libs/vgfont
	INCLUDE_DIRECTORIES( 
		"/opt/vc/include"			#  bcm_host.h
		"/opt/vc/include/interface/vcos/pthreads"
		"/opt/vc/include/interface/vmcs_host/linux"
		)
	
	LINK_DIRECTORIES( "/opt/vc/lib" )

	SET(	SOURCES
				OGLESApplicationContext.h
				OGLESApplicationContext.cpp
				OGLESApplication.h
				OGLESApplication.cpp
				OGLESApplicationRunner.h
				OGLESApplicationRunner.cpp
				OGLESApplicationRunner_RaspberryPi.h
				OGLESApplicationRunner_RaspberryPi.cpp
			)

	ADD_LIBRARY( ${PROJECT_NAME} STATIC ${SOURCES} )

	# LDFLAGS+=-L$(SDKSTAGE)/opt/vc/lib/ -lGLESv2 -lEGL -lopenmaxil -lbcm_host -lvcos -lvchiq_arm -lpthread -lrt -L../libs/ilclient -L../libs/vgfont
	TARGET_LINK_LIBRARIES( ${PROJECT_NAME}
					GLESv2		# glClearColor  glEnable glBlabla...
					EGL			# eglTerminate
					#openmaxil		 
					bcm_host	# vc_dispmanx_element_add
					#vcos
					#vchiq_arm
					#pthread
					#rt
				)
	
ELSE()
	MESSAGE("Project ${PROJECT_NAME_TEMP}: platform not supported. Project won't be compiled")
ENDIF()