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
#include "RiftOnThePiApp.h"

#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <cmath>

#include "Common.h"
#include "Kernel/OVR_Timer.h"

#define check() assert(glGetError() == 0)

namespace OGLESSandbox
{

static const float gPi = static_cast<float>(3.14159265358979323846);

//
// Box
//
static const char VertexShaderStringBox[] = 
	"uniform mat4 Projection; \n"
	"uniform mat4 ModelView; \n"
	"attribute vec4 Position; \n"
	"attribute vec4 SourceColor; \n"
	"varying vec4 DestinationColor; \n"
	"void main(void) \n"
	"{ \n"
	"  DestinationColor = SourceColor; \n"
	"  gl_Position = Projection * ModelView * Position; \n"
	"} \n";

static const char FragmentShaderStringBox[] = 
	"varying lowp vec4 DestinationColor; \n"
	"void main(void) \n"
	"{ \n"
	"  gl_FragColor = DestinationColor; \n"
	"} \n";

typedef struct {
    float Position[3];
    float Color[4];
} VertexWithColor;
 
static const VertexWithColor VerticesBox[] = {
    {{ 1, -1,  1},	{1, 0, 0, 1}},
    {{ 1,  1,  1},	{0, 1, 0, 1}},
    {{-1,  1,  1},	{0, 0, 1, 1}},
    {{-1, -1,  1},	{1, 0, 1, 1}},
	{{ 1, -1, -1},	{0, 0, 0, 1}},
    {{ 1,  1, -1},	{0, 0, 0, 1}},
    {{-1,  1, -1},	{1, 1, 1, 1}},
    {{-1, -1, -1},	{1, 1, 1, 1}}
};
 
static const GLubyte IndicesBox[] = {
    // Front
    0, 1, 2,
    0, 2, 3,
	// Right
	4, 5, 1,
	4, 1, 0, 
	// Back
	7, 6, 5,
	7, 5, 4,
	// Left
	3, 2, 6,
	3, 6, 7,
	// Top
	1, 6, 2,
	1, 5, 6,
	// Bottom
	0, 3, 7,
	0, 7, 4
};

//
// Quad
//
static const char VertexShaderStringQuad[] = 
	"attribute vec4 Position; \n"
	"attribute vec2 InputTexCoord; \n"
	"uniform mat4 Texm;\n"
	"varying vec2 oTexCoord; \n"
	"void main() \n"
	"{ \n"
	"   oTexCoord = vec2(Texm * vec4(InputTexCoord,0,1));\n"
	"	gl_Position = Position; \n"
	"} \n";

static const char* FragmentShader0StringQuad=
	"uniform sampler2D Texture0;\n"
	"varying vec2 oTexCoord;\n"
	"void main()\n"
	"{\n"
	"   gl_FragColor = texture2D(Texture0, oTexCoord);\n"
	"}\n";

static const char* FragmentShader1StringQuad=
	"uniform vec2 LensCenter;\n"
	"uniform vec2 ScreenCenter;\n"
	"uniform vec2 Scale;\n"
	"uniform vec2 ScaleIn;\n"
	"uniform vec4 HmdWarpParam;\n"
	"uniform sampler2D Texture0;\n"
	"varying vec2 oTexCoord;\n"
	"\n"
	"vec2 HmdWarp(vec2 in01)\n"
	"{\n"
	"   vec2  theta = (in01 - LensCenter) * ScaleIn;\n" // Scales to [-1, 1]
	"   float rSq = theta.x * theta.x + theta.y * theta.y;\n"
	"   vec2  theta1 = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq + "
	"                           HmdWarpParam.z * rSq * rSq + HmdWarpParam.w * rSq * rSq * rSq);\n"
	"   return LensCenter + Scale * theta1;\n"
	"}\n"
	"void main()\n"
	"{\n"
	"   vec2 tc = HmdWarp(oTexCoord);\n"
	"   if (!all(equal(clamp(tc, ScreenCenter-vec2(0.25,0.5), ScreenCenter+vec2(0.25,0.5)), tc)))\n"
	"       gl_FragColor = vec4(1, 0, 1, 1);\n"  // JBM: was vec4(0) in original shader
	"   else\n"
	"       gl_FragColor = texture2D(Texture0, tc);\n"
	"}\n";

// Shader with lens distortion and chromatic aberration correction.
static const char* FragmentShader2StringQuad =
	"uniform vec2 LensCenter;\n"
	"uniform vec2 ScreenCenter;\n"
	"uniform vec2 Scale;\n"
	"uniform vec2 ScaleIn;\n"
	"uniform vec4 HmdWarpParam;\n"
	"uniform vec4 ChromAbParam;\n"
	"uniform sampler2D Texture0;\n"
	"varying vec2 oTexCoord;\n"
	"\n"
	// Scales input texture coordinates for distortion.
	// ScaleIn maps texture coordinates to Scales to ([-1, 1]), although top/bottom will be
	// larger due to aspect ratio.
	"void main()\n"
	"{\n"
	"   vec2  theta = (oTexCoord - LensCenter) * ScaleIn;\n" // Scales to [-1, 1]
	"   float rSq= theta.x * theta.x + theta.y * theta.y;\n"
	"   vec2  theta1 = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq + "
	"                  HmdWarpParam.z * rSq * rSq + HmdWarpParam.w * rSq * rSq * rSq);\n"
	"   \n"
	"   // Detect whether blue texture coordinates are out of range since these will scaled out the furthest.\n"
	"   vec2 thetaBlue = theta1 * (ChromAbParam.z + ChromAbParam.w * rSq);\n"
	"   vec2 tcBlue = LensCenter + Scale * thetaBlue;\n"
	"   if (!all(equal(clamp(tcBlue, ScreenCenter-vec2(0.25,0.5), ScreenCenter+vec2(0.25,0.5)), tcBlue)))\n"
	"   {\n"
	"       gl_FragColor = vec4(1, 0, 1, 1);\n"  // JBM: was vec4(0) in original shader
	"       return;\n"
	"   }\n"
	"   \n"
	"   // Now do blue texture lookup.\n"
	"   float blue = texture2D(Texture0, tcBlue).b;\n"
	"   \n"
	"   // Do green lookup (no scaling).\n"
	"   vec2  tcGreen = LensCenter + Scale * theta1;\n"
	"   vec4  center = texture2D(Texture0, tcGreen);\n"
	"   \n"
	"   // Do red scale and lookup.\n"
	"   vec2  thetaRed = theta1 * (ChromAbParam.x + ChromAbParam.y * rSq);\n"
	"   vec2  tcRed = LensCenter + Scale * thetaRed;\n"
	"   float red = texture2D(Texture0, tcRed).r;\n"
	"   \n"
	"   gl_FragColor = vec4(red, center.g, blue, 1);\n"
	"}\n";

typedef struct {
    float Position[3];
    float UV[2];
} VertexWithUV;

static const VertexWithUV VerticesQuad[] = {
    {{ 1.f, -1.f, 0.f}, { 1, 0}},				// Change to 0.95 for eg to check the quad covers the entire screen
    {{ 1.f,  1.f, 0.f}, { 1, 1}},
    {{-1.f,  1.f, 0.f}, { 0, 1}},
    {{-1.f, -1.f, 0.f}, { 0, 0}}
};

static const GLubyte IndicesQuad[] = {
     0, 1, 2,
     2, 3, 0
};

RiftOnThePiApp::RiftOnThePiApp()
	: mCounter(0),
	  mLastTime(0),
	  mStereoRenderTechnique(RenderTextureDistortionCorrection),
	  mDistortionScaleEnabled(false),
	  mAnimationEnabled(true),
	  mUseRiftOrientation(false),
	  mDeviceManager(),
	  mHMD(),
	  mSensor(),
	  mSensorFusion(NULL),
	  mStereoConfig(),
	  mScreenHResolution(0),
	  mScreenVResolution(0),
	  mBoxAngleX(0.f),
	  mBoxAngleY(0.f),
	  mBoxAngleZ(0.f),
	  mShaderProgramBox(0),
	  mVertexBufferBox(0),
	  mIndexBufferBox(0),
	  mShaderProgramQuad(0),
	  mVertexBufferQuad(0),
	  mIndexBufferQuad(0),
	  mTexture(0),
	  mTextureFrameBuffer(0),

