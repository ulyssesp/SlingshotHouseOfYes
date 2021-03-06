#include "FireCue.h"

using namespace ci;

FireCue::FireCue(const World & world)
	//:mCam( app::getWindowWidth(), app::getWindowHeight(), 90.0f, 0.1f, 20.0f )
{
	//mCam.lookAt( vec3( 0.0f, 0.0f, 2.0f ), vec3( 0 ) );

	vec2 fluidResolution = vec2(1024, 512);
	mFluid = Fluid(fluidResolution);

	gl::GlslProg::Format updateFormat;
	updateFormat.vertex(app::loadAsset("Shaders/passthru.vert"));
	//updateFormat.geometry(app::loadAsset("Shaders/Fluid/2d/raycast.geom"));
	updateFormat.fragment(app::loadAsset("Shaders/Fluid/2D/fire_draw.frag"));
	//mRaycastShader = gl::GlslProg::create(updateFormat);
	//mRaycastShader->uniform("i_resolution", world.windowSize);
	//mRaycastShader->uniform("i_origin", mCam.getEyePoint());
	mSmokeDrawShader = gl::GlslProg::create(updateFormat);
	mSmokeDrawShader->uniform("i_resolution", world.windowSize);

	//updateFormat.vertex(app::loadAsset("Shaders/Fluid/pick.vert"));
	//updateFormat.geometry(app::loadAsset("Shaders/Fluid/pick.geom"));
	updateFormat.fragment(app::loadAsset("Shaders/Fluid/2D/fire_drop_forces.frag"));
	mForcesShader = gl::GlslProg::create(updateFormat);
	mForcesShader->uniform("i_resolution", fluidResolution);

	updateFormat.fragment(app::loadAsset("Shaders/Fluid/2D/fire_drop.frag"));
	mSmokeDropShader = gl::GlslProg::create(updateFormat);
	mSmokeDropShader->uniform("i_resolution", world.windowSize);
	mSmokeDropShader->uniform("i_smokeDropPos", vec2(0.5, 0.8));

	updateFormat.fragment(app::loadAsset("Shaders/Fluid/2D/fire_drop_map.frag"));
	mFireDropTexShader = gl::GlslProg::create(updateFormat);
	mFireDropTexShader->uniform("i_resolution", world.windowSize);
	mFireDropTexShader->uniform("i_mult", 1.0f);

	gl::Texture2d::Format texFmt;
	texFmt.setInternalFormat(GL_RGBA16F);
	texFmt.setDataType(GL_HALF_FLOAT);
	texFmt.setTarget(GL_TEXTURE_2D);
	texFmt.setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	gl::Fbo::Format fmt;
	fmt.disableDepth()
		.setColorTextureFormat(texFmt);
	mSmokeField = PingPongFBO(fmt, world.windowSize, 2);

	mHoYTex = gl::Texture::create(loadImage(app::loadAsset("Images/HOYFire.png")));

	mFireDropTex = PingPongFBO(fmt, world.windowSize, 2);

	gl::GlslProgRef drawTex = gl::getStockShader(gl::ShaderDef().texture(mHoYTex));

	{
		gl::ScopedTextureBind hoy(mHoYTex);
		mFireDropTex.render(drawTex);
	}

	world.oscController->subscribe("/hoy/cues/fire/intensity/value", [=](const osc::Message message) {
		mFireDropTexShader->uniform("i_mult", message.getArgFloat(0));
	});
}

void FireCue::update(const World & world)
{
	mForcesShader->uniform("i_dt", world.dt);
	mForcesShader->uniform("i_time", world.time);
	mFluid.update(world.dt, mForcesShader, mSmokeField.getTexture(), 0.98);

	// Use the fluid to advect the smoke
	mFluid.advect(world.dt, &mSmokeField, 0.98);


	gl::ScopedTextureBind scopeFireTexDrop(mFireDropTex.getTexture(), 2);
	mFireDropTexShader->uniform("tex_fire", 2);

	mFireDropTex.render(mFireDropTexShader);

	gl::ScopedTextureBind scopeSmokeDrop(mSmokeField.getTexture(), 0);
	mSmokeDropShader->uniform("tex_prev", 0);
	gl::ScopedTextureBind hoyTex(mFireDropTex.getTexture(), 1);
	mSmokeDropShader->uniform("tex_hoy", 1);
	mSmokeDropShader->uniform("i_dt", world.dt);
	mSmokeDropShader->uniform("i_time", world.time);
	mSmokeField.render(mSmokeDropShader);
}

void FireCue::draw(const World & world)
{
	gl::ScopedTextureBind smokeTex(mSmokeField.getTexture(), 0);
	//gl::ScopedTextureBind smokeTex(mFluid.getVelocityTexture(), 0);
	//mRaycastShader->uniform("tex_smoke", 0);
	mSmokeDrawShader->uniform("tex", 0);

	gl::ScopedViewport vp(ivec2(0), world.windowSize);
	//gl::ScopedGlslProg glsl(mRaycastShader);
	gl::ScopedGlslProg glsl(mSmokeDrawShader);

	gl::enableDepthRead();
	gl::enableDepthWrite();

	gl::pushMatrices();
	//gl::setMatrices(mCam);
	gl::setMatricesWindow(world.windowSize);

	//mBatch->draw();
	gl::drawSolidRect(Rectf(vec2(0), world.windowSize));

	gl::popMatrices();
}