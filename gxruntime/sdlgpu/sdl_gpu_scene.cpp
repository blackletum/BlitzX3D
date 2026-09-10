#include "sdl_gpu_scene.h"
#include "sdl_gpu_mesh.h"
#include "sdl_gpu_pipeline.h"
#include "sdl_gpu_texture.h"

#include "../std.h"
#include "../gxcanvas.h"

#include <cstdio>
#include <SDL3/SDL_log.h>

#include <SDL3/SDL_gpu.h>

namespace sdlgpu {

static void ReleaseTargetsLocked(SDL_GPUDevice* dev, GpuSceneFrame& frame) {
	if (frame.colorTarget) { SDL_ReleaseGPUTexture(dev, frame.colorTarget); frame.colorTarget = nullptr; }
	if (frame.depthTarget) { SDL_ReleaseGPUTexture(dev, frame.depthTarget); frame.depthTarget = nullptr; }
	frame.width = frame.height = 0;
	frame.optimClearR = frame.optimClearG = frame.optimClearB = 0.0f;
	frame.optimClearA = 1.0f;
}

void ReleaseSceneTargets(SDL_GPUDevice* dev, GpuSceneFrame& frame) {
	if (!dev) return;
	ReleaseTargetsLocked(dev, frame);
}

bool BeginSceneFrame(GpuSceneFrame& frame, SDL_GPUDevice* dev, unsigned w, unsigned h, float clearR, float clearG, float clearB) {
	if (!dev || !w || !h) return false;

	if (frame.dev != dev || frame.width != w || frame.height != h || !frame.colorTarget || !frame.depthTarget) {
		ReleaseTargetsLocked(dev, frame);
		frame.colorTarget = CreateColorTarget(dev, w, h, clearR, clearG, clearB, 1.0f);
		frame.depthTarget = CreateDepthTarget(dev, w, h, MeshDepthFormat(dev), 1.0f, 0);
		if (!frame.colorTarget || !frame.depthTarget) {
			ReleaseTargetsLocked(dev, frame);
			return false;
		}
		frame.dev = dev;
		frame.width = w;
		frame.height = h;
	}
	frame.optimClearR = clearR;
	frame.optimClearG = clearG;
	frame.optimClearB = clearB;
	frame.optimClearA = 1.0f;

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

void RenderSceneMesh(GpuSceneFrame& frame, GpuMesh* mesh, const MeshUniforms& uniforms, SDL_GPUTexture* tex, int first_vert, int vert_cnt, int first_tri, int tri_cnt, bool alphaBlend, int cullMode) {
	if (!frame.active() || !frame.cmds || !frame.dev || !mesh || tri_cnt <= 0 || vert_cnt <= 0) return;
	if (first_vert < 0 || first_tri < 0) return;
	if ((unsigned)first_vert + (unsigned)vert_cnt > mesh->maxVerts) return;
	if ((unsigned)first_tri + (unsigned)tri_cnt > mesh->maxTris) return;
	if (SDL_GetGPUShaderFormats(frame.dev) == SDL_GPU_SHADERFORMAT_INVALID) return;

	unsigned indexCount = (unsigned)tri_cnt * 3;
	unsigned startIndex = (unsigned)first_tri * 3;
	DrawMesh(frame.dev, nullptr, frame.cmds, frame.pass, mesh, (const float*)&uniforms, (unsigned)sizeof(uniforms), tex, indexCount, startIndex, first_vert, SceneColorFormat(), MeshDepthFormat(frame.dev), alphaBlend, (SDL_GPUCullMode)cullMode);
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
	if (!cmds) {
		SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "SDL_AcquireGPUCommandBuffer failed: %s", SDL_GetError());
		return false;
	}

	SDL_GPUTexture* swap = nullptr;
	Uint32 sw = 0, sh = 0;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmds, win, &swap, &sw, &sh)) {
		SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "SDL_WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
		SDL_CancelGPUCommandBuffer(cmds);
		return false;
	}
	if (!swap) {
		if (!SDL_SubmitGPUCommandBuffer(cmds)) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Submit minimized frame failed: %s", SDL_GetError());
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

bool PresentSceneWithCanvas(SDL_GPUDevice* dev, SDL_Window* win, GpuSceneFrame& frame, ::gxCanvas* canvas) {
	if (!dev || !win) return false;
	bool has3D = frame.colorTarget && frame.width && frame.height && !frame.pass && !frame.cmds;
	if (!has3D && !canvas) return false;
	if (has3D && (frame.pass || frame.cmds)) return false;

	SDL_GPUCommandBuffer* cmds = SDL_AcquireGPUCommandBuffer(dev);
	if (!cmds) {
		SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "SDL_AcquireGPUCommandBuffer failed: %s", SDL_GetError());
		return false;
	}
	SDL_GPUTexture* swap = nullptr;
	Uint32 sw = 0, sh = 0;
	if (!SDL_WaitAndAcquireGPUSwapchainTexture(cmds, win, &swap, &sw, &sh)) {
		SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "SDL_WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
		SDL_CancelGPUCommandBuffer(cmds);
		return false;
	}
	if (!swap) { if (!SDL_SubmitGPUCommandBuffer(cmds)) SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Submit minimized frame failed: %s", SDL_GetError()); return true; }

	if (has3D) {
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
	}

	SDL_GPUTexture* canvasTex = canvas ? GetCanvasOverlayTexture(dev, canvas) : nullptr;
	if (canvasTex) {
		SDL_GPUColorTargetInfo ci{};
		ci.texture = swap;
		ci.load_op = has3D ? SDL_GPU_LOADOP_LOAD : SDL_GPU_LOADOP_CLEAR;
		ci.store_op = SDL_GPU_STOREOP_STORE;
		ci.clear_color = SDL_FColor{0,0,0,1};
		SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmds, &ci, 1, nullptr);
		if (pass) {
			SDL_GPUViewport vp{};
			vp.x = 0; vp.y = 0; vp.w = (float)sw; vp.h = (float)sh;
			vp.min_depth = 0.0f; vp.max_depth = 1.0f;
			SDL_SetGPUViewport(pass, &vp);
			DrawCanvasOverlay(dev, win, pass, canvasTex);
			SDL_EndGPURenderPass(pass);
		}
	} else if (!has3D) {
		SDL_GPUColorTargetInfo ci{};
		ci.texture = swap;
		ci.load_op = SDL_GPU_LOADOP_CLEAR;
		ci.store_op = SDL_GPU_STOREOP_STORE;
		ci.clear_color = SDL_FColor{0,0,0,1};
		SDL_GPURenderPass* pass = SDL_BeginGPURenderPass(cmds, &ci, 1, nullptr);
		if (pass) SDL_EndGPURenderPass(pass);
	}

	return SDL_SubmitGPUCommandBuffer(cmds);
}

}