	  mBoxProjectionUniform(0),
	  mBoxModelViewUniform(0),
	  mBoxPositionAttrib(0),
	  mBoxColorAttrib(0),

	  mQuadTexmUniform(0),
	  mQuadLensCenterUniform(0),
	  mQuadScreenCenterCenterUniform(0),
	  mQuadScaleCenterUniform(0),
	  mQuadScaleInCenterUniform(0),
	  mQuadHmdWarpParamCenterUniform(0),
	  mQuadChromAbParamUniform(0),
	  mQuadTexture0Uniform(0),
	  mQuadPositionAttrib(0),
	  mQuadInputTexCoordAttrib(0)
{
	mLastTime = OVR::Timer::GetTicksMs();
}

bool RiftOnThePiApp::initialize( const ApplicationContext& context ) 
{
	readParameters(context);
	if ( !initOculus() )
		return false;
	createShaderPrograms();
	createGeometries();
	createTexture();
	return true;
}


void RiftOnThePiApp::readParameters( const ApplicationContext& context )
{
	printf("readParameters\n");

	for ( std::size_t i=1; i<context.parameters.size(); ++i )
	{
		const std::string& name = context.parameters[i].first;
		const std::string& value = context.parameters[i].second;
		int intValue = atoi( value.c_str() );
		if ( name=="--StereoRenderTechnique" )
			mStereoRenderTechnique = static_cast<StereoRenderTechnique>(intValue);
		else if ( name=="--DistortionScaleEnabled" )
			mDistortionScaleEnabled = intValue!=0;
		else if ( name=="--AnimationEnabled" )
			mAnimationEnabled = intValue!=0;
		else if ( name=="--UseRiftOrientation" )
			mUseRiftOrientation = intValue!=0;
		else
			printf("Parameter %s is not supported\n", name.c_str() );
	}
	printf("StereoRenderTechnique: %d\n", mStereoRenderTechnique );
	printf("DistortionScaleEnabled: %d\n", mDistortionScaleEnabled );
	printf("AnimationEnabled: %d\n", mAnimationEnabled );
	printf("UseRiftOrientation: %d\n", mUseRiftOrientation );
}

bool RiftOnThePiApp::initOculus()
{
	printf("initOculus\n");

	OVR::System::Init( OVR::Log::ConfigureDefaultLog(OVR::LogMask_All));

	mDeviceManager = *OVR::DeviceManager::Create();
	if ( !mDeviceManager )
	{
		printf("Failed to create DeviceManager\n");
		return false;
	}
	mHMD = *mDeviceManager->EnumerateDevices<OVR::HMDDevice>().CreateDevice();
	if ( !mHMD )
	{
		printf("Failed to create Device\n");
		return false;
	}
	
	mSensor = *mHMD->GetSensor();
	if ( !mSensor )
	{
		printf("Failed to get sensor\n");
		return false;
	}

	mSensorFusion = new OVR::SensorFusion(mSensor);
	
	
	OVR::HMDInfo hmd;
	if (!mHMD->GetDeviceInfo(&hmd))
		return false;

	printf("HResolution: %d\n", hmd.HResolution );
	printf("VResolution: %d\n", hmd.VResolution );
	printf("HScreenSize: %f\n", hmd.HScreenSize );
	printf("VScreenSize: %f\n", hmd.VScreenSize );
	printf("VScreenCenter: %f\n", hmd.VScreenCenter );
	printf("EyeToScreenDistance: %f\n", hmd.EyeToScreenDistance );
	printf("LensSeparationDistance: %f\n", hmd.LensSeparationDistance );
	printf("InterpupillaryDistance: %f\n", hmd.InterpupillaryDistance );
	
	// Remember screen resolution
	mScreenHResolution = hmd.HResolution;
	mScreenVResolution = hmd.VResolution;
	
	// Prepare StereoConfig object
	mStereoConfig.SetHMDInfo(hmd);
	mStereoConfig.SetFullViewport( OVR::Util::Render::Viewport(0,0, hmd.HResolution, hmd.VResolution) );
	mStereoConfig.SetStereoMode( OVR::Util::Render::Stereo_LeftRight_Multipass);

	if ( mStereoRenderTechnique==NoCorrection )
	{
		// No distortion correction, so no scaling
		// Important to do even if we don't correct, because it affects the FOV used to render the scene
		mStereoConfig.SetDistortionFitPointVP(0.f, 0.f);	
	}
	else
	{
		if ( mDistortionScaleEnabled )
		{
			// Configure proper Distortion Fit.
			// For 7" screen, fit to touch left side of the view, leaving a bit of invisible
			// screen on the top (saves on rendering cost).
			// For smaller screens (5.5"), fit to the top.
			if (hmd.HScreenSize > 0.0f)
			{
				if (hmd.HScreenSize > 0.140f) // 7"
					mStereoConfig.SetDistortionFitPointVP(-1.0f, 0.0f);
				else
					mStereoConfig.SetDistortionFitPointVP(0.0f, 1.0f);
			}
		}
		else
		{
			mStereoConfig.SetDistortionFitPointVP(0.f, 0.f);	
		}
	}
	printf("DistortionScale: %f\n", mStereoConfig.GetDistortionScale() );    
	mStereoConfig.Set2DAreaFov(OVR::DegreeToRad(85.0f));		// This is the default value in mStereoConfig 

	return true;
}

void RiftOnThePiApp::createShaderPrograms()
{
	printf("createShaderPrograms\n");
	{
		GLuint vertexShader = Common::createAndCompileShader( GL_VERTEX_SHADER, VertexShaderStringBox );
		if ( vertexShader==0 )
			return;
		GLuint fragmentShader = Common::createAndCompileShader( GL_FRAGMENT_SHADER, FragmentShaderStringBox );
		if ( fragmentShader==0 )
			return;
		GLuint programObject = Common::createAndLinkProgram( vertexShader, fragmentShader );
		if ( programObject==0 )
			return;
	
		// Store
		mShaderProgramBox = programObject;

		mBoxProjectionUniform = glGetUniformLocation(mShaderProgramBox, "Projection");
		mBoxModelViewUniform = glGetUniformLocation(mShaderProgramBox, "ModelView");
		mBoxPositionAttrib = glGetAttribLocation(mShaderProgramBox, "Position");
		mBoxColorAttrib = glGetAttribLocation(mShaderProgramBox, "SourceColor");
	}

	if ( mStereoRenderTechnique!=NoCorrection )
	{
		GLuint vertexShader = Common::createAndCompileShader( GL_VERTEX_SHADER, VertexShaderStringQuad );
		if ( vertexShader==0 )
			return;

		GLuint fragmentShader = 0;
		if ( mStereoRenderTechnique==RenderTextureNoDistortionCorrection)
			fragmentShader = Common::createAndCompileShader( GL_FRAGMENT_SHADER, FragmentShader0StringQuad );
		else if ( mStereoRenderTechnique==RenderTextureDistortionCorrection )
			fragmentShader = Common::createAndCompileShader( GL_FRAGMENT_SHADER, FragmentShader1StringQuad );
		else if ( mStereoRenderTechnique==RenderTextureDistortionAndChromaCorrection )
			fragmentShader = Common::createAndCompileShader( GL_FRAGMENT_SHADER, FragmentShader2StringQuad );
		
		if ( fragmentShader==0 )
			return;

		GLuint programObject = Common::createAndLinkProgram( vertexShader, fragmentShader );
		if ( programObject==0 )
			return;
	
		// Store
		mShaderProgramQuad = programObject;

		mQuadTexmUniform = glGetUniformLocation(mShaderProgramQuad, "Texm");
		mQuadLensCenterUniform = glGetUniformLocation(mShaderProgramQuad, "LensCenter");
		mQuadScreenCenterCenterUniform = glGetUniformLocation(mShaderProgramQuad, "ScreenCenter");
		mQuadScaleCenterUniform = glGetUniformLocation(mShaderProgramQuad, "Scale");
		mQuadScaleInCenterUniform = glGetUniformLocation(mShaderProgramQuad, "ScaleIn");
		mQuadHmdWarpParamCenterUniform = glGetUniformLocation(mShaderProgramQuad, "HmdWarpParam");
		mQuadChromAbParamUniform = glGetUniformLocation(mShaderProgramQuad, "ChromAbParam");
		mQuadTexture0Uniform = glGetUniformLocation(mShaderProgramQuad, "Texture0");
		mQuadPositionAttrib = glGetAttribLocation(mShaderProgramQuad, "Position");
		mQuadInputTexCoordAttrib = glGetAttribLocation(mShaderProgramQuad, "InputTexCoord");
	}
}

void RiftOnThePiApp::createGeometries()
{
	printf("createGeometries\n");
	{
		GLuint vertexBuffer;
		glGenBuffers(1, &vertexBuffer);
		check();
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		check();
		glBufferData(GL_ARRAY_BUFFER, sizeof(VerticesBox), VerticesBox, GL_STATIC_DRAW);
		check();
 
		GLuint indexBuffer;
		glGenBuffers(1, &indexBuffer);
		check();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		check();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(IndicesBox), IndicesBox, GL_STATIC_DRAW);
		check();

		// Store
		mVertexBufferBox = vertexBuffer;
		mIndexBufferBox = indexBuffer;
	}

