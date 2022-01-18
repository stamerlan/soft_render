#include "line.h"
#include <cmath>
#include <display/display.h>
#include <matrix.h>
#include <render/zbuf.h>

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

extern Mat4x4f model_view;
extern Mat4x4f viewport;
extern Mat4x4f projection;
void render::line(Vec3f p0, Vec3f p1, uint32_t color)
{
	auto r = viewport * projection * model_view * Mat4x1f{ p0.x, p0.y, p0.z, 1.f };
	Vec3f v0{ r / r(3, 0) };
	r = viewport * projection * model_view * Mat4x1f{ p1.x, p1.y, p1.z, 1.f };
	Vec3f v1{ r / r(3, 0) };

	bool steep = false;
	if (std::abs(v0.x - v1.x) < std::abs(v0.y - v1.y)) {
		std::swap(v0[0], v0[1]);
		std::swap(v1[0], v1[1]);
		steep = true;
	}

	if (v0.x > v1.x) {
		std::swap(v0, v1);
	}

	int dx = (int)(v1.x - v0.x);
	int dy = (int)(v1.y - v0.y);
	int y_dir = dy > 0 ? 1 : -1;

	int derr = std::abs(dy) * 2;
	int err = 0;

	float z_step = (v1.z - v0.z) / (dx + 1);
	float z = v0.z;
	for (int x = (int)v0.x, y = (int)v0.y; x <= v1.x; x++, z += z_step) {
		if (steep) {
			if (zbuf::depth_test(y, x, z))
				display::put(y, x, color);
		} else {
			if (zbuf::depth_test(x, y, z))
				display::put(x, y, color);
		}

		err += derr;
		if (err > dx) {
			y += y_dir;
			err -= dx * 2;
		}
	}
}
