#ifndef GXEFFECT_H
#define GXEFFECT_H

#include <d3d9.h>
#include <d3dx9.h>
#include <string>
#include <unordered_map>

#include "../gfx/gfxeffect.h"

class gxGraphicsD3D9;
class gxCanvasD3D9;

class gxEffectD3D9 : public gxEffect {
public:
    gxEffectD3D9(gxGraphicsD3D9* gfx, ID3DXEffect* effect);
    ~gxEffectD3D9();

    void onLostDevice();
    void onResetDevice();

    bool setFloat(const std::string& name, float value) override;
    bool setVector(const std::string& name, const float vec[4]) override;
    bool setMatrix(const std::string& name, const float mat[16]) override;
    bool setMatrix(const std::string& name, const D3DXMATRIX& mat);
    void setAutoMatrices(const D3DXMATRIX& world, const D3DXMATRIX& view, const D3DXMATRIX& proj);
    bool setTexture(const std::string& name, gxCanvas* canvas) override;
    bool setTexture(const std::string& name, IDirect3DBaseTexture9* tex);

    bool begin(UINT* passes) override;
    bool beginPass(UINT pass) override;
    bool endPass() override;
    bool end() override;

    ID3DXEffect* getEffect() const { return effect; }

private:
    gxGraphicsD3D9* graphics;
    ID3DXEffect* effect;
    std::unordered_map<std::string, D3DXHANDLE> handleCache;

    D3DXHANDLE getHandle(const std::string& name);
};

#endif