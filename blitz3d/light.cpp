#include "std.h"
#include "light.h"
#include "../gxruntime/gxscene.h"
#include "../gxruntime/gxgraphics.h"

extern gxScene* gx_scene;
extern gxGraphics* gx_graphics;

Light::Light(int type) {
	light = gx_scene->createLight(type);
}

Light::~Light() {
	if (gx_scene) gx_scene->freeLight(light);
}

void Light::setRange(float r) {
	light->setRange(r);
}

void Light::setColor(const Vector& v) {
	light->setColor((float*)&v.x);
}

void Light::setConeAngles(float inner, float outer) {
	light->setConeAngles(inner, outer);
}

bool Light::beginRender(float tween) {
	Object::beginRender(tween);
	light->setPosition(&getRenderTform().v.x);
	light->setDirection(&getRenderTform().m.k.x);
	return true;
}

bool Light::setRealtimeShadow(bool enable) {
	return light->setRealtimeShadow(enable, gx_graphics);
}

bool Light::hasRealtimeShadow()const {
	return light->hasRealtimeShadow();
}

void Light::setShadowRange(float range) {
	light->setShadowRange(range);
}

void Light::setShadowResolution(int resolution) {
	light->setShadowResolution(resolution);
}