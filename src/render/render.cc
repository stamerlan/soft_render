#include "render.h"
#include <display/display.h>

static bool zbuf_enabled = true;

/* Apply model rotation, scaling, transformation.
 * In other words converts model coordinates to world coordinates:
 * model * v = world_coordinates
 */
static Mat4x4f model;
/* Project world coordinates to camera space:
 * view * world_coordinates = camera_coordinates
 */
static Mat4x4f view;
/* Apply perspective projection:
 * projection * camera_coordinates = homogeneous_coordinates
 */
static Mat4x4f projection;
/* Convert homogeneous coordinates (-1.0, 1.0) to screen coordinates */
static Mat4x4f viewport;

/* A product of (viewport * projection * view * model) */
static Mat4x4f MVP;

static void update_MVP(void)
{
	MVP = viewport * projection * view * model;
}

int render::init(int w, int h)
{
	if (display::init(w, h))
		return 1;
	if (zbuf::init(w, h)) {
		display::release();
		return 1;
	}

	model.identity();
	view.identity();
	projection.identity();

	viewport.identity();
	viewport(0, 0) = w / 2.f;
	viewport(0, 3) = w / 2.f;

	viewport(1, 1) = (-h / 2.f);
	viewport(1, 3) = h / 2.f;

	update_MVP();
	return 0;
}

void render::release(void)
{
	zbuf::release();
	display::release();
}

void render::clear(void)
{
	display::clear();
	zbuf::clear();
}

int render::update(void)
{
	return display::update();
}

bool render::is_zbuf_enabled(void)
{
	return zbuf_enabled;
}

void render::zbuf_enable(bool en)
{
	zbuf_enabled = en;
}

Vec3f render::project_to_screen(const Vec3f& v)
{
	auto r = MVP * Mat4x1f{ v.x, v.y, v.z, 1.f };
	return r;
}

void render::lookat(const Vec3f& eye, const Vec3f& at, const Vec3f& up)
{
	auto forward = (eye - at).normalize();
	auto up_norm = up;
	up_norm.normalize();
	auto right = up_norm ^ forward;

	view.identity();
	for (size_t i = 0; i < 3; i++) {
		view(0, i) = right[i];
		view(1, i) = up_norm[i];
		view(2, i) = forward[i];
	}

	update_MVP();
}
