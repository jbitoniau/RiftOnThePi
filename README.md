RiftOnThePi
===========

A simple Oculus Rift test application targeted at the Raspberry Pi.

The goal is to evaluate how the Raspberry Pi performs with the Rift and check whether it can handle it (the answer: yes to some extent!).
Rendering is done using OpenGL ES 2.0. Accessing the Oculus Rift API is done through the Oculus SDK with a minor fix for the Raspberry Pi.

[![Oculus Rift on the Raspberry Pi](http://img.youtube.com/vi/69mZpevHiRA/0.jpg)](http://www.youtube.com/watch?v=69mZpevHiRA)

More information about this project can be found here: http://bitoniau.blogspot.fr/2014/04/oculus-rift-on-raspberry-pi.html

Tested on:
- Oculus Rift DK1
- Raspberry PI Model B and Raspberry PI 2
- Raspbian 2015-05-05 Kernel 3.18 (SHA-1:cb799af077930ff7cbcfaa251b4c6e25b11483de)

# Prerequisite
- Install gcc and cmake
```Bash
	sudo apt-get install build-essential cmake
```

- Packages required for Oculus LibOVR
```Bash
	sudo apt-get install libudev-dev libxext-dev mesa-common-dev freeglut3-dev libxinerama-dev
```


# Getting and preparing the Oculus SDK
- Download the Linux version of the Oculus SDK at https://developer.oculusvr.com/ (version 0.2.5c at the time of writing)
- Extract it to a temporary folder of your choice
- Run the ConfigurePermissionsAndPackages.sh (this adds udev rule for the device and download required packages)
- Git-clone the code from RiftOnThePi 
```Bash
	git clone https://github.com/jbitoniau/RiftOnThePi
```
- Copy the Oculus SDK "LibOVR/Include" and "LibOVR/Source" directories into RiftOnThePi/Dependencies/LibOVR 
  (the directories already exist there but are empty)
- A minor fix is needed in the file Src/Kernel/OVR_Atomic.h for it to compile on the Raspberry Pi
- In struct AtomicOpsRawBase (at around line 94), replace the code in the OVR_CPU_ARM defined section with this:
```C++
	#elif defined(OVR_CPU_ARM)
	//struct FullSync { inline FullSync() { asm volatile("dmb\n"); } ~FullSync() { asm volatile("dmb\n"); } };
    //struct AcquireSync { inline AcquireSync() { } ~AcquireSync() { asm volatile("dmb\n"); } };
    //struct ReleaseSync { inline ReleaseSync() { asm volatile("dmb\n"); } };

	#define MB()  __asm__ __volatile__ ("mcr p15, 0, r0, c7, c10, 5" : : : "memory")
	struct FullSync { inline FullSync() { MB(); } ~FullSync() { MB(); } };
	struct AcquireSync { inline AcquireSync() { } ~AcquireSync() { MB(); } };
	struct ReleaseSync { inline ReleaseSync() { MB(); } };
```

# Compiling and running RiftOnThePi
- Go to the main RiftOnThePi directory
```Bash
	mkdir Build
	cd Build
	cmake .. -DCMAKE_BUILT_TYPE=Release
	make
```
- To run the application from this Build directory, simply type:
```Bash
    RiftOnThePi/RiftOnThePi 
```
- The program accepts several optional arguments, see the code for details:
```Bash
	RiftOnThePi	--StereoRenderTechnique=<0 to 3> --DistortionScaleEnabled=<0 or 1> --AnimationEnabled=<0 or 1> --UseRiftOrientation=<0 or 1>
```		

# Running on Windows
It was faster and more practical to develop this application on a Windows desktop machine. RiftOnThePi therefore also works on Windows using
the AMD OpenGL ES SDK (you must have an AMD graphics card though). Here are the steps to compile the code:
- Download the SDK at http://developer.amd.com/tools-and-sdks/graphics-development/amd-opengl-es-sdk/
- Extract the gles_sdk.zip archive into RiftOnThePi/Dependencies/gles_sdk directory (there's already a placeholder folder there)
- Build RiftOnThePi using CMake as explained for above