	if ( mStereoRenderTechnique!=NoCorrection )
	{
		GLuint vertexBuffer;
		glGenBuffers(1, &vertexBuffer);
		check();
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		check();
		glBufferData(GL_ARRAY_BUFFER, sizeof(VerticesQuad), VerticesQuad, GL_STATIC_DRAW);
		check();
 
		GLuint indexBuffer;
		glGenBuffers(1, &indexBuffer);
		check();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		check();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(IndicesQuad), IndicesQuad, GL_STATIC_DRAW);
		check();

		// Store
		mVertexBufferQuad = vertexBuffer;
		mIndexBufferQuad = indexBuffer;
	}
}

void RiftOnThePiApp::createTexture()		
{
	printf("createTexture\n");
	if ( mStereoRenderTechnique==NoCorrection )
	{
		printf("No render texture\n");
		return;
	}
	
	// Prepare a texture image
	GLuint texture = 0;
	glGenTextures(1, &texture);
	check();
	glBindTexture(GL_TEXTURE_2D, texture);
	check();
	
	// The texture we render into is scaled to be potentially larger than the screen (to compensate
	// for the pinching in effect). 
	// See RenderDevice::initPostProcessSupport(PostProcessType pptype) in RenderTiny_Device.cpp
	float sceneRenderScale = mStereoConfig.GetDistortionScale();
	GLsizei w = (int)ceil(sceneRenderScale * mScreenHResolution);	
	GLsizei h = (int)ceil(sceneRenderScale * mScreenVResolution);
	printf( "TextureWidth: %d\n", w );
	printf( "TextureHeight: %d\n", h );
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	check();
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	check();
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	check();

	// Store
	mTexture = texture;

	// Prepare a framebuffer for rendering
	GLuint textureFrameBuffer = 0;
	glGenFramebuffers(1, &textureFrameBuffer);
	check();
	glBindFramebuffer(GL_FRAMEBUFFER, textureFrameBuffer);
	check();
	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	check();
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	check();

	// Store
	mTextureFrameBuffer = textureFrameBuffer;
}

