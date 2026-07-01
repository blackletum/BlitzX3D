#ifndef GXMESH_H
#define GXMESH_H

#include <d3d9.h>

class gxGraphics;

class gxMesh {
public:
    struct dxVertex {
        float coords[3];
        float normal[3];
        unsigned argb;
        float tex_coords[4];   // 2 sets x 2 floats
    };

    struct dxSkinVertex {
        float coords[3];
        float weights[3];
        DWORD matrix_indices;
        float normal[3];
        unsigned argb;
        float tex_coords[4];
    };

    static const int MAX_BLEND_BONES = 4;

    gxMesh(gxGraphics* graphics, IDirect3DVertexBuffer9* verts, IDirect3DIndexBuffer9* indices, int max_verts, int max_tris, bool skinned);
    ~gxMesh();

    int maxVerts() const { return max_verts; }
    int maxTris()  const { return max_tris; }

    bool dirty() const { return mesh_dirty; }

    void render(int first_vert, int vert_cnt, int first_tri, int tri_cnt);
    void renderSkinned(int first_vert, int vert_cnt, int first_tri, int tri_cnt, const D3DMATRIX* bone_matrices, int bone_cnt);

    void backup();
    void restore();

    static const DWORD VTXFMT = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEXCOORDSIZE2(1);
    static const DWORD SKIN_VTXFMT = D3DFVF_XYZB4 | D3DFVF_LASTBETA_UBYTE4 | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX2 | D3DFVF_TEXCOORDSIZE2(0) | D3DFVF_TEXCOORDSIZE2(1);

private:
    gxGraphics* graphics;
    IDirect3DVertexBuffer9* vertex_buff;
    IDirect3DIndexBuffer9* index_buff;

    bool skinned;
    int  vertex_stride;
    int  max_verts, max_tris;
    bool mesh_dirty;
    char* locked_verts;
    WORD* locked_indices;

    /***** GX INTERFACE *****/
public:
    bool lock(bool all);
    void unlock();

    void setVertex(int n, const void* v) {
        memcpy(locked_verts + n, v, sizeof(dxVertex));
    }
    void setVertex(int n, const float coords[3], const float normal[3], const float tex_coords[2][2]) {
        dxVertex* t = reinterpret_cast<dxVertex*>(locked_verts + n * vertex_stride);
        memcpy(t->coords, coords, 12);
        memcpy(t->normal, normal, 12);
        t->argb = 0xffffffff;
        memcpy(t->tex_coords, tex_coords, 16);
    }
    void setVertex(int n, const float coords[3], const float normal[3], unsigned argb, const float tex_coords[2][2]) {
        dxVertex* t = reinterpret_cast<dxVertex*>(locked_verts + n * vertex_stride);
        memcpy(t->coords, coords, 12);
        memcpy(t->normal, normal, 12);
        t->argb = argb;
        memcpy(t->tex_coords, tex_coords, 16);
    }
    void setSkinVertex(int n, const float coords[3], const float normal[3], unsigned argb,
        const float tex_coords[2][2], const float weights[3], const unsigned char indices[4]) {
        dxSkinVertex* t = reinterpret_cast<dxSkinVertex*>(locked_verts + n * vertex_stride);
        memcpy(t->coords, coords, 12);
        memcpy(t->weights, weights, 12);
        t->matrix_indices = indices[0] | (indices[1] << 8) | (indices[2] << 16) | (indices[3] << 24);
        memcpy(t->normal, normal, 12);
        t->argb = argb;
        memcpy(t->tex_coords, tex_coords, 16);
    }
    void setTriangle(int n, int v0, int v1, int v2) {
        locked_indices[n * 3] = (WORD)v0;
        locked_indices[n * 3 + 1] = (WORD)v1;
        locked_indices[n * 3 + 2] = (WORD)v2;
    }
};

#endif