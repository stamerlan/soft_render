#include "render.h"
#include <display/display.h>

static bool zbuf_enabled = true;

/* Apply model rotation, scaling, transformation.
 * In other words converts model coordinates to world coordinates:
 * model * v = world_coordinates
 */
static mat4x4f_t model;
/* Project world coordinates to camera space:
 * view * world_coordinates = camera_coordinates
 */
static mat4x4f_t view;
/* Apply perspective projection:
 * projection * camera_coordinates = homogeneous_coordinates
 */
static mat4x4f_t projection;
/* Convert homogeneous coordinates (-1.0, 1.0) to screen coordinates */
static mat4x4f_t viewport;

/* A product of (viewport * projection * view * model) */
static mat4x4f_t MVP;

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

vec3f_t render::project_to_screen(const vec3f_t& v)
{
	vec4f_t r = MVP * mat4x1f_t{ v.x, v.y, v.z, 1.f };
	r /= r.w;
	return r;
}

void render::lookat(const vec3f_t& eye, const vec3f_t& at, const vec3f_t& up)
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

	projection(3, 2) = -1.f / (eye - at).length();

	update_MVP();
}

void render::model_mat::identity(void)
{
	model.identity();
	update_MVP();
}

void render::model_mat::scale(float x, float y, float z)
{
	mat4x4f_t scaling;
	scaling(0, 0) = x;
	scaling(1, 1) = y;
	scaling(2, 2) = z;
	scaling(3, 3) = 1.f;

	model *= scaling;
	update_MVP();
}

void render::model_mat::translate(float x, float y, float z)
{
	mat4x4f_t translation;
	translation.identity();
	translation(0, 3) = x;
	translation(1, 3) = y;
	translation(2, 3) = z;

	model *= translation;
	update_MVP();
}

void render::model_mat::rotate_x(float angle)
{
	mat4x4f_t rotation;
	rotation.identity();

	float c = cos(angle);
	float s = sin(angle);

	rotation(1, 1) = c;
	rotation(1, 2) = -s;
	rotation(2, 1) = s;
	rotation(2, 2) = c;

	model *= rotation;
	update_MVP();
}
