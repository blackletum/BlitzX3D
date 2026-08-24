#include "std.h"
#include "gxeffect.h"
#include "gxgraphics.h"
#include <cstring>

gxEffectD3D9::gxEffectD3D9(gxGraphicsD3D9* gfx, ID3DXEffect* e)
    : graphics(gfx), effect(e) {
    effect->AddRef();
}

gxEffectD3D9::~gxEffectD3D9() {
    if (effect) effect->Release();
}

void gxEffectD3D9::onLostDevice() {
    if (effect) effect->OnLostDevice();
}

void gxEffectD3D9::onResetDevice() {
    if (effect) effect->OnResetDevice();
}

D3DXHANDLE gxEffectD3D9::getHandle(const std::string& name) {
    auto it = handleCache.find(name);
    if (it != handleCache.end()) return it->second;
    D3DXHANDLE h = effect->GetParameterByName(nullptr, name.c_str());
    handleCache[name] = h;
    return h;
}

bool gxEffectD3D9::setFloat(const std::string& name, float value) {
    D3DXHANDLE h = getHandle(name);
    if (!h) return false;
    return SUCCEEDED(effect->SetFloat(h, value));
}

bool gxEffectD3D9::setVector(const std::string& name, const float vec[4]) {
    D3DXHANDLE h = getHandle(name);
    if (!h) return false;
    return SUCCEEDED(effect->SetFloatArray(h, vec, 4));
}

bool gxEffectD3D9::setMatrix(const std::string& name, const D3DXMATRIX& mat) {
    D3DXHANDLE h = getHandle(name);
    if (!h) return false;
    return SUCCEEDED(effect->SetMatrix(h, &mat));
}

bool gxEffectD3D9::setMatrix(const std::string& name, const float mat[16]) {
    D3DXMATRIX m;
    memcpy(&m, mat, sizeof(m));
    return setMatrix(name, m);
}

void gxEffectD3D9::setAutoMatrices(const D3DXMATRIX& world,
    const D3DXMATRIX& view,
    const D3DXMATRIX& proj) {
    D3DXMATRIX wv = world * view;
    D3DXMATRIX wvp = wv * proj;

    setMatrix("World", world);
    setMatrix("View", view);
    setMatrix("Projection", proj);
    setMatrix("WorldView", wv);
    setMatrix("WorldViewProj", wvp);
}

bool gxEffectD3D9::setTexture(const std::string& name, IDirect3DBaseTexture9* tex) {
    if (!effect) return false;
    D3DXHANDLE h = getHandle(name);
    if (!h) return false;
    HRESULT hr = effect->SetTexture(h, tex);
    return SUCCEEDED(hr);
}

bool gxEffectD3D9::setTexture(const std::string& name, gxCanvas* canvas) {
    if (!canvas) return false;
    gxCanvasD3D9* c = static_cast<gxCanvasD3D9*>(canvas);
    return setTexture(name, c->getTexSurface());
}

bool gxEffectD3D9::begin(UINT* passes) {
    return SUCCEEDED(effect->Begin(passes, 0));
}

bool gxEffectD3D9::beginPass(UINT pass) {
    return SUCCEEDED(effect->BeginPass(pass));
}

bool gxEffectD3D9::endPass() {
    return SUCCEEDED(effect->EndPass());
}

bool gxEffectD3D9::end() {
    return SUCCEEDED(effect->End());
}