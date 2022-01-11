#include "line.h"
#include <cmath>
#include <display/display.h>

void render::line(int x0, int y0, int x1, int y1, uint32_t color)
{
	bool steep = false;
	if (std::abs(x0 - x1) < std::abs(y0 - y1)) {
		std::swap(x0, y0);
		std::swap(x1, y1);
		steep = true;
	}

	if (x0 > x1) {
		std::swap(x0, x1);
		std::swap(y0, y1);
	}

	int dx = x1 - x0;
	int dy = y1 - y0;
	int y_dir = dy > 0 ? 1 : -1;

	int derr = std::abs(dy) * 2;
	int err = 0;

	for (int x = x0, y = y0; x <= x1; x++) {
		if (steep)
			display::put(y, x, color);
		else
			display::put(x, y, color);

		err += derr;
		if (err > dx) {
			y += y_dir;
			err -= dx * 2;
		}
	}
}

void render::line(Vec2f p0, Vec2f p1, uint32_t color)
{
	auto [width, height] = display::get_resolution();
	int x0 = (int)((p0.x + 1.f) * width / 2);
	int x1 = (int)((p1.x + 1.f) * width / 2);
	int y0 = (int)((1.f - p0.y) * height / 2);
	int y1 = (int)((1.f - p1.y) * height / 2);

	line(x0, y0, x1, y1, color);
}
