#ifndef GXLIGHT_H
#define GXLIGHT_H

#include <cstring>
#include <d3d9.h>

#include "../gfx/gfxlight.h"

class gxSceneD3D9;

class gxLightD3D9 : public gxLight {
public:
	gxLightD3D9(gxSceneD3D9* scene, int type);
	~gxLightD3D9();

	D3DLIGHT9 d3d_light;

private:
	gxSceneD3D9* scene;

	/***** GX INTERFACE *****/
public:
	void setRange(float range) override;

	void setColor(const float rgb[3]) override {
		memcpy(&d3d_light.Diffuse, rgb, sizeof(float) * 3);
		d3d_light.Diffuse.a = 1.0f;
	}

	void setPosition(const float pos[3]) override;
	void setDirection(const float dir[3]) override;
	void setConeAngles(float inner, float outer) override;

	void getColor(float rgb[3]) override {
		memcpy(rgb, &d3d_light.Diffuse, sizeof(float) * 3);
	}
};

#endif