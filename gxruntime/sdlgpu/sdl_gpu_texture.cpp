#include "sdl_gpu_texture.h"

#include "../std.h"
#include "../gxcanvas.h"
#include "sdl_gpu_pipeline.h"

#include <cstdio>
#include <cstring>
#include <unordered_map>
#include <vector>

#include <SDL3/SDL_gpu.h>
#include <SDL3/SDL_properties.h>

namespace sdlgpu {

static struct CanvasTexEntry {
	SDL_GPUTexture* tex = nullptr;
	SDL_GPUDevice* dev = nullptr;
	int modCnt = -1;
	unsigned w = 0, h = 0;
} ;

static std::unordered_map< ::gxCanvas*, CanvasTexEntry> g_canvasTexMap;
static std::unordered_map< ::gxCanvas*, CanvasTexEntry> g_canvasOverlayMap;

namespace {
	struct RetiredTexture { SDL_GPUTexture* tex = nullptr; SDL_GPUDevice* dev = nullptr; SDL_GPUFence* fence = nullptr; };
	std::vector<RetiredTexture> g_retiredTextures;
}

static void RetireTexture(SDL_GPUDevice* dev, SDL_GPUTexture* tex) {
	if (!dev || !tex) return;
	for (auto it = g_retiredTextures.begin(); it != g_retiredTextures.end(); ) {
		if (it->dev == dev && it->fence && SDL_QueryGPUFence(dev, it->fence)) {
			SDL_ReleaseGPUFence(dev, it->fence);
			SDL_ReleaseGPUTexture(dev, it->tex);
			it = g_retiredTextures.erase(it);
		} else ++it;
	}
	SDL_GPUCommandBuffer* cmds = SDL_AcquireGPUCommandBuffer(dev);
	SDL_GPUFence* fence = cmds ? SDL_SubmitGPUCommandBufferAndAcquireFence(cmds) : nullptr;
	if (!fence) {
		SDL_ReleaseGPUTexture(dev, tex);
		return;
	}
	g_retiredTextures.push_back({ tex, dev, fence });
}

void ReapRetiredTextures(SDL_GPUDevice* dev) {
	for (auto it = g_retiredTextures.begin(); it != g_retiredTextures.end(); ) {
		if (it->dev == dev && it->fence && SDL_QueryGPUFence(dev, it->fence)) {
			SDL_ReleaseGPUFence(dev, it->fence);
			SDL_ReleaseGPUTexture(dev, it->tex);
			it = g_retiredTextures.erase(it);
		} else ++it;
	}
}

void TeardownTexturePools(SDL_GPUDevice* dev) {
	for (auto it = g_retiredTextures.begin(); it != g_retiredTextures.end(); ) {
		if (it->dev == dev) {
			if (it->fence) SDL_ReleaseGPUFence(dev, it->fence);
			if (it->tex) SDL_ReleaseGPUTexture(dev, it->tex);
			it = g_retiredTextures.erase(it);
		} else ++it;
	}
	for (auto it = g_canvasTexMap.begin(); it != g_canvasTexMap.end(); ) {
		if (it->second.dev == dev) {
			if (it->second.tex) SDL_ReleaseGPUTexture(dev, it->second.tex);
			it = g_canvasTexMap.erase(it);
		} else ++it;
	}
	for (auto it = g_canvasOverlayMap.begin(); it != g_canvasOverlayMap.end(); ) {
		if (it->second.dev == dev) {
			if (it->second.tex) SDL_ReleaseGPUTexture(dev, it->second.tex);
			it = g_canvasOverlayMap.erase(it);
		} else ++it;
	}
}

SDL_GPUTexture* GetCanvasTexture(SDL_GPUDevice* dev, ::gxCanvas* canvas) {
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
	if (old) RetireTexture(dev, old);

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

SDL_GPUTexture* GetCanvasOverlayTexture(SDL_GPUDevice* dev, ::gxCanvas* canvas) {
	if (!dev || !canvas) return nullptr;
	unsigned w = (unsigned)canvas->getWidth();
	unsigned h = (unsigned)canvas->getHeight();
	if (!w || !h) return nullptr;
	int mod = canvas->getModify();
	auto it = g_canvasOverlayMap.find(canvas);
	if (it != g_canvasOverlayMap.end() && it->second.tex && it->second.dev == dev && it->second.modCnt == mod && it->second.w == w && it->second.h == h) {
		return it->second.tex;
	}
	SDL_GPUTexture* old = nullptr;
	if (it != g_canvasOverlayMap.end()) { old = it->second.tex; g_canvasOverlayMap.erase(it); }
	if (old) RetireTexture(dev, old);
	SDL_GPUTexture* tex = CreateTexture2D(dev, w, h);
	if (!tex) return nullptr;
	std::vector<unsigned char> rgba;
	rgba.resize((size_t)w * h * 4);
	if (!canvas->lock()) { SDL_ReleaseGPUTexture(dev, tex); return nullptr; }
	unsigned clsRgb = canvas->getClsColor() & 0x00ffffff;
	for (unsigned y = 0; y < h; ++y) {
		for (unsigned x = 0; x < w; ++x) {
			unsigned argb = canvas->getPixelFast((int)x, (int)y);
			unsigned rgb = argb & 0x00ffffff;
			unsigned a = (rgb == clsRgb) ? 0 : 255;
			unsigned char* dst = &rgba[(size_t)(y * w + x) * 4];
			dst[0] = (argb >> 16) & 0xff;
			dst[1] = (argb >> 8) & 0xff;
			dst[2] = argb & 0xff;
			dst[3] = (unsigned char)a;
		}
	}
	canvas->unlock();
	if (!UploadTextureRGBA(dev, tex, w, h, rgba.data())) { SDL_ReleaseGPUTexture(dev, tex); return nullptr; }
	CanvasTexEntry e; e.tex = tex; e.dev = dev; e.modCnt = mod; e.w = w; e.h = h;
	g_canvasOverlayMap[canvas] = e;
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

	SDL_GPUTransferBuffer* buf = AcquireUploadTransferBuffer(dev, size);
	if (!buf) return false;

	void* dst = SDL_MapGPUTransferBuffer(dev, buf, true);
	if (!dst) { ReleaseUploadTransferBuffer(dev, buf); return false; }
	memcpy(dst, px, size);
	SDL_UnmapGPUTransferBuffer(dev, buf);

	SDL_GPUCommandBuffer* cmds = SDL_AcquireGPUCommandBuffer(dev);
	if (!cmds) { ReleaseUploadTransferBuffer(dev, buf); return false; }
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

	SDL_UploadToGPUTexture(pass, &src, &dstReg, true);
	SDL_EndGPUCopyPass(pass);
	SDL_GPUFence* fence = SDL_SubmitGPUCommandBufferAndAcquireFence(cmds);
	ReleaseUploadTransferBufferWithFence(dev, buf, fence);
	return fence != nullptr;
}

void ReleaseTexture(SDL_GPUDevice* dev, SDL_GPUTexture* tex) {
	if (!dev || !tex) return;
	SDL_ReleaseGPUTexture(dev, tex);
}

SDL_GPUTexture* CreateColorTarget(SDL_GPUDevice* dev, unsigned w, unsigned h) {
	return CreateColorTarget(dev, w, h, 0.0f, 0.0f, 0.0f, 1.0f);
}

SDL_GPUTexture* CreateColorTarget(SDL_GPUDevice* dev, unsigned w, unsigned h, float r, float g, float b, float a) {
	if (!dev || !w || !h) return nullptr;
	SDL_PropertiesID props = SDL_CreateProperties();
	if (props) {
		SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_R_FLOAT, r);
		SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_G_FLOAT, g);
		SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_B_FLOAT, b);
		SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_A_FLOAT, a);
	}
	SDL_GPUTextureCreateInfo info{};
	info.type = SDL_GPU_TEXTURETYPE_2D;
	info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
	info.usage = SDL_GPU_TEXTUREUSAGE_COLOR_TARGET | SDL_GPU_TEXTUREUSAGE_SAMPLER;
	info.width = w;
	info.height = h;
	info.layer_count_or_depth = 1;
	info.num_levels = 1;
	info.sample_count = SDL_GPU_SAMPLECOUNT_1;
	info.props = props;
	SDL_GPUTexture* tex = SDL_CreateGPUTexture(dev, &info);
	if (props) SDL_DestroyProperties(props);
	return tex;
}

