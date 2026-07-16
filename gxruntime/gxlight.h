#ifndef GXLIGHT_H
#define GXLIGHT_H

#include <cstring>
#include <string>
#include <d3d9.h>

class gxScene;
class gxGraphics;
class gxShadowMap;
class gxEffect;

class gxLight {
public:
	gxLight(gxScene* scene, int type);
	~gxLight();

	D3DLIGHT9 d3d_light;

private:
	gxScene* scene;

	/***** GX INTERFACE *****/
public:
	enum {
		LIGHT_DISTANT = 1, LIGHT_POINT = 2, LIGHT_SPOT = 3
	};
	void setRange(float range);

	void setColor(const float rgb[3]) {
		memcpy(&d3d_light.Diffuse, rgb, sizeof(float) * 3);
		d3d_light.Diffuse.a = 1.0f;
	}

	void setPosition(const float pos[3]);
	void setDirection(const float dir[3]);
	void setConeAngles(float inner, float outer);

	void getColor(float rgb[3]) {
		memcpy(rgb, &d3d_light.Diffuse, sizeof(float) * 3);
	}

	bool setRealtimeShadow(bool enable, gxGraphics* graphics);
	bool hasRealtimeShadow()const;

	void setShadowRange(float range) { shadow_range = range; }
	float getShadowRange()const { return shadow_range; }

	void setShadowResolution(int resolution) { shadow_resolution = resolution; }
	int getShadowResolution()const { return shadow_resolution; }

	gxShadowMap* getShadowMap()const { return shadow_map; }
	bool isPointShadow()const { return hasRealtimeShadow() && d3d_light.Type == D3DLIGHT_POINT; }

	gxEffect* getDepthEffect(gxGraphics* graphics);
	gxEffect* getLitEffect(gxGraphics* graphics);

private:
	bool shadow_enabled;
	float shadow_range;
	int shadow_resolution;
	gxShadowMap* shadow_map;
	gxEffect* depth_effect;
	gxEffect* lit_effect;

	void freeShadowResources();
};

#endif