void RiftOnThePiApp::draw( const ApplicationContext& context ) 
{
	unsigned int time = OVR::Timer::GetTicksMs();
	float deltaTime = static_cast<float>( time - mLastTime );

	bool displayDrawTime = (mLastTime/1000 != time/1000);
	mLastTime = time;

	if ( mAnimationEnabled )
	{
		mBoxAngleX = 0.f;
		mBoxAngleY += (deltaTime / 30.f);
		mBoxAngleZ += (deltaTime / 100.f); 
	}
	else
	{
		mBoxAngleX = 0.f;
		mBoxAngleY = 0.f;
		mBoxAngleZ = 0.f;	
	}

	if ( mStereoRenderTechnique!=NoCorrection )
	{
		// Clear texture frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, mTextureFrameBuffer);
		check();
		glClearColor( 0.4f, 0.4f, 0.4f, 1.f );
		check();
		glClearDepthf(1.f);
		check();
		glClear( GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);
		check();
	}
	else
	{
		// Clear frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		check();
		glClearColor( 0.4f, 0.4f, 0.4f, 1.f );
		check();
		glClearDepthf(1.f);
		check();
		glClear( GL_COLOR_BUFFER_BIT |GL_DEPTH_BUFFER_BIT);			// doesn't make any difference in terms of time whether we do it or not
		check();
	}

	// Draw left eye
	OVR::Util::Render::StereoEyeParams leftEye = mStereoConfig.GetEyeRenderParams(OVR::Util::Render::StereoEye_Left);
	drawForEye( leftEye );
	
	// Draw right eye
	OVR::Util::Render::StereoEyeParams rightEye = mStereoConfig.GetEyeRenderParams(OVR::Util::Render::StereoEye_Right);
	drawForEye( rightEye );

	unsigned int time2 = OVR::Timer::GetTicksMs();
	unsigned int drawTime = time2 - time;

	// Present the result
	eglSwapBuffers(context.display, context.surface);
	check();
	mCounter++;

	unsigned int time3 = OVR::Timer::GetTicksMs();
	unsigned int swapTime = time3 - time2;

	if ( displayDrawTime )
		printf("draw:%d swap:%d\n", drawTime, swapTime );

}

