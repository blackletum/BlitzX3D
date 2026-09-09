#include "sdl_gpu_texture.h"

#include "../std.h"
#include "../gxcanvas.h"

#include <cstring>
#include <unordered_map>
#include <vector>

#include <SDL3/SDL_gpu.h>

namespace sdlgpu {

static struct CanvasTexEntry {
	SDL_GPUTexture* tex = nullptr;
	SDL_GPUDevice* dev = nullptr;
	int modCnt = -1;
	unsigned w = 0, h = 0;
} ;

static std::unordered_map<gxCanvas*, CanvasTexEntry> g_canvasTexMap;

SDL_GPUTexture* GetCanvasTexture(SDL_GPUDevice* dev, gxCanvas* canvas) {
	if (!dev || !canvas) return nullptr;
	unsigned w = (unsigned)canvas->getWidth();
	unsigned h = (unsigned)canvas->getHeight();
	if (!w || !h) return nullptr;
	int mod = canvas->getModify();
	auto it = g_canvasTexMap.find(canvas);
	if (it != g_canvasTexMap.end() && it->second.tex && it->second.dev == dev && it->second.modCnt == mod && it->second.w == w && it->second.h == h) {
		return it->second.tex;
	}
	SDL_GPUTexture* old = nullptr;
	if (it != g_canvasTexMap.end()) { old = it->second.tex; g_canvasTexMap.erase(it); }
	if (old) SDL_ReleaseGPUTexture(dev, old);

	SDL_GPUTexture* tex = CreateTexture2D(dev, w, h);
	if (!tex) return nullptr;

	std::vector<unsigned char> rgba;
	rgba.resize((size_t)w * h * 4);

	if (!canvas->lock()) { SDL_ReleaseGPUTexture(dev, tex); return nullptr; }
	for (unsigned y = 0; y < h; ++y) {
		for (unsigned x = 0; x < w; ++x) {
			unsigned argb = canvas->getPixelFast((int)x, (int)y);
			unsigned char* dst = &rgba[(size_t)(y * w + x) * 4];
			dst[0] = (argb >> 16) & 0xff;
			dst[1] = (argb >> 8) & 0xff;
			dst[2] = argb & 0xff;
			dst[3] = (argb >> 24) & 0xff;
		}
	}
	canvas->unlock();

	if (!UploadTextureRGBA(dev, tex, w, h, rgba.data())) {
		SDL_ReleaseGPUTexture(dev, tex);
		return nullptr;
	}
	CanvasTexEntry e;
	e.tex = tex; e.dev = dev; e.modCnt = mod; e.w = w; e.h = h;
	g_canvasTexMap[canvas] = e;
	return tex;
}

SDL_GPUTexture* CreateTexture2D(SDL_GPUDevice* dev, unsigned w, unsigned h) {
	if (!dev || !w || !h) return nullptr;
	SDL_GPUTextureCreateInfo info{};
	info.type = SDL_GPU_TEXTURETYPE_2D;
	info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;
	info.width = w;
	info.height = h;
	info.layer_count_or_depth = 1;
	info.num_levels = 1;
	info.sample_count = SDL_GPU_SAMPLECOUNT_1;
	return SDL_CreateGPUTexture(dev, &info);
}

bool UploadTextureRGBA(SDL_GPUDevice* dev, SDL_GPUTexture* tex, unsigned w, unsigned h, const void* px) {
	if (!dev || !tex || !w || !h || !px) return false;
	Uint32 size = w * h * 4;

	SDL_GPUTransferBufferCreateInfo bufInfo{};
	bufInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
	bufInfo.size = size;
	SDL_GPUTransferBuffer* buf = SDL_CreateGPUTransferBuffer(dev, &bufInfo);
	if (!buf) return false;

	void* dst = SDL_MapGPUTransferBuffer(dev, buf, false);
	if (!dst) { SDL_ReleaseGPUTransferBuffer(dev, buf); return false; }
	memcpy(dst, px, size);
	SDL_UnmapGPUTransferBuffer(dev, buf);

	SDL_GPUCommandBuffer* cmds = SDL_AcquireGPUCommandBuffer(dev);
	if (!cmds) { SDL_ReleaseGPUTransferBuffer(dev, buf); return false; }
	SDL_GPUCopyPass* pass = SDL_BeginGPUCopyPass(cmds);

	SDL_GPUTextureTransferInfo src{};
	src.transfer_buffer = buf;
	src.pixels_per_row = w;
	src.rows_per_layer = h;

	SDL_GPUTextureRegion dstReg{};
	dstReg.texture = tex;
	dstReg.w = w;
	dstReg.h = h;
	dstReg.d = 1;

	SDL_UploadToGPUTexture(pass, &src, &dstReg, false);
	SDL_EndGPUCopyPass(pass);
	bool ok = SDL_SubmitGPUCommandBuffer(cmds);
	SDL_ReleaseGPUTransferBuffer(dev, buf);
	return ok;
}

void ReleaseTexture(SDL_GPUDevice* dev, SDL_GPUTexture* tex) {
	if (!dev || !tex) return;
	SDL_ReleaseGPUTexture(dev, tex);
}

SDL_GPUTexture* CreateColorTarget(SDL_GPUDevice* dev, unsigned w, unsigned h) {
	if (!dev || !w || !h) return nullptr;
	SDL_GPUTextureCreateInfo info{};
	info.type = SDL_GPU_TEXTURETYPE_2D;
	info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
	info.width = w;
	info.height = h;
	info.layer_count_or_depth = 1;
	info.num_levels = 1;
	info.sample_count = SDL_GPU_SAMPLECOUNT_1;
	return SDL_CreateGPUTexture(dev, &info);
}

SDL_GPUTexture* CreateDepthTarget(SDL_GPUDevice* dev, unsigned w, unsigned h, int formatValue) {
	if (!dev || !w || !h) return nullptr;
	SDL_GPUTextureCreateInfo info{};
	info.type = SDL_GPU_TEXTURETYPE_2D;
	info.format = (SDL_GPUTextureFormat)formatValue;
	info.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
	info.width = w;
	info.height = h;
	info.layer_count_or_depth = 1;
	info.num_levels = 1;
	info.sample_count = SDL_GPU_SAMPLECOUNT_1;
	return SDL_CreateGPUTexture(dev, &info);
}

}
