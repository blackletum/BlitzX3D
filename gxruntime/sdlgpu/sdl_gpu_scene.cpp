#include "sdl_gpu_scene.h"
#include "sdl_gpu_mesh.h"
#include "sdl_gpu_pipeline.h"
#include "sdl_gpu_texture.h"

#include "../std.h"

#include <SDL3/SDL_gpu.h>

namespace sdlgpu {

static void ReleaseTargetsLocked(SDL_GPUDevice* dev, GpuSceneFrame& frame) {
	if (frame.colorTarget) { SDL_ReleaseGPUTexture(dev, frame.colorTarget); frame.colorTarget = nullptr; }
	if (frame.depthTarget) { SDL_ReleaseGPUTexture(dev, frame.depthTarget); frame.depthTarget = nullptr; }
	frame.width = frame.height = 0;
}

void ReleaseSceneTargets(SDL_GPUDevice* dev, GpuSceneFrame& frame) {
	if (!dev) return;
	ReleaseTargetsLocked(dev, frame);
}

bool BeginSceneFrame(GpuSceneFrame& frame, SDL_GPUDevice* dev, unsigned w, unsigned h, float clearR, float clearG, float clearB) {
	if (!dev || !w || !h) return false;

	if (frame.dev != dev || frame.width != w || frame.height != h || !frame.colorTarget || !frame.depthTarget) {
		ReleaseTargetsLocked(dev, frame);
		frame.colorTarget = CreateColorTarget(dev, w, h);
		frame.depthTarget = CreateDepthTarget(dev, w, h, MeshDepthFormat(dev));
		if (!frame.colorTarget || !frame.depthTarget) {
			ReleaseTargetsLocked(dev, frame);
			return false;
		}
		frame.dev = dev;
		frame.width = w;
		frame.height = h;
	}

	frame.cmds = SDL_AcquireGPUCommandBuffer(dev);
	if (!frame.cmds) return false;

	SDL_GPUColorTargetInfo colorInfo{};
	colorInfo.texture = frame.colorTarget;
	colorInfo.load_op = SDL_GPU_LOADOP_CLEAR;
	colorInfo.store_op = SDL_GPU_STOREOP_STORE;
	colorInfo.clear_color = SDL_FColor{ clearR, clearG, clearB, 1.0f };

	SDL_GPUDepthStencilTargetInfo depthInfo{};
	depthInfo.texture = frame.depthTarget;
	depthInfo.load_op = SDL_GPU_LOADOP_CLEAR;
	depthInfo.store_op = SDL_GPU_STOREOP_STORE;
	depthInfo.stencil_load_op = SDL_GPU_LOADOP_DONT_CARE;
	depthInfo.stencil_store_op = SDL_GPU_STOREOP_DONT_CARE;
	depthInfo.clear_depth = 1.0f;
	depthInfo.clear_stencil = 0;

	frame.pass = SDL_BeginGPURenderPass(frame.cmds, &colorInfo, 1, &depthInfo);
	if (!frame.pass) {
		SDL_CancelGPUCommandBuffer(frame.cmds);
		frame.cmds = nullptr;
		return false;
	}
	SDL_GPUViewport vp{};
	vp.x = 0; vp.y = 0; vp.w = (float)w; vp.h = (float)h;
	vp.min_depth = 0.0f; vp.max_depth = 1.0f;
	SDL_SetGPUViewport(frame.pass, &vp);
	return true;
}

void RenderSceneMesh(GpuSceneFrame& frame, GpuMesh* mesh, const MeshUniforms& uniforms, SDL_GPUTexture* tex, int first_vert, int vert_cnt, int first_tri, int tri_cnt) {
	(void)vert_cnt;
	if (!frame.active() || !mesh || tri_cnt <= 0) return;

	unsigned indexCount = (unsigned)tri_cnt * 3;
	unsigned startIndex = (unsigned)first_tri * 3;
	DrawMesh(frame.dev, nullptr, frame.cmds, frame.pass, mesh, (const float*)&uniforms, (unsigned)sizeof(uniforms), tex, indexCount, startIndex, first_vert, SceneColorFormat(), MeshDepthFormat(frame.dev));
}

void EndSceneFrame(GpuSceneFrame& frame) {
	if (frame.pass) {
		SDL_EndGPURenderPass(frame.pass);
		frame.pass = nullptr;
	}
	if (frame.cmds) {
		SDL_SubmitGPUCommandBuffer(frame.cmds);
		frame.cmds = nullptr;
	}
}

bool PresentSceneFrame(SDL_GPUDevice* dev, SDL_Window* win, GpuSceneFrame& frame) {
	if (!dev || !win || !frame.colorTarget || !frame.width || !frame.height) return false;
	if (frame.pass || frame.cmds) return false;

	SDL_GPUCommandBuffer* cmds = SDL_AcquireGPUCommandBuffer(dev);
	if (!cmds) return false;

	SDL_GPUTexture* swap = nullptr;
	Uint32 sw = 0, sh = 0;
	if (!SDL_AcquireGPUSwapchainTexture(cmds, win, &swap, &sw, &sh)) {
		SDL_CancelGPUCommandBuffer(cmds);
		return false;
	}
	if (!swap) {
		SDL_SubmitGPUCommandBuffer(cmds);
		return true;
	}

	SDL_GPUBlitInfo blit{};
	blit.source.texture = frame.colorTarget;
	blit.source.w = frame.width;
	blit.source.h = frame.height;
	blit.destination.texture = swap;
	blit.destination.w = sw;
	blit.destination.h = sh;
	blit.load_op = SDL_GPU_LOADOP_DONT_CARE;
	blit.flip_mode = SDL_FLIP_NONE;
	blit.filter = SDL_GPU_FILTER_LINEAR;
	blit.cycle = false;
	SDL_BlitGPUTexture(cmds, &blit);

	return SDL_SubmitGPUCommandBuffer(cmds);
}

}
