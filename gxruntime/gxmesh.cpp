#include "std.h"
#include "gxmesh.h"
#include "gxgraphics.h"

#include "gxruntime.h"
#include "sdlgpu/sdl_gpu_mesh.h"

extern gxRuntime* gx_runtime;

gxMesh::gxMesh(gxGraphics* g, IDirect3DVertexBuffer9* vs, IDirect3DIndexBuffer9* is,
    int max_vs, int max_ts) :
    graphics(g), vertex_buff(vs), index_buff(is), vertex_decl(nullptr),
    locked_verts(nullptr), locked_skin_verts(nullptr), locked_indices(nullptr),
    gpu_dirty_vmin(-1), gpu_dirty_vmax(-1), gpu_dirty_tmin(-1), gpu_dirty_tmax(-1), gpu_uploaded(false),
    max_verts(max_vs), max_tris(max_ts), mesh_dirty(false), skinned(false) {
    if (g && g->runtime && g->runtime->sdlGpu) {
        gpuMirror = sdlgpu::CreateMesh(g->runtime->sdlGpu, sizeof(dxVertex), max_vs, max_ts);
    }
}

gxMesh::gxMesh(gxGraphics* g, IDirect3DVertexBuffer9* vs, IDirect3DIndexBuffer9* is,
    IDirect3DVertexDeclaration9* decl, int max_vs, int max_ts) :
    graphics(g), vertex_buff(vs), index_buff(is), vertex_decl(decl),
    locked_verts(nullptr), locked_skin_verts(nullptr), locked_indices(nullptr),
    gpu_dirty_vmin(-1), gpu_dirty_vmax(-1), gpu_dirty_tmin(-1), gpu_dirty_tmax(-1), gpu_uploaded(false),
    max_verts(max_vs), max_tris(max_ts), mesh_dirty(false), skinned(true) {
    if (g && g->runtime && g->runtime->sdlGpu) {
        gpuMirror = sdlgpu::CreateMesh(g->runtime->sdlGpu, sizeof(dxSkinVertex), max_vs, max_ts);
    }
}

gxMesh::~gxMesh() {
    unlock();
    if (graphics && graphics->runtime && gpuMirror) {
        sdlgpu::ReleaseMesh(graphics->runtime->sdlGpu, gpuMirror);
        gpuMirror = nullptr;
    }
    if (vertex_buff) { vertex_buff->Release(); vertex_buff = nullptr; }
    if (index_buff) { index_buff->Release();  index_buff = nullptr; }
}

bool gxMesh::lock(bool all) {
    if ((locked_verts || locked_skin_verts) && locked_indices) return true;

    // lock vert buffer
    if (skinned) {
        if (!locked_skin_verts) {
            DWORD vflags = D3DLOCK_NOSYSLOCK | (all ? D3DLOCK_DISCARD : D3DLOCK_NOOVERWRITE);
            void* ptr = nullptr;
            if (FAILED(vertex_buff->Lock(0, 0, &ptr, vflags))) {
                return false;
            }
            locked_skin_verts = reinterpret_cast<dxSkinVertex*>(ptr);
        }
    }
    else if (!locked_verts) {
        DWORD vflags = D3DLOCK_NOSYSLOCK | (all ? D3DLOCK_DISCARD : D3DLOCK_NOOVERWRITE);
        void* ptr = nullptr;
        if (FAILED(vertex_buff->Lock(0, 0, &ptr, vflags))) {
            return false;
        }
        locked_verts = reinterpret_cast<dxVertex*>(ptr);
    }

    // lock index buffer
    if (!locked_indices) {
        DWORD iflags = D3DLOCK_NOSYSLOCK | (all ? D3DLOCK_DISCARD : D3DLOCK_NOOVERWRITE);
        void* ptr = nullptr;
        if (FAILED(index_buff->Lock(0, 0, &ptr, iflags))) {
            if (locked_verts) { vertex_buff->Unlock(); locked_verts = nullptr; }
            if (locked_skin_verts) { vertex_buff->Unlock(); locked_skin_verts = nullptr; }
            return false;
        }
        locked_indices = reinterpret_cast<WORD*>(ptr);
    }

    if (all) markGpuFullDirty();
    mesh_dirty = false;
    return true;
}

