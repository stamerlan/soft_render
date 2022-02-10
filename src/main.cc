#include <chrono>
#include <iostream>
#include <numbers>
#include "display/display.h"
#include "matrix.h"
#include "message_queue.h"
#include "model/model.h"
#include "render/render.h"

int main(void)
{
	/* display resolution */
	constexpr int w = 600;
	constexpr int h = 600;

	/* camera position */
	const vec3f_t eye{ 0.f, 0.f, 3.f };
	const vec3f_t center{ 0.f, 0.f, 0.f };
	const vec3f_t up{ 0.f, 1.f, 0.f };

	/* model position */
	vec3f_t pos{ 0.f, 0.f, 0.f };

	render::init(w, h);
	auto [width, height] = display::get_resolution();

	model_t obj("data/african_head.obj", "data/african_head_diffuse.tga");
	if (!obj.is_loaded())
		goto out;
	
	render::set_texture(obj.texture_image_, obj.texture_width_, obj.texture_height_);
	render::lookat(eye, center, up);

	Message m;
	for (bool quit = false; !quit;) {
		while (display::get_msg(m)) {
			switch (m.type) {
			case Message::type::QUIT:
				quit = true;
				break;
			case Message::type::MOVE_FURTHER:
				pos.z -= 1.f;
				std::cout << "pos: " << pos << "\n";
				break;
			case Message::type::MOVE_CLOSER:
				pos.z += 1.f;
				std::cout << "pos: " << pos << "\n";
				break;
			case Message::type::MOVE_LEFT:
				pos.x -= 1.f;
				std::cout << "pos: " << pos << "\n";
				break;
			case Message::type::MOVE_RIGHT:
				pos.x += 1.f;
				std::cout << "pos: " << pos << "\n";
				break;
			}
		}

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

		render::model_mat::translate(pos.x, pos.y, pos.z);
		render::model_mat::rotate(angle, 0.f, 1.f, 0.f);
		render::triangle(obj.faces_, obj.vertices_, obj.normals_, obj.texture_);
		render::update();
	}
 out:
	render::release();
	return 0;
}
