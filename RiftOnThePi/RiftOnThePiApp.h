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

#include "OGLESApplication.h"

#include "OVR.h"

namespace OGLESSandbox
{

class RiftOnThePiApp : public Application
{
public:
	enum StereoRenderTechnique
	{
		NoCorrection,								// For each eye, the scene is rendered directly to the frame buffer 
		RenderTextureNoDistortionCorrection,		// For each eye, the scene is rendered in a texture first then on screen with a shader that doesn't correct distortion. 
													// For benchmark/test purpose only as final image isn't exactly Rift-correct
		RenderTextureDistortionCorrection,			// For each eye, the scene is rendered in a texture first then on screen with a shader that corrects distortion
		RenderTextureDistortionAndChromaCorrection,	// For each eye, the scene is rendered in a texture first then on screen with a shader that corrects distortion and chromatic aberration
	};

	RiftOnThePiApp();
	virtual bool initialize( const ApplicationContext& context );
	virtual void draw( const ApplicationContext& context );

private:
	void	readParameters( const ApplicationContext& context );
	bool	initOculus();
	void	createShaderPrograms();
	void	createGeometries();
	void	createTexture();

	void	drawForEye( const OVR::Util::Render::StereoEyeParams& stereoEyeParam );
	void	drawBox( const OVR::Matrix4f& projectionMat, const OVR::Matrix4f& viewAdjustMat );
	void	drawQuad(  const OVR::Util::Render::Viewport& VP, const OVR::Util::Render::DistortionConfig& distortionConfig );
	
	int				mCounter;
	unsigned int	mLastTime;

	StereoRenderTechnique mStereoRenderTechnique;
	bool	mDistortionScaleEnabled;					// If distortion correction is enabled, indicate whether we enlarge the render target texture and FOV to take the most of the Rift FOV
	bool	mAnimationEnabled;							// Is the box rotating
	bool	mUseRiftOrientation;				

	OVR::Ptr<OVR::DeviceManager>	mDeviceManager;
	OVR::Ptr<OVR::HMDDevice>		mHMD;
	OVR::Ptr<OVR::SensorDevice>		mSensor;
	OVR::SensorFusion*				mSensorFusion;
	OVR::Util::Render::StereoConfig mStereoConfig;
	unsigned int					mScreenHResolution;
	unsigned int					mScreenVResolution; 

	float	mBoxAngleX;		// In degrees
	float	mBoxAngleY;
	float	mBoxAngleZ;

	GLuint	mShaderProgramBox;
	GLuint	mVertexBufferBox;
	GLuint	mIndexBufferBox;

	GLuint	mShaderProgramQuad;
	GLuint	mVertexBufferQuad;
	GLuint	mIndexBufferQuad;

	GLuint	mTexture;
	GLuint	mTextureFrameBuffer;

	GLint mBoxProjectionUniform;
	GLint mBoxModelViewUniform;
	GLint mBoxPositionAttrib;
	GLint mBoxColorAttrib;

	GLint	mQuadTexmUniform;
	GLint	mQuadLensCenterUniform;
	GLint	mQuadScreenCenterCenterUniform;
	GLint	mQuadScaleCenterUniform;
	GLint	mQuadScaleInCenterUniform;
	GLint	mQuadHmdWarpParamCenterUniform;
	GLint	mQuadChromAbParamUniform;
	GLint	mQuadTexture0Uniform;
	GLint	mQuadPositionAttrib;
	GLint	mQuadInputTexCoordAttrib;
};

}
