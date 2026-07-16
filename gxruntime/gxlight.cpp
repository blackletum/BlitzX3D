#include "std.h"
#include "gxlight.h"
#include "gxscene.h"
#include "gxgraphics.h"
#include "gxshadowmap.h"
#include "gxeffect.h"
#include "EmbeddedShaders.h"

// rest in peace incompetence
//static const char* SHADOWDEPTH_FX = "GFX/Shaders/ShadowDepth.fx";
//static const char* SHADOWLIT_FX = "GFX/Shaders/ShadowLit.fx";

gxLight::gxLight(gxScene* s, int type) :
	scene(s), shadow_enabled(false), shadow_range(1000.f),
	shadow_resolution(1024), shadow_map(nullptr),
	depth_effect(nullptr), lit_effect(nullptr) {

	memset(&d3d_light, 0, sizeof(d3d_light));

	switch (type) {
	case LIGHT_POINT:
		d3d_light.Type = D3DLIGHT_POINT;
		break;
	case LIGHT_SPOT:
		d3d_light.Type = D3DLIGHT_SPOT;
		break;
	default:
		d3d_light.Type = D3DLIGHT_DIRECTIONAL;
	}

	d3d_light.Diffuse.a = 1;
	d3d_light.Diffuse.r = d3d_light.Diffuse.g = d3d_light.Diffuse.b = 1;
	d3d_light.Specular.r = d3d_light.Specular.g = d3d_light.Specular.b = 1;
	d3d_light.Range = FLT_MAX;
	d3d_light.Theta = 0;
	d3d_light.Phi = HALFPI;
	d3d_light.Falloff = 1;
	d3d_light.Direction.z = 1;
	setRange(1000);
}

gxLight::~gxLight() {
	freeShadowResources();
}

void gxLight::freeShadowResources() {
	delete shadow_map; shadow_map = nullptr;
}

bool gxLight::setRealtimeShadow(bool enable, gxGraphics* graphics) {
	if (!enable) {
		if (graphics) {
			if (depth_effect) graphics->freeEffect(depth_effect);
			if (lit_effect) graphics->freeEffect(lit_effect);
		}
		depth_effect = lit_effect = nullptr;
		delete shadow_map; shadow_map = nullptr;
		shadow_enabled = false;
		return true;
	}

	if(!graphics) {
		shadow_enabled = false;
		return false;
	}

	bool is_point = (d3d_light.Type == D3DLIGHT_POINT);

	if(!shadow_map || shadow_map->getResolution() != shadow_resolution || shadow_map->isCube() != is_point) {
		delete shadow_map;
		shadow_map = new gxShadowMap(graphics, shadow_resolution, is_point);
	}
	if (!shadow_map->isValid()) {
		delete shadow_map; shadow_map = nullptr;
		shadow_enabled = false;
		return false;
	}

	const char* depthShader = is_point ? EmbeddedShaders::ShadowDepthCube : EmbeddedShaders::ShadowDepth;
	const char* litShader = is_point ? EmbeddedShaders::ShadowLitPoint : EmbeddedShaders::ShadowLit;

	if (!depth_effect) depth_effect = graphics->createEffect(depthShader, strlen(depthShader));
	if (!lit_effect) lit_effect = graphics->createEffect(litShader, strlen(litShader));
	if(!depth_effect || !lit_effect) {
		if(depth_effect) { graphics->freeEffect(depth_effect); depth_effect = nullptr; }
		if(lit_effect) { graphics->freeEffect(lit_effect); lit_effect = nullptr; }
		delete shadow_map; shadow_map = nullptr;
		shadow_enabled = false;
		return false;
	}

	shadow_enabled = true;
	return true;
}

bool gxLight::hasRealtimeShadow()const {
	return shadow_enabled && shadow_map && shadow_map->isValid() && depth_effect && lit_effect;
}

gxEffect* gxLight::getDepthEffect(gxGraphics* graphics) {
	return graphics && graphics->verifyEffect(depth_effect) ? depth_effect : nullptr;
}

gxEffect* gxLight::getLitEffect(gxGraphics* graphics) {
	return graphics && graphics->verifyEffect(lit_effect) ? lit_effect : nullptr;
}

void gxLight::setRange(float r) {
    d3d_light.Attenuation1 = 1.0f / r;
}

void gxLight::setPosition(const float pos[3]) {
    d3d_light.Position.x = pos[0];
    d3d_light.Position.y = pos[1];
    d3d_light.Position.z = pos[2];
}

void gxLight::setDirection(const float dir[3]) {
    d3d_light.Direction.x = dir[0];
    d3d_light.Direction.y = dir[1];
    d3d_light.Direction.z = dir[2];
}

void gxLight::setConeAngles(float inner, float outer) {
    d3d_light.Theta = inner;
    d3d_light.Phi = outer;
}