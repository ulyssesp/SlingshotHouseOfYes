#pragma once

#include "cinder/gl/gl.h"

#include "Cue.h"
#include "Fluid.h"

class FireCue : public Cue {
public:
	FireCue(const World& world);
	void update(const World& world) override;
	void draw(const World& world) override;

private:
	Fluid mFluid;
	PingPongFBO mSmokeField,
		mFireDropTex;

	ci::gl::GlslProgRef mForcesShader,
		mSmokeDropShader,
		mSmokeDrawShader,
		mFireDropTexShader;
		//mRaycastShader;

	ci::gl::TextureRef mHoYTex;

	//ci::gl::VertBatchRef mBatch;
	//ci::CameraPersp	mCam;
};
