#include "display.h"
#include <vector>
#include <SDL.h>

static bool init_done;

static SDL_Window *window;
static SDL_Renderer *renderer;
static SDL_Texture *canvas;

static unsigned width;
static unsigned height;

static std::vector<uint32_t> framebuffer;

int display::init(int w, int h)
{
	if (w <= 0 || h <= 0)
		return 1;

	if (SDL_Init(SDL_INIT_VIDEO))
		goto ret;
	if (!(window = SDL_CreateWindow("soft_render", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, 0)))
		goto quit;
	if (!(renderer = SDL_CreateRenderer(window, -1, 0)))
		goto destroy_wnd;
	if (!(canvas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, w, h)))
		goto destroy_rend;

	framebuffer.resize(w * h);
	width = w;
	height = h;
	init_done = true;

	return 0;

 destroy_rend:
	SDL_DestroyRenderer(renderer);
	renderer = nullptr;
 destroy_wnd:
	SDL_DestroyWindow(window);
	window = nullptr;
 quit:
	SDL_Quit();
 ret:
	return 2;
}

void display::release(void)
{
	if (canvas) {
		SDL_DestroyTexture(canvas);
		canvas = nullptr;
	}
	if (renderer) {
		SDL_DestroyRenderer(renderer);
		renderer = nullptr;
	}
	if (window) {
		SDL_DestroyWindow(window);
		window = nullptr;
	}
	SDL_Quit();

	framebuffer.resize(0);
	width = height = 0;
	init_done = false;
}

void display::clear(uint32_t color)
{
	for (auto& i : framebuffer)
		i = color | (uint32_t)0xFF000000;
}

void display::put(int x, int y, uint32_t color)
{
	if (x < 0 || y < 0 || (unsigned)x >= width || (unsigned)y >= height)
		return;

	framebuffer[(size_t)y * width + x] = (uint32_t)0xFF000000 | color;
}

std::tuple<int, int> display::get_resolution(void)
{
	return { width, height };
}

int display::update(void)
{
	void *pixels;
	int pitch;

	if (SDL_LockTexture(canvas, nullptr, &pixels, &pitch))
		return 1;

	uint32_t *src = framebuffer.data();
	for (size_t row = 0; row < height; row++) {
		void *dst = (void *)((uintptr_t)pixels + row * pitch);
		memcpy(dst, src, sizeof(uint32_t) * width);
		src += width;
	}

	SDL_UnlockTexture(canvas);

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, canvas, nullptr, nullptr);
	SDL_RenderPresent(renderer);

	return 0;
}

int display::get_msg(Message& m)
{
	if (!init_done)
		return 0;

	SDL_Event evt;
	if (!SDL_PollEvent(&evt))
		return 0;

	switch (evt.type) {
	case SDL_QUIT:
		m.type = Message::type::QUIT;
		return 1;
	case SDL_KEYDOWN:
		if (evt.key.keysym.sym == SDLK_ESCAPE || evt.key.keysym.sym == SDLK_SPACE) {
			m.type = Message::type::QUIT;
			return 1;
		}
		break;
	}

	return 0;
}
