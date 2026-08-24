#ifndef GXMESH_H
#define GXMESH_H

#include <d3d9.h>
#include <d3dx9.h>

#include "../gfx/gfxmesh.h"

class gxGraphicsD3D9;

class gxMeshD3D9 : public gxMesh {
public:
    typedef gxMesh::Vertex dxVertex;
    typedef gxMesh::SkinVertex dxSkinVertex;

    gxMeshD3D9(gxGraphicsD3D9* graphics, IDirect3DVertexBuffer9* verts, IDirect3DIndexBuffer9* indices, int max_verts, int max_tris);
    gxMeshD3D9(gxGraphicsD3D9* graphics, IDirect3DVertexBuffer9* verts, IDirect3DIndexBuffer9* indices, IDirect3DVertexDeclaration9* decl, int max_verts, int max_tris);
    ~gxMeshD3D9();

    int maxVerts() const override { return max_verts; }
    int maxTris()  const override { return max_tris; }

    bool dirty() const override { return mesh_dirty; }
    bool isSkinned() const override { return skinned; }

    void render(int first_vert, int vert_cnt, int first_tri, int tri_cnt) override;
    void renderSkinned(int first_vert, int vert_cnt, int first_tri, int tri_cnt, const float* bone_data, int bone_cnt) override;

    void backup();
    void restore();

    static const DWORD VTXFMT = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEXCOORDSIZE2(1);

private:
    gxGraphicsD3D9* graphics;
    IDirect3DVertexBuffer9* vertex_buff;
    IDirect3DIndexBuffer9* index_buff;
    IDirect3DVertexDeclaration9* vertex_decl;

    int  max_verts, max_tris;
    bool mesh_dirty;
    bool skinned;
    dxVertex* locked_verts;
    dxSkinVertex* locked_skin_verts;
    WORD* locked_indices;

    /***** GX INTERFACE *****/
public:
    bool lock(bool all) override;
    void unlock() override;

    void setVertex(int n, const void* v) override {
        memcpy(locked_verts + n, v, sizeof(dxVertex));
    }
    void setVertex(int n, const float coords[3], const float normal[3], const float tex_coords[2][2]) override {
        dxVertex* t = locked_verts + n;
        memcpy(t->coords, coords, 12);
        memcpy(t->normal, normal, 12);
        t->argb = 0xffffffff;
        memcpy(t->tex_coords, tex_coords, 16);
    }
    void setVertex(int n, const float coords[3], const float normal[3], unsigned argb, const float tex_coords[2][2]) override {
        dxVertex* t = locked_verts + n;
        memcpy(t->coords, coords, 12);
        memcpy(t->normal, normal, 12);
        t->argb = argb;
        memcpy(t->tex_coords, tex_coords, 16);
    }
    void setSkinVertex(int n, const float coords[3], const float normal[3], unsigned argb, const float tex_coords[2][2], const unsigned char bone_indices[4], const float bone_weights[4]) override {
        dxSkinVertex* t = locked_skin_verts + n;
        memcpy(t->coords, coords, 12);
        memcpy(t->normal, normal, 12);
        t->argb = argb;
        memcpy(t->tex_coords, tex_coords, 16);
        for(int i = 0; i < 4; ++i) {
            t->blend_indices[i] = (float)bone_indices[i];
            t->blend_weights[i] = bone_weights[i];
        }
    }
    void setTriangle(int n, int v0, int v1, int v2) override {
        locked_indices[n * 3] = (WORD)v0;
        locked_indices[n * 3 + 1] = (WORD)v1;
        locked_indices[n * 3 + 2] = (WORD)v2;
    }
};

#endif