void RiftOnThePiApp::drawForEye( const OVR::Util::Render::StereoEyeParams& stereoEyeParam )
{
	if ( mStereoRenderTechnique==NoCorrection )
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		check();
		OVR::Util::Render::Viewport svp = stereoEyeParam.VP;
		glViewport( svp.x, svp.y, svp.w, svp.h );		
		check();
		drawBox( stereoEyeParam.Projection, stereoEyeParam.ViewAdjust );
		glFlush();
		check();
		glFinish();
		check();
	}
	else
	{
		// Draw the box inside the render texture (which can be larger than the screen resolution)
		glBindFramebuffer(GL_FRAMEBUFFER, mTextureFrameBuffer);
		check();
		float sceneRenderScale = stereoEyeParam.pDistortion->Scale; 
		OVR::Util::Render::Viewport svp = stereoEyeParam.VP;
		svp.w = (int)ceil(sceneRenderScale * stereoEyeParam.VP.w);	// See void RenderDevice::SetViewport(const Viewport& vp) in RenderDevice.cpp
		svp.h = (int)ceil(sceneRenderScale * stereoEyeParam.VP.h);
		svp.x = (int)ceil(sceneRenderScale * stereoEyeParam.VP.x);
		svp.y = (int)ceil(sceneRenderScale * stereoEyeParam.VP.y);
		glViewport( svp.x, svp.y, svp.w, svp.h );		
		check();
		drawBox( stereoEyeParam.Projection, stereoEyeParam.ViewAdjust );
		glFlush();
		check();
		glFinish();
		check();

		// Draw the render texture in a quad covering the screen
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		check();
		glViewport( stereoEyeParam.VP.x, stereoEyeParam.VP.y, stereoEyeParam.VP.w, stereoEyeParam.VP.h );
		check();
		// The sign of the Distortion.XCenterOffset value must be chagned for the right eye. 
		// This tweak is done in Render_Tiny::SetDistortionConfig().
		// It's weird this isn't taken care of by the StereoEyeParam object!
		OVR::Util::Render::DistortionConfig distortionConfig = *stereoEyeParam.pDistortion;
		if ( stereoEyeParam.Eye==OVR::Util::Render::StereoEye_Right )
			distortionConfig.XCenterOffset = -distortionConfig.XCenterOffset;
		drawQuad( stereoEyeParam.VP, distortionConfig );
		glFlush();
		check();
		glFinish();
		check();
	}
}

