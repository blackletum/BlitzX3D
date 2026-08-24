#ifndef GFXMESH_H
#define GFXMESH_H

class gxMesh {
public:
	virtual ~gxMesh() {}

	static const int MESH_DYNAMIC = 1;
	static const int MESH_SKINNED = 2;
	static const int MAX_SKIN_BONES = 64;
	static const int MAX_VERTEX_BONES = 4;

	struct Vertex {
		float coords[3];
		float normal[3];
		unsigned argb;
		float tex_coords[4];   // 2 sets x 2 floats
	};

	struct SkinVertex {
		float coords[3];
		float normal[3];
		unsigned argb;
		float tex_coords[4];   // 2 sets x 2 floats again
		float blend_indices[4];
		float blend_weights[4];
	};

	virtual int maxVerts() const = 0;
	virtual int maxTris()  const = 0;

	virtual bool dirty() const = 0;
	virtual bool isSkinned() const = 0;

	virtual void render(int first_vert, int vert_cnt, int first_tri, int tri_cnt) = 0;
	virtual void renderSkinned(int first_vert, int vert_cnt, int first_tri, int tri_cnt, const float* bone_data, int bone_cnt) = 0;

	virtual bool lock(bool all) = 0;
	virtual void unlock() = 0;

	virtual void setVertex(int n, const void* v) = 0;
	virtual void setVertex(int n, const float coords[3], const float normal[3], const float tex_coords[2][2]) = 0;
	virtual void setVertex(int n, const float coords[3], const float normal[3], unsigned argb, const float tex_coords[2][2]) = 0;
	virtual void setSkinVertex(int n, const float coords[3], const float normal[3], unsigned argb, const float tex_coords[2][2], const unsigned char bone_indices[4], const float bone_weights[4]) = 0;
	virtual void setTriangle(int n, int v0, int v1, int v2) = 0;
};

#endif