void gxMesh::unlock() {
    if (gpuMirror && graphics && graphics->runtime && graphics->runtime->sdlGpu) {
        const void* verts = skinned ? (const void*)locked_skin_verts : (const void*)locked_verts;
        unsigned stride = skinned ? sizeof(dxSkinVertex) : sizeof(dxVertex);
        if (verts && locked_indices) {
            SDL_GPUDevice* dev = (SDL_GPUDevice*)graphics->runtime->sdlGpu;
            bool ok = true;
            if (!gpu_uploaded) {
                ok = sdlgpu::UploadMesh(dev, gpuMirror,
                    verts, stride * (unsigned)max_verts,
                    locked_indices, (unsigned)sizeof(WORD) * (unsigned)max_tris * 3);
            } else if (gpu_dirty_vmin >= 0 || gpu_dirty_tmin >= 0) {
                unsigned vOff = 0, vBytes = 0, iOff = 0, iBytes = 0;
                if (gpu_dirty_vmin >= 0) {
                    vOff = (unsigned)gpu_dirty_vmin * stride;
                    vBytes = (unsigned)(gpu_dirty_vmax - gpu_dirty_vmin + 1) * stride;
                }
                if (gpu_dirty_tmin >= 0) {
                    iOff = (unsigned)gpu_dirty_tmin * 3 * (unsigned)sizeof(WORD);
                    iBytes = (unsigned)(gpu_dirty_tmax - gpu_dirty_tmin + 1) * 3 * (unsigned)sizeof(WORD);
                }
                if (vBytes || iBytes) {
                    const char* vb = (const char*)verts + vOff;
                    const char* ib = (const char*)locked_indices + iOff;
                    ok = sdlgpu::UploadMeshRange(dev, gpuMirror, vb, vOff, vBytes, ib, iOff, iBytes);
                }
            }
            if (ok) {
                gpu_dirty_vmin = gpu_dirty_vmax = gpu_dirty_tmin = gpu_dirty_tmax = -1;
                gpu_uploaded = true;
            }
        }
    }
    if (locked_verts) {
        vertex_buff->Unlock();
        locked_verts = nullptr;
    }
    if (locked_skin_verts) {
        vertex_buff->Unlock();
        locked_skin_verts = nullptr;
    }
    if (locked_indices) {
        index_buff->Unlock();
        locked_indices = nullptr;
    }
}

void gxMesh::backup() {
	unlock();
}

void gxMesh::restore() {
	mesh_dirty = true;
}

void gxMesh::render(int first_vert, int vert_cnt, int first_tri, int tri_cnt, bool skipDxDraw) {
    unlock();
    if (skipDxDraw) return;

    IDirect3DDevice9* dev = graphics->dir3dDev;

    if (skinned) {
        dev->SetVertexDeclaration(vertex_decl);
        dev->SetStreamSource(0, vertex_buff, 0, sizeof(dxSkinVertex));
        dev->SetIndices(index_buff);
        dev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, first_vert, 0, vert_cnt, first_tri * 3, tri_cnt);
        return;
    }

    dev->SetStreamSource(0, vertex_buff, 0, sizeof(dxVertex));
    dev->SetFVF(VTXFMT);
    dev->SetIndices(index_buff);
    dev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, first_vert, 0, vert_cnt, first_tri * 3, tri_cnt);
}

void gxMesh::renderSkinned(int first_vert, int vert_cnt, int first_tri, int tri_cnt,
    const float* bone_data, int bone_cnt) {
    unlock();

    IDirect3DDevice9* dev = graphics->dir3dDev;
    IDirect3DVertexShader9* shader = graphics->getSkinningShader();
    if (!shader || !vertex_decl) return;

    if (bone_cnt > MAX_SKIN_BONES) bone_cnt = MAX_SKIN_BONES;

    dev->SetVertexShaderConstantF(0, bone_data, bone_cnt * 3);

    dev->SetVertexDeclaration(vertex_decl);
    dev->SetVertexShader(shader);
    dev->SetStreamSource(0, vertex_buff, 0, sizeof(dxSkinVertex));
    dev->SetIndices(index_buff);
    dev->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, first_vert, 0, vert_cnt, first_tri * 3, tri_cnt);

    dev->SetVertexShader(nullptr);
    dev->SetVertexDeclaration(nullptr);
}
