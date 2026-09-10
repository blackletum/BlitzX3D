#include "sdl_gpu_mesh.h"
#include "sdl_gpu_pipeline.h"

#include "../std.h"

#include <cstdio>
#include <cstring>

#include <SDL3/SDL_gpu.h>

namespace sdlgpu {

GpuMesh* CreateMesh(SDL_GPUDevice* dev, unsigned vertStride, unsigned maxVerts, unsigned maxTris) {
	if (!dev || !vertStride || !maxVerts || !maxTris) return nullptr;
	GpuMesh* mesh = new GpuMesh;
	mesh->vertStride = vertStride;
	mesh->maxVerts = maxVerts;
	mesh->maxTris = maxTris;

	SDL_GPUBufferCreateInfo vertInfo{};
	vertInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
	vertInfo.size = vertStride * maxVerts;
	mesh->verts = SDL_CreateGPUBuffer(dev, &vertInfo);

	SDL_GPUBufferCreateInfo idxInfo{};
	idxInfo.usage = SDL_GPU_BUFFERUSAGE_INDEX;
	idxInfo.size = (unsigned)sizeof(unsigned short) * maxTris * 3;
	mesh->indices = SDL_CreateGPUBuffer(dev, &idxInfo);

	if (!mesh->verts || !mesh->indices) {
		ReleaseMesh(dev, mesh);
		return nullptr;
	}
	return mesh;
}

static bool UploadToBuffer(SDL_GPUDevice* dev, SDL_GPUBuffer* dst, const void* data, unsigned bytes) {
	SDL_GPUTransferBuffer* buf = AcquireUploadTransferBuffer(dev, bytes);
	if (!buf) return false;
	void* mapped = SDL_MapGPUTransferBuffer(dev, buf, false);
	if (!mapped) { ReleaseUploadTransferBuffer(dev, buf); return false; }
	memcpy(mapped, data, bytes);
	SDL_UnmapGPUTransferBuffer(dev, buf);

	SDL_GPUCommandBuffer* cmds = SDL_AcquireGPUCommandBuffer(dev);
	if (!cmds) { ReleaseUploadTransferBuffer(dev, buf); return false; }
	SDL_GPUCopyPass* pass = SDL_BeginGPUCopyPass(cmds);
	SDL_GPUTransferBufferLocation src{};
	src.transfer_buffer = buf;
	SDL_GPUBufferRegion region{};
	region.buffer = dst;
	region.size = bytes;
	SDL_UploadToGPUBuffer(pass, &src, &region, false);
	SDL_EndGPUCopyPass(pass);
	bool ok = SDL_SubmitGPUCommandBuffer(cmds);
	SDL_ReleaseGPUTransferBuffer(dev, buf);
	return ok;
}

bool UploadMesh(SDL_GPUDevice* dev, GpuMesh* mesh, const void* vertData, unsigned vertBytes, const void* idxData, unsigned idxBytes) {
	if (!dev || !mesh || !vertData || !idxData) return false;
	if (!vertBytes || vertBytes > mesh->vertStride * mesh->maxVerts) return false;
	if (!idxBytes || idxBytes > (unsigned)sizeof(unsigned short) * mesh->maxTris * 3) return false;
	unsigned total = vertBytes + idxBytes;
	SDL_GPUTransferBuffer* buf = AcquireUploadTransferBuffer(dev, total);
	if (!buf) return UploadToBuffer(dev, mesh->verts, vertData, vertBytes) && UploadToBuffer(dev, mesh->indices, idxData, idxBytes);
	void* mapped = SDL_MapGPUTransferBuffer(dev, buf, false);
	if (!mapped) { ReleaseUploadTransferBuffer(dev, buf); return false; }
	memcpy(mapped, vertData, vertBytes);
	memcpy((char*)mapped + vertBytes, idxData, idxBytes);
	SDL_UnmapGPUTransferBuffer(dev, buf);
	SDL_GPUCommandBuffer* cmds = SDL_AcquireGPUCommandBuffer(dev);
	if (!cmds) { ReleaseUploadTransferBuffer(dev, buf); return false; }
	SDL_GPUCopyPass* pass = SDL_BeginGPUCopyPass(cmds);
	SDL_GPUTransferBufferLocation src{};
	src.transfer_buffer = buf;
	SDL_GPUBufferRegion vr{}; vr.buffer = mesh->verts; vr.offset = 0; vr.size = vertBytes;
	src.offset = 0;
	SDL_UploadToGPUBuffer(pass, &src, &vr, false);
	SDL_GPUBufferRegion ir{}; ir.buffer = mesh->indices; ir.offset = 0; ir.size = idxBytes;
	src.offset = vertBytes;
	SDL_UploadToGPUBuffer(pass, &src, &ir, false);
	SDL_EndGPUCopyPass(pass);
	bool ok = SDL_SubmitGPUCommandBuffer(cmds);
	SDL_ReleaseGPUTransferBuffer(dev, buf);
	return ok;
}

void ReleaseMesh(SDL_GPUDevice* dev, GpuMesh* mesh) {
	if (!mesh) return;
	if (dev) {
		if (mesh->verts) SDL_ReleaseGPUBuffer(dev, mesh->verts);
		if (mesh->indices) SDL_ReleaseGPUBuffer(dev, mesh->indices);
	}
	delete mesh;
}

}
