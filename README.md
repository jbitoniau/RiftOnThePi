RiftOnThePi
===========

A simple Oculus Rift test application targeted at the Raspberry Pi.

The goal is to evaluate how the Raspberry Pi performs with the Rift and if it can handle it.
Rendering is done using OpenGL ES 2.0. Accessing the Oculus Rift API is done through the Oculus SDK with a minor fix for the Raspberry Pi.

# Getting and preparing the Oculus SDK
- Download the Linux version of the Oculus SDK at https://developer.oculusvr.com/
- Extract it to a temporary folder of your choice
- Run the ConfigurePermissionsAndPackages.sh (this adds udev rule for the device and download required packages)
- Git-clone the code from RiftOnThePi 
- Copy the Oculus SDK "Include" and "Source" directories into RiftOnThePi/Dependencies/LibOVR 
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
