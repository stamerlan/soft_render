#include "display/display.h"
#include "matrix.h"
#include "message_queue.h"
#include "model/model.h"
#include "render/render.h"

static void set_viewport(const int w, const int h)
{
	extern Mat4x4f viewport;
	viewport.identity();

	viewport(0, 0) = w / 2.f;
	viewport(0, 3) = w / 2.f;

	viewport(1, 1) = (-h / 2.f);
	viewport(1, 3) = h / 2.f;
}

static void set_perspective_projection(const int w, const int h)
{
	extern Mat4x4f projection;
	projection.identity();
	projection(1, 1) = (float)w / (float)h;
}

static void lookat(const Vec3f& eye, const Vec3f& center, const Vec3f& up)
{
	Vec3f z = (eye - center).normalize();
	Vec3f x = (up ^ z).normalize();
	Vec3f y = (z ^ x).normalize();

	Mat4x4f Minv;
	Minv.identity();
	Mat4x4f Tr;
	Tr.identity();

	for (size_t i = 0; i < 3; i++) {
		Minv(0, i) = x[i];
		Minv(1, i) = y[i];
		Minv(2, i) = z[i];
		Tr(i, 3) = -center[i];
	}

	extern Mat4x4f model_view;
	model_view = Minv * Tr;

	extern Mat4x4f projection;
	projection(3, 3) = (eye - center).length();
}

int main(void)
{
	constexpr int w = 800;
	constexpr int h = 600;

	const Vec3f eye{ 0.5f, 0.25f, 1.f * ((float)w / (float)h) };
	const Vec3f center{ 0.f, 0.f, 0.f };

	render::init(w, h);
	auto [width, height] = display::get_resolution();

	Model obj("data/african_head.obj", "data/african_head_diffuse.tga");
	if (!obj.is_loaded())
		goto out;
	
	render::set_texture(obj.texture_image_, obj.texture_width_, obj.texture_height_);

	set_viewport(width, height);
	set_perspective_projection(width, height);
	lookat(eye, center, { 0.f, 1.f, 0.f });

	Message m;
	for (bool quit = false; !quit;) {
		while (display::get_msg(m))
			if (m.type == Message::type::QUIT)
				quit = true;

		render::clear();
		render::triangle(obj.faces_, obj.vertices_, obj.normals_, obj.texture_);
		render::update();
	}
 out:
	render::release();
	return 0;
}