void RiftOnThePiApp::drawBox( const OVR::Matrix4f& projectionMat, const OVR::Matrix4f& viewAdjustMat )
{  
	glEnable(GL_CULL_FACE);
	check();
	glEnable(GL_DEPTH_TEST);
	check();
	glDepthFunc(GL_LEQUAL);
	check();

	glUseProgram( mShaderProgramBox );
	check();
		
	glUniformMatrix4fv(mBoxProjectionUniform, 1, 0, reinterpret_cast<const float*>(projectionMat.Transposed().M) );		
	check();

	float x = 0.f; 
	float y = 0.f;
	float z = -3.f;
	//float ax = mBoxAngleX / 360.f * (2.f * gPi);
	float az = mBoxAngleY / 360.f * (2.f * gPi);
	float ay = mBoxAngleZ / 360.f * (2.f * gPi);
	OVR::Matrix4f modelViewMat;
	modelViewMat.SetIdentity();
	modelViewMat = OVR::Matrix4f::RotationZ(az) * modelViewMat;
	modelViewMat = OVR::Matrix4f::RotationY(ay) * modelViewMat;
	modelViewMat = OVR::Matrix4f::Translation( x, y, z ) * modelViewMat;
	modelViewMat = viewAdjustMat * modelViewMat;
	
	// Rift orientation
	if ( mUseRiftOrientation )
	{
		OVR::Quatf orientation = mSensorFusion->GetOrientation(); 
		OVR::Matrix4f orientationMat = orientation;
		modelViewMat = (orientationMat.Inverted()) * modelViewMat;
	}

	glUniformMatrix4fv(mBoxModelViewUniform, 1, 0, reinterpret_cast<float*>( modelViewMat.Transposed().M ) );
	check();

	glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferBox);
	check();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBufferBox);
	check();
			
	// Define the vertex format
	glVertexAttribPointer(mBoxPositionAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(VertexWithColor), 0);
	check();
	glVertexAttribPointer(mBoxColorAttrib, 4, GL_FLOAT, GL_FALSE, sizeof(VertexWithColor), (GLvoid*) (sizeof(float) * 3));
	check();
	glEnableVertexAttribArray(mBoxPositionAttrib);
	check();
	glEnableVertexAttribArray(mBoxColorAttrib);
	check();
	
	glDrawElements(GL_TRIANGLES, sizeof(IndicesBox)/sizeof(IndicesBox[0]), GL_UNSIGNED_BYTE, 0);
	check();
}

