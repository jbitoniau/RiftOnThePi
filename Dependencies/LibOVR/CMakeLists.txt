CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT( LibOVR )

#IF(CMAKE_BUILD_TYPE MATCHES DEBUG)				# In debug we should activate that, but it also means doing it in the client projects that
#   ADD_DEFINITIONS(-DOVR_BUILD_DEBUG)			# use the lib. Otherwise, things get messy as LibOVR is compiled one way and app using it
#ENDIF(CMAKE_BUILD_TYPE MATCHES DEBUG)			# is compiled another way... for now, I don't activate debug on LibOVR.

SET(	SOURCES
		Include/OVR.h
		Include/OVRVersion.h
		Src/Kernel/OVR_Alg.h
		Src/Kernel/OVR_Alg.cpp
		Src/Kernel/OVR_Allocator.h
		Src/Kernel/OVR_Allocator.cpp
		Src/Kernel/OVR_Array.h
		Src/Kernel/OVR_Atomic.h
		Src/Kernel/OVR_Atomic.cpp
		Src/Kernel/OVR_Color.h
		Src/Kernel/OVR_ContainerAllocator.h
		Src/Kernel/OVR_File.h
		Src/Kernel/OVR_File.cpp
		Src/Kernel/OVR_FileFILE.cpp
		Src/Kernel/OVR_Hash.h
		Src/Kernel/OVR_KeyCodes.h
		Src/Kernel/OVR_List.h
		Src/Kernel/OVR_Log.h
		Src/Kernel/OVR_Log.cpp
		Src/Kernel/OVR_Math.h
		Src/Kernel/OVR_Math.cpp
		Src/Kernel/OVR_RefCount.h
		Src/Kernel/OVR_RefCount.cpp
		Src/Kernel/OVR_Std.h
		Src/Kernel/OVR_Std.cpp
		Src/Kernel/OVR_String.h
		Src/Kernel/OVR_String.cpp
		Src/Kernel/OVR_String_FormatUtil.cpp
		Src/Kernel/OVR_String_PathUtil.cpp
		Src/Kernel/OVR_StringHash.h
		Src/Kernel/OVR_SysFile.h
		Src/Kernel/OVR_SysFile.cpp
		Src/Kernel/OVR_System.h
		Src/Kernel/OVR_System.cpp
		Src/Kernel/OVR_Threads.h
		Src/Kernel/OVR_Timer.h
		Src/Kernel/OVR_Timer.cpp
		Src/Kernel/OVR_Types.h
		Src/Kernel/OVR_UTF8Util.h
		Src/Kernel/OVR_UTF8Util.cpp
		
		Src/Util/Util_LatencyTest.h
		Src/Util/Util_LatencyTest.cpp
		Src/Util/Util_Render_Stereo.h
		Src/Util/Util_Render_Stereo.cpp
		
		Src/OVR_Device.h
		Src/OVR_DeviceConstants.h
		Src/OVR_DeviceHandle.h
		Src/OVR_DeviceHandle.cpp
		Src/OVR_DeviceImpl.h
		Src/OVR_DeviceImpl.cpp
		Src/OVR_DeviceMessages.h
		Src/OVR_HIDDevice.h
		Src/OVR_HIDDeviceBase.h
		Src/OVR_HIDDeviceImpl.h
		Src/OVR_JSON.h
		Src/OVR_JSON.cpp
		Src/OVR_LatencyTestImpl.h
		Src/OVR_LatencyTestImpl.cpp
		Src/OVR_Profile.h
		Src/OVR_Profile.cpp
		Src/OVR_SensorFilter.h
		Src/OVR_SensorFilter.cpp
		Src/OVR_SensorFusion.h
		Src/OVR_SensorFusion.cpp
		Src/OVR_SensorImpl.h
		Src/OVR_SensorImpl.cpp
		Src/OVR_ThreadCommandQueue.h
		Src/OVR_ThreadCommandQueue.cpp
	)

IF( CMAKE_SYSTEM_NAME MATCHES "Linux" )

	SET( 	EXTRA_SOURCES ${EXTRA_SOURCES} 
			Src/Kernel/OVR_ThreadsPthread.cpp	
			Src/OVR_Linux_DeviceManager.h		
			Src/OVR_Linux_DeviceManager.cpp
			Src/OVR_Linux_HIDDevice.h			
			Src/OVR_Linux_HIDDevice.cpp		
			Src/OVR_Linux_HMDDevice.h
			Src/OVR_Linux_HMDDevice.cpp
			Src/OVR_Linux_SensorDevice.cpp
		)
		
		SET( EXTRA_LIBS ${EXTRA_LIBS}
			 udev
			 pthread
			 #GL
			 X11
			 Xinerama 
		)
		
#ADD_DEF(		 -pipe -fPIC -DDEBUG -g
#	RELEASETYPE   = Debug
#lse
#	CXXFLAGS      = -pipe -fPIC -O2

ELSEIF( CMAKE_SYSTEM_NAME MATCHES "Windows" )

	IF( MSVC )
		INCLUDE( VisualStudioSettings.cmake )
	ENDIF()

	SET( 	EXTRA_SOURCES ${EXTRA_SOURCES} 
			Src/Kernel/OVR_ThreadsWinAPI.cpp		
			Src/OVR_Win32_DeviceManager.h		
			Src/OVR_Win32_DeviceManager.cpp
			Src/OVR_Win32_DeviceStatus.h
			Src/OVR_Win32_DeviceStatus.cpp
			Src/OVR_Win32_HIDDevice.h			
			Src/OVR_Win32_HIDDevice.cpp		
			Src/OVR_Win32_HMDDevice.h
			Src/OVR_Win32_HMDDevice.cpp
			Src/OVR_Win32_SensorDevice.cpp
		)
	SET( 	EXTRA_LIBS ${EXTRA_LIBS}
			Winmm.lib Setupapi.lib )
ENDIF()

SET( SOURCES ${SOURCES} 
	 ${EXTRA_SOURCES} )

SOURCE_GROUP("" FILES ${SOURCES} )		# Avoid "Header Files" and "Source Files" virtual folders in VisualStudio

INCLUDE_DIRECTORIES( "Include" )

ADD_LIBRARY( ${PROJECT_NAME} STATIC ${SOURCES} )

TARGET_LINK_LIBRARIES( ${PROJECT_NAME} ${EXTRA_LIBS} )