SDL_GPUTexture* CreateDepthTarget(SDL_GPUDevice* dev, unsigned w, unsigned h, int formatValue) {
	return CreateDepthTarget(dev, w, h, formatValue, 1.0f, 0);
}

SDL_GPUTexture* CreateDepthTarget(SDL_GPUDevice* dev, unsigned w, unsigned h, int formatValue, float depth, unsigned char stencil) {
	if (!dev || !w || !h) return nullptr;
	SDL_PropertiesID props = SDL_CreateProperties();
	if (props) {
		SDL_SetFloatProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_DEPTH_FLOAT, depth);
		SDL_SetNumberProperty(props, SDL_PROP_GPU_TEXTURE_CREATE_D3D12_CLEAR_STENCIL_NUMBER, stencil);
	}
	SDL_GPUTextureCreateInfo info{};
	info.type = SDL_GPU_TEXTURETYPE_2D;
	info.format = (SDL_GPUTextureFormat)formatValue;
	info.usage = SDL_GPU_TEXTUREUSAGE_DEPTH_STENCIL_TARGET;
	info.width = w;
	info.height = h;
	info.layer_count_or_depth = 1;
	info.num_levels = 1;
	info.sample_count = SDL_GPU_SAMPLECOUNT_1;
	info.props = props;
	SDL_GPUTexture* tex = SDL_CreateGPUTexture(dev, &info);
	if (props) SDL_DestroyProperties(props);
	return tex;
}

}