void RiftOnThePiApp::drawQuad( const OVR::Util::Render::Viewport& VP, const OVR::Util::Render::DistortionConfig& distortionConfig )		
{
	glUseProgram( mShaderProgramQuad );
	check();
	
	float w = float(VP.w) / float(mScreenHResolution);
	float h = float(VP.h) / float(mScreenVResolution);
	float x = float(VP.x) / float(mScreenHResolution);
	float y = float(VP.y) / float(mScreenVResolution);
	if ( mStereoRenderTechnique!=NoCorrection )
	{
		OVR::Matrix4f texm(	w, 0, 0, x,
						0, h, 0, y,
						0, 0, 0, 0,
						0, 0, 0, 1);
		glUniformMatrix4fv(mQuadTexmUniform, 1, 0, reinterpret_cast<float*>( texm.Transposed().M ) );
		check();
	}

	if ( mStereoRenderTechnique==RenderTextureDistortionCorrection ||
		 mStereoRenderTechnique==RenderTextureDistortionAndChromaCorrection )
	{
		float as = float(VP.w) / float(VP.h);
		float scaleFactor = 1.0f / distortionConfig.Scale;		

		float lensCenter[2];
		lensCenter[0] = x + (w + distortionConfig.XCenterOffset * 0.5f)*0.5f;
		lensCenter[1] = y + h * 0.5f;
		glUniform2fv(mQuadLensCenterUniform, 1, lensCenter  );
		check();
		
		float screenCenterCenter[2];
		screenCenterCenter[0] = x + w*0.5f;
		screenCenterCenter[1] = y + h*0.5f;
		glUniform2fv(mQuadScreenCenterCenterUniform, 1, screenCenterCenter  );
		check();

		float scaleCenter[2];
		scaleCenter[0] = (w/2.f) * scaleFactor;
		scaleCenter[1] = (h/2.f) * scaleFactor * as;
		glUniform2fv(mQuadScaleCenterUniform, 1, scaleCenter  );
		check();
		
		float scaleInCenter[2];
		scaleInCenter[0] = (2.f/w);
		scaleInCenter[1] = (2.f/h) / as;
		glUniform2fv(mQuadScaleInCenterUniform, 1, scaleInCenter  );
		check();
	
		float hmdWarpParamCenter[4];
		hmdWarpParamCenter[0] = distortionConfig.K[0];
		hmdWarpParamCenter[1] = distortionConfig.K[1];
		hmdWarpParamCenter[2] = distortionConfig.K[2];
		hmdWarpParamCenter[3] = distortionConfig.K[3];
		glUniform4fv(mQuadHmdWarpParamCenterUniform, 1, hmdWarpParamCenter );
		check();
	}
	
	if ( mStereoRenderTechnique==RenderTextureDistortionAndChromaCorrection )
	{
		float chromAbParam[4];
		chromAbParam[0] = distortionConfig.ChromaticAberration[0];
		chromAbParam[1] = distortionConfig.ChromaticAberration[1];
		chromAbParam[2] = distortionConfig.ChromaticAberration[2];
		chromAbParam[3] = distortionConfig.ChromaticAberration[3];
		glUniform4fv(mQuadChromAbParamUniform, 1, chromAbParam );
		check();
	}
	
	glActiveTexture( GL_TEXTURE0 );
	check();
	glBindTexture( GL_TEXTURE_2D, mTexture );
	check();
	glUniform1i( mQuadTexture0Uniform, 0 );
	check();
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_BLEND);
	glBindBuffer(GL_ARRAY_BUFFER, mVertexBufferQuad);
	check();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBufferQuad);
	check();
	
	check();
	glVertexAttribPointer(mQuadPositionAttrib, 3, GL_FLOAT, GL_FALSE, sizeof(VertexWithUV), 0);
	check();
	glVertexAttribPointer(mQuadInputTexCoordAttrib, 2, GL_FLOAT, GL_FALSE, sizeof(VertexWithUV), (GLvoid*) (sizeof(float) * 3));
	check();
	
	glEnableVertexAttribArray(mQuadPositionAttrib);
	check();
	glEnableVertexAttribArray(mQuadInputTexCoordAttrib);
	check();
	glDrawElements(GL_TRIANGLES, sizeof(IndicesQuad)/sizeof(IndicesQuad[0]), GL_UNSIGNED_BYTE, 0);
	check();
	//glDisable(GL_BLEND);
}

}