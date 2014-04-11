CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

IF( MSVC )
	# Set Unicode as character set for Visual Studio projects. By default it's Multi-Byte
	ADD_DEFINITIONS(-DUNICODE -D_UNICODE)
			
	# Set Warning level to 4
	IF ( CMAKE_CXX_FLAGS MATCHES "/W[0-4]" )
		STRING( REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" )
	ELSE()
		SET( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4" )
	ENDIF()
ENDIF()

