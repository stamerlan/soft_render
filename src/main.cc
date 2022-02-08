#include <chrono>
#include <numbers>
#include "display/display.h"
#include "matrix.h"
#include "message_queue.h"
#include "model/model.h"
#include "render/render.h"

int main(void)
{
	constexpr int w = 600;
	constexpr int h = 600;

	const vec3f_t eye{ 0.f, 0.f, 3.f };
	const vec3f_t center{ 0.f, 0.f, 0.f };
	const vec3f_t up{ 0.f, 1.f, 0.f };

	render::init(w, h);
	auto [width, height] = display::get_resolution();

	model_t obj("data/african_head.obj", "data/african_head_diffuse.tga");
	if (!obj.is_loaded())
		goto out;
	
	render::set_texture(obj.texture_image_, obj.texture_width_, obj.texture_height_);
	render::lookat(eye, center, up);

	Message m;
	for (bool quit = false; !quit;) {
		while (display::get_msg(m))
			if (m.type == Message::type::QUIT)
				quit = true;

		static float angle{ 0.f };
		{
			using namespace std::chrono;
			static auto last_rotation_ts{ steady_clock::now() };
		
			auto elapsed = steady_clock::now() - last_rotation_ts;
			if (elapsed >= 20ms) {
				angle += std::numbers::pi_v<float> / 180.f;
				if (angle > 2 * std::numbers::pi_v<float>)
					angle -= 2 * std::numbers::pi_v<float>;
				last_rotation_ts = steady_clock::now();
			}
		}

		render::clear();
		render::model_mat::identity();
		render::line({ -2.f, 0.f, 0.f }, { 2.f, 0.f, 0.f }, 0xFF0000);
		render::line({ 0.f, -2.f, 0.f }, { 0.f, 2.f, 0.f }, 0x00FF00);
		render::line({ 0.f, 0.f, -4.f }, { 0.f, 0.f, 4.f }, 0x0000FF);

		render::model_mat::scale(0.75f, 0.75f, 1.f);
		render::model_mat::translate(-1.f, 0.f, -1.5f);
		render::model_mat::rotate_x(angle);
		render::triangle(obj.faces_, obj.vertices_, obj.normals_, obj.texture_);
		render::update();
	}
 out:
	render::release();
	return 0;
}
