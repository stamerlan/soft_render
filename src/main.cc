#include "display/display.h"
#include "matrix.h"
#include "message_queue.h"
#include "model/model.h"
#include "render/render.h"

/* Draw a line in one color, than draw the same line in backward direction with
 * the same color. Expected new line fully replaces the old one.
 */
static constexpr bool draw_backfill_test = false;
/* Draw steep line (one horizontal and one vertical). Expected continuous line
 * with no spaces inside.
 */
static constexpr bool draw_line_test = false;
/* Draw axis */
static constexpr bool draw_axis = false;
/* Draw a triangle, each vertex has its own color. Expected correct color
 * interpolation.
 */
static constexpr bool draw_color_interpolation_test = false;
/* Draw square to check if there are no holes between triangles */
static constexpr bool draw_square_test = false;
/* Draw two triangles which intersect each other in the middle. Expected correct
 * intersection in the middle of a triangle instead of overlapping.
 */
static constexpr bool draw_depth_test = false;

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
}

int main(void)
{
	constexpr int w = 800;
	constexpr int h = 600;

	const Vec3f eye{ 0.f, 0.f, 1.f };
	const Vec3f center{ -0.25f, 0.f, 0.f };

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

		if (draw_backfill_test) {
			render::line(0, 0, 600, 600, 0xFF0000);
			render::line(600, 600, 0, 0, 0xFFFFFF);
			//render::line(0, 600, 600, 0, 0xFFFFFF);
		}

		if (draw_line_test) {
			render::line(50, 75, 30, 275, 0xFFFF00);
			render::line(30, 275, 230, 255, 0x00FFFF);
		}

		if (draw_axis) {
			render::line({ 0., 0. }, { 0., 1. }, 0x00FF00);
			render::line({ 0., 0. }, { 1., 0. }, 0xFF0000);
		}

		if (draw_color_interpolation_test) {
			static const render::Vertex v0{ { 0.5f, -0.75f,  0.f}, {0.f, 0.f, 0.f}, {0.f, 0.f}, 0xFF0000 };
			static const render::Vertex v1{ { 0.0f, -0.25f, 0.f}, {0.f, 0.f, 0.f}, {0.f, 0.f}, 0x00FF00 };
			static const render::Vertex v2{ {-0.5f, -0.75f,  0.f}, {0.f, 0.f, 0.f}, {0.f, 0.f}, 0x0000FF };
			render::triangle(v0, v1, v2);
		}

		if (draw_square_test) {
			static const render::Vertex t[] = {
				{ {-0.5f,   0.f,   0.f}, {0.f, 0.f, 0.f}, {0.f, 0.f}, 0x808080 },
				{ {-0.75f, -0.5f,  0.f}, {0.f, 0.f, 0.f}, {0.f, 0.f}, 0x808080 },
				{ {-0.5f,  -0.75f, 0.f}, {0.f, 0.f, 0.f}, {0.f, 0.f}, 0x808080 },
				{ {-0.25f, -0.25f, 0.f}, {0.f, 0.f, 0.f}, {0.f, 0.f}, 0x808080 },
			};
			render::triangle(t[0], t[1], t[2]);
			render::triangle(t[2], t[3], t[0]);
		}

		if (draw_depth_test) {
			static const render::Vertex t1[3] = {
				{ {0.75f, 0.75f, 0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f}, 0xFF0000 },
				{ {0.25f, 0.50f, 1.0f}, {0.f, 0.f, 0.f}, {0.f, 0.f}, 0x00FF00 },
				{ {0.75f, 0.25f, 0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f}, 0x0000FF },
			};

			static const render::Vertex t2[3] = {
				{ {0.25f, 0.75f, 0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f}, 0xFF0000 },
				{ {0.25f, 0.25f, 0.5f}, {0.f, 0.f, 0.f}, {0.f, 0.f}, 0x0000FF },
				{ {0.75f, 0.50f, 1.0f}, {0.f, 0.f, 0.f}, {0.f, 0.f}, 0x00FF00 },
			};
			render::triangle(t1[0], t1[1], t1[2]);
			render::triangle(t2[0], t2[1], t2[2]);
		}

		render::triangle(obj.faces_, obj.vertices_, obj.normals_, obj.texture_);

		render::update();
	}
 out:
	render::release();
	return 0;
}
